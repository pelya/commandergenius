dnl Process this file with autoconf to produce a configure script.
AC_INIT
AC_CONFIG_SRCDIR([src/backbuffer.h])

AM_INIT_AUTOMAKE(zipios++,0.1.5)
AC_CONFIG_HEADERS([zipios++/zipios-config.h])


dnl Add -mno-cygwin to CXXFLAGS and CFLAGS if working under cygwin
dnl if test "$host_os in *cygwin*"; then
dnl   CXXFLAGS="$CXXFLAGS -mno-cygwin"
dnl   CFLAGS="$CFLAGS -mno-cygwin"
dnl fi

dnl to run test progs with CXX: AC_LANG([C++])

dnl Use std compliant iostream library if present?
AC_ARG_WITH(std-compliant-iostream, 
[  --with-std-compliant-iostream 
         Include iostream (if present) instead of iostream.h. (Default is yes) ] )

if test -n "$with_std_compliant_iostream" -a \
        "$with_std_compliant_iostream" != "no" -a \
        "$with_std_compliant_iostream" != "yes" ; then
  AC_MSG_ERROR(
[illegal argument specified for --with-std-compliant-iostream. 
Only 'yes' and 'no' are accepted.])
fi
if test -z "$with_std_compliant_iostream" -o \
        "$with_std_compliant_iostream" = "yes" ; then
  AC_DEFINE(USE_STD_IOSTREAM, 1, [Define if the std compliant iostream library should be used (if present)])
fi

dnl Checks for programs.
AC_PROG_CXX
AC_PROG_CC
AC_PROG_CPP
AC_PROG_INSTALL
AC_PROG_RANLIB

dnl Checks for libraries.
AC_CHECK_LIB(z, inflate, [cv_libz=yes], [cv_libz=no])

if test "$cv_libz" != "yes"; then
  AC_MSG_ERROR([zlib is required to compile $PACKAGE.])
fi

dnl acconfig.h file added (with #undef for HAVE_ZERROR), which is required
dnl for symbols defined "manually" with AC_DEFINE
AC_CHECK_LIB(z, zError, AC_DEFINE(HAVE_ZERROR, 1, [Define if zlib has zError]) )

dnl Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS(unistd.h)
AC_CXX_HAVE_STL
AC_CXX_HAVE_STD
dnl Not necessary anymore: AC_CXX_HAVE_SSTREAM
AC_CXX_HAVE_STD_IOSTREAM

dnl Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST
AC_C_INLINE

dnl Checks for library functions.

AC_CONFIG_FILES([Makefile zipios++/Makefile src/Makefile])
AC_OUTPUT
