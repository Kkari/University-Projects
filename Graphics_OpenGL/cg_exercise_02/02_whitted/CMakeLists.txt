#cmake_minimum_required(VERSION 2.8)
#project(cg)
#find_package(glfw)
#find_package(OpenGL)
#find_package(GLEW)
#
#set(CMAKE_CXX_FLAGS "-g -Wall -std=c++11")
#add_subdirectory(src)
#
cmake_minimum_required (VERSION 2.8) 
project (cg) 

set(EXERCISE_NUMBER 2)

if (NOT DEFINED CGLIB_DIR)
	set(CGLIB_DIR ${CMAKE_SOURCE_DIR}/../cglib)
endif()
get_filename_component(CGLIB_DIR ${CGLIB_DIR} ABSOLUTE)

include(${CGLIB_DIR}/CMakeListsExercises.txt)

