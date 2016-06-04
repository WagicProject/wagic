set(SOURCES         
    #tinystr.cpp as we use -DTIXML_USE_STL
    tinyxmlerror.cpp
    tinyxmlparser.cpp
    tinyxml.cpp)

set(TINYXML_SRC ${SOURCES} PARENT_SCOPE)

add_definitions(-DTIXML_USE_STL)

add_library(tinyxml ${SOURCES})
