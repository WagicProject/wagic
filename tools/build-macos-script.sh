#!/bin/sh -e

## New of branch to use
TRAVIS_MAC_BRANCH=travis_mac_osx

## Only cross-compile on Mac the master branch
test "$TRAVIS_BRANCH" != "master" && exit 0

## Configure Git to use OAuth token
git config credential.helper "store --file=.git/credentials"
echo "https://${GH_TOKEN}:@github.com" > .git/credentials
git config --global user.name $GH_USER
git config --global user.email $GH_EMAIL
git remote set-url origin "https://${GH_TOKEN}@github.com/WagicProject/wagic.git"

## Delete remote Travis-Mac branch (if any)
export REMOTE=$(git branch -r | grep "origin/$TRAVIS_MAC_BRANCH\$")
if test -n "$REMOTE" ; then
    # Delete remote branch
    git branch -r -D "origin/$TRAVIS_MAC_BRANCH"
    # Push (delete) remote branch on temote server (e.g. github)
    git push origin ":$TRAVIS_MAC_BRANCH"
fi

## Create a new branch
git checkout -q -b "$TRAVIS_MAC_BRANCH" "$TRAVIS_BRANCH"

## Write a new Travis-CI configuration file
cp tools/macos.travis.yml .travis.yml
git add .travis.yml
git rm appveyor.yml
git commit -m "Auto-Updated Travis-CI configuration for Mac"
## Push new branch to remote server
git push -q origin $TRAVIS_MAC_BRANCH:$TRAVIS_MAC_BRANCH  2> /dev/null > /dev/null
