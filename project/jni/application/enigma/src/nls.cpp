#include "nls.hh"
#include "main.hh"
#include "ecl_system.hh"

#include <iostream>
#include <string>
#include <cstdlib>

#include <config.h>

using namespace std;

#if !defined (HAVE_SETENV) && defined (HAVE_PUTENV)
static char lang_env[256];
#endif

static void my_setenv (const std::string &var, const std::string &val)
{
#if defined (HAVE_SETENV)
    setenv(var.c_str(), val.c_str(), 1);
#elif defined (HAVE_PUTENV)
    snprintf (lang_env, sizeof(lang_env), "%s=%s", var.c_str(), val.c_str());
    putenv(lang_env);
#endif
}

void nls::SetMessageLocale (const std::string &language)
{
    if (language != "")
        my_setenv ("LANG", language);

#if defined(ENABLE_NLS) && defined(HAVE_LC_MESSAGES)
    // Hack to fool libintl into changing the message locale more than
    // once
    setlocale (LC_MESSAGES, "C");
    setlocale (LC_MESSAGES, ""); //language.c_str());
#endif

    std::string li = ecl::SysMessageLocaleName();
    enigma::Log << "locale name: " << li << endl;
    enigma::Log << "language code: " << ecl::GetLanguageCode (li) << endl;
}

