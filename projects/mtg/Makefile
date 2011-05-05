OBJS = objs/ActionElement.o objs/ActionLayer.o objs/ActionStack.o objs/AIHints.o objs/AIMomirPlayer.o objs/AIPlayer.o objs/AIStats.o objs/AllAbilities.o objs/CardGui.o objs/CardDescriptor.o objs/CardDisplay.o objs/CardEffect.o objs/CardPrimitive.o objs/CardSelector.o objs/CardSelectorSingleton.o objs/Counters.o objs/Credits.o objs/Damage.o objs/DamagerDamaged.o objs/DeckDataWrapper.o objs/DeckEditorMenu.o objs/DeckMenu.o objs/DeckMenuItem.o objs/DeckMetaData.o objs/DeckStats.o objs/DuelLayers.o objs/Effects.o objs/ExtraCost.o objs/GameApp.o objs/GameLauncher.o objs/GameObserver.o objs/GameOptions.o objs/GameState.o objs/GameStateAwards.o objs/GameStateDeckViewer.o objs/GameStateDuel.o  objs/DeckManager.o objs/GameStateMenu.o objs/GameStateOptions.o objs/GameStateShop.o objs/GameStateStory.o objs/GameStateTransitions.o objs/GuiAvatars.o objs/GuiBackground.o objs/GuiCardsController.o objs/GuiCombat.o objs/GuiFrame.o objs/GuiHand.o objs/GuiLayers.o objs/GuiMana.o objs/GuiPhaseBar.o objs/GuiPlay.o objs/GuiStatic.o objs/ManaCost.o objs/ManaCostHybrid.o  objs/MenuItem.o objs/ModRules.o objs/MTGAbility.o objs/MTGCardInstance.o objs/MTGCard.o objs/MTGDeck.o objs/MTGDefinitions.o objs/MTGGamePhase.o objs/MTGGameZones.o objs/MTGPack.o objs/MTGRules.o objs/Navigator.o objs/ObjectAnalytics.o objs/OptionItem.o objs/PhaseRing.o objs/Player.o objs/PlayerData.o  objs/PlayGuiObjectController.o objs/PlayGuiObject.o objs/PlayRestrictions.o objs/Pos.o objs/PrecompiledHeader.o objs/PriceList.o objs/ReplacementEffects.o objs/Rules.o objs/SimpleMenu.o objs/SimpleMenuItem.o objs/SimplePad.o objs/SimplePopup.o objs/StoryFlow.o objs/StyleManager.o objs/Subtypes.o objs/TargetChooser.o objs/TargetsList.o objs/TextScroller.o objs/ThisDescriptor.o objs/Token.o objs/Translate.o objs/TranslateKeys.o objs/Trash.o objs/utils.o objs/WEvent.o objs/WResourceManager.o objs/WCachedResource.o objs/WDataSrc.o objs/WGui.o objs/WFilter.o objs/Tasks.o objs/WFont.o
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
#SIGN_PRX = 1
LIBS = -ljge300 -lhgetools -lfreetype -ljpeg -lgif -lpng -lz -lm -lmikmod -lpsppower -lpspmpeg -lpspaudiocodec -lpspaudiolib -lpspaudio -lpspmp3 -lpspgum -lpspgu -lpsprtc -lstdc++ -lpspfpu

EXTRA_TARGETS = EBOOT.PBP
PSP_EBOOT_TITLE = Wagic, the Homebrew?!
PSP_EBOOT_ICON = icon.png
#PSP_EBOOT_ICON1 = icon1.pmf
PSP_EBOOT_UNKPNG = pic0.png
PSP_EBOOT_PIC1 = pic1.png
INCDIR = ../../JGE/include ../../JGE/include/psp ../../JGE/include/psp/freetype2 ../../JGE/src ../../projects/mtg/include ../../Boost
LIBDIR = ../../JGE/lib/psp
CFLAGS = -O2 -G0 -DPSPFW3XX -DDEVHOOK -DUSE_PRECOMPILED_HEADERS=1 -DPSP
else
OBJS += objs/TestSuiteAI.o
INCDIR = -I../../JGE/include -I../../JGE/src -I/usr/X11/include -I../../projects/mtg/include -I../../Boost
LIBDIR = -L../../JGE/lib/linux -L../../JGE -L/usr/X11/lib -L../../Boost/lib
LIBS = -ljge -lfreetype -ljpeg -lgif -lpng -lz -lm -lstdc++ -lhgetools -lGL -lGLU -lX11 -lboost_thread $(FMOD)
CFLAGS = $(INCDIR) -DLINUX -DUSE_PRECOMPILED_HEADERS=1
ASFLAGS = $(CXXFLAGS)

all: $(DEFAULT_RULE)

endif

CFLAGS := -Wall -W -Werror -Wno-unused $(CFLAGS)
CXXFLAGS += $(CFLAGS)
# -fno-exceptions

LDFLAGS += $(LIBS)

debug: CXXFLAGS += -ggdb3 -D_DEBUG -DDEBUG

log: CXXFLAGS += -DDOLOG


ifeq ($(TARGET_ARCHITECTURE),psp)
include $(PSPSDK)/lib/build.mak

EXTRA_CLEAN += PrecompiledHeader.h.gch ../../projects/mtg/include/PrecompiledHeader.h.gch

log: all
debug: all

3xx:
	@echo Rule 3xx is deprecated. Did you want to use just "make" ?

else



$(TARGET): Makefile.$(TARGET_ARCHITECTURE) $(OBJS) ../../JGE/lib/linux/libjge.a
	$(CXX) -o $(TARGET) $(OBJS) $(LIBS) $(LIBDIR)

linux: $(TARGET)

debug: linux

clean:
	$(RM) PrecompiledHeader.h.gch $(OBJS) Makefile.$(TARGET_ARCHITECTURE)

endif

define compile
$(CXX) -c $(CXXFLAGS) $< -o $@  
endef

PrecompiledHeader.h.gch: ../../projects/mtg/include/PrecompiledHeader.h
	$(compile)

$(OBJS): objs/%.o: src/%.cpp PrecompiledHeader.h.gch
	$(compile)

$(DEPS): deps/%.d: src/%.cpp
	@$(CXX) -MM $(CXXFLAGS) -MQ $(patsubst deps/%.d, objs/%.o, $(@)) -MQ $@ $< > $@

.DEFAULT:
	@echo $@ has been deleted : updating deps.

Makefile.linux:
	$(CXX) -o /dev/null src/testfeatures.c $(LIBDIR) -lfmod-3.75 > /dev/null 2>&1 ; if [ "0" = "$$?" ]; then echo 'FMOD=-lfmod-3.75'; else echo 'FMOD='; fi > $@

-include $(DEPS)
-include Makefile.$(TARGET_ARCHITECTURE)
