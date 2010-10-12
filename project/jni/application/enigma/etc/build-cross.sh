#!/bin/sh

# This is my script for building a complete cross-compiler toolchain.
# It is based partly on Ray Kelm's script, which in turn was built on
# Mo Dejong's script for doing the same, but with some added fixes.
# The intent with this script is to build a cross-compiled version
# of the current MinGW environment.
#
# Updated by Sam Lantinga <slouken@libsdl.org>
#
# Updated for current mingw version and added exception handling and threads
# for Enigma project with xerces-c

# what flavor are we building?

TARGET=i586-mingw32msvc

# where does it go?

PREFIX=/usr/i586-mingw32msvc

# you probably don't need to change anything from here down

TOPDIR=`pwd`
SRCDIR="$TOPDIR/source"

# These are the files from the MinGW 3.1 release

MINGW_URL=http://mesh.dl.sourceforge.net/sourceforge/mingw
GCC_VERSION=3.4.2-20040916-1
GCC=gcc-$GCC_VERSION
GCC_ARCHIVES="gcc-core-$GCC_VERSION-src.tar.gz gcc-g++-$GCC_VERSION-src.tar.gz"
GCC_PATCH=""
BINUTILS=binutils-2.16.91-20050827-1
BINUTILS_ARCHIVE=$BINUTILS-src.tar.gz
MINGW=mingw-runtime-3.8
MINGW_ARCHIVE=$MINGW.tar.gz
W32API=w32api-3.3
W32API_ARCHIVE=$W32API.tar.gz

# These are the files from the SDL website

SDL_URL=http://www.libsdl.org/extras/win32/common
OPENGL_ARCHIVE=opengl-devel.tar.gz
DIRECTX_ARCHIVE=directx-devel.tar.gz

# need install directory first on the path so gcc can find binutils

PATH="$PREFIX/bin:$PATH"

#
# download a file from a given url, only if it is not present
#

download_file()
{
	cd "$SRCDIR"
	if test ! -f $1 ; then
		echo "Downloading $1"
		wget "$2/$1"
		if test ! -f $1 ; then
			echo "Could not download $1"
			exit 1
		fi
	else
		echo "Found $1 in the srcdir $SRCDIR"
	fi
  	cd "$TOPDIR"
}

download_files()
{
	mkdir -p "$SRCDIR"
	
	# Make sure wget is installed
	if test "x`which wget`" = "x" ; then
		echo "You need to install wget."
		exit 1
	fi
	for file in $GCC_ARCHIVES; do
		download_file "$file" "$MINGW_URL"
	done
	download_file "$BINUTILS_ARCHIVE" "$MINGW_URL"
	download_file "$MINGW_ARCHIVE" "$MINGW_URL"
	download_file "$W32API_ARCHIVE" "$MINGW_URL"
	download_file "$OPENGL_ARCHIVE" "$SDL_URL"
	download_file "$DIRECTX_ARCHIVE" "$SDL_URL"
}

install_libs()
{
	echo "Installing cross libs and includes"
	mkdir -p "$PREFIX/$TARGET"
	cd "$PREFIX/$TARGET"
	gzip -dc "$SRCDIR/$MINGW_ARCHIVE" | tar xf -
	gzip -dc "$SRCDIR/$W32API_ARCHIVE" | tar xf -
	gzip -dc "$SRCDIR/$OPENGL_ARCHIVE" | tar xf -
	gzip -dc "$SRCDIR/$DIRECTX_ARCHIVE" | tar xf -
	cd "$TOPDIR"
}

extract_binutils()
{
	cd "$SRCDIR"
	rm -rf "$BINUTILS"
	echo "Extracting binutils"
	gzip -dc "$SRCDIR/$BINUTILS_ARCHIVE" | tar xf -
	cd "$TOPDIR"
}

configure_binutils()
{
	cd "$TOPDIR"
	rm -rf "binutils-$TARGET"
	mkdir "binutils-$TARGET"
	cd "binutils-$TARGET"
	echo "Configuring binutils"
	"$SRCDIR/$BINUTILS/configure" --prefix="$PREFIX" --target=$TARGET &> configure.log
	cd "$TOPDIR"
}

build_binutils()
{
	cd "$TOPDIR/binutils-$TARGET"
	echo "Building binutils"
	make &> make.log
	if test $? -ne 0; then
		echo "make failed - log available: binutils-$TARGET/make.log"
		exit 1
	fi
	cd "$TOPDIR"
}

install_binutils()
{
	cd "$TOPDIR/binutils-$TARGET"
	echo "Installing binutils"
	make install &> make-install.log
	if test $? -ne 0; then
		echo "install failed - log available: binutils-$TARGET/make-install.log"
		exit 1
	fi
	cd "$TOPDIR"
}

extract_gcc()
{
	cd "$SRCDIR"
	rm -rf "$GCC"
	echo "Extracting gcc"
	for file in $GCC_ARCHIVES; do
		gzip -dc "$SRCDIR/$file" | tar xf -
	done
	cd "$TOPDIR"
}

patch_gcc()
{
	if [ "$GCC_PATCH" != "" ]; then
		echo "Patching gcc"
		cd "$SRCDIR/$GCC"
		patch -p1 < "$SRCDIR/$GCC_PATCH"
		cd "$TOPDIR"
	fi
}

configure_gcc()
{
	cd "$TOPDIR"
	rm -rf "gcc-$TARGET"
	mkdir "gcc-$TARGET"
	cd "gcc-$TARGET"
	echo "Configuring gcc"
	"$SRCDIR/$GCC/configure" -v \
		--prefix="$PREFIX" --target=$TARGET \
		--with-headers="$PREFIX/$TARGET/include" \
		--with-gnu-as --with-gnu-ld \
                --enable-sjlj-exceptions --enable-threads\
		--without-newlib --disable-multilib &> configure.log
	cd "$TOPDIR"
}

build_gcc()
{
	cd "$TOPDIR/gcc-$TARGET"
	echo "Building gcc"
	make &> make.log
	if test $? -ne 0; then
		echo "make failed - log available: gcc-$TARGET/make.log"
		exit 1
	fi
	cd "$TOPDIR"
}

install_gcc()
{
	cd "$TOPDIR/gcc-$TARGET"
	echo "Installing gcc"
	make LANGUAGES="c c++" install &> make-install.log
	if test $? -ne 0; then
		echo "install failed - log available: gcc-$TARGET/make-install.log"
		exit 1
	fi
	cd "$TOPDIR"
}

final_tweaks()
{
	echo "Finalizing installation"

	# remove gcc build headers
	rm -rf "$PREFIX/$TARGET/sys-include"

        # Add extra binary links
	if [ ! -f "$PREFIX/$TARGET/bin/objdump" ]; then
		ln "$PREFIX/bin/$TARGET-objdump" "$PREFIX/$TARGET/bin/objdump"
	fi

	# make cc and c++ symlinks to gcc and g++
	if [ ! -f "$PREFIX/$TARGET/bin/g++" ]; then
		ln "$PREFIX/bin/$TARGET-g++" "$PREFIX/$TARGET/bin/g++"
	fi
	if [ ! -f "$PREFIX/$TARGET/bin/cc" ]; then
		ln -s "gcc" "$PREFIX/$TARGET/bin/cc"
	fi
	if [ ! -f "$PREFIX/$TARGET/bin/c++" ]; then
		ln -s "g++" "$PREFIX/$TARGET/bin/c++"
	fi

	# strip all the binaries
	ls "$PREFIX"/bin/* "$PREFIX/$TARGET"/bin/* | egrep -v '.dll$' | egrep -v 'gccbug$' |
	while read file; do
		strip "$file"
	done

	echo "Installation complete!"
}

download_files

install_libs

extract_binutils
configure_binutils
build_binutils
install_binutils

extract_gcc
patch_gcc
configure_gcc
build_gcc
install_gcc

final_tweaks

