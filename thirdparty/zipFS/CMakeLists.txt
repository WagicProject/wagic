set(SOURCES
    zfsystem.cpp
    zstream.cpp
    ziphdr.cpp)

set(ZIPFS_SRC ${SOURCES} PARENT_SCOPE)

#FindOrBuildZLIB()
include_directories(${CMAKE_SOURCE_DIR}/JGE/include ${EXTRA_INCLUDES} ${ZLIB_INCLUDE_DIRS})

add_library(zipFS ${SOURCES})
#target_link_libraries(zipFS ${ZLIB_LIBRARIES})
