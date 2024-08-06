/* Data mover responsible for maintaining the NVMe state
 * and prefetching the data
 */
#include <filesystem>
#include <string>
#include <queue>
#include <iostream>
#include <fstream>

#include <pthread.h>
#include <string.h>

#include "hvac_logging.h"
#include "hvac_data_mover_internal.h"
#include "hvac_fault.h"
using namespace std;
namespace fs = std::filesystem;

pthread_cond_t data_cond = PTHREAD_COND_INITIALIZER;
pthread_mutex_t data_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t path_map_mutex = PTHREAD_MUTEX_INITIALIZER;
std::condition_variable flush_data_cond;

map<int,string> fd_to_path;
map<string, string> path_cache_map;
queue<string> data_queue;

void *hvac_data_mover_fn(void *args)
{
    queue<string> local_list;

    if (getenv("BBPATH") == NULL){
        L4C_ERR("Set BBPATH Prior to using HVAC");        
    }

    string nvmepath = string(getenv("BBPATH")) + "/XXXXXX";    

    while (1) {
        pthread_mutex_lock(&data_mutex);
        
		while(data_queue.empty()){
			pthread_cond_wait(&data_cond, &data_mutex);
        }
        /* We can do stuff here when signaled */
        while (!data_queue.empty()){
            local_list.push(data_queue.front());
            data_queue.pop();
        }

        pthread_mutex_unlock(&data_mutex);

        /* Now we copy the local list to the NVMes*/
        while (!local_list.empty())
        {
            char *newdir = (char *)malloc(strlen(nvmepath.c_str())+1);
            strcpy(newdir,nvmepath.c_str());
            mkdtemp(newdir);
            string dirpath = newdir;
            string filename = dirpath + string("/") + fs::path(local_list.front().c_str()).filename().string();

            try{
            	fs::copy(local_list.front(), filename);
				pthread_mutex_lock(&path_map_mutex); //sy: add
            	path_cache_map[local_list.front()] = filename;
				pthread_mutex_unlock(&path_map_mutex); //sy: add
							
	
            } catch (...)
            {
                L4C_INFO("Failed to copy %s to %s\n",local_list.front().c_str(), filename);
            }        
            local_list.pop();
        }
    }
    return NULL;
}
/*
void *hvac_data_flusher_fn(void *args) {
    if (getenv("BBPATH") == NULL) {
        L4C_ERR("Set BBPATH Prior to using HVAC");
    }

    string nvmepath = string(getenv("BBPATH")) + "/XXXXXX";

    while (1) {
        std::vector<FileData> to_flush;

		std::unique_lock<std::mutex> lock(flush_data_storage_mutex);
		if (data_storage_to_flush.empty()) {
    		// If there is no data to flush, wait for a signal
    		flush_data_cond.wait(lock, []{ return !data_storage_to_flush.empty(); });
		}

            to_flush = std::move(data_storage_to_flush);
            data_storage_to_flush.clear();
			lock.unlock(); 

        for (const FileData& file_data : to_flush) {
            char *newdir = (char *)malloc(strlen(nvmepath.c_str()) + 1);
            strcpy(newdir, nvmepath.c_str());
            mkdtemp(newdir);
            string dirpath = newdir;
            string filename = dirpath + string("/") + fs::path(file_data.filepath).filename().string();

            try {
                fs::create_directories(dirpath);
                std::ofstream file_stream(filename, std::ios::binary);
                if (file_stream.is_open()) {
                    file_stream.write(file_data.buffer.data(), file_data.buffer.size());
                    file_stream.close();

                    pthread_mutex_lock(&path_map_mutex);
                    path_cache_map[file_data.filepath] = filename;
                    pthread_mutex_unlock(&path_map_mutex);
                } else {
                    throw std::runtime_error("Failed to open file stream.");
                }
            } catch (const std::exception &e) {
                L4C_INFO("Failed to write %s to %s: %s\n", file_data.filepath.c_str(), filename.c_str(), e.what());
            } catch (...) {
                L4C_INFO("Failed to write %s to %s\n", file_data.filepath.c_str(), filename);
            }
        }
    }
    return NULL;
}
*/

void* hvac_data_flusher_fn(void* args) {
    if (getenv("BBPATH") == NULL) {
        L4C_ERR("Set BBPATH Prior to using HVAC");
    }

    string nvmepath = string(getenv("BBPATH")) + "/XXXXXX";

    while (1) {
        std::vector<FileData> to_flush;

        std::unique_lock<std::mutex> lock(flush_data_storage_mutex);
        if (data_storage_to_flush.empty()) {
            // If there is no data to flush, wait for a signal
            flush_data_cond.wait(lock, []{ return !data_storage_to_flush.empty(); });
        }

        to_flush = std::move(data_storage_to_flush);
        data_storage_to_flush.clear();
        lock.unlock();  // Unlock the mutex before processing

        /* Handle flushing of data_storage_to_flush */
        for (const FileData& file_data : to_flush) {
            char *newdir = (char *)malloc(strlen(nvmepath.c_str()) + 1);
            strcpy(newdir, nvmepath.c_str());
            mkdtemp(newdir);
            string dirpath = newdir;
            string filename = dirpath + string("/") + fs::path(file_data.filepath).filename().string();

            try {
                fs::create_directories(dirpath);
                std::ofstream file_stream(filename, std::ios::binary);
                if (file_stream.is_open()) {
                    file_stream.write(file_data.buffer.data(), file_data.buffer.size());
                    file_stream.close();

                    pthread_mutex_lock(&path_map_mutex);
                    path_cache_map[file_data.filepath] = filename;
                    pthread_mutex_unlock(&path_map_mutex);

                    // Print the original file path, new NVMe file path, and size
                    L4C_INFO("Original file path: %s\n", file_data.filepath.c_str());
                    L4C_INFO("New NVMe file path: %s\n", filename.c_str());
                    L4C_INFO("Size: %zu bytes\n", file_data.buffer.size());

                    // Print buffer content in hexadecimal
                    char* hex_str = buffer_to_hex(file_data.buffer.data(), file_data.buffer.size());
                    if (hex_str) {
                        L4C_INFO("Buffer content (hex): %s\n", hex_str);
                        free(hex_str);
                    }

                    // Read and print the original file content in hexadecimal
                    std::ifstream original_file(file_data.filepath, std::ios::binary);
                    if (original_file.is_open()) {
                        std::vector<char> original_buffer((std::istreambuf_iterator<char>(original_file)),
                                                           std::istreambuf_iterator<char>());
                        original_file.close();

                        char* original_hex_str = buffer_to_hex(original_buffer.data(), original_buffer.size());
                        if (original_hex_str) {
                            L4C_INFO("Original file content (hex): %s\n", original_hex_str);
                            free(original_hex_str);
                        }
                    } else {
                        L4C_ERR("Failed to open original file: %s\n", file_data.filepath.c_str());
                    }
                } else {
                    throw std::runtime_error("Failed to open file stream.");
                }
            } catch (const std::exception &e) {
                L4C_INFO("Failed to write %s to %s: %s\n", file_data.filepath.c_str(), filename.c_str(), e.what());
            } catch (...) {
                L4C_INFO("Failed to write %s to %s\n", file_data.filepath.c_str(), filename.c_str());
            }
        }
    }
    return NULL;
}

