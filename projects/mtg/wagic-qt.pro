include(wagic.pri)

# Add more folders to ship with the application, here
addExclusiveBuilds(graphics, Graphics, console, Console)

INCLUDEPATH += ../../JGE/include/qt
CONFIG(console, graphics|console){
    QT += core network
    QT -= gui

    DEFINES += CONSOLE_CONFIG
    CONFIG   += console
    CONFIG   -= app_bundle
    DEFINES += TESTSUITE
}
else:CONFIG(graphics, graphics|console){
    folder_01.source = qml/QmlWagic
    folder_01.target = /usr/share
    DEPLOYMENTFOLDERS = folder_01
    QT += core gui opengl network
    QT -= declarative quick qml
    #maemo5:DEFINES += QT_WIDGET
    DEFINES += QT_WIDGET
    unix:!symbian:INCLUDEPATH += /usr/include/GL

    # Please do not modify the following two lines. Required for deployment.
#    !maemo5:include(qml/qmlapplicationviewer/qmlapplicationviewer.pri)
#    !maemo5:qtcAddDeployment()
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
        ../../JGE/include/qt/qtcorewrapper.h

    SOURCES += \
        ../../JGE/src/corewrapper.cpp\
        ../../JGE/src/qt/filedownloader.cpp\
        ../../JGE/src/qt/qtcorewrapper.cpp\
        ../../JGE/src/Qtmain.cpp\
        ../../JGE/src/JMD2Model.cpp\
        ../../JGE/src/pc/JGfx.cpp
}
else:CONFIG(console, graphics|console){
    HEADERS += \
        ../../JGE/include/OutputCapturer.h

    SOURCES += \
        ../../JGE/src/OutputCapturer.cpp\
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




