LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := sdl_savepng

LOCAL_C_INCLUDES := $(LOCAL_PATH) $(LOCAL_PATH)/../png/include \
    $(LOCAL_PATH) $(LOCAL_PATH)/../png/include/android \
    $(LOCAL_PATH)/../sdl-1.2/include \
    $(LOCAL_PATH)/include

LOCAL_CPP_EXTENSION := .cpp

LOCAL_SRC_FILES := $(notdir $(wildcard $(LOCAL_PATH)/*.c))

LOCAL_STATIC_LIBRARIES := png

LOCAL_SHARED_LIBRARIES := sdl-1.2

include $(BUILD_STATIC_LIBRARY)
