# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.30

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
CMAKE_COMMAND = /usr/local/bin/cmake

# The command to remove a file.
RM = /usr/local/bin/cmake -E rm -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/isacharles/unix-shell

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/isacharles/unix-shell/build

# Include any dependencies generated for this target.
include CMakeFiles/sleeptest.dir/depend.make
# Include any dependencies generated by the compiler for this target.
include CMakeFiles/sleeptest.dir/compiler_depend.make

# Include the progress variables for this target.
include CMakeFiles/sleeptest.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/sleeptest.dir/flags.make

CMakeFiles/sleeptest.dir/sleeptest.c.o: CMakeFiles/sleeptest.dir/flags.make
CMakeFiles/sleeptest.dir/sleeptest.c.o: sleeptest.c
CMakeFiles/sleeptest.dir/sleeptest.c.o: CMakeFiles/sleeptest.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green --progress-dir=/home/isacharles/unix-shell/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building C object CMakeFiles/sleeptest.dir/sleeptest.c.o"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -MD -MT CMakeFiles/sleeptest.dir/sleeptest.c.o -MF CMakeFiles/sleeptest.dir/sleeptest.c.o.d -o CMakeFiles/sleeptest.dir/sleeptest.c.o -c /home/isacharles/unix-shell/build/sleeptest.c

CMakeFiles/sleeptest.dir/sleeptest.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Preprocessing C source to CMakeFiles/sleeptest.dir/sleeptest.c.i"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /home/isacharles/unix-shell/build/sleeptest.c > CMakeFiles/sleeptest.dir/sleeptest.c.i

CMakeFiles/sleeptest.dir/sleeptest.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Compiling C source to assembly CMakeFiles/sleeptest.dir/sleeptest.c.s"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /home/isacharles/unix-shell/build/sleeptest.c -o CMakeFiles/sleeptest.dir/sleeptest.c.s

# Object files for target sleeptest
sleeptest_OBJECTS = \
"CMakeFiles/sleeptest.dir/sleeptest.c.o"

# External object files for target sleeptest
sleeptest_EXTERNAL_OBJECTS =

sleeptest: CMakeFiles/sleeptest.dir/sleeptest.c.o
sleeptest: CMakeFiles/sleeptest.dir/build.make
sleeptest: CMakeFiles/sleeptest.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green --bold --progress-dir=/home/isacharles/unix-shell/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking C executable sleeptest"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/sleeptest.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/sleeptest.dir/build: sleeptest
.PHONY : CMakeFiles/sleeptest.dir/build

CMakeFiles/sleeptest.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/sleeptest.dir/cmake_clean.cmake
.PHONY : CMakeFiles/sleeptest.dir/clean

CMakeFiles/sleeptest.dir/depend:
	cd /home/isacharles/unix-shell/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/isacharles/unix-shell /home/isacharles/unix-shell /home/isacharles/unix-shell/build /home/isacharles/unix-shell/build /home/isacharles/unix-shell/build/CMakeFiles/sleeptest.dir/DependInfo.cmake "--color=$(COLOR)"
.PHONY : CMakeFiles/sleeptest.dir/depend

