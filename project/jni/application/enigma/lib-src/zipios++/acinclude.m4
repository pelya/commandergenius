dnl @synopsis AC_CXX_HAVE_STL
dnl
dnl If the compiler supports the Standard Template Library, define HAVE_STL.
dnl
dnl @version $Id: acinclude.m4,v 1.1 2003/06/10 22:07:18 dheck Exp $
dnl @author Luc Maisonobe
dnl
AC_DEFUN(AC_CXX_HAVE_STL,
[AC_CACHE_CHECK(whether the compiler supports Standard Template Library,
ac_cv_cxx_have_stl,
[AC_REQUIRE([AC_CXX_NAMESPACES])
 AC_LANG_SAVE
 AC_LANG_CPLUSPLUS
 AC_TRY_COMPILE([#include <list>
#include <deque>
#ifdef HAVE_NAMESPACES
using namespace std;
#endif],[list<int> x; x.push_back(5);
list<int>::iterator iter = x.begin(); if (iter != x.end()) ++iter; return 0;],
 ac_cv_cxx_have_stl=yes, ac_cv_cxx_have_stl=no)
 AC_LANG_RESTORE
])
if test "$ac_cv_cxx_have_stl" = yes; then
  AC_DEFINE(HAVE_STL,,[define if the compiler supports Standard Template Library])
fi
])
dnl @synopsis AC_CXX_HAVE_STD
dnl
dnl If the compiler supports ISO C++ standard library (i.e., can include the
dnl files iostream, map, iomanip and cmath}), define HAVE_STD.
dnl
dnl @version $Id: acinclude.m4,v 1.1 2003/06/10 22:07:18 dheck Exp $
dnl @author Luc Maisonobe
dnl
AC_DEFUN(AC_CXX_HAVE_STD,
[AC_CACHE_CHECK(whether the compiler supports ISO C++ standard library,
ac_cv_cxx_have_std,
[AC_REQUIRE([AC_CXX_NAMESPACES])
 AC_LANG_SAVE
 AC_LANG_CPLUSPLUS
 AC_TRY_COMPILE([#include <iostream>
#include <map>
#include <iomanip>
#include <cmath>
#ifdef HAVE_NAMESPACES
using namespace std;
#endif],[return 0;],
 ac_cv_cxx_have_std=yes, ac_cv_cxx_have_std=no)
 AC_LANG_RESTORE
])
if test "$ac_cv_cxx_have_std" = yes; then
  AC_DEFINE(HAVE_STD,,[define if the compiler supports ISO C++ standard library])
fi
])
dnl @synopsis AC_CXX_NAMESPACES
dnl
dnl If the compiler can prevent names clashes using namespaces, define
dnl HAVE_NAMESPACES.
dnl
dnl @version $Id: acinclude.m4,v 1.1 2003/06/10 22:07:18 dheck Exp $
dnl @author Luc Maisonobe
dnl
AC_DEFUN(AC_CXX_NAMESPACES,
[AC_CACHE_CHECK(whether the compiler implements namespaces,
ac_cv_cxx_namespaces,
[AC_LANG_SAVE
 AC_LANG_CPLUSPLUS
 AC_TRY_COMPILE([namespace Outer { namespace Inner { int i = 0; }}],
                [using namespace Outer::Inner; return i;],
 ac_cv_cxx_namespaces=yes, ac_cv_cxx_namespaces=no)
 AC_LANG_RESTORE
])
if test "$ac_cv_cxx_namespaces" = yes; then
  AC_DEFINE(HAVE_NAMESPACES,,[define if the compiler implements namespaces])
fi
])
dnl @synopsis AC_CXX_HAVE_SSTREAM
dnl
dnl If sstream (part of Standard C++ Library) exists
dnl define HAVE_SSTREAM.
dnl
dnl @version ac_cxx_have_std.m4 Tue Mar 28 18:20:26 CEST 2000
dnl @author Thomas Sondergaard thomass@deltadata.dk
dnl
AC_DEFUN(AC_CXX_HAVE_SSTREAM,
[AC_CACHE_CHECK(for sstream,
ac_cv_cxx_have_sstream,
[AC_REQUIRE([AC_CXX_NAMESPACES])
 AC_LANG_SAVE
 AC_LANG_CPLUSPLUS
 AC_TRY_COMPILE([#include <sstream>
#ifdef HAVE_NAMESPACES
using namespace std;
#endif],[return 0;],
 ac_cv_cxx_have_sstream=yes, ac_cv_cxx_have_sstream=no)
 AC_LANG_RESTORE
])
if test "$ac_cv_cxx_have_sstream" = yes; then
  AC_DEFINE(HAVE_SSTREAM,1,[define if the compiler supports sstream])
fi
])
dnl @synopsis AC_CXX_HAVE_STD_IOSTREAM
dnl
dnl If std iostream (part of Standard C++ Library) exists
dnl define HAVE_STD_IOSTREAM.
dnl
dnl @version ac_cxx_have_std.m4 Tue Mar 28 18:20:26 CEST 2000
dnl @author Thomas Sondergaard thomass@deltadata.dk
dnl
AC_DEFUN(AC_CXX_HAVE_STD_IOSTREAM,
[AC_CACHE_CHECK(for std iostream,
ac_cv_cxx_have_std_iostream,
[AC_REQUIRE([AC_CXX_NAMESPACES])
 AC_LANG_SAVE
 AC_LANG_CPLUSPLUS
 AC_TRY_COMPILE([
#include <sstream>
#include <streambuf>
#include <ios>
#ifdef HAVE_NAMESPACES
using namespace std;
#endif
],[return 0;],
 ac_cv_cxx_have_std_iostream=yes, ac_cv_cxx_have_std_iostream=no)
 AC_LANG_RESTORE
])
if test "$ac_cv_cxx_have_std_iostream" = yes; then
  AC_DEFINE(HAVE_STD_IOSTREAM,,[define if the compiler has std compliant iostream library])
fi
])
