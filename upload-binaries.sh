if [ "$TRAVIS_PULL_REQUEST" == "false" ]; then
if [ "$TRAVIS_BRANCH" == "ci_upload_binaries" ]; then
#  echo -e "Creating a release\n"
#  curl -X POST -H "Authorization: token ${GH_TOKEN}" \
      -d '{"tag_name": "pre-release-'${TRAVIS_BUILD_NUMBER}'", "target_commitish": "master", "name": "pre-release-'${TRAVIS_BUILD_NUMBER}'", "body": "Automatic pre-release '${TRAVIS_BUILD_NUMBER}'", "draft": true, "prerelease": true}' "https://api.github.com/repos/WagicProject/wagic/releases"

  echo -e "Uploading android package\n"
  curl -X POST -H "Authorization: token ${GH_TOKEN}" \
     -H "Accept: application/vnd.github.manifold-preview" \
     -H "Content-Type: application/zip" \
     --data-binary @projects/mtg/Android/bin/Wagic-debug.apk \
     "https://uploads.github.com/repos/WagicProject/wagic/releases/113675/assets?name=Wagic-android-${TRAVIS_BUILD_NUMBER}.apk"

  echo -e "Uploading PSP package\n"
  curl -X POST -H "Authorization: token ${GH_TOKEN}" \
     -H "Accept: application/vnd.github.manifold-preview" \
     -H "Content-Type: application/zip" \
     --data-binary @projects/mtg/psprelease.zip \
     "https://uploads.github.com/repos/WagicProject/wagic/releases/113675/assets?name=Wagic-psp-${TRAVIS_BUILD_NUMBER}.zip"

  echo -e "Done uploading\n"
fi
fi
