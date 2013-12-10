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

# we're building a PSP binary here
cd JGE
make -j 8
cd ..
cd projects/mtg
mkdir objs
make -j 8
mkdir psprelease
mv EBOOT.PBP psprelease/
mv wagic.elf psprelease/
mv wagic.prx psprelease/
zip psprelease.zip -r psprelease/
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

# we create resource package
cd projects/mtg/bin/Res
python createResourceZip.py
# if we let the zip here, Wagic will use it in the testsuite 
# and we'll get 51 failed test cases
mv core_*.zip ../../../../core.zip
cd ../../../..

# Now we run the testsuite (Res needs to be in the working directory)
cd projects/mtg
../../wagic
cd ../..

