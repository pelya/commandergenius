LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := application

APP_SUBDIRS := $(patsubst $(LOCAL_PATH)/%, %, $(shell find $(LOCAL_PATH)/src/ -type d))
ifneq ($(APPLICATION_SUBDIRS_BUILD),)
APP_SUBDIRS := $(APPLICATION_SUBDIRS_BUILD)
endif

LOCAL_CFLAGS := -DHAS_SOCKLEN_T

ifeq ($(CRYSTAX_TOOLCHAIN),)
# Paths should be on newline so launchConfigure.sh will work properly
LOCAL_CFLAGS += \
				-I$(LOCAL_PATH)/../stlport/stlport
endif

# Paths should be on newline so launchConfigure.sh will work properly
LOCAL_CFLAGS += \
				$(foreach D, $(APP_SUBDIRS), -I$(LOCAL_PATH)/$(D)) \
				-I$(LOCAL_PATH)/../sdl/include \
				-I$(LOCAL_PATH)/../sdl_mixer \
				-I$(LOCAL_PATH)/../sdl_image \
				-I$(LOCAL_PATH)/../sdl_ttf \
				-I$(LOCAL_PATH)/../sdl_net \
				-I$(LOCAL_PATH)/../sdl_blitpool \
				-I$(LOCAL_PATH)/../sdl_gfx \
				-I$(LOCAL_PATH)/../png \
				-I$(LOCAL_PATH)/../jpeg \
				-I$(LOCAL_PATH)/../intl \
				-I$(LOCAL_PATH)/../freetype/include \
				-I$(LOCAL_PATH)/../xml2/include \
				-I$(LOCAL_PATH)/..

LOCAL_CFLAGS += $(APPLICATION_ADDITIONAL_CFLAGS)

#Change C++ file extension as appropriate
LOCAL_CPP_EXTENSION := .cpp

LOCAL_SRC_FILES := $(foreach F, $(APP_SUBDIRS), $(addprefix $(F)/,$(notdir $(wildcard $(LOCAL_PATH)/$(F)/*.cpp))))
# Uncomment to also add C sources
LOCAL_SRC_FILES += $(foreach F, $(APP_SUBDIRS), $(addprefix $(F)/,$(notdir $(wildcard $(LOCAL_PATH)/$(F)/*.c))))

ifneq ($(APPLICATION_CUSTOM_BUILD_SCRIPT),)
LOCAL_SRC_FILES := dummy.c
endif

LOCAL_SHARED_LIBRARIES := sdl $(COMPILED_LIBRARIES)

LOCAL_STATIC_LIBRARIES := stlport

LOCAL_LDLIBS := -lGLESv1_CM -ldl -llog -lz

LOCAL_LDFLAGS := -Lobj/local/armeabi

LOCAL_LDFLAGS += $(APPLICATION_ADDITIONAL_LDFLAGS)


LIBS_WITH_LONG_SYMBOLS := $(strip $(shell \
	for f in $(LOCAL_PATH)/../../libs/armeabi/*.so ; do \
		if echo $$f | grep "libapplication[.]so" > /dev/null ; then \
			continue ; \
		fi ; \
		if [ -e "$$f" ] ; then \
			if nm -g $$f | cut -c 12- | egrep '.{128}' > /dev/null ; then \
				echo $$f | grep -o 'lib[^/]*[.]so' ; \
			fi ; \
		fi ; \
	done \
) )

ifneq "$(LIBS_WITH_LONG_SYMBOLS)" ""
$(foreach F, $(LIBS_WITH_LONG_SYMBOLS), \
$(info Library $(F): abusing symbol names are: \
$(shell nm -g $(LOCAL_PATH)/../../libs/armeabi/$(F) | cut -c 12- | egrep '.{128}' ) ) \
$(info Library $(F) contains symbol names longer than 128 bytes, \
YOUR CODE WILL DEADLOCK WITHOUT ANY WARNING when you'll access such function - \
please make this library static to avoid problems. ) )
$(error Detected libraries with too long symbol names. Remove all files under project/libs/armeabi, make these libs static, and recompile)
endif

include $(BUILD_SHARED_LIBRARY)

ifneq ($(APPLICATION_CUSTOM_BUILD_SCRIPT),)

$(info LOCAL_PATH $(LOCAL_PATH) )
$(info $(LOCAL_PATH)/src/libapplication.so )
$(info $(realpath $(LOCAL_PATH)/../../obj/local/armeabi/libapplication.so) )

LOCAL_PATH_SDL_APPLICATION := $(LOCAL_PATH)

$(LOCAL_PATH)/src/libapplication.so: $(LOCAL_PATH)/src/AndroidBuild.sh $(LOCAL_PATH)/src/AndroidAppSettings.cfg
	cd $(LOCAL_PATH_SDL_APPLICATION)/src && ./AndroidBuild.sh

# $(realpath $(LOCAL_PATH)/../../libs/armeabi/libapplication.so) \

$(realpath $(LOCAL_PATH)/../../obj/local/armeabi/libapplication.so): $(LOCAL_PATH)/src/libapplication.so OVERRIDE_CUSTOM_LIB
	cp -f $< $@
#	$(patsubst %-gcc,%-strip,$(TARGET_CC)) -g $@

.PHONY: OVERRIDE_CUSTOM_LIB

OVERRIDE_CUSTOM_LIB:

endif
