LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := sdl_main

ifndef SDL_JAVA_PACKAGE_PATH
$(error Please define SDL_JAVA_PACKAGE_PATH to the path of your Java package with dots replaced with underscores, for example "com_example_SanAngeles")
endif

LOCAL_C_INCLUDES := $(LOCAL_PATH)/../sdl-1.2/include
LOCAL_CFLAGS := -DSDL_JAVA_PACKAGE_PATH=$(SDL_JAVA_PACKAGE_PATH) -DSDL_CURDIR_PATH=\"$(SDL_CURDIR_PATH)\"

LOCAL_CPP_EXTENSION := .cpp

LOCAL_SRC_FILES := sdl_main.c

LOCAL_SHARED_LIBRARIES := sdl-1.2 application
LOCAL_LDLIBS := -llog

ifeq ($(CUSTOM_BUILD_SCRIPT_FIRST_PASS),)

include $(BUILD_SHARED_LIBRARY)

endif
