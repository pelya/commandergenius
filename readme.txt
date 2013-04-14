This is SDL 1.2 and 1.3 ported to Google Android (also bunch of other libs included).
Sources or patches of the individual games are in the directory project/jni/application.

The libsdl.org now has an official SDL 1.3 Android port, which is more recent and
better suited for creating new applications from scratch, this port is focused mainly
on SDL 1.2 and compiling existing applications, it's up to you to decide which port is better.
Also this port is developed very slowly, although the same is true for an official port.


Installation
============

This project should be compiled with Android 4.2 SDK (API level 17) and NDK r8, r7c, r6 or r5c,
google for them and install them as described in their docs.
You'll need to install Java Ant too.
The application will run on Android OS 1.6 and above, but will use features from Android 4.2 if available.
Also it's compatible with NDK r4b and all versions of CrystaX NDK starting from r4b.
CrystaX NDK adds support for wide chars, and required if you want to use Boost libraries.
http://www.crystax.net/android/ndk.php
The most supported environment for this port is Linux, MacOs should be okay too.
If you're developing under Windows you'd better install Portable Ubuntu, to get proper Linux environment
running inside Windows, then install Linux toolchain on it.
https://sourceforge.net/projects/portableubuntu/
Cygwin is not supported by the NDK, starting from the NDK r6.


How to compile demo application
===============================

Launch commands
	rm project/jni/application/src
	ln -s ballfield project/jni/application/src
	./changeAppSettings.sh -a
	android update project -p project -t android-15
Then edit file build.sh if needed to add NDK dir to your PATH, then launch it.
It will compile a bunch of libs under project/libs/armeabi,
create Android package file project/bin/MainActivity-debug.apk,
and install it to your device or emulator, if you specify option -i or -r to build.sh.
Then you can test it by launching Ballfield icon from Android applications menu.

There are other applications inside project/jni/application directory,
some of them are referenced using Git submodule mechanism, you may download them using command
git submodule update --init
Some of them may be outdated and won't compile, some contain only patch file and no sources,
so you should check out Git logs before compiling a particular app, and checkout whole repo at that date:
gitk project/jni/application/<directory>

The game enforces horizontal screen orientation, you may slide-open your keyboard if you have it
and use it for additional keys - the device will just keep current screen orientation.
Recent Android phone models like HTC Evo have no keyboard at all, on-screen keyboard built into SDL
is available for such devices - it has joystick (which can be configured as arrow buttons or analog joystick),
and 6 configurable keys, full text input is toggled with 7-th key. Both user and application may redefine
button layout and returned keycodes, and also toggle full text input - see SDL_screenkeyboard.h.
Also you can read multitouch events and accelerometer events - they are passed as joystick events,
see Ballfield sample app for the input event handling code.

This port also supports GL ES + SDL combo - there is GLXGears demo app in project/jni/application/glxgears,
to compile it remove project/jni/application/src symlink and make new one pointing to glxgears, and run build.sh
Note that GL ES is NOT pure OpenGL - there are no glBegin() and glEnd() call and other widely used functions,
and generally it will take a lot of effort to port OpenGL application to GL ES.

How to compile a specific SDL based application
===============================================
. build/envsetup.sh
build

How to compile your own application
===================================

You may find quick Android game porting manual at http://anddev.at.ua/src/porting_manual.txt

If you're porting existing app which uses SDL 1.2 please always use SW mode:
neither SDL_SetVideoMode() call nor SDL_CreateRGBSurface() etc functions shall contain SDL_HWSURFACE flags.
The BPP in SDL_SetVideoMode() shall be set to the same value you've specified in ChangeAppSettings.sh,
and audio format - to AUDIO_S8 or AUDIO_S16. Also bear in mind that 16-bit BPP is always faster than 24 or 32-bit,
even on good devices, because most GFX chips on Android do not have separate RAM, and use system RAM instead,
so with 16 bit color mode you'll get lesser memory copying operations.

The native Android 16-bit pixel format is RGB_565, even for OpenGL, not BGR_565 as all other OpenGL implementations have.

Colorkey surfaces and alpha surfaces are supported, SDL_RLEACCEL is not supported.

To compile your own app, put your app sources into project/jni/application dir (or create symlink to them),
and change symlink "src" to point to your app:

	cp -r /path/to/my/app project/jni/application/myapp
or
	ln -s /path/to/my/app project/jni/application/myapp
then
	rm project/jni/application/src
	ln -s myapp project/jni/application/src
(the second one should be relative link without slashes)

Also your main() function name should be redefined to SDL_main(), include SDL.h so it will be done automatically.

Then launch script ChangeAppSettings.sh - it will ask few questions and modify several file in the project -
there's no way around such external configure script, because Java does not support preprocessor,
and the Java code is a part of SDL lib, the application generally should not care about it.
You may take AndroidAppSettings.cfg file from some other application to get sane defaults,
you may launch ChangeAppSettings.sh with -a or -v parameter to skip questions altogether or to ask only version code.
The C++ files shall have .cpp extension to be compiled, rename them if necessary.
Also you have to create an icon image file at project/jni/application/src/icon.png, and you may create a file
project/jni/application/src/AndroidData/logo.png to be used as a splash screen image.
Then you may launch build.sh.

C++ RTTI and exceptions give very slight memory overhead, if you need them -
add "-frtti -fexceptions" to the AppCflags inside AndroidAppSettings.cfg
If you use autoconf/automake/configure scripts with setEnvironment.sh, you may write
env CXXFLAGS='-frtti -fexceptions' ../setEnvironment.sh ./configure

Application data may be bundled with app itself, or downloaded from the internet on the first run -
if you want to put app data inside .apk file - create a .zip archive and put it into the directory
project/jni/application/src/AndroidData (create it if it doesn't exist), then run ChangeAppSettings.sh
and specify the file name there. If the data files are more than 10 Mb it's a good idea to put them
on public HTTP server - you may specify URL in ChangeAppSettings.sh, also you may specify several files.
If you'll release new version of data files you should change download URL or data file name and update your app as well -
the app will re-download the data if URL does not match the saved URL from previous download.

All devices have different screen resolutions, you may toggle automatic screen resizing
in ChangeAppSettings.sh and draw to virtual 640x480 screen - it will be HW accelerated
and will not impact performance. Automatic screen resizing does not work in SDL 1.3/2.0.
SDL_GetVideoInfo() or SDL_ListModes(NULL, 0)[0] will always return native screen resolution.
Also make sure that your HW textures are not wider than 1024 pixels, or it will fail to allocate such
texture on HTC G1, and other low-end devices. Software surfaces may be of any size of course.

If you want HW acceleration - just use OpenGL, that's the easiest and most cross-platform way,
however if you'll use on-screen keyboard (even the text input button) the OpenGL state will get
messed up after each frame - after each SDL_GL_SwapBuffers() you'll need to restore following GL state:

glEnable(GL_TEXTURE_2D);
glActiveTexture(GL_TEXTURE0);
glClientActiveTexture(GL_TEXTURE0);
glBindTexture(GL_TEXTURE_2D, your_texture_id);
glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
glEnable(GL_BLEND);
glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
glEnableClientState(GL_COLOR_ARRAY);

Previously I've got the code to save/restore OpenGL state, but it doens't work on every device -
you may wish to uncomment it inside file SDL_touchscreenkeyboard.c in functions beginDrawingTex() and endDrawingTex().

If you don't use on-screen keyboard you don't need to reinit OpenGL state - set following in AndroidAppSettings.cfg:
AppNeedsArrowKeys=n
AppNeedsTextInput=n
AppTouchscreenKeyboardKeysAmount=0

SDL 1.2 supports HW acceleration, however it has many limitations:
You should use 16-bit color depth.
You cannot blit SW surface to screen, it should be only HW surface.
You can use colorkey, per-surface alpha and per-pixel alpha with HW surfaces.
If you're using SDL 1.3 always use SDL_Texture, if you'll be using SDL_Surface or call SDL_SetVideoMode()
with SDL 1.3 it will automatically switch to SW mode.
Also the screen is always double-buffered, and after each SDL_Flip() there is garbage in pixel buffer,
so forget about dirty rects and partial screen updates - you have to re-render whole picture each frame.
Calling SDL_UpdateRects() just calls SDL_Flip() internally, updating the whole screen at once.
Single-buffer rendering might be possible with techniques like glFramebufferTexture2D(),
however it is not present on all devices, so I won't do that.
Basically your code should be like this for SDL 1.2 (also set SwVideoMode=n in AndroidAppSetings.cfg):

// ----- HW-accelerated video output for Android example
// Init HW-accelerated video
SDL_SetVideoMode( 640, 480, 16, SDL_DOUBLEBUF | SDL_HWSURFACE );

// Load graphics
SDL_Surface *sprite = IMG_Load( "sprite.png" );
SDL_Surface * hwSprite;

if( sprite->format->Amask )
{
	// Surface contains per-pixel alpha, convert it to HW-accelerated format
	hwSprite = SDL_DisplayFormatAlpha(sprite);
}
else
{
	// Set pink color as transparent
	SDL_SetColorKey( sprite, SDL_SRCCOLORKEY, SDL_MapRGB(sprite->format, 255, 0, 255) );
	// Create HW-accelerated surface
	hwSprite = SDL_DisplayFormat(sprite);
	// Set per-surface alpha, if necessary
	SDL_SetAlpha( hwSprite, SDL_SRCALPHA, 128 );
}

// Blit it in HW-accelerated way
SDL_BlitSurface(hwSprite, sourceRect, SDL_GetVideoSurface(), &targetRect);

// Render the resulting video frame to device screen
SDL_Flip(SDL_GetVideoSurface());

// Supported, but VERY slow (slower than blitting in SW mode)
SDL_BlitSurface(sprite, sourceRect, SDL_GetVideoSurface(), &targetRect);

// Supported, but VERY slow (use in cases where you need to take a screenshot)
SDL_BlitSurface(SDL_GetVideoSurface(), sourceRect, sprite, &targetRect);

// ----- End of example

To get HW acceleration in SDL 1.3/2.0 just follow the instructions on libsdl.org

If you'll add new libs - add them to project/jni/, copy Android.mk from existing lib, and
add libname to project/jni/<yourapp>/Android.mk
Also you'll need to move all include files to <libname>/include dir.
If lib contains "configure" script - go to lib dir and execute command "../launchConfigureLib.sh" - it will
launch "configure" with appropriate environment and will create the "config.h" file at least, though linking
will most probably fail because of ranlib - just edit Android.mk to compile lib sources and remove all tools and tests.

MIDI support can be emulated via SDL_mixer lib (it uses Timidity internally)- download file
http://www.libsdl.org/projects/mixer/timidity/timidity.tar.gz
unpack it and put "timidity" dir into your game data zipfile.
Or you may paste this URL directly as an optional download in ChangeAppSettings.sh:
MIDI music support (18 Mb)|http://sourceforge.net/projects/libsdl-android/files/timidity.zip/download

SDL by default listens to the Volume Up and Volume Down hardware keys, and sends them to the application,
instead of changing volume. Most users expect those keys to actually change volume, instead of performing some in-game action.
To make SDL ignore those keys, and let the Android framework handle them instead, set
RedefinedKeys="XXX YYY NO_REMAP NO_REMAP ZZZ BBB CCC" inside AndroidAppSettings.cfg, that is,
the third and fourth keycode should be a special value "NO_REMAP" instead of SDL keycode.
XXX, YYY and ZZZ are placeholders for SDL keycodes of other hardware keys -
XXX is sent when user touches the screen and app is not using mouse or multitouch,
YYY is for DPAD_CENTER/SEARCH keys, ZZZ is for MENU key, BBB is for BACK key, CCC is for CAMERA key.

The ARM architecture has some limitations which you have to be aware about -
if you'll access integer that's not 4-byte aligned you'll get garbage instead of correct value,
and it's processor-model specific - it may work on some devices and do not work on another ones -
you may wish to check your code in Android 1.6 emulator from time to time to catch such bugs.

char * p = 0x13; // Non-4 byte aligned pointer
int i = (int *) p; // We have garbage inside i now
memcpy( &i, p, sizeof(int) ); // The correct way to dereference a non-aligned pointer

This compiler flags will catch most obvious errors, you may add them to AppCflags var in settings:
-Wstrict-aliasing -Wcast-align -Wpointer-arith -Waddress
Also beware of the NDK - some system headers contain the code that triggers that warnings.

The application will automatically get moved to SD-card on Android 2.2 or newer,
(or you can install app2sd for older, but rooted phones),
however the shared libraries have to be stored on the device internal storage,
and that may be not desired for older phones with very little storage.
The script app2sd.sh will re-package your .apk file in such a way that
the shared libraries will not be extracted by Android OS but by application itself,
and it will remove them from internal storage right after starting up,
so you still need that space free, but only temporarily.
However your application will start up slower.

SDL supports AdMob advertisements, you need to set your publisher ID inside AndroidAppSettings.cfg,
see project test-advertisements for details.
Also you can hide or reposition your ad from C code, check out file SDL_android.h for details.


How to compile your own application using automake/configure scripts
====================================================================

There is limited support for "configure" scripts, I'm compiling scummvm and openttd this way,
though ./configure scripts tend to have stupid bugs in various places, and ranlib command never works.
You should enable custom build script in ChangeAppSettings.sh, and you should create script
AndroidBuild.sh and put it under project/jni/application/src dir. The AndroidBuild.sh script should
generate file project/jni/application/src/libapplication.so, which will be copied into .apk file by build system.
There is helper script project/jni/application/setEnvironment.sh which will set CFLAGS and LDFLAGS
for configure script and makefile, see AndroidBuild.sh in project/jni/application/scummvm dir for reference.


Android application sleep/resume support
========================================

Application may be put to background at any time, for example if user gets phone call onto the device.
The application will lose OpenGL context then, and has to re-create it when put to foreground.

The SDL provides function
SDL_ANDROID_SetApplicationPutToBackgroundCallback( callback_t appPutToBackground, callback_t appRestored );
where callback_t is function pointer of type "void (*) void".
The default callbacks will call another Android-specific functions:
SDL_ANDROID_PauseAudioPlayback() and SDL_ANDROID_ResumeAudioPlayback()
which will pause and resume audio from HW layer, so appplication does not need to destroy and re-init audio,
and in general you don't need to redefine those functions, unless you want to play audio in background.
Also, the usual event SDL_ACTIVEEVENT with flag SDL_APPACTIVE will be sent when that happens,
and also SDL_VIDEORESIZE event will be sent (the same behavior as in MacOsX SDL implementation).
If you're using OpenAL it will be paused automatically when your app goes to background.

If you're using pure SDL 1.2 API (with or without HW acceleration) you don't need to worry about anything -
the SDL itself will re-create GL textures and fill them with pixel data from existing SDL HW surfaces,
so you may leave the callbacks to defaults.

If you're using SDL 1.3 API and using SDL_Texture, then the textures pixeldata is lost - you will need
to call SDL_UpdateTexture() to refill texture pixeldata from appRestored() callback for all your textures.
If you're using compatibility API with SDL_Surfaces you don't have to worry about that.

If you're using SDL with OpenGL with either SDL 1.2 or SDL 1.3, the situation is even more grim -
not only all your GL textures are lost, but all GL matrices, blend modes, etc. has to be re-created.

OS may decide there's too little free RAM left on device, and kill background applications
without notice, so it vill be good to create temporary savegame etc. from appPutToBackground() callback.

Also it's a good practice to pause any application audio, especially if the user gets phone call,
and if you won't set your own callbacks the default callbacks will do exactly that.
There are circumstances when you want to avoid that, for example if the application is audio player,
or if application gets some notification over network (for example you're running a game server,
and want a beep when someone connects to you) - you may unpause audio for some short time,
that will require another thread to watch the network, because main thread will be blocked inside SDL_Flip().

The application is not allowed to do any GFX output without OpenGL context (or it will crash),
that's why SDL_Flip() call will block until we're re-acquired context, and the callbacks will be called
from inside SDL_Flip().
The whole idea behind callbacks is that the existing application should not be modified to
operate correctly - the whole time in background will just look to app as one very long SDL_Flip(),
so it's good idea to implement some maximum time cap on game frame, so it won't process
the game to the end level 'till the app is in background, or calculate the difference in time
between appPutToBackground() and appRestored() and update game time variables.

Alternatively, you may enable option for unblocked SDL_Flip() in ChangeAppSettings script,
then you'll have to implement special event loop right after each SDL_Flip() call:

SDL_Flip(SDL_GetVideoSurface());
SDL_Event evt;
while( SDL_PollEvent(&evt) )
{
	if( evt.type == SDL_ACTIVEEVENT && evt.active.gain == 0 && evt.active.state & SDL_APPACTIVE )
	{
		// We've lost GL context, we are not allowed to do any GFX output here, or app will crash!
		while( 1 )
		{
			SDL_PollEvent(&evt);
			if( evt.type == SDL_ACTIVEEVENT && evt.active.gain && evt.active.state & SDL_APPACTIVE )
			{
				SDL_Flip(SDL_GetVideoSurface()); // One SDL_Flip() call is required here to restore OpenGL context
				// Re-load all textures, matrixes and all other GL states if we're in SDL+OpenGL mode
				// Re-load all images to SDL_Texture if we're using it
				// Now we can draw
				break;
			}
			// Process network stuff, maybe play some sounds using SDL_ANDROID_PauseAudioPlayback() / SDL_ANDROID_ResumeAudioPlayback()
			SDL_Delay(300);
		}
	}
}


Quick guide to debug native code
================================

The debugging of multi-threaded apps is not supported with NDK r4 or r4b, you'll need NDK r5c
and Android 2.3 emulator or device.
To debug your application go to "project" dir and launch command
	ndk-gdb --verbose --start --force
then you can run usual GDB commands, like:
	cont - continue execution.
	info threads - list all threads, there will usually be like 11 of them with thread 10 being your main thread.
	bt - list stack trace / call hierarchy
	up / down - go up / down in the call hierarchy
	print var - print the value of variable "var"

You can also debug by adding extensive logs to your app:
	__android_log_print(ANDROID_LOG_INFO, "My App", "We somehow reached execution point #224");
and then watching "adb logcat" output.

If your application crashed, you should use following steps:

1. Gather the crash report from "adb logcat" - it should contain stack trace, if it does not then you're unlucky,

I/DEBUG   (   51): *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** ***
I/DEBUG   (   51): Build fingerprint: 'sprint/htc_supersonic/supersonic/supersonic:2.1-update1/ERE27/194487:userdebug/test-keys'
I/DEBUG   (   51): pid: 915, tid: 924  >>> de.schwardtnet.alienblaster <<<
I/DEBUG   (   51): signal 11 (SIGSEGV), fault addr deadbaad
I/DEBUG   (   51):  r0 00000000  r1 afe133f1  r2 00000027  r3 00000058
I/DEBUG   (   51):  r4 afe3ae08  r5 00000000  r6 00000000  r7 70477020
I/DEBUG   (   51):  r8 000000b0  r9 ffffff20  10 48552868  fp 00000234
I/DEBUG   (   51):  ip 00002ee4  sp 485527f8  lr deadbaad  pc afe10aac  cpsr 60000030
I/DEBUG   (   51):          #00  pc 00010aac  /system/lib/libc.so
I/DEBUG   (   51):          #01  pc 0000c00e  /system/lib/libc.so
I/DEBUG   (   51):          #02  pc 0000c0a4  /system/lib/libc.so
I/DEBUG   (   51):          #03  pc 0002ca00  /data/data/de.schwardtnet.alienblaster/lib/libsdl.so
I/DEBUG   (   51):          #04  pc 00028b6e  /data/data/de.schwardtnet.alienblaster/lib/libsdl.so
I/DEBUG   (   51):          #05  pc 0002d080  /data/data/de.schwardtnet.alienblaster/lib/libsdl.so

2. Go to project/bin/ndk/local/armeabi dir, find there the library mentioned in stacktrace
(libsdl.so in our example), copy the address of the first line of stacktrace (0002ca00), and execute command

<your NDK path>/toolchains/arm-linux-androideabi-4.4.3/prebuilt/linux-x86/bin/arm-linux-androideabi-gdb libsdl.so -ex "list *0x0002ca00" -ex "list *0x00028b6e" -ex "list *0x0002d080"

It will output the exact line in your source where the application crashed, and some stack trace if available.

If your application does not work for unknown reasons, there may be the case when it exports some symbol
that clash with exports from system libraries - run checkExports.sh to check this.
Also there are some symbols that are present in the NDK but are not on the device - run checkMissing.sh to check.

If your application fails to load past startup SDL logo with error

I/dalvikvm( 3401): Unable to dlopen(/data/data/com.svc/lib/libapplication.so): Cannot load library: alloc_mem_region[847]: OOPS:  1268 cannot map library 'libapplication.so'. no vspace available.

that means you're allocating huge data buffer in heap (that may be C static or global buffer variable) -
run checkStaticDataSize.sh to see the size of all static symbols inside your application,
heap memory limit on most phones is 24 Mb.

If the error string is like this:

I/dalvikvm(18105): Unable to dlopen(/data/data/net.olofson.kobodl/lib/libapplication.so): Cannot load library: link_image[1995]: failed to link libapplication.so

that means your application contains undefined symbols, absent in the system libraries,
you may check for all missing symbols by running script checkMissing.sh .
That typically happens because of linking to the dynamic libstdc++ which is not included into the .apk file -
specify "-lgnustl_static" in the linker flags to fix that.


License information
===================

The SDL 1.2 port is licensed under LGPL, so you may use it for commercial purposes
without releasing source code, however to fullfill LGPL requirements you'll have to publish
the file AndroidAppSettings.cfg to allow linking other version of libsdl-1.2.so with the libraries
in the binary package you're distributing - typically libapplication.so and other
closed-source libraries in your .apk file.

The SDL 1.3 port and Java source files are licensed under zlib license, which means
you may modify them as you like without releasing source code.

The libraries under project/jni have their own license, I've tried to compile all LGPL-ed libs
as shared libs but you should anyway inspect the licenses of the libraries you're linking to.
libmad and liblzo2 are licensed under GPL, so if you're planning to make commercial app you should avoid
using them, otherwise you'll have to release your whole application sources under GPL too.

The "Ultimate Droid" on-screen keyboard theme by Sean Stieber is licensed under Creative Commons - Attribution license.
The "Simple Theme" on-screen keyboard theme by Dmitry Matveev is licensed under zlib license.
The "Sun" on-screen keyboard theme by Sirea (Martina Smejkalova) is licensed under Creative Commons - Attribution license.
