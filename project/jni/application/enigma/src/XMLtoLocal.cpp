/*
 * Copyright (C) 2005 Ronald Lamprecht
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

#include "XMLtoLocal.hh"

XERCES_CPP_NAMESPACE_USE

namespace enigma
{
    XMLtoLocal::XMLtoLocal(const XMLCh* const toTranscode) {
        // Use XML for transcoding -- the returned string is owned by us
        // but managed by XMLString!
        localString = XMLString::transcode(toTranscode);
    }

    XMLtoLocal::~XMLtoLocal() {
        XMLString::release(&localString);
    }

    const char* XMLtoLocal::c_str() const {
        return localString;
    }
    
} //namespace enigma
