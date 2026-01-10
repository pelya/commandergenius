LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := $(notdir $(LOCAL_PATH))

ifneq ($(filter $(LOCAL_MODULE), $(APP_MODULES)),)

LOCAL_C_INCLUDES := $(LOCAL_PATH)/include

LOCAL_SRC_FILES := lib/$(TARGET_ARCH_ABI)/lib$(LOCAL_MODULE).so

include $(PREBUILT_SHARED_LIBRARY)

endif
