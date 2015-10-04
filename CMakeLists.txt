cmake_minimum_required(VERSION 2.8.7)
cmake_policy(SET CMP0054 NEW)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_SOURCE_DIR}/CMakeModules)

#this block wil fix install prefixes to install everything in a subdirectory
#of cmake_binary_dir if we are on windows/android to make packaging more easy
if(NOT CMAKE_TOOLCHAIN_FILE)
  if(WIN32)
    set(CMAKE_INSTALL_PREFIX "${CMAKE_BINARY_DIR}/install" CACHE PATH "Install directory")
  else()
    set(CMAKE_INSTALL_PREFIX "/usr/local" CACHE PATH "Install directory")
  endif()
else() #we are cross-compiling (psp/android)
  #Android: set output folder for platform/android to pick up
  set(LIBRARY_OUTPUT_PATH_ROOT ${CMAKE_BINARY_DIR} CACHE PATH "library output root")
  set(CMAKE_INSTALL_PREFIX "${CMAKE_BINARY_DIR}/install" CACHE PATH "Install directory")
endif()

#set available build types (debug/release)
set(CMAKE_CONFIGURATION_TYPES "Debug;Release" CACHE STRING "Configs" FORCE)
if(DEFINED CMAKE_BUILD_TYPE)
  set_property( CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS ${CMAKE_CONFIGURATION_TYPES} )
endif()

set(EXECUTABLE_OUTPUT_PATH "${CMAKE_BINARY_DIR}/bin" CACHE PATH "Application output directory")


project(wagic CXX C)
#todo: somehow determine wagics version
set(WAGIC_VERSION "0.19")

#add standard paths to search for libraries. borrowed from opencv
if(UNIX AND NOT ANDROID)
  if(X86_64 OR CMAKE_SIZEOF_VOID_P EQUAL 8)
    if(EXISTS /lib64)
      list(APPEND CMAKE_LIBRARY_PATH /lib64)
    else()
      list(APPEND CMAKE_LIBRARY_PATH /lib)
    endif()
    if(EXISTS /usr/lib64)
      list(APPEND CMAKE_LIBRARY_PATH /usr/lib64)
    else()
      list(APPEND CMAKE_LIBRARY_PATH /usr/lib)
    endif()
  elseif(X86 OR CMAKE_SIZEOF_VOID_P EQUAL 4)
    if(EXISTS /lib32)
      list(APPEND CMAKE_LIBRARY_PATH /lib32)
    else()
      list(APPEND CMAKE_LIBRARY_PATH /lib)
    endif()
    if(EXISTS /usr/lib32)
      list(APPEND CMAKE_LIBRARY_PATH /usr/lib32)
    else()
      list(APPEND CMAKE_LIBRARY_PATH /usr/lib)
    endif()
  endif()
endif()

if(ANDROID OR PSP)
    #to allow finding of pathes/headers/libs within the source tree
    #even if only search for target platform libs
    set(CMAKE_FIND_ROOT_PATH ${CMAKE_SOURCE_DIR} ${CMAKE_FIND_ROOT_PATH})
endif()

#also borrowed from opencv
if(MINGW)
  if(EXISTS /mingw)
      list(APPEND CMAKE_INCLUDE_PATH /mingw)
  endif()
  if(EXISTS /mingw32)
      list(APPEND CMAKE_INCLUDE_PATH /mingw32)
  endif()
  if(EXISTS /mingw64)
      list(APPEND CMAKE_INCLUDE_PATH /mingw64)
  endif()
endif()

include(utils)

#select the target backend
if(PSP)
    WAGIC_OPTION(backend_psp         "build for psp"          ON)
endif()
if(WIN32 OR ANDROID OR UNIX)
    WAGIC_OPTION(backend_sdl         "build for sdl"          (WIN32 OR ANDROID))
endif()
if(UNIX AND NOT ANDROID)
    WAGIC_OPTION(backend_qt_console  "build qt-console version with testsuit" ON)
    WAGIC_OPTION(backend_qt_widget   "build qt-widget version"                OFF)
endif()

#third party build options
WAGIC_OPTION(BUILD_ZLIB    "build zlib from source"    WIN32)
WAGIC_OPTION(BUILD_JPEG    "build jpeg from source"    (WIN32 OR APPLE OR PSP OR ANDROID))
WAGIC_OPTION(BUILD_PNG     "build png from source"     (WIN32 OR APPLE OR PSP OR ANDROID))
WAGIC_OPTION(BUILD_UNZIP   "build unzip from source"   ON)
WAGIC_OPTION(BUILD_TINYXML "build tinyxml from source" (WIN32 OR APPLE OR PSP OR ANDROID))
WAGIC_OPTION(BUILD_ZIPFS   "build zipfs from source"   ON)

#project options
if(ANDROID)
    WAGIC_OPTION(BUILD_ANDROID_PACKAGE "put the compiled code in an android package" ON)
endif()


if(ANDROID)
    set(LIBRARY_OUTPUT_PATH        "${CMAKE_BINARY_DIR}/lib/${ANDROID_NDK_ABI_NAME}")
    set(3P_LIBRARY_OUTPUT_PATH     "${CMAKE_BINARY_DIR}/3rdparty/lib/${ANDROID_NDK_ABI_NAME}")
    set(WAGIC_LIB_INSTALL_PATH     sdk/native/libs/${ANDROID_NDK_ABI_NAME})
    set(WAGIC_3P_LIB_INSTALL_PATH  sdk/native/3rdparty/libs/${ANDROID_NDK_ABI_NAME})
    set(WAGIC_CONFIG_INSTALL_PATH  sdk/native/jni)
    set(WAGIC_INCLUDE_INSTALL_PATH sdk/native/jni/include)
    set(EXECUTABLE_OUTPUT_PATH     "${CMAKE_BINARY_DIR}/package/bin")
else()
    set(LIBRARY_OUTPUT_PATH        "${CMAKE_BINARY_DIR}/lib")
    set(3P_LIBRARY_OUTPUT_PATH     "${OpenCV_BINARY_DIR}/3rdparty/lib${LIB_SUFFIX}")
    set(WAGIC_LIB_INSTALL_PATH     lib${LIB_SUFFIX})
    set(WAGIC_3P_LIB_INSTALL_PATH  share/wagic/3rdparty/${OPENCV_LIB_INSTALL_PATH})
    set(WAGIC_INCLUDE_INSTALL_PATH "include")
    set(WAGIC_CONFIG_INSTALL_PATH  share/wagic)
endif()

set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/${WAGIC_LIB_INSTALL_PATH}")
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

#if no build type is specified, we assume debug
if(CMAKE_GENERATOR MATCHES "Makefiles|Ninja" AND "${CMAKE_BUILD_TYPE}" STREQUAL "")
  set(CMAKE_BUILD_TYPE Debug)
endif()

include(FindOrBuild)

#set platform dependend configurations
if(PSP)
    FindOrBuildPSPSDK()
    include(platforms/psp/configure.cmake)
elseif(WIN32)
    include(platforms/win/configure.cmake)
elseif(UNIX)
    include(platforms/unix/configure.cmake)
elseif(ANDROID)
    include(platforms/android/configure.cmake)
endif()

#set backend dependend configurations
if(backend_qt_console OR backend_qt_widget)
    add_definitions(-DQT_CONFIG)
    if(backend_qt_console)
        add_definitions(-DTESTSUITE -D_DEBUG)
        add_definitions(-DCONSOLE_CONFIG -DCAPTURE_STDERR)
    endif()
elseif(backend_sdl)
    add_definitions(-DSDL_CONFIG)
endif()

# find or build 3rd party libraries

FindOrBuildZLIB()
FindOrBuildOpenGL()
FindOrBuildTinyXML()
FindOrBuildZipFS()
FindOrBuildUNZIP()
FindOrBuildPNG()
FindOrBuildJPEG()
FindOrBuildBoost()
if(PSP)
    FindOrBuildGIF()
endif()
if(ANDROID)
    FindOrBuildOpenSL()
endif()
if(backend_sdl)
    FindOrBuildSDL2()
endif()
if(backend_psp)
    FindOrBuildFreetype()
    FindOrBuildHgeTools()
    FindOrBuildMikMod()
endif()
if(backend_qt_console OR backend_qt_widget)
    FindOrBuildQt()
endif()
if(ANDROID)
    include(DetectAndroidSDK)
endif()

if(BUILD_ANDROID_PACKAGE)
    find_package(Ant REQUIRED)
endif()

#add jge and mtg projects
add_subdirectory(JGE)
add_subdirectory(projects/mtg)

if(BUILD_ANDROID_PACKAGE)
  add_subdirectory(platforms/android/package)
endif()
