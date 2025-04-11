macro(FindOrBuildZipFS)
    if(BUILD_ZIPFS)
        add_subdirectory(
            ${CMAKE_SOURCE_DIR}/thirdparty/zipFS 
            ${CMAKE_BINARY_DIR}/thirdparty/zipFS)
        set(ZIPFS_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/thirdparty/zipFS)
        set(ZIPFS_LIBRARY zipFS)
    else()
        message(WARNING "ZIPFS must get build")
    endif()
endmacro()

macro(FindOrBuildTinyXML)
    if(BUILD_TINYXML)
        add_definitions(-DTIXML_USE_STL)
        add_subdirectory(
            ${CMAKE_SOURCE_DIR}/thirdparty/tinyxml
            ${CMAKE_BINARY_DIR}/thirdparty/tinyxml)
        set(TINYXML_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/thirdparty/tinyxml)
        set(TINYXML_LIBRARIES tinyxml)
    else()
        find_package(TinyXML REQUIRED)
    endif()
endmacro()

macro(FindOrBuildSDL2)
    if(BUILD_SDL2)
        if(WIN32)
            #SDL2 DirectX build is somehow broken...
            set(SDL_AUDIO FALSE)
            set(SDL_JOYSTICK FALSE)
            set(SDL_HAPTIC FALSE)
            # Forcing static build on Windows as SDL2main is often needed and easier to link statically.
            # Set SDL_SHARED to OFF *before* adding the subdirectory to influence its build.
            set(SDL_SHARED OFF CACHE BOOL "Build SDL2 as a static library" FORCE)
        endif()
        # Add the subdirectory. This creates targets like SDL2::SDL2, SDL2::SDL2-static, SDL2::SDL2main
        add_subdirectory(${CMAKE_SOURCE_DIR}/thirdparty/SDL2 ${CMAKE_BINARY_DIR}/thirdparty/SDL2)
        set(SDL2_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/thirdparty/SDL2/include ${CMAKE_BINARY_DIR}/thirdparty/SDL2)
        # Linking against SDL2::SDL2 below handles includes automatically.
        if(EMSCRIPTEN)
            # Emscripten typically uses static linking
            set(SDL2_LIBRARY SDL2::SDL2-static) # Use the CMake target
        else()
            # Link against SDL2 (shared or static based on SDL_SHARED) and SDL2main
            # SDL2::SDL2 resolves to the correct shared/static target.
            # SDL2::SDL2main is needed on many platforms, especially Windows.
            set(SDL2_LIBRARY SDL2::SDL2 SDL2::SDL2main) # Use the CMake targets
        endif()
    elseif(NOT EMSCRIPTEN)
        # If not building from source, use the find_package module
        find_package(SDL2)
    endif()
endmacro()

macro(FindOrBuildUNZIP)
    if(BUILD_UNZIP)
        add_subdirectory(
            ${CMAKE_SOURCE_DIR}/thirdparty/unzip 
            ${CMAKE_BINARY_DIR}/thirdparty/unzip)
        set(UNZIP_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/thirdparty/unzip)
        set(UNZIP_LIBRARY unzip)
    else()
        message(WARNING "UNZIP must get build")
    endif()
endmacro()

macro(FindOrBuildBoost)
    if(PSP OR UNIX OR WIN32 OR IOS)
        #the psp build does not need more than a few headers
        #todo: remove from the repository
        set(BOOST_INCLUDE_DIRS ${CMAKE_SOURCE_DIR}/thirdparty/Boost)    
#    elseif(WIN32)
        #set BOOST_ROOT to the root of boost
#        set(Boost_USE_STATIC_LIBS ON)
#        set(Boost_USE_MULTITHREADED ON)
#        set(BOOST_ROOT $ENV{BOOST_ROOT})
		
#        set(BOOST_LIBRARYDIR ${BOOST_ROOT}/libs)
#        set(BOOST_INCLUDEDIR ${BOOST_ROOT})
        
#		find_package(Boost COMPONENTS system thread date_time REQUIRED)
#		if(NOT Boost_FOUND)
#            message("Set the BOOST_ROOT environment variable to point to your boost installation.")
#            message("We need system thread and date_time compiled static libs")
#            message("These libs are compiler specific.")
#		endif()
#    elseif(UNIX AND NOT ANDROID)
#        find_package(Boost COMPONENTS system thread date_time REQUIRED)
    elseif(ANDROID)
        #this is a hack. we compile a few boost libds directly into 
        #the application. we should require static libs for android
        #to be available. maybe we could add the build option to
        #download and build a compatible boost version
        find_path(BOOST_INCLUDE_DIRS NAMES bind.hpp HINTS $ENV{ANDROID_BOOST_ROOT} PATH_SUFFIXES boost)
        
        if(BOOST_INCLUDE_DIRS)
            get_filename_component(BOOST_INCLUDE_DIRS ${BOOST_INCLUDE_DIRS} PATH)
            set(ANDROID_BOOST_PTHREAD_SRC_DIR ${BOOST_INCLUDE_DIRS}/libs/thread/src/pthread)
            set(ANDROID_BOOST_SYSTEM_SRC_DIR ${BOOST_INCLUDE_DIRS}/libs/system/src/)
        else()
            message(SEND_ERROR "We require a few boost sources to get compiled into wagic. Please point the ANDROID_BOOST_ROOT environment variable to a boost-source copy root.")
        endif()
    endif()
endmacro()


macro(FindOrBuildZLIB)
    if(BUILD_ZLIB)
        add_subdirectory(${CMAKE_SOURCE_DIR}/thirdparty/zlib)
        set(ZLIB_INCLUDE_DIRS ${CMAKE_SOURCE_DIR}/thirdparty/zlib ${CMAKE_BINARY_DIR}/thirdparty/zlib)
        set(ZLIB_LIBRARIES zlib)
    else()
        if(WIN32)
            set(ZLIB_ROOT ${CMAKE_SOURCE_DIR}/thirdparty/binary/win)
            find_package(ZLIB)
        else()
            if(backend_qt_console OR backend_qt_widget OR EMSCRIPTEN)
        	set(ZLIB_INCLUDE_DIRS ${CMAKE_SOURCE_DIR}/thirdparty/zlib)
            else()
                find_package(ZLIB)
            endif()
        endif()
    endif()
endmacro()

macro(FindOrBuildGIF)
    if(BUILD_GIF)
        message(WARNING "GIF sources are currently not included within the wagic tree")
    else()
		if(PSP)
			find_package(GIF)
		endif()
    endif()
endmacro()

macro(FindOrBuildJPEG)
    if(BUILD_JPEG)
        add_subdirectory(${CMAKE_SOURCE_DIR}/thirdparty/libjpeg ${CMAKE_BINARY_DIR}/thirdparty/libjpeg)
        set(JPEG_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/thirdparty/libjpeg)
        set(JPEG_LIBRARY jpeg)
    else()
        if(WIN32)
            #findJPEG does currently not provide prefix vars to guide it
            find_path(JPEG_INCLUDE_DIR jpeglib.h HINTS ${CMAKE_SOURCE_DIR}/thirdparty/binary/win/include)
            find_library(JPEG_LIBRARY NAMES libjpeg-static-mt HINTS ${CMAKE_SOURCE_DIR}/thirdparty/binary/win/lib)
            
            if(JPEG_INCLUDE_DIR AND JPEG_LIBRARY)
                set(JPEG_FOUND ON)
                mark_as_advanced(JPEG_INCLUDE_DIR JPEG_LIBRARY)
            else()
                message(FATAL_ERROR "Could not find JPEG on windows")
            endif()
        else()
            find_package(JPEG)
        endif()
    endif()
endmacro()

macro(FindOrBuildPNG)
    if(BUILD_PNG)
        add_subdirectory(${CMAKE_SOURCE_DIR}/thirdparty/libpng ${CMAKE_BINARY_DIR}/thirdparty/libpng)
        set(PNG_INCLUDE_DIRS ${CMAKE_SOURCE_DIR}/thirdparty/libpng)
#        find_path(PNG_INCLUDE_DIRS NAMES png.h HINTS ${CMAKE_SOURCE_DIR}/thirdparty/libpng)
        set(PNG_LIBRARIES png)
    else()
        if(WIN32)
            #findPNG does currently not provide prefix vars. so we find
            find_path(PNG_INCLUDE_DIRS png.h HINTS ${CMAKE_SOURCE_DIR}/thirdparty/binary/win/include)
            find_library(PNG_LIBRARIES libpng HINTS ${CMAKE_SOURCE_DIR}/thirdparty/binary/win/lib)
                    
            if (PNG_LIBRARIES AND PNG_INCLUDE_DIRS)
                set(PNG_FOUND ON)
                mark_as_advanced(PNG_INCLUDE_DIRS PNG_LIBRARIES)
            else()
                message(FATAL_ERROR "Could not find PNG on windows")
            endif()
        else()
            find_package(PNG)
        endif()
    endif()
endmacro()

macro(FindOrBuildFreetype)
    if(PSP)
        set(ENV{FREETYPE_INCLUDE_DIRS} ${CMAKE_SOURCE_DIR}/thirdparty/binary/psp/include)
        set(ENV{FREETYPE_LIBRARIES} freetype)
    endif()
endmacro()

macro(FindOrBuildHgeTools)
	if(PSP)
		find_library(HGETOOLS_LIBRARY NAMES hgetools HINTS "${CMAKE_SOURCE_DIR}/thirdparty/binary/psp/lib")
		set(HGETOOLS_INCLUDE_DIRS "${CMAKE_SOURCE_DIR}/thirdparty/binary/psp/include")
	endif()
endmacro()

macro(FindOrBuildMikMod)
	if(PSP)
		find_library(MIKMOD_LIBRARY NAMES mikmod HINTS "${CMAKE_SOURCE_DIR}/thirdparty/binary/psp/lib")
        set(MIKMOD_INCLUDE_DIRS "${CMAKE_SOURCE_DIR}/thirdparty/binary/psp/include")
    endif()
endmacro()

macro(FindOrBuildQt)
    if(backend_qt_console)
        add_definitions(-DCONSOLE_CONFIG -DTESTSUITE)
        find_package(Qt5Core REQUIRED)
        find_package(Qt5 COMPONENTS Core Network Multimedia REQUIRED)
    elseif(backend_qt_widget)
        add_definitions(-DQT_WIDGET)
        find_package(OpenGL REQUIRED)
        find_package(X11 REQUIRED)
        find_package(Qt5 COMPONENTS Core Gui OpenGL Network Multimedia REQUIRED)
    endif()
#    include(${QT_USE_FILE})
endmacro()

macro(FindOrBuildOpenGL)
    if(backend_sdl OR backend_qt_console OR backend_qt_widget)
        if(ANDROID)
            #find openglesv on android
            set(OPENGL_LIBRARIES "-ldl -lGLESv1_CM -lGLESv2 -llog -landroid")
        elseif(EMSCRIPTEN)
            set(OPENGL_LIBRARIES "")
        else()
            find_package(OpenGL)
#            find_package(GLUT)
        endif()
    endif()
endmacro()

macro(FindOrBuildOpenSL)
    find_package(OpenSL)
endmacro()

macro(FindOrBuildPSPSDK)
    find_package(PSPSDK COMPONENTS psppower pspmpeg pspaudiocodec pspaudiolib pspaudio pspmp3 pspgum pspgu psprtc pspfpu REQUIRED)
endmacro()
