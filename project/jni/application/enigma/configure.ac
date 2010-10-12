# Process this file with autoconf to produce a configure script.
AC_INIT([enigma],[1.01])
AC_PREREQ(2.59)
AC_CANONICAL_BUILD
AC_CANONICAL_TARGET
AC_CONFIG_SRCDIR([src/enigma.cc])

AM_INIT_AUTOMAKE(AC_PACKAGE_TARNAME, AC_PACKAGE_VERSION, -)
AC_CONFIG_HEADERS([src/config.h])
AC_GNU_SOURCE

ALL_LINGUAS="de fr nl it es sv ru hu pt fi"
AC_SUBST(ALL_LINGUAS)
AM_GNU_GETTEXT

case $host_os in
  *mingw32* ) MINGW32=yes;;
          * ) MINGW32=no;;
esac

case $host_os in
  *cygwin* ) CYGWIN=yes;;
         * ) CYGWIN=no;;
esac
case $host_os in
  *linux* ) LINUX=yes;;
        * ) LINUX=no;;
esac

case $host_os in
 *darwin* ) MACOSX=yes;;
       * ) MACOSX=no;;
esac

AM_CONDITIONAL(MINGW32, test x$MINGW32 = xyes)
AM_CONDITIONAL(LINUX, test x$LINUX = xyes)

dnl ======================================================================
dnl Check for programs
dnl ======================================================================
CXXTMPFLAGS="$CXXFLAGS"
AC_PROG_CC
AM_PROG_CC_C_O
AC_PROG_CXX
AC_PROG_CPP
AC_PROG_RANLIB
dnl do not add the AC_PROG_CXX default CXXFLAGS
CXXFLAGS="$CXXTMPFLAGS"

dnl Add -mno-cygwin to CXXFLAGS and CFLAGS if working under cygwin
if test "x$CYGWIN" = xyes; then
  CXXFLAGS="$CXXFLAGS -mno-cygwin"
  CFLAGS="$CFLAGS -mno-cygwin"
  CPPFLAGS="$CXXFLAGS -mno-cygwin"
  WINDRES="${WINDRES-windres}"
fi
if test "x$WINDRES" = x; then
  WINDRES="windres"
fi
AC_SUBST(WINDRES)

dnl Add -DMACOSX to CXXFLAGS and CFLAGS if working under darwin
if test "x$MACOSX" = xyes; then
 CPPFLAGS="$CPPFLAGS -DMACOSX"
fi

dnl ---------- Texi2html ----------
AC_PATH_PROG(TEXI2HTML, texi2html, "")

dnl ---------- Use the included tolua ----------
AC_SUBST([TOLUA])
TOLUA=["\$(top_builddir)/tools/tolua"]
dnl AC_PATH_PROG(TOLUA, tolua, "", ["\$(top_builddir)/tools/tolua"])


dnl ======================================================================
dnl Check for system headers
dnl ======================================================================
AC_CHECK_HEADERS([dirent.h])

dnl ======================================================================
dnl Check for libraries
dnl ======================================================================
AC_SEARCH_LIBS([dlopen], [dl])

dnl When using the included gettext package from the /intl subdir
dnl /intl needs to be added to the list of include folders
if test "$BUILD_INCLUDED_LIBINTL" = yes; then
   CXXFLAGS="$CXXFLAGS -I\$(top_srcdir)/intl"
fi

dnl This is to check for winmm which must be included to satisfy enet
dnl when compiling under mingw32
if test "$MINGW32" = yes; then
   AC_CHECK_LIB(winmm,main)
fi

dnl Save a _NOSDL state set of flags for compilation of tools
CFLAGS_NOSDL="$CFLAGS"
CXXFLAGS_NOSDL="$CXXFLAGS"
LIBS_NOSDL="$LIBS"

AM_PATH_SDL(1.2.0)
AC_SUBST(SDL_LIBS)
AC_SUBST(SDL_CFLAGS)
dnl The flags are now substituted inside the Makefile, but not used for the
dnl rest of the script. So we manually put them in. This is necessary if
dnl SDL has not been installed in the canonical locations.

CFLAGS="$CFLAGS $SDL_CFLAGS"
CXXFLAGS="$CXXFLAGS $SDL_CFLAGS"
LIBS="$LIBS $SDL_LIBS"

dnl ----------------------------------------
dnl Check for SDL image library
dnl ----------------------------------------
AC_CHECK_LIB(SDL_image,main,,[AC_MSG_ERROR([SDL_image is required to compile Enigma])])

SDL_LIBS="$SDL_LIBS -lSDL_image"

AC_CHECK_LIB(SDL_image, IMG_Init, have_img_init="yes", have_img_init="no")
if test "$have_img_init" = yes; then
  AC_DEFINE(SDL_IMG_INIT, , [Whether SDL_image library provides init])
fi

dnl ----------------------------------------
dnl Check for SDL mixer library
dnl ----------------------------------------
AC_CHECK_LIB(SDL_mixer,main,,[AC_MSG_ERROR([SDL_mixer is required to compile Enigma])])

SDL_LIBS="$SDL_LIBS -lSDL_mixer"

AC_CHECK_LIB(SDL_mixer, Mix_QuickLoad_RAW, have_mix_quickload="yes", have_mix_quickload="no")
if test "$have_mix_quickload" = no; then
  AC_MSG_ERROR([SDL_mixer >= 1.2.5 is required to compile Enigma])
fi

AC_CHECK_LIB(SDL_mixer, Mix_Init, have_mix_init="yes", have_mix_init="no")
if test "$have_mix_init" = yes; then
  AC_DEFINE(SDL_MIX_INIT, , [Whether SDL_mixer library provides init])
fi

dnl ----------------------------------------
dnl Check for SDL_ttf library
dnl ---------------------------------------
AC_MSG_CHECKING([for SDL_ttf >=2.0.6])
AC_RUN_IFELSE([AC_LANG_SOURCE(
    [[#include <SDL_ttf.h>
      int main(int argc, char *argv[]) {
      if (TTF_MAJOR_VERSION < 2)
	return 1;
      else if (TTF_MAJOR_VERSION == 2 && TTF_MINOR_VERSION == 0 && TTF_PATCHLEVEL < 6) 
	return 1;
      return 0;}]])],
  [AC_MSG_RESULT([found])],
  [AC_MSG_ERROR([SDL_ttf >= 2.0.6 not found.])])
AC_CHECK_LIB(SDL_ttf, main,,[AC_MSG_ERROR([SDL_ttf is required to compile Enigma])])

SDL_LIBS="$SDL_LIBS -lSDL_ttf"

dnl ---------------------------------------
dnl Restoring variables to _NOSDL State
dnl ---------------------------------------

CFLAGS="$CFLAGS_NOSDL"
CXXFLAGS="$CXXFLAGS_NOSDL"
LIBS="$LIBS_NOSDL"

dnl ----------------------------------------
dnl Check for libpng
dnl ---------------------------------------
if test "$MINGW32" = yes; then
  AC_CHECK_LIB(png12,png_create_write_struct,,[AC_MSG_ERROR([libpng is required to compile Enigma])])
else
  AC_CHECK_LIB(png,png_create_write_struct,,[AC_MSG_ERROR([libpng is required to compile Enigma])])
fi

dnl ----------------------------------------
dnl Check for xerces
dnl ----------------------------------------
if test "$MINGW32" = yes; then
  AC_MSG_CHECKING([for Xerces release 3.0])
  AC_EGREP_HEADER([gXercesMajVersion = 3],[xercesc/util/XercesVersion.hpp],
      [AC_MSG_RESULT([found])],
      [AC_MSG_ERROR([not found])])      
   AC_CHECK_LIB(xerces-c, main,,[AC_MSG_ERROR([xerces is required to compile Enigma])])
else
  AC_MSG_CHECKING([for current Xerces release 3.0])
  AC_EGREP_HEADER([gXercesMajVersion = 3],[xercesc/util/XercesVersion.hpp],
      xerces3exp="yes",
      xerces3exp="no")      
  if test "x$xerces3exp" = xyes; then
    AC_MSG_RESULT([found])
    AC_CHECK_LIB(xerces-c, main,,[AC_MSG_ERROR([xerces is required to compile Enigma])])
  else
    AC_MSG_RESULT([not found])
    AC_MSG_CHECKING([for old Xerces release >=2.4])
    AC_RUN_IFELSE([AC_LANG_PROGRAM([[#include <xercesc/util/XercesVersion.hpp>]],
          [[if (XERCES_VERSION_MAJOR < 2)
              return 1;
            else if (XERCES_VERSION_MAJOR == 2 && XERCES_VERSION_MINOR < 4) 
              return 1;
            return 0;]])],
        [AC_MSG_RESULT([found])],
        [AC_MSG_ERROR([Xerces >= 2.4 not found.])])
    AC_CHECK_LIB(xerces-c, main,,[AC_MSG_ERROR([xerces is required to compile Enigma])])
  fi
fi

dnl ----------------------------------------
dnl Activate optimizations when profiling, to get rid of
dnl inlineable functions.
dnl ----------------------------------------
AC_MSG_CHECKING(whether to include profiling information)
AC_ARG_ENABLE(profile,
  AS_HELP_STRING(--enable-profile,Compile with profiling information), ,
  enable_profile=no
)
if test "x$enable_profile" = xyes; then
  CXXFLAGS="$CXXFLAGS -pg -O2"
  AC_MSG_RESULT(yes)
else
  AC_MSG_RESULT(no)
fi

dnl ----------------------------------------
dnl Build the developer tools?
dnl ----------------------------------------

AC_ARG_ENABLE(tools,
  AS_HELP_STRING(--enable-tools,Build developer tools [default=no]), ,
  enable_tools=no
)

AM_CONDITIONAL(BUILDTOOLS, [test "$enable_tools" = yes])
AM_CONDITIONAL(BUILDTTF2BMF, [test "$have_sdlttf" = yes -a "$enable_tools" = yes])

dnl ----------------------------------------
dnl Include experimental features?
dnl ----------------------------------------
AC_ARG_ENABLE(experimental,
  AS_HELP_STRING(--enable-experimental,Include experimental features [default=no]), 
  , enable_experimental=no)
if test "x$enable_experimental" = xyes; then
  AC_DEFINE(ENABLE_EXPERIMENTAL, , [Include experimental features])
fi

dnl ----------------------------------------
dnl Include asserts ?
dnl ----------------------------------------
AC_MSG_CHECKING(whether to include assert statements)
AC_ARG_ENABLE(assert,
  AS_HELP_STRING(--enable-assert,Include assert statements [default=yes]), 
  , enable_assert=yes)
if test "x$enable_assert" = xyes; then
  CXXFLAGS="$CXXFLAGS -DENABLE_ASSERT"
  AC_MSG_RESULT(yes)
else
  AC_MSG_RESULT(no)
fi

dnl ----------------------------------------
dnl Optimize?
dnl ----------------------------------------

AC_MSG_CHECKING(whether to optimize)
AC_ARG_ENABLE(optimize,
  AS_HELP_STRING(--enable-optimize,Compile with optimizations [default=no]), ,
  enable_optimize=no
)
if test "x$enable_optimize" = xyes; then
	CXXFLAGS="$CXXFLAGS -O2 -ffast-math -fomit-frame-pointer"
	AC_MSG_RESULT(yes)
else
	CXXFLAGS="$CXXFLAGS -g"
	AC_MSG_RESULT(no)
fi

dnl ----------------------------------------
dnl Check for gdb debugging
dnl ----------------------------------------

AC_MSG_CHECKING(whether to debug the game with gdb)
AC_ARG_ENABLE(debug-gdb,
  AS_HELP_STRING(--enable-debug-gdb,Compile with special debugging options for gdb), ,
  enable_debug_gdb=no
)
if test "x$enable_debug_gdb" = xyes; then
	CXXFLAGS="$CXXFLAGS -ggdb3 -fno-inline -fno-default-inline -fno-omit-frame-pointer -fno-optimize-sibling-calls"
	AC_MSG_RESULT(yes)
else
	CXXFLAGS="$CXXFLAGS"
	AC_MSG_RESULT(no)
fi

dnl ----------------------------------------
dnl Check for C++ Lua
dnl ----------------------------------------

AC_MSG_CHECKING(whether to build lua using c++)
AC_ARG_ENABLE(cxxlua,
  AS_HELP_STRING(--enable-cxxlua, Build Lua with C++ [default=yes]),,enable_cxxlua=yes)
if test "x$enable_cxxlua" = xyes; then
  CXXFLAGS="$CXXFLAGS -DCXXLUA"
  AC_MSG_RESULT(yes)
else
  AC_MSG_RESULT(no)
fi

AM_CONDITIONAL(CXXLUA, test x$enable_cxxlua = xyes)

dnl ----------------------------------------------------------
dnl Check whether compiler warnings should be emitted
dnl ----------------------------------------------------------

AC_MSG_CHECKING(whether to enable warnings)
AC_ARG_ENABLE(warnings,
  AS_HELP_STRING(--enable-warnings,Enable additional compiler warnings), ,
  enable_warnings=no
)
if test "x$enable_warnings" = xyes; then
	CXXFLAGS="$CXXFLAGS -O2 -Wall -W"
	CXXFLAGS="$CXXFLAGS -Wundef -Wshadow -Wpointer-arith -Wcast-qual -Wcast-align"
	CXXFLAGS="$CXXFLAGS -Wwrite-strings -Wconversion -Wsign-compare -Wformat"
	CXXFLAGS="$CXXFLAGS -Wdisabled-optimization -Wno-system-headers"
	AC_MSG_RESULT(yes)
else
	CXXFLAGS="$CXXFLAGS"
	AC_MSG_RESULT(no)
fi

dnl ----------------------------------------------------------
dnl Configure libraries
dnl ----------------------------------------------------------
AC_CONFIG_SUBDIRS(lib-src/zipios++)
AC_CONFIG_SUBDIRS(lib-src/enet)

AC_CONFIG_FILES([Makefile m4/Makefile  intl/Makefile 
           data/Makefile
           data/gfx/Makefile
           data/gfx32/Makefile
           data/gfx40/Makefile
           data/gfx48/Makefile
           data/levels/Makefile
           data/levels/enigma_tutorial/Makefile
           data/levels/enigma_i/Makefile
           data/levels/enigma_ii/Makefile
           data/levels/enigma_iii/Makefile
           data/levels/enigma_iv/Makefile
           data/levels/enigma_v/Makefile
           data/levels/enigma_vi/Makefile
           data/levels/enigma_vii/Makefile
           data/levels/enigma_cross/Makefile
           data/levels/enigma_esprit/Makefile
           data/levels/enigma_oxyd/Makefile
           data/levels/enigma_peroxyd/Makefile
           data/levels/enigma_oxydmagnum/Makefile
           data/levels/enigma_oxydextra/Makefile
           data/levels/enigma_experimental/Makefile
           data/levels/enigma_stable/Makefile
           data/levels/enigma_sokoban/Makefile
           data/levels/enigma_microban/Makefile
           data/levels/enigma_mas_microban/Makefile
           data/levels/lib/Makefile
           data/levels/patches/Makefile
           data/fonts/Makefile
           data/sound/Makefile
           data/soundsets/Makefile
           data/soundsets/enigma/Makefile
           data/schemas/Makefile
	   doc/Makefile
	   doc/images/Makefile
	   doc/images/flags25x15/Makefile
	   doc/manual/Makefile
	   doc/manual/images/Makefile
	   doc/reference/Makefile
	   doc/reference/images/Makefile
	   etc/Makefile
           lib-src/Makefile
	   lib-src/oxydlib/Makefile
	   lib-src/lua/Makefile
           lib-src/enigma-core/Makefile
	   src/Makefile
	   tools/Makefile
	   etc/enigma.spec
	   etc/enigma.nsi
	   etc/Info.plist
	   po/Makefile.in
])

AC_CONFIG_FILES([etc/mingw32-dist.sh],[chmod +x etc/mingw32-dist.sh])

AC_OUTPUT

AC_MSG_RESULT([

Enigma is now configured

   Source directory:    $srcdir
   Installation prefix: $prefix
   C++ compiler:        $CXX $CXXFLAGS
   Libraries:           $LIBS
   Linker options:      $LDFLAGS
   Languages:		$ALL_LINGUAS

If these values seem to make sense, you can now run make.
])

