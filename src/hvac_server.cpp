#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "hvac_comm.h"
#include "hvac_data_mover_internal.h"


#define HVAC_SERVER 1

extern "C" {
#include "hvac_logging.h"
}

__thread bool tl_disable_redirect = false;
uint32_t hvac_server_count = 0;

struct hvac_lookup_arg {
	hg_class_t *hg_class;
	hg_context_t *context;
	hg_id_t id;
	hg_addr_t addr;
};

int hvac_start_comm_server(void)
{
    HG_Set_log_level("DEBUG");

    /* Start the data mover before anything else */
    pthread_t hvac_data_mover_tid;
    if (pthread_create(&hvac_data_mover_tid, NULL, hvac_data_mover_fn, NULL) != 0){
		L4C_FATAL("Failed to initialized mecury progress thread\n");
	}

    /* True means we're a listener */
    hvac_init_comm(true);

    /* Post our address */
    hvac_comm_list_addr(true);

    /* Register basic RPC */
    hvac_rpc_register();
    hvac_open_rpc_register();
    hvac_close_rpc_register();
    hvac_seek_rpc_register();
	hvac_write_rpc_register();


    while (1)
        sleep(1);

    return EXIT_SUCCESS;
}



int main(int argc, char **argv)
{
    int l_error = 0;

    // Quick and dirty for prototype
    // TODO actual arg parser
    if (argc < 2)
    {
        fprintf(stderr, "Please supply server count\n");
        exit(-1);
    }

    hvac_server_count = atoi(argv[1]);

    hvac_init_logging();
    L4C_INFO("Server process starting up");
    hvac_start_comm_server();
    L4C_INFO("HVAC Server process shutting down");
    return (l_error);
}
