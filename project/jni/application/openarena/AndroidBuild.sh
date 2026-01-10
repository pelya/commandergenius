#!/bin/sh

LOCAL_PATH=`dirname $0`
LOCAL_PATH=`cd $LOCAL_PATH && pwd`

#env NO_SHARED_LIBS=1 V=1 ../setEnvironment-$1.sh make -C vm -j8 PLATFORM=android ARCH=$1 USE_LOCAL_HEADERS=0 BUILD_MISSIONPACK=0 || exit 1

# Do not generate shared game logic libs - QVM files are used instead
# ../setEnvironment-armeabi.sh sh -c "cd vm/build/release-android-$1/baseq3 && \$STRIP --strip-unneeded *.so && zip ../../../../AndroidData/binaries.zip *.so"

mkdir -p AndroidData/lib/$1
cp -f upnpc-$1 AndroidData/lib/$1/libupnpc.so

env PATH=`pwd`/..:$PATH CFLAGS="-Oz -flto=thin" \
LDFLAGS="-L$LOCAL_PATH/../../../obj/local/$1 -Oz -flto=thin -lGLESv1_CM" \
../setEnvironment-$1.sh make -j8 -C engine release V=1 \
PLATFORM=android ARCH=$1 USE_GLES=1 USE_LOCAL_HEADERS=0 BUILD_CLIENT_SMP=0 \
USE_OPENAL=1 USE_OPENAL_DLOPEN=0 USE_VOIP=1 USE_CURL=1 USE_CURL_DLOPEN=0 USE_CODEC_VORBIS=1 USE_MUMBLE=0 USE_FREETYPE=1 \
USE_RENDERER_DLOPEN=0 USE_INTERNAL_ZLIB=0 USE_INTERNAL_JPEG=1 BUILD_RENDERER_REND2=0 C_ONLY=1 && \
echo "Copying engine/build/release-android-$1/openarena.$1 -> libapplication-$1.so" && \
cp -f engine/build/release-android-$1/openarena.$1 libapplication-$1.so || exit 1
exit 0
