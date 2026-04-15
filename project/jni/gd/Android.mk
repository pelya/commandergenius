LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := gd

LOCAL_C_INCLUDES := $(LOCAL_PATH)/include $(LOCAL_PATH)/src \
        $(LOCAL_PATH)/../jpeg/include $(LOCAL_PATH)/../jpeg/include/android \
        $(LOCAL_PATH)/../png/include $(LOCAL_PATH)/../png/include/android

LOCAL_CFLAGS := -O3 -DHAVE_CONFIG_H

LOCAL_CPP_EXTENSION := .cpp

LOCAL_SRC_FILES := $(addprefix src/, $(notdir $(wildcard $(LOCAL_PATH)/src/*.c) $(wildcard $(LOCAL_PATH)/src/*.cpp)))

LOCAL_STATIC_LIBRARIES := 

include $(BUILD_STATIC_LIBRARY)
