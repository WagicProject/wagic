ifeq ($(MAKECMDGOALS),debug)
DEBUG = true
endif


GENERIC_OBJS = src/JApp.o src/JGBKFont.o \
	src/JGE.o src/JGui.o src/JLBFont.o \
	src/JGameObject.o src/JSpline.o src/JAnimator.o \
	src/JResourceManager.o src/JFileSystem.o \
	src/JNetwork.o \
	src/JParticle.o src/JParticleEmitter.o src/JParticleEffect.o \
	src/JParticleSystem.o \
	src/unzip/ioapi.o src/unzip/mztools.o src/unzip/unzip.o \
	src/JSprite.o src/Vector2D.o \
	src/tinyxml/tinystr.o src/tinyxml/tinyxml.o \
	src/tinyxml/tinyxmlparser.o src/tinyxml/tinyxmlerror.o \
	src/Encoding.o src/JTTFont.o \
	src/JMD2Model.o src/JOBJModel.o
PSP_OBJS = src/JSocket.o src/JGfx.o src/JSfx.o src/JAudio.o src/JMP3.o src/decoder_prx.o src/main.o src/vram.o
LINUX_OBJS = src/linux/JGfx.o src/linux/JSfx.o src/Xmain.o


HGE_OBJS = src/hge/hgecolor.o src/hge/hgeparticle.o \
       src/hge/hgerect.o src/hge/hgevector.o \
       src/hge/hgedistort.o src/hge/hgefont.o


CXXFLAGS = -W -Wall -Werror -Wno-unused

ifdef DEBUG
CXXFLAGS += -ggdb3
endif

# Determination of target.
# TARGET_ARCHITECTURE variable will then be set to either linux or psp.

RESULT = $(shell psp-config --psp-prefix 2> Makefile.cache)
ifeq ($(RESULT),)
DEFAULT_RULE = linux
TARGET_ARCHITECTURE = linux
else
DEFAULT_RULE = 3xx
TARGET_ARCHITECTURE = psp
TARGET_LIB = libjge300.a
endif

ifeq ($(MAKECMDGOALS),linux)
DEFAULT_RULE = linux
TARGET_ARCHITECTURE = linux
endif
ifeq ($(MAKECMDGOALS),3xx)
DEFAULT_RULE = 3xx
endif
ifeq ($(MAKECMDGOALS),1xx)
DEFAULT_RULE = 1xx
endif

ifeq ($(DEFAULT_RULE),3xx)
TARGET_ARCHITECTURE = psp
TARGET_LIB = libjge300.a
CXXFLAGS += -DDEVHOOK -DPSPFW3XX
PSP_FW_VERSION=371
endif
ifeq ($(DEFAULT_RULE),1xx)
OBJS = $(GENERIC_OBJS) $(PSP_OBJS)
TARGET_LIB = libjge100.a
TARGET_ARCHITECTURE = psp
PSP_FW_VERSION=150
endif

ifeq ($(TARGET_ARCHITECTURE),psp)
PSPSDK = $(shell psp-config --pspsdk-path)
PSPDIR = $(shell psp-config --psp-prefix)
OBJS = $(GENERIC_OBJS) $(PSP_OBJS)
TARGET_HGE = libhgetools.a
INCDIR = include/psp include/psp/freetype2
CXXFLAGS += -O2 -G0
LIBDIR = lib/psp
endif
ifeq ($(TARGET_ARCHITECTURE),linux)
OBJS = $(GENERIC_OBJS) $(LINUX_OBJS)
TARGET_LIB = libjge.a
TARGET_HGE = libhgetools.a
INCDIR = $(shell freetype-config --cflags 2> /dev/null)
CXXFLAGS += -DLINUX
CXXFLAGS += $(INCDIR)
LIBDIR = lib/linux
endif

# Set definitive values for variables.
TARGET_LIB := $(LIBDIR)/$(TARGET_LIB)
TARGET_HGE := $(LIBDIR)/$(TARGET_HGE)

ifeq ($(TARGET_ARCHITECTURE),psp)
include $(PSPSDK)/lib/build.mak
endif

all: $(DEFAULT_RULE) hge

debug: $(DEFAULT_RULE) hge

linux: $(TARGET_LIB) hge

3xx: $(TARGET_LIB)

1xx: $(TARGET_LIB)

install: $(TARGET_LIB) hge

hge: $(TARGET_HGE)

ifeq ($(TARGET_ARCHITECTURE),linux)
$(TARGET_LIB): $(OBJS)
	ar r $(TARGET_LIB) $(OBJS)

$(TARGET_HGE): $(HGE_OBJS)
	ar r $(TARGET_HGE) $(HGE_OBJS)

clean:
	$(RM) -f $(OBJS) $(HGE_OBJS)
endif
