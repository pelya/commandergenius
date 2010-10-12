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
#ifndef SYSTEM_HH
#define SYSTEM_HH

#include <string>
#include <ctime>
#include <set>

namespace ecl
{
    extern const char *PathSeparator;
    extern const char *PathSeparators;
    extern const char *PathsSeparator;

    std::string ExpandPath (const std::string &path);

    bool        FileExists (const std::string &fname);
    std::time_t FileModTime (const std::string &fname);
        
    bool FolderExists (const std::string &fname);
    bool FolderCreate (const std::string &fname);

#ifdef __MINGW32__
    std::string ApplicationDataPath();
    void ToLowerCase(std::string &filename);
    std::set<std::string> UniqueFilenameSet(std::set<std::string> inSet);
#endif

/* -------------------- Locales -------------------- */

    std::string SysMessageLocaleName ();

    std::string GetLanguageCode (const std::string &localename);
}
#endif
