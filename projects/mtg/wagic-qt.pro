# Add more folders to ship with the application, here
folder_01.source = qml/QmlWagic
folder_01.target = qml
DEPLOYMENTFOLDERS = folder_01

TARGET = wagic

QT += core gui opengl network
!android:!symbian:QT += phonon
maemo5:QT += dbus

TARGET = wagic
TEMPLATE = app

#!macx:CONFIG += precompile_header
unix|macx:QMAKE_CXXFLAGS += -Wno-unused-parameter
unix|macx:!maemo5:!symbian:QMAKE_CXXFLAGS += -Werror
windows:DEFINES += _CRT_SECURE_NO_WARNINGS
unix|macx:DEFINES += LINUX
CONFIG(debug, debug|release):DEFINES += _DEBUG
DEFINES += QT_CONFIG
!android:!symbian:DEFINES += USE_PHONON
android:INCLUDEPATH += $$ANDROID_NDK_ROOT/platforms/android-9/arch-arm/usr/include
#DEFINES += QT_NO_DEBUG_OUTPUT
maemo5:DEFINES += QT_WIDGET

windows:INCLUDEPATH += ../../JGE/Dependencies/include
windows{
    *-g++* {
        DEFINES += LINUX
    }
    *-msvc* {
        INCLUDEPATH += extra
        DEFINES += WIN32
    }
}
unix:!symbian:INCLUDEPATH += /usr/include/GL
macx:INCLUDEPATH += /opt/include
INCLUDEPATH += ../../JGE/include/qt
INCLUDEPATH += ../../JGE/include
INCLUDEPATH += ../../JGE/src/zipFS
INCLUDEPATH += ../../Boost
INCLUDEPATH += include
#!symbian:DESTDIR = bin

unix:!symbian:LIBS += -lz
PRECOMPILED_HEADER = include/PrecompiledHeader.h

#DEFINES += TESTSUITE
#DEFINES += TRACK_OBJECT_USAGE
#DEFINES += AI_CHANGE_TESTING
#DEFINES += ACTION_LOGGING_TESTING

SOURCES += \
        src/AbilityParser.cpp\
        src/ActionElement.cpp\
        src/ActionLayer.cpp\
        src/ActionStack.cpp\
        src/AIHints.cpp\
        src/AIMomirPlayer.cpp\
        src/AIPlayer.cpp\
        src/AIPlayerBaka.cpp\
        src/AIStats.cpp\
        src/AllAbilities.cpp\
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
        src/DeckEditorMenu.cpp\
        src/DeckManager.cpp\
        src/DeckMenu.cpp\
        src/DeckMenuItem.cpp\
        src/DeckMetaData.cpp\
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
        src/IconButton.cpp\
        src/InteractiveButton.cpp\
        src/ManaCost.cpp\
        src/ManaCostHybrid.cpp\
        src/MenuItem.cpp\
        src/ModRules.cpp\
        src/MTGAbility.cpp\
        src/MTGCard.cpp\
        src/MTGCardInstance.cpp\
        src/MTGDeck.cpp\
        src/MTGDefinitions.cpp\
        src/MTGGamePhase.cpp\
        src/MTGGameZones.cpp\
        src/MTGPack.cpp\
        src/MTGRules.cpp\
        src/ObjectAnalytics.cpp\
        src/OptionItem.cpp\
        src/PhaseRing.cpp\
        src/Player.cpp\
        src/PlayerData.cpp\
        src/PlayGuiObject.cpp\
        src/PlayGuiObjectController.cpp\
        src/PlayRestrictions.cpp\
        src/Pos.cpp\
        src/PriceList.cpp\
        src/ReplacementEffects.cpp\
        src/Rules.cpp\
        src/SimpleButton.cpp\
        src/SimpleMenu.cpp\
        src/SimpleMenuItem.cpp\
        src/SimplePad.cpp\
        src/SimplePopup.cpp\
        src/StoryFlow.cpp\
        src/Subtypes.cpp\
        src/StyleManager.cpp\
        src/TargetChooser.cpp\
        src/TargetsList.cpp\
        src/Tasks.cpp\
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
        src/WResourceManager.cpp \
        src/AIPlayerBakaB.cpp

CONFIG(debug, debug|release):
SOURCES += src/TestSuiteAI.cpp

HEADERS  += \
        include/CacheEngine.h\
        include/AllAbilities.h\
        include/AbilityParser.h\
        include/PrecompiledHeader.h\
        include/WResource_Fwd.h\
        include/PlayRestrictions.h\
        include/ModRules.h\
        include/AIHints.h\
        include/AIPlayerBaka.h\
        include/AIPlayerBakaB.h\
        include/DeckEditorMenu.h\
        include/WResourceManagerImpl.h\
        include/DeckMenu.h\
        include/DeckMenuItem.h\
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
        include/IconButton.h\
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
        include/Effects.h\
        include/StyleManager.h\
        include/WFont.h\
        include/DeckManager.h\
        include/SimplePopup.h\
        include/SimpleMenu.h\
        include/SimpleButton.h\
        include/InteractiveButton.h\
        include/ObjectAnalytics.h

# JGE, could probably be moved outside
SOURCES += \
        ../../JGE/src/qt/filedownloader.cpp\
        ../../JGE/src/qt/corewrapper.cpp\
        ../../JGE/src/Qtmain.cpp\
        ../../JGE/src/Encoding.cpp\
        ../../JGE/src/JAnimator.cpp\
        ../../JGE/src/JApp.cpp\
        ../../JGE/src/JDistortionMesh.cpp\
        ../../JGE/src/JFileSystem.cpp\
        ../../JGE/src/JGameObject.cpp\
        ../../JGE/src/JGE.cpp\
        ../../JGE/src/JGui.cpp\
        ../../JGE/src/JLogger.cpp\
        ../../JGE/src/JLBFont.cpp\
        ../../JGE/src/JMD2Model.cpp\
        ../../JGE/src/JOBJModel.cpp\
        ../../JGE/src/JParticle.cpp\
        ../../JGE/src/JParticleEffect.cpp\
        ../../JGE/src/JParticleEmitter.cpp\
        ../../JGE/src/JParticleSystem.cpp\
        ../../JGE/src/JResourceManager.cpp\
        ../../JGE/src/JSpline.cpp\
        ../../JGE/src/JSprite.cpp\
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
        ../../JGE/src/zipFS/zfsystem.cpp\
        ../../JGE/src/zipFS/ziphdr.cpp\
        ../../JGE/src/zipFS/zstream.cpp\
        ../../JGE/src/pc/JSfx.cpp\
        ../../JGE/src/pc/JGfx.cpp

HEADERS += \
        ../../JGE/include/qt/filedownloader.h\
        ../../JGE/include/qt/corewrapper.h\
        ../../JGE/include/Threading.h\
        ../../JGE/include/decoder_prx.h\
        ../../JGE/include/DebugRoutines.h\
        ../../JGE/include/Encoding.h\
        ../../JGE/include/JAnimator.h\
        ../../JGE/include/JApp.h\
        ../../JGE/include/JAssert.h\
        ../../JGE/include/JCooleyesMP3.h\
        ../../JGE/include/JDistortionMesh.h\
        ../../JGE/include/JFileSystem.h\
        ../../JGE/include/JGameLauncher.h\
        ../../JGE/include/JGameObject.h\
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
        ../../JGE/include/JTypes.h\
        ../../JGE/include/Vector2D.h\
        ../../JGE/include/Vector3D.h\
        ../../JGE/include/vram.h\
        ../../JGE/include/hge/hgecolor.h\
        ../../JGE/include/hge/hgedistort.h\
        ../../JGE/include/hge/hgefont.h\
        ../../JGE/include/hge/hgeparticle.h\
        ../../JGE/include/hge/hgerect.h\
        ../../JGE/include/hge/hgevector.h\
        ../../JGE/src/unzip/unzip.h\
        ../../JGE/src/unzip/ioapi.h\
        ../../JGE/src/zipFS/zstream_zlib.h\
        ../../JGE/src/zipFS/zfsystem.h\
        ../../JGE/src/zipFS/zstream.h\
        ../../JGE/src/zipFS/ziphdr.h\
        ../../JGE/src/zipFS/stdafx.h\
        ../../JGE/src/zipFS/fileio.h\
        ../../JGE/src/tinyxml/tinystr.h\
        ../../JGE/src/tinyxml/tinyxml.h\
        ../../JGE/include/vram.h

# Please do not modify the following two lines. Required for deployment.
!maemo5:include(qml/qmlapplicationviewer/qmlapplicationviewer.pri)
!maemo5:qtcAddDeployment()

# maemo 5 packaging
maemo5: {
    # Variables
    BINDIR = /opt/wagic/bin
    RESDIR = /home/user/wagic/Res
    USERDIR = MyDocs/.Wagic
    ICONDIR = /usr/share

    DEFINES += RESDIR=\\\"$$RESDIR\\\"
    DEFINES += USERDIR=\\\"$$USERDIR\\\"

    INSTALLS += target \
        desktop \
        icon

    target.path = $$BINDIR

    desktop.path = $$ICONDIR/applications/hildon
    desktop.files += wagic.desktop

    icon.path = $$ICONDIR/icons/hicolor/64x64/apps
    icon.files += wagic-64x64.png

# Meego/maemo 6 packaging (no launcher)
} else:contains(MEEGO_EDITION,harmattan): {

    # Variables
    BINDIR = /opt/wagic/bin
    RESDIR = /opt/wagic/Res
    USERDIR = MyDocs/.Wagic
    ICONDIR = /usr/share

    DEFINES += RESDIR=\\\"$$RESDIR\\\"
    DEFINES += USERDIR=\\\"$$USERDIR\\\"

    INSTALLS += target \
        desktop \
        icon \
        policy

    target.path = $$BINDIR

    desktop.path = /usr/share/applications
    desktop.files += debian_harmattan/wagic.desktop

    icon.files = wagic-80x80.png
    icon.path = /usr/share/icons/hicolor/64x64/apps

    policy.files = debian_harmattan/wagic.conf
    policy.path = /usr/share/policy/etc/syspart.conf.d

} else:symbian {
    TARGET.UID3 = 0xE1D807D3

    # Smart Installer package's UID
    # This UID is from the protected range
    # and therefore the package will fail to install if self-signed
    # By default qmake uses the unprotected range value if unprotected UID is defined for the application
    # and 0x2002CCCF value if protected UID is given to the application
    #symbian:DEPLOYMENT.installer_header = 0x2002CCCF

    # Allow network access on Symbian... that's probably pointless
    TARGET.CAPABILITY += NetworkServices

    RESDIR = some/res/dir
    USERDIR = .Wagic
    DEFINES += RESDIR=\"$$RESDIR\"
    DEFINES += USERDIR=\"$$USERDIR\"
    ICON = wagic.svg
} else:android {
    DEFINES += Q_WS_ANDROID
    RESDIR = Res
    USERDIR = /sdcard/Wagic/Res
    DEFINES += RESDIR=\\\"$$RESDIR\\\"
    DEFINES += USERDIR=\\\"$$USERDIR\\\"
} else:unix {
    RESDIR = Res
    USERDIR = .Wagic
    DEFINES += RESDIR=\\\"$$RESDIR\\\"
    DEFINES += USERDIR=\\\"$$USERDIR\\\"

} else:windows {
    RESDIR = ./Res
    USERDIR = .Wagic
    DEFINES += RESDIR=\\\"$$RESDIR\\\"
    DEFINES += USERDIR=\\\"$$USERDIR\\\"
}

OTHER_FILES += \
    android/AndroidManifest.xml \
    android/res/layout/splash.xml \
    android/res/values-ru/strings.xml \
    android/res/values-it/strings.xml \
    android/res/values/strings.xml \
    android/res/values/libs.xml \
    android/res/values-id/strings.xml \
    android/res/values-rs/strings.xml \
    android/res/values-nl/strings.xml \
    android/res/values-zh-rCN/strings.xml \
    android/res/values-ro/strings.xml \
    android/res/drawable-ldpi/icon.png \
    android/res/drawable-mdpi/icon.png \
    android/res/values-et/strings.xml \
    android/res/values-fr/strings.xml \
    android/res/values-ja/strings.xml \
    android/res/values-el/strings.xml \
    android/res/values-pt-rBR/strings.xml \
    android/res/values-fa/strings.xml \
    android/res/drawable/logo.png \
    android/res/drawable/icon.png \
    android/res/values-nb/strings.xml \
    android/res/values-ms/strings.xml \
    android/res/values-de/strings.xml \
    android/res/values-zh-rTW/strings.xml \
    android/res/values-pl/strings.xml \
    android/res/drawable-hdpi/icon.png \
    android/src/org/kde/necessitas/origo/QtApplication.java \
    android/src/org/kde/necessitas/origo/QtActivity.java \
    android/src/org/kde/necessitas/ministro/IMinistroCallback.aidl \
    android/src/org/kde/necessitas/ministro/IMinistro.aidl \
    android/AndroidManifest.xml \
    android/res/layout/splash.xml \
    android/res/values-ru/strings.xml \
    android/res/values-es/strings.xml \
    android/res/values-it/strings.xml \
    android/res/values/strings.xml \
    android/res/values/libs.xml \
    android/res/values-id/strings.xml \
    android/res/values-rs/strings.xml \
    android/res/values-nl/strings.xml \
    android/res/values-zh-rCN/strings.xml \
    android/res/values-ro/strings.xml \
    android/res/drawable-ldpi/icon.png \
    android/res/drawable-mdpi/icon.png \
    android/res/values-et/strings.xml \
    android/res/values-fr/strings.xml \
    android/res/values-ja/strings.xml \
    android/res/values-el/strings.xml \
    android/res/values-pt-rBR/strings.xml \
    android/res/values-fa/strings.xml \
    android/res/drawable/logo.png \
    android/res/drawable/icon.png \
    android/res/values-nb/strings.xml \
    android/res/values-ms/strings.xml \
    android/res/values-de/strings.xml \
    android/res/values-zh-rTW/strings.xml \
    android/res/values-pl/strings.xml \
    android/res/drawable-hdpi/icon.png \
    android/src/org/kde/necessitas/origo/QtApplication.java \
    android/src/org/kde/necessitas/origo/QtActivity.java \
    android/src/org/kde/necessitas/ministro/IMinistroCallback.aidl \
    android/src/org/kde/necessitas/ministro/IMinistro.aidl




