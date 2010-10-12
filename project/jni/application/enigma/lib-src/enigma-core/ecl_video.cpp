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
#include "ecl_video.hh"
#include "ecl_error.hh"
#include "ecl_sdl.hh"

#include "SDL_image.h"
#include "SDL_syswm.h"
#include "SDL_gfxPrimitives.h"
#include "SDL_rotozoom.h"
#include "IMG_SavePNG.h"

#include <cassert>
#include <memory>
#include <cstdio>

using namespace std;
using namespace ecl;


/* -------------------- Graphics primitives -------------------- */

void ecl::frame (const GC &gc, int x, int y, int w, int h) {
    hline (gc, x, y, w);
    hline (gc, x, y+h-1, w);
    vline (gc, x, y, h);
    vline (gc, x+w-1, y, h);
}

void ecl::line (const GC &gc, int x1, int y1, int x2, int y2) {
    gc.drawable->line (gc, x1, y1, x2, y2);
}


/* -------------------- Clipping helper routines -------------------- */
namespace
{
    inline bool NOCLIP(const GS &gs) {
        return gs.flags & GS_NOCLIP;
    }

    bool clip_hline (const GS &gs, int &x, int &y, int &w) {
        const Rect &cliprect = gs.cliprect;

        if (y < cliprect.y || y >= cliprect.y + cliprect.h)
            return false;

        int d = x - cliprect.x;
        if (d < 0) {
            w += d;
            x = cliprect.x;
        }
        d = (cliprect.x+cliprect.w) - (x+w);
        if (d < 0)
            w += d;
        return w > 0;
    }

    bool clip_vline (const GS &gs, int &x, int &y, int &h) {
        const Rect &cliprect = gs.cliprect;

        if (x < cliprect.x || x >= cliprect.x + cliprect.w)
            return false;

        int d = y - cliprect.y;
        if (d < 0) {
            h += d;
            y = cliprect.y;
        }
        d = cliprect.y+cliprect.h - (y+h);
        if (d < 0)
            h += d;
        return h > 0;
    }

    bool clip_blit (Rect cliprect, int &x, int &y, Rect &r) {
        cliprect.x += r.x-x;
        cliprect.y += r.y-y;
        cliprect.intersect (r);

        if (cliprect.w > 0 && cliprect.h > 0) {
            x += cliprect.x-r.x;
            y += cliprect.y-r.y;
            r = cliprect;
            return true;
        }
        return false;
    }

    bool clip_rect (const GS &gs, Rect &r) {
        r.intersect (gs.cliprect);
        return r.w > 0 && r.h > 0;
    }

    inline bool clip_pixel (const GS &gs, int x, int y) {
        return gs.cliprect.contains(x,y);
    }
}


/* -------------------- Drawable implementation -------------------- */

/* `Xlib.h' also defines a type named `Drawable' so we have to specify
   the namespace explicitly and cannot simply use a using-declaration. */

void ecl::Drawable::set_pixels (const GS &gs, int n, const int* x, const int* y)
{
    const int *xp = x, *yp = y;
    for (int i=n; i; --i)
	set_pixel(gs, *xp++, *yp++);
}

void ecl::Drawable::hline (const GS &gs, int x, int y, int w)
{
    for (int i=w; i; --i)
	set_pixel (gs, x++, y);
}

void ecl::Drawable::vline (const GS &gs, int x, int y, int h)
{
    for (int i=h; i; --i)
	set_pixel (gs, x, y++);
}

void ecl::Drawable::box (const GS &gs, int x, int y, int w, int h)
{
    for (int i=h; i; --i)
	hline (gs, x, y--, w);
}

void ecl::Drawable::line(const GS &, int /*x1*/, int /*y1*/, int /*x2*/, int /*y2*/)
{
}


namespace
{

/* ---------- Generic Surface implementation ---------- */
    template <class PIXELT>
    class TSurface : virtual public Surface {
    public:
        TSurface (SDL_Surface *s=0) : Surface(s) {}

        PIXELT *pixel_pointer(int x, int y) {
            return static_cast<PIXELT*>(Surface::pixel_pointer(x,y));
        }

        /* ---------- Drawable interface ---------- */

        PackedColor get_pixel (int x, int y) {
            return *pixel_pointer(x,y);
        }

        void set_pixel (const GS &gs, int x, int y) {
            if (NOCLIP(gs) || clip_pixel(gs, x, y)) {
                *pixel_pointer(x, y) = gs.pcolor;
            }
        }

        void set_pixels(const GS &gs, int n, const int* xlist, const int* ylist, Uint32 color) {
            const int *xp = xlist, *yp = ylist;
            if (NOCLIP(gs)) {
                for (int i=n; i > 0; --i) {
                    int x = *xp++, y = *yp++;
                    *pixel_pointer(x,y) = gs.pcolor;
                }
            } else {
                for (int i=n; i > 0; --i) {
                    int x = *xp++, y = *yp++;
                    if (clip_pixel (gs, x, y))
                        *pixel_pointer(x,y) = gs.pcolor;
                }
            }
        }

        void hline (const GS &gs, int x, int y, int w) {
            if (NOCLIP(gs) || clip_hline(gs, x, y, w)) {
                PIXELT* dst = pixel_pointer(x, y);
                for (; w > 0; --w)
                    *dst++ = gs.pcolor;
            }
        }

        void vline (const GS &gs, int x, int y, int h) {
            if (NOCLIP(gs) || clip_vline(gs, x, y, h)) {
                PIXELT* dst = pixel_pointer(x, y);
                int offset = pitch() / bypp();
                for (; h > 0; --h)
                {
                    *dst = gs.pcolor;
                    dst += offset;
                }
            }
        }

    };

    typedef TSurface<Uint8> Surface8;
    typedef TSurface<Uint16> Surface16;
    typedef TSurface<Uint32> Surface32;

/* ---------- Surface24 ---------- */

    class Surface24 : virtual public Surface {
    public:
        Surface24 (SDL_Surface *s=0) : Surface(s) {}

        void set_pixel (const GS &gs, int x, int y) {
            if (NOCLIP(gs) || clip_pixel(gs, x, y)) {
                Uint8* p = static_cast<Uint8*> (pixel_pointer(x, y));
                SDL_GetRGB(gs.pcolor, m_surface->format, p,p+1,p+2);
            }
        }

        Uint32 get_pixel (int x, int y) {
//             if (NOCLIP(gs) || clip_pixel(gs, x, y)) {
            Uint8* p = static_cast<Uint8*> (pixel_pointer(x, y));
            return SDL_MapRGB(m_surface->format, p[0],p[1],p[2]);
//             }
        }
    };
}


/* -------------------- Surface -------------------- */

Surface::Surface (SDL_Surface* sfc)
: m_surface(sfc)
{
}

Surface::~Surface() {
    SDL_FreeSurface(m_surface);
}

void Surface::lock() {
    if (SDL_MUSTLOCK (m_surface))
        SDL_LockSurface (m_surface);
}

void Surface::unlock() {
    if (SDL_MUSTLOCK (m_surface))
        SDL_UnlockSurface (m_surface);
}

PackedColor
Surface::map_color(int r, int g, int b)
{
    return SDL_MapRGB(m_surface->format, r, g, b);
}

PackedColor
Surface::map_color(int r, int g, int b, int a)
{
    return SDL_MapRGBA(m_surface->format, r, g, b, a);
}


void Surface::box (const GS &gs, int x, int y, int w, int h) {
    Rect r (x, y, w, h);
    if (NOCLIP(gs) || clip_rect (gs, r)) {
        SDL_Rect dr;
        sdl::copy_rect (dr, r);
        SDL_FillRect (m_surface, &dr, gs.pcolor);
    }
}

void Surface::line (const GS &gs, int x1, int y1, int x2, int y2) {
    SDL_Rect s;
    sdl::copy_rect(s, gs.cliprect);
    SDL_SetClipRect (m_surface, &s);

    Uint8 r, g, b, a;
    SDL_GetRGBA (gs.pcolor, m_surface->format, &r, &g, &b, &a);

    if (has_flags(gs.flags, GS_ANTIALIAS))
        aalineRGBA (m_surface, x1, y1, x2, y2, r, g, b, a);
    else
        lineRGBA (m_surface, x1, y1, x2, y2, r, g, b, a);

    SDL_SetClipRect (m_surface, 0);
}

void Surface::blit (const GS &gs, int x, int y, const Surface* s, const Rect &r_) {
    Rect r(r_);
    if (NOCLIP(gs) || clip_blit (gs.cliprect, x, y, r)) {
        SDL_Rect r1;
        SDL_Rect r2;
        sdl::copy_rect (r1, r);
        r2.x = x; r2.y = y;
        SDL_BlitSurface (s->m_surface, &r1, m_surface, &r2);
    }
}

void Surface::blit (const GS &gs, int x, int y, const Surface* src) {
    assert (src != 0);
    blit (gs, x, y, src, src->size());
}

void Surface::set_color_key (int r, int g, int b) {
    Uint32 color = map_color(r,g,b);
    SDL_SetColorKey(get_surface(), SDL_SRCCOLORKEY | SDL_RLEACCEL, color);
}

void Surface::set_alpha(int a) {
    SDL_SetAlpha(get_surface(), SDL_SRCALPHA, a);
}

Surface * Surface::zoom(int w, int h) 
{
    SDL_Surface *s_new;
    s_new = zoomSurface( get_surface(), (double)w/width(), (double)h/height(), true);
    return Surface::make_surface (s_new);
}

Surface *
Surface::make_surface (SDL_Surface *sdls)
{
    if (sdls == 0) return 0;
    switch (sdls->format->BitsPerPixel) {
    case 8:  return new Surface8 (sdls); break;
    case 16: return new Surface16(sdls); break;
    case 32: return new Surface32(sdls); break;
    case 24: return new Surface24(sdls); break;
    default:
        fprintf(stderr, "Invalid bit depth in surface.\n");
        return 0; // throw XVideo("Invalid bit depth in surface.");
    }
}


/* -------------------- Screen -------------------- */

/* `Xlib.h' also defines a type named `Screen' so we have to specify
   the namespace explicitly and cannot simply use a using-declaration. */

ecl::Screen *ecl::Screen::m_instance = 0;

ecl::Screen *
ecl::Screen::get_instance() {
    return m_instance;
}


ecl::Screen::Screen (Surface *s)
: m_surface(s), m_sdlsurface(s->get_surface()), update_all_p(false)
{
    assert (m_instance == 0);
    m_instance = this;
}

ecl::Screen::Screen (SDL_Surface *s)
: m_surface (Surface::make_surface(s)), m_sdlsurface(s), update_all_p(false)
{
    assert (m_instance == 0);
    m_instance = this;
}

ecl::Screen::~Screen() {
    m_instance = 0;
}

void ecl::Screen::update_all()
{
    //SDL_UpdateRect(get_surface(), 0, 0, 0, 0);
    update_all_p = true;
}

void ecl::Screen::update_rect(const Rect& r)
{
    if (m_dirtyrects.size() < 200)
        m_dirtyrects.push_back(r);
    else
        update_all();
}

void ecl::Screen::set_caption(const char* str)
{
    SDL_WM_SetCaption(str, 0);
}

void ecl::Screen::flush_updates()
{
    if (update_all_p) {
        SDL_UpdateRect(m_sdlsurface, 0, 0, 0, 0);	// update everything
        update_all_p=false;
    }
    else if (!m_dirtyrects.empty()) {
        m_dirtyrects.intersect (size());

        vector<SDL_Rect> rects(m_dirtyrects.size());

        RectList::iterator j=m_dirtyrects.begin();;
        for (unsigned i=0; i<rects.size(); ++i, ++j)
            sdl::copy_rect (rects[i], *j);
        SDL_UpdateRects (m_sdlsurface, rects.size(), &rects[0]);
    }
    m_dirtyrects.clear();
}

Rect ecl::Screen::size() const {
    return Rect(0, 0, width(), height());
}

int ecl::Screen::width() const {
    return m_sdlsurface->w;
}

int ecl::Screen::height() const {
    return m_sdlsurface->h;
}


/* -------------------- Functions -------------------- */

/* Convert the surface to the native surface format.  A pointer to a
   new surface is returned; the old one must be deleted by hand if it
   is no longer needed.  */
Surface*
ecl::DisplayFormat(Surface* s)
{
    if (SDL_Surface* s2 = SDL_DisplayFormat(s->get_surface()))
        return Surface::make_surface(s2);
    return 0;
}


ecl::Screen *
ecl::OpenScreen(int w, int h, int bipp)
{
    SDL_Surface* sfc = SDL_SetVideoMode(w, h, bipp, SDL_SWSURFACE);
    return new Screen(sfc);
}

Surface *
ecl::Duplicate(const Surface *s)
{
    if (s==0) return 0;
    SDL_Surface *sdls = s->get_surface();
    SDL_Surface *copy = SDL_ConvertSurface(sdls, sdls->format, sdls->flags);

    if (sdls->format->palette != 0)
        SDL_SetColors(copy, sdls->format->palette->colors, 0,
                      sdls->format->palette->ncolors);

    return Surface::make_surface(copy);
}

void ecl::SavePNG (const ecl::Surface *s, const std::string &filename)
{
    IMG_SavePNG(s->get_surface(), filename.c_str());
}


void ecl::TintRect(Surface *s, Rect rect, Uint8 r, Uint8 g, Uint8 b, Uint8 a)
{
    std::auto_ptr<Surface> copy(Grab(s, rect));
    if (copy.get()) {
        copy->set_alpha(a);

        GC gc(s);
        set_color(gc, r,g,b);
        box(gc, rect);
        blit(gc, rect.x, rect.y, copy.get());
    }
}


/*
 * (SDL_ConvertSurface with a few changes)
 */

static SDL_Surface *CropSurface (SDL_Surface *surface,
                                 SDL_Rect rect,
                                 SDL_PixelFormat *format, 
                                 Uint32 flags)
{
    SDL_Surface *convert;
    Uint32 colorkey = 0;
    Uint8 alpha = 0;
    Uint32 surface_flags;
    SDL_Rect bounds;

    /* Check for empty destination palette! (results in empty image) */
    if ( format->palette != NULL ) {
        int i;
        for ( i=0; i<format->palette->ncolors; ++i ) {
            if ( (format->palette->colors[i].r != 0) ||
                 (format->palette->colors[i].g != 0) ||
                 (format->palette->colors[i].b != 0) )
                break;
        }
        if ( i == format->palette->ncolors ) {
            SDL_SetError("Empty destination palette");
            return(NULL);
        }
    }

    /* Create a new surface with the desired format */
    convert = SDL_CreateRGBSurface(flags,
                                   rect.w, rect.h, format->BitsPerPixel,
                                   format->Rmask, format->Gmask, format->Bmask, format->Amask);
    if ( convert == NULL ) {
        return(NULL);
    }

    /* Copy the palette if any */
    if ( format->palette && convert->format->palette ) {
        memcpy(convert->format->palette->colors,
               format->palette->colors,
               format->palette->ncolors*sizeof(SDL_Color));
        convert->format->palette->ncolors = format->palette->ncolors;
    }

    /* Save the original surface color key and alpha */
    surface_flags = surface->flags;
    if ( (surface_flags & SDL_SRCCOLORKEY) == SDL_SRCCOLORKEY ) {
        /* Convert colourkeyed surfaces to RGBA if requested */
        if((flags & SDL_SRCCOLORKEY) != SDL_SRCCOLORKEY
           && format->Amask) {
            surface_flags &= ~SDL_SRCCOLORKEY;
        } else {
            colorkey = surface->format->colorkey;
            SDL_SetColorKey(surface, 0, 0);
        }
    }
    if ( (surface_flags & SDL_SRCALPHA) == SDL_SRCALPHA ) {
        alpha = surface->format->alpha;
        SDL_SetAlpha(surface, 0, 0);
    }

    /* Copy over the image data */
    bounds.x = 0;
    bounds.y = 0;
    bounds.w = rect.w;
    bounds.h = rect.h;
    SDL_LowerBlit(surface, &rect, convert, &bounds);

    /* Clean up the original surface, and update converted surface */
//     if ( convert != NULL ) {
//         SDL_SetClipRect(convert, &surface->clip_rect);
//     }

    if ( (surface_flags & SDL_SRCCOLORKEY) == SDL_SRCCOLORKEY ) {
        Uint32 cflags = surface_flags&(SDL_SRCCOLORKEY|SDL_RLEACCELOK);
        if ( convert != NULL ) {
            Uint8 keyR, keyG, keyB;

            SDL_GetRGB(colorkey,surface->format,&keyR,&keyG,&keyB);
            SDL_SetColorKey(convert, cflags|(flags&SDL_RLEACCELOK),
                            SDL_MapRGB(convert->format, keyR, keyG, keyB));
        }
        SDL_SetColorKey(surface, cflags, colorkey);
    }
    if ( (surface_flags & SDL_SRCALPHA) == SDL_SRCALPHA ) {
        Uint32 aflags = surface_flags&(SDL_SRCALPHA|SDL_RLEACCELOK);
        if ( convert != NULL ) {
            SDL_SetAlpha(convert, aflags|(flags&SDL_RLEACCELOK),
                         alpha);
        }
        SDL_SetAlpha(surface, aflags, alpha);
    }

    /* We're ready to go! */
    return(convert);
}

Surface * ecl::Grab (const Surface *s, Rect &r) {
    if (s==0)
        return 0;
    SDL_Surface *sdls = s->get_surface();

    int x = 0;
    int y = 0;

    clip_blit (s->size(), x, y, r);

    SDL_Rect rect;
    sdl::copy_rect (rect, r);
    SDL_Surface *copy = CropSurface (sdls, rect, sdls->format, sdls->flags);
    return Surface::make_surface (copy);
}

// LoadImage is already defined as a macro in some Windows header file
#undef LoadImage


Surface* ecl::LoadImage (const char* filename)
{
    return LoadImage(IMG_Load(filename));
}

Surface* ecl::LoadImage(SDL_RWops *src, int freesrc) {
    return LoadImage(IMG_Load_RW(src, freesrc));
}

Surface* ecl::LoadImage(SDL_Surface *tmp) {
    if (tmp != NULL)
    {
        SDL_Surface* img;

        if (tmp->flags & SDL_SRCALPHA) {
            SDL_SetAlpha(tmp, SDL_RLEACCEL, 0);
            img = SDL_DisplayFormatAlpha(tmp);
        }
        else if (tmp->flags & SDL_SRCCOLORKEY) {
            SDL_SetColorKey(tmp, SDL_SRCCOLORKEY | SDL_RLEACCEL,
                            tmp->format->colorkey);
            img = SDL_DisplayFormat(tmp);
        }
        else
            img = SDL_DisplayFormat(tmp);
        if (img == 0)
            return Surface::make_surface (tmp);
        SDL_FreeSurface(tmp);
        return Surface::make_surface (img);
    }
    return 0;
}

Surface * ecl::MakeSurface(int w, int h, int bipp, const RGBA_Mask &mask)
{
    SDL_Surface* sfc;
    sfc = SDL_CreateRGBSurface(SDL_SWSURFACE, w, h, bipp,
                               mask.r, mask.g, mask.g, mask.a);
    if (sfc == 0)
        return 0; //throw XVideo(string("Couldn't create surface: ") + SDL_GetError());
    return Surface::make_surface (sfc);
}


/* Create a surface from image data that is already somewhere in
   memory.  */
Surface * ecl::MakeSurface(void *data, int w, int h, int bipp, int pitch,
                          const RGBA_Mask &mask)
{
    SDL_Surface* sfc;
    sfc = SDL_CreateRGBSurfaceFrom(data, w, h, bipp,
                                   pitch, mask.r, mask.g, mask.b, mask.a);
    if (sfc == 0)
        return 0; //throw XVideo(string("Couldn't create surface:") + SDL_GetError());
    return Surface::make_surface (sfc);
}

Surface *ecl::MakeSurfaceLike (int w, int h, Surface *surface) 
{
    if (surface == 0)
        return 0;
    SDL_Surface *sdls = surface->get_surface();

    return MakeSurface (w, h, sdls->format->BitsPerPixel,
                        RGBA_Mask(sdls->format->Rmask,
                                  sdls->format->Gmask,
                                  sdls->format->Bmask,
                                  sdls->format->Amask));
}

/*! Resample a region inside a surface to a new size.  Returns a
  new 32 bit RGBA image containing the scaled image. */
Surface *ecl::Resample (Surface *s, Rect rect, int neww, int newh,
                       ResampleFilter /*filter*/)
{
    SDL_Surface *sdls = SDL_CreateRGBSurface(SDL_SWSURFACE, rect.w, rect.h, 32,
                                             0, 0, 0, 0);
    SDL_Rect r;
    sdl::copy_rect (r, rect);
    SDL_BlitSurface (s->get_surface(), &r, sdls, 0);

    SDL_Surface *s_new;
    s_new  = zoomSurface (sdls,
                          (double)neww/rect.w, 
                          (double)newh/rect.h, true);

    SDL_FreeSurface (sdls);
    return Surface::make_surface (s_new);
}

