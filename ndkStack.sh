#!/bin/sh

adb logcat | ndk-stack -s project/libs/arm64-v8a
