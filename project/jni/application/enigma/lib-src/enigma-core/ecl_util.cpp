/*
 * Copyright (C) 2002,2003 Ralf Westram
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

#include "ecl_util.hh"
#include "ecl_system.hh"

#include <cstdarg>
#include <cstdio>
#include <cassert>

using namespace std;
using namespace ecl;

string ecl::concat_paths (const string& path, const string& filename)
{
    // concatenate path and filename (or relative subpath)
    return
        path.substr(0, path.find_last_not_of (PathSeparator)+1) +
        PathSeparator +
        filename.substr(filename.find_first_not_of (PathSeparator));
}

bool ecl::split_path (const string& path, string* dir_part, string* filename_part)
{
    size_t lslash = path.find_last_of (PathSeparators);
    if (
#ifdef __MINGW32__
	    lslash == 2 && path[1] == ':' && (path[0] >= 'A' && path[0] <= 'Z')
#else
	    lslash == 0 
#endif
	    ) {
	return false; // we cannot split the root directory apart
    }
    
    if (lslash == path.length()-1) // trailing slash
        return split_path(path.substr(0, lslash), dir_part, filename_part);

    if (lslash == string::npos)
        return false;

    size_t lnslash = path.find_last_not_of (PathSeparators, lslash);

    if (dir_part) *dir_part           = path.substr(0, lnslash+1);
    if (filename_part) *filename_part = path.substr(lslash+1);

    return true;
}

std::string ecl::strf(const char *format, ...) {
    static size_t  buf_size = 512;
    static char   *buffer   = new char[buf_size];
    
    va_list argPtr;
    size_t length;

    while (true) {
        if (!buffer) {
            assert(buffer); // to stop when debugging
            return "<alloc problem>";
        }

        va_start(argPtr, format);
        length = vsnprintf(buffer, buf_size, format, argPtr);
        va_end(argPtr);
        
        if (length >= 0 && length < buf_size - 1) 
            // string fits into current buffer
            return std::string(buffer, length);

        // otherwise resize buffer :
        buf_size *= 2;
//        fprintf(stderr, "Reallocating vstrf-buffer to size=%u\n", buf_size);
        delete [] buffer;
        buffer = new char[buf_size];
    }
}

