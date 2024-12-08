/*****************************************************************************
 * Author:     Christopher J. Zimmer
 *             Oak Ridge National Lab
 * Date:       11/11/2020
 * Purpose:    Functions & structures for intercepting I/O calls for caching.
 *
 * Updated:    01/08/2021 - Purpose moved to HVAC modified to build without 
 *             warning under C++ : Skeleton for HVAC Built and configured
 * 
 * Copyright 2020 UT Battelle, LLC
 *
 * This work was supported by the Oak Ridge Leadership Computing Facility at
 * the Oak Ridge National Laboratory, which is managed by UT Battelle, LLC for
 * the U.S. DOE (under the contract No. DE-AC05-00OR22725).
 *
 * This file is part of the HVAC project.
 ****************************************************************************/

#include <dlfcn.h>
#include <string.h>
#include <assert.h>

// sy: add for debugging purpose
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdarg.h>
#include <sys/resource.h>
#include <pthread.h>

#include "hvac_internal.h"
#include "hvac_logging.h"
#include "execinfo.h"

// Global symbol that will "turn off" all I/O redirection.  Set during init
// and shutdown to prevent us from getting into init loops that cause a
// segfault. (ie: fopen() calls hvac_init()) which needs to write a log
// message, so it calls fopen()...)
extern bool g_disable_redirect;

// Thread-local symbol for disabling redirects.  Used by the L4C_* macros
// to make sure I/O to our log files doesn't get redirected.
extern __thread bool tl_disable_redirect;

// sy: add to avoid unnecessary open
#define MAX_FDS 1024
#define MAX_PATH_LENGTH 1024

typedef struct {
    int fd;                 // File descriptor
    char path[MAX_PATH_LENGTH]; // File path
} FDEntry;

static FDEntry fd_table[MAX_FDS];
pthread_mutex_t fd_mutex = PTHREAD_MUTEX_INITIALIZER;
static bool fd_table_initialized = false; 

void initialize_fd_table_if_needed() {
    pthread_mutex_lock(&fd_mutex); // Lock the mutex
    if (!fd_table_initialized) {
        for (int i = 0; i < MAX_FDS; i++) {
            fd_table[i].fd = -1; // Mark as unused
            fd_table[i].path[0] = '\0'; // Empty path
        }
        fd_table_initialized = true;
    }
    pthread_mutex_unlock(&fd_mutex); // Unlock the mutex
}

void add_fd(int fd, const char *path) {
    initialize_fd_table_if_needed(); // Ensure the table is initialized

    pthread_mutex_lock(&fd_mutex); // Lock the mutex

    for (int i = 0; i < MAX_FDS; i++) {
        if (fd_table[i].fd == -1) { // Find an unused slot
            fd_table[i].fd = fd;
            strncpy(fd_table[i].path, path, MAX_PATH_LENGTH - 1);
            fd_table[i].path[MAX_PATH_LENGTH - 1] = '\0'; // Null-terminate
            break;
        }
    }

    pthread_mutex_unlock(&fd_mutex); // Unlock the mutex
}

void remove_fd(int fd) {
    initialize_fd_table_if_needed(); // Ensure the table is initialized

    pthread_mutex_lock(&fd_mutex); // Lock the mutex

    for (int i = 0; i < MAX_FDS; i++) {
        if (fd_table[i].fd == fd) {
            fd_table[i].fd = -1; // Mark as unused
            fd_table[i].path[0] = '\0'; // Clear the path
            break;
        }
    }

    pthread_mutex_unlock(&fd_mutex); // Unlock the mutex
}

const char* get_fd_path(int fd) {
    initialize_fd_table_if_needed(); // Ensure the table is initialized

    pthread_mutex_lock(&fd_mutex); // Lock the mutex

    const char *path = NULL;
    for (int i = 0; i < MAX_FDS; i++) {
        if (fd_table[i].fd == fd) {
            path = fd_table[i].path;
            break;
        }
    }

    pthread_mutex_unlock(&fd_mutex); // Unlock the mutex
    return path;
}


/* fopen wrapper */
FILE *WRAP_DECL(fopen)(const char *path, const char *mode)
{

	MAP_OR_FAIL(fopen);
	if (g_disable_redirect || tl_disable_redirect) return __real_fopen( path, mode);

	FILE *ptr = __real_fopen(path,mode);

	/*
	if (ptr != NULL)
	{
		int fd = fileno(ptr);
		if (hvac_track_file(path, O_RDONLY, &fd) != -1)
		{
			L4C_INFO("FOpen: Tracking File %s",path);
		}
	}	
	*/
	return ptr;
}


/* fopen wrapper */
FILE *WRAP_DECL(fopen64)(const char *path, const char *mode)
{

	MAP_OR_FAIL(fopen64);
	if (g_disable_redirect || tl_disable_redirect) return __real_fopen64( path, mode);

	FILE *ptr = __real_fopen64(path,mode);
/*
	if (ptr != NULL)
	{
		int fd = fileno(ptr);
		if (hvac_track_file(path, O_RDONLY, &fd) != -1)
		{
			L4C_INFO("FOpen64: Tracking File %s",path);
		}
	}	
*/	
	return ptr;
}

//sy: add to avoid unnecessary open
int generate_and_add_fd(const char *path) {
    struct rlimit rl;
    getrlimit(RLIMIT_NOFILE, &rl);
    int max_fd = rl.rlim_cur > MAX_FDS ? MAX_FDS : rl.rlim_cur;
    int fd;

    pthread_mutex_lock(&fd_mutex); // Lock the mutex for thread safety

    // Generate a random FD that is not already in use
    do {
        fd = rand() % max_fd;
        // Check if the fd already exists in the table
        int exists = 0;
        for (int i = 0; i < MAX_FDS; i++) {
            if (fd_table[i].fd == fd) {
                exists = 1;
                break;
            }
        }
        if (!exists) break; // Exit loop if fd is not in use
    } while (1);

    // Add the FD and the file path to the table
    for (int i = 0; i < MAX_FDS; i++) {
        if (fd_table[i].fd == -1) { // Find an unused slot
            fd_table[i].fd = fd;
            strncpy(fd_table[i].path, path, MAX_PATH_LENGTH - 1);
            fd_table[i].path[MAX_PATH_LENGTH - 1] = '\0'; // Null-terminate
            break;
        }
    }

    pthread_mutex_unlock(&fd_mutex); // Unlock the mutex

    return fd; // Return the generated FD
}




int WRAP_DECL(open)(const char *pathname, int flags, ...)
{
	int ret = 0;
	va_list ap;
	int mode = 0;
	int use_mode = 0; //sy: add // you can revert this change

	if (flags & O_CREAT)
	{
		va_start(ap, flags);
		mode = va_arg(ap, int);
		va_end(ap);
		use_mode = 1; //sy: add
	}

	MAP_OR_FAIL(open);
	if (g_disable_redirect || tl_disable_redirect){
		if (use_mode) { //sy: add
            return __real_open(pathname, flags, mode);
        }
		else {
			return __real_open(pathname, flags, mode);
		}
	}

	/* For now pass the open to GPFS  - I think the open is cheap
	 * possibly asychronous.
	 * If this impedes performance we can investigate a cheap way of generating
	 * an FD
	 */
	//ret = __real_open(pathname, flags, mode); //original code
//	ret = use_mode ? __real_open(pathname, flags, mode) : __real_open(pathname, flags); //sy: add

	else {
//		ret = generate_and_add_fd(pathname);
	// C++ code determines whether to track
		
/*		if (hvac_track_file(pathname, flags, ret))

		{
//			L4C_INFO("Open: Tracking File %s",pathname);
		}
*/
		int fd;
		ret = hvac_track_file(pathname, flags, &fd);
		if(ret <= 0){
			ret = use_mode ? __real_open(pathname, flags, mode) : __real_open(pathname, flags);
		}
		else{
			add_fd(ret, pathname);
		}
	}
	
	return ret;
}

int WRAP_DECL(open64)(const char *pathname, int flags, ...)
{
	int ret = 0;
	va_list ap;
	int mode = 0;


	if (flags & O_CREAT)
	{
		va_start(ap, flags);
		mode = va_arg(ap, int);
		va_end(ap);
	}


	MAP_OR_FAIL(open64);
	if (g_disable_redirect || tl_disable_redirect) return __real_open64(pathname, flags, mode);	


	if (mode)
	{
		ret = __real_open64(pathname, flags, mode);
	}
	else
	{
		ret = __real_open64(pathname, flags);
	}


	if (ret != -1)
	{
		int fd;
		if (hvac_track_file(pathname, flags, &fd) != -1){
			L4C_INFO("Open64: Tracking file %s",pathname);
			return fd;
		}
	}

	
	return ret;

}



int WRAP_DECL(close)(int fd)
{
	int ret = 0;
	/* Check if hvac data has been initialized? Can we possibly hit a close call before an open call? */
	MAP_OR_FAIL(close);
	if (g_disable_redirect || tl_disable_redirect) return __real_close(fd);

	const char *path = hvac_get_path(fd);
	if (path)
	{
//		L4C_INFO("Close to file %s",path);
		hvac_remove_fd(fd);
		remove_fd(fd);
	}

	else{
		if (get_fd_path(fd) == NULL) {

			if ((ret = __real_close(fd)) != 0)
//	if ((ret = __real_close(fd)) != 0)
			{
				L4C_PERROR("Error from close");
				return ret;
			}
		}
		else {
			remove_fd(fd);
		}

	}

	return ret;
}


ssize_t WRAP_DECL(read)(int fd, void *buf, size_t count)
{
	int ret = -1;
	
	//remove me
    MAP_OR_FAIL(read);	
	
    const char *path = hvac_get_path(fd);


	ret = hvac_remote_read(fd,buf,count);

	if (path)
    {
//        L4C_INFO("Read to file %s of size %ld returning %ld bytes",path,count,ret);
    }
	
	if (ret < 0)
	{
		ret = __real_read(fd,buf,count);	
	}
		
    return ret;
}

/* sy: function for debugging */
char *buffer_to_hex(const void *buf, size_t size) {
    const char *hex_digits = "0123456789ABCDEF";
    const unsigned char *buffer = (const unsigned char *)buf;
    char *hex_str = (char *)malloc(size * 2 + 1); // 2 hex chars per byte + null terminator
    if (!hex_str) {
        perror("malloc");
        return NULL;
    }
    for (size_t i = 0; i < size; ++i) {
        hex_str[i * 2] = hex_digits[(buffer[i] >> 4) & 0xF];
        hex_str[i * 2 + 1] = hex_digits[buffer[i] & 0xF];
    }
    hex_str[size * 2] = '\0'; // Null terminator
    return hex_str;
}


ssize_t WRAP_DECL(pread)(int fd, void *buf, size_t count, off_t offset)
{
	ssize_t ret = -1;
	MAP_OR_FAIL(pread);

	const char *path = hvac_get_path(fd);

	if (path)
	{                
//		L4C_INFO("pread to tracked file %s",path);
		
//		memset(buf, 0, count);
		ret = hvac_remote_pread(fd, buf, count, offset);

/*
            if (ret >0) {
		            char *hex_buf = buffer_to_hex(buf, ret);
                L4C_INFO("Buffer content after remote read: %s\n", hex_buf);
                free(hex_buf);
            }
		memset(buf, 0, count);
		ssize_t cnt = __real_pread(fd, buf, count, offset);
		 char *hex_buff = buffer_to_hex(buf,cnt);
            if (hex_buff) {
                L4C_INFO("Buffer content after real read: %s\n", hex_buff);
                free(hex_buff);
            }
			
                L4C_INFO("offset %d bytesRead original %d bytesRead hvac %d\n", offset, cnt, ret);
*/
/*		if(ret < 0){
			
			L4C_INFO("remote pread_error returned %s",path);
			ret = __real_pread(fd,buf,count,offset);
			L4C_INFO("readbytes %d\n", ret);
		}
*/
		 if (ret < 0){
            // On remote pread error, log the error
            L4C_INFO("remote pread_error returned %s", path);

            // Check if the path exists in fd_table
            const char *real_path = get_fd_path(fd);
            if (real_path != NULL)
            {
                // Open the file using the path
                int temp_fd = __real_open(real_path, O_RDONLY);
                if (temp_fd < 0)
                {
                    L4C_PERROR("Error opening file for real pread");
                    return ret; // Return error
                }

                // Perform the real pread
                ret = __real_pread(temp_fd, buf, count, offset);

                // Close the temporary FD after reading
                __real_close(temp_fd);

                L4C_INFO("readbytes %d\n", ret);
            }
            else
            {
                ret = __real_pread(fd, buf, count, offset);
                L4C_INFO("Fallback readbytes %d\n", ret);
            }
        }
	}
	else
	{
		ret = __real_pread(fd,buf,count,offset);
	}

	return ret;
}



ssize_t WRAP_DECL(read64)(int fd, void *buf, size_t count)
{
	//remove me
	MAP_OR_FAIL(read64);


	const char *path = hvac_get_path(fd);
	if (path)
	{
		L4C_INFO("Read64 to file %s of size %ld",path,count);
	}

	return __real_read64(fd,buf,count);
}

ssize_t WRAP_DECL(write)(int fd, const void *buf, size_t count)
{
	MAP_OR_FAIL(write);
	return __real_write(fd, buf, count);

	const char *path = hvac_get_path(fd);
	if (path)
	{
		L4C_ERR("Write to file %s of size %ld",path,count);
		assert(false);
	}

	return __real_write(fd, buf, count);
}

off_t WRAP_DECL(lseek)(int fd, off_t offset, int whence)
{
	MAP_OR_FAIL(lseek);
	if (g_disable_redirect || tl_disable_redirect) return __real_lseek(fd,offset,whence);

	if (hvac_file_tracked(fd)){
		L4C_INFO("Got an LSEEK on a tracked file %d %ld\n", fd, offset);	
		return hvac_remote_lseek(fd,offset,whence);
	}

	return __real_lseek(fd, offset, whence);
}

off64_t WRAP_DECL(lseek64)(int fd, off64_t offset, int whence)
{
	MAP_OR_FAIL(lseek64);
	if (g_disable_redirect || tl_disable_redirect) return __real_lseek64(fd,offset,whence);
	if (hvac_file_tracked(fd)){
		L4C_INFO("Got an LSEEK64 on a tracked file %d %ld\n", fd, offset);	
		return hvac_remote_lseek(fd,offset,whence);
	}
	return __real_lseek64(fd, offset, whence);
}

ssize_t WRAP_DECL(readv)(int fd, const struct iovec *iov, int iovcnt)
{
	MAP_OR_FAIL(readv);
	const char *path = hvac_get_path(fd);
	if (path)
	{
		L4C_INFO("Readv to tracked file %s",path);
	}

	return __real_readv(fd, iov, iovcnt);

}
/*
   void* WRAP_DECL(mmap)(void *addr, ssize_t length, int prot, int flags, int fd, off_t offset)
   {
   MAP_OR_FAIL(mmap);
   if (path)
   {
   L4C_INFO("MMAP to tracked file %s Length %ld Offset %ld",path, length, offset);
   }

   return __real_mmap(addr,length, prot, flags, fd, offset);

   }
   */


#if 0




size_t WRAP_DECL(fwrite)(const void *ptr, size_t size, size_t count, FILE *stream)
{
	MAP_OR_FAIL(fwrite);

	return __real_fwrite(ptr,size,count,stream);
}

int WRAP_DECL(fsync)(int fd)
{
	MAP_OR_FAIL(fsync);
	if (g_disable_redirect || tl_disable_redirect) return __real_fsync(fd);

	return __real_fsync(fd);
}

int WRAP_DECL(fdatasync)(int fd)
{
	MAP_OR_FAIL(fdatasync);
	if (g_disable_redirect || tl_disable_redirect) return __real_fdatasync(fd);

	return __real_fdatasync(fd);
}

off_t WRAP_DECL(lseek)(int fd, off_t offset, int whence)
{
	MAP_OR_FAIL(lseek);
	if (g_disable_redirect || tl_disable_redirect) return __real_lseek(fd,offset,whence);
	L4C_INFO("Got a LSEEK --- Damnit\n");
	return __real_lseek(fd, offset, whence);
}

off64_t WRAP_DECL(lseek64)(int fd, off64_t offset, int whence)
{
	MAP_OR_FAIL(lseek64);
	if (g_disable_redirect || tl_disable_redirect) return __real_lseek64(fd,offset,whence);
	if (hvac_file_tracked(fd))
		L4C_INFO("Got an LSEEK64 on a tracked file %d %ld\n", fd, offset);
	return __real_lseek64(fd, offset, whence);
}

/* fopen wrapper */
FILE *WRAP_DECL(fopen)(const char *path, const char *mode)
{

	MAP_OR_FAIL(fopen);
	if (g_disable_redirect || tl_disable_redirect) return __real_fopen( path, mode);


	L4C_INFO("Intercepted Fopen %s",path);

	return __real_fopen(path, mode);
}



bool check_open_mode(const int flags, bool ignore_check)
{
	//Always back out of RDONLY
	if ((flags & O_ACCMODE) == O_WRONLY) {
		return false;
	}

	if ((flags & O_APPEND)) {
		return false;
	}
	return true;
}

/* Wrappers */
int WRAP_DECL(fclose)(FILE *fp)
{
	int ret = 0;

	/* RTLD Next fclose call */
	MAP_OR_FAIL(fclose);

	if (g_disable_redirect || tl_disable_redirect) return __real_fclose(fp);

	if ((ret = __real_fclose(fp)) != 0)
	{
		L4C_PERROR("Error from fclose");
		return ret;
	}

	return ret;
}


ssize_t WRAP_DECL(pwrite)(int fd, const void *buf, size_t count, off_t offset)
{
	MAP_OR_FAIL(pwrite);
	return __real_pwrite(fd, buf, count, offset);
}


ssize_t WRAP_DECL(pread)(int fd, void *buf, size_t count, off_t offset)
{
	MAP_OR_FAIL(pread);
	return __real_pread(fd,buf,count,offset);
}


ssize_t WRAP_DECL(write)(int fd, const void *buf, size_t count)
{
	MAP_OR_FAIL(write);
	return __real_write(fd, buf, count);

	const char *path = hvac_get_path(fd);
	if (path)
	{
		L4C_INFO("Write to file %s of size %ld",path,count);
	}

	return __real_write(fd, buf, count);
}

#endif
