#!/usr/bin/env bash

set -e
AUTO=a
CHANGED=
JAVA_SRC_PATH=project/java
[ -z "$ANDROID_SDK_ROOT" ] && ANDROID_SDK_ROOT="$ANDROID_HOME"

if [ "X$1" = "X-a" ]; then
	AUTO=a
	shift
fi
if [ "X$1" = "X-v" ]; then
	AUTO=v
	shift
fi
if [ "X$1" = "X-u" ]; then
	CHANGED=1
	AUTO=a
	shift
fi
if [ "X$1" = "X-h" ]; then
	echo "Usage: $0 [-a] [-v] [-u]"
	echo "       -a: auto-update project files without asking questions, it's the default action"
	echo "       -v: ask for new version number on terminal"
	echo "       -u: update AndroidAppSettings.cfg, this may add new config options to it"
	exit
fi

if [ "$#" -gt 0 ]; then
	echo "Switching build target to $1"
	if [ -e project/jni/application/$1 ]; then
		rm -f project/jni/application/src
		ln -s "$1" project/jni/application/src
	else
		echo "Error: no app $1 under project/jni/application"
		echo "Available applications:"
		cd project/jni/application
		for f in *; do
			if [ -e "$f/AndroidAppSettings.cfg" ]; then
				echo "$f"
			fi
		done
		exit 1
	fi
	shift
fi

source ./AndroidAppSettings.cfg

var=""

if [ -n "${APP_FULL_NAME}" ]; then
	echo ${APP_FULL_NAME}
	AppFullName="${APP_FULL_NAME}"
	CHANGED=1
fi

if [ "$CompatibilityHacks" = y ]; then
	SwVideoMode=y
fi

if [ "$SwVideoMode" = "y" ]; then
	NeedDepthBuffer=n
	NeedStencilBuffer=n
	NeedGles2=n
	NeedGles3=n
fi


if [ "$AppUsesJoystick" != "y" ]; then
	AppUsesSecondJoystick=n
fi

MenuOptionsAvailable=
for FF in Menu MenuMisc MenuMouse MenuKeyboard ; do
	MenuOptionsAvailable1=`grep 'extends Menu' $JAVA_SRC_PATH/Settings$FF.java | sed "s/.* class \(.*\) extends .*/Settings$FF.\1/" | tr '\n' ' '`
	MenuOptionsAvailable="$MenuOptionsAvailable $MenuOptionsAvailable1"
done

FirstStartMenuOptionsDefault='new SettingsMenuMisc.ShowReadme(), (AppUsesMouse \&\& \! ForceRelativeMouseMode \? new SettingsMenuMouse.DisplaySizeConfig(true) : new SettingsMenu.DummyMenu()), new SettingsMenuMisc.OptionalDownloadConfig(true), new SettingsMenuMisc.GyroscopeCalibration()'

if [ -z "$CompatibilityHacksForceScreenUpdate" ]; then
	CompatibilityHacksForceScreenUpdate=$CompatibilityHacks
fi

if [ -z "$CompatibilityHacksForceScreenUpdateMouseClick" ]; then
	CompatibilityHacksForceScreenUpdateMouseClick=n
fi

if [ -z "$TouchscreenKeysTheme" ]; then
	TouchscreenKeysTheme=2
fi


if [ -z "$AppVersionCode" -o "-$AUTO" != "-a" ]; then
	echo
	echo -n "Application version code (integer) ($AppVersionCode): "
	read var
	if [ -n "$var" ] ; then
		AppVersionCode="$var"
		CHANGED=1
	fi
fi

if [ -z "$AppVersionName" -o "-$AUTO" != "-a" ]; then
	echo
	echo -n "Application user-visible version name (string) ($AppVersionName): "
	read var
	if [ -n "$var" ] ; then
		AppVersionName="$var"
		CHANGED=1
	fi
fi

if [ -z "$ResetSdlConfigForThisVersion" -o "-$AUTO" != "-a" ]; then
	echo
	echo -n "Reset SDL config when updating application to the new version (y) / (n) ($ResetSdlConfigForThisVersion): "
	read var
	if [ -n "$var" ] ; then
		ResetSdlConfigForThisVersion="$var"
		CHANGED=1
	fi
fi

if [ "-$AUTO" != "-a" ]; then
	echo
	echo -n "Delete application data files when upgrading (specify file/dir paths separated by spaces): ($DeleteFilesOnUpgrade): "
	read var
	if [ -n "$var" ] ; then
		DeleteFilesOnUpgrade="$var"
		CHANGED=1
	fi
fi

# Compatibility - if RedefinedKeysScreenGestures is empty, copy keycodes from RedefinedKeysScreenKb
KEY2=0
if [ -z "$RedefinedKeysScreenGestures" ] ; then
	RedefinedKeysScreenGestures="$(
		for KEY in $RedefinedKeysScreenKb; do
			if [ $KEY2 -ge 6 ] && [ $KEY2 -le 9 ]; then
				echo -n $KEY ' '
			fi
			KEY2=$(expr $KEY2 '+' 1)
		done
	)"
	RedefinedKeysScreenKb="$(
		for KEY in $RedefinedKeysScreenKb; do
			if [ $KEY2 -lt 6 ] || [ $KEY2 -gt 9 ]; then
				echo -n $KEY ' '
			fi
			KEY2=$(expr $KEY2 '+' 1)
		done
	)"
fi

if [ -n "$CHANGED" ]; then
cat /dev/null > AndroidAppSettings.cfg
cat <<EOF >./AndroidAppSettings.cfg
# The application settings for Android libSDL port

# Specify application name (e.x. My Application)
AppName="$AppName"

# Specify reversed site name of application (e.x. com.mysite.myapp)
AppFullName=$AppFullName

# Application version code (integer)
AppVersionCode=$AppVersionCode

# Application user-visible version name (string)
AppVersionName="$AppVersionName"

# Specify path to download application data in zip archive in the form "Description|URL|MirrorURL^Description2|URL2|MirrorURL2^...'
# If you'll start Description with '!' symbol it will be enabled by default, '!!' will also hide the entry from the menu, so it cannot be disabled
# If the URL in in the form ':dir/file.dat:http://URL/' it will be downloaded as binary BLOB to the application dir and not unzipped
# If the URL does not contain 'http://' or 'https://', it is treated as file from 'project/jni/application/src/AndroidData' dir -
# these files are put inside .apk package by the build system
# You can specify Google Play expansion files in the form 'obb:main.12345' or 'obb:patch.12345' where 12345 is the app version for the obb file
# You can mount expansion files created with jobb tool if you put 'mnt:main.12345' or 'mnt:patch.12345'
# The mount directory will be returned by calling getenv("ANDROID_OBB_MOUNT_DIR")
# You can use .zip.xz archives for better compression, but you need to add 'lzma' to CompiledLibraries
# Generate .zip.xz files like this: zip -0 -r data.zip your-data/* ; xz -8 data.zip
AppDataDownloadUrl="$AppDataDownloadUrl"

# Reset SDL config when updating application to the new version (y) / (n)
ResetSdlConfigForThisVersion=$ResetSdlConfigForThisVersion

# Delete application data files when upgrading (specify file/dir paths separated by spaces)
DeleteFilesOnUpgrade="$DeleteFilesOnUpgrade"

# Here you may type readme text, which will be shown during startup. Format is:
# Text in English, use \\\\\\\\n to separate lines (that's four backslashes)^de:Text in Deutsch^ru:Text in Russian^button:Button that will open some URL:http://url-to-open/
ReadmeText='$ReadmeText' | sed 's/\\\\n/\\\\\\\\n/g'

# libSDL version to use (1.2/2)
LibSdlVersion=$LibSdlVersion

# Specify screen orientation: (v)ertical/(p)ortrait or (h)orizontal/(l)andscape
ScreenOrientation=$ScreenOrientation

# Video color depth - 16 BPP is the fastest and supported for all modes, 24 bpp is supported only
# with SwVideoMode=y, SDL_OPENGL mode supports everything. (16)/(24)/(32)
VideoDepthBpp=$VideoDepthBpp

# Enable OpenGL depth buffer (needed only for 3-d applications, small speed decrease) (y) or (n)
NeedDepthBuffer=$NeedDepthBuffer

# Enable OpenGL stencil buffer (needed only for 3-d applications, small speed decrease) (y) or (n)
NeedStencilBuffer=$NeedStencilBuffer

# Use GLES 2.x context
# you need this option only if you're developing 3-d app (y) or (n)
NeedGles2=$NeedGles2

# Use GLES 3.x context
# you need this option only if you're developing 3-d app (y) or (n)
NeedGles3=$NeedGles3

# Use gl4es library for provide OpenGL 1.x functionality to OpenGL ES accelerated cards (y) or (n)
UseGl4es=$UseGl4es

# Application uses software video buffer - you're calling SDL_SetVideoMode() without SDL_HWSURFACE and without SDL_OPENGL,
# this will allow small speed optimization. Enable this even when you're using SDL_HWSURFACE. (y) or (n)
SwVideoMode=$SwVideoMode

# Application video output will be resized to fit into native device screen (y)/(n)
SdlVideoResize=$SdlVideoResize

# Application resizing will keep 4:3 aspect ratio, with black bars at sides (y)/(n)
SdlVideoResizeKeepAspect=$SdlVideoResizeKeepAspect

# Do not allow device to sleep when the application is in foreground, set this for video players or apps which use accelerometer
InhibitSuspend=$InhibitSuspend

# Create Android service, so the app is less likely to be killed while in background
CreateService=$CreateService

# Application does not call SDL_Flip() or SDL_UpdateRects() appropriately, or draws from non-main thread -
# enabling the compatibility mode will force screen update every 100 milliseconds, which is laggy and inefficient (y) or (n)
CompatibilityHacksForceScreenUpdate=$CompatibilityHacksForceScreenUpdate

# Application does not call SDL_Flip() or SDL_UpdateRects() after mouse click (ScummVM and all Amiga emulators do that) -
# force screen update by moving mouse cursor a little after each click (y) or (n)
CompatibilityHacksForceScreenUpdateMouseClick=$CompatibilityHacksForceScreenUpdateMouseClick

# Application initializes SDL audio/video inside static constructors (which is bad, you won't be able to run ndk-gdb) (y)/(n)
CompatibilityHacksStaticInit=$CompatibilityHacksStaticInit

# On-screen Android soft text input emulates hardware keyboard, this will only work with Hackers Keyboard app (y)/(n)
CompatibilityHacksTextInputEmulatesHwKeyboard=$CompatibilityHacksTextInputEmulatesHwKeyboard

# Built-in text input keyboards with custom layouts for emulators, requires CompatibilityHacksTextInputEmulatesHwKeyboard=y
# 0 or empty - standard Android keyboard
# 1 - Simple QWERTY keyboard, no function keys, no arrow keys
# 2 - Commodore 64 keyboard
# 3 - Amiga keyboard
# 4 - Atari800 keyboard
TextInputKeyboard=$TextInputKeyboard

# Hack for broken devices: prevent audio chopping, by sleeping a bit after pushing each audio chunk (y)/(n)
CompatibilityHacksPreventAudioChopping=$CompatibilityHacksPreventAudioChopping

# Hack for broken apps: application ignores audio buffer size returned by SDL (y)/(n)
CompatibilityHacksAppIgnoresAudioBufferSize=$CompatibilityHacksAppIgnoresAudioBufferSize

# Hack for VCMI: preload additional shared libraries before aplication start
CompatibilityHacksAdditionalPreloadedSharedLibraries="$CompatibilityHacksAdditionalPreloadedSharedLibraries"

# Hack for Free Heroes 2, which redraws the screen inside SDL_PumpEvents(): slow and compatible SDL event queue -
# do not use it with accelerometer/gyroscope, or your app may freeze at random (y)/(n)
CompatibilityHacksSlowCompatibleEventQueue=$CompatibilityHacksSlowCompatibleEventQueue

# Save and restore OpenGL state when drawing on-screen keyboard for apps that use SDL_OPENGL
CompatibilityHacksTouchscreenKeyboardSaveRestoreOpenGLState=$CompatibilityHacksTouchscreenKeyboardSaveRestoreOpenGLState

# Application uses SDL_UpdateRects() properly, and does not draw in any region outside those rects.
# This improves drawing speed, but I know only one application that does that, and it's written by me (y)/(n)
CompatibilityHacksProperUsageOfSDL_UpdateRects=$CompatibilityHacksProperUsageOfSDL_UpdateRects

# Application uses mouse (y) or (n), this will show mouse emulation dialog to the user
AppUsesMouse=$AppUsesMouse

# Application needs two-button mouse, will also enable advanced point-and-click features (y) or (n)
AppNeedsTwoButtonMouse=$AppNeedsTwoButtonMouse

# Right mouse button can do long-press/drag&drop action, necessary for some games (y) or (n)
# If you disable it, swiping with two fingers will send mouse wheel events
RightMouseButtonLongPress=$RightMouseButtonLongPress

# Show SDL mouse cursor, for applications that do not draw cursor at all (y) or (n)
ShowMouseCursor=$ShowMouseCursor

# Screen follows mouse cursor, when it's covered by soft keyboard, this works only in software video mode (y) or (n)
ScreenFollowsMouse=$ScreenFollowsMouse

# Generate more touch events, by default SDL generates one event per one video frame, this is useful for drawing apps (y) or (n)
GenerateSubframeTouchEvents=$GenerateSubframeTouchEvents

# Force relative (laptop) mouse movement mode, useful when both on-screen keyboard and mouse are needed (y) or (n)
ForceRelativeMouseMode=$ForceRelativeMouseMode

# Show on-screen dpad/joystick, that will act as arrow keys (y) or (n)
AppNeedsArrowKeys=$AppNeedsArrowKeys

# On-screen dpad/joystick will appear under finger when it touches the screen (y) or (n)
# Joystick always follows finger, so moving mouse requires touching the screen with other finger
FloatingScreenJoystick=$FloatingScreenJoystick

# Application needs text input (y) or (n), enables button for text input on screen
AppNeedsTextInput=$AppNeedsTextInput

# Application uses joystick (y) or (n), the on-screen DPAD will be used as joystick 0 axes 0-1
# This will disable AppNeedsArrowKeys option
AppUsesJoystick=$AppUsesJoystick

# Application uses second on-screen joystick, as SDL joystick 0 axes 2-3 (y)/(n)
AppUsesSecondJoystick=$AppUsesSecondJoystick

# Application uses third on-screen joystick, as SDL joystick 0 axes 20-21 (y)/(n)
AppUsesThirdJoystick=$AppUsesThirdJoystick

# Application uses accelerometer (y) or (n), the accelerometer will be used as joystick 1 axes 0-1 and 5-7
AppUsesAccelerometer=$AppUsesAccelerometer

# Application uses gyroscope (y) or (n), the gyroscope will be used as joystick 1 axes 2-4
AppUsesGyroscope=$AppUsesGyroscope

# Application uses orientation sensor (y) or (n), reported as joystick 1 axes 8-10
AppUsesOrientationSensor=$AppUsesOrientationSensor

# Use gyroscope to move mouse cursor (y) or (n), it eats battery, and can be disabled in settings, do not use with AppUsesGyroscope setting
MoveMouseWithGyroscope=$MoveMouseWithGyroscope

# Application uses multitouch (y) or (n), multitouch events are passed as SDL_JOYBALLMOTION events for the joystick 0
AppUsesMultitouch=$AppUsesMultitouch

# Application records audio (it will use any available source, such a s microphone)
# API is defined in file SDL_android.h: int SDL_ANDROID_OpenAudioRecording(SDL_AudioSpec *spec); void SDL_ANDROID_CloseAudioRecording(void);
# This option will add additional permission to Android manifest (y)/(n)
AppRecordsAudio=$AppRecordsAudio

# Application needs read/write access SD card. Always disable it, unless you want to access user photos and downloads. (y) / (n)
AccessSdCard=$AccessSdCard

# Application needs to read it's own OBB file. Enable this if you are using Play Store expansion files. (y) / (n)
ReadObbFile=$ReadObbFile

# Application needs Internet access. If you disable it, you'll have to bundle all your data files inside .apk (y) / (n)
AccessInternet=$AccessInternet

# Immersive mode - Android will hide on-screen Home/Back keys. Looks bad if you invoke Android keyboard. (y) / (n)
ImmersiveMode=$ImmersiveMode

# Draw in the display cutout area. (y) / (n)
DrawInDisplayCutout=$DrawInDisplayCutout

# Hide Android system mouse cursor image when USB mouse is attached (y) or (n) - the app must draw it's own mouse cursor
HideSystemMousePointer=$HideSystemMousePointer

# Application implements Android-specific routines to put to background, and will not draw anything to screen
# between SDL_ACTIVEEVENT lost / gained notifications - you should check for them
# rigth after SDL_Flip(), if (n) then SDL_Flip() will block till app in background (y) or (n)
# This option is reported to be buggy, sometimes failing to restore video state
NonBlockingSwapBuffers=$NonBlockingSwapBuffers

# Redefine common hardware keys to SDL keysyms
# BACK hardware key is available on all devices, MENU is available on pre-ICS devices, other keys may be absent
# SEARCH and CALL by default return same keycode as DPAD_CENTER - one of those keys is available on most devices
# Use word NO_REMAP if you want to preserve native functionality for certain key (volume keys are 3-rd and 4-th)
# Keys: TOUCHSCREEN (works only when AppUsesMouse=n), DPAD_CENTER/SEARCH, VOLUMEUP, VOLUMEDOWN, MENU, BACK, CAMERA
RedefinedKeys="$RedefinedKeys"

# Number of virtual keyboard keys - currently 12 keys is the maximum
AppTouchscreenKeyboardKeysAmount=$AppTouchscreenKeyboardKeysAmount

# Define SDL keysyms for multitouch gestures - pinch-zoom in, pinch-zoom out, rotate left, rotate right
RedefinedKeysScreenGestures="$RedefinedKeysScreenGestures"

# Redefine on-screen keyboard keys to SDL keysyms - currently 12 keys is the maximum
RedefinedKeysScreenKb="$RedefinedKeysScreenKb"

# Names for on-screen keyboard keys, such as Fire, Jump, Run etc, separated by spaces, they are used in SDL config menu
RedefinedKeysScreenKbNames="$RedefinedKeysScreenKbNames"

# On-screen keys theme
# 0 = Ultimate Droid by Sean Stieber (green, with cross joystick)
# 1 = Simple Theme by Beholder (white, with cross joystick)
# 2 = Sun by Sirea (yellow, with round joystick)
# 3 = Keen by Gerstrong (multicolor, with round joystick)
# 4 = Retro by Santiago Radeff (red/white, with cross joystick)
# 5 = GameBoy from RetroArch
# 6 = PlayStation from RetroArch
# 7 = SuperNintendo from RetroArch
# 8 = DualShock from RetroArch
# 9 = Nintendo64 from RetroArch
TouchscreenKeysTheme=$TouchscreenKeysTheme

# Redefine gamepad keys to SDL keysyms, button order is:
# A B X Y L1 R1 L2 R2 LThumb RThumb Start Select Up Down Left Right LThumbUp LThumbDown LThumbLeft LThumbRight RThumbUp RThumbDown RThumbLeft RThumbRight
RedefinedKeysGamepad="$RedefinedKeysGamepad"

# Redefine keys for the second gamepad, same as the first gamepad if not set:
RedefinedKeysSecondGamepad="$RedefinedKeysSecondGamepad"

# Redefine keys for the third gamepad, same as the first gamepad if not set:
RedefinedKeysThirdGamepad="$RedefinedKeysThirdGamepad"

# Redefine keys for the fourth gamepad, same as the first gamepad if not set:
RedefinedKeysFourthGamepad="$RedefinedKeysFourthGamepad"

# How long to show startup menu button, in msec, 0 to disable startup menu
StartupMenuButtonTimeout=$StartupMenuButtonTimeout

# Menu items to hide from startup menu, available menu items (SDL 1.2 only):
# $MenuOptionsAvailable
HiddenMenuOptions='$HiddenMenuOptions'

# Menu items to show at startup - this is Java code snippet, leave empty for default
# $FirstStartMenuOptionsDefault
# Available menu items:
# $MenuOptionsAvailable
FirstStartMenuOptions='$FirstStartMenuOptions'

# Minimum amount of RAM application requires, in Mb, SDL will print warning to user if it's lower
AppMinimumRAM=$AppMinimumRAM

# GCC version, or 'clang' for CLANG
NDK_TOOLCHAIN_VERSION=$NDK_TOOLCHAIN_VERSION

# Android platform version.
# android-16 = Android 4.1, the earliest supported version in NDK r18.
# android-18 = Android 4.3, the first version supporting GLES3.
# android-21 = Android 5.1, the first version with SO_REUSEPORT defined.
APP_PLATFORM=$APP_PLATFORM

# Specify architectures to compile, 'all' or 'y' to compile for all architectures.
# Available architectures: armeabi-v7a arm64-v8a x86 x86_64
MultiABI='$MultiABI'

# Optional shared libraries to compile - removing some of them will save space
# MP3 patents are expired, but libmad license is GPL, not LGPL
# Available libraries: mad (GPL-ed!) sdl_mixer sdl_image sdl_ttf sdl_net sdl_blitpool sdl_gfx sdl_sound intl xml2 lua jpeg png ogg flac tremor vorbis freetype xerces curl theora fluidsynth lzma lzo2 mikmod openal timidity zzip bzip2 yaml-cpp python boost_date_time boost_filesystem boost_iostreams boost_program_options boost_regex boost_signals boost_system boost_thread glu avcodec avdevice avfilter avformat avresample avutil swscale swresample bzip2
# rep 'Available' project/jni/SettingsTemplate.mk
CompiledLibraries="$CompiledLibraries"

# Application uses custom build script AndroidBuild.sh instead of Android.mk (y) or (n)
CustomBuildScript=$CustomBuildScript

# Aditional CFLAGS for application
AppCflags='$AppCflags'

# Aditional C++-specific compiler flags for application, added after AppCflags
AppCppflags='$AppCppflags'

# Additional LDFLAGS for application
AppLdflags='$AppLdflags'

# If application has headers with the same name as system headers, this option tries to fix compiler flags to make it compilable
AppOverlapsSystemHeaders=$AppOverlapsSystemHeaders

# Build only following subdirs (empty will build all dirs, ignored with custom script)
AppSubdirsBuild='$AppSubdirsBuild'

# Exclude these files from build
AppBuildExclude='$AppBuildExclude'

# Application command line parameters, including app name as 0-th param
AppCmdline='$AppCmdline'

# Screen size is used by Google Play to prevent an app to be installed on devices with smaller screens
# Minimum screen size that application supports: (s)mall / (m)edium / (l)arge
MinimumScreenSize=$MinimumScreenSize

# Your AdMob Publisher ID, (n) if you don't want advertisements
AdmobPublisherId=$AdmobPublisherId

# Your AdMob test device ID, to receive a test ad
AdmobTestDeviceId=$AdmobTestDeviceId

# Your AdMob banner size (BANNER/FULL_BANNER/LEADERBOARD/MEDIUM_RECTANGLE/SMART_BANNER/WIDE_SKYSCRAPER/FULL_WIDTH:Height/Width:AUTO_HEIGHT/Width:Height)
AdmobBannerSize=$AdmobBannerSize

# Google Play Game Services application ID, required for cloud saves to work
GooglePlayGameServicesId=$GooglePlayGameServicesId

# The app will open files with following extension, file path will be added to commandline params
AppOpenFileExtension='$AppOpenFileExtension'
EOF
fi

AppShortName=`echo $AppName | sed 's/ //g'`
DataPath="$AppFullName"
AppFullNameUnderscored=`echo $AppFullName | sed 's/[.]/_/g'`
AppSharedLibrariesPath=/data/data/$AppFullName/lib
ScreenOrientation1=sensorPortrait
HorizontalOrientation=false

UsingSdl2=false
if [ "$LibSdlVersion" = "2.0" ] ; then
	LibSdlVersion="2"
fi
if [ "$LibSdlVersion" = "2" ] ; then
	UsingSdl2=true
fi

if [ "$ScreenOrientation" = "h" -o "$ScreenOrientation" = "l" ] ; then
	ScreenOrientation1=sensorLandscape
	HorizontalOrientation=true
fi

AppDataDownloadUrl1="`echo $AppDataDownloadUrl | sed 's/[&]/%26/g'`"

if [ "$SdlVideoResize" = "y" ] ; then
	SdlVideoResize=1
else
	SdlVideoResize=0
fi

if [ "$SdlVideoResizeKeepAspect" = "y" ] ; then
	SdlVideoResizeKeepAspect=true
else
	SdlVideoResizeKeepAspect=false
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

if [ "$NeedStencilBuffer" = "y" ] ; then
	NeedStencilBuffer=true
else
	NeedStencilBuffer=false
fi

if [ "$UseGl4es" = "y" ] ; then
	UseGl4esCFlags=-DUSE_GL4ES=1
else
	UseGl4es=
	UseGl4esCFlags=
fi

if [ "$SwVideoMode" = "y" ] ; then
	SwVideoMode=true
else
	SwVideoMode=false
fi

if [ "$CompatibilityHacksForceScreenUpdate" = "y" ] ; then
	CompatibilityHacksForceScreenUpdate=true
else
	CompatibilityHacksForceScreenUpdate=false
fi

if [ "$CompatibilityHacksForceScreenUpdateMouseClick" = "y" ] ; then
	CompatibilityHacksForceScreenUpdateMouseClick=true
else
	CompatibilityHacksForceScreenUpdateMouseClick=false
fi

if [ "$CompatibilityHacksStaticInit" = "y" ] ; then
	CompatibilityHacksStaticInit=true
else
	CompatibilityHacksStaticInit=false
fi

if [ "$CompatibilityHacksTextInputEmulatesHwKeyboard" = "y" ] ; then
	CompatibilityHacksTextInputEmulatesHwKeyboard=true
else
	CompatibilityHacksTextInputEmulatesHwKeyboard=false
fi

if [ -z "$TextInputKeyboard" ] ; then
	TextInputKeyboard=0
fi

if [ "$CompatibilityHacksPreventAudioChopping" = "y" ] ; then
	CompatibilityHacksPreventAudioChopping=-DSDL_AUDIO_PREVENT_CHOPPING_WITH_DELAY=1
else
	CompatibilityHacksPreventAudioChopping=
fi

if [ "$CompatibilityHacksAppIgnoresAudioBufferSize" = "y" ] ; then
	CompatibilityHacksAppIgnoresAudioBufferSize=-DSDL_AUDIO_APP_IGNORES_RETURNED_BUFFER_SIZE=1
else
	CompatibilityHacksAppIgnoresAudioBufferSize=
fi

if [ "$CompatibilityHacksSlowCompatibleEventQueue" = "y" ]; then
	CompatibilityHacksSlowCompatibleEventQueue=-DSDL_COMPATIBILITY_HACKS_SLOW_COMPATIBLE_EVENT_QUEUE=1
else
	CompatibilityHacksSlowCompatibleEventQueue=
fi

if [ "$CompatibilityHacksTouchscreenKeyboardSaveRestoreOpenGLState" = "y" ]; then
	CompatibilityHacksTouchscreenKeyboardSaveRestoreOpenGLState=-DSDL_TOUCHSCREEN_KEYBOARD_SAVE_RESTORE_OPENGL_STATE=1
else
	CompatibilityHacksTouchscreenKeyboardSaveRestoreOpenGLState=
fi

if [ "$CompatibilityHacksProperUsageOfSDL_UpdateRects" = "y" ]; then
	CompatibilityHacksProperUsageOfSDL_UpdateRects=-DSDL_COMPATIBILITY_HACKS_PROPER_USADE_OF_SDL_UPDATERECTS=1
else
	CompatibilityHacksProperUsageOfSDL_UpdateRects=
fi

if [ "$AppUsesMouse" = "y" ] ; then
	AppUsesMouse=true
else
	AppUsesMouse=false
fi

if [ "$AppNeedsTwoButtonMouse" = "y" ] ; then
	AppNeedsTwoButtonMouse=true
else
	AppNeedsTwoButtonMouse=false
fi

if [ "$RightMouseButtonLongPress" = "n" ] ; then
	RightMouseButtonLongPress=false
else
	RightMouseButtonLongPress=true
fi

if [ "$ForceRelativeMouseMode" = "y" ] ; then
	ForceRelativeMouseMode=true
else
	ForceRelativeMouseMode=false
fi

if [ "$ShowMouseCursor" = "y" ] ; then
	ShowMouseCursor=true
else
	ShowMouseCursor=false
fi

if [ "$ScreenFollowsMouse" = "y" ] ; then
	ScreenFollowsMouse=true
else
	ScreenFollowsMouse=false
fi

if [ "$GenerateSubframeTouchEvents" = "y" ] ; then
	GenerateSubframeTouchEvents=true
else
	GenerateSubframeTouchEvents=false
fi

if [ "$AppNeedsArrowKeys" = "y" ] ; then
	AppNeedsArrowKeys=true
else
	AppNeedsArrowKeys=false
fi

if [ "$FloatingScreenJoystick" = "y" ] ; then
	FloatingScreenJoystick=true
else
	FloatingScreenJoystick=false
fi

if [ "$AppNeedsTextInput" = "y" ] ; then
	AppNeedsTextInput=true
else
	AppNeedsTextInput=false
fi

if [ "$AppUsesJoystick" = "y" ] ; then
	AppUsesJoystick=true
else
	AppUsesJoystick=false
fi

if [ "$AppUsesSecondJoystick" = "y" ] ; then
	AppUsesSecondJoystick=true
else
	AppUsesSecondJoystick=false
fi

if [ "$AppUsesThirdJoystick" = "y" ] ; then
	AppUsesThirdJoystick=true
else
	AppUsesThirdJoystick=false
fi

if [ "$AppUsesAccelerometer" = "y" ] ; then
	AppUsesAccelerometer=true
else
	AppUsesAccelerometer=false
fi

if [ "$AppUsesGyroscope" = "y" ] ; then
	AppUsesGyroscope=true
else
	AppUsesGyroscope=false
fi

if [ "$AppUsesOrientationSensor" = "y" ] ; then
	AppUsesOrientationSensor=true
else
	AppUsesOrientationSensor=false
fi

if [ "$MoveMouseWithGyroscope" = "y" ] ; then
	MoveMouseWithGyroscope=true
else
	MoveMouseWithGyroscope=false
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

if [ "$ResetSdlConfigForThisVersion" = "y" ] ; then
	ResetSdlConfigForThisVersion=true
else
	ResetSdlConfigForThisVersion=false
fi

if [ "$AccessSdCard" = "y" ]; then
	AccessSdCard=true
else
	AccessSdCard=false
fi

KEY2=0
for KEY in $RedefinedKeys; do
	RedefinedKeycodes="$RedefinedKeycodes -DSDL_ANDROID_KEYCODE_$KEY2=$KEY"
	KEY2=`expr $KEY2 '+' 1`
done

KEY2=0
for KEY in $RedefinedKeysScreenGestures; do
	RedefinedSDLScreenGestures="$RedefinedSDLScreenGestures -DSDL_ANDROID_SCREEN_GESTURE_KEYCODE_$KEY2=$KEY"
	KEY2=`expr $KEY2 '+' 1`
done

KEY2=0
for KEY in $RedefinedKeysScreenKb; do
	RedefinedKeycodesScreenKb="$RedefinedKeycodesScreenKb -DSDL_ANDROID_SCREENKB_KEYCODE_$KEY2=$KEY"
	KEY2=`expr $KEY2 '+' 1`
done

KEY2=0
for KEY in $RedefinedKeysGamepad; do
	RedefinedKeycodesGamepad="$RedefinedKeycodesGamepad -DSDL_ANDROID_GAMEPAD_0_KEYCODE_$KEY2=$KEY"
	KEY2=`expr $KEY2 '+' 1`
done

KEY2=0
for KEY in $RedefinedKeysSecondGamepad; do
	RedefinedKeycodesGamepad="$RedefinedKeycodesGamepad -DSDL_ANDROID_GAMEPAD_1_KEYCODE_$KEY2=$KEY"
	KEY2=`expr $KEY2 '+' 1`
done

KEY2=0
for KEY in $RedefinedKeysThirdGamepad; do
	RedefinedKeycodesGamepad="$RedefinedKeycodesGamepad -DSDL_ANDROID_GAMEPAD_2_KEYCODE_$KEY2=$KEY"
	KEY2=`expr $KEY2 '+' 1`
done

KEY2=0
for KEY in $RedefinedKeysFourthGamepad; do
	RedefinedKeycodesGamepad="$RedefinedKeycodesGamepad -DSDL_ANDROID_GAMEPAD_3_KEYCODE_$KEY2=$KEY"
	KEY2=`expr $KEY2 '+' 1`
done

if [ "$APP_PLATFORM" = "" ]; then
	APP_PLATFORM=android-21
fi

if [ "$MultiABI" = "y" ] ; then
	MultiABI="all"
elif [ "$MultiABI" = "n" ] ; then
	MultiABI="armeabi-v7a"
else
	MultiABI="$MultiABI"
fi

LibrariesToLoad="\\\"sdl_native_helpers\\\", \\\"`$UsingSdl2 && echo SDL2 || echo sdl-1.2`\\\""

StaticLibraries="`echo '
include project/jni/SettingsTemplate.mk
all:
	@echo $(APP_AVAILABLE_STATIC_LIBS)
.PHONY: all' | make -s -f -`"
for lib in $CompiledLibraries; do
	process=true
	for lib1 in $StaticLibraries; do
		if [ "$lib" = "$lib1" ]; then process=false; fi
	done
	if $process; then
		LibrariesToLoad="$LibrariesToLoad, \\\"$lib\\\""
	fi
done

MainLibrariesToLoad=""
for lib in $CompatibilityHacksAdditionalPreloadedSharedLibraries; do
	MainLibrariesToLoad="$MainLibrariesToLoad \\\"$lib\\\","
done

if $UsingSdl2; then
	MainLibrariesToLoad="$MainLibrariesToLoad \\\"application\\\""
else
	MainLibrariesToLoad="$MainLibrariesToLoad \\\"application\\\", \\\"sdl_main\\\""
fi

if [ "$CustomBuildScript" = "n" ] ; then
	CustomBuildScript=
fi

HiddenMenuOptions1=""
for F in $HiddenMenuOptions; do
	HiddenMenuOptions1="$HiddenMenuOptions1 new $F(),"
done

FirstStartMenuOptions1=""
for F in $FirstStartMenuOptions; do
	FirstStartMenuOptions1="$FirstStartMenuOptions1 new $F(),"
done

#if [ -z "$FirstStartMenuOptions" ]; then
#	FirstStartMenuOptions="$FirstStartMenuOptionsDefault"
#fi

ReadmeText="`echo $ReadmeText | sed 's/\"/\\\\\\\\\"/g' | sed 's/[&%]//g'`"


SEDI="sed -i"
if uname -s | grep -i "darwin" > /dev/null ; then
	SEDI="sed -i.killme.tmp" # MacOsX version of sed is buggy, and requires a mandatory parameter
fi

rm -rf project/src
mkdir -p project/src

if $UsingSdl2; then
	JAVA_SRC_PATH=project/javaSDL2
fi

cd $JAVA_SRC_PATH
for F in *.java; do
	echo '// DO NOT EDIT THIS FILE - it is automatically generated, ALL YOUR CHANGES WILL BE OVERWRITTEN, edit the file under '$JAVA_SRC_PATH' dir' | cat - $F > ../src/$F
done

for F in ../src/*.java; do
	echo Patching $F
	$SEDI "s/^package .*;/package $AppFullName;/" $F
done

if $UsingSdl2; then
	# Keep package name org.libsdl.app, it's hardcoded inside libSDL2.so
	for F in `ls ../jni/sdl2/android-project/app/src/main/java/org/libsdl/app/`; do
		echo '// DO NOT EDIT THIS FILE - it is automatically generated, ALL YOUR CHANGES WILL BE OVERWRITTEN,' \
			'edit the file under project/jni/sdl2/android-project/app/src/main/java/org/libsdl/app dir' | \
			cat - ../jni/sdl2/android-project/app/src/main/java/org/libsdl/app/$F > ../src/$F
	done
fi

if [ -e ../jni/application/src/java.diff ]; then patch -d ../src --no-backup-if-mismatch < ../jni/application/src/java.diff || exit 1 ; fi
if [ -e ../jni/application/src/java.patch ]; then patch -d ../src --no-backup-if-mismatch < ../jni/application/src/java.patch || exit 1 ; fi
if ls ../jni/application/src/*.java > /dev/null 2>&1; then cp -f ../jni/application/src/*.java ../src ; fi

cd ../..

if $UsingSdl2; then
	ANDROID_MANIFEST_TEMPLATE=project/jni/sdl2/android-project/app/src/main/AndroidManifest.xml
else
	ANDROID_MANIFEST_TEMPLATE=project/AndroidManifestTemplate.xml
fi

echo Patching project/AndroidManifest.xml
cat $ANDROID_MANIFEST_TEMPLATE | \
	sed "s/package=.*//" | \
	sed "s/android:screenOrientation=.*/android:screenOrientation=\"$ScreenOrientation1\"/" | \
	sed "s^android:versionCode=.*^android:versionCode=\"$AppVersionCode\"^" | \
	sed "s^android:versionName=.*^android:versionName=\"$AppVersionName\"^" | \
	sed "s^activity android:name=\"SDLActivity\"^activity android:name=\"MainActivity\"^" > \
	project/AndroidManifest.xml
if [ "$AdmobPublisherId" = "n" -o -z "$AdmobPublisherId" ] ; then
	$SEDI "/==ADMOB==/ d" project/AndroidManifest.xml
	AdmobPublisherId=""
else
	F=$JAVA_SRC_PATH/admob/Advertisement.java
	echo Patching $F
	echo '// DO NOT EDIT THIS FILE - it is automatically generated, edit file under $JAVA_SRC_PATH dir' > project/src/Advertisement.java
	cat $F | sed "s/^package .*;/package $AppFullName;/" >> project/src/Advertisement.java
fi

if [ -z "$ANDROID_NDK_HOME" ]; then
	export ANDROID_NDK_HOME="$(which ndk-build | sed 's@/ndk-build@@')"
fi
if [ -z "$ANDROID_NDK_HOME" ]; then
	echo "Set ANDROID_NDK_HOME env variable, or put ndk-build into your PATH"
	exit 1
fi
NDK_VER=$(echo ${ANDROID_NDK_HOME} | grep -Eo '[^/]+$')

cat project/app/build-template.gradle | \
	sed 's/applicationId .*/applicationId "'"${AppFullName}"'"/' | \
	sed 's/namespace .*/namespace '"'"${AppFullName}"'"'/' | \
	sed 's/ndkVersion .*/ndkVersion "'"${NDK_VER}"'"/' > \
	project/app/build.gradle

echo "-keep class $AppFullName.** { *; }" > project/proguard-local.cfg

if [ "$AppRecordsAudio" = "n" -o -z "$AppRecordsAudio" ] ; then
	$SEDI "/==RECORD_AUDIO==/ d" project/AndroidManifest.xml
fi

case "$MinimumScreenSize" in
	n|m)
		$SEDI "/==SCREEN-SIZE-SMALL==/ d" project/AndroidManifest.xml
		$SEDI "/==SCREEN-SIZE-LARGE==/ d" project/AndroidManifest.xml
		;;
	l)
		$SEDI "/==SCREEN-SIZE-SMALL==/ d" project/AndroidManifest.xml
		$SEDI "/==SCREEN-SIZE-NORMAL==/ d" project/AndroidManifest.xml
		;;
	*)
		$SEDI "/==SCREEN-SIZE-NORMAL==/ d" project/AndroidManifest.xml
		$SEDI "/==SCREEN-SIZE-LARGE==/ d" project/AndroidManifest.xml
		;;
esac

if [ "$AccessSdCard" = "true" ]; then
	$SEDI "/==NOT_EXTERNAL_STORAGE==/ d" project/AndroidManifest.xml
	$SEDI "/==READ_OBB==/ d" project/AndroidManifest.xml
else
	if [ "$ReadObbFile" = "y" ]; then
		$SEDI "/==EXTERNAL_STORAGE==/ d" project/AndroidManifest.xml # Disabled by default
		$SEDI "/==NOT_EXTERNAL_STORAGE==/ d" project/AndroidManifest.xml
	else
		$SEDI "/==EXTERNAL_STORAGE==/ d" project/AndroidManifest.xml # Disabled by default
		$SEDI "/==READ_OBB==/ d" project/AndroidManifest.xml
	fi
fi

if [ "$AccessInternet" = "n" ]; then
	$SEDI "/==INTERNET==/ d" project/AndroidManifest.xml
fi

if [ -z "$AppOpenFileExtension" ]; then
	$SEDI "/==OPENFILE==/ d" project/AndroidManifest.xml
else
	EXTS="`for EXT in $AppOpenFileExtension; do echo -n '\\1'$EXT'\\2' ; done`"
	$SEDI "s/\(.*\)==OPENFILE-EXT==\(.*\)/$EXTS/g" project/AndroidManifest.xml
fi

if [ "$ImmersiveMode" = "n" ]; then
	ImmersiveMode=false
else
	ImmersiveMode=true
fi

if [ "$DrawInDisplayCutout" = "y" ]; then
	DrawInDisplayCutout=true
else
	DrawInDisplayCutout=false
fi

if [ "$HideSystemMousePointer" = "n" ]; then
	HideSystemMousePointer=false
else
	HideSystemMousePointer=true
fi

if [ "$CreateService" = "y" ] ; then
	CreateService=true
else
	CreateService=false
	$SEDI "/==FOREGROUND_SERVICE==/ d" project/AndroidManifest.xml
fi

GLESLib=-lGLESv1_CM
GLESVersion=-DSDL_VIDEO_OPENGL_ES_VERSION=1

if [ "$NeedGles2" = "y" ] ; then
	NeedGles2=true
	GLESLib=-lGLESv2
	GLESVersion=-DSDL_VIDEO_OPENGL_ES_VERSION=2
else
	NeedGles2=false
	$SEDI "/==GLES2==/ d" project/AndroidManifest.xml
fi

if [ "$NeedGles3" = "y" ] ; then
	NeedGles3=true
	GLESLib=-lGLESv3
	GLESVersion=-DSDL_VIDEO_OPENGL_ES_VERSION=3
else
	NeedGles3=false
	$SEDI "/==GLES3==/ d" project/AndroidManifest.xml
fi


echo Patching project/src/Globals.java
$SEDI "s/public static String ApplicationName = .*;/public static String ApplicationName = \"$AppShortName\";/" project/src/Globals.java

$SEDI "s/public static final boolean UsingSDL2 = .*;/public static final boolean UsingSDL2 = $UsingSdl2;/" project/src/Globals.java

# Work around "Argument list too long" problem when compiling VICE
#$SEDI "s@public static String DataDownloadUrl = .*@public static String DataDownloadUrl = \"$AppDataDownloadUrl1\";@" project/src/Globals.java
$SEDI "s@public static String\[\] DataDownloadUrl = .*@public static String[] DataDownloadUrl = { ### };@" project/src/Globals.java
echo "$AppDataDownloadUrl1" | tr '^' '\n' | while read URL; do $SEDI "s@###@\"$URL\", ###@" project/src/Globals.java ; done
$SEDI "s@###@@" project/src/Globals.java

$SEDI "s/public static boolean SwVideoMode = .*;/public static boolean SwVideoMode = $SwVideoMode;/" project/src/Globals.java
$SEDI "s/public static int VideoDepthBpp = .*;/public static int VideoDepthBpp = $VideoDepthBpp;/" project/src/Globals.java
$SEDI "s/public static boolean NeedDepthBuffer = .*;/public static boolean NeedDepthBuffer = $NeedDepthBuffer;/" project/src/Globals.java
$SEDI "s/public static boolean NeedStencilBuffer = .*;/public static boolean NeedStencilBuffer = $NeedStencilBuffer;/" project/src/Globals.java
$SEDI "s/public static boolean NeedGles2 = .*;/public static boolean NeedGles2 = $NeedGles2;/" project/src/Globals.java
$SEDI "s/public static boolean NeedGles3 = .*;/public static boolean NeedGles3 = $NeedGles3;/" project/src/Globals.java
$SEDI "s/public static boolean CompatibilityHacksVideo = .*;/public static boolean CompatibilityHacksVideo = $CompatibilityHacksForceScreenUpdate;/" project/src/Globals.java
$SEDI "s/public static boolean CompatibilityHacksStaticInit = .*;/public static boolean CompatibilityHacksStaticInit = $CompatibilityHacksStaticInit;/" project/src/Globals.java
$SEDI "s/public static boolean CompatibilityHacksTextInputEmulatesHwKeyboard = .*;/public static boolean CompatibilityHacksTextInputEmulatesHwKeyboard = $CompatibilityHacksTextInputEmulatesHwKeyboard;/" project/src/Globals.java
$SEDI "s/public static int TextInputKeyboard = .*;/public static int TextInputKeyboard = $TextInputKeyboard;/" project/src/Globals.java
$SEDI "s/public static boolean CompatibilityHacksForceScreenUpdateMouseClick = .*;/public static boolean CompatibilityHacksForceScreenUpdateMouseClick = $CompatibilityHacksForceScreenUpdateMouseClick;/" project/src/Globals.java
$SEDI "s/public static boolean HorizontalOrientation = .*;/public static boolean HorizontalOrientation = $HorizontalOrientation;/" project/src/Globals.java
$SEDI "s^public static boolean KeepAspectRatioDefaultSetting = .*^public static boolean KeepAspectRatioDefaultSetting = $SdlVideoResizeKeepAspect;^" project/src/Globals.java
$SEDI "s/public static boolean InhibitSuspend = .*;/public static boolean InhibitSuspend = $InhibitSuspend;/" project/src/Globals.java
$SEDI "s/public static boolean CreateService = .*;/public static boolean CreateService = $CreateService;/" project/src/Globals.java
$SEDI "s/public static boolean AppUsesMouse = .*;/public static boolean AppUsesMouse = $AppUsesMouse;/" project/src/Globals.java
$SEDI "s/public static boolean AppNeedsTwoButtonMouse = .*;/public static boolean AppNeedsTwoButtonMouse = $AppNeedsTwoButtonMouse;/" project/src/Globals.java
$SEDI "s/public static boolean RightMouseButtonLongPress = .*;/public static boolean RightMouseButtonLongPress = $RightMouseButtonLongPress;/" project/src/Globals.java
$SEDI "s/public static boolean ForceRelativeMouseMode = .*;/public static boolean ForceRelativeMouseMode = $ForceRelativeMouseMode;/" project/src/Globals.java
$SEDI "s/public static boolean ShowMouseCursor = .*;/public static boolean ShowMouseCursor = $ShowMouseCursor;/" project/src/Globals.java
$SEDI "s/public static boolean ScreenFollowsMouse = .*;/public static boolean ScreenFollowsMouse = $ScreenFollowsMouse;/" project/src/Globals.java
$SEDI "s/public static boolean GenerateSubframeTouchEvents = .*;/public static boolean GenerateSubframeTouchEvents = $GenerateSubframeTouchEvents;/" project/src/Globals.java
$SEDI "s/public static boolean AppNeedsArrowKeys = .*;/public static boolean AppNeedsArrowKeys = $AppNeedsArrowKeys;/" project/src/Globals.java
$SEDI "s/public static boolean FloatingScreenJoystick = .*;/public static boolean FloatingScreenJoystick = $FloatingScreenJoystick;/" project/src/Globals.java
$SEDI "s/public static boolean AppNeedsTextInput = .*;/public static boolean AppNeedsTextInput = $AppNeedsTextInput;/" project/src/Globals.java
$SEDI "s/public static boolean AppUsesJoystick = .*;/public static boolean AppUsesJoystick = $AppUsesJoystick;/" project/src/Globals.java
$SEDI "s/public static boolean AppUsesSecondJoystick = .*;/public static boolean AppUsesSecondJoystick = $AppUsesSecondJoystick;/" project/src/Globals.java
$SEDI "s/public static boolean AppUsesThirdJoystick = .*;/public static boolean AppUsesThirdJoystick = $AppUsesThirdJoystick;/" project/src/Globals.java
$SEDI "s/public static boolean AppUsesAccelerometer = .*;/public static boolean AppUsesAccelerometer = $AppUsesAccelerometer;/" project/src/Globals.java
$SEDI "s/public static boolean AppUsesGyroscope = .*;/public static boolean AppUsesGyroscope = $AppUsesGyroscope;/" project/src/Globals.java
$SEDI "s/public static boolean AppUsesOrientationSensor = .*;/public static boolean AppUsesOrientationSensor = $AppUsesOrientationSensor;/" project/src/Globals.java
$SEDI "s/public static boolean MoveMouseWithGyroscope = .*;/public static boolean MoveMouseWithGyroscope = $MoveMouseWithGyroscope;/" project/src/Globals.java
$SEDI "s/public static boolean AppUsesMultitouch = .*;/public static boolean AppUsesMultitouch = $AppUsesMultitouch;/" project/src/Globals.java
$SEDI "s/public static boolean NonBlockingSwapBuffers = .*;/public static boolean NonBlockingSwapBuffers = $NonBlockingSwapBuffers;/" project/src/Globals.java
$SEDI "s/public static boolean ResetSdlConfigForThisVersion = .*;/public static boolean ResetSdlConfigForThisVersion = $ResetSdlConfigForThisVersion;/" project/src/Globals.java
$SEDI "s/public static boolean ImmersiveMode = .*;/public static boolean ImmersiveMode = $ImmersiveMode;/" project/src/Globals.java
$SEDI "s/public static boolean DrawInDisplayCutout = .*;/public static boolean DrawInDisplayCutout = $DrawInDisplayCutout;/" project/src/Globals.java
$SEDI "s/public static boolean HideSystemMousePointer = .*;/public static boolean HideSystemMousePointer = $HideSystemMousePointer;/" project/src/Globals.java
$SEDI "s|public static String DeleteFilesOnUpgrade = .*;|public static String DeleteFilesOnUpgrade = \"$DeleteFilesOnUpgrade\";|" project/src/Globals.java
$SEDI "s/public static int AppTouchscreenKeyboardKeysAmount = .*;/public static int AppTouchscreenKeyboardKeysAmount = $AppTouchscreenKeyboardKeysAmount;/" project/src/Globals.java
$SEDI "s@public static String\\[\\] AppTouchscreenKeyboardKeysNames = .*;@public static String[] AppTouchscreenKeyboardKeysNames = \"$RedefinedKeysScreenKbNames\".split(\" \");@" project/src/Globals.java
$SEDI "s/public static int TouchscreenKeyboardTheme = .*;/public static int TouchscreenKeyboardTheme = $TouchscreenKeysTheme;/" project/src/Globals.java
$SEDI "s/public static int StartupMenuButtonTimeout = .*;/public static int StartupMenuButtonTimeout = $StartupMenuButtonTimeout;/" project/src/Globals.java
$SEDI "s/public static int AppMinimumRAM = .*;/public static int AppMinimumRAM = $AppMinimumRAM;/" project/src/Globals.java
$SEDI "s/public static SettingsMenu.Menu HiddenMenuOptions .*;/public static SettingsMenu.Menu HiddenMenuOptions [] = { $HiddenMenuOptions1 };/" project/src/Globals.java
[ -n "$FirstStartMenuOptions1" ] && $SEDI "s@public static SettingsMenu.Menu FirstStartMenuOptions .*;@public static SettingsMenu.Menu FirstStartMenuOptions [] = { $FirstStartMenuOptions1 };@" project/src/Globals.java
$SEDI "s%public static String ReadmeText = .*%public static String ReadmeText = \"$ReadmeText\";%" project/src/Globals.java
$SEDI "s%public static String CommandLine = .*%public static String CommandLine = \"$AppCmdline\";%" project/src/Globals.java
$SEDI "s%public static String AdmobPublisherId = .*%public static String AdmobPublisherId = \"$AdmobPublisherId\";%" project/src/Globals.java
$SEDI "s/public static String AdmobTestDeviceId = .*/public static String AdmobTestDeviceId = \"$AdmobTestDeviceId\";/" project/src/Globals.java
$SEDI "s/public static String AdmobBannerSize = .*/public static String AdmobBannerSize = \"$AdmobBannerSize\";/" project/src/Globals.java
$SEDI "s%public static String GooglePlayGameServicesId = .*%public static String GooglePlayGameServicesId = \"$GooglePlayGameServicesId\";%" project/src/Globals.java
$SEDI "s/public static boolean AccessSdCard.*/public static boolean AccessSdCard = $AccessSdCard;/" project/src/Globals.java
$SEDI "s/public static String AppLibraries.*/public static String AppLibraries[] = { $LibrariesToLoad };/" project/src/Globals.java
$SEDI "s/public static String AppMainLibraries.*/public static String AppMainLibraries[] = { $MainLibrariesToLoad };/" project/src/Globals.java


if $UsingSdl2; then
	# Delete options that reference classes from SDL 1.2
	$SEDI "s/public static SettingsMenu.Menu HiddenMenuOptions .*;//" project/src/Globals.java
	$SEDI "s/public static SettingsMenu.Menu FirstStartMenuOptions .*;//" project/src/Globals.java
fi

echo Patching project/jni/Settings.mk
echo '# DO NOT EDIT THIS FILE - it is automatically generated, edit file SettingsTemplate.mk' > project/jni/Settings.mk
cat project/jni/SettingsTemplate.mk | \
	sed "s/APP_MODULES := .*/APP_MODULES := `$UsingSdl2 && echo SDL2 || echo sdl-1.2` sdl_native_helpers jpeg png ogg flac vorbis freetype $CompiledLibraries/" | \
	sed "s/APP_ABI := .*/APP_ABI := $MultiABI/" | \
	sed "s/SDL_JAVA_PACKAGE_PATH := .*/SDL_JAVA_PACKAGE_PATH := $AppFullNameUnderscored/" | \
	sed "s^SDL_CURDIR_PATH := .*^SDL_CURDIR_PATH := $DataPath^" | \
	sed "s^SDL_VIDEO_RENDER_RESIZE := .*^SDL_VIDEO_RENDER_RESIZE := $SdlVideoResize^" | \
	sed "s^COMPILED_LIBRARIES := .*^COMPILED_LIBRARIES := $CompiledLibraries^" | \
	sed "s^APPLICATION_ADDITIONAL_CFLAGS :=.*^APPLICATION_ADDITIONAL_CFLAGS := $AppCflags^" | \
	sed "s^APPLICATION_ADDITIONAL_CPPFLAGS :=.*^APPLICATION_ADDITIONAL_CPPFLAGS := $AppCppflags^" | \
	sed "s^APPLICATION_ADDITIONAL_LDFLAGS :=.*^APPLICATION_ADDITIONAL_LDFLAGS := -lc++_shared $AppLdflags^" | \
	sed "s^APPLICATION_GLES_LIBRARY :=.*^APPLICATION_GLES_LIBRARY := $GLESLib^" | \
	sed "s^APPLICATION_OVERLAPS_SYSTEM_HEADERS :=.*^APPLICATION_OVERLAPS_SYSTEM_HEADERS := $AppOverlapsSystemHeaders^" | \
	sed "s^USE_GL4ES :=.*^USE_GL4ES := $UseGl4es^" | \
	sed "s^SDL_ADDITIONAL_CFLAGS :=.*^SDL_ADDITIONAL_CFLAGS := \
		$RedefinedKeycodes \
		$RedefinedSDLScreenGestures \
		$RedefinedKeycodesScreenKb \
		$RedefinedKeycodesGamepad \
		$CompatibilityHacksPreventAudioChopping \
		$CompatibilityHacksAppIgnoresAudioBufferSize \
		$CompatibilityHacksSlowCompatibleEventQueue \
		$CompatibilityHacksTouchscreenKeyboardSaveRestoreOpenGLState \
		$CompatibilityHacksProperUsageOfSDL_UpdateRects \
		$UseGl4esCFlags \
		$GLESVersion^" | \
	sed "s^APPLICATION_SUBDIRS_BUILD :=.*^APPLICATION_SUBDIRS_BUILD := $AppSubdirsBuild^" | \
	sed "s^APPLICATION_BUILD_EXCLUDE :=.*^APPLICATION_BUILD_EXCLUDE := $AppBuildExclude^" | \
	sed "s^APPLICATION_CUSTOM_BUILD_SCRIPT :=.*^APPLICATION_CUSTOM_BUILD_SCRIPT := $CustomBuildScript^" | \
	sed "s^SDL_VERSION :=.*^SDL_VERSION := $LibSdlVersion^" | \
	sed "s^NDK_TOOLCHAIN_VERSION :=.*^NDK_TOOLCHAIN_VERSION := $NDK_TOOLCHAIN_VERSION^" | \
	sed "s^APP_PLATFORM :=.*^APP_PLATFORM := $APP_PLATFORM^" >> \
	project/jni/Settings.mk

echo Patching strings.xml
rm -rf project/res/values*/strings.xml
cd $JAVA_SRC_PATH/translations
for F in */strings.xml; do
	mkdir -p ../../res/`dirname $F`
	cat $F | \
	sed "s^[<]string name=\"app_name\"[>].*^<string name=\"app_name\">$AppName</string>^" > \
	../../res/$F
done
cd ../../..

SDK_DIR=`grep '^sdk.dir' project/local.properties | sed 's/.*=//'`
[ -z "$SDK_DIR" ] && SDK_DIR="$ANDROID_HOME"
[ -z "$SDK_DIR" ] && SDK_DIR=`which android | sed 's@/tools/android$@@'`
mkdir -p project/libs
echo "sdk.dir=$SDK_DIR" > project/local.properties
echo 'proguard.config=proguard.cfg;proguard-local.cfg' >> project/local.properties

if [ "$GooglePlayGameServicesId" = "n" -o -z "$GooglePlayGameServicesId" ] ; then
	$SEDI "/==GOOGLEPLAYGAMESERVICES==/ d" project/AndroidManifest.xml
	$SEDI "/==GOOGLEPLAYGAMESERVICES==/ d" project/app/build.gradle
	GooglePlayGameServicesId=""
else
	for F in $JAVA_SRC_PATH/googleplaygameservices/*.java; do
		OUT=`echo $F | sed 's@.*/@@'` # basename tool is not available everywhere
		echo Patching $F
		echo '// DO NOT EDIT THIS FILE - it is automatically generated, edit file under $JAVA_SRC_PATH dir' > project/src/$OUT
		cat $F | sed "s/^package .*;/package $AppFullName;/" >> project/src/$OUT
	done

	$SEDI "s/==GOOGLEPLAYGAMESERVICES_APP_ID==/$GooglePlayGameServicesId/g" project/res/values/strings.xml
fi

if [ -e "project/jni/application/src/AndroidData/assetpack" ] ; then
	true # Do nothing...
else
	$SEDI "/==ASSETPACK==/ d" project/app/build.gradle
fi

if [ -e project/jni/application/src/project.diff ]; then patch -p1 --dry-run -f -R < project/jni/application/src/project.diff > /dev/null 2>&1 || patch -p1 --no-backup-if-mismatch < project/jni/application/src/project.diff || exit 1 ; fi
if [ -e project/jni/application/src/project.patch ]; then patch -p1 --dry-run -f -R < project/jni/application/src/project.patch > /dev/null 2>&1 || patch -p1 --no-backup-if-mismatch < project/jni/application/src/project.patch || exit 1 ; fi

rm -f project/lib
ln -s -f libs project/lib

echo Cleaning up dependencies

rm -rf project/libs/*/* project/gen
rm -rf project/obj/local/*/objs*/sdl_main/* project/$OUT/local/*/libsdl_main.so
rm -rf project/obj/local/*/libsdl-*.so
rm -rf project/obj/local/*/libsdl_*.so
rm -rf project/obj/local/*/objs*/sdl-*/src/*/android
rm -rf project/obj/local/*/objs*/sdl-*/src/video/SDL_video.o
rm -rf project/obj/local/*/objs*/sdl-*/SDL_renderer_gles.o
rm -rf project/obj/local/*/objs*/sdl_*
rm -rf project/obj/local/*/objs*/lzma/src/XZInputStream.o
rm -rf project/obj/local/*/objs*/liblzma.so
rm -rf project/obj/local/*/objs*/openal/src/Alc/android.o
rm -rf project/obj/local/*/objs*/libopenal.so
# No need to recompile SDL2 libraries, it does not contain package name

rm -rf project/jni/application/src/AndroidData/lib

rm -rf project/bin/classes
rm -rf project/bin/res
rm -rf project/app/build

# Generate OUYA icon, for that one user who still got an OUYA in his living room and won't throw it away just because someone else decides that it's dead
rm -rf project/res/drawable-xhdpi/ouya_icon.png
if which convert > /dev/null; then
	mkdir -p project/res/drawable-xhdpi
	convert project/res/drawable/icon.png -resize '732x412' -background none -gravity center -extent '732x412' project/res/drawable-xhdpi/ouya_icon.png
else
	echo "Install ImageMagick to auto-resize Ouya icon from icon.png"
fi

./copyAssets.sh || exit 1

rm -rf project/jni/android-support

rm -rf project/res/drawable/banner.png
if [ -e project/jni/application/src/banner.png ]; then
	ln -s ../../jni/application/src/banner.png project/res/drawable/banner.png
else
	ln -s ../../themes/tv-banner-placeholder.png project/res/drawable/banner.png
fi

if uname -s | grep -i "darwin" > /dev/null ; then
	find project/src -name "*.killme.tmp" -delete
fi

echo Compiling prebuilt libraries

if echo "$CompiledLibraries" | grep -E 'crypto|ssl' > /dev/null; then
	make -C project/jni -f Makefile.prebuilt openssl ARCH_LIST="$MultiABI"
fi

if echo "$CompiledLibraries" | grep -E 'iconv|charset|icu' > /dev/null; then
	echo "#=Compiling prebuilt icu"
	make -C project/jni -f Makefile.prebuilt icu ARCH_LIST="$MultiABI"
fi

if echo "$CompiledLibraries" | grep 'boost_' > /dev/null; then
	make -C project/jni -f Makefile.prebuilt boost ARCH_LIST="$MultiABI"
fi

echo Done
