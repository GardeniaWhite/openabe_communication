# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.5

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
CMAKE_SOURCE_DIR = /home/gardenia/openabe/deps/relic/relic-toolkit-0.5.0

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/gardenia/openabe/deps/relic/tmpbp-mCDa

# Include any dependencies generated for this target.
include test/CMakeFiles/test_epx.dir/depend.make

# Include the progress variables for this target.
include test/CMakeFiles/test_epx.dir/progress.make

# Include the compile flags for this target's objects.
include test/CMakeFiles/test_epx.dir/flags.make

test/CMakeFiles/test_epx.dir/test_epx.c.o: test/CMakeFiles/test_epx.dir/flags.make
test/CMakeFiles/test_epx.dir/test_epx.c.o: /home/gardenia/openabe/deps/relic/relic-toolkit-0.5.0/test/test_epx.c
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/gardenia/openabe/deps/relic/tmpbp-mCDa/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building C object test/CMakeFiles/test_epx.dir/test_epx.c.o"
	cd /home/gardenia/openabe/deps/relic/tmpbp-mCDa/test && /usr/bin/cc  $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -o CMakeFiles/test_epx.dir/test_epx.c.o   -c /home/gardenia/openabe/deps/relic/relic-toolkit-0.5.0/test/test_epx.c

test/CMakeFiles/test_epx.dir/test_epx.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/test_epx.dir/test_epx.c.i"
	cd /home/gardenia/openabe/deps/relic/tmpbp-mCDa/test && /usr/bin/cc  $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /home/gardenia/openabe/deps/relic/relic-toolkit-0.5.0/test/test_epx.c > CMakeFiles/test_epx.dir/test_epx.c.i

test/CMakeFiles/test_epx.dir/test_epx.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/test_epx.dir/test_epx.c.s"
	cd /home/gardenia/openabe/deps/relic/tmpbp-mCDa/test && /usr/bin/cc  $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /home/gardenia/openabe/deps/relic/relic-toolkit-0.5.0/test/test_epx.c -o CMakeFiles/test_epx.dir/test_epx.c.s

test/CMakeFiles/test_epx.dir/test_epx.c.o.requires:

.PHONY : test/CMakeFiles/test_epx.dir/test_epx.c.o.requires

test/CMakeFiles/test_epx.dir/test_epx.c.o.provides: test/CMakeFiles/test_epx.dir/test_epx.c.o.requires
	$(MAKE) -f test/CMakeFiles/test_epx.dir/build.make test/CMakeFiles/test_epx.dir/test_epx.c.o.provides.build
.PHONY : test/CMakeFiles/test_epx.dir/test_epx.c.o.provides

test/CMakeFiles/test_epx.dir/test_epx.c.o.provides.build: test/CMakeFiles/test_epx.dir/test_epx.c.o


# Object files for target test_epx
test_epx_OBJECTS = \
"CMakeFiles/test_epx.dir/test_epx.c.o"

# External object files for target test_epx
test_epx_EXTERNAL_OBJECTS =

bin/test_epx: test/CMakeFiles/test_epx.dir/test_epx.c.o
bin/test_epx: test/CMakeFiles/test_epx.dir/build.make
bin/test_epx: lib/librelic_s.a
bin/test_epx: /usr/lib/x86_64-linux-gnu/libgmp.so
bin/test_epx: test/CMakeFiles/test_epx.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/gardenia/openabe/deps/relic/tmpbp-mCDa/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking C executable ../bin/test_epx"
	cd /home/gardenia/openabe/deps/relic/tmpbp-mCDa/test && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/test_epx.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
test/CMakeFiles/test_epx.dir/build: bin/test_epx

.PHONY : test/CMakeFiles/test_epx.dir/build

test/CMakeFiles/test_epx.dir/requires: test/CMakeFiles/test_epx.dir/test_epx.c.o.requires

.PHONY : test/CMakeFiles/test_epx.dir/requires

test/CMakeFiles/test_epx.dir/clean:
	cd /home/gardenia/openabe/deps/relic/tmpbp-mCDa/test && $(CMAKE_COMMAND) -P CMakeFiles/test_epx.dir/cmake_clean.cmake
.PHONY : test/CMakeFiles/test_epx.dir/clean

test/CMakeFiles/test_epx.dir/depend:
	cd /home/gardenia/openabe/deps/relic/tmpbp-mCDa && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/gardenia/openabe/deps/relic/relic-toolkit-0.5.0 /home/gardenia/openabe/deps/relic/relic-toolkit-0.5.0/test /home/gardenia/openabe/deps/relic/tmpbp-mCDa /home/gardenia/openabe/deps/relic/tmpbp-mCDa/test /home/gardenia/openabe/deps/relic/tmpbp-mCDa/test/CMakeFiles/test_epx.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : test/CMakeFiles/test_epx.dir/depend
