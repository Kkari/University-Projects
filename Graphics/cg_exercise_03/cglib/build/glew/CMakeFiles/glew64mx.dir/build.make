# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 2.8

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
CMAKE_SOURCE_DIR = /home/kkari/UniStuff/Graphics/cg_exercise_03/03_textures

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/kkari/UniStuff/Graphics/cg_exercise_03/03_textures/build

# Include any dependencies generated for this target.
include /home/kkari/UniStuff/Graphics/cg_exercise_03/cglib/build/glew/CMakeFiles/glew64mx.dir/depend.make

# Include the progress variables for this target.
include /home/kkari/UniStuff/Graphics/cg_exercise_03/cglib/build/glew/CMakeFiles/glew64mx.dir/progress.make

# Include the compile flags for this target's objects.
include /home/kkari/UniStuff/Graphics/cg_exercise_03/cglib/build/glew/CMakeFiles/glew64mx.dir/flags.make

/home/kkari/UniStuff/Graphics/cg_exercise_03/cglib/build/glew/CMakeFiles/glew64mx.dir/src/glew.c.o: /home/kkari/UniStuff/Graphics/cg_exercise_03/cglib/build/glew/CMakeFiles/glew64mx.dir/flags.make
/home/kkari/UniStuff/Graphics/cg_exercise_03/cglib/build/glew/CMakeFiles/glew64mx.dir/src/glew.c.o: /home/kkari/UniStuff/Graphics/cg_exercise_03/cglib/lib/glew/src/glew.c
	$(CMAKE_COMMAND) -E cmake_progress_report /home/kkari/UniStuff/Graphics/cg_exercise_03/03_textures/build/CMakeFiles $(CMAKE_PROGRESS_1)
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Building C object /home/kkari/UniStuff/Graphics/cg_exercise_03/cglib/build/glew/CMakeFiles/glew64mx.dir/src/glew.c.o"
	cd /home/kkari/UniStuff/Graphics/cg_exercise_03/cglib/build/glew && /usr/bin/cc  $(C_DEFINES) $(C_FLAGS) -o CMakeFiles/glew64mx.dir/src/glew.c.o   -c /home/kkari/UniStuff/Graphics/cg_exercise_03/cglib/lib/glew/src/glew.c

/home/kkari/UniStuff/Graphics/cg_exercise_03/cglib/build/glew/CMakeFiles/glew64mx.dir/src/glew.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/glew64mx.dir/src/glew.c.i"
	cd /home/kkari/UniStuff/Graphics/cg_exercise_03/cglib/build/glew && /usr/bin/cc  $(C_DEFINES) $(C_FLAGS) -E /home/kkari/UniStuff/Graphics/cg_exercise_03/cglib/lib/glew/src/glew.c > CMakeFiles/glew64mx.dir/src/glew.c.i

/home/kkari/UniStuff/Graphics/cg_exercise_03/cglib/build/glew/CMakeFiles/glew64mx.dir/src/glew.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/glew64mx.dir/src/glew.c.s"
	cd /home/kkari/UniStuff/Graphics/cg_exercise_03/cglib/build/glew && /usr/bin/cc  $(C_DEFINES) $(C_FLAGS) -S /home/kkari/UniStuff/Graphics/cg_exercise_03/cglib/lib/glew/src/glew.c -o CMakeFiles/glew64mx.dir/src/glew.c.s

/home/kkari/UniStuff/Graphics/cg_exercise_03/cglib/build/glew/CMakeFiles/glew64mx.dir/src/glew.c.o.requires:
.PHONY : /home/kkari/UniStuff/Graphics/cg_exercise_03/cglib/build/glew/CMakeFiles/glew64mx.dir/src/glew.c.o.requires

/home/kkari/UniStuff/Graphics/cg_exercise_03/cglib/build/glew/CMakeFiles/glew64mx.dir/src/glew.c.o.provides: /home/kkari/UniStuff/Graphics/cg_exercise_03/cglib/build/glew/CMakeFiles/glew64mx.dir/src/glew.c.o.requires
	$(MAKE) -f /home/kkari/UniStuff/Graphics/cg_exercise_03/cglib/build/glew/CMakeFiles/glew64mx.dir/build.make /home/kkari/UniStuff/Graphics/cg_exercise_03/cglib/build/glew/CMakeFiles/glew64mx.dir/src/glew.c.o.provides.build
.PHONY : /home/kkari/UniStuff/Graphics/cg_exercise_03/cglib/build/glew/CMakeFiles/glew64mx.dir/src/glew.c.o.provides

/home/kkari/UniStuff/Graphics/cg_exercise_03/cglib/build/glew/CMakeFiles/glew64mx.dir/src/glew.c.o.provides.build: /home/kkari/UniStuff/Graphics/cg_exercise_03/cglib/build/glew/CMakeFiles/glew64mx.dir/src/glew.c.o

# Object files for target glew64mx
glew64mx_OBJECTS = \
"CMakeFiles/glew64mx.dir/src/glew.c.o"

# External object files for target glew64mx
glew64mx_EXTERNAL_OBJECTS =

/home/kkari/UniStuff/Graphics/cg_exercise_03/cglib/build/glew/libglew64mx.so: /home/kkari/UniStuff/Graphics/cg_exercise_03/cglib/build/glew/CMakeFiles/glew64mx.dir/src/glew.c.o
/home/kkari/UniStuff/Graphics/cg_exercise_03/cglib/build/glew/libglew64mx.so: /home/kkari/UniStuff/Graphics/cg_exercise_03/cglib/build/glew/CMakeFiles/glew64mx.dir/build.make
/home/kkari/UniStuff/Graphics/cg_exercise_03/cglib/build/glew/libglew64mx.so: /home/kkari/UniStuff/Graphics/cg_exercise_03/cglib/build/glew/CMakeFiles/glew64mx.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --red --bold "Linking C shared library libglew64mx.so"
	cd /home/kkari/UniStuff/Graphics/cg_exercise_03/cglib/build/glew && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/glew64mx.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
/home/kkari/UniStuff/Graphics/cg_exercise_03/cglib/build/glew/CMakeFiles/glew64mx.dir/build: /home/kkari/UniStuff/Graphics/cg_exercise_03/cglib/build/glew/libglew64mx.so
.PHONY : /home/kkari/UniStuff/Graphics/cg_exercise_03/cglib/build/glew/CMakeFiles/glew64mx.dir/build

/home/kkari/UniStuff/Graphics/cg_exercise_03/cglib/build/glew/CMakeFiles/glew64mx.dir/requires: /home/kkari/UniStuff/Graphics/cg_exercise_03/cglib/build/glew/CMakeFiles/glew64mx.dir/src/glew.c.o.requires
.PHONY : /home/kkari/UniStuff/Graphics/cg_exercise_03/cglib/build/glew/CMakeFiles/glew64mx.dir/requires

/home/kkari/UniStuff/Graphics/cg_exercise_03/cglib/build/glew/CMakeFiles/glew64mx.dir/clean:
	cd /home/kkari/UniStuff/Graphics/cg_exercise_03/cglib/build/glew && $(CMAKE_COMMAND) -P CMakeFiles/glew64mx.dir/cmake_clean.cmake
.PHONY : /home/kkari/UniStuff/Graphics/cg_exercise_03/cglib/build/glew/CMakeFiles/glew64mx.dir/clean

/home/kkari/UniStuff/Graphics/cg_exercise_03/cglib/build/glew/CMakeFiles/glew64mx.dir/depend:
	cd /home/kkari/UniStuff/Graphics/cg_exercise_03/03_textures/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/kkari/UniStuff/Graphics/cg_exercise_03/03_textures /home/kkari/UniStuff/Graphics/cg_exercise_03/cglib/lib/glew /home/kkari/UniStuff/Graphics/cg_exercise_03/03_textures/build /home/kkari/UniStuff/Graphics/cg_exercise_03/cglib/build/glew /home/kkari/UniStuff/Graphics/cg_exercise_03/cglib/build/glew/CMakeFiles/glew64mx.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : /home/kkari/UniStuff/Graphics/cg_exercise_03/cglib/build/glew/CMakeFiles/glew64mx.dir/depend

