#!/bin/sh -ex

# we're building a PSP binary here
cd JGE
make -j 8
cd ..
cd projects/mtg
mkdir objs
make -j 8
cd ../..

# we're building an Android binary here
android-ndk-r9/ndk-build -C projects/mtg/Android -j8
$ANDROID list targets
$ANDROID update project -t 1 -p projects/mtg/Android
ant debug -f projects/mtg/Android/build.xml

# we're building a Qt version with GUI here
mkdir qt-gui-build
cd qt-gui-build
qmake ../projects/mtg/wagic-qt.pro CONFIG+=debug CONFIG+=graphics
make -j 8
cd ..

# let's try an Intel linux binary
qmake projects/mtg/wagic-qt.pro CONFIG+=console CONFIG+=debug DEFINES+=CAPTURE_STDERR
make -j 8

# and finish by running the testsuite
./wagic
