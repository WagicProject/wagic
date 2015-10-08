#build unzip

SET(SOURCE
    ioapi.c
    miniunz.c
    unzip.c)

include_directories(${ZLIB_INCLUDE_DIRS} ${EXTRA_INCLUDES})

add_library(unzip ${SOURCE})
target_link_libraries(unzip ${ZLIB_LIBRARIES})

if(PSP OR UNIX)
	set_target_properties(unzip PROPERTIES COMPILE_FLAGS "-Dunix")
endif()
