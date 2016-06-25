#!/bin/sh -e

## New of branch to use
TARGET_BRANCH=gh_pages

## Only deploy on gh_pages the cmake or master branch
(test "$TRAVIS_BRANCH" != "master" && test "$TRAVIS_BRANCH" != "cmake") && exit 0

## Configure Git to use OAuth token
REPO=`git config remote.origin.url`
git config credential.helper "store --file=.git/credentials"
echo "https://${GH_TOKEN}:@github.com" > .git/credentials
git config --global user.name $GH_USER
git config --global user.email $GH_EMAIL
git remote set-url origin "https://${GH_TOKEN}@github.com/WagicProject/wagic.git"

## checkout gh_branch and cleans it up
git clone $REPO out
cd out
git checkout $TARGET_BRANCH || git checkout --orphan $TARGET_BRANCH
rm -rf **/* || exit 0

## Move emscripten files in the out directory
mv ../build_emscripten/bin/wagic.html index.html
mv ../build_emscripten/bin/wagic.js wagic.js
mv ../build_emscripten/bin/wagic.data wagic.data

## Add, commit and push to GH
git add .
git commit -m "Deploy Emscripten to Github pages"
## Push new branch to remote server
git push -q origin $TARGET_BRANCH:$TARGET_BRANCH  2> /dev/null > /dev/null
