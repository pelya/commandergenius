/*
 * Copyright (C) 2002,2003,2004,2005 Daniel Heck
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
#ifndef ERRORS_HH
#define ERRORS_HH

#include "ecl_error.hh"
#include <string>

namespace enigma { 
/* -------------------- Exceptions -------------------- */

    class XLevelLoading : public ecl::XGeneric {
    public:
        XLevelLoading (const std::string &msg)
        : XGeneric (msg)
        {}
    };

    class XLevelPackInit : public ecl::XGeneric {
    public:
        XLevelPackInit (const std::string &msg)
        : XGeneric (msg)
        {}
    };
    
    class XLevelRuntime : public ecl::XGeneric {
    public:
        XLevelRuntime (const std::string &msg)
        : XGeneric (msg)
        {}
    };

    class XFrontend : public ecl::XGeneric {
    public:
        XFrontend (const std::string &msg)
        : XGeneric (msg)
        {}
    };
} // namespace enigma
#endif
