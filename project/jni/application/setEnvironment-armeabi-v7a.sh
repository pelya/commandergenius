#!/bin/sh

IFS='
'

NDK=${ANDROID_NDK_HOME:-$(dirname $(which ndk-build))}

if uname -s | grep -i "linux" > /dev/null ; then
	MYARCH=linux-$(uname -m)
	NDK=`readlink -f $NDK`
elif uname -s | grep -i "darwin" > /dev/null ; then
	MYARCH=darwin-x86_64
elif uname -s | grep -i "windows" > /dev/null ; then
	MYARCH=windows-x86_64
fi

#echo NDK $NDK
[ -z "$NDK_TOOLCHAIN_VERSION" ] && NDK_TOOLCHAIN_VERSION=4.9
LOCAL_PATH=`dirname $0`
if which realpath > /dev/null ; then
	LOCAL_PATH=`realpath $LOCAL_PATH`
else
	LOCAL_PATH=`cd $LOCAL_PATH && pwd`
fi
ARCH=armeabi-v7a
GCCPREFIX=armv7a-linux-androideabi
BINUTILSPREFIX=arm-linux-androideabi
APILEVEL=21

APP_MODULES=`grep 'APP_MODULES [:][=]' $LOCAL_PATH/../Settings.mk | sed 's@.*[=]\(.*\)@\1@' | sed 's@\b\(application\|sdl_main\|sdl_native_helpers\|c++_shared\)\b@@g'`

APP_AVAILABLE_STATIC_LIBS="`echo '
include $(LOCAL_PATH)/../Settings.mk
all:
	@echo $(APP_AVAILABLE_STATIC_LIBS)
.PHONY: all' | make LOCAL_PATH=$LOCAL_PATH -s -f -`"

APP_SHARED_LIBS=$(
echo $APP_MODULES | xargs -n 1 echo | while read LIB ; do
	STATIC=`echo $APP_AVAILABLE_STATIC_LIBS | grep "\\\\b$LIB\\\\b"`
	if [ -n "$STATIC" ] ; then true
	else
		case $LIB in
			crypto) echo crypto.so.sdl.1;;
			ssl) echo ssl.so.sdl.1;;
			curl) echo curl-sdl;;
			expat) echo expat-sdl;;
			*) echo $LIB;;
		esac
	fi
done
)

if [ -z "$SHARED_LIBRARY_NAME" ]; then
	SHARED_LIBRARY_NAME=libapplication.so
fi
SHARED="-shared -Wl,-soname,$SHARED_LIBRARY_NAME"
if [ -n "$BUILD_EXECUTABLE" ]; then
	SHARED="-Wl,--gc-sections -Wl,-z,nocopyreloc -pie -fpie"
fi
if [ -n "$NO_SHARED_LIBS" ]; then
	APP_SHARED_LIBS=
fi

APP_SHARED_LIBS="`echo $APP_SHARED_LIBS | sed \"s@\([-a-zA-Z0-9_.]\+\)@$LOCAL_PATH/../../obj/local/$ARCH/lib\1.so@g\"`"
APP_MODULES_INCLUDE="`echo $APP_MODULES | sed \"s@\([-a-zA-Z0-9_.]\+\)@-isystem$LOCAL_PATH/../\1/include@g\"`"

CFLAGS="
-g
-ffunction-sections
-fdata-sections
-funwind-tables
-fstack-protector-strong
-no-canonical-prefixes
-mthumb
-Wformat
-Werror=format-security
-Oz
-DNDEBUG
-fPIC
$APP_MODULES_INCLUDE
$CFLAGS"

CFLAGS="`echo $CFLAGS | tr '\n' ' '`"

LDFLAGS="
-fPIC
-g
-ffunction-sections
-fdata-sections
-Wl,--gc-sections
-funwind-tables
-fstack-protector-strong
-no-canonical-prefixes
-mthumb
-Wformat
-Werror=format-security
-Oz
-DNDEBUG
-Wl,--build-id
-Wl,--warn-shared-textrel
-Wl,--fatal-warnings
-Wl,--no-undefined
-Wl,-z,noexecstack
-Qunused-arguments
-Wl,-z,relro
-Wl,-z,now
-Wl,--no-rosegment
$SHARED
$APP_SHARED_LIBS
-landroid
-llog
-latomic
-lm
-ldl
$LDFLAGS"

LDFLAGS="`echo $LDFLAGS | tr '\n' ' '`"

CC="$NDK/toolchains/llvm/prebuilt/$MYARCH/bin/$GCCPREFIX$APILEVEL-clang"
CXX="$NDK/toolchains/llvm/prebuilt/$MYARCH/bin/$GCCPREFIX$APILEVEL-clang++"
CPP="$CC -E $CFLAGS"

env \
CFLAGS="$CFLAGS" \
CXXFLAGS="$CXXFLAGS $CFLAGS -frtti -fexceptions" \
LDFLAGS="$LDFLAGS" \
CC="$CC" \
CXX="$CXX" \
RANLIB="$NDK/toolchains/llvm/prebuilt/$MYARCH/bin/llvm-ranlib" \
LD="$CXX" \
AR="$NDK/toolchains/llvm/prebuilt/$MYARCH/bin/llvm-ar" \
CPP="$CPP" \
NM="$NDK/toolchains/llvm/prebuilt/$MYARCH/bin/llvm-nm" \
AS="$CC" \
STRIP="$NDK/toolchains/llvm/prebuilt/$MYARCH/bin/llvm-strip" \
"$@"
