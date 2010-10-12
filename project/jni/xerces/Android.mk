LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

XERCES_SUBDIRS := $(patsubst $(LOCAL_PATH)/%, %, $(shell find $(LOCAL_PATH)/src/xercesc/ -type d))

LOCAL_MODULE := xerces

LOCAL_CFLAGS := -DHAVE_CONFIG_H -I$(LOCAL_PATH) -I$(LOCAL_PATH)/src -I$(LOCAL_PATH)/.. -I$(LOCAL_PATH)/../iconv

LOCAL_CPP_EXTENSION := .cpp

LOCAL_SRC_FILES := $(addprefix src/,$(notdir $(wildcard $(LOCAL_PATH)/src/*.cpp $(LOCAL_PATH)/src/*.c)))
LOCAL_SRC_FILES += $(foreach F, $(XERCES_SUBDIRS), $(addprefix $(F)/,$(notdir $(wildcard $(LOCAL_PATH)/$(F)/*.cpp))))

LOCAL_STATIC_LIBRARIES := iconv

include $(BUILD_SHARED_LIBRARY)
