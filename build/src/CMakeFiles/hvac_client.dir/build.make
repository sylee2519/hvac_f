# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.26

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:

#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:

# Disable VCS-based implicit rules.
% : %,v

# Disable VCS-based implicit rules.
% : RCS/%

# Disable VCS-based implicit rules.
% : RCS/%,v

# Disable VCS-based implicit rules.
% : SCCS/s.%

# Disable VCS-based implicit rules.
% : s.%

.SUFFIXES: .hpux_make_needs_suffix_list

# Command-line flag to silence nested $(MAKE).
$(VERBOSE)MAKESILENT = -s

#Suppress display of executed commands.
$(VERBOSE).SILENT:

# A target that is always out of date.
cmake_force:
.PHONY : cmake_force

#=============================================================================
# Set environment variables for the build.

# The shell in which to execute make rules.
SHELL = /bin/sh

# The CMake executable.
CMAKE_COMMAND = /apps/applications/cmake/3.26.2/bin/cmake

# The command to remove a file.
RM = /apps/applications/cmake/3.26.2/bin/cmake -E rm -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /scratch/s5104a21/hvactest

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /scratch/s5104a21/hvactest/build

# Include any dependencies generated for this target.
include src/CMakeFiles/hvac_client.dir/depend.make
# Include any dependencies generated by the compiler for this target.
include src/CMakeFiles/hvac_client.dir/compiler_depend.make

# Include the progress variables for this target.
include src/CMakeFiles/hvac_client.dir/progress.make

# Include the compile flags for this target's objects.
include src/CMakeFiles/hvac_client.dir/flags.make

src/CMakeFiles/hvac_client.dir/hvac.cpp.o: src/CMakeFiles/hvac_client.dir/flags.make
src/CMakeFiles/hvac_client.dir/hvac.cpp.o: /scratch/s5104a21/hvactest/src/hvac.cpp
src/CMakeFiles/hvac_client.dir/hvac.cpp.o: src/CMakeFiles/hvac_client.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/scratch/s5104a21/hvactest/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object src/CMakeFiles/hvac_client.dir/hvac.cpp.o"
	cd /scratch/s5104a21/hvactest/build/src && /apps/compiler/gcc/10.2.0/bin/g++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT src/CMakeFiles/hvac_client.dir/hvac.cpp.o -MF CMakeFiles/hvac_client.dir/hvac.cpp.o.d -o CMakeFiles/hvac_client.dir/hvac.cpp.o -c /scratch/s5104a21/hvactest/src/hvac.cpp

src/CMakeFiles/hvac_client.dir/hvac.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/hvac_client.dir/hvac.cpp.i"
	cd /scratch/s5104a21/hvactest/build/src && /apps/compiler/gcc/10.2.0/bin/g++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /scratch/s5104a21/hvactest/src/hvac.cpp > CMakeFiles/hvac_client.dir/hvac.cpp.i

src/CMakeFiles/hvac_client.dir/hvac.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/hvac_client.dir/hvac.cpp.s"
	cd /scratch/s5104a21/hvactest/build/src && /apps/compiler/gcc/10.2.0/bin/g++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /scratch/s5104a21/hvactest/src/hvac.cpp -o CMakeFiles/hvac_client.dir/hvac.cpp.s

src/CMakeFiles/hvac_client.dir/hvac_client.cpp.o: src/CMakeFiles/hvac_client.dir/flags.make
src/CMakeFiles/hvac_client.dir/hvac_client.cpp.o: /scratch/s5104a21/hvactest/src/hvac_client.cpp
src/CMakeFiles/hvac_client.dir/hvac_client.cpp.o: src/CMakeFiles/hvac_client.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/scratch/s5104a21/hvactest/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Building CXX object src/CMakeFiles/hvac_client.dir/hvac_client.cpp.o"
	cd /scratch/s5104a21/hvactest/build/src && /apps/compiler/gcc/10.2.0/bin/g++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT src/CMakeFiles/hvac_client.dir/hvac_client.cpp.o -MF CMakeFiles/hvac_client.dir/hvac_client.cpp.o.d -o CMakeFiles/hvac_client.dir/hvac_client.cpp.o -c /scratch/s5104a21/hvactest/src/hvac_client.cpp

src/CMakeFiles/hvac_client.dir/hvac_client.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/hvac_client.dir/hvac_client.cpp.i"
	cd /scratch/s5104a21/hvactest/build/src && /apps/compiler/gcc/10.2.0/bin/g++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /scratch/s5104a21/hvactest/src/hvac_client.cpp > CMakeFiles/hvac_client.dir/hvac_client.cpp.i

src/CMakeFiles/hvac_client.dir/hvac_client.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/hvac_client.dir/hvac_client.cpp.s"
	cd /scratch/s5104a21/hvactest/build/src && /apps/compiler/gcc/10.2.0/bin/g++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /scratch/s5104a21/hvactest/src/hvac_client.cpp -o CMakeFiles/hvac_client.dir/hvac_client.cpp.s

src/CMakeFiles/hvac_client.dir/wrappers.c.o: src/CMakeFiles/hvac_client.dir/flags.make
src/CMakeFiles/hvac_client.dir/wrappers.c.o: /scratch/s5104a21/hvactest/src/wrappers.c
src/CMakeFiles/hvac_client.dir/wrappers.c.o: src/CMakeFiles/hvac_client.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/scratch/s5104a21/hvactest/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_3) "Building C object src/CMakeFiles/hvac_client.dir/wrappers.c.o"
	cd /scratch/s5104a21/hvactest/build/src && /apps/compiler/gcc/10.2.0/bin/gcc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -MD -MT src/CMakeFiles/hvac_client.dir/wrappers.c.o -MF CMakeFiles/hvac_client.dir/wrappers.c.o.d -o CMakeFiles/hvac_client.dir/wrappers.c.o -c /scratch/s5104a21/hvactest/src/wrappers.c

src/CMakeFiles/hvac_client.dir/wrappers.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/hvac_client.dir/wrappers.c.i"
	cd /scratch/s5104a21/hvactest/build/src && /apps/compiler/gcc/10.2.0/bin/gcc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /scratch/s5104a21/hvactest/src/wrappers.c > CMakeFiles/hvac_client.dir/wrappers.c.i

src/CMakeFiles/hvac_client.dir/wrappers.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/hvac_client.dir/wrappers.c.s"
	cd /scratch/s5104a21/hvactest/build/src && /apps/compiler/gcc/10.2.0/bin/gcc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /scratch/s5104a21/hvactest/src/wrappers.c -o CMakeFiles/hvac_client.dir/wrappers.c.s

src/CMakeFiles/hvac_client.dir/hvac_data_mover.cpp.o: src/CMakeFiles/hvac_client.dir/flags.make
src/CMakeFiles/hvac_client.dir/hvac_data_mover.cpp.o: /scratch/s5104a21/hvactest/src/hvac_data_mover.cpp
src/CMakeFiles/hvac_client.dir/hvac_data_mover.cpp.o: src/CMakeFiles/hvac_client.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/scratch/s5104a21/hvactest/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_4) "Building CXX object src/CMakeFiles/hvac_client.dir/hvac_data_mover.cpp.o"
	cd /scratch/s5104a21/hvactest/build/src && /apps/compiler/gcc/10.2.0/bin/g++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT src/CMakeFiles/hvac_client.dir/hvac_data_mover.cpp.o -MF CMakeFiles/hvac_client.dir/hvac_data_mover.cpp.o.d -o CMakeFiles/hvac_client.dir/hvac_data_mover.cpp.o -c /scratch/s5104a21/hvactest/src/hvac_data_mover.cpp

src/CMakeFiles/hvac_client.dir/hvac_data_mover.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/hvac_client.dir/hvac_data_mover.cpp.i"
	cd /scratch/s5104a21/hvactest/build/src && /apps/compiler/gcc/10.2.0/bin/g++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /scratch/s5104a21/hvactest/src/hvac_data_mover.cpp > CMakeFiles/hvac_client.dir/hvac_data_mover.cpp.i

src/CMakeFiles/hvac_client.dir/hvac_data_mover.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/hvac_client.dir/hvac_data_mover.cpp.s"
	cd /scratch/s5104a21/hvactest/build/src && /apps/compiler/gcc/10.2.0/bin/g++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /scratch/s5104a21/hvactest/src/hvac_data_mover.cpp -o CMakeFiles/hvac_client.dir/hvac_data_mover.cpp.s

src/CMakeFiles/hvac_client.dir/hvac_logging.c.o: src/CMakeFiles/hvac_client.dir/flags.make
src/CMakeFiles/hvac_client.dir/hvac_logging.c.o: /scratch/s5104a21/hvactest/src/hvac_logging.c
src/CMakeFiles/hvac_client.dir/hvac_logging.c.o: src/CMakeFiles/hvac_client.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/scratch/s5104a21/hvactest/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_5) "Building C object src/CMakeFiles/hvac_client.dir/hvac_logging.c.o"
	cd /scratch/s5104a21/hvactest/build/src && /apps/compiler/gcc/10.2.0/bin/gcc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -MD -MT src/CMakeFiles/hvac_client.dir/hvac_logging.c.o -MF CMakeFiles/hvac_client.dir/hvac_logging.c.o.d -o CMakeFiles/hvac_client.dir/hvac_logging.c.o -c /scratch/s5104a21/hvactest/src/hvac_logging.c

src/CMakeFiles/hvac_client.dir/hvac_logging.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/hvac_client.dir/hvac_logging.c.i"
	cd /scratch/s5104a21/hvactest/build/src && /apps/compiler/gcc/10.2.0/bin/gcc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /scratch/s5104a21/hvactest/src/hvac_logging.c > CMakeFiles/hvac_client.dir/hvac_logging.c.i

src/CMakeFiles/hvac_client.dir/hvac_logging.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/hvac_client.dir/hvac_logging.c.s"
	cd /scratch/s5104a21/hvactest/build/src && /apps/compiler/gcc/10.2.0/bin/gcc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /scratch/s5104a21/hvactest/src/hvac_logging.c -o CMakeFiles/hvac_client.dir/hvac_logging.c.s

src/CMakeFiles/hvac_client.dir/hvac_comm.cpp.o: src/CMakeFiles/hvac_client.dir/flags.make
src/CMakeFiles/hvac_client.dir/hvac_comm.cpp.o: /scratch/s5104a21/hvactest/src/hvac_comm.cpp
src/CMakeFiles/hvac_client.dir/hvac_comm.cpp.o: src/CMakeFiles/hvac_client.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/scratch/s5104a21/hvactest/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_6) "Building CXX object src/CMakeFiles/hvac_client.dir/hvac_comm.cpp.o"
	cd /scratch/s5104a21/hvactest/build/src && /apps/compiler/gcc/10.2.0/bin/g++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT src/CMakeFiles/hvac_client.dir/hvac_comm.cpp.o -MF CMakeFiles/hvac_client.dir/hvac_comm.cpp.o.d -o CMakeFiles/hvac_client.dir/hvac_comm.cpp.o -c /scratch/s5104a21/hvactest/src/hvac_comm.cpp

src/CMakeFiles/hvac_client.dir/hvac_comm.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/hvac_client.dir/hvac_comm.cpp.i"
	cd /scratch/s5104a21/hvactest/build/src && /apps/compiler/gcc/10.2.0/bin/g++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /scratch/s5104a21/hvactest/src/hvac_comm.cpp > CMakeFiles/hvac_client.dir/hvac_comm.cpp.i

src/CMakeFiles/hvac_client.dir/hvac_comm.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/hvac_client.dir/hvac_comm.cpp.s"
	cd /scratch/s5104a21/hvactest/build/src && /apps/compiler/gcc/10.2.0/bin/g++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /scratch/s5104a21/hvactest/src/hvac_comm.cpp -o CMakeFiles/hvac_client.dir/hvac_comm.cpp.s

src/CMakeFiles/hvac_client.dir/hvac_comm_client.cpp.o: src/CMakeFiles/hvac_client.dir/flags.make
src/CMakeFiles/hvac_client.dir/hvac_comm_client.cpp.o: /scratch/s5104a21/hvactest/src/hvac_comm_client.cpp
src/CMakeFiles/hvac_client.dir/hvac_comm_client.cpp.o: src/CMakeFiles/hvac_client.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/scratch/s5104a21/hvactest/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_7) "Building CXX object src/CMakeFiles/hvac_client.dir/hvac_comm_client.cpp.o"
	cd /scratch/s5104a21/hvactest/build/src && /apps/compiler/gcc/10.2.0/bin/g++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT src/CMakeFiles/hvac_client.dir/hvac_comm_client.cpp.o -MF CMakeFiles/hvac_client.dir/hvac_comm_client.cpp.o.d -o CMakeFiles/hvac_client.dir/hvac_comm_client.cpp.o -c /scratch/s5104a21/hvactest/src/hvac_comm_client.cpp

src/CMakeFiles/hvac_client.dir/hvac_comm_client.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/hvac_client.dir/hvac_comm_client.cpp.i"
	cd /scratch/s5104a21/hvactest/build/src && /apps/compiler/gcc/10.2.0/bin/g++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /scratch/s5104a21/hvactest/src/hvac_comm_client.cpp > CMakeFiles/hvac_client.dir/hvac_comm_client.cpp.i

src/CMakeFiles/hvac_client.dir/hvac_comm_client.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/hvac_client.dir/hvac_comm_client.cpp.s"
	cd /scratch/s5104a21/hvactest/build/src && /apps/compiler/gcc/10.2.0/bin/g++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /scratch/s5104a21/hvactest/src/hvac_comm_client.cpp -o CMakeFiles/hvac_client.dir/hvac_comm_client.cpp.s

src/CMakeFiles/hvac_client.dir/hvac_fault_client.cpp.o: src/CMakeFiles/hvac_client.dir/flags.make
src/CMakeFiles/hvac_client.dir/hvac_fault_client.cpp.o: /scratch/s5104a21/hvactest/src/hvac_fault_client.cpp
src/CMakeFiles/hvac_client.dir/hvac_fault_client.cpp.o: src/CMakeFiles/hvac_client.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/scratch/s5104a21/hvactest/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_8) "Building CXX object src/CMakeFiles/hvac_client.dir/hvac_fault_client.cpp.o"
	cd /scratch/s5104a21/hvactest/build/src && /apps/compiler/gcc/10.2.0/bin/g++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT src/CMakeFiles/hvac_client.dir/hvac_fault_client.cpp.o -MF CMakeFiles/hvac_client.dir/hvac_fault_client.cpp.o.d -o CMakeFiles/hvac_client.dir/hvac_fault_client.cpp.o -c /scratch/s5104a21/hvactest/src/hvac_fault_client.cpp

src/CMakeFiles/hvac_client.dir/hvac_fault_client.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/hvac_client.dir/hvac_fault_client.cpp.i"
	cd /scratch/s5104a21/hvactest/build/src && /apps/compiler/gcc/10.2.0/bin/g++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /scratch/s5104a21/hvactest/src/hvac_fault_client.cpp > CMakeFiles/hvac_client.dir/hvac_fault_client.cpp.i

src/CMakeFiles/hvac_client.dir/hvac_fault_client.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/hvac_client.dir/hvac_fault_client.cpp.s"
	cd /scratch/s5104a21/hvactest/build/src && /apps/compiler/gcc/10.2.0/bin/g++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /scratch/s5104a21/hvactest/src/hvac_fault_client.cpp -o CMakeFiles/hvac_client.dir/hvac_fault_client.cpp.s

# Object files for target hvac_client
hvac_client_OBJECTS = \
"CMakeFiles/hvac_client.dir/hvac.cpp.o" \
"CMakeFiles/hvac_client.dir/hvac_client.cpp.o" \
"CMakeFiles/hvac_client.dir/wrappers.c.o" \
"CMakeFiles/hvac_client.dir/hvac_data_mover.cpp.o" \
"CMakeFiles/hvac_client.dir/hvac_logging.c.o" \
"CMakeFiles/hvac_client.dir/hvac_comm.cpp.o" \
"CMakeFiles/hvac_client.dir/hvac_comm_client.cpp.o" \
"CMakeFiles/hvac_client.dir/hvac_fault_client.cpp.o"

# External object files for target hvac_client
hvac_client_EXTERNAL_OBJECTS =

src/libhvac_client.so: src/CMakeFiles/hvac_client.dir/hvac.cpp.o
src/libhvac_client.so: src/CMakeFiles/hvac_client.dir/hvac_client.cpp.o
src/libhvac_client.so: src/CMakeFiles/hvac_client.dir/wrappers.c.o
src/libhvac_client.so: src/CMakeFiles/hvac_client.dir/hvac_data_mover.cpp.o
src/libhvac_client.so: src/CMakeFiles/hvac_client.dir/hvac_logging.c.o
src/libhvac_client.so: src/CMakeFiles/hvac_client.dir/hvac_comm.cpp.o
src/libhvac_client.so: src/CMakeFiles/hvac_client.dir/hvac_comm_client.cpp.o
src/libhvac_client.so: src/CMakeFiles/hvac_client.dir/hvac_fault_client.cpp.o
src/libhvac_client.so: src/CMakeFiles/hvac_client.dir/build.make
src/libhvac_client.so: src/CMakeFiles/hvac_client.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/scratch/s5104a21/hvactest/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_9) "Linking CXX shared library libhvac_client.so"
	cd /scratch/s5104a21/hvactest/build/src && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/hvac_client.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
src/CMakeFiles/hvac_client.dir/build: src/libhvac_client.so
.PHONY : src/CMakeFiles/hvac_client.dir/build

src/CMakeFiles/hvac_client.dir/clean:
	cd /scratch/s5104a21/hvactest/build/src && $(CMAKE_COMMAND) -P CMakeFiles/hvac_client.dir/cmake_clean.cmake
.PHONY : src/CMakeFiles/hvac_client.dir/clean

src/CMakeFiles/hvac_client.dir/depend:
	cd /scratch/s5104a21/hvactest/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /scratch/s5104a21/hvactest /scratch/s5104a21/hvactest/src /scratch/s5104a21/hvactest/build /scratch/s5104a21/hvactest/build/src /scratch/s5104a21/hvactest/build/src/CMakeFiles/hvac_client.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : src/CMakeFiles/hvac_client.dir/depend

