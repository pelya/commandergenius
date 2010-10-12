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

#include "main.hh"
#include "gui/widgets.hh"
#include "gui/TextField.hh"
#include "ecl_utf.hh"
#include "enigma.hh"
#include "sound.hh"
#include "video.hh"
#include "options.hh"
#include "nls.hh"
#include "ecl.hh"
#include <cassert>
#include <algorithm>
#include <iostream>

using namespace enigma::gui;
using namespace ecl;
using namespace std;

#define SCREEN ecl::Screen::get_instance()

ecl::Font *TextField::menufont = 0;

TextField::TextField(const std::string &t, ActionListener *al) : cursorTime(0),
        showCursor(true), isLastActionReturn (false), invalidChars(""),
        isLimitedToValidChars(false), maxChars(-1) {
    if (menufont == 0) {
        menufont = enigma::GetFont("menufont");
    }
    set_listener(al);
    textPreCursor = t;
    ecl::utf8CharSizes(textPreCursor, charSizesPreCursor);
    textPostCursor= "";
}

void TextField::set_text(const std::string &t) {
    textPreCursor = t;
    charSizesPreCursor.clear();
    ecl::utf8CharSizes(textPreCursor, charSizesPreCursor);
    textPostCursor= "";
    charSizesPostCursor.clear();
}

std::string TextField::getText() {
    string total = textPreCursor;
    total += textPostCursor;
    return total;
}

void TextField::setInvalidChars(std::string forbiddenChars) {
    invalidChars = forbiddenChars;
}

void TextField::setMaxChars(int max) {
    maxChars = max;
}
bool TextField::wasLastActionReturn() {
    return isLastActionReturn;
}

void TextField::tick (double dtime) {
    cursorTime += dtime;
    if (cursorTime > 0.5) {
        cursorTime = 0.0;
        showCursor = !showCursor;
        invalidate();
    }
}

void TextField::draw(ecl::GC &gc, const ecl::Rect &r) {
    Button::draw(gc,r);
    Font   *f    = menufont;
    int     h    = f->get_height();
    int     w_pre    = f->get_width(textPreCursor.c_str());
    int     w_post    = f->get_width(textPostCursor.c_str());
    int     w_cursor = m_activep ? 1 : 0;
    int     x    = get_x() + (get_w()- w_pre - w_post - w_cursor )/2;
    int     y    = get_y() + (get_h()-h)/2;

    // cursor always visible
    if (x + w_pre < get_x() + 5) 
        // cursor would be left of textfield - shift centered text right
        x = get_x() + 5 - w_pre;
    
    if (x + w_pre > get_x() + get_w() - 5)
        // cursor would be right of textfiled - shift centered text left
        x = get_x() + get_w() - 5 - w_pre;
    
    f->render (gc, x, y, textPreCursor.c_str());
    
    x += w_pre; 
    if (m_activep) {
        if (showCursor) {
            set_color(gc, 200,200,200);
            vline(gc, x, y, h);
        }
        x += w_cursor;
    }
    
    f->render (gc, x, y, textPostCursor.c_str());
   
}

bool TextField::on_event(const SDL_Event &e) {
    bool handeled = false;
    bool modified = false;
    
    switch (e.type) {
        case SDL_MOUSEBUTTONDOWN:
            // set cursor
            break;
        case SDL_KEYDOWN:
            switch (e.key.keysym.sym) {
                case SDLK_RETURN:
                    handeled = true;
                    isLastActionReturn = true;
                    invoke_listener();
                    break;                    
                case SDLK_RIGHT:
                    if(textPostCursor.size() > 0) {
                        int size = charSizesPostCursor.back();
                        charSizesPostCursor.pop_back();
                        charSizesPreCursor.push_back(size);
                        textPreCursor.append(textPostCursor, 0, size);
                        textPostCursor.erase(0, size); 
                    }
                    invalidate();
                    handeled = true;
                    break;
                case SDLK_LEFT:
                    if(textPreCursor.size() > 0) {
                        int size = charSizesPreCursor.back();
                        charSizesPreCursor.pop_back();
                        charSizesPostCursor.push_back(size);
                        textPostCursor.insert(0, textPreCursor.substr(textPreCursor.size() - size));
                        textPreCursor.erase(textPreCursor.size() - size); 
                    }
                    invalidate();
                    handeled = true;
                    break;
                case SDLK_INSERT:
                    handeled = true;
                    break;
                case SDLK_HOME:
                    if(textPreCursor.size() > 0) {
                        int i;
                        int preChars = charSizesPreCursor.size();
                        for (i = 0; i < preChars; i++) {
                            int size = charSizesPreCursor.back();
                            charSizesPreCursor.pop_back();
                            charSizesPostCursor.push_back(size);
                        }
                        textPostCursor.insert(0, textPreCursor);
                        textPreCursor.clear(); 
                    }
                    invalidate();
                    handeled = true;
                    break;
                case SDLK_END:
                    if(textPostCursor.size() > 0) {
                        int size;
                        int i;
                        int postChars = charSizesPostCursor.size();
                        for (i = 0; i < postChars; i++) {
                            size = charSizesPostCursor.back();
                            charSizesPostCursor.pop_back();
                            charSizesPreCursor.push_back(size);
                        }
                        textPreCursor.append(textPostCursor);
                        textPostCursor.clear(); 
                    }
                    invalidate();
                    handeled = true;
                    break;
                case SDLK_DELETE:
                    if(textPostCursor.size() > 0) {
                        int size = charSizesPostCursor.back();
                        textPostCursor.erase(0, size); 
                        charSizesPostCursor.pop_back();
                    }
                    invalidate();
                    handeled = true;
                    modified = true;
                    break;
                case SDLK_BACKSPACE:
                    if(textPreCursor.size() > 0) {
                       int size = charSizesPreCursor.back();
                        textPreCursor.erase(textPreCursor.size() - size); 
                        charSizesPreCursor.pop_back();
                    }
                    invalidate();
                    handeled = true;
                    modified = true;
                    break;
                case SDLK_ESCAPE:
                case SDLK_DOWN:  
                case SDLK_UP:
                    // menu active widget movements
                    break;
                default:
                    // get char
                    if (e.key.keysym.unicode != 0 ) {
                        UTF16 realChar;
                        if (e.key.keysym.unicode >= 0x20 && 
                                (e.key.keysym.unicode < 0x80 ||   // key pad
                                e.key.keysym.sym < 0x80)) {       // windows umlaute
                            // the unicode is correct in these cases
                            realChar = e.key.keysym.unicode;
                        }
                        else if (e.key.keysym.unicode >= 0x80 &&
                                e.key.keysym.sym < 0x100) {
                            // Linux: bad unicode but sym is ISO-8859-1
                            
                            // incomplete workaround - runs only for some lower
                            // case umlauts
                            // we would need to handle shift key in language
                            // dependent manner -- or fix SDL Linux
                            realChar = e.key.keysym.sym;
                        }
                        else {
                            // chars like ctrl-a - ctrl-z
                            sound::EmitSoundEvent ("menustop");
                            break;
                        }
                        if ((maxChars >= 0 && (charSizesPreCursor.size() + charSizesPostCursor.size()) >= maxChars) ||
                                realChar < 0x100 && invalidChars.find((char)realChar) != std::string::npos) {
                            // string too long or invalid char
                            sound::EmitSoundEvent ("menustop");
                            break;
                        }
                        unsigned char utf8Char[4];
                        UTF16 const * utf16Ptr = (UTF16 *)&realChar;
                        UTF8 * utf8Ptr = utf8Char;
                        ConversionResult result;
                        result = ConvertUTF16toUTF8 (&utf16Ptr, utf16Ptr + 1,
                                &utf8Ptr, utf8Char + 4, strictConversion);
                        *utf8Ptr = 0;
                        textPreCursor += (const char *)utf8Char;
                        charSizesPreCursor.push_back(utf8Ptr - utf8Char);

                        invalidate();
                        handeled = true;
                        modified = true;
                        break;
                    }
                    if (e.key.keysym.sym < 300 || e.key.keysym.sym > 314 ){
                        // chars like PageUp, F1 but not key state modifier
                        // like shift, alt,...
                        sound::EmitSoundEvent ("menustop");
                    }
                    break;
            }
            break;
        default:
            break;
    }
    if (modified) {
        isLastActionReturn = false;
        invoke_listener();
    }
    return handeled;
}

