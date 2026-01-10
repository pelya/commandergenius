#!/bin/sh

source ./AndroidAppSettings.cfg

adb shell pm clear $AppFullName
