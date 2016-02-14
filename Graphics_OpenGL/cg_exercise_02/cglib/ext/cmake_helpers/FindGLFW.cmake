# Locate the glfw library
# This module defines the following variables:
# GLFW_LIBRARY, the name of the library;
# GLFW_INCLUDE_DIR, where to find glfw include files.
# GLFW_FOUND, true if both the GLFW_LIBRARY and GLFW_INCLUDE_DIR have been found.
#
# To help locate the library and include file, you could define an environment variable called
# GLFW_ROOT which points to the root of the glfw library installation. This is pretty useful
# on a Windows platform.
#
#
# Usage example to compile an "executable" target to the glfw library:
#
# FIND_PACKAGE (glfw REQUIRED)
# INCLUDE_DIRECTORIES (${GLFW_INCLUDE_DIR})
# ADD_EXECUTABLE (executable ${EXECUTABLE_SRCS})
# TARGET_LINK_LIBRARIES (executable ${GLFW_LIBRARY})
#
# TODO:
# Allow the user to select to link to a shared library or to a static library.

#Search for the include file...
FIND_PATH(GLFW_INCLUDE_DIR GLFW/glfw3.h DOC "Path to GLFW include directory."
  HINTS
  $ENV{GLFW_ROOT}
  ${GLFW_ROOT}
  PATH_SUFFIX include #For finding the include file under the root of the glfw expanded archive, typically on Windows.
  PATHS
  /usr/include/
  /usr/local/include/
  # By default headers are under GL subfolder
  /usr/include/GL
  /usr/local/include/GL
  ${GLFW_ROOT}/include/ # added by ptr
 
)

if (WIN32)
  if (CMAKE_CL_64)
    SET(GLFW_PATH_SUFFIXES lib/win64)
  else (CMAKE_CL_64)
    SET(GLFW_PATH_SUFFIXES lib/win32)
  endif (CMAKE_CL_64)
elseif (UNIX AND NOT APPLE)
  set (GLFW_PATH_SUFFIXES lib/x11)
elseif (UNIX AND APPLE)
  if (CMAKE_SIZEOF_VOID_P MATCHES "8")
    set (GLFW_PATH_SUFFIXES lib/cocoa)
  else ()
    set (GLFW_PATH_SUFFIXES lib/carbon)
  endif ()
endif ()

FIND_LIBRARY(GLFW_LIBRARY DOC "Absolute path to GLFW library."
  NAMES glfw glfw3 GLFW3.lib GLFW.lib
  HINTS $ENV{GLFW_ROOT} ${GLFW_ROOT} ${GLFW_LIBRARY_DIR}
  PATH_SUFFIXES ${GLFW_PATH_SUFFIXES} /lib-msvc100/release /src #/Release /RelWithDebInfo /MinSizeRel /Debug
  PATHS
  /usr/local/lib
  /usr/lib
  ${GLFW_LIBRARY_DIR}
)

# FIXME: does Windows also use GLFW3.dll?
FIND_LIBRARY(GLFW_LIBRARY_SHARED DOC "Absolute path to GLFW shared library."
  NAMES glfw glfw3 GLFW.dll GLFW3.dll
  HINTS $ENV{GLFW_ROOT}
  PATH_SUFFIXES ${GLFW_PATH_SUFFIXES} /lib-msvc100/release
  PATHS
  /usr/local/lib
  /usr/lib
  ${GLFW_LIBRARY_DIR}
)

SET(GLFW_FOUND 0)
IF(GLFW_LIBRARY AND GLFW_INCLUDE_DIR)
  SET(GLFW_FOUND 1)
  message(STATUS "GLFW found in " ${GLFW_LIBRARY} )
ENDIF(GLFW_LIBRARY AND GLFW_INCLUDE_DIR)
