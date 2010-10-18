#-------------------------------------------------
#
# Project created by QtCreator 2010-06-30T19:48:30
#
#-------------------------------------------------

QT       += core gui opengl
VERSION = 0.13.0
TARGET = wagic
TEMPLATE = app
windows:DEFINES += WIN32
windows:DEFINES += _CRT_SECURE_NO_WARNINGS
windows:DEFINES += FORCE_GL2
unix:DEFINES += LINUX
DEFINES += QT_CONFIG
maemo5 {
DEFINES += USE_PHONON
QT += phonon
}
windows:INCLUDEPATH += ../../JGE/Dependencies/include
unix:INCLUDEPATH += /usr/include/GL
unix:INCLUDEPATH += /usr/include/freetype2
INCLUDEPATH += ../../JGE/include
INCLUDEPATH += include
OBJECTS_DIR = objs
MOC_DIR = objs
DESTDIR = bin

unix:LIBS += -ljpeg -lgif -lpng12
windows:LIBS += -LD:\Projects\Wagic\JGE\Dependencies\lib -llibjpeg-static-mt-debug -lgiflib -llibpng -lfmodvc

# MGT
SOURCES += \
        src/ActionElement.cpp\
        src/ActionLayer.cpp\
        src/ActionStack.cpp\
        src/AIMomirPlayer.cpp\
        src/AIPlayer.cpp\
        src/AIStats.cpp\
        src/CardDescriptor.cpp\
        src/CardDisplay.cpp\
        src/CardEffect.cpp\
        src/CardGui.cpp\
        src/CardPrimitive.cpp\
        src/CardSelector.cpp\
        src/Closest.cpp\
        src/Counters.cpp\
        src/Credits.cpp\
        src/Damage.cpp\
        src/DamagerDamaged.cpp\
        src/DeckDataWrapper.cpp\
        src/DeckMetaData.cpp\
        src/DeckManager.cpp\
        src/DeckStats.cpp\
        src/DuelLayers.cpp\
        src/Effects.cpp\
        src/ExtraCost.cpp\
        src/GameApp.cpp\
        src/GameLauncher.cpp\
        src/GameObserver.cpp\
        src/GameOptions.cpp\
        src/GameStateAwards.cpp\
        src/GameState.cpp\
        src/GameStateDeckViewer.cpp\
        src/GameStateDuel.cpp\
        src/GameStateMenu.cpp\
        src/GameStateOptions.cpp\
        src/GameStateShop.cpp\
        src/GameStateStory.cpp\
        src/GameStateTransitions.cpp\
        src/GuiAvatars.cpp\
        src/GuiBackground.cpp\
        src/GuiCardsController.cpp\
        src/GuiCombat.cpp\
        src/GuiFrame.cpp\
        src/GuiHand.cpp\
        src/GuiLayers.cpp\
        src/GuiMana.cpp\
        src/GuiPhaseBar.cpp\
        src/GuiPlay.cpp\
        src/GuiStatic.cpp\
        src/Logger.cpp\
        src/ManaCost.cpp\
        src/ManaCostHybrid.cpp\
        src/MenuItem.cpp\
        src/MTGAbility.cpp\
        src/MTGCard.cpp\
        src/MTGCardInstance.cpp\
        src/MTGDeck.cpp\
        src/MTGDefinitions.cpp\
        src/MTGGamePhase.cpp\
        src/MTGGameZones.cpp\
        src/MTGPack.cpp\
        src/MTGRules.cpp\
        src/OptionItem.cpp\
        src/PhaseRing.cpp\
        src/Player.cpp\
        src/PlayerData.cpp\
        src/PlayGuiObject.cpp\
        src/PlayGuiObjectController.cpp\
        src/Pos.cpp\
        src/PriceList.cpp\
        src/ReplacementEffects.cpp\
        src/Rules.cpp\
        src/SimpleMenu.cpp\
        src/SimpleMenuItem.cpp\
        src/SimplePad.cpp\
        src/StoryFlow.cpp\
        src/Subtypes.cpp\
        src/StyleManager.cpp\
        src/TargetChooser.cpp\
        src/TargetsList.cpp\
        src/Tasks.cpp\
        src/TestSuiteAI.cpp\
        src/TextScroller.cpp\
        src/ThisDescriptor.cpp\
        src/Token.cpp\
        src/Translate.cpp\
        src/TranslateKeys.cpp\
        src/Trash.cpp\
        src/utils.cpp\
        src/WCachedResource.cpp\
        src/WDataSrc.cpp\
        src/WEvent.cpp\
        src/WFilter.cpp\
        src/WFont.cpp\
        src/WGui.cpp\
        src/WResourceManager.cpp

HEADERS  += \
        include/ExtraCost.h\
        include/ManaCost.h\
        include/SimpleMenuItem.h\
        include/GameApp.h\
        include/ManaCostHybrid.h\
        include/SimplePad.h\
        include/ActionElement.h\
        include/GameObserver.h\
        include/MenuItem.h\
        include/StoryFlow.h\
        include/ActionLayer.h\
        include/GameOptions.h\
        include/MTGAbility.h\
        include/Subtypes.h\
        include/ActionStack.h\
        include/GameStateAwards.h\
        include/MTGCard.h\
        include/AIMomirPlayer.h\
        include/GameStateDeckViewer.h\
        include/MTGCardInstance.h\
        include/Targetable.h\
        include/AIPlayer.h\
        include/GameStateDuel.h\
        include/MTGDeck.h\
        include/TargetChooser.h\
        include/AIStats.h\
        include/GameState.h\
        include/MTGDefinitions.h\
        include/TargetsList.h\
        include/AllAbilities.h\
        include/GameStateMenu.h\
        include/MTGGamePhase.h\
        include/Tasks.h\
        include/CardDescriptor.h\
        include/GameStateOptions.h\
        include/MTGGameZones.h\
        include/TestSuiteAI.h\
        include/CardDisplay.h\
        include/GameStateShop.h\
        include/MTGPack.h\
        include/TextScroller.h\
        include/CardEffect.h\
        include/GameStateStory.h\
        include/MTGRules.h\
        include/ThisDescriptor.h\
        include/CardGui.h\
        include/GameStateTransitions.h\
        include/OptionItem.h\
        include/Token.h\
        include/CardPrimitive.h\
        include/GuiAvatars.h\
        include/OSD.h\
        include/Translate.h\
        include/CardSelector.h\
        include/GuiBackground.h\
        include/PhaseRing.h\
        include/TranslateKeys.h\
        include/config.h\
        include/GuiCardsController.h\
        include/PlayerData.h\
        include/Trash.h\
        include/Counters.h\
        include/GuiCombat.h\
        include/Player.h\
        include/utils.h\
        include/Credits.h\
        include/GuiFrame.h\
        include/PlayGuiObjectController.h\
        include/WCachedResource.h\
        include/Damage.h\
        include/GuiHand.h\
        include/PlayGuiObject.h\
        include/WDataSrc.h\
        include/DamagerDamaged.h\
        include/GuiLayers.h\
        include/Pos.h\
        include/WEvent.h\
        include/DeckDataWrapper.h\
        include/GuiMana.h\
        include/PriceList.h\
        include/WFilter.h\
        include/DeckMetaData.h\
        include/GuiPhaseBar.h\
        include/ReplacementEffects.h\
        include/WGui.h\
        include/DeckStats.h\
        include/GuiPlay.h\
        include/Rules.h\
        include/WResourceManager.h\
        include/DuelLayers.h\
        include/GuiStatic.h\
        include/ShopItem.h\
        include/Effects.h\
        include/Logger.h\
        include/StyleManager.h\
        include/WFont.h\
        include/DeckManager.h\
        include/SimpleMenu.h

# JGE, could probably be moved outside
SOURCES += \
        ../../JGE/src/Qtmain.cpp\
        ../../JGE/src/Encoding.cpp\
        ../../JGE/src/JAnimator.cpp\
        ../../JGE/src/JApp.cpp\
        ../../JGE/src/JDistortionMesh.cpp\
        ../../JGE/src/JFileSystem.cpp\
        ../../JGE/src/JGameObject.cpp\
        ../../JGE/src/JGBKFont.cpp\
        ../../JGE/src/JGE.cpp\
        ../../JGE/src/JGui.cpp\
        ../../JGE/src/JLBFont.cpp\
        ../../JGE/src/JLogger.cpp\
        ../../JGE/src/JMD2Model.cpp\
        ../../JGE/src/JOBJModel.cpp\
        ../../JGE/src/JParticle.cpp\
        ../../JGE/src/JParticleEffect.cpp\
        ../../JGE/src/JParticleEmitter.cpp\
        ../../JGE/src/JParticleSystem.cpp\
        ../../JGE/src/JResourceManager.cpp\
        ../../JGE/src/JSpline.cpp\
        ../../JGE/src/JSprite.cpp\
        ../../JGE/src/JTTFont.cpp\
        ../../JGE/src/Vector2D.cpp\
        ../../JGE/src/tinyxml/tinystr.cpp\
        ../../JGE/src/tinyxml/tinyxml.cpp\
        ../../JGE/src/tinyxml/tinyxmlerror.cpp\
        ../../JGE/src/tinyxml/tinyxmlparser.cpp\
        ../../JGE/src/hge/hgecolor.cpp\
        ../../JGE/src/hge/hgedistort.cpp\
        ../../JGE/src/hge/hgefont.cpp\
        ../../JGE/src/hge/hgeparticle.cpp\
        ../../JGE/src/hge/hgerect.cpp\
        ../../JGE/src/hge/hgevector.cpp\
        ../../JGE/src/unzip/ioapi.c\
        ../../JGE/src/unzip/mztools.c\
        ../../JGE/src/unzip/unzip.c\
        ../../JGE/src/pc/JSfx.cpp\
        ../../JGE/src/pc/JGfx.cpp

HEADERS += \
        ../../JGE/include/decoder_prx.h\
        ../../JGE/include/Encoding.h\
        ../../JGE/include/JAnimator.h\
        ../../JGE/include/JApp.h\
        ../../JGE/include/JAssert.h\
        ../../JGE/include/JCooleyesMP3.h\
        ../../JGE/include/JDistortionMesh.h\
        ../../JGE/include/JFileSystem.h\
        ../../JGE/include/JGameLauncher.h\
        ../../JGE/include/JGameObject.h\
        ../../JGE/include/JGBKFont.h\
        ../../JGE/include/JGE.h\
        ../../JGE/include/JGui.h\
        ../../JGE/include/JLBFont.h\
        ../../JGE/include/JLogger.h\
        ../../JGE/include/JMD2Model.h\
        ../../JGE/include/JMP3.h\
        ../../JGE/include/JNetwork.h\
        ../../JGE/include/JOBJModel.h\
        ../../JGE/include/JParticleEffect.h\
        ../../JGE/include/JParticleEmitter.h\
        ../../JGE/include/JParticle.h\
        ../../JGE/include/JParticleSystem.h\
        ../../JGE/include/JRenderer.h\
        ../../JGE/include/JResourceManager.h\
        ../../JGE/include/JSocket.h\
        ../../JGE/include/JSoundSystem.h\
        ../../JGE/include/JSpline.h\
        ../../JGE/include/JSprite.h\
        ../../JGE/include/JTTFont.h\
        ../../JGE/include/JTypes.h\
        ../../JGE/include/Vector2D.h\
        ../../JGE/include/Vector3D.h\
        ../../JGE/include/vram.h\
        ../../JGE/src/tinyxml\tinystr.h\
        ../../JGE/src/tinyxml\tinyxml.h\
        ../../JGE/include/vram.h

maemo5: {
    # Variables
    BINDIR = /opt/wagic
    RESDIR = /opt/wagic/ResInst
    ICONDIR = /usr/share
    DEFINES += RESDIR=\\\"$$RESDIR\\\"

    INSTALLS += target \
        desktop \
        icon \
        res \

    target.path = $$BINDIR

    desktop.path = $$ICONDIR/applications/hildon
    desktop.files += wagic.desktop

    icon.path = $$ICONDIR/icons/hicolor/64x64/apps
    icon.files += wagic-64x64.png

    res.path = $$RESDIR
    res.files += bin/Res/*
    # res.extra = tar -C ../../../../src/projects/mtg/bin -czf Res.tgz Res
}
