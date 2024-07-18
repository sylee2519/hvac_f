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

std::unordered_map<uint32_t, std::vector<Data>> data_storage;
std::map<std::tring, std::string> path_cache_map;
string my_address;
#endif