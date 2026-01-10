#!/bin/sh

APPNAME=`grep AppName AndroidAppSettings.cfg | sed 's/.*=//' | tr -d '"' | tr " '/" '---'`
APPVER=`grep AppVersionName AndroidAppSettings.cfg | sed 's/.*=//' | tr -d '"' | tr " '/" '---'`

ARCHIVER=gzip
EXT=gz
which xz > /dev/null && ARCHIVER="xz -z -T 0" && EXT=xz
#which pxz > /dev/null && ARCHIVER=pxz && EXT=xz || echo "Install pxz for faster archiving: sudo apt-get install pxz"

# TODO: Boost, Python and ffmpeg are stored in repository as precompiled binaries, the proper way to fix that is to build them using scripts, and remove that binaries
# --exclude="*.a" --exclude="*.so"
tar -c --exclude-vcs --exclude="*.o" --exclude="*.d" --exclude="*.dep" \
--exclude="libboost_*.a" --exclude="libcharset.so" --exclude="libiconv.so" \
--exclude="libicu*.a" --exclude="libharfbuzz.a" --exclude="libcrypto.so*" --exclude="libssl.so*" \
`git ls-files --exclude-standard | grep -v '^project/jni/application/.*'` \
`find  project/jni/application -maxdepth 1 -type f -o -type l` \
project/jni/application/src \
project/jni/application/`readlink project/jni/application/src` \
project/AndroidManifest.xml project/src \
project/obj/local/armeabi-v7a/*.so project/obj/local/arm64-v8a/*.so project/obj/local/x86/*.so  project/obj/local/x86_64/*.so  \
project/app/build/outputs/mapping/release/mapping.txt \
"$@" | $ARCHIVER > $APPNAME-$APPVER-src.tar.$EXT
