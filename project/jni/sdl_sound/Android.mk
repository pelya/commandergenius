LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := sdl_sound

LOCAL_C_INCLUDES := $(LOCAL_PATH) $(LOCAL_PATH)/../sdl-1.2/include $(LOCAL_PATH)/include $(LOCAL_PATH)/decoders $(LOCAL_PATH)/decoders/mpglib \
					$(LOCAL_PATH)/../flac/include $(LOCAL_PATH)/../ogg/include \
					$(LOCAL_PATH)/../vorbis/include $(LOCAL_PATH)/../tremor/include $(LOCAL_PATH)/../mikmod/include \
					$(LOCAL_PATH)/timidity
LOCAL_CFLAGS := -O3 -DHAVE_CONFIG_H -DLAYER1 -DLAYER2 -DLAYER3

LOCAL_CPP_EXTENSION := .cpp

LOCAL_SRC_FILES := $(notdir $(wildcard $(LOCAL_PATH)/*.c)) \
					$(addprefix decoders/, $(notdir $(wildcard $(LOCAL_PATH)/decoders/*.c))) \
					$(addprefix decoders/mpglib/, $(notdir $(wildcard $(LOCAL_PATH)/decoders/mpglib/*.c)))

LOCAL_SHARED_LIBRARIES := sdl-1.2
LOCAL_STATIC_LIBRARIES := flac mikmod vorbis ogg

LOCAL_LDLIBS :=

include $(BUILD_SHARED_LIBRARY)

