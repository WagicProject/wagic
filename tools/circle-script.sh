#!/bin/sh -e

# let's dump some info to debug a bit
echo ls = `ls`
echo pwd = `pwd`
# computing potential release name
echo CIRCLE_PR_NUMBER = $CIRCLE_PR_NUMBER
echo CIRCLE_BRANCH = $CIRCLE_BRANCH

if [ -z "$CIRCLE_PR_NUMBER" ]; then
    if [ "$CIRCLE_BRANCH" = "alphas" ]; then
        export RELEASE_NAME="alpha-${CIRCLE_BUILD_NUM}"
    elif [ "$CIRCLE_BRANCH" = "master" ]; then
        export RELEASE_NAME="latest-master"
    fi
fi

echo RELEASE_NAME = $RELEASE_NAME


# updating versions with the CIRCLE build numbers
cd projects/mtg/
ant update > error.txt
cd ../..

# we create resource package
if [ "$BUILD_RES" = "YES" ] || [ "$BUILD_PSP" = "YES" ]; then
    cd projects/mtg/bin/Res
    python createResourceZip.py
    # if we let the zip here, Wagic will use it in the testsuite
    # and we'll get 51 failed test cases
    mv core_*.zip ../../../../core.zip
    cd ../../../..
fi

# we're building an Android binary here
if [ "$BUILD_ANDROID" = "YES" ]; then
    android-ndk-r22/ndk-build -C projects/mtg/Android -j4
    android-sdk-linux/tools/android list targets
    android-sdk-linux/tools/android update project -t 1 -p projects/mtg/Android
    ant debug -f projects/mtg/Android/build.xml
fi
