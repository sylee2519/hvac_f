# This is primarily a cpp code but will use log4c to log both the C and C++ aspects
include(FindPkgConfig)

# Check for Mercury and log4c
pkg_check_modules(MERCURY REQUIRED mercury)
pkg_check_modules(LOG4C REQUIRED log4c)

# Dynamic Target for hvac_client
add_library(hvac_client SHARED 
  hvac.cpp 
  hvac_client.cpp 
  wrappers.c 
  hvac_data_mover.cpp 
  hvac_logging.c 
  hvac_comm.cpp 
  hvac_comm_client.cpp
  hvac_fault_client.cpp
)

target_compile_definitions(hvac_client PUBLIC HVAC_CLIENT)
target_compile_definitions(hvac_client PUBLIC HVAC_PRELOAD)
target_include_directories(hvac_client PRIVATE ${CMAKE_SOURCE_DIR}/include ${LOG4C_INCLUDE_DIRS} ${MERCURY_INCLUDE_DIRS})

# Set RPATH properties if needed (adjust as necessary)
set_target_properties(hvac_client PROPERTIES BUILD_RPATH /scratch/s5104a21/lib/mercury/lib)

# Link against required libraries
target_link_libraries(hvac_client 
  PRIVATE pthread 
  PRIVATE dl 
  PRIVATE ${LOG4C_LIBRARIES} 
  PRIVATE ${MERCURY_LIBRARIES}
)

# Server Daemon target for hvac_server
add_executable(hvac_server 
  hvac.cpp 
  hvac_server.cpp 
  hvac_data_mover.cpp 
  hvac_comm.cpp 
  hvac_logging.c
)

target_compile_definitions(hvac_server PUBLIC HVAC_SERVER)
target_include_directories(hvac_server PRIVATE ${CMAKE_SOURCE_DIR}/include ${LOG4C_INCLUDE_DIRS} ${MERCURY_INCLUDE_DIRS})

# Set RPATH properties if needed (adjust as necessary)
set_target_properties(hvac_server PROPERTIES BUILD_RPATH /scratch/s5104a21/lib/mercury/lib)

# Link against required libraries
target_link_libraries(hvac_server 
  PRIVATE pthread 
  PRIVATE ${LOG4C_LIBRARIES} 
  PRIVATE rt 
  PRIVATE ${MERCURY_LIBRARIES}
)

# Install targets
install(TARGETS hvac_client DESTINATION lib)
install(TARGETS hvac_server DESTINATION bin)

