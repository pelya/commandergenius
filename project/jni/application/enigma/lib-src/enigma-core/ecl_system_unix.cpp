/*
 * Copyright (C) 2002,2003,2004 Daniel Heck
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */
#include "ecl_system.hh"
#include "ecl_util.hh"

#include <cstdlib>
#include <iostream>
#include <sys/types.h>
#include <sys/stat.h>
#include <locale.h>
#include <unistd.h>
#include <cassert>

#include <config.h>

#ifdef __MINGW32__
#include <windows.h>
#endif

using namespace ecl;
using std::string;

#ifdef __MINGW32__
const char *ecl::PathSeparator = "\\";      // for path assembly
const char *ecl::PathsSeparator = ";";      // for listing paths in a string
#else
const char *ecl::PathSeparator = "/";       // for path assembly
const char *ecl::PathsSeparator = ":";      // for listing paths in a string
#endif
const char *ecl::PathSeparators = "/\\";    // for path splits

string ecl::ExpandPath (const string &pth)
{
    string path = pth;
    string::size_type p=path.find("~");
    if (p != string::npos) {
        string home;
        if (char *h = getenv("HOME"))
            home = h;
        path.replace(p, 1, home);
    }
    return path;
}

bool ecl::FileExists (const std::string &fname)
{
    struct stat s;
    return (stat(fname.c_str(), &s)==0 && S_ISREG(s.st_mode));
}

time_t ecl::FileModTime (const std::string &fname)
{
    struct stat s;
    if (stat(fname.c_str(), &s) == 0) {
        return s.st_mtime;
    }

    return 0;                   // beginning of time
}

bool ecl::FolderExists (const std::string &fname)
{
    struct stat s;
    return (stat(fname.c_str(), &s)==0 && S_ISDIR(s.st_mode));
}

bool ecl::FolderCreate (const std::string &fname)
{
    string parent_folder;
    string sub_folder;
    bool   ok = true;

    assert(!FolderExists(fname));

    if (split_path(fname, &parent_folder, &sub_folder)) {
        if (!FolderExists(parent_folder)) {
            ok = FolderCreate(parent_folder);
        }
    }

    if (ok) {
#ifdef __MINGW32__
        ok = mkdir(fname.c_str()) == 0;
#else
        ok = mkdir(fname.c_str(), 0777) == 0;
#endif
    }
    return ok;
}

#ifdef __MINGW32__
// should be ecl_system_windows.cc ?
std::string ecl::ApplicationDataPath()
{
    typedef HRESULT (WINAPI *SHGETFOLDERPATH)( HWND, int, HANDLE, DWORD, LPTSTR );
#   define CSIDL_FLAG_CREATE 0x8000
#   define CSIDL_APPDATA 0x1A
#   define SHGFP_TYPE_CURRENT 0

    HINSTANCE shfolder_dll;
    SHGETFOLDERPATH SHGetFolderPath ;

    static bool didRun = false;
    static std::string result = "";  // remember exec results 
    
    if (!didRun) {
        didRun = true;
        /* load the shfolder.dll to retreive SHGetFolderPath */
        if( ( shfolder_dll = LoadLibrary("shfolder.dll") ) != NULL )
        {
            SHGetFolderPath = (SHGETFOLDERPATH)GetProcAddress( shfolder_dll, "SHGetFolderPathA");
            if ( SHGetFolderPath != NULL )
            {
                TCHAR szPath[MAX_PATH] = "";
    
                /* get the "Application Data" folder for the current user */
                if( S_OK == SHGetFolderPath( NULL, CSIDL_APPDATA | CSIDL_FLAG_CREATE,
                                             NULL, SHGFP_TYPE_CURRENT, szPath) )
                {
                    result = szPath;
                }
            }
            FreeLibrary( shfolder_dll);
        }
    }
    return result;
}

void ecl::ToLowerCase(std::string &filename) {
    for (int i = 0; i < filename.length(); i++) {
	char c = filename[i];
	if (c >= 'A' && c <= 'Z')
	    filename.replace(i, 1, 1, c - 'A' + 'a');
    }
}

std::set<std::string> ecl::UniqueFilenameSet(std::set<std::string> inSet) {
    std::set<std::string>::iterator it = inSet.begin();
    std::set<std::string> outSet;
    std::set<std::string> lowerSet;
    for (; it != inSet.end(); it++) {
	std::string name = *it;
	ecl::ToLowerCase(name);
	if (lowerSet.find(name) == lowerSet.end()) {
	    outSet.insert(*it);
	    lowerSet.insert(name);
	}
    }
    return outSet;
}

#endif


std::string ecl::GetLanguageCode (const std::string &ln)
{
    if (ln == "C" || ln == "POSIX" || ln.size() < 2)
        return "en";

    return ln.substr (0, 2);
}

