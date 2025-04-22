APP_PROJECT_PATH := $(call my-dir)/..
APP_CPPFLAGS += -frtti -fexceptions
APP_ABI := arm64-v8a
APP_PLATFORM := android-21
APP_CFLAGS += -march=armv8.1-a
APP_CPPFLAGS += -D__ARM_FEATURE_LSE=1
#APP_ABI := x86 # mainly for emulators
APP_STL := c++_static
APP_MODULES := libpng libjpeg main SDL

#APP_OPTIM is 'release' by default
APP_OPTIM := release
