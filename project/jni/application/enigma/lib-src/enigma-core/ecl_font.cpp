/*
 * Copyright (C) 2002,2003,2004 Daniel Heck
 * Copyright (C) 2006,2007      Ronald Lamprecht
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
#include "ecl_font.hh"
#include "ecl_geom.hh"
#include "ecl_utf.hh"
#include "ecl_video.hh"
#include <vector>
#include <string>
#include <memory>               // for auto_ptr
#include <stdio.h>
#include <cstdlib>
#include <ostream>
#include <iostream>

#include <config.h>

using namespace ecl;
using namespace std;

void Font::render(const GC &gc, int x, int y, std::string text,
        Font * altFont, int maxwidth) {
    render(gc, x, y, text.c_str());
}

std::string::size_type 
ecl::breakString (Font *font,
                  const std::string &str,
                  const std::string &breakChars, 
                  int targetWidth)
{
    if (font->get_width (str.c_str()) <= targetWidth)
        return str.size();     // the complete string fits into a line

    bool breakFound = false;
    std::string::size_type pos = 0;
    while (true) {
        std::string::size_type nextpos = str.find_first_of (breakChars, pos);

        if (nextpos == std::string::npos)
            // no more line breaks
            return breakFound ? pos : str.size();
        
        if (font->get_width (str.substr(0, nextpos+1).c_str()) > targetWidth)
            // now the string is too long
            return breakFound ? pos : nextpos + 1;
            
        pos = nextpos+1;
        breakFound = true;
    } 
}



//
// Bitmap fonts
//

namespace
{
    class InvalidFont {};

    class BitmapFont : public Font {
        vector<Rect>    char_rects;
        vector<int>     advance;
        Surface         *surface;
    public:
        BitmapFont(Surface *s, const char *descr);
        ~BitmapFont() { delete surface; }

        int get_lineskip() { return surface->height() + 3; }
        int get_width(char c);
        virtual int get_width(const char *str, Font * altFont = NULL);
        int get_height();

        virtual Surface *render(const char *str);
        virtual void render(const GC &gc, int x, int y, const char *str);
        virtual void render(const GC &gc, int x, int y, std::string text,
                Font * altFont = NULL, int maxwidth = -1);
    };
}

BitmapFont::BitmapFont(Surface *s, const char *descr)
    : char_rects(256), advance(256), surface(s)
{
    // Read and interpret the font description file.
    // expected line format:
    // charno xpos width xadvance

    FILE *fp=fopen(descr, "rt");
    if (!fp) return ; //throw InvalidFont();

    int c;
    int x=0, w=0, adv=0;
    while (fscanf(fp, "%d %d %d %d\n", &c, &x, &w, &adv) != EOF) {
        char_rects[c].x = x;
        char_rects[c].w = w;
        char_rects[c].y = 0;
        char_rects[c].h = s->height();
        advance[c] = adv;
        if (adv = 0)
            std::cout << "BitFont 0\n";
    }
}

int BitmapFont::get_width(char c) {
    return advance[int(c)];
}

int BitmapFont::get_width(const char *str, Font * altFont) {
    int width = 0;
    for (const char *p=str; *p; ++p) {
        // utf-8 char handling
        int len = utf8NextCharSize(p); // num of bytes that represents one real character 
        if (len == 0) {
            // a spurious follow up byte
            continue;
        }
                     
        if (len > 1 || advance[int(*p)] == 0) {
            if (altFont != NULL) {
                std::string utf8char(p, len);
                width += altFont->get_width(utf8char.c_str());
            }
            p += len - 1;
        } else {
            width += get_width(*p);
        }
    }
    return width;
}

int BitmapFont::get_height() {
    return surface->height();
}

Surface * BitmapFont::render(const char *str) {
    Surface *s = MakeSurface(get_width(str), get_height(), 16);
    s->set_color_key(0,0,0);
    render (GC(s), 0, 0, str);
    return s;
}

void BitmapFont::render(const GC &gc, int x, int y, const char *str) {
    render(gc, x, y, std::string(str));
}

void BitmapFont::render(const GC &gc, int x, int y, std::string text,
        Font * altFont, int maxwidth) {
    int width = 0;
    for (const char *p=text.c_str(); *p; ++p) {
        // utf-8 char handling
        int len = utf8NextCharSize(p); // num of bytes that represents one real character 
        if (len == 0) {
            // a spurious follow up byte
            continue;
        }
                     
        if (len > 1 || advance[int(*p)] == 0) {
            if (altFont != NULL) {
                std::string utf8char(p, len);
                int charWidth = altFont->get_width(utf8char.c_str());
                width += charWidth;
                if (maxwidth <= 0 || width < maxwidth) {
                   altFont->render(gc, x, y, utf8char.c_str());
                    x += altFont->get_width(utf8char.c_str());
                }
            }
            p += len - 1;
        } else {
            int charWidth = get_width(*p);
            width += charWidth;
            if (maxwidth <= 0 || width < maxwidth) {
                blit(gc, x, y, surface, char_rects[int(*p)]);
                x += charWidth;
            }
        }
    }
}

Font *
ecl::LoadBitmapFont(const char * imgname, const char * descrname)
{
    if (Surface *s = LoadImage(imgname))
        return new BitmapFont(s, descrname);
    return 0;
}



//
// TrueType fonts 
//
#include "SDL_ttf.h"

namespace
{
    class TrueTypeFont : public Font {
        // Variables
        TTF_Font *font;
        SDL_Color fgcolor;


        // Inhibit copying
        TrueTypeFont (const TrueTypeFont &);
        TrueTypeFont &operator= (const TrueTypeFont &);
    public:
        TrueTypeFont (TTF_Font *font_, int r, int g, int b);
        
        ~TrueTypeFont();
        
        // Font interface
        int get_lineskip();
        int get_height();
        int get_width (char c);
        virtual int get_width(const char *str, Font * altFont = NULL);

        Surface *render (const char *str);
        void     render (const GC &gc, int x, int y, const char *str);
    };
}

TrueTypeFont::TrueTypeFont (TTF_Font *font_, int r, int g, int b)
: font (font_)
{
    fgcolor.r = r;
    fgcolor.g = g;
    fgcolor.b = b;
}
        
TrueTypeFont::~TrueTypeFont()
{
    TTF_CloseFont (font);
}


int TrueTypeFont::get_lineskip() { 
    return TTF_FontLineSkip (font); 
}
int TrueTypeFont::get_height() { 
    return TTF_FontHeight(font); 
}
        
int TrueTypeFont::get_width(char c) {
    int minx, maxx, miny, maxy, advance;
    TTF_GlyphMetrics(font, c, &minx, &maxx, &miny, &maxy, &advance);
    return advance;
}


Surface *TrueTypeFont::render (const char *str) 
{
    SDL_Surface *s = 0;
    SDL_Color bgcolor = { 0, 0, 0, 0 };

    s = TTF_RenderUTF8_Shaded (font, str, fgcolor, bgcolor);
    if (s) {
        SDL_SetColorKey (s, SDL_SRCCOLORKEY,0);
        return Surface::make_surface (s);
    }
    return MakeSurface(0, get_height(), 16);
}

void TrueTypeFont::render (const GC &gc, int x, int y, const char *str) 
{
    std::auto_ptr<Surface> s (render (str));
    if (s.get())
        blit (gc, x, y, s.get());
}

int TrueTypeFont::get_width(const char *str, Font * altFont) 
{
    int w,h;
    TTF_SizeUTF8 (font, str, &w, &h);
    return w;
}

Font *ecl::LoadTTF (const char *filename, int ptsize, int r, int g, int b)
{
    if (!TTF_WasInit() && TTF_Init()==-1) {
        fprintf(stderr, "Couldn't initialize SDL_ttf: %s\n", SDL_GetError());
        exit(1);
    }
    TTF_Font *font = TTF_OpenFont (filename, ptsize);
    return (font) ? new TrueTypeFont (font, r, g, b) : 0;
}

