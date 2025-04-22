include ../Settings.mk

APPDIR := $(shell readlink src)

all: $(foreach ARCH, $(APP_ABI), $(APPDIR)/libapplication-$(ARCH).so)

.PHONY: all $(foreach ARCH, $(APP_ABI), $(APPDIR)/libapplication-$(ARCH).so)

TARGET_GCC_PREFIX_armeabi-v7a := arm-linux-androideabi
TARGET_GCC_PREFIX_x86 := i686-linux-android
TARGET_GCC_PREFIX_arm64-v8a := aarch64-linux-android
TARGET_GCC_PREFIX_x86_64 := x86_64-linux-android

LOCAL_STATIC_LIBRARIES := $(filter $(APP_AVAILABLE_STATIC_LIBS), $(COMPILED_LIBRARIES))

LOCAL_SHARED_LIBRARIES := $(if $(filter 1.2, $(SDL_VERSION)), sdl-1.2, SDL2) $(filter-out $(APP_AVAILABLE_STATIC_LIBS), $(COMPILED_LIBRARIES))

LOCAL_SHARED_LIBRARIES := $(patsubst crypto, crypto.so.sdl.1, $(LOCAL_SHARED_LIBRARIES))
LOCAL_SHARED_LIBRARIES := $(patsubst ssl, ssl.so.sdl.1, $(LOCAL_SHARED_LIBRARIES))
LOCAL_SHARED_LIBRARIES := $(patsubst curl, curl-sdl, $(LOCAL_SHARED_LIBRARIES))
LOCAL_SHARED_LIBRARIES := $(patsubst expat, expat-sdl, $(LOCAL_SHARED_LIBRARIES))

define DEPENDS_FOR_ARCH =

../../obj/local/$(1)/$(2):
	make -C .. -f Makefile.prebuilt $$(abspath $$@)

SDL_APP_LIB_DEPENDS_$(1) += ../../obj/local/$(1)/$(2)

#$$(warning === ../../obj/local/$(1)/$(2):)

endef

$(foreach ARCH, $(APP_ABI), $(foreach LIB, $(LOCAL_SHARED_LIBRARIES), $(eval $(call DEPENDS_FOR_ARCH,$(ARCH),lib$(LIB).so))))

$(foreach ARCH, $(APP_ABI), $(foreach LIB, $(LOCAL_STATIC_LIBRARIES), $(eval $(call DEPENDS_FOR_ARCH,$(ARCH),lib$(LIB).a))))

define BUILD_FOR_ARCH =

SDL_APP_LIB_DEPENDS_$(1) += $$(APPDIR)/AndroidBuild.sh $$(APPDIR)/AndroidAppSettings.cfg

$$(APPDIR)/libapplication-$(1).so: $$(SDL_APP_LIB_DEPENDS_$(1))
	cd $$(APPDIR) && ./AndroidBuild.sh $(1) $$(TARGET_GCC_PREFIX_$(1))
	@objdump -p $$@ | grep 'SONAME' && { \
		objdump -p $$@ | grep 'SONAME *libapplication.so' || { \
			rm $$@ ; echo 'Error: $$@ must have SONAME set to "libapplication.so", add option -Wl,-soname=libapplication.so to your linker flags' ; \
		} ; \
	}

#$$(warning ====== $$(APPDIR)/libapplication-$(1).so: ==> $$(SDL_APP_LIB_DEPENDS_$(1)))

endef

$(foreach ARCH, $(APP_ABI), $(eval $(call BUILD_FOR_ARCH,$(ARCH))))
