#!/bin/sh -ex

if [ "$TRAVIS_PULL_REQUEST" == "false" && "$TRAVIS_BRANCH" == "ci_upload_binaries"]; then
  echo -e "Creating a release\n"
  curl -X POST -H "Authorization: token ${GH_TOKEN}" -d '{"tag_name": "pre-release-${TRAVIS_BUILD_NUMBER}", "target_commitish": "master", "name": "pre-release-${TRAVIS_BUILD_NUMBER}", "body": "Automatic pre-release ${TRAVIS_BUILD_NUMBER}", "draft": true, "prerelease": true}' "https://api.github.com/repos/WagicProject/wagic/releases"

  #copy data we're interested in to other place
  cp -R coverage $HOME/coverage

  #go to home and setup git
  cd $HOME
  git config --global user.email "travis@travis-ci.org"
  git config --global user.name "Travis"

  #using token clone gh-pages branch
  git clone --quiet --branch=gh-pages https://${GH_TOKEN}@github.com/Uko/Rubidium-WHOIS.git  gh-pages > /dev/null

  #go into diractory and copy data we're interested in to that directory
  cd gh-pages
  cp -Rf $HOME/coverage/* .

  #add, commit and push files
  git add -f .
  git commit -m "Travis build $TRAVIS_BUILD_NUMBER pushed to gh-pages"
  git push -fq origin gh-pages > /dev/null

  echo -e "Done magic with coverage\n"
fi
