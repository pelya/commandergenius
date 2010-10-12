/*
 * Copyright (C) 2006 Ronald Lamprecht
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

#include "gui/MonospacedLabel.hh"
#include "ecl.hh"
#include "nls.hh"

using namespace ecl;
using namespace std;

namespace enigma { namespace gui {
    
    MonospacedLabel::MonospacedLabel (const std::string &text, char widthSample,
                                      std::string monospaceCharacters, HAlignment halign,  VAlignment valign) :
            Label (text, halign, valign), sampleChar (widthSample),
            monoChars (monospaceCharacters) {
    }
    
    void MonospacedLabel::draw (ecl::GC &gc, const ecl::Rect &) {
        Font *f = m_font;
        int w, h;
        naturalsize (w, h);
        
        int x = get_x(), y=get_y();
        switch (m_halign) {
        case HALIGN_LEFT: break;
        case HALIGN_RIGHT: x += get_w() - w; break;
        case HALIGN_CENTER: x += (get_w()-w)/2; break;
        }
        switch (m_valign) {
        case VALIGN_TOP: break;
        case VALIGN_BOTTOM: y += get_h() - h; break;
        case VALIGN_CENTER: y += (get_h()-h)/2; break;
        }
        // translate if not an empty string
        const char * translation = _(m_text.c_str());
        int len = strlen(translation);
        int monoWidth = m_font->get_width(sampleChar);
        char c[] = " ";
        for (int i = 0; i<len; i++) {
            c[0] = translation[i];
            if (monoChars.empty() || monoChars.find_first_of(c[0]) != 
                    std::string::npos) {
                int charWidth = m_font->get_width(c[0]);
                // centere char into monoWodth
                f->render (gc, x + (monoWidth-charWidth)/2, y, c);
                x += monoWidth;
            } else {
                f->render (gc, x, y, c);
                x += m_font->get_width(c);
            }
        }
    }
    
    void MonospacedLabel::naturalsize (int &w, int &h) const {
        h = m_font->get_height();
        w = 0; 
        const char * translation = _(m_text.c_str());
        int len = strlen(translation);
        int monoWidth = m_font->get_width(sampleChar);
        for (int i = 0; i<len; i++) {
            if (monoChars.empty() || monoChars.find_first_of(translation[i]) != 
                    std::string::npos) {
                w += monoWidth;
            } else {
                w += m_font->get_width(translation[i]);
            }
        }
    }
}} // namespace enigma::lev
