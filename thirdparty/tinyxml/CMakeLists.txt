set(SOURCES         
    #tinystr.cpp as we use -DTIXML_USE_STL
    tinyxmlerror.cpp
    tinyxmlparser.cpp
    tinyxml.cpp)
    
add_definitions(-DTIXML_USE_STL)

add_library(tinyxml ${SOURCES})