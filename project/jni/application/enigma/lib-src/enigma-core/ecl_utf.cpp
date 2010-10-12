/*
 * This file is based on 
 *   http://www.unicode.org/Public/PROGRAMS/CVTUTF/ConvertUTF.c
 *
 * The original parts have the following copyright
 * ---------------------------------------------------------------------
 * Copyright 2001-2004 Unicode, Inc.
 * 
 * Disclaimer
 * 
 * This source code is provided as is by Unicode, Inc. No claims are
 * made as to fitness for any particular purpose. No warranties of any
 * kind are expressed or implied. The recipient agrees to determine
 * applicability of information provided. If this file has been
 * purchased on magnetic or optical media from Unicode, Inc., the
 * sole remedy for any claim will be exchange of defective media
 * within 90 days of receipt.
 * 
 * Limitations on Rights to Redistribute This Code
 * 
 * Unicode, Inc. hereby grants the right to freely use the information
 * supplied in this file in the creation of products supporting the
 * Unicode Standard, and to make copies of this file in any form
 * for internal or external distribution as long as this notice
 * remains attached.
 * ---------------------------------------------------------------------
 *
 * The modifications and additions have the following copyright
 *
 * Copyright (C) 2005, 2007 Ronald Lamprecht
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
 */

#include "ecl_utf.hh"

#define UNI_SUR_HIGH_START  (UTF32)0xD800
#define UNI_SUR_HIGH_END    (UTF32)0xDBFF
#define UNI_SUR_LOW_START   (UTF32)0xDC00
#define UNI_SUR_LOW_END     (UTF32)0xDFFF
#define false	    0
#define true	    1


namespace ecl
{
    static const int halfShift  = 10; /* used for shifting by 10 bits */
    
    static const UTF32 halfBase = 0x0010000UL;
    static const UTF32 halfMask = 0x3FFUL;


    /**
     * Index into the table below with the first byte of a UTF-8 sequence to
     * get the number of trailing bytes that are supposed to follow it.
     * Note that *legal* UTF-8 values can't have 4 or 5-bytes. The table is
     * left as-is for anyone who may want to do such conversion, which was
     * allowed in earlier algorithms.
     */
    static const char trailingBytesForUTF8[256] = {
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
        2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2, 3,3,3,3,3,3,3,3,4,4,4,4,5,5,5,5
    };
    
    /*
     * Once the bits are split out into bytes of UTF-8, this is a mask OR-ed
     * into the first byte, depending on how many bytes follow.  There are
     * as many entries in this table as there are UTF-8 sequence types.
     * (I.e., one byte sequence, two byte... etc.). Remember that sequencs
     * for *legal* UTF-8 will be 4 or fewer bytes total.
     */
    static const UTF8 firstByteMark[7] = { 0x00, 0x00, 0xC0, 0xE0, 0xF0, 0xF8, 0xFC };
    
    /* --------------------------------------------------------------------- */
    
    /* The interface converts a whole buffer to avoid function-call overhead.
     * Constants have been gathered. Loops & conditionals have been removed as
     * much as possible for efficiency, in favor of drop-through switches.
     * (See "Note A" at the bottom of the file for equivalent code.)
     * If your compiler supports it, the "isLegalUTF8" call can be turned
     * into an inline function.
     */
    
    /* --------------------------------------------------------------------- */
    
    ConversionResult ConvertUTF16toUTF8 (
            const UTF16** sourceStart, const UTF16* sourceEnd, 
            UTF8** targetStart, UTF8* targetEnd, ConversionFlags flags) {
        ConversionResult result = conversionOK;
        const UTF16* source = *sourceStart;
        UTF8* target = *targetStart;
        while (source < sourceEnd) {
            UTF32 ch;
            unsigned short bytesToWrite = 0;
            const UTF32 byteMask = 0xBF;
            const UTF32 byteMark = 0x80; 
            const UTF16* oldSource = source; /* In case we have to back up because of target overflow. */
            ch = *source++;
            /* If we have a surrogate pair, convert to UTF32 first. */
            if (ch >= UNI_SUR_HIGH_START && ch <= UNI_SUR_HIGH_END) {
                /* If the 16 bits following the high surrogate are in the source buffer... */
                if (source < sourceEnd) {
                    UTF32 ch2 = *source;
                    /* If it's a low surrogate, convert to UTF32. */
                    if (ch2 >= UNI_SUR_LOW_START && ch2 <= UNI_SUR_LOW_END) {
                        ch = ((ch - UNI_SUR_HIGH_START) << halfShift)
                            + (ch2 - UNI_SUR_LOW_START) + halfBase;
                        ++source;
                    } else if (flags == strictConversion) { /* it's an unpaired high surrogate */
                        --source; /* return to the illegal value itself */
                        result = sourceIllegal;
                        break;
                    }
                } else { /* We don't have the 16 bits following the high surrogate. */
                    --source; /* return to the high surrogate */
                    result = sourceExhausted;
                    break;
                }
            } else if (flags == strictConversion) {
                /* UTF-16 surrogate values are illegal in UTF-32 */
                if (ch >= UNI_SUR_LOW_START && ch <= UNI_SUR_LOW_END) {
                    --source; /* return to the illegal value itself */
                    result = sourceIllegal;
                    break;
                }
            }
            /* Figure out how many bytes the result will require */
            if (ch < (UTF32)0x80) {	     bytesToWrite = 1;
            } else if (ch < (UTF32)0x800) {     bytesToWrite = 2;
            } else if (ch < (UTF32)0x10000) {   bytesToWrite = 3;
            } else if (ch < (UTF32)0x110000) {  bytesToWrite = 4;
            } else {			    bytesToWrite = 3;
                                                ch = UNI_REPLACEMENT_CHAR;
            }
    
            target += bytesToWrite;
            if (target > targetEnd) {
                source = oldSource; /* Back up source pointer! */
                target -= bytesToWrite; result = targetExhausted; break;
            }
            switch (bytesToWrite) { /* note: everything falls through. */
                case 4: *--target = (UTF8)((ch | byteMark) & byteMask); ch >>= 6;
                case 3: *--target = (UTF8)((ch | byteMark) & byteMask); ch >>= 6;
                case 2: *--target = (UTF8)((ch | byteMark) & byteMask); ch >>= 6;
                case 1: *--target =  (UTF8)(ch | firstByteMark[bytesToWrite]);
            }
            target += bytesToWrite;
        }
        *sourceStart = source;
        *targetStart = target;
        return result;
    }
    
    void utf8CharSizes(const std::string &utf8String, std::vector<unsigned char> &sizes) 
    {
        int length = utf8String.size();
        
        for (int i = 0; i < length; ) {
            unsigned char c = utf8String[i];
            int charSize = trailingBytesForUTF8[c] + 1;
            sizes.push_back(charSize);
            i += charSize;
        }
    }
    
    int utf8NextCharSize(const std::string &utf8String) {
        unsigned char c = utf8String[0];
        int len = 1; // num of bytes that represents one real character 
        if ((c & 0xC0) == 0x80) {
            // a spurious follow up byte
            len = 0;
        } else if ((c & 0xE0) == 0xC0) {
            len = 2;
        } else if ((c & 0xF0) == 0xE0) {
            len = 3;
        } else if ((c & 0xF8) == 0xF0) {
            len = 4;
        }
        return len;
    }
} //namespace ecl
