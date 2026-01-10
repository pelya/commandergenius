LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := application

APPDIR := $(shell readlink $(LOCAL_PATH)/src)

LOCAL_SHARED_LIBRARIES := $(if $(filter 1.2, $(SDL_VERSION)), sdl-1.2, SDL2) $(filter-out $(APP_AVAILABLE_STATIC_LIBS), $(COMPILED_LIBRARIES))

LOCAL_STATIC_LIBRARIES := $(filter $(APP_AVAILABLE_STATIC_LIBS), $(COMPILED_LIBRARIES))

LOCAL_LDLIBS := $(APPLICATION_GLES_LIBRARY) -ldl -llog -lz

LOCAL_LDFLAGS := -Lobj/local/$(TARGET_ARCH_ABI)

LOCAL_LDFLAGS += $(APPLICATION_ADDITIONAL_LDFLAGS)

ifeq ($(APPLICATION_CUSTOM_BUILD_SCRIPT),)

APP_SUBDIRS := $(patsubst $(LOCAL_PATH)/%, %, $(shell find $(LOCAL_PATH)/$(APPDIR) -path '*/.svn' -prune -o -type d -print))
ifneq ($(APPLICATION_SUBDIRS_BUILD),)
APPLICATION_SUBDIRS_BUILD_NONRECURSIVE := $(addprefix $(APPDIR)/, $(filter-out %/*, $(APPLICATION_SUBDIRS_BUILD)))
APPLICATION_SUBDIRS_BUILD_RECURSIVE := $(patsubst %/*, %, $(filter %/*,$(APPLICATION_SUBDIRS_BUILD)))
APPLICATION_SUBDIRS_BUILD_RECURSIVE := $(foreach FINDDIR, $(APPLICATION_SUBDIRS_BUILD_RECURSIVE), $(shell find $(LOCAL_PATH)/$(APPDIR)/$(FINDDIR) -path '*/.svn' -prune -o -type d -print))
APPLICATION_SUBDIRS_BUILD_RECURSIVE := $(patsubst $(LOCAL_PATH)/%, %, $(APPLICATION_SUBDIRS_BUILD_RECURSIVE) )
APP_SUBDIRS := $(APPLICATION_SUBDIRS_BUILD_NONRECURSIVE) $(APPLICATION_SUBDIRS_BUILD_RECURSIVE)
endif

LOCAL_SRC_FILES := $(filter %.c %.cpp %.cc, $(APP_SUBDIRS))
APP_SUBDIRS := $(filter-out %.c %.cpp %.cc, $(APP_SUBDIRS))
LOCAL_SRC_FILES += $(foreach F, $(APP_SUBDIRS), $(addprefix $(F)/,$(notdir $(wildcard $(LOCAL_PATH)/$(F)/*.cpp))))
LOCAL_SRC_FILES += $(foreach F, $(APP_SUBDIRS), $(addprefix $(F)/,$(notdir $(wildcard $(LOCAL_PATH)/$(F)/*.cc))))
LOCAL_SRC_FILES += $(foreach F, $(APP_SUBDIRS), $(addprefix $(F)/,$(notdir $(wildcard $(LOCAL_PATH)/$(F)/*.c))))
LOCAL_SRC_FILES := $(filter-out $(addprefix $(APPDIR)/, $(APPLICATION_BUILD_EXCLUDE)), $(LOCAL_SRC_FILES))

# Disabled because they give slight overhead, add "-frtti -fexceptions" to the AppCflags inside AndroidAppSettings.cfg if you need them
# If you use setEnvironment.sh you may write "env CXXFLAGS='-frtti -fexceptions' ../setEnvironment.sh ./configure".
#LOCAL_CPP_FEATURES := exceptions rtti

LOCAL_CFLAGS :=
LOCAL_C_INCLUDES := $(foreach D, $(APP_SUBDIRS), $(LOCAL_PATH)/$(D))

ifeq ($(APPLICATION_OVERLAPS_SYSTEM_HEADERS),y)
LOCAL_CFLAGS += $(foreach D, $(LOCAL_C_INCLUDES), -iquote$(D))
LOCAL_C_INCLUDES :=
endif

LOCAL_C_INCLUDES += $(LOCAL_PATH)/../$(strip $(if $(filter 1.2, $(SDL_VERSION)), sdl-1.2, SDL2))/include
LOCAL_C_INCLUDES += $(foreach L, $(COMPILED_LIBRARIES), $(LOCAL_PATH)/../$(L)/include)

LOCAL_CFLAGS += $(APPLICATION_ADDITIONAL_CFLAGS)
LOCAL_CPPFLAGS += $(APPLICATION_ADDITIONAL_CPPFLAGS)

# Change C++ file extension as appropriate
LOCAL_CPP_EXTENSION := .cpp .cxx .cc

include $(BUILD_SHARED_LIBRARY)

else ifeq ($(CUSTOM_BUILD_SCRIPT_FIRST_PASS),) # APPLICATION_CUSTOM_BUILD_SCRIPT and not CUSTOM_BUILD_SCRIPT_FIRST_PASS

LOCAL_SRC_FILES := $(APPDIR)/libapplication-$(TARGET_ARCH_ABI).so
LOCAL_MODULE_FILENAME := libapplication

include $(PREBUILT_SHARED_LIBRARY)

endif
