#-------------------------------------------------
#-------------------------------------------------

include(wagic.pri)

DEFINES += SDL_CONFIG
#
# Project created by QtCreator 2010-06-30T19:48:30
#
QT -= core gui opengl network declarative

#unix|windows:QMAKE_CXXFLAGS += -std=c++11

INCLUDEPATH += ../../JGE/Dependencies/SDL/include
unix:INCLUDEPATH += /usr/include/GL
unix:INCLUDEPATH += /usr/local/include/SDL
OBJECTS_DIR = objs
MOC_DIR = objs
DESTDIR = bin

macx|unix:LIBS += -lz -lboost_thread-mt
unix:LIBS += -ljpeg -lgif -lpng12 -L/usr/local/lib -lGL -lGLU -lSDL
windows:LIBS += -L../../JGE/Dependencies/lib -L../../Boost/lib -llibjpeg-static-mt-debug -lgiflib -llibpng -lfmodvc

CONFIG(debug, debug|release):SOURCES += src/TestSuiteAI.cpp

# JGE, could probably be moved outside
SOURCES += \
        ../../JGE/src/SDLmain.cpp\
        ../../JGE/src/JMD2Model.cpp

windows{

    SOURCES += \
            ../../JGE/Dependencies/SDL/src/core/windows/SDL_windows.c\
            ../../JGE/Dependencies/SDL/src/events/SDL_clipboardevents.c\
            ../../JGE/Dependencies/SDL/src/events/SDL_gesture.c\
            ../../JGE/Dependencies/SDL/src/events/SDL_touch.c\
            ../../JGE/Dependencies/SDL/src/libm/e_atan2.c\
            ../../JGE/Dependencies/SDL/src/libm/e_log.c\
            ../../JGE/Dependencies/SDL/src/libm/e_pow.c\
            ../../JGE/Dependencies/SDL/src/libm/e_rem_pio2.c\
            ../../JGE/Dependencies/SDL/src/libm/e_sqrt.c\
            ../../JGE/Dependencies/SDL/src/libm/k_cos.c\
            ../../JGE/Dependencies/SDL/src/libm/k_rem_pio2.c\
            ../../JGE/Dependencies/SDL/src/libm/k_sin.c\
            ../../JGE/Dependencies/SDL/src/libm/s_atan.c\
            ../../JGE/Dependencies/SDL/src/libm/s_copysign.c\
            ../../JGE/Dependencies/SDL/src/libm/s_cos.c\
            ../../JGE/Dependencies/SDL/src/libm/s_fabs.c\
            ../../JGE/Dependencies/SDL/src/libm/s_floor.c\
            ../../JGE/Dependencies/SDL/src/libm/s_scalbn.c\
            ../../JGE/Dependencies/SDL/src/libm/s_sin.c\
            ../../JGE/Dependencies/SDL/src/render/direct3d/SDL_render_d3d.c\
            ../../JGE/Dependencies/SDL/src/render/opengl/SDL_render_gl.c\
            ../../JGE/Dependencies/SDL/src/render/opengl/SDL_shaders_gl.c\
            ../../JGE/Dependencies/SDL/src/render/SDL_render.c\
            ../../JGE/Dependencies/SDL/src/render/SDL_yuv_mmx.c\
            ../../JGE/Dependencies/SDL/src/render/SDL_yuv_sw.c\
            ../../JGE/Dependencies/SDL/src/render/software/SDL_blendfillrect.c\
            ../../JGE/Dependencies/SDL/src/render/software/SDL_blendline.c\
            ../../JGE/Dependencies/SDL/src/render/software/SDL_blendpoint.c\
            ../../JGE/Dependencies/SDL/src/render/software/SDL_drawline.c\
            ../../JGE/Dependencies/SDL/src/render/software/SDL_drawpoint.c\
            ../../JGE/Dependencies/SDL/src/render/software/SDL_render_sw.c\
            ../../JGE/Dependencies/SDL/src/SDL.c\
            ../../JGE/Dependencies/SDL/src/SDL_assert.c\
            ../../JGE/Dependencies/SDL/src/atomic/SDL_atomic.c\
            ../../JGE/Dependencies/SDL/src/atomic/SDL_spinlock.c\
            ../../JGE/Dependencies/SDL/src/audio/SDL_audio.c\
            ../../JGE/Dependencies/SDL/src/audio/SDL_audiocvt.c\
            ../../JGE/Dependencies/SDL/src/audio/SDL_audiodev.c\
            ../../JGE/Dependencies/SDL/src/audio/SDL_audiotypecvt.c\
            ../../JGE/Dependencies/SDL/src/SDL_hints.c\
            ../../JGE/Dependencies/SDL/src/SDL_log.c\
            ../../JGE/Dependencies/SDL/src/video/dummy/SDL_nullframebuffer.c\
            ../../JGE/Dependencies/SDL/src/video/SDL_blit.c\
            ../../JGE/Dependencies/SDL/src/video/SDL_blit_0.c\
            ../../JGE/Dependencies/SDL/src/video/SDL_blit_1.c\
            ../../JGE/Dependencies/SDL/src/video/SDL_blit_A.c\
            ../../JGE/Dependencies/SDL/src/video/SDL_blit_auto.c\
            ../../JGE/Dependencies/SDL/src/video/SDL_blit_copy.c\
            ../../JGE/Dependencies/SDL/src/video/SDL_blit_N.c\
            ../../JGE/Dependencies/SDL/src/video/SDL_blit_slow.c\
            ../../JGE/Dependencies/SDL/src/video/SDL_bmp.c\
            ../../JGE/Dependencies/SDL/src/SDL_compat.c\
            ../../JGE/Dependencies/SDL/src/cpuinfo/SDL_cpuinfo.c\
            ../../JGE/Dependencies/SDL/src/video/SDL_clipboard.c\
            ../../JGE/Dependencies/SDL/src/video/SDL_shape.c\
            ../../JGE/Dependencies/SDL/src/audio/windib/SDL_dibaudio.c\
            ../../JGE/Dependencies/SDL/src/audio/disk/SDL_diskaudio.c\
            ../../JGE/Dependencies/SDL/src/audio/dummy/SDL_dummyaudio.c\
            ../../JGE/Dependencies/SDL/src/audio/windx5/SDL_dx5audio.c\
            ../../JGE/Dependencies/SDL/src/joystick/windows/SDL_dxjoystick.c\
            ../../JGE/Dependencies/SDL/src/SDL_error.c\
            ../../JGE/Dependencies/SDL/src/events/SDL_events.c\
            ../../JGE/Dependencies/SDL/src/SDL_fatal.c\
            ../../JGE/Dependencies/SDL/src/video/SDL_fillrect.c\
            ../../JGE/Dependencies/SDL/src/stdlib/SDL_getenv.c\
            ../../JGE/Dependencies/SDL/src/haptic/SDL_haptic.c\
            ../../JGE/Dependencies/SDL/src/stdlib/SDL_iconv.c\
            ../../JGE/Dependencies/SDL/src/joystick/SDL_joystick.c\
            ../../JGE/Dependencies/SDL/src/events/SDL_keyboard.c\
            ../../JGE/Dependencies/SDL/src/stdlib/SDL_malloc.c\
            ../../JGE/Dependencies/SDL/src/audio/SDL_mixer.c\
            ../../JGE/Dependencies/SDL/src/joystick/windows/SDL_mmjoystick.c\
            ../../JGE/Dependencies/SDL/src/events/SDL_mouse.c\
            ../../JGE/Dependencies/SDL/src/video/dummy/SDL_nullevents.c\
            ../../JGE/Dependencies/SDL/src/video/dummy/SDL_nullvideo.c\
            ../../JGE/Dependencies/SDL/src/video/SDL_pixels.c\
            ../../JGE/Dependencies/SDL/src/power/SDL_power.c\
            ../../JGE/Dependencies/SDL/src/stdlib/SDL_qsort.c\
            ../../JGE/Dependencies/SDL/src/events/SDL_quit.c\
            ../../JGE/Dependencies/SDL/src/video/SDL_rect.c\
            ../../JGE/Dependencies/SDL/src/video/SDL_RLEaccel.c\
            ../../JGE/Dependencies/SDL/src/file/SDL_rwops.c\
            ../../JGE/Dependencies/SDL/src/stdlib/SDL_stdlib.c\
            ../../JGE/Dependencies/SDL/src/video/SDL_stretch.c\
            ../../JGE/Dependencies/SDL/src/stdlib/SDL_string.c\
            ../../JGE/Dependencies/SDL/src/video/SDL_surface.c\
            ../../JGE/Dependencies/SDL/src/haptic/windows/SDL_syshaptic.c\
            ../../JGE/Dependencies/SDL/src/loadso/windows/SDL_sysloadso.c\
            ../../JGE/Dependencies/SDL/src/thread/windows/SDL_sysmutex.c\
            ../../JGE/Dependencies/SDL/src/power/windows/SDL_syspower.c\
            ../../JGE/Dependencies/SDL/src/thread/windows/SDL_syssem.c\
            ../../JGE/Dependencies/SDL/src/thread/windows/SDL_systhread.c\
            ../../JGE/Dependencies/SDL/src/timer/windows/SDL_systimer.c\
            ../../JGE/Dependencies/SDL/src/thread/SDL_thread.c\
            ../../JGE/Dependencies/SDL/src/timer/SDL_timer.c\
            ../../JGE/Dependencies/SDL/src/video/SDL_video.c\
            ../../JGE/Dependencies/SDL/src/audio/SDL_wave.c\
            ../../JGE/Dependencies/SDL/src/video/windows/SDL_windowsclipboard.c\
            ../../JGE/Dependencies/SDL/src/video/windows/SDL_windowsevents.c\
            ../../JGE/Dependencies/SDL/src/video/windows/SDL_windowsframebuffer.c\
            ../../JGE/Dependencies/SDL/src/video/windows/SDL_windowskeyboard.c\
            ../../JGE/Dependencies/SDL/src/video/windows/SDL_windowsmodes.c\
            ../../JGE/Dependencies/SDL/src/video/windows/SDL_windowsmouse.c\
            ../../JGE/Dependencies/SDL/src/video/windows/SDL_windowsopengl.c\
            ../../JGE/Dependencies/SDL/src/video/windows/SDL_windowsshape.c\
            ../../JGE/Dependencies/SDL/src/video/windows/SDL_windowsvideo.c\
            ../../JGE/Dependencies/SDL/src/video/windows/SDL_windowswindow.c\
            ../../JGE/Dependencies/SDL/src/events/SDL_windowevents.c


    HEADERS += \
            ../../JGE/Dependencies/SDL/include/SDL.h\
            ../../JGE/Dependencies/SDL/include/SDL_assert.h\
            ../../JGE/Dependencies/SDL/include/SDL_atomic.h\
            ../../JGE/Dependencies/SDL/include/SDL_audio.h\
            ../../JGE/Dependencies/SDL/include/SDL_blendmode.h\
            ../../JGE/Dependencies/SDL/include/SDL_clipboard.h\
            ../../JGE/Dependencies/SDL/include/SDL_compat.h\
            ../../JGE/Dependencies/SDL/include/SDL_config.h\
            ../../JGE/Dependencies/SDL/include/SDL_config_windows.h\
            ../../JGE/Dependencies/SDL/include/SDL_copying.h\
            ../../JGE/Dependencies/SDL/include/SDL_cpuinfo.h\
            ../../JGE/Dependencies/SDL/include/SDL_endian.h\
            ../../JGE/Dependencies/SDL/include/SDL_error.h\
            ../../JGE/Dependencies/SDL/include/SDL_events.h\
            ../../JGE/Dependencies/SDL/include/SDL_gesture.h\
            ../../JGE/Dependencies/SDL/include/SDL_haptic.h\
            ../../JGE/Dependencies/SDL/include/SDL_hints.h\
            ../../JGE/Dependencies/SDL/include/SDL_input.h\
            ../../JGE/Dependencies/SDL/include/SDL_joystick.h\
            ../../JGE/Dependencies/SDL/include/SDL_keyboard.h\
            ../../JGE/Dependencies/SDL/include/SDL_keycode.h\
            ../../JGE/Dependencies/SDL/include/SDL_loadso.h\
            ../../JGE/Dependencies/SDL/include/SDL_log.h\
            ../../JGE/Dependencies/SDL/include/SDL_main.h\
            ../../JGE/Dependencies/SDL/include/SDL_mouse.h\
            ../../JGE/Dependencies/SDL/include/SDL_mutex.h\
            ../../JGE/Dependencies/SDL/include/SDL_name.h\
            ../../JGE/Dependencies/SDL/include/SDL_opengl.h\
            ../../JGE/Dependencies/SDL/include/SDL_opengles.h\
            ../../JGE/Dependencies/SDL/include/SDL_pixels.h\
            ../../JGE/Dependencies/SDL/include/SDL_platform.h\
            ../../JGE/Dependencies/SDL/include/SDL_power.h\
            ../../JGE/Dependencies/SDL/include/SDL_quit.h\
            ../../JGE/Dependencies/SDL/include/SDL_rect.h\
            ../../JGE/Dependencies/SDL/include/SDL_render.h\
            ../../JGE/Dependencies/SDL/include/SDL_revision.h\
            ../../JGE/Dependencies/SDL/include/SDL_rwops.h\
            ../../JGE/Dependencies/SDL/include/SDL_scancode.h\
            ../../JGE/Dependencies/SDL/include/SDL_shape.h\
            ../../JGE/Dependencies/SDL/include/SDL_stdinc.h\
            ../../JGE/Dependencies/SDL/include/SDL_surface.h\
            ../../JGE/Dependencies/SDL/include/SDL_syswm.h\
            ../../JGE/Dependencies/SDL/include/SDL_thread.h\
            ../../JGE/Dependencies/SDL/include/SDL_timer.h\
            ../../JGE/Dependencies/SDL/include/SDL_touch.h\
            ../../JGE/Dependencies/SDL/include/SDL_types.h\
            ../../JGE/Dependencies/SDL/include/SDL_version.h\
            ../../JGE/Dependencies/SDL/include/SDL_video.h\
            ../../JGE/Dependencies/SDL/src/core/windows/SDL_windows.h\
            ../../JGE/Dependencies/SDL/src/events/blank_cursor.h\
            ../../JGE/Dependencies/SDL/src/events/default_cursor.h\
            ../../JGE/Dependencies/SDL/src/audio/windx5\directx.h\
            ../../JGE/Dependencies/SDL/src/events/SDL_clipboardevents_c.h\
            ../../JGE/Dependencies/SDL/src/events/SDL_gesture_c.h\
            ../../JGE/Dependencies/SDL/src/events/SDL_touch_c.h\
            ../../JGE/Dependencies/SDL/src/libm/math.h\
            ../../JGE/Dependencies/SDL/src/libm/math_private.h\
            ../../JGE/Dependencies/SDL/src/render/mmx.h\
            ../../JGE/Dependencies/SDL/src/render/opengl\SDL_shaders_gl.h\
            ../../JGE/Dependencies/SDL/src/render/SDL_sysrender.h\
            ../../JGE/Dependencies/SDL/src/render/SDL_yuv_sw_c.h\
            ../../JGE/Dependencies/SDL/src/audio/SDL_audio_c.h\
            ../../JGE/Dependencies/SDL/src/audio/SDL_audiodev_c.h\
            ../../JGE/Dependencies/SDL/src/audio/SDL_audiomem.h\
            ../../JGE/Dependencies/SDL/src/render/software/SDL_blendfillrect.h\
            ../../JGE/Dependencies/SDL/src/render/software/SDL_blendline.h\
            ../../JGE/Dependencies/SDL/src/render/software/SDL_blendpoint.h\
            ../../JGE/Dependencies/SDL/src/render/software/SDL_draw.h\
            ../../JGE/Dependencies/SDL/src/render/software/SDL_drawline.h\
            ../../JGE/Dependencies/SDL/src/render/software/SDL_drawpoint.h\
            ../../JGE/Dependencies/SDL/src/render/software/SDL_render_sw_c.h\
            ../../JGE/Dependencies/SDL/src/video/dummy/SDL_nullframebuffer_c.h\
            ../../JGE/Dependencies/SDL/src/video/SDL_blit.h\
            ../../JGE/Dependencies/SDL/src/video/SDL_blit_auto.h\
            ../../JGE/Dependencies/SDL/src/video/SDL_blit_copy.h\
            ../../JGE/Dependencies/SDL/src/video/SDL_blit_slow.h\
            ../../JGE/Dependencies/SDL/src/video/SDL_shape_internals.h\
            ../../JGE/Dependencies/SDL/src/audio/windib/SDL_dibaudio.h\
            ../../JGE/Dependencies/SDL/src/audio/disk/SDL_diskaudio.h\
            ../../JGE/Dependencies/SDL/src/audio/dummy/SDL_dummyaudio.h\
            ../../JGE/Dependencies/SDL/src/audio/windx5/SDL_dx5audio.h\
            ../../JGE/Dependencies/SDL/src/SDL_error_c.h\
            ../../JGE/Dependencies/SDL/src/events/SDL_events_c.h\
            ../../JGE/Dependencies/SDL/src/SDL_fatal.h\
            ../../JGE/Dependencies/SDL/src/video/SDL_glesfuncs.h\
            ../../JGE/Dependencies/SDL/src/video/SDL_glfuncs.h\
            ../../JGE/Dependencies/SDL/src/joystick/SDL_joystick_c.h\
            ../../JGE/Dependencies/SDL/src/events/SDL_keyboard_c.h\
            ../../JGE/Dependencies/SDL/src/events/SDL_mouse_c.h\
            ../../JGE/Dependencies/SDL/src/video/dummy/SDL_nullevents_c.h\
            ../../JGE/Dependencies/SDL/src/video/dummy/SDL_nullvideo.h\
            ../../JGE/Dependencies/SDL/src/video/SDL_pixels_c.h\
            ../../JGE/Dependencies/SDL/src/video/SDL_rect_c.h\
            ../../JGE/Dependencies/SDL/src/video/SDL_RLEaccel_c.h\
            ../../JGE/Dependencies/SDL/src/video/SDL_stretch_c.h\
            ../../JGE/Dependencies/SDL/src/audio/SDL_sysaudio.h\
            ../../JGE/Dependencies/SDL/src/events/SDL_sysevents.h\
            ../../JGE/Dependencies/SDL/src/haptic/SDL_syshaptic.h\
            ../../JGE/Dependencies/SDL/src/joystick/SDL_sysjoystick.h\
            ../../JGE/Dependencies/SDL/src/thread/SDL_systhread.h\
            ../../JGE/Dependencies/SDL/src/thread/windows\SDL_systhread_c.h\
            ../../JGE/Dependencies/SDL/src/timer/SDL_systimer.h\
            ../../JGE/Dependencies/SDL/src/video/SDL_sysvideo.h\
            ../../JGE/Dependencies/SDL/src/thread/SDL_thread_c.h\
            ../../JGE/Dependencies/SDL/src/timer/SDL_timer_c.h\
            ../../JGE/Dependencies/SDL/src/video/windows/SDL_vkeys.h\
            ../../JGE/Dependencies/SDL/src/audio/SDL_wave.h\
            ../../JGE/Dependencies/SDL/src/video/windows/SDL_windowsclipboard.h\
            ../../JGE/Dependencies/SDL/src/video/windows/SDL_windowsevents.h\
            ../../JGE/Dependencies/SDL/src/video/windows/SDL_windowsframebuffer.h\
            ../../JGE/Dependencies/SDL/src/video/windows/SDL_windowskeyboard.h\
            ../../JGE/Dependencies/SDL/src/video/windows/SDL_windowsmodes.h\
            ../../JGE/Dependencies/SDL/src/video/windows/SDL_windowsmouse.h\
            ../../JGE/Dependencies/SDL/src/video/windows/SDL_windowsopengl.h\
            ../../JGE/Dependencies/SDL/src/video/windows/SDL_windowsshape.h\
            ../../JGE/Dependencies/SDL/src/video/windows/SDL_windowsvideo.h\
            ../../JGE/Dependencies/SDL/src/video/windows/SDL_windowswindow.h\
            ../../JGE/Dependencies/SDL/src/events/SDL_windowevents_c.h\
            ../../JGE/Dependencies/SDL/src/video/windows/wmmsg.h\
            ../../JGE/Dependencies/SDL/VisualC/SDL/resource.h

}
