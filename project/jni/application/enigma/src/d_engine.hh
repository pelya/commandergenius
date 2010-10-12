/*
 * Copyright (C) 2003 Daniel Heck
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
#ifndef D_ENGINE_HH
#define D_ENGINE_HH

#include "ecl_geom.hh"
#include "ecl_array2.hh"
#include "ecl_alist.hh"
#include "ecl_fwd.hh"

namespace display
{

/* -------------------- DisplayEngine -------------------- */

    class DisplayEngine {
    public:
        DisplayEngine (int tilew=32, int tileh=32);
        ~DisplayEngine();

        /* ---------- Class configuration ---------- */
        void  add_layer (DisplayLayer *l);
        void  set_screen_area (const ecl::Rect & r);
        void  set_tilesize (int w, int h);

        int   get_tilew () const { return m_tilew; }
        int   get_tileh () const { return m_tileh; }
        int   get_width() const { return m_width; }
        int   get_height() const { return m_height; }
        const ecl::Rect &get_area() const { return m_area; }

        /* ---------- Scrolling / page flipping ---------- */
        void set_offset (const ecl::V2 &off);
        void move_offset (const ecl::V2 &off);
        ecl::V2 get_offset () const { return m_offset; }

        /* ---------- Game-related stuff ---------- */
        void new_world (int w, int h);
        void tick (double dtime);

        /* ---------- Coordinate conversion ---------- */
        void      world_to_screen (const ecl::V2 & pos, int *x, int *y);
        WorldArea screen_to_world (const ScreenArea &a);
        ScreenArea world_to_screen (const WorldArea &a);

        /* "Video" coordinates are like screen coordinates, except the
           origin coincides with the world origin, not the current
           scrolling position. */
        void world_to_video (const ecl::V2 &pos, int *x, int *y);
        void video_to_screen (int x, int y, int *xx, int *yy);
        void video_to_world (const ecl::Rect &r, ecl::Rect &s);

        V2 to_world (const V2 &pos);

        /* ---------- Screen upates ---------- */

        void mark_redraw_screen();
        void mark_redraw_area (const WorldArea &wa, int delay=0);

        void redraw_screen_area (const ScreenArea &a);
        void redraw_world_area (const WorldArea &a);

        void update_screen();
        void draw_all (ecl::GC &gc);
        void update_offset();

    private:
        void update_layer (DisplayLayer *l, WorldArea wa);

        /* ---------- Variables ---------- */

        std::vector<DisplayLayer *> m_layers;
        int m_tilew, m_tileh;

        // Offset of screen
        ecl::V2 m_offset;        // Offset in world units
        ecl::V2 m_new_offset;    // New offset in world units
        int    m_screenoffset[2]; // Offset in screen units


        // Screen area occupied by level display
        ecl::Rect m_area;

        // Width and height of the world in tiles
        int m_width, m_height;

        ecl::Array2<char> m_redrawp;
    };


/* -------------------- DisplayLayer -------------------- */

    class DisplayLayer {
    public:
        DisplayLayer() {}
        virtual ~DisplayLayer() {}

        /* ---------- Class configuration ---------- */
        void set_engine (DisplayEngine *e) { m_engine = e; }
        DisplayEngine *get_engine() const { return m_engine; }

        /* ---------- DisplayLayer interface ---------- */
        virtual void prepare_draw (const WorldArea &) {}
        virtual void draw (ecl::GC &gc, const WorldArea &a, int x, int y) = 0;
        virtual void draw_onepass (ecl::GC &/*gc*/) {}
        virtual void tick (double /*dtime*/) {}
        virtual void new_world (int /*w*/, int /*h*/) {}

        // Functions.
        void mark_redraw_area (const ecl::Rect &r)
        {
            get_engine()->mark_redraw_area(r);
        }
    private:
        DisplayEngine *m_engine;
    };


/* -------------------- ModelLayer -------------------- */

    /*! The base class for all layers that contains Models. */
    class ModelLayer : public DisplayLayer {
    public:
        ModelLayer() {}

        // DisplayLayer interface
        void tick (double dtime);
        void new_world (int, int);

        // Member functions
        void activate (Model *m);
        void deactivate (Model *m);
        void maybe_redraw_model(Model *m, bool immediately=false);

        virtual int redraw_size () const { return 2; }
    private:

        // Variables
        ModelList m_active_models;
        ModelList m_active_models_new;
    };


/* -------------------- DL_Grid -------------------- */

    /*! Layer for grid-aligned models (stones, floor tiles, items). */

    class DL_Grid : public ModelLayer {
    public:
        DL_Grid(int redrawsize = 1);
        ~DL_Grid();

        void set_model (int x, int y, Model *m);
        Model *get_model (int x, int y);
        Model *yield_model (int x, int y);

    private:
        // DL_Grid interface.
        void mark_redraw (int x, int y);

        // DisplayLayer interface.
        void new_world (int w, int h);
        void draw (ecl::GC &gc, const WorldArea &a, int x, int y);

        // ModelLayer interface
        virtual int redraw_size () const { return m_redrawsize; }

        // Variables.
        typedef ecl::Array2<Model*> ModelArray;
        ModelArray m_models;
        int m_redrawsize;
    };


/* -------------------- Sprites -------------------- */

    class Sprite : public ecl::Nocopy {
    public:
        Model       *model;
        V2           pos;
        int          screenpos[2];
        SpriteLayer  layer;
        bool         visible;
        bool         mayNeedRedraw;

        Sprite (const V2 & p, SpriteLayer l, Model *m)
        : model(m), pos(p), layer(l), visible(true), mayNeedRedraw(false)
        {
            screenpos[0] = screenpos[1] = 0;
        }
        ~Sprite() { delete model; }
    };

    typedef std::vector<Sprite*> SpriteList;

    class DL_Sprites : public ModelLayer {
    public:
        DL_Sprites();
        ~DL_Sprites();

        /* ---------- DisplayLayer interface ---------- */
        void draw (ecl::GC &gc, const WorldArea &a, int x, int y);
        void draw_onepass (ecl::GC &gc);
        void new_world (int, int);

        /* ---------- Member functions ---------- */
        SpriteId add_sprite (Sprite *sprite);
        void kill_sprite (SpriteId id);
        void move_sprite (SpriteId, const ecl::V2& newpos);
        void replace_sprite (SpriteId id, Model *m);

        void redraw_sprite_region (SpriteId id);
        void draw_sprites (bool shades, ecl::GC &gc);

        Model *get_model (SpriteId id) { return sprites[id]->model; }

        void set_maxsprites (unsigned m) { maxsprites = m; }

        Sprite *get_sprite(SpriteId id);

        static const SpriteId MAGIC_SPRITEID = 1000000;
        SpriteList sprites;

    private:
        // ModelLayer interface
        virtual void tick (double /*dtime*/);

        // Variables.
        unsigned numsprites;    // Current number of sprites
        unsigned maxsprites;    // Maximum number of sprites
    };


/* -------------------- Shadows -------------------- */

    struct StoneShadowCache;

    class DL_Shadows : public DisplayLayer {
    public:
        DL_Shadows(DL_Grid *grid, DL_Sprites *sprites);
        ~DL_Shadows();

        void new_world(int w, int h);
        void draw (ecl::GC &gc, int xpos, int ypos, int x, int y);

        void draw (ecl::GC &gc, const WorldArea &a, int x, int y);
    private:
        /* ---------- Private functions ---------- */
        void shadow_blit (ecl::Surface *scr, int x, int y,
                          ecl::Surface *shadows, ecl::Rect r);

        bool has_actor (int x, int y);
        virtual void prepare_draw (const WorldArea &);

        Model * get_shadow_model(int x, int y);

        /* ---------- Variables ---------- */
        DL_Grid    *m_grid;     // Stone models
        DL_Sprites *m_sprites;  // Sprite models

        StoneShadowCache *m_cache;

        Uint32       shadow_ckey; // Color key
        ecl::Surface *buffer;

        ecl::Array2<bool>   m_hasactor;
    };


/* -------------------- Lines -------------------- */

    struct Line {
        V2 start,end;
        V2 oldstart, oldend;

        Line(const V2 &s, const V2 &e) :start(s), end(e) {}
        Line() {}
    };


    typedef ecl::AssocList<unsigned, Line> LineMap;

    class DL_Lines : public DisplayLayer {
    public:
        DL_Lines() : m_id(1)
        {
        }

        void draw (ecl::GC &/*gc*/, const WorldArea &/*a*/, int /*x*/, int /*y*/)
        {}
        void draw_onepass (ecl::GC &gc);

        RubberHandle add_line (const V2 &p1, const V2 &p2);
        void set_startpoint (unsigned id, const V2 &p1);
        void set_endpoint (unsigned id, const V2 &p2);
        void kill_line (unsigned id);

    private:
        // Private methods.
        void mark_redraw_line (const Line &r);

        // Variables.
        unsigned  m_id;
        LineMap   m_rubbers;
    };


/* -------------------- CommonDisplay -------------------- */

    /*! Parts of the display engine that are common to the game and
      the editor. */
    class CommonDisplay {
    public:
        CommonDisplay (const ScreenArea &a = ScreenArea (0, 0, 10, 10));
        ~CommonDisplay();

        Model *set_model (const GridLoc &l, Model *m);
        Model *get_model (const GridLoc &l);
        Model *yield_model (const GridLoc &l);

        void set_floor (int x, int y, Model *m);
        void set_item (int x, int y, Model *m);
        void set_stone (int x, int y, Model *m);

        DisplayEngine *get_engine() const { return m_engine; }

        SpriteHandle add_effect (const V2& pos, Model *m);
        SpriteHandle add_sprite (const V2 &pos, Model *m);

        RubberHandle add_line (V2 p1, V2 p2);

        void new_world (int w, int h);
        void redraw();

    protected:
        DL_Grid    *floor_layer;
        DL_Grid    *item_layer;
        DL_Grid    *stone_layer;

        DL_Sprites *effects_layer;

        DL_Lines   *line_layer;
        DL_Sprites *sprite_layer;
        DL_Shadows *shadow_layer;

    private:

        DisplayEngine *m_engine;
    };


/* -------------------- Scrolling -------------------- */


    class Follower {
    public:
        Follower (DisplayEngine *e);
        virtual ~Follower() {}
        virtual void tick(double dtime, const ecl::V2 &point) = 0;
        virtual void center(const ecl::V2 &point);

        void set_boundary (double b) { m_boundary = b; }

    protected:
        DisplayEngine *get_engine() const { return m_engine; }
        bool set_offset (V2 offs);
        double get_hoff() const;
        double get_voff() const;
        ecl::V2 get_scrollpos(const ecl::V2 &point);

        double m_boundary;

    private:
        DisplayEngine *m_engine;
    };

    /*! Follows a sprite by flipping to the next screen as soon as the
      sprite reaches the border of the current screen. */
    class Follower_Screen : public Follower {
    public:
        Follower_Screen(DisplayEngine *e);
        void tick(double dtime, const ecl::V2 &point);
    };

    /*! Follows a sprite by softly scrolling the visible area of the
      screen as soon as the sprite reaches the border of the current
      screen. */
    class Follower_Scrolling : public Follower {
    public:
        Follower_Scrolling (DisplayEngine *e, bool screenwise_);
        void tick(double dtime, const ecl::V2 &point);
        void center(const ecl::V2 &point);
    private:
        bool   currently_scrolling;
        V2     curpos, destpos;
        V2     dir;
        double scrollspeed;
        double resttime;
        bool screenwise;
    };

    class Follower_Smooth : public Follower {
    public:
        Follower_Smooth (DisplayEngine *e);
        void tick (double time, const ecl::V2 &point);
        void center (const ecl::V2 &point);
        virtual void set_boundary (double b) {}

        ecl::V2 calc_offset (const ecl::V2 &point);
    };


/* -------------------- GameDisplay -------------------- */

    class GameDisplay : public CommonDisplay {
    public:
        GameDisplay (const ScreenArea &gamearea, 
                     const ScreenArea &inventoryarea);
        ~GameDisplay();

        StatusBar * get_status_bar() const;

        void tick(double dtime);
        void new_world (int w, int h);

        void resize_game_area (int w, int h);

        /* ---------- Scrolling ---------- */
        void set_follow_mode (FollowMode m);
        void follow_center();
        void set_follow_sprite(SpriteId id);
        void set_reference_point (const ecl::V2 &point);
        void set_scroll_boundary (double d);

        // current screen coordinates of reference point
        void get_reference_point_coordinates(int *x, int *y);

        /* ---------- Screen updates ---------- */
        void redraw (ecl::Screen *scr);
        void redraw_all (ecl::Screen *scr);
        void draw_all (ecl::GC &gc);

    private:
        void set_follower (Follower *f);
        void draw_borders (ecl::GC &gc);

        /* ---------- Variables ---------- */
        Uint32         last_frame_time;
        bool           redraw_everything;
        StatusBarImpl *status_bar;

        V2          m_reference_point;
        Follower   *m_follower;

        ScreenArea inventoryarea;
    };

    class ModelHandle {
    public:
        ModelHandle ();
    };
}

#endif
