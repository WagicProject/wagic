add_definitions(-DPSP -G0)
add_definitions(-D_PSP_FW_VERSION=371)
add_definitions(-DDEVHOOK -DPSPFW3XX) 

include_directories(${PSPSDK_PATH}/include)
