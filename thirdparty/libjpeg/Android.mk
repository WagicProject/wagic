#### libjpeg
#
LOCAL_PATH      := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE    := libjpeg
LOCAL_CFLAGS    := \
  -D__ANDROID__ \
  -DUNICODE \

LOCAL_SRC_FILES := \
  jcapimin.c \
  jcapistd.c \
  jccoefct.c \
  jccolor.c \
  jcdctmgr.c \
  jchuff.c \
  jcinit.c \
  jcmainct.c \
  jcmarker.c \
  jcmaster.c \
  jcomapi.c \
  jcparam.c \
  jcphuff.c \
  jcprepct.c \
  jcsample.c \
  jdapimin.c \
  jdapistd.c \
  jdatadst.c \
  jdatasrc.c \
  jdcoefct.c \
  jdcolor.c \
  jddctmgr.c \
  jdhuff.c \
  jdinput.c \
  jdmainct.c \
  jdmarker.c \
  jdmaster.c \
  jdmerge.c \
  jdphuff.c \
  jdpostct.c \
  jdsample.c \
  jerror.c \
  jfdctflt.c \
  jfdctfst.c \
  jfdctint.c \
  jidctflt.c \
  jidctfst.c \
  jidctint.c \
  jmemmgr.c \
  jmemnobs.c \
  jquant1.c \
  jquant2.c \
  jutils.c \

include $(BUILD_STATIC_LIBRARY)

