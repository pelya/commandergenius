#!/bin/sh
set -eu

if [ $# -gt 0 -a $1 = "-r" ]; then
	shift
	run_apk=true
fi

# Set here your own NDK path if needed
# export PATH=$PATH:~/src/endless_space/android-ndk-r7
NDKBUILDPATH=$PATH
export `grep "AppFullName=" AndroidAppSettings.cfg`
if ( grep "package $AppFullName;" project/src/Globals.java > /dev/null && \
		[ "`readlink AndroidAppSettings.cfg`" -ot "project/src/Globals.java" ] && \
		[ -z "`find project/java/* project/AndroidManifestTemplate.xml -cnewer project/src/Globals.java`" ] && \
		[ -z "`find project/jni/application/src/AndroidData/* -cnewer project/src/Globals.java`" ] ) ; then true ; else
	./ChangeAppSettings.sh -a
	sleep 1
	touch project/src/Globals.java
fi

MYARCH=linux-x86
NCPU=4
if uname -s | grep -i "linux" > /dev/null ; then
	MYARCH=linux-x86
	NCPU=`cat /proc/cpuinfo | grep -c -i processor`
fi
if uname -s | grep -i "darwin" > /dev/null ; then
	MYARCH=darwin-x86
fi
if uname -s | grep -i "windows" > /dev/null ; then
	MYARCH=windows-x86
fi

rm -r -f project/bin/* # New Android SDK introduced some lame-ass optimizations to the build system which we should take care about

cd project && env PATH=$NDKBUILDPATH nice -n19 ndk-build V=1 -j$NCPU && \
 { grep "CustomBuildScript=y" ../AndroidAppSettings.cfg > /dev/null && \
   [ -`which ndk-build | xargs readlink -f | grep '/android-ndk-r[56789]'` != - ] && \
   echo Stripping libapplication.so by hand \
   rm obj/local/armeabi/libapplication.so && \
   cp jni/application/src/libapplication.so obj/local/armeabi && \
   cp jni/application/src/libapplication.so libs/armeabi && \
   `which ndk-build | sed 's@/ndk-build@@'`/toolchains/arm-linux-androideabi-4.4.3/prebuilt/$MYARCH/bin/arm-linux-androideabi-strip --strip-unneeded libs/armeabi/libapplication.so \
   || true ; } && \
 ant debug && \
 [ -n "`adb devices | tail -n +2`" ] && \
if [ $# -eq 0 ]; then # It seems peyla wanted build.sh to not install if an arg is given..
 cd bin
 { adb install -r MainActivity-debug.apk | grep 'Failure' && \
 adb uninstall `grep AppFullName ../../AndroidAppSettings.cfg | sed 's/.*=//'` && adb install -r MainActivity-debug.apk ; true ; } && \
 if [ "$run_apk" = true ]; then
   ActivityName="`grep AppFullName ../../AndroidAppSettings.cfg | sed 's/.*=//'`/.MainActivity"
   RUN_APK="adb shell am start -n $ActivityName"
   echo "Running $ActivityName on the USB-connected device:"
   echo "$RUN_APK"
   eval $RUN_APK
 fi
fi
