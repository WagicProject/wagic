#### libpng
#
LOCAL_PATH      := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE    := libpng
LOCAL_CFLAGS    := \
  -D__ANDROID__ \
  -DUNICODE \

#  -DCHROME_PNG_WRITE_SUPPORT \
#  -DPNG_USER_CONFIG \

LOCAL_SRC_FILES := \
  png.c \
  pngerror.c \
  pnggccrd.c \
  pngget.c \
  pngmem.c \
  pngpread.c \
  pngread.c \
  pngrio.c \
  pngrtran.c \
  pngrutil.c \
  pngset.c \
  pngtrans.c \
  pngvcrd.c \
  pngwio.c \
  pngwrite.c \
  pngwtran.c \
  pngwutil.c \

include $(BUILD_STATIC_LIBRARY)


