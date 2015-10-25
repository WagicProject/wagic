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
    mkdir build_psp
    cd build_psp
    cmake -DCMAKE_TOOLCHAIN_FILE=../CMakeModules/psp.toolchain.cmake ..
    make -j8
    cd ..
fi

# we're building an Android binary here
if [ "$BUILD_ANDROID" = "YES" ]; then
	mkdir build_sdl
	cd build_sdl
	cmake -DCMAKE_TOOLCHAIN_FILE=../CMakeModules/android.toolchain.cmake -DANDROID_NATIVE_API_LEVEL=android-10
	make -j8
	cd ..
fi

# we're building a Qt version with GUI here
if [ "$BUILD_Qt" = "YES" ]; then
	mkdir build_qt_widget
	cd build_qt_widget
	cmake -Dbackend_qt_widget=ON -Dbackend_qt_console=OFF ..
	make -j4
	cd ..

    # let's try an Intel linux binary in debug text-mode-only
	mkdir build_qt_console
	cd build_qt_console
	cmake -Dbackend_qt_console=ON ..
	make -j4
	cd ..

    # Now we run the testsuite (Res needs to be in the working directory)
    cd projects/mtg
    ./../../build_qt_console/bin/wagic
    cd ../..
fi

# we're building a SDL version
if [ "$BUILD_SDL" = "YES" ]; then
	mkdir build_SDL
	cd build_SDL
	cmake -Dbackend_sdl=ON ..
	make -j4
	cd ..
fi
# Let's launch de Mac cross-compilation
if [ "$BUILD_MAC" = "YES" ]; then
    ./tools/build-macos-script.sh
fi
