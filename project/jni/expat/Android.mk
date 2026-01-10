LOCAL_PATH:= $(call my-dir)

common_SRC_FILES := \
	lib/xmlparse.c \
	lib/xmlrole.c \
	lib/xmltok.c

common_CFLAGS := \
    -Wall \
    -Wmissing-prototypes -Wstrict-prototypes \
    -Wno-unused-parameter -Wno-missing-field-initializers \
    -fexceptions \
    -DHAVE_EXPAT_CONFIG_H \
    -I$(LOCAL_PATH)/lib -I$(LOCAL_PATH)

common_C_INCLUDES += \
	$(LOCAL_PATH)/include

common_COPY_HEADERS_TO := libexpat
common_COPY_HEADERS := \
	lib/expat.h \
	lib/expat_external.h

# Device shared library
include $(CLEAR_VARS)

LOCAL_SRC_FILES := $(common_SRC_FILES)
LOCAL_CFLAGS += $(common_CFLAGS)
LOCAL_C_INCLUDES += $(common_C_INCLUDES)

LOCAL_MODULE := expat
LOCAL_MODULE_FILENAME := libexpat-sdl # It clashes with system libexpat on Android 5 and older

include $(BUILD_SHARED_LIBRARY)
