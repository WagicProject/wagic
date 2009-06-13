OBJS = objs/ActionElement.o objs/ActionLayer.o objs/ActionStack.o objs/AIMomirPlayer.o objs/AIPlayer.o objs/AIStats.o objs/Blocker.o objs/CardGui.o objs/CardDescriptor.o objs/CardDisplay.o objs/CardEffect.o objs/ConstraintResolver.o objs/Counters.o objs/Credits.o objs/Damage.o objs/DamagerDamaged.o objs/DamageResolverLayer.o objs/DeckDataWrapper.o objs/DeckStats.o objs/DuelLayers.o objs/Effects.o objs/ExtraCost.o objs/GameApp.o objs/GameLauncher.o objs/GameObserver.o objs/GameOptions.o objs/GameStateDuel.o objs/GameStateMenu.o objs/GameStateOptions.o objs/GameStateShop.o objs/GuiCardsController.o objs/GuiLayers.o objs/Logger.o objs/ManaCost.o objs/ManaCostHybrid.o  objs/MenuItem.o objs/MTGAbility.o objs/MTGCardInstance.o objs/MTGCard.o objs/MTGDeck.o objs/MTGDefinitions.o objs/MTGGamePhase.o objs/MTGGameZones.o objs/MTGGuiHand.o objs/MTGGuiPlay.o objs/MTGRules.o objs/OptionItem.o objs/PhaseRing.o objs/Player.o objs/PlayerData.o  objs/PlayGuiObjectController.o objs/PlayGuiObject.o objs/PriceList.o objs/ShopItem.o  objs/SimpleMenu.o objs/SimpleMenuItem.o objs/Subtypes.o objs/TargetChooser.o objs/TargetsList.o objs/TextScroller.o objs/TexturesCache.o objs/Token.o objs/Translate.o objs/utils.o objs/WEvent.o
DEPS = $(patsubst objs/%.o, deps/%.d, $(OBJS))

RESULT = $(shell psp-config --psp-prefix 2> Makefile.cache)
ifeq ($(RESULT),)
DEFAULT_RULE = linux
TARGET_ARCHITECTURE = linux
TARGET = bin/wagic
else
DEFAULT_RULE = 3xx
TARGET_ARCHITECTURE = psp
PSPSDK = $(shell psp-config --pspsdk-path)
PSPDIR = $(shell psp-config --psp-prefix)
TARGET = wagic
endif
ifeq ($(MAKECMDGOALS),debug)
DEFAULT_RULE = debug
TARGET_ARCHITECTURE = linux
endif
ifeq ($(MAKECMDGOALS),linux)
DEFAULT_RULE = linux
TARGET_ARCHITECTURE = linux
endif

ifeq ($(TARGET_ARCHITECTURE),psp)
DEFAULT_RULE = 3xx
TARGET_ARCHITECTURE = psp
PSP_FW_VERSION=371
BUILD_PRX = 1
LIBS = -ljge300 -lhgetools -lfreetype -ljpeg -lgif -lpng -lz -lm -lmikmod -lpsppower -lpspmpeg -lpspaudiocodec -lpspaudiolib -lpspaudio -lpspmp3 -lpspgum -lpspgu -lpsprtc -lstdc++ -lpspfpu

EXTRA_TARGETS = EBOOT.PBP
PSP_EBOOT_TITLE = WTH?!
PSP_EBOOT_ICON = icon.png
CXXFLAGS = -O2 -G0 -Wall -DDEVHOOK -DPSPFW3XX -fno-exceptions
INCDIR = ../../JGE/include ../../JGE/include/psp ../../JGE/include/psp/freetype2 ../../JGE/src
LIBDIR = ../../JGE/lib/psp
else
OBJS += objs/TestSuiteAI.o
INCDIR = -I ../../JGE/include -I ../../JGE/src
LIBDIR = -L ../../JGE/lib/linux -L ../../JGE
LIBS = -ljge -lfreetype -ljpeg -lgif -lpng -lz -lm -lstdc++ -lglut -lhgetools -lfmod-3.75
CFLAGS = $(INCDIR) -O2 -Wall -W -Werror -Wno-unused -DDEVHOOK -DLINUX
CXXFLAGS += $(CFLAGS) -fno-exceptions
ASFLAGS = $(CXXFLAGS)

all: $(DEFAULT_RULE)

endif

LDFLAGS = $(LIBS)



debug: CXXFLAGS = -Wall -W -Werror -Wno-unused -DDEVHOOK -DPSPFW3XX -fno-exceptions -ggdb3 -D_DEBUG -DDEBUG -DLINUX $(INCDIR)




ifeq ($(TARGET_ARCHITECTURE),psp)
include $(PSPSDK)/lib/build.mak

3xx:
	@echo Rule 3xx is deprecated. Did you want to use just "make" ?

else



$(TARGET): $(OBJS) ../../JGE/lib/linux/libjge.a
	g++ -o $(TARGET) $(OBJS) $(LIBS) $(LIBDIR)

linux: $(TARGET)

debug: linux

clean:
	$(RM) $(OBJS)

endif

$(OBJS): objs/%.o: src/%.cpp
	$(CXX) -c -o $@ $(CXXFLAGS) $<

$(DEPS): deps/%.d: src/%.cpp
	@$(CXX) -MM $(CXXFLAGS) -MQ $(patsubst deps/%.d, objs/%.o, $(@)) -MQ $@ $< > $@

-include $(DEPS)
