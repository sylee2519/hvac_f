
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
std::map<std::string, std::string> exclusive_data;
int firstEpochData = 1280 / 2; // data size divide by the number of clients
int tmp = 0;
uint32_t my_hash;


/* Mercury Data Caching */
std::map<int, std::string> address_cache;
extern std::map<int, int > fd_redir_map;

/* struct used to carry state of overall operation across callbacks */
struct hvac_rpc_state {
//    uint32_t value;
    hg_size_t size;
    void *buffer;
    hg_bulk_t bulk_handle;
    hg_handle_t handle;
    // sy: add
    hg_addr_t node;
    char path[256];
};



// Carry CB Information for CB
struct hvac_open_state{
    uint32_t local_fd;
};

// sy: add
// DJB2
struct CustomHash{
    std::size_t operator()(const std::string& str) const {
        std::size_t hash = 5381;
        for (char c : str) {
            hash = ((hash << 5) + hash) + c; // hash * 33 + c
        }
        return hash;
    }
};

// sy: add
// Find rank by address
int find_rank_by_addr(const std::map<int, std::string>& address_cache, hg_addr_t target_addr) {
    for (const auto& entry : address_cache) {
        hg_addr_t current_addr;
        if (HG_Addr_lookup2(hvac_comm_get_class(), entry.second.c_str(), &current_addr) == HG_SUCCESS) {
            if (HG_Addr_cmp(hvac_comm_get_class(), target_addr, current_addr) == 0) {
                HG_Addr_free(hvac_comm_get_class(), current_addr);
                return entry.first;
            }
            HG_Addr_free(hvac_comm_get_class(), current_addr);
        }
    }
    return -1; // Indicating not found
}

// sy: add
// Find Node
int find_valid_host(const std::string& input, int self_id) {
    CustomHash custom_hash;
    std::size_t hash_value = custom_hash(input);
    int host = hash_value % (g_hvac_server_count-1);

    if (host == self_id) {
        host = (host + 1) % (g_hvac_server_count);
    }

    return host;
}

// sy: add
// map serialization for update RPC
void* serialize_map(const std::map<std::string, std::string>& map, hg_size_t* out_size) {
    hg_size_t total_size = 0;

    for (const auto& pair : map) {
		L4C_INFO("Key: %s, Value: %s\n", pair.first.c_str(), pair.second.c_str());
        total_size += pair.first.size() + 1;  
        total_size += pair.second.size() + 1; 
    }

    char* buffer = (char*)malloc(total_size);
    if (!buffer) {
        return nullptr;
    }

    char* ptr = buffer;
    for (const auto& pair : map) {
        strcpy(ptr, pair.first.c_str());
        ptr += pair.first.size() + 1; // including Null ptr
        strcpy(ptr, pair.second.c_str());
        ptr += pair.second.size() + 1; // including Null ptr
    }

    *out_size = total_size;
    return buffer;
}




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
	tmp++;
	hvac_get_addr();

    if (firstEpochData > tmp && bytes_read > 0) {
		int cmp_result = HG_Addr_cmp(hvac_comm_get_class(), hvac_rpc_state_p->node, my_address);
		// Shared data
        if (cmp_result != 0){
			L4C_INFO("store data is called\n");
			storeData(hvac_rpc_state_p->node, hvac_rpc_state_p->path, hvac_rpc_state_p->buffer, bytes_read);
		}
		// Exclusive data copy
		else{
			L4C_INFO("exclusive copy is called\n");
    		std::string bufferString;
    		bufferString = static_cast<char*>(hvac_rpc_state_p->buffer);
			exclusive_data.insert({hvac_rpc_state_p->path, bufferString});
			int host = find_rank_by_addr(address_cache, my_address);
			host = find_valid_host(hvac_rpc_state_p->path, host); 
			hvac_client_comm_gen_update_rpc(0, exclusive_data);
		}
    }
    /* sy add */
	/* Need to be replaced by failure detection code */
	if(tmp == firstEpochData){
		writeToFile(hvac_rpc_state_p->node);
	}
    /* clean up resources consumed by this rpc */
	exclusive_data.clear(); // sy: add
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

/* sy: add */
/* callback function for udpate rpc */
static hg_return_t update_cb(const struct hg_cb_info *callback_info) {
	hvac_rpc_state *state = (hvac_rpc_state *)callback_info->arg;

    L4C_INFO("update_cb called\n");

    HG_Bulk_free(state->bulk_handle);

    free(state->buffer);

	HG_Destroy(state->handle);

	hg_return_t ret = HG_Addr_free(HG_Get_info(state->handle)->hg_class, state->node);
    if (ret != HG_SUCCESS) {
        L4C_INFO("Failed to free address\n");
    }

    free(state);

    L4C_INFO("update_cb completed\n");

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
    hvac_rpc_state_p->node = svr_addr;
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

/* sy: add */
/* Flag 0 for exclusive data copy, and 1 for path update after rebuilding process */
void hvac_client_comm_gen_update_rpc(int flag, const std::map<std::string, std::string>& path_cache_map)
{

    hvac_update_in_t in;
    hg_handle_t handle;
    struct hvac_rpc_state *hvac_rpc_state_p;
	hg_addr_t exclusive_addr;
	
    hvac_get_addr();
	hvac_rpc_state_p = (struct hvac_rpc_state *)malloc(sizeof(*hvac_rpc_state_p));

	hg_size_t bulk_size;
    hvac_rpc_state_p->buffer = serialize_map(path_cache_map, &bulk_size);
    assert(hvac_rpc_state_p->buffer);
	
	L4C_INFO("after setting paths to send\n");
    hvac_rpc_state_p->size = bulk_size;
    hvac_rpc_state_p->bulk_handle = HG_BULK_NULL;

    // Create handle to represent this RPC operation

	// In case of Exclusive data	
	if(flag == 0){
		int host = find_rank_by_addr(address_cache, my_address);
        host = find_valid_host(hvac_rpc_state_p->path, host);	
        L4C_INFO("Exclusive copy - Host %d", host);
	    exclusive_addr = hvac_client_comm_lookup_addr(host);	
    	hvac_comm_create_handle(exclusive_addr, hvac_client_update_id, &handle);
		hvac_rpc_state_p->node = exclusive_addr;
	}
	// In case of path update after failure
	else {
        L4C_INFO("update\n");
    	hvac_comm_create_handle(my_address, hvac_client_update_id, &handle);
		hvac_rpc_state_p->node = my_address;
	}

	hvac_rpc_state_p->handle = handle;	

	L4C_INFO("after create handle\n");		
    // Buffer registration
    const struct hg_info* hgi = HG_Get_info(handle);
    assert(hgi);
    int ret = HG_Bulk_create(hgi->hg_class, 1, &hvac_rpc_state_p->buffer, &hvac_rpc_state_p->size, HG_BULK_READ_ONLY, &in.bulk_handle);

	if (ret != 0) {
        fprintf(stderr, "Failed to create bulk handle\n");
        free(hvac_rpc_state_p->buffer);
        free(hvac_rpc_state_p);
		return;
    }

	L4C_INFO("after bulk create\n");		
	in.bulk_size = bulk_size;
    ret = HG_Forward(handle, update_cb, hvac_rpc_state_p, &in);
	if (ret != 0) {
        fprintf(stderr, "Failed to forward handle\n");
        HG_Bulk_free(in.bulk_handle);
        free(hvac_rpc_state_p->buffer);
        free(hvac_rpc_state_p);
		return;
    }

	L4C_INFO("after forward\n");		

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
