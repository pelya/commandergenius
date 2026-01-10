#!/usr/bin/env bash

set -e

# Base game data
mkdir -p ./data-plat-indp
pushd ./data-plat-indp
cmp .ottdrev ../src/.ottdrev && {
	echo "Version did not change - no need to download data files"
	exit
}

GFX_VERSION=7.1
SFX_VERSION=1.0.3
MSX_VERSION=0.4.2
ANDROID_DATA_FULLPATH=$(realpath ./AndroidData/)

GFX_VERSION=$(curl --fail https://cdn.openttd.org/opengfx-releases/latest.yaml | grep -Po "version: \K[0-9.]+")
SFX_VERSION=$(curl --fail https://cdn.openttd.org/opensfx-releases/latest.yaml | grep -Po "version: \K[0-9.]+")
MSX_VERSION=$(curl --fail https://cdn.openttd.org/openmsx-releases/latest.yaml | grep -Po "version: \K[0-9.]+")


if ! [ -e "./opengfx-${GFX_VERSION}.tar" ]; then
	curl --fail https://cdn.openttd.org/opengfx-releases/${GFX_VERSION}/opengfx-${GFX_VERSION}-all.zip | jar xv
fi
if ! [ -e "./opensfx-${SFX_VERSION}.tar" ]; then
curl --fail https://cdn.openttd.org/opensfx-releases/${SFX_VERSION}/opensfx-${SFX_VERSION}-all.zip | jar xv
fi
if ! [ -d "./openmsx-${MSX_VERSION}/" ]; then
	curl --fail https://cdn.openttd.org/openmsx-releases/${MSX_VERSION}/openmsx-${MSX_VERSION}-all.zip | jar xv
	tar xvf ./openmsx-${MSX_VERSION}.tar && rm ./openmsx-${MSX_VERSION}.tar
fi

cp -f ../src/.ottdrev ./
