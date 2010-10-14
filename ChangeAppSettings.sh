#!/bin/sh

CHANGE_APP_SETTINGS_VERSION=11
AUTO=

if [ "X$1" = "X-a" ]; then
	AUTO=1
fi

. ./AndroidAppSettings.cfg

if [ "$CHANGE_APP_SETTINGS_VERSION" != "$AppSettingVersion" ]; then
	AUTO=
fi

LibSdlVersionOld=`readlink project/jni/sdl | grep -o '[0-9][.][0-9]'`
CompiledLibrariesOld=$CompiledLibraries

var=""

if [ -z "$AUTO" ]; then

echo -n "\n===== libSDL on Android configuration =====\n\nIf you will supply empty string as answer the previous value will be used\n"

echo -n "\nlibSDL version to use (1.2 or 1.3) ($LibSdlVersion): "
read var
if [ -n "$var" ] ; then
	LibSdlVersion="$var"
fi

echo -n "\nSpecify application name (e.x. My Application) ($AppName): "
read var
if [ -n "$var" ] ; then
	AppName="$var"
fi

echo -n "\nSpecify reversed site name of application (e.x. com.mysite.myapp) ($AppFullName): "
read var
if [ -n "$var" ] ; then
	AppFullName="$var"
fi

echo -n "\nSpecify screen orientation: (v)ertical or (h)orizontal ($ScreenOrientation): "
read var
if [ -n "$var" ] ; then
	ScreenOrientation="$var"
fi

echo -n "\nSpecify path to download application data in zip archive in the form 'Description|http://URL|http://BackupURL|...'"
echo -n "\nYou may specify additional paths to optional game content delimited by newlines (empty line to finish)"
echo -n "\nIf the URL in in the form ':dir/file.dat:http://URL/' it will be downloaded as-is to game dir and not unzipped\n\n"
echo -n "`echo $AppDataDownloadUrl | tr '^' '\\n'`"
echo
AppDataDownloadUrl1=""
while true; do
	read var
	if [ -n "$var" ] ; then
		if [ -z "$AppDataDownloadUrl1" ]; then
			AppDataDownloadUrl1="$var"
		else
			AppDataDownloadUrl1="$AppDataDownloadUrl1^$var"
		fi
	else
		break
	fi
done
if [ -n "$AppDataDownloadUrl1" ] ; then
	AppDataDownloadUrl="$AppDataDownloadUrl1"
fi

echo -n "\nApplication window should be resized to fit into native device screen (480x320 or 800x480) (y) or (n) ($SdlVideoResize): "
read var
if [ -n "$var" ] ; then
	SdlVideoResize="$var"
fi

echo -n "\nApplication resizing should preserve the aspect ratio, creating black bars  (y) or (n) ($SdlVideoResizeKeepAspect): "
read var
if [ -n "$var" ] ; then
        SdlVideoResizeKeepAspect="$var"
fi


echo -n "\nEnable OpenGL depth buffer (needed only for 3-d applications, small speed decrease) (y) or (n) ($NeedDepthBuffer): "
read var
if [ -n "$var" ] ; then
	NeedDepthBuffer="$var"
fi

echo -n "\nApplication uses mouse, disables touchscreen keyboard currently (y) or (n) ($AppUsesMouse): "
read var
if [ -n "$var" ] ; then
	AppUsesMouse="$var"
fi

echo -n "\nApplication needs arrow keys (y) or (n), if (y) the accelerometer or touchscreen keyboard\nwill be used as arrow keys if phone does not have dpad/trackball ($AppNeedsArrowKeys): "
read var
if [ -n "$var" ] ; then
	AppNeedsArrowKeys="$var"
fi

echo -n "\nApplication uses joystick (y) or (n), the accelerometer (2-axis) or orientation sensor (3-axis)\nwill be used as joystick 0 if not used as arrow keys ($AppUsesJoystick): "
read var
if [ -n "$var" ] ; then
	AppUsesJoystick="$var"
fi

echo -n "\nApplication uses multitouch (y) or (n), multitouch events are passed as \n4-axis joysticks 1-5, including pressure and size ($AppUsesMultitouch): "
read var
if [ -n "$var" ] ; then
	AppUsesMultitouch="$var"
fi

echo -n "\nApplication implements Android-specific routines to put to background, and will not draw anything to screen\n"
echo -n "between SDL_ACTIVEEVENT->SDL_APPACTIVE lost / gained notifications - you should check for them\n"
echo -n "rigth after SDL_Flip(), if (n) then SDL_Flip() will block till app in background (y) or (n) ($NonBlockingSwapBuffers): "
read var
if [ -n "$var" ] ; then
	NonBlockingSwapBuffers="$var"
fi

echo -n "\nPrevent device from going to suspend mode while application is running (y/n) ($InhibitSuspend): "
read var
if [ -n "$var" ] ; then
        InhibitSuspend="$var"
fi


echo -n "\nRedefine common keys to SDL keysyms: TOUCHSCREEN SEARCH/CALL/DPAD_CENTER VOLUMEUP VOLUMEDOWN MENU BACK CAMERA ENTER DEL"
echo -n "\nMENU and BACK hardware keys and TOUCHSCREEN virtual 'key' are available on all devices, other keys may be absent"
echo -n "\nThe same key values are used if touchscreen keyboard is enabled, except for MENU and BACK\n($RedefinedKeys)\n: "
read var
if [ -n "$var" ] ; then
	RedefinedKeys="$var"
fi

echo -n "\nNumber of virtual keyboard keys (currently 7 is maximum) ($AppTouchscreenKeyboardKeysAmount): "
read var
if [ -n "$var" ] ; then
	AppTouchscreenKeyboardKeysAmount="$var"
fi

echo -n "\nNumber of virtual keyboard keys that support autofire (currently 2 is maximum) ($AppTouchscreenKeyboardKeysAmountAutoFire): "
read var
if [ -n "$var" ] ; then
	AppTouchscreenKeyboardKeysAmountAutoFire="$var"
fi

echo -n "\nEnable multi-ABI binary, with hardware FPU support - \nit will also work on old devices, but .apk size is 2x bigger (y) or (n) ($MultiABI): "
read var
if [ -n "$var" ] ; then
	MultiABI="$var"
fi

echo -n "\nApplication version code (integer) ($AppVersionCode): "
read var
if [ -n "$var" ] ; then
	AppVersionCode="$var"
fi

echo -n "\nApplication user-visible version name (string) ($AppVersionName): "
read var
if [ -n "$var" ] ; then
	AppVersionName="$var"
fi

echo -n "\nOptional shared libraries to compile - removing some of them will save space\nMP3 support by libMAD is encumbered by patents and libMAD is GPL-ed\n"
grep 'Available libraries:' project/jni/Application.mk 
grep 'depends on' project/jni/Application.mk
echo -n "Current: $CompiledLibraries\n\n: "
read var
if [ -n "$var" ] ; then
	CompiledLibraries="$var"
fi

echo -n "\nApplication uses custom build script AndroidBuild.sh instead of Android.mk (y) or (n) ($CustomBuildScript): "
read var
if [ -n "$var" ] ; then
	CustomBuildScript="$var"
fi

echo -n "\nAditional CFLAGS for application ($AppCflags): "
read var
if [ -n "$var" ] ; then
	AppCflags="$var"
fi

echo -n "\nAditional LDFLAGS for application ($AppLdflags): "
read var
if [ -n "$var" ] ; then
	AppLdflags="$var"
fi

echo -n "\nBuild only following subdirs (empty will build all dirs) ($AppSubdirsBuild): "
read var
if [ -n "$var" ] ; then
	AppSubdirsBuild="$var"
fi

echo -n "\nApplication requires C++ RTTI and exceptions - you will need CrystaX toolchain installed at\n"
echo -n `which ndk-build | sed 's@/[^/]*/ndk-build@/android-ndk-r4-crystax@'` "(y) or (n) ($AppUseCrystaXToolchain): "
read var
if [ -n "$var" ] ; then
	AppUseCrystaXToolchain="$var"
fi

echo -n "\nHere you may type some short readme text that will be shown when app data is downloaded."
echo -n "\nCurrent text:\n"
echo -n "`echo $ReadmeText | tr '^' '\\n'`"
echo -n "\n\nNew text (empty line to finish):\n\n"

ReadmeText1=""
while true; do
	read var
	if [ -n "$var" ] ; then
		ReadmeText1="$ReadmeText1^$var"
	else
		break
	fi
done
if [ -n "$ReadmeText1" ] ; then
	ReadmeText="$ReadmeText1"
fi

fi # AUTO

echo

cat /dev/null > AndroidAppSettings.cfg
echo "# The application settings for Android libSDL port" >> AndroidAppSettings.cfg
echo AppSettingVersion=$CHANGE_APP_SETTINGS_VERSION >> AndroidAppSettings.cfg
echo LibSdlVersion=$LibSdlVersion >> AndroidAppSettings.cfg
echo AppName=\"$AppName\" >> AndroidAppSettings.cfg
echo AppFullName=$AppFullName >> AndroidAppSettings.cfg
echo ScreenOrientation=$ScreenOrientation >> AndroidAppSettings.cfg
echo InhibitSuspend=$InhibitSuspend >> AndroidAppSettings.cfg
echo AppDataDownloadUrl=\"$AppDataDownloadUrl\" >> AndroidAppSettings.cfg
echo SdlVideoResize=$SdlVideoResize >> AndroidAppSettings.cfg
echo SdlVideoResizeKeepAspect=$SdlVideoResizeKeepAspect >> AndroidAppSettings.cfg
echo NeedDepthBuffer=$NeedDepthBuffer >> AndroidAppSettings.cfg
echo AppUsesMouse=$AppUsesMouse >> AndroidAppSettings.cfg
echo AppNeedsArrowKeys=$AppNeedsArrowKeys >> AndroidAppSettings.cfg
echo AppUsesJoystick=$AppUsesJoystick >> AndroidAppSettings.cfg
echo AppUsesMultitouch=$AppUsesMultitouch >> AndroidAppSettings.cfg
echo NonBlockingSwapBuffers=$NonBlockingSwapBuffers >> AndroidAppSettings.cfg
echo RedefinedKeys=\"$RedefinedKeys\" >> AndroidAppSettings.cfg
echo AppTouchscreenKeyboardKeysAmount=$AppTouchscreenKeyboardKeysAmount >> AndroidAppSettings.cfg
echo AppTouchscreenKeyboardKeysAmountAutoFire=$AppTouchscreenKeyboardKeysAmountAutoFire >> AndroidAppSettings.cfg
echo MultiABI=$MultiABI >> AndroidAppSettings.cfg
echo AppVersionCode=$AppVersionCode >> AndroidAppSettings.cfg
echo AppVersionName=\"$AppVersionName\" >> AndroidAppSettings.cfg
echo CompiledLibraries=\"$CompiledLibraries\" >> AndroidAppSettings.cfg
echo CustomBuildScript=$CustomBuildScript >> AndroidAppSettings.cfg
echo AppCflags=\'$AppCflags\' >> AndroidAppSettings.cfg
echo AppLdflags=\'$AppLdflags\' >> AndroidAppSettings.cfg
echo AppSubdirsBuild=\'$AppSubdirsBuild\' >> AndroidAppSettings.cfg
echo AppUseCrystaXToolchain=$AppUseCrystaXToolchain >> AndroidAppSettings.cfg
echo ReadmeText=\'$ReadmeText\' >> AndroidAppSettings.cfg

AppShortName=`echo $AppName | sed 's/ //g'`
DataPath="$AppFullName"
AppFullNameUnderscored=`echo $AppFullName | sed 's/[.]/_/g'`
AppSharedLibrariesPath=/data/data/$AppFullName/lib
ScreenOrientation1=portrait
HorizontalOrientation=false
if [ "$ScreenOrientation" = "h" ] ; then
	ScreenOrientation1=landscape
	HorizontalOrientation=true
fi
AppDataDownloadUrl1="`echo $AppDataDownloadUrl | sed 's/[&]/%26/g'`"
if [ "$SdlVideoResize" = "y" ] ; then
	SdlVideoResize=1
else
	SdlVideoResize=0
fi
if [ "$SdlVideoResizeKeepAspect" = "y" ] ; then
        SdlVideoResizeKeepAspect=1
else
        SdlVideoResizeKeepAspect=0
fi

if [ "$InhibitSuspend" = "y" ] ; then
        InhibitSuspend=true
else
        InhibitSuspend=false
fi


if [ "$NeedDepthBuffer" = "y" ] ; then
	NeedDepthBuffer=true
else
	NeedDepthBuffer=false
fi
MouseKeycode=UNKNOWN
if [ "$AppUsesMouse" = "y" ] ; then
	AppUsesMouse=true
elif [ "$AppUsesMouse" = "n" ] ; then
	AppUsesMouse=false
else
	MouseKeycode=$AppUsesMouse
	AppUsesMouse=false
fi
if [ "$AppNeedsArrowKeys" = "y" ] ; then
	AppNeedsArrowKeys=true
else
	AppNeedsArrowKeys=false
fi
if [ "$AppUsesJoystick" = "y" ] ; then
	AppUsesJoystick=true
else
	AppUsesJoystick=false
fi
if [ "$AppUsesMultitouch" = "y" ] ; then
	AppUsesMultitouch=true
else
	AppUsesMultitouch=false
fi

if [ "$NonBlockingSwapBuffers" = "y" ] ; then
	NonBlockingSwapBuffers=true
else
	NonBlockingSwapBuffers=false
fi

RedefinedKeycodes="-DSDL_ANDROID_KEYCODE_MOUSE=$MouseKeycode"
KEY2=0
for KEY in $RedefinedKeys; do
	RedefinedKeycodes="$RedefinedKeycodes -DSDL_ANDROID_KEYCODE_$KEY2=$KEY"
	KEY2=`expr $KEY2 '+' 1`
done

if [ "$MultiABI" = "y" ] ; then
	MultiABI="armeabi armeabi-v7a"
else
	MultiABI="armeabi"
fi
LibrariesToLoad="System.loadLibrary(\\\"sdl\\\");"
for lib in $CompiledLibraries; do
	LibrariesToLoad="$LibrariesToLoad System.loadLibrary(\\\"$lib\\\");"
done

if [ "$CustomBuildScript" = "n" ] ; then
	CustomBuildScript=
fi

ReadmeText="`echo $ReadmeText | sed 's/\"/\\\\\\\\\"/g' | sed 's/[&%]//g'`"

echo Creating symlink to libSDL
if [ "`readlink project/jni/sdl`" '!=' "../sdl/sdl-$LibSdlVersion" ]; then
	rm -f project/jni/sdl
	ln -s ../sdl/sdl-$LibSdlVersion project/jni/sdl
fi

echo Patching project/AndroidManifest.xml
cat project/AndroidManifest.xml | \
	sed "s/package=.*/package=\"$AppFullName\"/" | \
	sed "s/android:screenOrientation=.*/android:screenOrientation=\"$ScreenOrientation1\"/" | \
	sed "s^android:versionCode=.*^android:versionCode=\"$AppVersionCode\"^" | \
	sed "s^android:versionName=.*^android:versionName=\"$AppVersionName\"^" > \
	project/AndroidManifest.xml.1
mv -f project/AndroidManifest.xml.1 project/AndroidManifest.xml

for F in project/src/*.java; do
	echo Patching $F
	cat $F | \
		sed "s/package .*;/package $AppFullName;/" > \
		$F.1
	mv -f $F.1 $F
done

echo Patching project/src/Globals.java
cat project/src/Globals.java | \
	sed "s/public static String ApplicationName = .*;/public static String ApplicationName = \"$AppShortName\";/" | \
	sed "s@public static String DataDownloadUrl = .*@public static String DataDownloadUrl = \"$AppDataDownloadUrl1\";@" | \
	sed "s/public static boolean NeedDepthBuffer = .*;/public static boolean NeedDepthBuffer = $NeedDepthBuffer;/" | \
	sed "s/public static boolean HorizontalOrientation = .*;/public static boolean HorizontalOrientation = $HorizontalOrientation;/" | \
        sed "s/public static boolean InhibitSuspend = .*;/public static boolean InhibitSuspend = $InhibitSuspend;/" | \
	sed "s/public static boolean AppUsesMouse = .*;/public static boolean AppUsesMouse = $AppUsesMouse;/" | \
	sed "s/public static boolean AppNeedsArrowKeys = .*;/public static boolean AppNeedsArrowKeys = $AppNeedsArrowKeys;/" | \
	sed "s/public static boolean AppUsesJoystick = .*;/public static boolean AppUsesJoystick = $AppUsesJoystick;/" | \
	sed "s/public static boolean AppUsesMultitouch = .*;/public static boolean AppUsesMultitouch = $AppUsesMultitouch;/" | \
	sed "s/public static boolean NonBlockingSwapBuffers = .*;/public static boolean NonBlockingSwapBuffers = $NonBlockingSwapBuffers;/" | \
	sed "s/public static int AppTouchscreenKeyboardKeysAmount = .*;/public static int AppTouchscreenKeyboardKeysAmount = $AppTouchscreenKeyboardKeysAmount;/" | \
	sed "s/public static int AppTouchscreenKeyboardKeysAmountAutoFire = .*;/public static int AppTouchscreenKeyboardKeysAmountAutoFire = $AppTouchscreenKeyboardKeysAmountAutoFire;/" | \
	sed "s%public static String ReadmeText = .*%public static String ReadmeText = \"$ReadmeText\".replace(\"^\",\"\\\n\");%" | \
	sed "s/public LoadLibrary() .*/public LoadLibrary() { $LibrariesToLoad };/" > \
	project/src/Globals.java.1
mv -f project/src/Globals.java.1 project/src/Globals.java

echo Patching project/jni/Android.mk
cat project/jni/Android.mk | \
	sed "s/SDL_JAVA_PACKAGE_PATH := .*/SDL_JAVA_PACKAGE_PATH := $AppFullNameUnderscored/" | \
	sed "s^SDL_CURDIR_PATH := .*^SDL_CURDIR_PATH := $DataPath^" | \
	sed "s^SDL_VIDEO_RENDER_RESIZE := .*^SDL_VIDEO_RENDER_RESIZE := $SdlVideoResize^" | \
        sed "s^SDL_VIDEO_RENDER_RESIZE_KEEP_ASPECT := .*^SDL_VIDEO_RENDER_RESIZE_KEEP_ASPECT := $SdlVideoResizeKeepAspect^" | \
	sed "s^COMPILED_LIBRARIES := .*^COMPILED_LIBRARIES := $CompiledLibraries^" | \
	sed "s^APPLICATION_ADDITIONAL_CFLAGS :=.*^APPLICATION_ADDITIONAL_CFLAGS := $AppCflags^" | \
	sed "s^APPLICATION_ADDITIONAL_LDFLAGS :=.*^APPLICATION_ADDITIONAL_LDFLAGS := $AppLdflags^" | \
	sed "s^SDL_ADDITIONAL_CFLAGS :=.*^SDL_ADDITIONAL_CFLAGS := $RedefinedKeycodes^" | \
	sed "s^APPLICATION_SUBDIRS_BUILD :=.*^APPLICATION_SUBDIRS_BUILD := $AppSubdirsBuild^" | \
	sed "s^APPLICATION_CUSTOM_BUILD_SCRIPT :=.*^APPLICATION_CUSTOM_BUILD_SCRIPT := $CustomBuildScript^" > \
	project/jni/Android.mk.1
if [ -n "`diff -w project/jni/Android.mk.1 project/jni/Android.mk`" ] ; then
	mv -f project/jni/Android.mk.1 project/jni/Android.mk
else
	rm -rf project/jni/Android.mk.1
fi

echo Patching project/jni/Application.mk
cat project/jni/Application.mk | \
	sed "s/APP_MODULES := .*/APP_MODULES := application sdl sdl_main stlport tremor png jpeg freetype $CompiledLibraries/" | \
	sed "s/APP_ABI := .*/APP_ABI := $MultiABI/" > \
	project/jni/Application.mk.1
if [ -n "`diff -w project/jni/Application.mk.1 project/jni/Application.mk`" ] ; then
	mv -f project/jni/Application.mk.1 project/jni/Application.mk
else
	rm -rf project/jni/Application.mk.1
fi

echo Patching project/res/values/strings.xml
cat project/res/values/strings.xml | \
	sed "s^[<]string name=\"app_name\"[>].*^<string name=\"app_name\">$AppName</string>^" > \
	project/res/values/strings.xml.1
mv -f project/res/values/strings.xml.1 project/res/values/strings.xml

echo Forcing rebuild of specific files
rm -rf project/libs/*
for OUT in obj; do
rm -rf project/$OUT/local/*/objs/sdl_main/* project/$OUT/local/*/libsdl_main.so
rm -rf project/$OUT/local/*/libsdl.so
rm -rf project/$OUT/local/*/libstlport.a # Should be re-linked if you're changing toolchain
rm -rf project/$OUT/local/*/objs/sdl/src/*/android
rm -rf project/$OUT/local/*/objs/sdl/src/video/SDL_video.o
rm -rf project/$OUT/local/*/objs/sdl/SDL_renderer_gles.o
if [ "$LibSdlVersionOld" '!=' "$LibSdlVersion" ]; then
	# Internal types are different in SDL 1.2 and 1.3, namely SDL_Rect, so all libs using it have to be recompiled
	rm -rf project/$OUT/local/*/objs/sdl* project/$OUT/local/*/libsdl*
	rm -rf project/$OUT/local/*/objs/application project/$OUT/local/*/libapplication.so
fi
# Do not rebuild libraries that do not need that
# find project/$OUT/local -name "*.[oa]" -exec touch '{}' \;
done

echo Done
