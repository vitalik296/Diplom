# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.10

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
CMAKE_COMMAND = /usr/bin/cmake

# The command to remove a file.
RM = /usr/bin/cmake -E remove -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/vitaliy/coursework/diplom/client/fuse

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/vitaliy/coursework/diplom/client/fuse

# Include any dependencies generated for this target.
include CMakeFiles/fuse.dir/depend.make

# Include the progress variables for this target.
include CMakeFiles/fuse.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/fuse.dir/flags.make

CMakeFiles/fuse.dir/fuse.c.o: CMakeFiles/fuse.dir/flags.make
CMakeFiles/fuse.dir/fuse.c.o: fuse.c
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/vitaliy/coursework/diplom/client/fuse/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building C object CMakeFiles/fuse.dir/fuse.c.o"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -o CMakeFiles/fuse.dir/fuse.c.o   -c /home/vitaliy/coursework/diplom/client/fuse/fuse.c

CMakeFiles/fuse.dir/fuse.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/fuse.dir/fuse.c.i"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /home/vitaliy/coursework/diplom/client/fuse/fuse.c > CMakeFiles/fuse.dir/fuse.c.i

CMakeFiles/fuse.dir/fuse.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/fuse.dir/fuse.c.s"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /home/vitaliy/coursework/diplom/client/fuse/fuse.c -o CMakeFiles/fuse.dir/fuse.c.s

CMakeFiles/fuse.dir/fuse.c.o.requires:

.PHONY : CMakeFiles/fuse.dir/fuse.c.o.requires

CMakeFiles/fuse.dir/fuse.c.o.provides: CMakeFiles/fuse.dir/fuse.c.o.requires
	$(MAKE) -f CMakeFiles/fuse.dir/build.make CMakeFiles/fuse.dir/fuse.c.o.provides.build
.PHONY : CMakeFiles/fuse.dir/fuse.c.o.provides

CMakeFiles/fuse.dir/fuse.c.o.provides.build: CMakeFiles/fuse.dir/fuse.c.o


# Object files for target fuse
fuse_OBJECTS = \
"CMakeFiles/fuse.dir/fuse.c.o"

# External object files for target fuse
fuse_EXTERNAL_OBJECTS =

bin/fuse: CMakeFiles/fuse.dir/fuse.c.o
bin/fuse: CMakeFiles/fuse.dir/build.make
bin/fuse: /usr/lib/x86_64-linux-gnu/libfuse.so
bin/fuse: CMakeFiles/fuse.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/vitaliy/coursework/diplom/client/fuse/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking C executable bin/fuse"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/fuse.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/fuse.dir/build: bin/fuse

.PHONY : CMakeFiles/fuse.dir/build

CMakeFiles/fuse.dir/requires: CMakeFiles/fuse.dir/fuse.c.o.requires

.PHONY : CMakeFiles/fuse.dir/requires

CMakeFiles/fuse.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/fuse.dir/cmake_clean.cmake
.PHONY : CMakeFiles/fuse.dir/clean

CMakeFiles/fuse.dir/depend:
	cd /home/vitaliy/coursework/diplom/client/fuse && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/vitaliy/coursework/diplom/client/fuse /home/vitaliy/coursework/diplom/client/fuse /home/vitaliy/coursework/diplom/client/fuse /home/vitaliy/coursework/diplom/client/fuse /home/vitaliy/coursework/diplom/client/fuse/CMakeFiles/fuse.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/fuse.dir/depend

