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

#include <cstring>
#include "Utf8ToXML.hh"
#include "main.hh"
#include <xercesc/util/XMLString.hpp>
#include <xercesc/util/TransService.hpp>

XERCES_CPP_NAMESPACE_USE

namespace enigma
{
    Utf8ToXML::Utf8ToXML(const char * const toTranscode) {
        init(toTranscode);
    }
    
    Utf8ToXML::Utf8ToXML(const std::string * const toTranscode) {
        init(toTranscode->c_str());
    }
    
    Utf8ToXML::Utf8ToXML(const std::string toTranscode) {
        init(toTranscode.c_str());
    }

    void Utf8ToXML::init(const char * const toTranscode) {
#if _XERCES_VERSION >= 30000
        XMLSize_t srcLength = std::strlen(toTranscode) + 1;
        // make safe assumptions on utf-16 size
        XMLSize_t maxDestLength = srcLength;
        XMLSize_t charsEaten;
        XMLSize_t destLength;
#else
        unsigned int srcLength = std::strlen(toTranscode) + 1;
        // make safe assumptions on utf-16 size
        unsigned int maxDestLength = srcLength;
        unsigned int charsEaten;
        unsigned int destLength;
#endif
        unsigned char *charSizes = new unsigned char[maxDestLength]; // just junk
        // make a buffer - size does not matter - the object is temporary 
        xmlString = new XMLCh[maxDestLength];
        // transcode to utf-8 -- there are no unrepresentable chars
        destLength = app.xmlUtf8Transcoder->transcodeFrom((XMLByte *)toTranscode, 
                srcLength,
                xmlString, maxDestLength,
                charsEaten, charSizes);
        delete[] charSizes;
        if (charsEaten < srcLength)
            // an assert - should never occur
            Log << "Utf8toXML: incomplete transcoding - only "<< charsEaten <<
                    " of " << srcLength << "bytes were processed!" << std::endl;
    }
    
    
    Utf8ToXML::~Utf8ToXML() {
        delete [] xmlString;
    }

    const XMLCh * Utf8ToXML::x_str() const {
        return xmlString;
    };
} //namespace enigma

