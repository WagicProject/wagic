set(ENGINE_LIB_NAME jge)

#the sources, jge will need always
set(JGE_generic_src
        src/Encoding.cpp
        src/JAnimator.cpp
        src/JApp.cpp
        src/JDistortionMesh.cpp
        src/JFileSystem.cpp
        src/JGameObject.cpp
        src/JGE.cpp
        src/JGui.cpp
        src/JLogger.cpp
        src/JLBFont.cpp
        src/JOBJModel.cpp
        src/JParticle.cpp
        src/JParticleEffect.cpp
        src/JParticleEmitter.cpp
        src/JParticleSystem.cpp
        src/JResourceManager.cpp
        src/JSpline.cpp
        src/JSprite.cpp
        src/Vector2D.cpp)

set(JGE_hge_src
        src/hge/hgecolor.cpp
        src/hge/hgedistort.cpp
        src/hge/hgefont.cpp
        src/hge/hgeparticle.cpp
        src/hge/hgerect.cpp
        src/hge/hgevector.cpp)

#the sources we need to get graphical output desktops
#used by qt-widget build option
set(JGE_graphics_src
        src/Downloader.cpp
        include/Downloader.h
        src/JMD2Model.cpp
        src/pc/JGfx.cpp)

#the sources we need to fake graphical output on desktops
#these are used for the console-only testsuit
set(JGE_console_src
        src/OutputCapturer.cpp
        src/JGfx-fake.cpp)

#the sources we need on linux
set(JGE_linux_src
        src/JNetwork.cpp
        src/pc/JSocket.cpp
        src/pc/JSfx.cpp)
        
set(JGE_android_src
        src/JNetwork.cpp
        src/JSocket.cpp
        src/android/JSfx.cpp)

#the sources we need if we compile for psp
#note: main.cpp contains the main-function.
#the other main functions reside in projects/mtg to reduce cross-library dependencies.
#there may be a way to get all main-functions into the same project
set(JGE_psp_src
        src/JNetwork.cpp
        src/JSocket.cpp
        src/JGfx.cpp
        src/JSfx.cpp
        src/JAudio.cpp
        src/JMP3.cpp
        src/decoder_prx.cpp
        src/main.cpp
        src/vram.cpp)

set(JGE_ios_src
	src/iOS/main.m
	src/iOS/wagicAppDelegate.m
	src/iOS/ES2Renderer.m
	src/iOS/EAGLView.m
	src/iOS/EAGLViewController.m
	src/iOS/JSfx.cpp)

set(JGE_windows_src
        src/JNetwork.cpp
        src/pc/JSocket.cpp
	src/windows/JSfx.cpp)

set(JGE_INCLUDE_DIRS include include/hge)
set(EXTRA_INCLUDE_DIR ../projects/mtg/extra)


set(JGE_SOURCES ${JGE_generic_src} ${JGE_hge_src})
set(JGE_INTERNAL_INCLUDE_DIRS ${TINYXML_INCLUDE_DIR} ${ZIPFS_INCLUDE_DIR} ${UNZIP_INCLUDE_DIR})

set(JGE_ZIP_LIBRARIES ${ZIPFS_LIBRARY} ${UNZIP_LIRARY} ${ZLIB_LIBRARIES})
set(JGE_IMG_LIBRARIES ${JPEG_LIBRARY} ${PNG_LIBRARIES})
set(JGE_LINK_LIBRARIES ${TINYXML_LIBRARIES} ${JGE_ZIP_LIBRARIES})

if(backend_qt_console OR backend_qt_widget)
    set(CMAKE_INCLUDE_CURRENT_DIR ON)
    set(CMAKE_AUTOMOC TRUE)

    if(UNIX AND NOT ANDROID)
        set(JGE_SOURCES ${JGE_SOURCES} ${JGE_linux_src})

        if(backend_qt_console)
            set(JGE_SOURCES ${JGE_SOURCES}  ${JGE_console_src})
            set(JGE_QT_LINK_LIBRARIES Qt5::Core Qt5::Multimedia Qt5::Network )
        elseif(backend_qt_widget)
            set(JGE_SOURCES ${JGE_SOURCES} ${JGE_graphics_src})
            set(JGE_QT_LINK_LIBRARIES Qt5::Core Qt5::Multimedia Qt5::Network Qt5::OpenGL ${OPENGL_LIBRARIES} )
        endif()
        set(JGE_LINK_LIBRARIES ${JGE_QT_LINK_LIBRARIES} ${JGE_LINK_LIBRARIES} ${JGE_IMG_LIBRARIES})
    else()
        message(FATAL_ERROR "qt builds of jge platforms other than linux are not supported")
    endif()

endif()

if(backend_sdl)
    set(JGE_INTERNAL_INCLUDE_DIRS ${JGE_INTERNAL_INCLUDE_DIRS} ${ZLIB_INCLUDE_DIRS} ${SDL2_INCLUDE_DIR} ${PNG_INCLUDE_DIRS} ${JPEG_INCLUDE_DIR} )
    set(JGE_SOURCES ${JGE_SOURCES} ${JGE_graphics_src})

    if((UNIX OR EMSCRIPTEN) AND NOT ANDROID)
        set(JGE_INTERNAL_INCLUDE_DIRS ${JGE_INTERNAL_INCLUDE_DIRS} ${OPENGL_INCLUDE_DIR} ${GLUT_INCLUDE_DIR})
        set(JGE_SOURCES ${JGE_SOURCES} ${JGE_linux_src})
        set(JGE_SDL_LINK_LIBRARIES ${SDL2_LIBRARY} ${OPENGL_LIBRARIES} ${GLUT_LIBRARY})
    elseif(ANDROID)
        set(JGE_INTERNAL_INCLUDE_DIRS ${JGE_INTERNAL_INCLUDE_DIRS} ${BOOST_INCLUDE_DIRS} ${OPENSL_INCLUDE_DIR} )
        set(JGE_SOURCES ${JGE_SOURCES} ${JGE_android_src})
        set(JGE_SDL_LINK_LIBRARIES ${SDL2_LIBRARY} ${OPENSL_LIBRARIES})
    elseif(WIN32)
        set(JGE_INTERNAL_INCLUDE_DIRS ${JGE_INTERNAL_INCLUDE_DIRS} ${EXTRA_INCLUDE_DIR} ${CMAKE_SOURCE_DIR}/thirdparty/binary/win/include)
        set(JGE_SOURCES ${JGE_SOURCES} ${JGE_windows_src})
        set(JGE_LINK_DIRECTORIES ${Boost_LIBRARY_DIR})
        set(JGE_SDL_LINK_LIBRARIES ${SDL2_LIBRARY} ${OPENSL_LIBRARIES})
    endif()
    set(JGE_LINK_LIBRARIES ${JGE_SDL_LINK_LIBRARIES} ${JGE_LINK_LIBRARIES} ${JGE_IMG_LIBRARIES})
endif()

if(backend_psp)
    if(PSP)        
        set(JGE_INTERNAL_INCLUDE_DIRS ${JGE_INTERNAL_INCLUDE_DIRS} ${CMAKE_SOURCE_DIR}/thirdparty/psp/include ${BOOST_INCLUDE_DIRS} ${GIF_INCLUDE_DIR} ${PNG_INCLUDE_DIRS} )
        set(JGE_SOURCES ${JGE_SOURCES} ${JGE_psp_src})
        set(JGE_LINK_LIBRARIES ${JGE_LINK_LIBRARIES} ${GIF_LIBRARIES} ${JGE_IMG_LIBRARIES} ${PSPSDK_LIB} ${HGETOOLS_LIBRARY} ${FREETYPE_LIB} z m ${MIKMOD_LIBRARY} stdc++ ${PSPSDK_REQUIRED_LIB})
    else()
        message(FATAL_ERROR "use the cross-compile toolchain to build as the psp target")
    endif()
endif()

if(IOS)
    set(JGE_SOURCES ${JGE_ios_src} src/pc/JGfx.cpp)
    set(JGE_SOURCES ${JGE_SOURCES} PARENT_SCOPE)
    set(JGE_INCLUDE_DIRS ${JGE_INCLUDE_DIRS} src/iOS PARENT_SCOPE)
endif()

add_library(${ENGINE_LIB_NAME} STATIC ${JGE_SOURCES})
add_library(Wagic::JGE ALIAS ${ENGINE_LIB_NAME})

target_include_directories(${ENGINE_LIB_NAME} PUBLIC ${JGE_INCLUDE_DIRS} ${JGE_INTERNAL_INCLUDE_DIRS})

target_link_libraries(${ENGINE_LIB_NAME} PUBLIC ${JGE_LINK_LIBRARIES})
link_directories(${JGE_LINK_DIRECTORIES})

set_target_properties(${ENGINE_LIB_NAME} PROPERTIES CXX_STANDARD 11 CXX_STANDARD_REQUIRED YES CXX_EXTENSIONS NO )
