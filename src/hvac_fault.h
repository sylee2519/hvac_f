#ifndef __HVAC_FAULT_H__
#define __HVAC_FAULT_H__

#include <unordered_map>
#include <vector>
#include <map>
#include <string>

struct Data {
    char file_path[256]; 
    void *value;
    ssize_t size;        
};

extern std::unordered_map<uint32_t, std::vector<Data>> data_storage;
extern std::map<std::string, std::string> path_cache_map;
extern std::string my_address;

void storeData(uint32_t node, const char* path, void *buffer, ssize_t size);
void writeToFile(uint32_t node);

#endif
