cmake_minimum_required(VERSION 4.0)
if(CMAKE_MAJOR_VERSION STRGREATER 3)
	cmake_policy(SET CMP0054 NEW)
endif()

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_SOURCE_DIR}/CMakeModules)

#this block will fix install prefixes to install everything in a subdirectory
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

if(ANDROID OR PSP OR IOS)
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
    WAGIC_OPTION(backend_sdl         "build for sdl"          (WIN32 OR ANDROID OR EMSCRIPTEN))
endif()
if(NOT backend_sdl AND UNIX AND NOT ANDROID AND NOT IOS)
    WAGIC_OPTION(backend_qt_console  "build qt-console version with testsuit" ON)
    WAGIC_OPTION(backend_qt_widget   "build qt-widget version"                OFF)
endif()

#third party build options
WAGIC_OPTION(BUILD_ZLIB    "build zlib from source"    ((NOT MINGW) AND (WIN32 OR APPLE)))
WAGIC_OPTION(BUILD_JPEG    "build jpeg from source"    (WIN32 OR APPLE OR PSP OR ANDROID OR EMSCRIPTEN))
WAGIC_OPTION(BUILD_PNG     "build png from source"     (WIN32 OR APPLE OR PSP OR ANDROID OR EMSCRIPTEN))
WAGIC_OPTION(BUILD_UNZIP   "build unzip from source"   ON)
WAGIC_OPTION(BUILD_TINYXML "build tinyxml from source" (WIN32 OR APPLE OR PSP OR ANDROID OR EMSCRIPTEN OR IOS))
WAGIC_OPTION(BUILD_ZIPFS   "build zipfs from source"   ON)
WAGIC_OPTION(BUILD_SDL2    "build SDL2 from source"    (backend_sdl AND (UNIX OR WIN32 OR ANDROID) AND (NOT EMSCRIPTEN AND NOT IOS)))
#WAGIC_OPTION(BUILD_CURL    "build curl from source"  (backend_sdl AND WIN32))

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
    set(3P_LIBRARY_OUTPUT_PATH     "${CMAKE_BINARY_DIR}/3rdparty/lib${LIB_SUFFIX}")
    set(WAGIC_LIB_INSTALL_PATH     lib${LIB_SUFFIX})
    set(WAGIC_3P_LIB_INSTALL_PATH  share/wagic/3rdparty/${WAGIC_LIB_INSTALL_PATH})
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
elseif(EMSCRIPTEN)
    include(platforms/emscripten/configure.cmake)
elseif(WIN32)
    include(platforms/win/configure.cmake)
elseif(APPLE)
    include(platforms/macosx/configure.cmake)
elseif(UNIX AND NOT ANDROID)
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
#FindOrBuildCurl()

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

if(EMSCRIPTEN)
  set(CMAKE_EXECUTABLE_SUFFIX ".html")
endif()

if(IOS)
  list_add_prefix(JGE_SOURCES " JGE/")
  list_add_prefix(MTG_generic_src " projects/mtg/")
  list_add_prefix(JGE_INCLUDE_DIRS " -IJGE/")
  list_add_prefix(MTG_INCLUDE_DIRS " -Iprojects/mtg/")
  list_add_prefix(TINYXML_SRC " thirdparty/tinyxml/")
  list_add_prefix(UNZIP_SRC " thirdparty/unzip/")
  list_add_prefix(ZIPFS_SRC " thirdparty/zipFS/")

  file(WRITE makefile.ios
     "ARCHS= armv7 armv7s \n"
     "include ${THEOS_PATH}/makefiles/common.mk\n"
     "TARGET= iphone:clang:latest:8.0 \n"
     "APPLICATION_NAME = " ${PROJECT_NAME} "\n"
     ${PROJECT_NAME} "_FILES =" ${ZIPFS_SRC} ${UNZIP_SRC} ${TINYXML_SRC} ${JGE_SOURCES} ${MTG_generic_src} " thirdparty/Boost/lib/pthread/once.cpp thirdparty/Boost/lib/pthread/thread.cpp\n"
     ${PROJECT_NAME} "_LDFLAGS = -lz\n"
     ${PROJECT_NAME} "_FRAMEWORKS = UIKit CoreGraphics OpenGLES Foundation CFNetwork MobileCoreServices AVFoundation OpenAL AudioToolbox QuartzCore SystemConfiguration\n"
     "include ${THEOS_PATH}/makefiles/application.mk\n"
     "ADDITIONAL_CFLAGS = " ${MTG_INCLUDE_DIRS} ${JGE_INCLUDE_DIRS} " -I" ${ZIPFS_INCLUDE_DIR} " -I" ${UNZIP_INCLUDE_DIR} " -I" ${HGE_INCLUDE_DIR} ${TINYXML_INCLUDE_DIR} " -I" ${ZLIB_INCLUDE_DIRS} ${BOOST_INCLUDE_DIRS} " "
         "-DIOS -D__arm__ -DTIXML_USE_STL -DVERSION=\"$(GIT_VERSION)\" "
         "-Wno-parentheses-equality -Wno-delete-non-virtual-dtor "
         "-Wno-tautological-undefined-compare -Wno-undefined-bool-conversion "
         "-Wno-visibility -Wno-deprecated-declarations -Wno-non-literal-null-conversion "
         "-Wno-format -Wno-distributed-object-modifiers -Wno-missing-braces -Wno-uninitialized "
         "-Wno-unused-const-variable -Wno-unused-function -Wno-unknown-warning-option -Wno-unused-local-typedef "
         "-x objective-c++ \n"

     "_THEOS_TARGET_ONLY_OBJCFLAGS :=\"\"\n"
     )
endif()
