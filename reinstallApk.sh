#!/bin/sh

APK="$1"
[ -z "$APK" ] && APK=project/app/build/outputs/apk/release/app-release.apk

LOG=/tmp/reinstall-apk-$$.log

adb install -r "$APK" | tee $LOG

grep '^Failure' $LOG && {
	adb uninstall `aapt dump badging "$APK" | grep 'package:' | sed "s/.*name='\([^']*\)'.*/\1/"`
	adb install -r "$APK"
}

rm -f $LOG
