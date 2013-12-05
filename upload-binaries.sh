if [ "$TRAVIS_PULL_REQUEST" == "false" ]; then
if [ "$TRAVIS_BRANCH" == "ci_upload_binaries" ]; then
  echo -e "Creating a release\n"
  curl -X POST -H "Authorization: token ${GH_TOKEN}" \
      -d '{"tag_name": "pre-release-${TRAVIS_BUILD_NUMBER}", "target_commitish": "master", "name": "pre-release-${TRAVIS_BUILD_NUMBER}", "body": "Automatic pre-release ${TRAVIS_BUILD_NUMBER}", "draft": true, "prerelease": true}' "https://api.github.com/repos/WagicProject/wagic/releases"

  # Rename android release before upload
  cp projects/mtg/Android/bin/Wagic-debug.apk release/Wagic-android-${TRAVIS_BUILD_NUMBER}.apk

  # Now we upload
  curl -X POST -H "Authorization: token ${GH_TOKEN}" \
     -H "Accept: application/vnd.github.manifold-preview" \
     -H "Content-Type: application/zip" \
     --data-binary @release/Wagic-${TRAVIS_BUILD_NUMBER}.apk \
     "https://uploads.github.com/repos/WagicProject/wagic/releases/pre-release-${TRAVIS_BUILD_NUMBER}/assets?name=Wagic-android-${TRAVIS_BUILD_NUMBER}.apk"

  echo -e "Done uploading\n"
fi
fi
