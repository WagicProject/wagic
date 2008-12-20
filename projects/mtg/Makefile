TARGET=template
OBJS = src/ActionElement.o src/ActionLayer.o src/ActionStack.o src/AIPlayer.o src/AIStats.o src/Blocker.o src/CardGui.o src/CardDescriptor.o src/CardDisplay.o  src/ConstraintResolver.o src/Counters.o src/Damage.o src/DamagerDamaged.o src/DamageResolverLayer.o src/DeckDataWrapper.o src/DeckStats.o src/DuelLayers.o src/GameApp.o src/GameLauncher.o src/GameObserver.o src/GameOptions.o src/GameStateDuel.o src/GameStateOptions.o src/GameStateShop.o src/GuiCardsController.o src/GuiLayers.o src/Logger.o src/ManaCost.o src/ManaCostHybrid.o  src/MenuItem.o src/MTGAbility.o src/MTGCardInstance.o src/MTGCard.o src/MTGDeck.o src/MTGGamePhase.o src/MTGGameZones.o src/MTGGuiHand.o src/MTGGuiPlay.o src/MTGRules.o src/OptionItem.o src/PhaseRing.o src/Player.o src/PlayerData.o  src/PlayGuiObjectController.o src/PlayGuiObject.o src/PriceList.o src/ShopItem.o  src/SimpleMenu.o src/SimpleMenuItem.o src/Subtypes.o src/TargetChooser.o src/TargetsList.o src/TexturesCache.o src/Token.o src/utils.o

RESULT = $(shell psp-config --psp-prefix 2> Makefile.cache)
ifeq ($(RESULT),)
DEFAULT_RULE = linux
TARGET_ARCHITECTURE = linux
else
DEFAULT_RULE = 3xx
TARGET_ARCHITECTURE = psp
PSPSDK = $(shell psp-config --pspsdk-path)
PSPDIR = $(shell psp-config --psp-prefix)
endif
ifeq ($(MAKECMDGOALS),linux)
DEFAULT_RULE = linux
TARGET_ARCHITECTURE = linux
endif
ifeq ($(MAKECMDGOALS),3xx)
DEFAULT_RULE = 3xx
TARGET_ARCHITECTURE = psp
endif
ifeq ($(MAKECMDGOALS),1xx)
DEFAULT_RULE = 1xx
TARGET_ARCHITECTURE = psp
endif

ifeq ($(TARGET_ARCHITECTURE),psp)
LDFLAGS =
EXTRA_TARGETS = EBOOT.PBP
PSP_EBOOT_TITLE = WTH?!
PSP_EBOOT_ICON = icon.png
CXXFLAGS = -O2 -G0 -Wall -DDEVHOOK -DPSPFW3XX -fno-exceptions -fno-rtti
else
OBJS += src/TestSuiteAI.o
endif

CXXFLAGS := $(INCDIR) $(CXXFLAGS)
LDFLAGS := $(LIBS)


all: $(DEFAULT_RULE)

debug: CXXFLAGS += -ggdb3 -D_DEBUG -DDEBUG

linux: INCDIR = -I ../../JGE/include -I ../../JGE/src
linux: LIBDIR = -L ../../JGE/lib/linux -L ../../JGE
linux: LIBS = -ljge -lfreetype -ljpeg -lgif -lpng -lz -lm -lstdc++ -lglut -lhgetools -lfmod-3.75
linux: CFLAGS = $(INCDIR) -O2 -Wall -W -Werror -Wno-unused -DDEVHOOK -DLINUX
linux: CXXFLAGS += $(CFLAGS) -fno-exceptions -fno-rtti
linux: ASFLAGS = $(CXXFLAGS)

3xx: BUILD_PRX = 1
3xx: PSP_PW_VERSION=371
3xx: LIBS = -ljge300 -lhgetools -lfreetype -ljpeg -lgif -lpng -lz -lm -lmikmod -lpsppower -lpspmpeg -lpspaudiocodec -lpspaudiolib -lpspaudio -lpspmp3 -lpspgum -lpspgu -lpsprtc -lstdc++ -lpspfpu
3xx: INCDIR = ../../JGE/include ../../JGE/include/psp ../../JGE/include/psp/freetype2 ../../JGE/src
3xx: LIBDIR = ../../JGE/lib/psp

1xx: LIBS = -ljge100 -lfreetype -lpng -lz -lm -lmikmod -lmmio -lpsppower -lpspaudiolib -lpspaudio -lpspgum -lpspgu -lpsprtc -lstdc++ -ljpeg -lgif -lpspaudiocodec -lpspmpeg
1xx: INCDIR = ../../JGE/include ../../JGE/include/psp ../../JGE/include/psp/freetype2 ../../JGE/src
1xx: LIBDIR = ../../JGE/lib/psp

ifeq ($(TARGET_ARCHITECTURE),psp)
include $(PSPSDK)/lib/build.mak
else
linux: TARGET = bin/wagic



$(TARGET): $(OBJS)
	g++ -o $(TARGET) $(OBJS) $(LIBS) $(LIBDIR)

linux: $(TARGET)

debug: linux

clean:
	rm -f $(OBJS)

endif

