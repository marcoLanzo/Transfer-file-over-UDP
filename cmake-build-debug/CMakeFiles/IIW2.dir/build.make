# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.12

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:


#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:


# Remove some rules from gmake that .SUFFIXES does not remove.
SUFFIXES =

.SUFFIXES: .hpux_make_needs_suffix_list


# Suppress display of executed commands.
$(VERBOSE).SILENT:


# A target that is always out of date.
cmake_force:

.PHONY : cmake_force

#=============================================================================
# Set environment variables for the build.

# The shell in which to execute make rules.
SHELL = /bin/sh

# The CMake executable.
CMAKE_COMMAND = /opt/clion-2018.2.4/bin/cmake/linux/bin/cmake

# The command to remove a file.
RM = /opt/clion-2018.2.4/bin/cmake/linux/bin/cmake -E remove -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/lombi/Scaricati/IIW2_final

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/lombi/Scaricati/IIW2_final/cmake-build-debug

# Include any dependencies generated for this target.
include CMakeFiles/IIW2.dir/depend.make

# Include the progress variables for this target.
include CMakeFiles/IIW2.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/IIW2.dir/flags.make

CMakeFiles/IIW2.dir/Server/Server.c.o: CMakeFiles/IIW2.dir/flags.make
CMakeFiles/IIW2.dir/Server/Server.c.o: ../Server/Server.c
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/lombi/Scaricati/IIW2_final/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building C object CMakeFiles/IIW2.dir/Server/Server.c.o"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -o CMakeFiles/IIW2.dir/Server/Server.c.o   -c /home/lombi/Scaricati/IIW2_final/Server/Server.c

CMakeFiles/IIW2.dir/Server/Server.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/IIW2.dir/Server/Server.c.i"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /home/lombi/Scaricati/IIW2_final/Server/Server.c > CMakeFiles/IIW2.dir/Server/Server.c.i

CMakeFiles/IIW2.dir/Server/Server.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/IIW2.dir/Server/Server.c.s"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /home/lombi/Scaricati/IIW2_final/Server/Server.c -o CMakeFiles/IIW2.dir/Server/Server.c.s

# Object files for target IIW2
IIW2_OBJECTS = \
"CMakeFiles/IIW2.dir/Server/Server.c.o"

# External object files for target IIW2
IIW2_EXTERNAL_OBJECTS =

IIW2: CMakeFiles/IIW2.dir/Server/Server.c.o
IIW2: CMakeFiles/IIW2.dir/build.make
IIW2: CMakeFiles/IIW2.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/lombi/Scaricati/IIW2_final/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking C executable IIW2"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/IIW2.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/IIW2.dir/build: IIW2

.PHONY : CMakeFiles/IIW2.dir/build

CMakeFiles/IIW2.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/IIW2.dir/cmake_clean.cmake
.PHONY : CMakeFiles/IIW2.dir/clean

CMakeFiles/IIW2.dir/depend:
	cd /home/lombi/Scaricati/IIW2_final/cmake-build-debug && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/lombi/Scaricati/IIW2_final /home/lombi/Scaricati/IIW2_final /home/lombi/Scaricati/IIW2_final/cmake-build-debug /home/lombi/Scaricati/IIW2_final/cmake-build-debug /home/lombi/Scaricati/IIW2_final/cmake-build-debug/CMakeFiles/IIW2.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/IIW2.dir/depend

