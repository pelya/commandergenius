#!/bin/sh

adb shell mkdir -p /sdcard/Android/obb/org.lethargik.supertux2
adb push data.zip /sdcard/Android/obb/org.lethargik.supertux2/main.5118.org.lethargik.supertux2.obb
