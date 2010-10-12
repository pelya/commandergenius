/*
 * Copyright (C) 2002,2003 Daniel Heck
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
#ifndef ECL_VIDEO_HH_INCLUDED
#define ECL_VIDEO_HH_INCLUDED

#include "ecl_fwd.hh"
#include "ecl_geom.hh"

#include "SDL.h"

namespace ecl
{

/* -------------------- Colors -------------------- */

    struct RGBA_Mask {
        RGBA_Mask(Uint32 rr=0, Uint32 gg=0, Uint32 bb=0, Uint32 aa=0)
        : r(rr), g(gg), b(bb), a(aa)
        {}
        Uint32 r,g,b,a;
    };

    struct RGB {
        RGB(char rr=0, char gg=0, char bb=0)
        : r(rr), g(gg), b(bb) 
        {}
        char r,g,b;
    };

    struct RGBA {
        RGBA(char rr=0, char gg=0, char bb=0, char aa=0)
        : r(rr), g(gg), b(bb), a(aa)
        {}
        char r,g,b,a;
    };

    typedef Uint32 PackedColor;


/* -------------------- Graphics State (GS) -------------------- */

    enum GS_Flags {
	GS_DEFAULT   = 0,
	GS_ANTIALIAS = 1,
        GS_NOCLIP    = 2
    };

    struct GraphicsState {
	// Constructors.
	GraphicsState (const Rect &clipr = Rect()) 
	: cliprect(clipr), pcolor(0), flags(GS_DEFAULT)
	{}

	// Variables.
	Rect        cliprect;   // current clipping rectangle
	PackedColor pcolor;	// current color
	unsigned    flags;
    };

    typedef GraphicsState GS;


/* -------------------- Drawable -------------------- */

    class Drawable {
    public:
	virtual ~Drawable() {}

        /* ---------- Drawable interface ---------- */

        virtual PackedColor map_color(int r, int g, int b) = 0;
        virtual PackedColor map_color(int r, int g, int b, int a) = 0;

	virtual void blit (const GS &gs, int x, int y, const Surface* s) = 0;
	virtual void blit (const GS &gs, int x, int y, const Surface* s, const Rect& r) = 0;

	virtual Uint32 get_pixel (int x, int y) = 0;

        //! Set a single pixel
	virtual void set_pixel  (const GS &gs, int x, int y) = 0;

        //! Set multiple pixels at once
	virtual void set_pixels (const GS &gs, int n, const int *x, const int *y);

        //! Draw a horizontal line
	virtual void hline (const GS &gs, int x, int y, int w);

        //! Draw a vertical line
	virtual void vline (const GS &gs, int x, int y, int h);

        //! Draw an arbitrary line
        virtual void line  (const GS &gs, int x1, int y1, int x2, int y2);

        //! Draw a filled box.
	virtual void box   (const GS &gs, int x, int y, int w, int h);

        //! Return size of drawable: Rect (0,0,width, height)
        virtual Rect size() const = 0;
    };

/* -------------------- Graphics Context (GC) -------------------- */

    struct GC : public GraphicsState {
        GC(Drawable* d) :GraphicsState (d->size()) { 
            drawable = d; 
        }

        Drawable     *drawable;
    };



/* -------------------- Surface -------------------- */

    class Surface : public Drawable {
    public:
	~Surface();

	Surface *zoom(int w, int h);

	void set_color_key (int r, int g, int b);
        void set_alpha (int a);

        void lock();
	void unlock();

        int bypp() const { return m_surface->format->BytesPerPixel; }
        int bipp() const { return m_surface->format->BitsPerPixel; }
        Uint32 pitch() const { return m_surface->pitch; }

        inline void* scanline_pointer(int y) { 
            return (Uint8*)m_surface->pixels + y*pitch();
        }
    
        inline void* pixel_pointer(int x, int y) {
            return static_cast<Uint8*>(scanline_pointer(y)) + x*bypp();
        }

        int height() const { return m_surface->h; }
        int width() const { return m_surface->w; }

        SDL_Surface *get_surface() const { return m_surface; }

        /* ---------- Drawable interface ---------- */
	PackedColor map_color(int r, int g, int b);
	PackedColor map_color(int r, int g, int b, int a);

        Rect size() const { return Rect(0,0,m_surface->w, m_surface->h); }
        void box (const GS &gs, int x, int y, int w, int h);
        void line (const GS &gs, int x1, int y1, int x2, int y2);
        void blit (const GS &gs, int x, int y, const Surface* s, const Rect &r);
        void blit (const GS &gs, int x, int y, const Surface* src);

	/* ---------- Static methods ---------- */

        //! Create a new surface
	static Surface *make_surface (SDL_Surface *s);

    protected:
	// Constructor.
	Surface (SDL_Surface* sfc);

	// Variables
	SDL_Surface *m_surface;

    private:
    };

    class SurfaceLock {
        Surface *s;
    public:
	SurfaceLock (Surface *s_) : s(s_) { s->lock(); }
	~SurfaceLock() { s->unlock(); }
    };


/* -------------------- Screen -------------------- */

    class Screen {
    public:
	Screen (Surface *s);
        Screen (SDL_Surface *s);
        ~Screen();

	void update_all();
	void update_rect(const Rect& r);
	void flush_updates();
	void set_caption(const char* str);

        /* ---------- Accessors ---------- */

        Surface *get_surface() const { return m_surface; }

        Rect size() const;
        int width() const;
        int height() const;

	/* ---------- Static methods ---------- */

        static Screen *get_instance();
    private:
        // Variables.
        static Screen *m_instance;

        Surface     *m_surface;
        SDL_Surface *m_sdlsurface;
	RectList     m_dirtyrects;
        bool         update_all_p;

	Screen(const Screen&);
	Screen& operator=(const Screen&);
    };


/* -------------------- Graphics primitives -------------------- */

    inline void set_color (GC &gc, int r, int g, int b, int a) {
        gc.pcolor = gc.drawable->map_color(r, g, b, a);
    }
        
    inline void set_color (GC &gc, int r, int g, int b) {
        gc.pcolor = gc.drawable->map_color(r, g, b);
    }

    inline void set_color (GC &gc, const RGB &c) {
        set_color (gc, c.r, c.g, c.b);
    }
        
    inline void set_color (GS &gs, PackedColor c) { 
        gs.pcolor=c; 
    }

    inline void enable_clipping(GS &gs) { 
        clear_flags (gs.flags, GS_NOCLIP); 
    }
    
    inline void disable_clipping (GS &gs) { 
        set_flags (gs.flags, GS_NOCLIP); 
    }

    inline void clip (GS &gs, const Rect& r) { 
        gs.cliprect = r; 
        enable_clipping(gs);
    }

    inline void clip (GC &gc, const Rect& r) { 
        gc.cliprect = intersect (r, gc.drawable->size());
        enable_clipping(gc);
    }

    inline void clip (GC &gc) {
        clip (gc, gc.drawable->size());
    }

    inline void blit (const GC &gc, int x, int y, const Surface *s) { 
        gc.drawable->blit (gc, x, y, s); 
    }

    inline void blit(const GC &gc, int x, int y, const Surface *s, const Rect &r) { 
        gc.drawable->blit (gc, x, y, s, r); 
    }

    inline void set_pixel(const GC &gc, int x, int y) { 
        gc.drawable->set_pixel (gc, x, y); 
    }

    inline void hline (const GC & gc, int x, int y, int w) { 
        gc.drawable->hline (gc, x, y, w); 
    }
    
    inline void vline (const GC & gc, int x, int y, int h) { 
        gc.drawable->vline (gc, x, y, h); 
    }

    inline void box (const GC &gc, const Rect& r) { 
        gc.drawable->box(gc, r.x, r.y, r.w, r.h);
    }
    
    inline void box (const GC &gc, int x, int y, int w, int h) { 
        gc.drawable->box (gc, x, y, w, h); 
    }

    void line (const GC &gc, int x1, int y1, int x2, int y2);
    void frame (const GC & gc, int x, int y, int w, int h);

    inline void frame (const GC &gc, const Rect& r) { 
        frame (gc, r.x, r.y, r.w, r.h);
    }


/* -------------------- Functions -------------------- */

    Screen *OpenScreen (int w, int h, int bipp);
    Screen* DisplayFormat(Screen* s);


    /*! Create a new surface. */
    Surface *MakeSurface(int w, int h, int bipp, 
                         const RGBA_Mask &mask = RGBA_Mask());

    /*! Create a surface from image data that is already somewhere in
      memory.  */
    Surface * MakeSurface(void* data, int w, int h, int bipp, int pitch,
                          const RGBA_Mask &mask = RGBA_Mask());

    /*! Create a new surface with the same image format as
      `surface'. */
    Surface *MakeSurfaceLike (int w, int h, Surface *surface);

    /*! Create a copy of a surface. */
    Surface *Duplicate(const Surface *s);

    /*! Save a surface to a PNG file. */
    void SavePNG (const Surface *s, const std::string& filename);

    /*! Create a new surface from a selection of an old one. Performs
      proper clipping and returns a surface of the appropriate size
      (this may be smaller than the original size given in `r'.)  The
      function stores the real region in `r'. */
    Surface *Grab(const Surface *s, Rect &r);

    /*! Convert a surface to the current screen's native format. */
    Surface* DisplayFormat(Surface* s);

    /*! Load an image using SDL_image and convert it to an optimized
      format. */
    Surface* LoadImage(const char* filename);
    Surface* LoadImage(SDL_RWops *src, int freesrc);
    Surface* LoadImage(SDL_Surface *tmpImage);
    
    /*! Overlay a rectangle `rect' in `s' with a transparent colored
      box. */
    void TintRect(Surface *s, Rect rect, Uint8 r, Uint8 g, Uint8 b, Uint8 a);


    enum ResampleFilter {
        FILTER_BILINEAR,
        FILTER_LANCZOS
    };

    /*! Resample a region inside a surface to a new size.  Returns a
      new 32 bit RGBA image containing the scaled image. 

      (Only partially implemented so far.)
    */
    Surface *Resample (Surface *s, Rect rect, int neww, int newh, 
                       ResampleFilter filter = FILTER_BILINEAR);
}

#endif /* !ECL_VIDEO_HH_INCLUDED */
