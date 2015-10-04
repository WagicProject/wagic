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
        src/JNetwork.cpp
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
        src/pc/JSocket.cpp
        src/pc/JSfx.cpp)
        
set(JGE_android_src
        src/JSocket.cpp 
        src/android/JSfx.cpp)

#the sources we need if we compile for psp
#note: main.cpp contains the main-function.
#the other main functions reside in projects/mtg to reduce cross-library dependencies.
#there may be a way to get all main-functions into the same project
set(JGE_psp_src
        src/JSocket.cpp 
        src/JGfx.cpp
        src/JSfx.cpp
        src/JAudio.cpp
        src/JMP3.cpp
        src/decoder_prx.cpp
        src/main.cpp
        src/vram.cpp)

set(JGE_INCLUDE_DIRS include include/hge)
set(EXTRA_INCLUDE_DIR ../projects/mtg/extra)

#turn moc on
if(backend_qt_console OR backend_qt_widget)
    set(CMAKE_INCLUDE_CURRENT_DIR ON)
    set(CMAKE_AUTOMOC TRUE)
endif()


if(backend_qt_console)
    if(UNIX AND NOT ANDROID)
        set(JGE_INTERNAL_INCLUDE_DIRS ${TINYXML_INCLUDE_DIR} ${ZIPFS_INCLUDE_DIR} 
            ${UNZIP_INCLUDE_DIR} ${QT_MKSPECS_DIR}/default ${Qt5Core_INCLUDE_DIRS} ${Qt5Multimedia_INCLUDE_DIRS})
        set(JGE_SOURCES ${JGE_generic_src} ${JGE_console_src} ${JGE_linux_src} ${JGE_hge_src})
        set(JGE_LINK_LIBRARIES ${Qt5Core_LIBRARIES} ${Qt5Multimedia_LIBRARIES} ${ZIPFS_LIBRARY} ${UNZIP_LIRARY}
            ${JPEG_LIBRARIES} ${PNG_LIBRARIES})
    else()
        message(FATAL_ERROR "qt builds of jge platforms other than linux are not supported")
    endif()
endif()

if(backend_qt_widget)
    if(UNIX AND NOT ANDROID)
        set(JGE_INTERNAL_INCLUDE_DIRS ${TINYXML_INCLUDE_DIR} ${ZIPFS_INCLUDE_DIR} 
            ${UNZIP_INCLUDE_DIR} ${QT_MKSPECS_DIR}/default ${Qt5Core_INCLUDE_DIRS} ${Qt5Multimedia_INCLUDE_DIRS} ${Qt5OpenGL_INCLUDE_DIRS} )
        set(JGE_SOURCES ${JGE_generic_src} ${JGE_graphics_src} ${JGE_linux_src} ${JGE_hge_src})
        set(JGE_LINK_LIBRARIES ${Qt5Core_LIBRARIES} ${Qt5Multimedia_LIBRARIES} ${Qt5OpenGL_LIBRARIES} ${ZIPFS_LIBRARY} ${UNZIP_LIRARY}
            ${JPEG_LIBRARIES} ${PNG_LIBRARIES})
    else()
        message(FATAL_ERROR "qt builds of jge platforms other than linux are not supported")
    endif()
endif()

if(backend_sdl)
    if(UNIX AND NOT ANDROID)
        set(JGE_INTERNAL_INCLUDE_DIRS ${TINYXML_INCLUDE_DIR} ${ZIPFS_INCLUDE_DIR}   
            ${SDL2_INCLUDE_DIR} ${UNZIP_INCLUDE_DIR} ${PNG_INCLUDE_DIRS} ${BOOST_INCLUDE_DIRS} ${ZLIB_INCLUDE_DIR})
        set(JGE_SOURCES ${JGE_generic_src} ${JGE_graphics_src} ${JGE_linux_src} ${JGE_hge_src})
        set(JGE_LINK_LIBRARIES ${ZIPFS_LIBRARY} ${UNZIP_LIRARY} ${JPEG_LIBRARY} 
            ${PNG_LIBRARIES})
    elseif(ANDROID)
        set(JGE_INTERNAL_INCLUDE_DIRS ${BOOST_INCLUDE_DIRS} ${TINYXML_INCLUDE_DIR} ${ZIPFS_INCLUDE_DIR}     
            ${SDL2_INCLUDE_DIR} ${UNZIP_INCLUDE_DIR} ${PNG_INCLUDE_DIRS} ${JPEG_INCLUDE_DIR} 
            ${OPENSL_INCLUDE_DIR} ${ZLIB_INCLUDE_DIR})
        set(JGE_SOURCES ${JGE_generic_src} ${JGE_android_src} ${JGE_graphics_src} ${JGE_hge_src})
        set(JGE_LINK_LIBRARIES ${ZIPFS_LIBRARY} ${UNZIP_LIRARY} ${JPEG_LIBRARY} ${PNG_LIBRARIES} 
            ${OPENSL_LIBRARIES}) 
    elseif(WIN32)
        set(JGE_INTERNAL_INCLUDE_DIRS ${EXTRA_INCLUDE_DIR} ${TINYXML_INCLUDE_DIR} ${ZIPFS_INCLUDE_DIR} ${SDL2_INCLUDE_DIR} 
            ${UNZIP_INCLUDE_DIR} ${PNG_INCLUDE_DIRS} ${JPEG_INCLUDE_DIR} ${ZLIB_INCLUDE_DIR} ${CMAKE_SOURCE_DIR}/thirdparty/binary/win/include)
        set(JGE_SOURCES ${JGE_generic_src} ${JGE_graphics_src} ${JGE_linux_src} ${JGE_hge_src})
        set(JGE_LINK_LIBRARIES ${ZIPFS_LIBRARY} ${UNZIP_LIRARY} ${JPEG_LIBRARY} ${PNG_LIBRARIES})
        set(JGE_LINK_DIRECTORIES ${Boost_LIBRARY_DIR} ${SDL2_LIBRARY_DIR})
    endif()
endif()

if(backend_psp)
    if(PSP)        
        #${PSPSDK_PATH}/include should be in system pathes
        set(JGE_INTERNAL_INCLUDE_DIRS ${CMAKE_SOURCE_DIR}/thirdparty/psp/include ${TINYXML_INCLUDE_DIR} 
            ${ZIPFS_INCLUDE_DIR} ${UNZIP_INCLUDE_DIR} ${BOOST_INCLUDE_DIRS} ${GIF_INCLUDE_DIR})
        set(JGE_SOURCES ${JGE_generic_src} ${JGE_psp_src} ${JGE_hge_src})
        set(JGE_LINK_LIBRARIES ${TINYXML_LIBRARIES} ${ZIPFS_LIBRARY} ${UNZIP_LIRARY})
    else()
        message(FATAL_ERROR "use the cross-compile toolchain to build as the psp target")
    endif()
endif()

add_library(jge STATIC ${JGE_SOURCES})
include_directories(${JGE_INTERNAL_INCLUDE_DIRS} ${JGE_INCLUDE_DIRS})
target_link_libraries(jge ${JGE_LINK_LIBRARIES})
link_directories(${JGE_LINK_DIRECTORIES})

set(${JGE_LIBRARIES} jge)
