include(wagic.pri)

# Add more folders to ship with the application, here
addExclusiveBuilds(graphics, Graphics, console, Console)

INCLUDEPATH += ../../JGE/include/qt
CONFIG(console, graphics|console){
    QT += core network multimedia
    QT -= gui

    DEFINES += CONSOLE_CONFIG
    CONFIG   += console
    CONFIG   -= app_bundle
    DEFINES += TESTSUITE

    QMAKE_CXXFLAGS += -g -fprofile-arcs -ftest-coverage
    QMAKE_LDFLAGS += -g -fprofile-arcs -ftest-coverage
    LIBS += -lgcov
}
else:CONFIG(graphics, graphics|console){
    folder_01.source = qml/QmlWagic
    folder_01.target = /usr/share
    DEPLOYMENTFOLDERS = folder_01
    QT += core gui opengl network multimedia
    QT -= declarative quick qml
    DEFINES += QT_WIDGET
    unix:!symbian:INCLUDEPATH += /usr/include/GL
}

#!android:!symbian:QT += phonon
maemo5:QT += dbus

DEFINES += QT_CONFIG
#!android:!symbian:DEFINES += USE_PHONON
android:INCLUDEPATH += $$ANDROID_NDK_ROOT/platforms/android-9/arch-arm/usr/include
#DEFINES += QT_NO_DEBUG_OUTPUT

CONFIG(graphics, graphics|console){
    HEADERS += \
        ../../JGE/include/qt/filedownloader.h\
        ../../JGE/include/qt/corewrapper.h

    SOURCES += \
        ../../JGE/src/qt/filedownloader.cpp\
        ../../JGE/src/qt/corewrapper.cpp\
        ../../JGE/src/Qtmain.cpp\
        ../../JGE/src/JMD2Model.cpp\
        ../../JGE/src/pc/JSfx.cpp\
        ../../JGE/src/pc/JGfx.cpp
}
else:CONFIG(console, graphics|console){
    HEADERS += \
        ../../JGE/include/OutputCapturer.h

    SOURCES += \
        ../../JGE/src/OutputCapturer.cpp\
        ../../JGE/src/pc/JSfx.cpp\
        ../../JGE/src/JGfx-fake.cpp\
        ../../JGE/src/Qtconsole.cpp\
}

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
} else:macx {
    # Copy the custom Info.plist to the app bundle
    QMAKE_INFO_PLIST = MacOS/Info.plist
    # Icon is mandatory for submission
    ICON = MacOS/wagic.icns

    #Move resource file
    res.commands = cd $$_PRO_FILE_PWD_/bin/Res; python createResourceZip.py;
    res.depends = all
    QMAKE_EXTRA_TARGETS += res

    # Create a dmg file
    dmg.commands = mkdir wagic.app/Contents/logs; mkdir wagic.app/Contents/Resources/Res; mv $$_PRO_FILE_PWD_/bin/Res/core*.zip wagic.app/Contents/Resources/Res; cp $$_PRO_FILE_PWD_/MacOS/wagic.launcher wagic.app/Contents/MacOS; $$dirname(QMAKE_QMAKE)/macdeployqt wagic.app -dmg
    dmg.depends = res
    QMAKE_EXTRA_TARGETS += dmg

    QMAKE_MACOSX_DEPLOYMENT_TARGET = 10.9
    QMAKE_MAC_SDK = macosx

    # Only Intel binaries are accepted so force this
    CONFIG += x86

} else:unix {
    # Variables
    BINDIR = /usr/bin
    ICONDIR = /usr/share
    RESDIR = Res
    USERDIR = .Wagic

    DEFINES += RESDIR=\\\"$$RESDIR\\\"
    DEFINES += USERDIR=\\\"$$USERDIR\\\"

    target.path = $$BINDIR

    desktop.path = $$ICONDIR/applications
    desktop.files += wagic.desktop

    icon.path = $$ICONDIR/icons/hicolor/64x64/apps
    icon.files += wagic-64x64.png

    INSTALLS += target \
        desktop \
        icon

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
    android/src/org/kde/necessitas/ministro/IMinistro.aidl \
    android/src/org/kde/necessitas/ministro/IMinistro.aidl \
    android/src/org/kde/necessitas/ministro/IMinistroCallback.aidl \
    android/src/org/qtproject/qt5/android/bindings/QtActivity.java \
    android/src/org/qtproject/qt5/android/bindings/QtApplication.java \
    android/version.xml \
    android/AndroidManifest.xml \
    android/res/values-zh-rCN/strings.xml \
    android/res/values-fa/strings.xml \
    android/res/values-pl/strings.xml \
    android/res/values-de/strings.xml \
    android/res/values-nl/strings.xml \
    android/res/values-id/strings.xml \
    android/res/values-nb/strings.xml \
    android/res/values-ms/strings.xml \
    android/res/values-ja/strings.xml \
    android/res/values-rs/strings.xml \
    android/res/values-it/strings.xml \
    android/res/values-pt-rBR/strings.xml \
    android/res/layout/splash.xml \
    android/res/values-ru/strings.xml \
    android/res/values-zh-rTW/strings.xml \
    android/res/values-et/strings.xml \
    android/res/values-fr/strings.xml \
    android/res/values-ro/strings.xml \
    android/res/values/libs.xml \
    android/res/values/strings.xml \
    android/res/values-es/strings.xml \
    android/res/values-el/strings.xml




