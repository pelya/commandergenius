#!/bin/sh

export CC="ccache i586-mingw32msvc-gcc"
# export CXX=i586-mingw32msvc-g++
export CXX="ccache i586-mingw32msvc-g++"
#export CXXFLAGS="-g0"
export RANLIB=i586-mingw32msvc-ranlib
export AR=i586-mingw32msvc-ar
export WINDRES=i586-mingw32msvc-windres
#make distclean
PATH=/usr/i586-mingw32msvc/bin:$PATH \
	./configure \
	--host=i586-mingw32msvc \
	--target=i586-mingw32msvc \
	--build=i586-linux \
	AR=i586-mingw32msvc-ar \
	$*

#	--enable-optimize \

