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
#ifndef DISPLAY_HH_INCLUDED
#define DISPLAY_HH_INCLUDED

#include "enigma.hh"
#include "ecl.hh"

//----------------------------------------
// Definition of models
//----------------------------------------
namespace display
{
    class DisplayLayer;
    class ModelLayer;

    /*! Animations can invoke a callback of this type on completion.
      Note that you may not delete or replace models other than the
      one that induced the callback from inside the callback--use a
      timer or a flag to do this. */
    class ModelCallback {
    public:
        virtual ~ModelCallback() {}
        virtual void animcb() = 0;
    };

    class Animation {
    public:
        virtual ~Animation() {}
        virtual void set_callback (ModelCallback *) {}
        virtual void reverse() {}
        virtual void restart() {}

	virtual bool is_garbage() const { return false; }
        virtual void tick(double /*dtime*/) {}
        virtual bool has_changed(ecl::Rect &/*changed_region*/) { return false; }
    };

    class Model : public Animation {
    public:

        virtual void draw(ecl::GC &/*gc*/, int /*x*/, int /*y*/) {}
        virtual void draw_shadow(ecl::GC &/*gc*/, int /*x*/, int /*y*/) {}

        virtual Model *get_shadow() const { return 0; }

        virtual void expose (ModelLayer * /*ml*/, int videox, int videoy) {}
        virtual void remove (ModelLayer * /*ml*/) {}

        virtual Model *clone() = 0;
        virtual void get_extension (ecl::Rect &r);
    };

/* -------------------- Functions -------------------- */

    void InitModels();
    void ShutdownModels();

    Model * MakeModel (const std::string &name);


    int DefineImage (const char *name, const char *fname,
                     int xoff, int yoff, int padding);
    int DefineSubImage (const char *name, const char *fname,
                        int xoff, int yoff, ecl::Rect r);
    void DefineRandModel (const char *name, int n, char **names);
    void DefineShadedModel (const char *name, const char *model, const char *shade);
    void DefineOverlayImage (const char *name, int n, char **images);
    void DefineComposite (const char *name, const char *bgname, const char *fgname);
    void DefineAnim (const char *name, bool loop_p);
    void AddFrame (const char *name, const char *model, double time);
    void DefineAlias (const char *name, const char *othername);
}

//----------------------------------------
// Models on the grid
//----------------------------------------
namespace display
{
    using enigma::GridPos;
    using enigma::GridLayer;
    using enigma::GridLoc;

    Model*  SetModel (const GridLoc & l, const std::string &modelname);
    Model*  SetModel (const GridLoc & l, Model *m);
    void    KillModel (const GridLoc & l);
    Model*  GetModel (const GridLoc & l);
    Model*  YieldModel (const GridLoc & l);
}

/* -------------------- Scrolling -------------------- */
namespace display
{
    enum FollowMode {
        FOLLOW_NONE            = 0, // Don't follow any sprite
        FOLLOW_SCROLLING       = 1, // Scroll the screen
        FOLLOW_SCREEN          = 2, // Flip the screen region
        FOLLOW_SCREENSCROLLING = 3, // Scroll to the next screen
        FOLLOW_SMOOTH          = 4, // Follow pixel by pixel
    };


    void SetFollowMode (FollowMode m);
    void SetScrollBoundary (double boundary);

    void SetReferencePoint (const ecl::V2 &point);
    void GetReferencePointCoordinates(int *x, int *y);
    void FocusReferencePoint();
}

/* -------------------- Sprites -------------------- */
namespace display
{
    enum SpriteLayer {
        SPRITE_ACTOR, SPRITE_EFFECT, SPRITE_DEBRIS
    };

    typedef unsigned int SpriteId;

    class DL_Sprites;

    class SpriteHandle {
        DL_Sprites *layer;
        unsigned id;
    public:
        SpriteHandle (DL_Sprites *l, unsigned spriteid);
        SpriteHandle();

        void kill();
        void move (const ecl::V2 &newpos) const;
        void replace_model (Model *m) const;
        Model *get_model () const;
        void set_callback (ModelCallback *cb) const;
        void hide() const;
        void show() const;
    };

    /*! Add a new effect sprite.  Sprites of this type are
      automatically deleted once the animation has finished.  */
    SpriteHandle AddEffect (const ecl::V2 &pos, const char *modelname);

    /*! Create a new sprite.  If modelname==0, the sprite is
      considered invisible.  Sprites of this type are _never_
      automatically deleted.  */
    SpriteHandle AddSprite (const ecl::V2 &pos, const char *modelname=0);

}

/* -------------------- Rubber bands -------------------- */
namespace display
{
    class DL_Lines;

    class RubberHandle {
    public:
        RubberHandle (DL_Lines *layer=0, unsigned id=0);
        operator unsigned() { return id; }

        void update_first (const ecl::V2 &p1);
        void update_second (const ecl::V2 &p2);
        void kill();

        DL_Lines *line_layer;
        unsigned id;
    };

    RubberHandle AddRubber (const ecl::V2 &p1, const ecl::V2 &p2);
}


/* -------------------- Status bar -------------------- */
namespace display
{
    using enigma_player::Inventory;

    class StatusBar {
    public:
        virtual ~StatusBar() {}
        virtual void set_inventory (const std::vector<std::string> &modelnames) = 0;

        virtual void show_text (const std::string &str, 
                                bool scrolling, 
                                double duration = -1) = 0;
        virtual void hide_text() = 0;

        virtual void show_move_counter (bool active) = 0;
        virtual void show_odometer (bool active)     = 0;

        virtual void set_time (double time) = 0;
        virtual void set_speed (double speed) = 0;
        virtual void set_travelled_distance (double distance) = 0;
        virtual void set_counter (int nummoves) = 0;
    };

    StatusBar *GetStatusBar();

#define STATUSBAR display::GetStatusBar()


/* -------------------- Interface to display engine -------------------- */

    enum DisplayFlags
    {
        SHOW_FLOOR     = 0x01,
        SHOW_STONES    = 0x02,
        SHOW_ITEMS     = 0x04,
        SHOW_SHADES    = 0x08,
        SHOW_SPRITES   = 0x10,
	SHOW_TIME      = 0x20,
	SHOW_INVENTORY = 0x40,
        SHOW_ALL       = 0x7f
    };

    void ToggleFlag(DisplayFlags flag);


    void Init();
    void Shutdown();

    void            NewWorld (int w, int h);
    void            ResizeGameArea (int w, int h);
    const ecl::Rect& GetGameArea ();

    void DrawAll (ecl::GC &gc);
    void RedrawAll (ecl::Screen *sfc);
    void Redraw (ecl::Screen *sfc);
    void Tick (double dtime);
}

#endif
