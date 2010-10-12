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

#ifndef ENIGMA_XMLTOUTF8_HH
#define ENIGMA_XMLTOUTF8_HH

#include <iostream>
#include <xercesc/util/XMLString.hpp>

namespace enigma
{
     /**
     * Transcoding utility for XMLCh strings to utf-8 strings.
     * Provides a simple interface for the memory management paradigm shift
     * even though not terribly efficient. Make sure all XMLtoUtf8 objects
     * exist only temporarily, f.e. 
     * <code>new string(XMLtoUtf8(XMLCh *toTranscode).c_str())</code>
     * Xerces should be initialized before using this class and all objects
     * should be deleted before terminating.
     */
    class XMLtoUtf8 {
    public :
        /**
         * Makes a transcoding to utf-8
         *
         * @param toTranscode XML managed string
         */
        XMLtoUtf8(const XMLCh* const toTranscode);
        ~XMLtoUtf8();

        /**
         * Returns the string coded in utf-8.  It remains owner of
         * the string.
         */
        const char * c_str() const;

    private :
        /**
         * A copy coded in utf-8. We are the owner.
         */
        char * utf8String;
    };
} //namespace enigma
#endif

