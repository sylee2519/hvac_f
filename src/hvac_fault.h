#ifndef __HVAC_FAULT_H__
#define __HVAC_FAULT_H__

#include <unordered_map>
#include <vector>
#include <map>
#include <string>
#include <algorithm>
#include "hvac_comm.h"

// For Client
extern std::vector<FileData> exclusive_to_send;
extern std::vector<FileData> shared_data1;
extern std::vector<FileData> shared_data2;
extern std::unordered_map<uint32_t, std::vector<FileData>> client_data_storage1;
extern std::unordered_map<uint32_t, std::vector<FileData>> client_data_storage2;
extern std::mutex data_storage_mutex;
extern std::map<int, std::string> fd_path_map;
extern std::map<int, std::string> first_epoch_fd_path_map;
extern bool use_first_buffer; // for double buffering


// For Server
extern std::unordered_map<uint32_t, std::vector<FileData>> memory_data_storage;
extern std::vector<FileData> data_storage_to_flush;
extern std::mutex memoroy_data_storage_mutex;
extern std::mutex flush_data_storage_mutex;
extern std::condition_variable flush_data_cond;

#endif
