#!/bin/sh

echo "Create directory data/baseoa and put your .pk3 files there"

jobb -pn ws.openarena.sdl -pv 8839 -d ./data -o main.8839.ws.openarena.sdl.obb

[ -n "$1" ] && {
	adb shell mkdir -p /sdcard/Android/obb/ws.openarena.sdl
	adb push main.8839.ws.openarena.sdl.obb /sdcard/Android/obb/ws.openarena.sdl/
}
