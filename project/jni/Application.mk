APP_PROJECT_PATH := $(call my-dir)/..

# Available libraries: mad sdl_mixer sdl_image sdl_ttf sdl_net sdl_blitpool sdl_gfx intl xml2 xerces
# sdl_mixer depends on tremor and optionally mad
# sdl_image depends on png and jpeg
# sdl_ttf depends on freetype
# xerces depends on iconv

APP_MODULES := application sdl sdl_main stlport tremor png jpeg freetype sdl_mixer sdl_image sdl_ttf intl xerces

APP_ABI := armeabi armeabi-v7a
