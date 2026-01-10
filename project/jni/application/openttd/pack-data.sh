#!/usr/bin/env bash

set -e

VER=14.1-0
ARCH=$1
ANDROID_DATA_FULLPATH=$(realpath ./AndroidData/)

# Base game data
[ -e ${ANDROID_DATA_FULLPATH}/openttd-data-$VER.zip.xz ] && [ -n "$NO_REBUILD_DATA" ] || {
	pushd ./data
	rm -f ${ANDROID_DATA_FULLPATH}/openttd-data-*.zip.xz ${ANDROID_DATA_FULLPATH}/openttd-data-*.zip

	pushd ./baseset
		cp ../../data-plat-indp/opengfx*.tar .
		cp ../../data-plat-indp/opensfx*.tar .
		cp -r ../../data-plat-indp/openmsx*/ .
	popd

	zip -0 -r ${ANDROID_DATA_FULLPATH}/openttd-data-$VER.zip ./ && xz -8 ${ANDROID_DATA_FULLPATH}/openttd-data-$VER.zip
	popd
}

# Timidity
[ -e ${ANDROID_DATA_FULLPATH}/timidity.zip.xz ] && [ -n "$NO_REBUILD_DATA" ] || {
	pushd ../../timidity/samples/
	rm -f ${ANDROID_DATA_FULLPATH}/timidity.zip.xz ${ANDROID_DATA_FULLPATH}/timidity.zip
	cp ./timidity.zip ${ANDROID_DATA_FULLPATH}/timidity.zip && xz -8 ${ANDROID_DATA_FULLPATH}/timidity.zip
	popd
}

# ICU
# TODO handle versioning. Use Makefile var
[ -e ${ANDROID_DATA_FULLPATH}/icudt62l.zip.xz ] && [ -n "$NO_REBUILD_DATA" ] || {
	pushd ../../icuuc
	rm -f ${ANDROID_DATA_FULLPATH}/icudt62l.zip.xz ${ANDROID_DATA_FULLPATH}/icudt62l.zip
	if ! [  -e ./share/icu/62.1/icudt62l.dat ]; then
		echo "Missing icudt62l.data; Try force recompiling prebuilt targets";
		exit 1
	fi
	zip -0 ${ANDROID_DATA_FULLPATH}/icudt62l.zip share/icu/62.1/icudt62l.dat && xz -8 ${ANDROID_DATA_FULLPATH}/icudt62l.zip
	popd
}

# Fonts
[ -e ${ANDROID_DATA_FULLPATH}/openttd-fonts.zip.xz ] && [ -n "$NO_REBUILD_DATA" ] || {
	pushd ../../freetype/
	rm -f ${ANDROID_DATA_FULLPATH}/openttd-fonts.zip.xz ${ANDROID_DATA_FULLPATH}/openttd-fonts.zip
	zip -0 -r ${ANDROID_DATA_FULLPATH}/openttd-fonts.zip ./fonts/ && xz -8 ${ANDROID_DATA_FULLPATH}/openttd-fonts.zip
	popd
}
