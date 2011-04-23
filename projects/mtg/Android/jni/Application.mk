APP_PROJECT_PATH := $(call my-dir)/..
APP_CPPFLAGS += -frtti -fexceptions
APP_ABI := armeabi armeabi-v7a
APP_STL := gnustl_static
APP_MODULES := libpng libjpeg main SDL