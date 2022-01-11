#!/bin/sh -e

# let's dump some info to debug a bit
echo ls = `ls`
echo pwd = `pwd`
# computing potential release name
echo TRAVIS_PULL_REQUEST = $TRAVIS_PULL_REQUEST
echo TRAVIS_BRANCH = $TRAVIS_BRANCH

if [ "$TRAVIS_PULL_REQUEST" = "false" ]; then
if [ "$TRAVIS_BRANCH" = "alphas" ]; then
    export RELEASE_NAME="alpha-${TRAVIS_BUILD_NUMBER}"
else
    export RELEASE_NAME="latest-${TRAVIS_BRANCH}"
fi
fi

echo RELEASE_NAME = $RELEASE_NAME

if [ "$TRAVIS_OS_NAME" = "linux" ]; then
    # updating versions with the TRAVIS build numbers
    cd projects/mtg/
    ant update > errors.txt
    cd ../..
fi

# we create resource package
if [ "$BUILD_RES" = "YES" ] || [ "$BUILD_PSP" = "YES" ]; then
    cd projects/mtg/bin/Res
    python createResourceZip.py
    # if we let the zip here, Wagic will use it in the testsuite
    # and we'll get 51 failed test cases
    mv core_*.zip ../../../../core.zip
    cd ../../../..
fi

# we're building a PSP binary here
if [ "$BUILD_TYPE" = "PSP" ]; then
    mkdir build_psp
    cd build_psp
    cmake -DCMAKE_TOOLCHAIN_FILE=../CMakeModules/psp.toolchain.cmake ..
    make -j8
    mkdir WTH
    mkdir WTH/Res
    mv bin/EBOOT.PBP WTH/
    mv ../thirdparty/exceptionHandler/prx/exception.prx WTH/
    cp ../core.zip WTH/Res
    cd WTH/Res
    unzip core.zip
    rm core.zip
    cd ..
    chmod -R 775 Res
    cd ..
    zip psprelease.zip -r WTH/
    cd ..
fi

# we're building an Android binary here
if [ "$BUILD_ANDROID" = "YES" ]; then
    android-ndk-r22/ndk-build -C projects/mtg/Android -j4
    $ANDROID list targets
    $ANDROID update project -t 1 -p projects/mtg/Android
    ant debug -f projects/mtg/Android/build.xml
fi

# we're building a linux Qt version with GUI here
if [ "$BUILD_Qt" = "YES" ] && [ "$TRAVIS_OS_NAME" = "linux" ]; then
    mkdir qt-gui-build
    cd qt-gui-build
    $QMAKE ../projects/mtg/wagic-qt.pro CONFIG+=release CONFIG+=graphics
    make -j 4
    chmod -R 775 wagic
    zip linuxqtrelease.zip ./wagic
    cd ..

    # let's try an Intel linux binary in debug text-mode-only
    mkdir build_qt_console
    cd build_qt_console
    cmake -Dbackend_qt_console=ON ..
    make -j4 wagic
    cd ..

    # Now we run the testsuite (Res needs to be in the working directory)
    cd projects/mtg
    ./../../build_qt_console/bin/wagic
    cd ../..
fi
# we're building a mac Qt version with GUI here
if [ "$BUILD_Qt" = "YES" ] && [ "$TRAVIS_OS_NAME" = "osx" ]; then
    mkdir qt-gui-build
    cd qt-gui-build
    $QMAKE ../projects/mtg/wagic-qt.pro CONFIG+=release CONFIG+=graphics
    make -j 4 dmg
fi
