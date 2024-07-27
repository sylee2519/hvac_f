
#include <string>
#include <iostream>
#include <map>	
#include <thread>

#include "hvac_comm.h"
#include "hvac_data_mover_internal.h"

extern "C" {
#include "hvac_logging.h"
#include <fcntl.h>
#include <cassert>
#include <unistd.h>
}

#define TIMEOUT_SECONDS 5 

/* RPC Block Constructs */
static hg_bool_t done = HG_FALSE;
static pthread_cond_t done_cond = PTHREAD_COND_INITIALIZER;
static pthread_mutex_t done_mutex = PTHREAD_MUTEX_INITIALIZER;

/* RPC Globals */
static hg_id_t hvac_client_rpc_id;
static hg_id_t hvac_client_open_id;
static hg_id_t hvac_client_close_id;
static hg_id_t hvac_client_seek_id;
ssize_t read_ret = -1;

/* Mercury Data Caching */
std::map<int, std::string> address_cache;
extern std::map<int, int > fd_redir_map;

/* for Fault tolerance */
std::vector<int> timeout_counters;
std::mutex timeout_mutex;

/* for logging */
hg_addr_t my_address = HG_ADDR_NULL;
char client_address[128];
int client_rank;

static hg_return_t
hvac_seek_cb(const struct hg_cb_info *info)
{
    hvac_seek_out_t out;
    ssize_t bytes_read = -1;


    HG_Get_output(info->info.forward.handle, &out);    
    //Set the SEEK OUTPUT
    bytes_read = out.ret;
    HG_Free_output(info->info.forward.handle, &out);
    HG_Destroy(info->info.forward.handle);

    /* signal to main() that we are done */
    pthread_mutex_lock(&done_mutex);
    done++;
    read_ret = bytes_read;
    pthread_cond_signal(&done_cond);
    pthread_mutex_unlock(&done_mutex);  
    return HG_SUCCESS;    
}

static hg_return_t
hvac_open_cb(const struct hg_cb_info *info)
{
    hvac_open_out_t out;
    struct hvac_open_state_t *hvac_open_state_p = (struct hvac_open_state_t *)info->arg;    
    log_info_t log_info;
	const struct hg_info *hgi; 
   
    assert(info->ret == HG_SUCCESS);
    HG_Get_output(info->info.forward.handle, &out);    
    gettimeofday(&log_info.clocktime, NULL);
    fd_redir_map[hvac_open_state_p->local_fd] = out.ret_status;
//	L4C_INFO("Open RPC Returned FD %d\n",out.ret_status);

	// sy: add - logging code
	char server_addr[128];
    char server_ip[128];
    hgi = HG_Get_info(info->info.forward.handle);
    if (hgi) {
        hg_size_t server_addr_str_size = sizeof(server_addr);
        HG_Addr_to_string(hgi->hg_class, server_addr, &server_addr_str_size, hgi->addr);
        extract_ip_portion(server_addr, server_ip, sizeof(server_ip));
        log_info.flag = (strcmp(server_ip, client_address) == 0) ? 1 : 0;
    } else {
        L4C_DEBUG("Failed to get client address info\n");
    }

	strncpy(log_info.filepath, hvac_open_state_p->filepath, sizeof(log_info.filepath));
    strncpy(log_info.request, "open", sizeof(log_info.request));
    log_info.client_rank = client_rank;
    log_info.server_rank = hvac_open_state_p->svr_hash;
    strncpy(log_info.expn, "CReceive", sizeof(log_info.expn));
    log_info.n_epoch = hvac_open_state_p->local_fd;
    log_info.n_batch = -1;

    logging_info(&log_info, "client");


    HG_Free_output(info->info.forward.handle, &out);
    HG_Destroy(info->info.forward.handle);

    /* signal to main() that we are done */
	// sy: modified logic 
    pthread_mutex_lock(hvac_open_state_p->mutex);
    *(hvac_open_state_p->done) = HG_TRUE;
    pthread_cond_signal(hvac_open_state_p->cond);
    pthread_mutex_unlock(hvac_open_state_p->mutex);

    free(hvac_open_state_p);

    return HG_SUCCESS;
}

/* callback triggered upon receipt of rpc response */
/* In this case there is no response since that call was response less */
static hg_return_t
hvac_read_cb(const struct hg_cb_info *info)
{
	hg_return_t ret;
    hvac_rpc_out_t out;
    struct hvac_rpc_state_t_client *hvac_rpc_state_p = (hvac_rpc_state_t_client *)info->arg;
	const struct hg_info *hgi;

    log_info_t log_info;
    assert(info->ret == HG_SUCCESS);
	if (info->ret != HG_SUCCESS) {
        L4C_INFO("RPC failed: %s", HG_Error_to_string(info->ret));
	} 
	else{
    /* decode response */
    	ret = HG_Get_output(info->info.forward.handle, &out);
    	gettimeofday(&log_info.clocktime, NULL);
		if (ret != HG_SUCCESS) {
    		L4C_INFO("Failed to get output: %s", HG_Error_to_string(ret));
   		}
		else {
			*(hvac_rpc_state_p->bytes_read) = out.ret;
		//	L4C_INFO("out.ret %d\n", out.ret);
			if (out.ret < 0) {
            	L4C_INFO("Server-side read failed with result: %zd", out.ret);
			}	
 	   		ret = HG_Free_output(info->info.forward.handle, &out);
			assert(ret == HG_SUCCESS);
		}
	} 

	// sy: add - logging code
	char server_addr[128];
    char server_ip[128];
	hgi = HG_Get_info(info->info.forward.handle);
    if (hgi) {
		hg_size_t server_addr_str_size = sizeof(server_addr);
        HG_Addr_to_string(hgi->hg_class, server_addr, &server_addr_str_size, hgi->addr);
		extract_ip_portion(server_addr, server_ip, sizeof(server_ip));
    	log_info.flag = (strcmp(server_ip, client_address) == 0) ? 1 : 0;
    } else {
        L4C_DEBUG("Failed to get client address info\n");
    }

	snprintf(log_info.filepath, sizeof(log_info.filepath), "fd_%d", hvac_rpc_state_p->local_fd);
    strncpy(log_info.request, "read", sizeof(log_info.request));
    log_info.client_rank = client_rank;
    log_info.server_rank = hvac_rpc_state_p->svr_hash;
    strncpy(log_info.expn, "CReceive", sizeof(log_info.expn));
    log_info.n_epoch = -1;
    log_info.n_batch = -1;
    gettimeofday(&log_info.clocktime, NULL);

    logging_info(&log_info, "client");
	
   /* clean up resources consumed by this rpc */
    ret = HG_Bulk_free(hvac_rpc_state_p->bulk_handle);
	assert(ret == HG_SUCCESS);
//	L4C_INFO("INFO: Freeing Bulk Handle"); //Does this deregister memory?

	ret = HG_Destroy(info->info.forward.handle);
	assert(ret == HG_SUCCESS);
    
	
    /* signal to main() that we are done */
	// sy: modified logic
	pthread_mutex_lock(hvac_rpc_state_p->mutex);
    *(hvac_rpc_state_p->done) = HG_TRUE;
    pthread_cond_signal(hvac_rpc_state_p->cond);
    pthread_mutex_unlock(hvac_rpc_state_p->mutex);	
 
	free(hvac_rpc_state_p);
//	L4C_INFO("after signaling\n");
//	L4C_INFO("done %d\n", done);
	
    return HG_SUCCESS;
}

void hvac_client_comm_register_rpc()
{   
    hvac_client_open_id = hvac_open_rpc_register();
    hvac_client_rpc_id = hvac_rpc_register();    
    hvac_client_close_id = hvac_close_rpc_register();
    hvac_client_seek_id = hvac_seek_rpc_register();
}

void hvac_client_block(uint32_t host, hg_bool_t *done, pthread_cond_t *cond, pthread_mutex_t *mutex)
{
    /* wait for callbacks to finish */
	// sy: modified logic + jh: timeout logic
	int wait_status;
    
    struct timespec timeout;
    clock_gettime(CLOCK_REALTIME, &timeout);
    timeout.tv_sec += TIMEOUT_SECONDS;

    pthread_mutex_lock(mutex);
    while (*done != HG_TRUE){
	    wait_status = pthread_cond_timedwait(cond, mutex, &timeout);
	    if (wait_status == ETIMEDOUT){
		    L4C_INFO("TIMEOUT: Timeout period elapsed in hvac_client_block");
			// Timeout counter update
			{
				std::lock_guard<std::mutex> lock(timeout_mutex);
            	timeout_counters[host]++;
			}
            pthread_mutex_unlock(mutex);
		    return;
	    }
     }
     pthread_mutex_unlock(mutex);

}

ssize_t hvac_read_block(uint32_t host, hg_bool_t *done, ssize_t *bytes_read, pthread_cond_t *cond, pthread_mutex_t *mutex)
{
	// sy: modified logic + jh: timeout logic

	struct timespec timeout;
	int wait_status;
    clock_gettime(CLOCK_REALTIME, &timeout);
    timeout.tv_sec += TIMEOUT_SECONDS;

//	L4C_INFO("before readblock\n");
	pthread_mutex_lock(mutex);

    while (*done != HG_TRUE) {
		wait_status = pthread_cond_timedwait(cond, mutex, &timeout);
        if (wait_status == ETIMEDOUT){
	    	L4C_INFO("TIMEOUT: HVAC remote read failed due to timeout; it will be redirected to pfs");
			// Timeout counter update
			{
				std::lock_guard<std::mutex> lock(timeout_mutex);
            	timeout_counters[host]++;
			}
		    pthread_mutex_unlock(mutex);
		    return -1;
    	}
    }
    ssize_t result = *bytes_read;
    pthread_mutex_unlock(mutex);
//	L4C_INFO("outside readblock\n");
	if (result < 0) {
        L4C_INFO("HVAC remote read failed with error %zd; %zd returned\n", result, result);
        return result;
    }
    return result;
	
}


ssize_t hvac_seek_block()
{
    ssize_t bytes_read;
    /* wait for callbacks to finish */
    pthread_mutex_lock(&done_mutex);
    while (done != HG_TRUE)
        pthread_cond_wait(&done_cond, &done_mutex);
    bytes_read = read_ret;
    pthread_mutex_unlock(&done_mutex);
    return bytes_read;
}


void hvac_client_comm_gen_close_rpc(uint32_t svr_hash, int fd, hvac_rpc_state_t_close* rpc_state)
{   
    hg_addr_t svr_addr; 
    hvac_close_in_t in;
    hg_handle_t handle; 
    int ret;

    /* Get address */
    svr_addr = hvac_client_comm_lookup_addr(svr_hash);        
	rpc_state->addr = svr_addr;	//sy: add
	rpc_state->host = svr_hash;

    /* create create handle to represent this rpc operation */
    hvac_comm_create_handle(svr_addr, hvac_client_close_id, &handle);
	rpc_state->handle = handle; //sy: add

    in.fd = fd_redir_map[fd];
	in.client_rank = client_rank;
	rpc_state->local_fd = fd;

	// sy: add - logging code
    log_info_t log_info;
	snprintf(log_info.filepath, sizeof(log_info.filepath), "fd_%d", fd);
    strncpy(log_info.request, "close", sizeof(log_info.request));

	char server_addr[128];
    size_t server_addr_str_size = sizeof(server_addr);
    ret = HG_Addr_to_string(hvac_comm_get_class(), server_addr, &server_addr_str_size, svr_addr);
    char server_ip[128];
    extract_ip_portion(server_addr, server_ip, sizeof(server_ip));
    log_info.flag = (strcmp(client_address, server_ip) == 0) ? 1 : 0;

    log_info.client_rank = client_rank;
    log_info.server_rank = svr_hash;
    strncpy(log_info.expn, "CRequest", sizeof(log_info.expn));
    log_info.n_epoch = -1;
    log_info.n_batch = -1;
    gettimeofday(&log_info.clocktime, NULL);

    logging_info(&log_info, "client");

    ret = HG_Forward(handle, NULL, NULL, &in);
    assert(ret == 0);

    fd_redir_map.erase(fd);
    HG_Destroy(handle);
    hvac_comm_free_addr(svr_addr);

    return;

}

void hvac_client_comm_gen_open_rpc(uint32_t svr_hash, string path, int fd, hvac_open_state_t *hvac_open_state_p)
{
	// sy: modified logic
    hg_addr_t svr_addr;
    hvac_open_in_t in;
    hg_handle_t handle;
    int ret;

    /* Get address */
    svr_addr = hvac_client_comm_lookup_addr(svr_hash);    

    /* Allocate args for callback pass through */
    hvac_open_state_p->local_fd = fd;

    /* create create handle to represent this rpc operation */    
    hvac_comm_create_handle(svr_addr, hvac_client_open_id, &handle);  

    in.path = (hg_string_t)malloc(strlen(path.c_str()) + 1 );
    sprintf(in.path,"%s",path.c_str());
//	strcpy(in.path, path.c_str());
	in.client_rank = client_rank;   
	in.localfd = fd;	

	strncpy(hvac_open_state_p->filepath, path.c_str(), sizeof(hvac_open_state_p->filepath) - 1);
	hvac_open_state_p->filepath[sizeof(hvac_open_state_p->filepath) - 1] = '\0'; // Ensure null termination
	hvac_open_state_p->svr_hash = svr_hash;

	// sy: add - logging code
    log_info_t log_info;
    strncpy(log_info.filepath, path.c_str(), sizeof(log_info.filepath));
    strncpy(log_info.request, "open", sizeof(log_info.request));
	
	char server_addr[128];
    size_t server_addr_str_size = sizeof(server_addr);
    ret = HG_Addr_to_string(hvac_comm_get_class(), server_addr, &server_addr_str_size, svr_addr);
    char server_ip[128];
    extract_ip_portion(server_addr, server_ip, sizeof(server_ip));
    log_info.flag = (strcmp(client_address, server_ip) == 0) ? 1 : 0;


    log_info.client_rank = client_rank;
    log_info.server_rank = svr_hash;  
    strncpy(log_info.expn, "CRequest", sizeof(log_info.expn));
	log_info.n_epoch = fd;
	log_info.n_batch = -1;
    gettimeofday(&log_info.clocktime, NULL);	
   
	logging_info(&log_info, "client"); 
	
    ret = HG_Forward(handle, hvac_open_cb, hvac_open_state_p, &in);
    assert(ret == 0);

    
    hvac_comm_free_addr(svr_addr);

    return;

}

void hvac_client_comm_gen_read_rpc(uint32_t svr_hash, int localfd, void *buffer, ssize_t count, off_t offset, hvac_rpc_state_t_client *hvac_rpc_state_p)
{
	//sy: modified logic
    hg_addr_t svr_addr;
    hvac_rpc_in_t in;
    const struct hg_info *hgi;
    int ret;
	
    svr_addr = hvac_client_comm_lookup_addr(svr_hash);

    /* set up state structure */
    hvac_rpc_state_p->size = count;


    /* This includes allocating a src buffer for bulk transfer */
    hvac_rpc_state_p->buffer = buffer;
    assert(hvac_rpc_state_p->buffer);
	hvac_rpc_state_p->bulk_handle = HG_BULK_NULL;

    /* create create handle to represent this rpc operation */
    hvac_comm_create_handle(svr_addr, hvac_client_rpc_id, &(hvac_rpc_state_p->handle));

    /* register buffer for rdma/bulk access by server */
    hgi = HG_Get_info(hvac_rpc_state_p->handle);
    assert(hgi);
    ret = HG_Bulk_create(hgi->hg_class, 1, (void**) &(buffer),
       &(hvac_rpc_state_p->size), HG_BULK_WRITE_ONLY, &(in.bulk_handle));

    hvac_rpc_state_p->bulk_handle = in.bulk_handle;
    assert(ret == HG_SUCCESS);
	hvac_rpc_state_p->local_fd = localfd; //sy: add
	hvac_rpc_state_p->offset = offset; //sy: add
    /* Send rpc. Note that we are also transmitting the bulk handle in the
     * input struct.  It was set above.
     */
    in.input_val = count;
    //Convert FD to remote FD
    in.accessfd = fd_redir_map[localfd];
	in.localfd = localfd; //sy: add
    in.offset = offset;
	in.client_rank = client_rank; //sy: add - for logging 

	hvac_rpc_state_p->svr_hash = svr_hash; //sy: add

	// sy: add - logging code
    log_info_t log_info;
	snprintf(log_info.filepath, sizeof(log_info.filepath), "fd_%d", localfd);
    strncpy(log_info.request, "read", sizeof(log_info.request));
	
	char server_addr[128];
    size_t server_addr_str_size = sizeof(server_addr);
    ret = HG_Addr_to_string(hvac_comm_get_class(), server_addr, &server_addr_str_size, svr_addr);
    char server_ip[128];
    extract_ip_portion(server_addr, server_ip, sizeof(server_ip));
    log_info.flag = (strcmp(client_address, server_ip) == 0) ? 1 : 0;    


	log_info.client_rank = client_rank;
    log_info.server_rank = svr_hash;
    strncpy(log_info.expn, "CRequest", sizeof(log_info.expn));
    log_info.n_epoch = -1;
    log_info.n_batch = -1;
    gettimeofday(&log_info.clocktime, NULL);

    logging_info(&log_info, "client");   
	 
    
    ret = HG_Forward(hvac_rpc_state_p->handle, hvac_read_cb, hvac_rpc_state_p, &in);
    assert(ret == 0);

    hvac_comm_free_addr(svr_addr);

    return;
}

void hvac_client_comm_gen_seek_rpc(uint32_t svr_hash, int fd, int offset, int whence)
{
    hg_addr_t svr_addr;
    hvac_seek_in_t in;
    hg_handle_t handle;
    int ret;
	pthread_mutex_lock(&done_mutex);
    read_ret = -1;
    done = HG_FALSE;
	pthread_mutex_unlock(&done_mutex);
    /* Get address */
    svr_addr = hvac_client_comm_lookup_addr(svr_hash);    

    /* Allocate args for callback pass through */    
    /* create create handle to represent this rpc operation */    
    hvac_comm_create_handle(svr_addr, hvac_client_seek_id, &handle);  

    in.fd = fd_redir_map[fd];
    in.offset = offset;
    in.whence = whence;
    

    ret = HG_Forward(handle, hvac_seek_cb, NULL, &in);
    assert(ret == 0);

    
    hvac_comm_free_addr(svr_addr);

    return;

}


//We've converted the filename to a rank
//Using standard c++ hashing modulo servers
//Find the address
hg_addr_t hvac_client_comm_lookup_addr(int rank)
{
	if (address_cache.find(rank) != address_cache.end())
	{
        hg_addr_t target_server;
        HG_Addr_lookup2(hvac_comm_get_class(), address_cache[rank].c_str(), &target_server);
		return target_server;
	}

	/* The hardway */
	char filename[PATH_MAX];
	char svr_str[PATH_MAX];
	int svr_rank = -1;
//	char *stepid = getenv("PMI_NAMESPACE");
	char *jobid = getenv("MY_JOBID");
	L4C_INFO("slurm jobid %s\n", jobid);
	hg_addr_t target_server;
	bool svr_found = false;
	FILE *na_config = NULL;
	sprintf(filename, "./.ports.cfg.%s", jobid);
//	sprintf(filename, "./.ports.cfg");
	na_config = fopen(filename,"r+");
   
	if (na_config == NULL) {
        L4C_DEBUG("Failed to open configuration file\n");
        return HG_ADDR_NULL;
    } 

	while (fscanf(na_config, "%d %s\n",&svr_rank, svr_str) == 2)
	{
		if (svr_rank == rank){
			L4C_INFO("Connecting to %s %d\n", svr_str, svr_rank);            
			svr_found = true;
            break;
		}
	}

	if (svr_found){
		//Do something
        address_cache[rank] = svr_str;
        HG_Addr_lookup2(hvac_comm_get_class(),svr_str,&target_server);		
	}

	return target_server;
}

void hvac_get_addr() {

    if(my_address == HG_ADDR_NULL){
        hg_addr_t client_addr;
        hg_size_t size = PATH_MAX;
        char addr_buffer[PATH_MAX];

        HG_Addr_self(hvac_comm_get_class(), &client_addr);
        HG_Addr_to_string(hvac_comm_get_class(), addr_buffer, &size, client_addr);
        HG_Addr_free(hvac_comm_get_class(), client_addr);

        std::string address = std::string(addr_buffer);
        HG_Addr_lookup2(hvac_comm_get_class(), address.c_str(), &my_address);
	
		extract_ip_portion(addr_buffer, client_address, sizeof(client_address));
	
		const char *rank_str = getenv("HOROVOD_RANK");
    	client_rank = (rank_str != NULL) ? atoi(rank_str) : -1;
    }
}

