LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_MODULE := vncserver

# Hide vncserver from the project if openssl is not compiled
ifneq ($(filter ssl, $(APP_MODULES)),)

LIBVNCSERVER_ROOT:=src

LIBVNCSERVER_SRC_FILES:= \
	$(LIBVNCSERVER_ROOT)/libvncserver/main.c \
	$(LIBVNCSERVER_ROOT)/libvncserver/rfbserver.c \
	$(LIBVNCSERVER_ROOT)/libvncserver/rfbregion.c \
	$(LIBVNCSERVER_ROOT)/libvncserver/auth.c \
	$(LIBVNCSERVER_ROOT)/libvncserver/sockets.c \
	$(LIBVNCSERVER_ROOT)/libvncserver/stats.c \
	$(LIBVNCSERVER_ROOT)/libvncserver/corre.c \
	$(LIBVNCSERVER_ROOT)/libvncserver/rfbssl_openssl.c \
	$(LIBVNCSERVER_ROOT)/libvncserver/rfbcrypto_openssl.c \
	$(LIBVNCSERVER_ROOT)/libvncserver/hextile.c \
	$(LIBVNCSERVER_ROOT)/libvncserver/rre.c \
	$(LIBVNCSERVER_ROOT)/libvncserver/translate.c \
	$(LIBVNCSERVER_ROOT)/libvncserver/cutpaste.c \
	$(LIBVNCSERVER_ROOT)/libvncserver/httpd.c \
	$(LIBVNCSERVER_ROOT)/libvncserver/cursor.c \
	$(LIBVNCSERVER_ROOT)/libvncserver/font.c \
	$(LIBVNCSERVER_ROOT)/libvncserver/draw.c \
	$(LIBVNCSERVER_ROOT)/libvncserver/websockets.c \
	$(LIBVNCSERVER_ROOT)/libvncserver/selbox.c \
	$(LIBVNCSERVER_ROOT)/libvncserver/cargs.c \
	$(LIBVNCSERVER_ROOT)/libvncserver/ultra.c \
	$(LIBVNCSERVER_ROOT)/libvncserver/scale.c \
	$(LIBVNCSERVER_ROOT)/libvncserver/zlib.c \
	$(LIBVNCSERVER_ROOT)/libvncserver/zrle.c \
	$(LIBVNCSERVER_ROOT)/libvncserver/zrleoutstream.c \
	$(LIBVNCSERVER_ROOT)/libvncserver/zrlepalettehelper.c \
	$(LIBVNCSERVER_ROOT)/libvncserver/tight.c \
	$(LIBVNCSERVER_ROOT)/common/d3des.c \
	$(LIBVNCSERVER_ROOT)/common/vncauth.c \
	$(LIBVNCSERVER_ROOT)/common/minilzo.c \
	$(LIBVNCSERVER_ROOT)/common/zywrletemplate.c \
	$(LIBVNCSERVER_ROOT)/common/turbojpeg.c

LOCAL_CFLAGS  +=  -Wall \
					-O3 \
					-DLIBVNCSERVER_WITH_WEBSOCKETS \
					-DLIBVNCSERVER_HAVE_LIBPNG \
					-DLIBVNCSERVER_HAVE_ZLIB \
					-DLIBVNCSERVER_HAVE_LIBJPEG \
					-I$(LOCAL_PATH)/include \
					-I$(LOCAL_PATH)/src \
					-I$(LOCAL_PATH)/src/common \
					-I$(LOCAL_PATH)/src/libvncserver \
					-I$(LOCAL_PATH)/../jpeg/include \
					-I$(LOCAL_PATH)/../png/include \
					-I$(LOCAL_PATH)/../crypto/include \

LOCAL_LDLIBS +=  -llog -lz -ldl

LOCAL_SRC_FILES += $(LIBVNCSERVER_SRC_FILES)

LOCAL_C_INCLUDES += include

LOCAL_STATIC_LIBRARIES := jpeg png ssl crypto

include $(BUILD_SHARED_LIBRARY)

endif
