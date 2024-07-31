#ifndef __HVAC_FAULT_H__
#define __HVAC_FAULT_H__

#include <unordered_map>
#include <vector>
#include <map>
#include <string>
#include <algorithm>
#include "hvac_comm.h"

std::vector<FileData> exclusive_to_send;
std::unordered_map<uint32_t, std::vector<FileData>> data_storage;
std::map<int, std::string> fd_path_map;
//std::string my_address;
#endif
