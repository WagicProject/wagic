include(CMakeForceCompiler)

# this one is important
SET(CMAKE_SYSTEM_NAME "Generic")
#this one not so much
SET(CMAKE_SYSTEM_VERSION 1)

find_program(PSP_CONFIG_PROGRAM psp-config)

execute_process(COMMAND psp-config --pspsdk-path OUTPUT_VARIABLE PSPSDK_PATH OUTPUT_STRIP_TRAILING_WHITESPACE)
execute_process(COMMAND psp-config --psp-prefix  OUTPUT_VARIABLE PSPSDK_PREFIX OUTPUT_STRIP_TRAILING_WHITESPACE)
	
# specify compiler and linker:
find_program(PSP_GPP psp-g++)
find_program(PSP_GCC psp-gcc)

CMAKE_FORCE_C_COMPILER(${PSP_GCC} GNU)
CMAKE_FORCE_CXX_COMPILER(${PSP_GPP} GNU)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -G0")

set(CMAKE_SHARED_LIBRARY_LINK_CXX_FLAGS "")
set(CMAKE_CXX_LINK_EXECUTABLE "${PSP_GCC} <CMAKE_CXX_LINK_FLAGS> <LINK_FLAGS> <FLAGS> <OBJECTS>  -o <TARGET> <LINK_LIBRARIES>")

#how libraries look
SET(CMAKE_FIND_LIBRARY_PREFIXES "lib")
SET(CMAKE_FIND_LIBRARY_SUFFIXES ".so" ".a")

# where is the target environment 
SET(CMAKE_SYSTEM_INCLUDE_PATH 
	${CMAKE_SOURCE_DIR}/thirdparty/binary/psp/include 
	${PSPSDK_PATH}/include 
	${PSPSDK_PREFIX}/include 
	${CMAKE_INSTALL_PREFIX}/include
	${CMAKE_SYSTEM_INCLUDE_PATH})
	
SET(CMAKE_SYSTEM_LIBRARY_PATH 
	${CMAKE_SOURCE_DIR}/thirdparty/binary/psp/lib 
	${PSPSDK_PATH}/lib 
	${PSPSDK_PREFIX}/lib
	${CMAKE_INSTALL_PREFIX}/lib
	${CMAKE_SYSTEM_LIBRARY_PATH})

SET(CMAKE_FIND_ROOT_PATH  
	${CMAKE_SOURCE_DIR}/thirdparty/binary/psp
	${CMAKE_SOURCE_DIR}/thirdparty/binary/psp/lib
	${CMAKE_SOURCE_DIR}/thirdparty/binary/psp/include
	${PSPSDK_PATH}
	${PSPSDK_PATH}/lib
	${PSPSDK_PATH}/include
	${PSPSDK_PREFIX}
	${PSPSDK_PREFIX}/lib
	${PSPSDK_PREFIX}/include
	)

# search for programs in the build host directories
# for libraries and headers in the target directories and then in the host
SET(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
SET(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY FIRST)
SET(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE FIRST)

set(PSP 1)
