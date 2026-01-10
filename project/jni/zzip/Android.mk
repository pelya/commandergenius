LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := zzip

APP_SUBDIRS := $(patsubst $(LOCAL_PATH)/%, %, $(shell find $(LOCAL_PATH)/zzip -type d))

LOCAL_C_INCLUDES := $(foreach D, $(APP_SUBDIRS), $(LOCAL_PATH)/$(D)) \
					$(LOCAL_PATH)/include $(LOCAL_PATH)/../$(strip $(if $(filter 1.2, $(SDL_VERSION)), sdl-1.2, SDL2))/include
LOCAL_CFLAGS := -O3


LOCAL_CPP_EXTENSION := .cpp

LOCAL_SRC_FILES := $(foreach F, $(APP_SUBDIRS), $(addprefix $(F)/,$(notdir $(wildcard $(LOCAL_PATH)/$(F)/*.cpp))))
LOCAL_SRC_FILES += $(foreach F, $(APP_SUBDIRS), $(addprefix $(F)/,$(notdir $(wildcard $(LOCAL_PATH)/$(F)/*.c))))

LOCAL_SHARED_LIBRARIES := $(if $(filter 1.2, $(SDL_VERSION)), sdl-1.2, SDL2)

LOCAL_STATIC_LIBRARIES := 

LOCAL_LDLIBS := -llog -lz

include $(BUILD_SHARED_LIBRARY)
