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
else if [ "$TRAVIS_BRANCH" = "master" ]; then
    export RELEASE_NAME="latest-master"
fi
fi
fi

echo RELEASE_NAME = $RELEASE_NAME


# updating versions with the TRAVIS build numbers
cd projects/mtg/
ant update > error.txt
cd ../..

# we create resource package
cd projects/mtg/bin/Res
python createResourceZip.py
# if we let the zip here, Wagic will use it in the testsuite
# and we'll get 51 failed test cases
mv core_*.zip ../../../../core.zip
cd ../../../..

# we're building a PSP binary here
if [ "$BUILD_PSP" = "YES" ]; then
    echo PSPDEV = $PSPDEV
    echo psp-config = `psp-config --psp-prefix`
    cd JGE
    make -j 4
    cd ..
    cd projects/mtg
    mkdir objs
    make -j 4
    mkdir WTH
    mkdir WTH/Res
    mv EBOOT.PBP WTH/
    mv ../../JGE/exceptionHandler/prx/exception.prx WTH/
    cp ../../core.zip WTH/Res
    cd WTH/Res
    unzip core.zip
    rm core.zip
    cd ..
    chmod -R 775 Res
    cd ..
    zip psprelease.zip -r WTH/
    cd ../..
fi

# we're building an Android binary here
if [ "$BUILD_ANDROID" = "YES" ]; then
    android-ndk-r9/ndk-build -C projects/mtg/Android -j4
    $ANDROID list targets
    $ANDROID update project -t 1 -p projects/mtg/Android
    ant debug -f projects/mtg/Android/build.xml
fi

# we're building a Qt version with GUI here
if [ "$BUILD_Qt" = "YES" ]; then
    mkdir qt-gui-build
    cd qt-gui-build
    $QMAKE ../projects/mtg/wagic-qt.pro CONFIG+=release CONFIG+=graphics
    make -j 4
    chmod -R 775 wagic
    zip linuxqtrelease.zip ./wagic
    cd ..
    # let's try an Intel linux binary in debug text-mode-only
    $QMAKE projects/mtg/wagic-qt.pro CONFIG+=console CONFIG+=debug DEFINES+=CAPTURE_STDERR
    make -j 4
    # Now we run the testsuite (Res needs to be in the working directory)
    cd projects/mtg
    ../../wagic
    cd ../..
fi

# Let's launch de Mac cross-compilation
if [ "$BUILD_MAC" = "YES" ]; then
    ./tools/build-macos-script.sh
fi
