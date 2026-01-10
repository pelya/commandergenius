LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := openal

APP_SUBDIRS := $(patsubst $(LOCAL_PATH)/%, %, $(shell find $(LOCAL_PATH)/src -type d))

LOCAL_C_INCLUDES := $(foreach D, $(APP_SUBDIRS), $(LOCAL_PATH)/$(D)) $(LOCAL_PATH)/include
LOCAL_CFLAGS := -O3 -DHAVE_CONFIG_H -DAL_ALEXT_PROTOTYPES $(if $(filter 1.2, $(SDL_VERSION)), -DAL_SDL_ONPAUSE_CALLBACK)

LOCAL_CPP_EXTENSION := .cpp

LOCAL_SRC_FILES := $(foreach F, $(APP_SUBDIRS), $(addprefix $(F)/,$(notdir $(wildcard $(LOCAL_PATH)/$(F)/*.cpp))))
LOCAL_SRC_FILES += $(foreach F, $(APP_SUBDIRS), $(addprefix $(F)/,$(notdir $(wildcard $(LOCAL_PATH)/$(F)/*.c))))

LOCAL_SHARED_LIBRARIES := $(if $(filter 1.2, $(SDL_VERSION)), sdl-1.2, SDL2)

LOCAL_STATIC_LIBRARIES :=

LOCAL_LDLIBS := -llog

include $(BUILD_SHARED_LIBRARY)
