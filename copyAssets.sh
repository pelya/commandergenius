#!/bin/sh

ARCHES="arm64-v8a armeabi-v7a x86 x86_64"

if [ "$1" = "pack-binaries" -o "$1" = "pack-binaries-bundle" ]; then
	[ -e jni/application/src/AndroidData/lib ] || exit 0
	[ -e jni/application/src/AndroidData/binaries*.zip ] && {
		echo "Error: binaries.zip no longer supported in Android 10"
		echo "Copy your executable binaries to AndroidData/lib/arm64-v8a"
		echo "Then execute them using \$LIBDIR or getenv(\"LIBDIR\")"
		exit 0
	}
	APK="`pwd`/$2"
	echo "Copying binaries to .apk file"
	cd jni/application/src/AndroidData/ || exit 1
	if [ "$1" = "pack-binaries-bundle" ]; then
		rm -rf base
		mkdir -p base
		cp -L -r lib base/
		zip -r -D "$APK" base || exit 1
		rm -rf base
	else
		zip -r -D "$APK" lib || exit 1
	fi
	cd ../../../../
	exit 0
fi

echo "Copying app data files from project/jni/application/src/AndroidData to project/assets"
mkdir -p project/assets
rm -f -r project/assets/*
if [ -d "project/jni/application/src/AndroidData" ] ; then
	for F in project/jni/application/src/AndroidData/*; do
		[ "$F" = "project/jni/application/src/AndroidData/lib" ] && continue
		[ "$F" = "project/jni/application/src/AndroidData/assetpack" ] && continue
		cp -L -r "$F" project/assets/
	done
fi

exit 0
