LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := iconv

LOCAL_CFLAGS := -I$(LOCAL_PATH) 

LOCAL_CPP_EXTENSION := .cpp

LOCAL_SRC_FILES := iconv.c

include $(BUILD_STATIC_LIBRARY)
