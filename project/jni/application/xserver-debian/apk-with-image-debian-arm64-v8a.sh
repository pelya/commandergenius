#!/bin/sh

cd ..

CURDIR=`pwd`

cd src/debian-image/img
#./prepare-img-overlay.sh
cd $CURDIR

#./build.sh || exit 1
#./build.sh debug || exit 1

cd src
zip $CURDIR/project/app/build/outputs/apk/release/app-release.apk assets/dist-debian-buster-arm64-v8a.tar.xz || exit 1
cd $CURDIR
#apksigner sign --ks ~/.android/debug.keystore --ks-key-alias androiddebugkey --ks-pass pass:android project/app/build/outputs/apk/release/app-release.apk || exit 1
./sign.sh

. ./AndroidAppSettings.cfg

#adb uninstall $AppFullName
#adb install -r project/app/build/outputs/apk/release/app-release.apk || exit 1
#adb install -r Debian-20.01.06.apk || exit 1
#adb shell am start -n $AppFullName/.MainActivity

