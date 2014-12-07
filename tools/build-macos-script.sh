#!/bin/sh -ex

## New of branch to use
TRAVIS_MAC_BRANCH=travis_mac_osx

## New Travis-CI configuration, adapted for Mac-OS
NEW_TRAVIS_YML=\
"language: objective-c
before_install:
- brew update
- brew install qt5
env:
  global:
    secure: "fJgWlCFbde96OSQNGKUmowGX+ERPeqP+n1EOMf1+FJzOU4DdkTLRAlV5+5qnEX9jB/3mWN6iPpmG1qEz/SdDG3KHxJYs4ZU/Lu485O24zZ/+GdYBNsrvhPD9ckPGEMLDa1foEVTDnW0Dlkz3BCFcszjhtXGUJv7v6Pj6LRk1Mg8="
script:
- /usr/local/opt/qt5/bin/qmake projects/mtg/wagic-qt.pro CONFIG+=graphics
- make -j 4 dmg
after_success:
- python tools/upload-binaries.py -t $GH_TOKEN -s $TRAVIS_COMMIT -l wagic.dmg -r Wagic-macosx.dmg -b $TRAVIS_BRANCH"

## Only cross-compile on Mac the master branch
test "$TRAVIS_BRANCH" != "master" && exit 0

## Configure Git to use OAuth token
git config credential.helper "store --file=.git/credentials"
echo "https://${GH_TOKEN}:@github.com" > .git/credentials

## Delete remote Travis-Mac branch (if any)
REMOTE=$(git branch -r | grep "origin/$TRAVIS_MAC_BRANCH\$")
if test -n "$REMOTE" ; then
    # Delete remote branch
    git branch -r -D "origin/$TRAVIS_MAC_BRANCH"
    # Push (delete) remote branch on temote server (e.g. github)
    git push origin ":$TRAVIS_MAC_BRANCH"
fi

## Create a new branch
git checkout -q -b "$TRAVIS_MAC_BRANCH" "$TRAVIS_BRANCH"

## Write a new Travis-CI configuration file
echo "$NEW_TRAVIS_YML" > .travis.yml
git add .travis.yml
git rm appveyor.yml
git commit -m "Auto-Updated Travis-CI configuration for Mac"

## Push new branch to remote server
git push -q origin $TRAVIS_MAC_BRANCH:$TRAVIS_MAC_BRANCH
