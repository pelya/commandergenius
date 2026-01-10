APP_PROJECT_PATH := $(call my-dir)/..

include jni/Settings.mk

APP_STL := c++_shared

APP_CFLAGS := -g
ifneq ($(NDK_DEBUG),1)
APP_CFLAGS += -Oz -DNDEBUG # -Oz works best with clang
endif
APP_PIE := true # This feature makes executables incompatible to Android API 15 or lower, but executables without PIE will not run on Android 5.0 and newer
SDL_EXCLUDE_LIBGCC := -Wl,--exclude-libs,libgcc.a
SDL_EXCLUDE_LIBUNWIND := -Wl,--exclude-libs,libunwind.a
APP_LDFLAGS = $(if $(filter clang%, $(NDK_TOOLCHAIN_VERSION)), $(SDL_EXCLUDE_LIBGCC) $(if $(filter armeabi%, $(APP_ABI)), $(SDL_EXCLUDE_LIBUNWIND)))
