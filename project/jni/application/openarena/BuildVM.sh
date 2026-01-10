#!/bin/sh

LOCAL_PATH=`dirname $0`
LOCAL_PATH=`cd $LOCAL_PATH && pwd`

mkdir -p AndroidData
make -j8 -C vm BUILD_MISSIONPACK=0 || exit 1
cd vm/build/release-linux-`uname -m`/baseq3
#rm -f ../../../../AndroidData/binaries.zip ../../../../AndroidData/pak7-android.pk3
zip -r ../../../../AndroidData/pak7-android.pk3 vm
cd ../../../android
zip -r ../../AndroidData/pak7-android.pk3 *
ln -sf ../engine/misc/quake3-tango.png ../../AndroidData/logo.png
