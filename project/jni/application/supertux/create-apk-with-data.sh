#!/bin/sh

# Set path to your Android keystore and your keystore alias here, or put them in your environment
[ -z "$ANDROID_KEYSTORE_FILE" ] && ANDROID_KEYSTORE_FILE=~/.android/debug.keystore
[ -z "$ANDROID_KEYSTORE_ALIAS" ] && ANDROID_KEYSTORE_ALIAS=androiddebugkey
PASS="--ks-pass pass:android"
[ -n "$ANDROID_KEYSTORE_PASS" ] && PASS="--ks-pass env:ANDROID_KEYSTORE_PASS"
[ -n "$ANDROID_KEYSTORE_PASS_FILE" ] && PASS="--ks-pass file:$ANDROID_KEYSTORE_PASS_FILE"

OUT=`pwd`/../../../../SuperTux-with-data.apk
DATAZIP=`pwd`/../../../../SuperTux-data.zip
rm -f $OUT $OUT-aligned
cp -f ../../../../project/app/build/outputs/apk/release/app-release.apk $OUT || exit 1
cd supertux/data || exit 1
if [ -e $HOME/.local/share/supertux2/tilecache ]; then
	mkdir -p tilecache
	cp -f $HOME/.local/share/supertux2/tilecache/* tilecache/
fi
if zipmerge -h >/dev/null; then
	[ -e $DATAZIP ] || zip -r -9 $DATAZIP * || exit 1
	zipmerge $OUT $DATAZIP || exit 1
else
	zip -r -9 $OUT * || exit 1
fi
zipalign -p 4 $OUT $OUT-aligned || exit 1
mv $OUT-aligned $OUT
apksigner sign --ks $ANDROID_KEYSTORE_FILE --ks-key-alias $ANDROID_KEYSTORE_ALIAS $PASS $OUT || exit 1
