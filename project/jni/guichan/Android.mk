LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := guichan

APP_SUBDIRS := $(patsubst $(LOCAL_PATH)/%, %, $(shell find $(LOCAL_PATH)/src -type d))

LOCAL_C_INCLUDES := $(LOCAL_PATH) $(LOCAL_PATH)/../sdl-1.2/include $(LOCAL_PATH)/include \
					$(LOCAL_PATH)/../sdl_image/include $(LOCAL_PATH)/../sdl_ttf/include
LOCAL_CFLAGS := -DHAVE_CONFIG_H -D_GNU_SOURCE=1 -D_REENTRANT -fexceptions -frtti

LOCAL_CPP_EXTENSION := .cpp

LOCAL_SRC_FILES := $(foreach F, $(APP_SUBDIRS), $(addprefix $(F)/,$(notdir $(wildcard $(LOCAL_PATH)/$(F)/*.cpp))))
LOCAL_SRC_FILES += $(foreach F, $(APP_SUBDIRS), $(addprefix $(F)/,$(notdir $(wildcard $(LOCAL_PATH)/$(F)/*.c))))

LOCAL_SHARED_LIBRARIES := sdl-1.2
LOCAL_SHARED_LIBRARIES += sdl_image sdl_ttf

LOCAL_STATIC_LIBRARIES := 

include $(BUILD_STATIC_LIBRARY)
