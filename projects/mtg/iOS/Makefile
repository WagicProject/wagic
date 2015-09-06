export ARCHS = armv7 armv7s
export TARGET = iphone:clang:latest:8.0

include ../../../theos/makefiles/common.mk

APPLICATION_NAME = wagic
wagic_FILES = ../../../JGE/src/iOS/main.m\
    ../../../JGE/src/iOS/wagicAppDelegate.m\
    ../../../JGE/src/iOS/ES2Renderer.m\
    ../../../JGE/src/iOS/EAGLView.m\
    ../../../JGE/src/iOS/EAGLViewController.m\
    ../../../JGE/src/iOS/JSfx.cpp\
    ../../../JGE/src/pc/JGfx.cpp\
    ../iOS/UI/WagicDownloadProgressViewController.m\
    ../iOS/asi-http-request/ASIAuthenticationDialog.m\
    ../iOS/asi-http-request/ASIDataCompressor.m\
    ../iOS/asi-http-request/ASIDataDecompressor.m\
    ../iOS/asi-http-request/ASIDownloadCache.m\
    ../iOS/asi-http-request/ASIFormDataRequest.m\
    ../iOS/asi-http-request/ASIHTTPRequest.m\
    ../iOS/asi-http-request/ASIInputStream.m\
    ../iOS/asi-http-request/ASINetworkQueue.m\
    ../iOS/SoundManager/SoundManager.m\
    ../iOS/SoundManager/MyOpenALSupport.c\
    ../iOS/Reachability/Reachability/Reachability.m\
    ../../../JGE/src/unzip/unzip.c\
    ../../../JGE/src/unzip/zip.c\
    ../../../JGE/src/unzip/ioapi.c\
    ../iOS/ZipArchive/ZipArchive.mm\
    ../src/SimpleMenu.cpp\
    ../src/AbilityParser.cpp\
    ../src/ActionElement.cpp\
    ../src/ActionLayer.cpp\
    ../src/ActionStack.cpp\
    ../src/AIHints.cpp\
    ../src/AIMomirPlayer.cpp\
    ../src/AIPlayer.cpp\
    ../src/AIPlayerBaka.cpp\
    ../src/AIStats.cpp\
    ../src/AllAbilities.cpp\
    ../src/CardDescriptor.cpp\
    ../src/CardDisplay.cpp\
    ../src/CardGui.cpp\
    ../src/CardPrimitive.cpp\
    ../src/CardSelector.cpp\
    ../src/CarouselDeckView.cpp\
    ../src/Closest.cpp\
    ../src/Counters.cpp\
    ../src/Credits.cpp\
    ../src/Damage.cpp\
    ../src/DamagerDamaged.cpp\
    ../src/DeckDataWrapper.cpp\
    ../src/DeckEditorMenu.cpp\
    ../src/DeckManager.cpp\
    ../src/DeckMenu.cpp\
    ../src/DeckMenuItem.cpp\
    ../src/DeckMetaData.cpp\
    ../src/DeckStats.cpp\
    ../src/DeckView.cpp\
    ../src/DuelLayers.cpp\
    ../src/ExtraCost.cpp\
    ../src/GameApp.cpp\
    ../src/GameLauncher.cpp\
    ../src/GameObserver.cpp\
    ../src/GameOptions.cpp\
    ../src/GameStateAwards.cpp\
    ../src/GameState.cpp\
    ../src/GameStateDeckViewer.cpp\
    ../src/GameStateDuel.cpp\
    ../src/GameStateMenu.cpp\
    ../src/GameStateOptions.cpp\
    ../src/GameStateShop.cpp\
    ../src/GameStateStory.cpp\
    ../src/GameStateTransitions.cpp\
    ../src/GridDeckView.cpp\
    ../src/GuiAvatars.cpp\
    ../src/GuiBackground.cpp\
    ../src/GuiCardsController.cpp\
    ../src/GuiCombat.cpp\
    ../src/GuiFrame.cpp\
    ../src/GuiHand.cpp\
    ../src/GuiLayers.cpp\
    ../src/GuiMana.cpp\
    ../src/GuiPhaseBar.cpp\
    ../src/GuiPlay.cpp\
    ../src/GuiStatic.cpp\
    ../src/IconButton.cpp\
    ../src/InteractiveButton.cpp\
    ../src/ManaCost.cpp\
    ../src/ManaCostHybrid.cpp\
    ../src/MenuItem.cpp\
    ../src/ModRules.cpp\
    ../src/MTGAbility.cpp\
    ../src/MTGCard.cpp\
    ../src/MTGCardInstance.cpp\
    ../src/MTGDeck.cpp\
    ../src/MTGDefinitions.cpp\
    ../src/MTGGamePhase.cpp\
    ../src/MTGGameZones.cpp\
    ../src/MTGPack.cpp\
    ../src/MTGRules.cpp\
    ../src/ObjectAnalytics.cpp\
    ../src/OptionItem.cpp\
    ../src/PhaseRing.cpp\
    ../src/Player.cpp\
    ../src/PlayerData.cpp\
    ../src/PlayGuiObject.cpp\
    ../src/PlayGuiObjectController.cpp\
    ../src/PlayRestrictions.cpp\
    ../src/Pos.cpp\
    ../src/PriceList.cpp\
    ../src/ReplacementEffects.cpp\
    ../src/Rules.cpp\
    ../src/SimpleButton.cpp\
    ../src/SimpleMenu.cpp\
    ../src/SimpleMenuItem.cpp\
    ../src/SimplePad.cpp\
    ../src/SimplePopup.cpp\
    ../src/StoryFlow.cpp\
    ../src/Subtypes.cpp\
    ../src/StyleManager.cpp\
    ../src/TargetChooser.cpp\
    ../src/TargetsList.cpp\
    ../src/Tasks.cpp\
    ../src/TextScroller.cpp\
    ../src/ThisDescriptor.cpp\
    ../src/Token.cpp\
    ../src/Translate.cpp\
    ../src/TranslateKeys.cpp\
    ../src/Trash.cpp\
    ../src/utils.cpp\
    ../src/WCachedResource.cpp\
    ../src/WDataSrc.cpp\
    ../src/WEvent.cpp\
    ../src/WFilter.cpp\
    ../src/WFont.cpp\
    ../src/WGui.cpp\
    ../src/WResourceManager.cpp \
    ../src/AIPlayerBakaB.cpp \
    ../src/TestSuiteAI.cpp\
    ../../../JGE/src/Encoding.cpp\
    ../../../JGE/src/JAnimator.cpp\
    ../../../JGE/src/JApp.cpp\
    ../../../JGE/src/JDistortionMesh.cpp\
    ../../../JGE/src/JFileSystem.cpp\
    ../../../JGE/src/JGameObject.cpp\
    ../../../JGE/src/JGE.cpp\
    ../../../JGE/src/JGui.cpp\
    ../../../JGE/src/JLogger.cpp\
    ../../../JGE/src/JLBFont.cpp\
    ../../../JGE/src/JOBJModel.cpp\
    ../../../JGE/src/JParticle.cpp\
    ../../../JGE/src/JParticleEffect.cpp\
    ../../../JGE/src/JParticleEmitter.cpp\
    ../../../JGE/src/JParticleSystem.cpp\
    ../../../JGE/src/JResourceManager.cpp\
    ../../../JGE/src/JSpline.cpp\
    ../../../JGE/src/JSprite.cpp\
    ../../../JGE/src/Vector2D.cpp\
    ../../../JGE/src/tinyxml/tinystr.cpp\
    ../../../JGE/src/tinyxml/tinyxml.cpp\
    ../../../JGE/src/tinyxml/tinyxmlerror.cpp\
    ../../../JGE/src/tinyxml/tinyxmlparser.cpp\
    ../../../JGE/src/hge/hgecolor.cpp\
    ../../../JGE/src/hge/hgedistort.cpp\
    ../../../JGE/src/hge/hgefont.cpp\
    ../../../JGE/src/hge/hgeparticle.cpp\
    ../../../JGE/src/hge/hgerect.cpp\
    ../../../JGE/src/hge/hgevector.cpp\
    ../../../JGE/src/zipFS/zfsystem.cpp\
    ../../../JGE/src/zipFS/ziphdr.cpp\
    ../../../JGE/src/zipFS/zstream.cpp\
    ../../../Boost/lib/pthread/once.cpp\
    ../../../Boost/lib/pthread/thread.cpp\


wagic_LDFLAGS = -lz
wagic_FRAMEWORKS = UIKit CoreGraphics OpenGLES Foundation CFNetwork MobileCoreServices AVFoundation OpenAL AudioToolbox QuartzCore SystemConfiguration
include ../../../theos/makefiles/application.mk

ADDITIONAL_CFLAGS = -I ../include\
    -I ../../../JGE/include\
    -I ../../../JGE/src/unzip\
    -I ../../../JGE/src/zipFS\
    -I ../../../JGE/Dependencies/include\
    -I ../../../Boost\
    -I ../../../JGE/src/iOS\
    -I ../iOS/UI\
    -I ../iOS/Reachability/Reachability\
    -I ../iOS/asi-http-request\
    -I ../iOS/ZipArchive\
    -I ../iOS/SoundManager\
    -DIOS -D__arm__ -DTIXML_USE_STL -DVERSION=\"$(GIT_VERSION)\"\
    -Wno-parentheses-equality -Wno-delete-non-virtual-dtor\
    -Wno-tautological-undefined-compare -Wno-undefined-bool-conversion\
    -Wno-visibility -Wno-deprecated-declarations -Wno-non-literal-null-conversion\
    -Wno-format -Wno-distributed-object-modifiers -Wno-missing-braces\
    -Wno-unused-const-variable -Wno-unused-function -Wno-unknown-warning-option\
    -x objective-c++

_THEOS_TARGET_ONLY_OBJCFLAGS :=""
