LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

MY_LOCAL_PATH := $(LOCAL_PATH)
LOCAL_MODULE := main

MY_WAGIC_ROOT := ../../../..
JGE_PATH := $(MY_WAGIC_ROOT)/JGE
MTG_PATH := $(MY_WAGIC_ROOT)/projects/mtg/
SDL_PATH := $(JGE_PATH)/Dependencies/SDL
BOOST_PATH := $(MY_WAGIC_ROOT)/Boost
JPEG_PATH := $(JGE_PATH)/Dependencies/libjpeg
PNG_PATH := $(JGE_PATH)/Dependencies/libpng

LOCAL_CFLAGS += -DLINUX -DANDROID -DSDL_CONFIG -D_DEBUG
LOCAL_CFLAGS += -D_STLP_USE_SIMPLE_NODE_ALLOC
LOCAL_CFLAGS += -D__arm__ -D_REENTRANT -D_GLIBCXX__PTHREADS
LOCAL_STATIC_LIBRARIES := libpng libjpeg
LOCAL_SHARED_LIBRARIES := SDL

LOCAL_C_INCLUDES := \
	$(LOCAL_PATH)/$(SDL_PATH)/include \
	$(LOCAL_PATH)/$(MTG_PATH)/include \
	$(LOCAL_PATH)/$(JGE_PATH)/include \
	$(LOCAL_PATH)/$(JGE_PATH)/src/zipFS \
	$(LOCAL_PATH)/$(BOOST_PATH) \
	$(LOCAL_PATH)/$(JPEG_PATH) \
	$(LOCAL_PATH)/$(PNG_PATH) \

LOCAL_SRC_FILES := $(SDL_PATH)/src/main/android/SDL_android_main.cpp \
        $(MTG_PATH)/src/ActionElement.cpp \
        $(MTG_PATH)/src/ActionLayer.cpp \
        $(MTG_PATH)/src/ActionStack.cpp \
        $(MTG_PATH)/src/AIHints.cpp \
        $(MTG_PATH)/src/AIMomirPlayer.cpp \
        $(MTG_PATH)/src/AIPlayer.cpp \
		$(MTG_PATH)/src/AIPlayerBaka.cpp \
        $(MTG_PATH)/src/AIStats.cpp \
        $(MTG_PATH)/src/AllAbilities.cpp \
        $(MTG_PATH)/src/CardDescriptor.cpp \
        $(MTG_PATH)/src/CardDisplay.cpp \
        $(MTG_PATH)/src/CardEffect.cpp \
        $(MTG_PATH)/src/CardGui.cpp \
        $(MTG_PATH)/src/CardPrimitive.cpp \
        $(MTG_PATH)/src/CardSelector.cpp \
        $(MTG_PATH)/src/CardSelectorSingleton.cpp \
        $(MTG_PATH)/src/Counters.cpp \
        $(MTG_PATH)/src/Credits.cpp \
        $(MTG_PATH)/src/Damage.cpp \
        $(MTG_PATH)/src/DamagerDamaged.cpp \
        $(MTG_PATH)/src/DeckDataWrapper.cpp \
        $(MTG_PATH)/src/DeckEditorMenu.cpp \
        $(MTG_PATH)/src/DeckManager.cpp \
        $(MTG_PATH)/src/DeckMenu.cpp \
        $(MTG_PATH)/src/DeckMenuItem.cpp \
        $(MTG_PATH)/src/DeckMetaData.cpp \
        $(MTG_PATH)/src/DeckStats.cpp \
        $(MTG_PATH)/src/DuelLayers.cpp \
        $(MTG_PATH)/src/Effects.cpp \
        $(MTG_PATH)/src/ExtraCost.cpp \
        $(MTG_PATH)/src/GameApp.cpp \
        $(MTG_PATH)/src/GameLauncher.cpp \
        $(MTG_PATH)/src/GameObserver.cpp \
        $(MTG_PATH)/src/GameOptions.cpp \
        $(MTG_PATH)/src/GameStateAwards.cpp \
        $(MTG_PATH)/src/GameState.cpp \
        $(MTG_PATH)/src/GameStateDeckViewer.cpp \
        $(MTG_PATH)/src/GameStateDuel.cpp \
        $(MTG_PATH)/src/GameStateMenu.cpp \
        $(MTG_PATH)/src/GameStateOptions.cpp \
        $(MTG_PATH)/src/GameStateShop.cpp \
        $(MTG_PATH)/src/GameStateStory.cpp \
        $(MTG_PATH)/src/GameStateTransitions.cpp \
        $(MTG_PATH)/src/GuiAvatars.cpp \
        $(MTG_PATH)/src/GuiBackground.cpp \
        $(MTG_PATH)/src/GuiCardsController.cpp \
        $(MTG_PATH)/src/GuiCombat.cpp \
        $(MTG_PATH)/src/GuiFrame.cpp \
        $(MTG_PATH)/src/GuiHand.cpp \
        $(MTG_PATH)/src/GuiLayers.cpp \
        $(MTG_PATH)/src/GuiMana.cpp \
        $(MTG_PATH)/src/GuiPhaseBar.cpp \
        $(MTG_PATH)/src/GuiPlay.cpp \
        $(MTG_PATH)/src/GuiStatic.cpp \
        $(MTG_PATH)/src/IconButton.cpp \
        $(MTG_PATH)/src/ManaCost.cpp \
        $(MTG_PATH)/src/ManaCostHybrid.cpp \
        $(MTG_PATH)/src/MenuItem.cpp \
        $(MTG_PATH)/src/ModRules.cpp \
        $(MTG_PATH)/src/MTGAbility.cpp \
        $(MTG_PATH)/src/MTGCard.cpp \
        $(MTG_PATH)/src/MTGCardInstance.cpp \
        $(MTG_PATH)/src/MTGDeck.cpp \
        $(MTG_PATH)/src/MTGDefinitions.cpp \
        $(MTG_PATH)/src/MTGGamePhase.cpp \
        $(MTG_PATH)/src/MTGGameZones.cpp \
        $(MTG_PATH)/src/MTGPack.cpp \
        $(MTG_PATH)/src/MTGRules.cpp \
        $(MTG_PATH)/src/ObjectAnalytics.cpp \
        $(MTG_PATH)/src/OptionItem.cpp \
        $(MTG_PATH)/src/PhaseRing.cpp \
        $(MTG_PATH)/src/Player.cpp \
        $(MTG_PATH)/src/PlayerData.cpp \
        $(MTG_PATH)/src/PlayGuiObject.cpp \
        $(MTG_PATH)/src/PlayGuiObjectController.cpp \
        $(MTG_PATH)/src/PlayRestrictions.cpp \
        $(MTG_PATH)/src/Pos.cpp \
        $(MTG_PATH)/src/PriceList.cpp \
        $(MTG_PATH)/src/ReplacementEffects.cpp \
        $(MTG_PATH)/src/Rules.cpp \
        $(MTG_PATH)/src/SimpleMenu.cpp \
        $(MTG_PATH)/src/SimpleMenuItem.cpp \
        $(MTG_PATH)/src/SimplePad.cpp \
        $(MTG_PATH)/src/SimplePopup.cpp \
        $(MTG_PATH)/src/StoryFlow.cpp \
        $(MTG_PATH)/src/Subtypes.cpp \
        $(MTG_PATH)/src/StyleManager.cpp \
        $(MTG_PATH)/src/TargetChooser.cpp \
        $(MTG_PATH)/src/TargetsList.cpp \
        $(MTG_PATH)/src/Tasks.cpp \
        $(MTG_PATH)/src/TestSuiteAI.cpp \
        $(MTG_PATH)/src/TextScroller.cpp \
        $(MTG_PATH)/src/ThisDescriptor.cpp \
        $(MTG_PATH)/src/Token.cpp \
        $(MTG_PATH)/src/Translate.cpp \
        $(MTG_PATH)/src/TranslateKeys.cpp \
        $(MTG_PATH)/src/Trash.cpp \
        $(MTG_PATH)/src/utils.cpp \
        $(MTG_PATH)/src/WCachedResource.cpp \
        $(MTG_PATH)/src/WDataSrc.cpp \
        $(MTG_PATH)/src/WEvent.cpp \
        $(MTG_PATH)/src/WFilter.cpp \
        $(MTG_PATH)/src/WFont.cpp \
        $(MTG_PATH)/src/WGui.cpp \
        $(MTG_PATH)/src/WResourceManager.cpp \
        $(MTG_PATH)/src/NetworkPlayer.cpp \
        $(JGE_PATH)/src/SDLmain.cpp \
        $(JGE_PATH)/src/Encoding.cpp \
        $(JGE_PATH)/src/JAnimator.cpp \
        $(JGE_PATH)/src/JApp.cpp \
        $(JGE_PATH)/src/JDistortionMesh.cpp \
        $(JGE_PATH)/src/JFileSystem.cpp \
        $(JGE_PATH)/src/JGameObject.cpp \
        $(JGE_PATH)/src/JGE.cpp \
        $(JGE_PATH)/src/JGui.cpp \
        $(JGE_PATH)/src/JLogger.cpp \
        $(JGE_PATH)/src/JLBFont.cpp \
        $(JGE_PATH)/src/JMD2Model.cpp \
        $(JGE_PATH)/src/JOBJModel.cpp \
        $(JGE_PATH)/src/JParticle.cpp \
        $(JGE_PATH)/src/JParticleEffect.cpp \
        $(JGE_PATH)/src/JParticleEmitter.cpp \
        $(JGE_PATH)/src/JParticleSystem.cpp \
        $(JGE_PATH)/src/JResourceManager.cpp \
        $(JGE_PATH)/src/JSpline.cpp \
        $(JGE_PATH)/src/JSprite.cpp \
        $(JGE_PATH)/src/Vector2D.cpp \
        $(JGE_PATH)/src/tinyxml/tinystr.cpp \
        $(JGE_PATH)/src/tinyxml/tinyxml.cpp \
        $(JGE_PATH)/src/tinyxml/tinyxmlerror.cpp \
        $(JGE_PATH)/src/tinyxml/tinyxmlparser.cpp \
        $(JGE_PATH)/src/hge/hgecolor.cpp \
        $(JGE_PATH)/src/hge/hgedistort.cpp \
        $(JGE_PATH)/src/hge/hgefont.cpp \
        $(JGE_PATH)/src/hge/hgeparticle.cpp \
        $(JGE_PATH)/src/hge/hgerect.cpp \
        $(JGE_PATH)/src/hge/hgevector.cpp \
        $(JGE_PATH)/src/zipFS/zfsystem.cpp \
        $(JGE_PATH)/src/zipFS/ziphdr.cpp \
        $(JGE_PATH)/src/zipFS/zstream.cpp \
        $(JGE_PATH)/src/android/JSfx.cpp \
        $(JGE_PATH)/src/pc/JGfx.cpp \
        $(JGE_PATH)/src/JNetwork.cpp \
        $(JGE_PATH)/src/pc/JSocket.cpp \
        $(BOOST_PATH)/lib/pthread/thread.cpp \
        $(BOOST_PATH)/lib/pthread/once.cpp

LOCAL_LDLIBS := -lGLESv1_CM -llog -lz -lOpenSLES

include $(BUILD_SHARED_LIBRARY)

# PNG static lib
include $(MY_LOCAL_PATH)/$(PNG_PATH)/Android.mk
include $(CLEAR_VARS)

# JPEG static lib
include $(MY_LOCAL_PATH)/$(JPEG_PATH)/Android.mk
include $(CLEAR_VARS)

# SDL dynamic lib
include $(MY_LOCAL_PATH)/$(SDL_PATH)/Android.mk
include $(CLEAR_VARS)