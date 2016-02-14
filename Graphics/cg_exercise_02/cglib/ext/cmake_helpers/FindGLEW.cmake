#
# Try to find GLEW library and include path.
# Once done this will define
#
# GLEW_FOUND
# GLEW_INCLUDE_PATH
# GLEW_LIBRARY
# 

IF(WIN32)
  FIND_PATH( GLEW_INCLUDE_PATH GL/glew.h
	$ENV{PROGRAMFILES}/GLEW/include
	${GLEW_ROOT_DIR}/include
	DOC "The directory where GL/glew.h resides")

  if( CMAKE_SIZEOF_VOID_P EQUAL 8 )
	set(SUFFIX "x64")
  else( CMAKE_SIZEOF_VOID_P EQUAL 8 )
	set(SUFFIX "w32")
  endif( CMAKE_SIZEOF_VOID_P EQUAL 8 )

  FIND_LIBRARY( GLEW_LIBRARY
    NAMES glew GLEW glew32 glew32s
	PATH_SUFFIXES /lib /lib/${SUFFIX} /src/nvgl/glew/lib
    PATHS
    $ENV{PROGRAMFILES}/GLEW/
    ${PROJECT_SOURCE_DIR}
	${GLEW_ROOT_DIR}
    DOC "The GLEW library")


  if(CMAKE_BUILD_TYPE MATCHES DEBUG) 
	set(GLFW_LIBRARY_SHARED_NAME glew32d.dll)
  else (CMAKE_BUILD_TYPE MATCHES DEBUG) 
	set(GLFW_LIBRARY_SHARED_NAME glew32.dll)
  endif(CMAKE_BUILD_TYPE MATCHES DEBUG)

  FIND_FILE( GLEW_LIBRARY_SHARED
    NAMES ${GLFW_LIBRARY_SHARED_NAME}
	PATH_SUFFIXES /bin /bin/${SUFFIX} /src/nvgl/glew/bin
    PATHS
    ${PROJECT_SOURCE_DIR}
	${GLEW_ROOT_DIR}
	${GLFW_INCLUDE_PATH}/..
    $ENV{PROGRAMFILES}/GLEW/
    DOC "The GLEW shared library")
ELSE(WIN32)
  FIND_PATH( GLEW_INCLUDE_PATH GL/glew.h
	/usr/include
	/usr/local/include
	/sw/include
    /opt/local/include
	${GLEW_ROOT_DIR}/include
	DOC "The directory where GL/glew.h resides")

  # Prefer the static library.
  FIND_LIBRARY( GLEW_LIBRARY
	NAMES libGLEW.a GLEW
	PATHS
	/usr/lib64
	/usr/lib
	/usr/local/lib64
	/usr/local/lib
	/sw/lib
	/opt/local/lib
	${GLEW_ROOT_DIR}/lib
	DOC "The GLEW library")
ENDIF(WIN32)

SET(GLEW_FOUND "NO")
IF(GLEW_INCLUDE_PATH AND GLEW_LIBRARY)
  SET(GLEW_LIBRARIES ${GLEW_LIBRARY})
  SET(GLEW_FOUND "YES")
  message(STATUS "Found GLEW" " in " ${GLEW_LIBRARY})
ENDIF(GLEW_INCLUDE_PATH AND GLEW_LIBRARY)
