#build unzip

SET(SOURCE
    ioapi.c
    miniunz.c
    unzip.c)

include_directories(${ZLIB_INCLUDE_DIR} ${EXTRA_INCLUDES} ${ZLIB_INCLUDE_DIR})

add_library(unzip ${SOURCE})
target_link_libraries(unzip ${ZLIB_LIBRARY})

if(PSP OR UNIX)
	set_target_properties(unzip PROPERTIES COMPILE_FLAGS "-Dunix")
endif()
