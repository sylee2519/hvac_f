#include <iostream>
#include <vector>
#include <cstring>
#include <cstdio> 
#include <cassert>
#include <cstdlib>
#include <filesystem> 

#include "hvac_logging.h"
#include "hvac_fault.h"
#include "hvac_comm.h"

namespace fs = std::filesystem;
std::unordered_map<uint32_t, std::vector<Data>> data_storage;

void storeData(uint32_t node, const char* path, void *buffer, ssize_t size) {
    void *data_copy = malloc(size);
    if(data_copy == nullptr){
        fprintf(stderr, "Error: Memory allocation failed.\n");
        return;
    }

    memcpy(data_copy, buffer, size);
    
    Data data;
    strncpy(data.file_path, path, sizeof(data.file_path) - 1);
    data.file_path[sizeof(data.file_path) - 1] = '\0'; // ensure null terminated
    data.value = data_copy;
    data.size = size;
    
    data_storage[node].push_back(data);

    L4C_INFO("contents of data storage for node %u \n", node);
    for (const auto& stored_data : data_storage[node]) {
        L4C_INFO("File Path: %-50s  Size: %-10zd  Value: %p\n", stored_data.file_path, stored_data.size, stored_data.value);
        // Adjust the output format as per your requirements
                   }
                           L4C_INFO("\n");
}

void writeToFile(uint32_t node) {
    if (data_storage.find(node) == data_storage.end()) {
        fprintf(stderr, "Error: Node %d data not found.\n", node);
        return;
    }

    if (getenv("BBPATH") == NULL){
        L4C_ERR("Set BBPATH Prior to using HVAC");        
    }
    std::string nvmepath = std::string(getenv("BBPATH")) + "/XXXXXX"; 
    
    for (const auto& data : data_storage[node]) {
        char *newdir = (char *)malloc(strlen(nvmepath.c_str())+1);
        strcpy(newdir,nvmepath.c_str());
        mkdtemp(newdir);
        std::string dirpath = newdir;
        std::string filename = dirpath + "/" + fs::path(data.file_path).filename().string();

        FILE* file = fopen(filename.c_str(), "wb");
        if (!file) {
            fprintf(stderr, "Error opening file %s for writing.\n", filename.c_str());
            free(newdir);
            continue;
        }

        fwrite(data.value, 1, data.size, file);
        fclose(file);

        path_cache_map[data.file_path] = filename;

//        free(data.value);
        free(newdir);
    }
    hvac_client_comm_gen_update_rpc(path_cache_map);

    // Clear data after writing
//    data_storage[node].clear();

}

void emptyStore() {
    data_storage.clear();
}

