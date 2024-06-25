
#include <string>
#include <iostream>
#include <map>	

#include "hvac_comm.h"
#include "hvac_data_mover_internal.h"
// sy: add
#include "hvac_fault.h"

extern "C" {
#include "hvac_logging.h"
#include <fcntl.h>
#include <cassert>
#include <unistd.h>
}

#include <atomic>
std::atomic<bool> write_called{false};
#include <new>
#include <utility> 

/* RPC Block Constructs */
static hg_bool_t done = HG_FALSE;
static pthread_cond_t done_cond = PTHREAD_COND_INITIALIZER;
static pthread_mutex_t done_mutex = PTHREAD_MUTEX_INITIALIZER;

/* RPC Globals */
static hg_id_t hvac_client_rpc_id;
static hg_id_t hvac_client_open_id;
static hg_id_t hvac_client_close_id;
static hg_id_t hvac_client_seek_id;
// sy: add
static hg_id_t hvac_client_update_id;
ssize_t read_ret = -1;

/* Mercury Data Caching */
std::map<int, std::string> address_cache;
extern std::map<int, int > fd_redir_map;
std::string my_address;

/* struct used to carry state of overall operation across callbacks */
struct hvac_rpc_state {
    uint32_t value;
    hg_size_t size;
    void *buffer;
    hg_bulk_t bulk_handle;
    hg_handle_t handle;
    // sy: add
    uint32_t node;
    char path[256];
};

// Carry CB Information for CB
struct hvac_open_state{
    uint32_t local_fd;
};

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
    struct hvac_open_state *open_state = (struct hvac_open_state *)info->arg;    
    
    assert(info->ret == HG_SUCCESS);
    HG_Get_output(info->info.forward.handle, &out);    
    fd_redir_map[open_state->local_fd] = out.ret_status;
	L4C_INFO("Open RPC Returned FD %d\n",out.ret_status);
    HG_Free_output(info->info.forward.handle, &out);
    HG_Destroy(info->info.forward.handle);

    /* signal to main() that we are done */
    pthread_mutex_lock(&done_mutex);
    done++;
    pthread_cond_signal(&done_cond);
    pthread_mutex_unlock(&done_mutex);  
    return HG_SUCCESS;
}

/* callback triggered upon receipt of rpc response */
/* In this case there is no response since that call was response less */
static hg_return_t
hvac_read_cb(const struct hg_cb_info *info)
{
	hg_return_t ret;
    hvac_rpc_out_t out;
    ssize_t bytes_read = -1;
    struct hvac_rpc_state *hvac_rpc_state_p = (hvac_rpc_state *)info->arg;

    assert(info->ret == HG_SUCCESS);

    /* decode response */
    HG_Get_output(info->info.forward.handle, &out);
    bytes_read = out.ret;

    /* sy add */
    if (bytes_read > 0) {
		L4C_INFO("store data is called\n");
        storeData(hvac_rpc_state_p->node, hvac_rpc_state_p->path, hvac_rpc_state_p->buffer, bytes_read);
    }
    /* sy add */
	if(!write_called.exchange(true)){
		writeToFile(hvac_rpc_state_p->node);
	}
    /* clean up resources consumed by this rpc */
    ret = HG_Bulk_free(hvac_rpc_state_p->bulk_handle);
	assert(ret == HG_SUCCESS);
	L4C_INFO("INFO: Freeing Bulk Handle"); //Does this deregister memory?

    ret = HG_Free_output(info->info.forward.handle, &out);
	assert(ret == HG_SUCCESS);
    
	ret = HG_Destroy(info->info.forward.handle);
	assert(ret == HG_SUCCESS);
    
	free(hvac_rpc_state_p);

    /* signal to main() that we are done */
    pthread_mutex_lock(&done_mutex);
    done++;
    read_ret=bytes_read;
    pthread_cond_signal(&done_cond);
    pthread_mutex_unlock(&done_mutex);  
    
    return HG_SUCCESS;
}

void hvac_client_comm_register_rpc()
{   
    hvac_client_open_id = hvac_open_rpc_register();
    hvac_client_rpc_id = hvac_rpc_register();    
    hvac_client_close_id = hvac_close_rpc_register();
    hvac_client_seek_id = hvac_seek_rpc_register();
    hvac_client_update_id = hvac_update_rpc_register();
}

void hvac_client_block()
{
    /* wait for callbacks to finish */
    pthread_mutex_lock(&done_mutex);
    while (done != HG_TRUE)
        pthread_cond_wait(&done_cond, &done_mutex);
    pthread_mutex_unlock(&done_mutex);
}

ssize_t hvac_read_block()
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


void hvac_client_comm_gen_close_rpc(uint32_t svr_hash, int fd)
{   
    hg_addr_t svr_addr; 
    hvac_close_in_t in;
    hg_handle_t handle; 
    int ret;

    /* Get address */
    svr_addr = hvac_client_comm_lookup_addr(svr_hash);        

    /* create create handle to represent this rpc operation */
    hvac_comm_create_handle(svr_addr, hvac_client_close_id, &handle);

    in.fd = fd_redir_map[fd];

    ret = HG_Forward(handle, NULL, NULL, &in);
    assert(ret == 0);

    fd_redir_map.erase(fd);

    HG_Destroy(handle);
    hvac_comm_free_addr(svr_addr);

    return;

}

void hvac_client_comm_gen_open_rpc(uint32_t svr_hash, string path, int fd)
{
    hg_addr_t svr_addr;
    hvac_open_in_t in;
    hg_handle_t handle;
    struct hvac_open_state *hvac_open_state_p;
    int ret;
    done = HG_FALSE;

    /* Get address */
    svr_addr = hvac_client_comm_lookup_addr(svr_hash);    

    /* Allocate args for callback pass through */
    hvac_open_state_p = (struct hvac_open_state *)malloc(sizeof(*hvac_open_state_p));
    hvac_open_state_p->local_fd = fd;

    /* create create handle to represent this rpc operation */    
    hvac_comm_create_handle(svr_addr, hvac_client_open_id, &handle);  

    in.path = (hg_string_t)malloc(strlen(path.c_str()) + 1 );
    sprintf(in.path,"%s",path.c_str());
    
    
    // hvac_open_cb --> rpc callback function
    ret = HG_Forward(handle, hvac_open_cb, hvac_open_state_p, &in);
    assert(ret == 0);
 
    hvac_comm_free_addr(svr_addr);

    return;

}

void hvac_client_comm_gen_read_rpc(uint32_t svr_hash, int localfd, void *buffer, ssize_t count, off_t offset)
{
    hg_addr_t svr_addr;
    hvac_rpc_in_t in;
    const struct hg_info *hgi;
    int ret;
    struct hvac_rpc_state *hvac_rpc_state_p;
    done = HG_FALSE;
    read_ret = -1;

    /* Get address */
    svr_addr = hvac_client_comm_lookup_addr(svr_hash);

    /* set up state structure */
    hvac_rpc_state_p = (struct hvac_rpc_state *)malloc(sizeof(*hvac_rpc_state_p));
    hvac_rpc_state_p->size = count;
    
    /* sy: add */
    hvac_rpc_state_p->node = svr_hash;
	std::string path = fd_map[localfd];
    strncpy(hvac_rpc_state_p->path, path.c_str(), sizeof(hvac_rpc_state_p->path) - 1);
    hvac_rpc_state_p->path[sizeof(hvac_rpc_state_p->path) - 1] = '\0';
    /* sy: add */

    /* This includes allocating a src buffer for bulk transfer */
    hvac_rpc_state_p->buffer = buffer;
    assert(hvac_rpc_state_p->buffer);
    //hvac_rpc_state_p->value = 5;

    /* create create handle to represent this rpc operation */
    hvac_comm_create_handle(svr_addr, hvac_client_rpc_id, &(hvac_rpc_state_p->handle));

    /* register buffer for rdma/bulk access by server */
    hgi = HG_Get_info(hvac_rpc_state_p->handle);
    assert(hgi);
    ret = HG_Bulk_create(hgi->hg_class, 1, (void**) &(buffer),
       &(hvac_rpc_state_p->size), HG_BULK_WRITE_ONLY, &(in.bulk_handle));

    hvac_rpc_state_p->bulk_handle = in.bulk_handle;
    assert(ret == HG_SUCCESS);

    /* Send rpc. Note that we are also transmitting the bulk handle in the
     * input struct.  It was set above.
     */
    in.input_val = count;
    //Convert FD to remote FD
    in.accessfd = fd_redir_map[localfd];
    in.offset = offset;
    
    
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
    read_ret = -1;
    int ret;
    done = HG_FALSE;

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

// sy: add

void hvac_client_comm_gen_update_rpc(const std::map<std::string, std::string>& path_cache_map)
{
    hg_addr_t svr_addr;
    hvac_update_in_t in;
    hg_handle_t handle;
    struct hvac_rpc_state *hvac_rpc_state_p;
	
	
    hvac_client_get_addr();
		L4C_INFO("before lookup\n");		
    HG_Addr_lookup2(hvac_comm_get_class(), my_address.c_str(), &svr_addr);
	
		L4C_INFO("after lookup\n");		
//    hvac_rpc_state_p = (struct hvac_rpc_state *)malloc(sizeof(*hvac_rpc_state_p));
	hvac_rpc_state_p = new hvac_rpc_state;
	if (!hvac_rpc_state_p) {
        fprintf(stderr, "Failed to allocate memory for hvac_rpc_state_p\n");
        return;
    }	

    hg_size_t num_paths = path_cache_map.size();
    hg_size_t bulk_size = num_paths * sizeof(std::pair<std::string, std::string>);

    // Buffer allocation for bulk transfer
//    hvac_rpc_state_p->buffer = malloc(bulk_size);
	hvac_rpc_state_p->buffer = operator new(bulk_size);
    if (!hvac_rpc_state_p->buffer) {
        fprintf(stderr, "Failed to allocate buffer for bulk transfer\n");
//		free(hvac_rpc_state_p);
		delete hvac_rpc_state_p;
        return;
    }

		L4C_INFO("before pair\n");		
    std::pair<std::string, std::string>* paths = static_cast<std::pair<std::string, std::string>*>(hvac_rpc_state_p->buffer);
    size_t index = 0;
    for (const auto& entry : path_cache_map) {
//        paths[index++] = entry;
		new (&paths[index++]) std::pair<std::string, std::string>(entry); 
    }

		L4C_INFO("after setting paths to send\n");		
    hvac_rpc_state_p->size = bulk_size;

    // Create handle to represent this RPC operation
    hvac_comm_create_handle(svr_addr, hvac_client_update_id, &handle);
		L4C_INFO("after create handle\n");		

    // Buffer registration
    const struct hg_info* hgi = HG_Get_info(handle);
    assert(hgi);
    int ret = HG_Bulk_create(hgi->hg_class, 1, &hvac_rpc_state_p->buffer, &hvac_rpc_state_p->size, HG_BULK_READ_ONLY, &in.bulk_handle);
//    assert(ret == 0);

	 if (ret != 0) {
        fprintf(stderr, "Failed to create bulk handle\n");
        for (size_t i = 0; i < num_paths; ++i) {
            paths[i].~pair();
        }
        operator delete(hvac_rpc_state_p->buffer);
        delete hvac_rpc_state_p;
        return;
    }

		L4C_INFO("after bulk create\n");		
    in.num_paths = num_paths;
//    in.bulk_handle = hvac_rpc_state_p->bulk_handle;
//not needed directly assgiend at HG_Bulk_create
    ret = HG_Forward(handle, NULL, NULL, &in);
//    assert(ret == 0);
	if (ret != 0) {
        fprintf(stderr, "Failed to forward handle\n");
        HG_Bulk_free(in.bulk_handle);
        for (size_t i = 0; i < num_paths; ++i) {
            paths[i].~pair();
        }
        operator delete(hvac_rpc_state_p->buffer);
        delete hvac_rpc_state_p;
        return;
    }

		L4C_INFO("after forward\n");		
    HG_Bulk_free(in.bulk_handle);
	for (size_t i = 0; i < num_paths; ++i) {
        paths[i].~pair();
    }
	operator delete(hvac_rpc_state_p->buffer);
    delete hvac_rpc_state_p;
//    free(hvac_rpc_state_p->buffer);
//    free(hvac_rpc_state_p);
    HG_Destroy(handle);
    hvac_comm_free_addr(svr_addr);

		L4C_INFO("after free\n");		
    return;
}

void hvac_client_get_addr() {

    if(my_address.empty()){
		L4C_INFO("my_address empty\n");		
        hg_addr_t client_addr;
        hg_size_t size = PATH_MAX;
        char addr_buffer[PATH_MAX];

        HG_Addr_self(hvac_comm_get_class(), &client_addr);
        HG_Addr_to_string(hvac_comm_get_class(), addr_buffer, &size, client_addr);
        HG_Addr_free(hvac_comm_get_class(), client_addr);

        my_address = std::string(addr_buffer);
		L4C_INFO("my_address %s\n", my_address.c_str());		
    }
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
	char *jobid = getenv("SLURM_JOBID");
	hg_addr_t target_server;
	bool svr_found = false;
	FILE *na_config = NULL;
	sprintf(filename, "./.ports.cfg.%s", jobid);
	na_config = fopen(filename,"r+");
    

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
