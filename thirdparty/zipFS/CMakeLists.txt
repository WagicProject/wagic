set(SOURCES
    stdafx.cpp
    zfs.cpp
    zfsystem.cpp
    zstream.cpp
    ziphdr.cpp)

FindOrBuildZLIB()
include_directories(${CMAKE_SOURCE_DIR}/JGE/include ${EXTRA_INCLUDES} ${ZLIB_INCLUDE_DIR})

add_library(zipFS ${SOURCES})
target_link_libraries(zipFS ${ZLIB_LIBRARY})