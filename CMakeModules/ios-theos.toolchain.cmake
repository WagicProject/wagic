include(CMakeForceCompiler)

# this one is important
SET(CMAKE_SYSTEM_NAME "Generic")
#this one not so much
SET(CMAKE_SYSTEM_VERSION 1)

set (CMAKE_SYSTEM_NAME Generic)

# Just point to anything that exists. 
# We don't need CMake to generate proper build files.
set(CMAKE_C_COMPILER ${CMAKE_CURRENT_SOURCE_DIR})
set(CMAKE_CXX_COMPILER ${CMAKE_CURRENT_SOURCE_DIR})

# Skip the platform compiler checks
set (CMAKE_CXX_COMPILER_WORKS TRUE)
set (CMAKE_C_COMPILER_WORKS TRUE)

#how libraries look
SET(CMAKE_FIND_LIBRARY_PREFIXES "lib")
SET(CMAKE_FIND_LIBRARY_SUFFIXES ".so" ".a")

SET(CMAKE_SYSTEM_INCLUDE_PATH 
	${THEOS_PATH}/include 
	${CMAKE_INSTALL_PREFIX}/include
	${CMAKE_SYSTEM_INCLUDE_PATH})

# where is the target environment 
SET(CMAKE_SYSTEM_LIBRARY_PATH 
	${THEOS_PATH}/lib 
	${CMAKE_INSTALL_PREFIX}/lib
	${CMAKE_SYSTEM_LIBRARY_PATH})


SET(CMAKE_FIND_ROOT_PATH  
	${CMAKE_SOURCE_DIR}
	${THEOS_PATH}
	${THEOS_PATH}/lib
	${THEOS_PATH}/include
	)

# search for programs in the build host directories
# for libraries and headers in the target directories and then in the host
SET(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
SET(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY FIRST)
SET(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE FIRST)

set(IOS 1)
