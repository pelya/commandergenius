#ifndef NLS_HH_INCLUDED
#define NLS_HH_INCLUDED
#include "config.h"

#if defined(ENABLE_NLS)
#  include <libintl.h>
#  define _(String) gettext(String)
#  define gettext_noop(String) (String)
#  define N_(String) gettext_noop(String)
#else // !defined(ENABLE_NLS) 
#  define _(String) (String)
#  define N_(String) (String)
#endif

#include <string>

namespace nls
{
    void SetMessageLocale (const std::string &language);
}

#endif
