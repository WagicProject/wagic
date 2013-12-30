#!/bin/sh -ex

# let's dump some info to debug a bit
echo PSPDEV = $PSPDEV
echo psp-config = `psp-config --psp-prefix`
echo ls = `ls`
echo pwd = `pwd`

# updating versions with the TRAVIS build numbers
cd projects/mtg/
ant update > error.txt
cd ../..

# we create resource package
cd projects/mtg/bin/Res
python createResourceZip.py
python createResourceZip.py —-platform=qt
# if we let the zip here, Wagic will use it in the testsuite
# and we'll get 51 failed test cases
mv core_*.zip ../../../../core.zip
cd ../../../..

# we're building a PSP binary here
cd JGE
make -j 8
make -j 8 -f Makefile.hge install
cd ..
cd projects/mtg
mkdir objs
make -j 8
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

# we're building an Android binary here
android-ndk-r9/ndk-build -C projects/mtg/Android -j8
$ANDROID list targets
$ANDROID update project -t 1 -p projects/mtg/Android
ant debug -f projects/mtg/Android/build.xml

# we're building a Qt version with GUI here
mkdir qt-gui-build
cd qt-gui-build
qmake ../projects/mtg/wagic-qt.pro CONFIG+=release CONFIG+=graphics
make -j 8
cd ..

# let's try an Intel linux binary in debug text-mode-only
qmake projects/mtg/wagic-qt.pro CONFIG+=console CONFIG+=debug DEFINES+=CAPTURE_STDERR
make -j 8

# we're cross-compiling a Qt Windows version here, 
# PATH is only set here to prevent collision
export PATH="$PATH:/opt/mingw32/bin"
mkdir build
cd build
mkdir win-cross
cd win-cross
mv ../../projects/mtg/bin/Res/settings/options.orig.txt ../../projects/mtg/bin/Res/settings/options.txt
mv ../../projects/mtg/bin/Res/player/options.orig.txt ../../projects/mtg/bin/Res/player/options.txt
/opt/mingw32/bin/qmake ../../projects/mtg/wagic-qt.pro CONFIG+=release CONFIG+=graphics
make -j 8
cd release
cp ../../../projects/mtg/bin/fmod.dll .
cp /opt/mingw32/bin/QtCore4.dll .
cp /opt/mingw32/bin/QtGui4.dll .
cp /opt/mingw32/bin/QtNetwork4.dll .
cp /opt/mingw32/bin/QtOpenGL4.dll .
cp ../../../projects/mtg/bin/zlib1.dll .
cp /opt/mingw32/bin/libpng15-15.dll .
cd ..
zip win-cross.zip -r release/
mv ../../projects/mtg/bin/Res/settings/options.txt ../../projects/mtg/bin/Res/settings/options.orig.txt
mv ../../projects/mtg/bin/Res/player/options.txt ../../projects/mtg/bin/Res/player/options.orig.txt
cd ../..

# Now we run the testsuite (Res needs to be in the working directory)
cd projects/mtg
../../wagic
cd ../..
