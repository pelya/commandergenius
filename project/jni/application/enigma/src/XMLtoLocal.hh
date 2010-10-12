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

#ifndef ENIGMA_XMLTOLOCAL_HH
#define ENIGMA_XMLTOLOCAL_HH

#include <iostream>
#include <xercesc/util/XMLString.hpp>

namespace enigma
{
    /**
     * Transcoding utility for XMLCh strings to local code page strings.
     * Provides a simple interface for the memory management paradigm shift
     * even though not terribly efficient. Make sure all XMLtoLocal objects
     * exist only temporarily, f.e.
     * <code>new String(XMLtoLocal(XMLCh *toTranscode).c_str())</code> or
     * <code>Log << XMLtoLocal(XMLCh *toTranscode)</code>
     * Xerces should be initialized before using this class and all objects
     * should be deleted before terminating.
     */
    class XMLtoLocal {
    public :
        /**
         * Makes a transcoding to the local code page.
         *
         * @param toTranscode XML managed string
         */
        XMLtoLocal(const XMLCh* const toTranscode);
        ~XMLtoLocal();
    
        /**
         * Returns the string coded in the local page.  It remains owner of
         * the string.
         */
        const char* c_str() const;
    
    private :
        /**
         * A copy coded in the local code page.
         * The string is managed by XMLString - we are the owner.
         */
        char*   localString;
    };
    
    /**
     * Enables efficient stream output of XMLCh * strings. Use it as
     * <code>Log << XML2Local(XMLCh *toTranscode)</code>. All resources
     * are managed and released.
     */
    inline std::ostream& operator<<(std::ostream& target, const XMLtoLocal& toDump) {
        target << toDump.c_str();
        return target;
    }
} //namespace enigma
#endif

