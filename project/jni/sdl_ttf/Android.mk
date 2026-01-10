LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := sdl_ttf

LOCAL_C_INCLUDES := $(LOCAL_PATH) $(LOCAL_PATH)/../sdl-1.2/include $(LOCAL_PATH)/../freetype/include $(LOCAL_PATH)/include
LOCAL_CFLAGS := -Os

LOCAL_CPP_EXTENSION := .cpp

LOCAL_SRC_FILES := SDL_ttf.c

LOCAL_SHARED_LIBRARIES := sdl-1.2
LOCAL_STATIC_LIBRARIES := freetype harfbuzz
LOCAL_LDLIBS := -lz

include $(BUILD_SHARED_LIBRARY)

