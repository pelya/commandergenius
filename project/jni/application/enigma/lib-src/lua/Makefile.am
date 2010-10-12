## Makefile.am -- Process this file with automake to process Makefile.in

noinst_LIBRARIES = liblua.a

# noinst_PROGRAMS = tolua

EXTRA_DIST = README

liblua_a_SOURCES = \
 lapi.c lauxlib.c lbaselib.c lcode.c ldblib.c ldebug.c \
 ldo.c ldump.c lfunc.c lgc.c linit.c liolib.c llex.c \
 lmathlib.c lmem.c loadlib.c lobject.c lopcodes.c loslib.c \
 lparser.c lstate.c lstring.c lstrlib.c ltable.c ltablib.c \
 ltm.c lundump.c lvm.c lzio.c lapi.h lauxlib.h lcode.h \
 ldebug.h ldo.h lfunc.h lgc.h llex.h llimits.h lmem.h lobject.h \
 lopcodes.h lparser.h  lstate.h lstring.h ltable.h ltm.h lua.h \
 luaconf.h lualib.h lundump.h lvm.h lzio.h tolua++.h \
 tolua_event.c tolua_event.h tolua_is.c tolua_map.c tolua_push.c \
 tolua_to.c
liblua_a_CFLAGS = $(AM_CFLAGS)

if LINUX
liblua_a_CFLAGS += -DLUA_USE_LINUX
endif

if CXXLUA
liblua_a_CFLAGS += -x c++ $(AM_CXXFLAGS) $(CXXFLAGS)
endif
