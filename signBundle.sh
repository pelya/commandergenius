#!/bin/sh
# Set path to your Android keystore and your keystore alias here, or put them in your environment
[ -z "$ANDROID_UPLOAD_KEYSTORE_FILE" ] && ANDROID_UPLOAD_KEYSTORE_FILE=$HOME/.android/debug.keystore
[ -z "$ANDROID_UPLOAD_KEYSTORE_ALIAS" ] && ANDROID_UPLOAD_KEYSTORE_ALIAS=androiddebugkey
PASS="-storepass android"
[ -n "$ANDROID_UPLOAD_KEYSTORE_PASS" ] && PASS="-storepass:env ANDROID_UPLOAD_KEYSTORE_PASS"
[ -n "$ANDROID_UPLOAD_KEYSTORE_PASS_FILE" ] && PASS="-storepass:file $ANDROID_UPLOAD_KEYSTORE_PASS_FILE"

APPNAME=`grep AppName AndroidAppSettings.cfg | sed 's/.*=//' | tr -d '"' | tr " '/" '---'`
APPVER=`grep AppVersionName AndroidAppSettings.cfg | sed 's/.*=//' | tr -d '"' | tr " '/" '---'`

# Fix Gradle compilation error
[ -z "$ANDROID_NDK_HOME" ] && export ANDROID_NDK_HOME="`which ndk-build | sed 's@/ndk-build@@'`"

cd project

./gradlew bundleReleaseWithDebugInfo || exit 1

../copyAssets.sh pack-binaries-bundle app/build/outputs/bundle/releaseWithDebugInfo/app-releaseWithDebugInfo.aab

cd app/build/outputs/bundle/releaseWithDebugInfo || exit 1

# Remove old certificate
cp -f app-releaseWithDebugInfo.aab ../../../../../../$APPNAME-$APPVER.aab || exit 1
# Sign with the new certificate
echo Using keystore $ANDROID_UPLOAD_KEYSTORE_FILE
stty -echo
jarsigner -verbose -sigalg SHA1withRSA -digestalg SHA1 -keystore $ANDROID_UPLOAD_KEYSTORE_FILE $PASS ../../../../../../$APPNAME-$APPVER.aab $ANDROID_UPLOAD_KEYSTORE_ALIAS || exit 1
stty echo
echo
