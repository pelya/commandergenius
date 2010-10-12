/*
 * Copyright (C) 2005, 2006 Ronald Lamprecht
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

#include "XMLtoUtf8.hh"
#include "main.hh"
#include <xercesc/util/XMLString.hpp>
#include <xercesc/util/TransService.hpp>

XERCES_CPP_NAMESPACE_USE

namespace enigma
{
    XMLtoUtf8::XMLtoUtf8(const XMLCh* const toTranscode) {
#if _XERCES_VERSION >= 30000
        XMLSize_t srcLength = XMLString::stringLen(toTranscode) + 1;
        // make safe assumptions on utf-8 size
        XMLSize_t maxDestLength = 3 * srcLength;
        XMLSize_t charsEaten;
        XMLSize_t destLength;
#else
        unsigned int srcLength = XMLString::stringLen(toTranscode) + 1;
        // make safe assumptions on utf-8 size
        unsigned int maxDestLength = 3 * srcLength;
        unsigned int charsEaten;
        unsigned int destLength;
#endif
        // make a buffer - size does not matter - the object is temporary 
        utf8String = new char[maxDestLength];
        // transcode to utf-8 -- there are no unrepresentable chars
        destLength = app.xmlUtf8Transcoder->transcodeTo(toTranscode, srcLength,
                (XMLByte *)utf8String, maxDestLength,
                charsEaten, XMLTranscoder::UnRep_RepChar);
        if (charsEaten < srcLength)
            // an assert - should never occur
            Log << "XMLtoUtf8: incomplete transcoding - only "<< charsEaten <<
                    " of " << srcLength << "characters were processed!" << std::endl;
    }
    
    XMLtoUtf8::~XMLtoUtf8() {
        delete [] utf8String;
    }

    const char* XMLtoUtf8::c_str() const {
        return utf8String;
    };
} //namespace enigma

