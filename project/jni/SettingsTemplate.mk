
SDL_VERSION := 1.2

# To filter out static libs from all libs in makefile
APP_AVAILABLE_STATIC_LIBS := jpeg png webp freetype fontconfig xerces ogg vorbis tremor flac \
	boost_atomic boost_chrono boost_container boost_context boost_contract boost_coroutine boost_date_time boost_exception boost_filesystem \
	boost_graph boost_iostreams boost_json boost_locale boost_log boost_log_setup boost_math_c99 boost_math_c99f boost_math_c99l boost_math_tr1 \
	boost_math_tr1f boost_math_tr1l boost_nowide boost_prg_exec_monitor boost_program_options boost_random boost_regex boost_serialization \
	boost_system boost_test_exec_monitor boost_thread boost_timer boost_type_erasure boost_unit_test_framework boost_wave boost_wserialization \
	glu icudata icutest icui18n icuio icule iculx icutu icuuc icu-le-hb harfbuzz sdl_savepng android_support \
	gl4es nanogl gd guichan glm

# Available libraries: mad (GPL-ed!) sdl_mixer sdl_image sdl_ttf sdl_net sdl_blitpool sdl_gfx sdl_sound intl xml2 lua jpeg png ogg flac tremor vorbis freetype xerces curl theora fluidsynth lzma lzo2 mikmod openal timidity zzip bzip2 yaml-cpp python boost_date_time boost_filesystem boost_iostreams boost_program_options boost_regex boost_signals boost_system boost_thread glu avcodec avdevice avfilter avformat avresample avutil swscale swresample bzip2 
APP_MODULES := application sdl-1.2 sdl_native_helpers jpeg png ogg flac vorbis freetype tremor ogg

ifeq ($(CUSTOM_BUILD_SCRIPT_FIRST_PASS),)
APP_MODULES += application $(if $(filter 1.2, $(SDL_VERSION)), sdl_main)
endif

ifeq ($(APP_ABI),)
APP_ABI := arm64-v8a armeabi-v7a x86 x86_64
endif

# The namespace in Java file, with dots replaced with underscores
SDL_JAVA_PACKAGE_PATH := net_sourceforge_clonekeenplus

# Path to files with application data - they should be downloaded from Internet on first app run inside
# Java sources, or unpacked from resources (TODO)
# Typically /sdcard/alienblaster 
# Or /data/data/de.schwardtnet.alienblaster/files if you're planning to unpack data in application private folder
# Your application will just set current directory there
SDL_CURDIR_PATH := net.sourceforge.clonekeenplus

# Android Dev Phone G1 has trackball instead of cursor keys, and 
# sends trackball movement events as rapid KeyDown/KeyUp events,
# this will make Up/Down/Left/Right key up events with X frames delay,
# so if application expects you to press and hold button it will process the event correctly.
# TODO: create a libsdl config file for that option and for key mapping/on-screen keyboard
SDL_TRACKBALL_KEYUP_DELAY := 1

# If the application designed for higher screen resolution enable this to get the screen
# resized in HW-accelerated way, however it eats a tiny bit of CPU
SDL_VIDEO_RENDER_RESIZE := 1

COMPILED_LIBRARIES := tremor ogg

APPLICATION_ADDITIONAL_CFLAGS :=

APPLICATION_ADDITIONAL_CPPFLAGS :=

APPLICATION_ADDITIONAL_LDFLAGS :=

APPLICATION_GLES_LIBRARY := -lGLESv1_CM

APPLICATION_OVERLAPS_SYSTEM_HEADERS := n

APPLICATION_SUBDIRS_BUILD := src/*

APPLICATION_BUILD_EXCLUDE := 

APPLICATION_CUSTOM_BUILD_SCRIPT := 

USE_GL4ES :=

SDL_ADDITIONAL_CFLAGS := -DSDL_ANDROID_KEYCODE_MOUSE=UNKNOWN -DSDL_ANDROID_KEYCODE_0=LCTRL -DSDL_ANDROID_KEYCODE_1=LALT -DSDL_ANDROID_KEYCODE_2=SPACE -DSDL_ANDROID_KEYCODE_3=RETURN -DSDL_ANDROID_KEYCODE_4=RETURN

NDK_TOOLCHAIN_VERSION := clang

APP_PLATFORM := android-16
