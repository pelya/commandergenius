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

/*
 * This file contains the code renders the graphics during the game
 * and in the editor.  This includes displaying the current landscape
 * with all its objects and the inventory at the bottom of the screen.
 */

#include "display_internal.hh"
#include "video.hh"
#include "main.hh"

#include "ecl_sdl.hh"
#include "ecl.hh"

#include <algorithm>
#include <functional>
#include <cmath>
#include <iostream>

using namespace std;
using namespace ecl;
using namespace display;
using namespace enigma;

#include "d_engine.hh"
#include "d_models.hh"

class dRect {
public:
    dRect (double x_, double y_, double w_, double h_) { 
        x = x_; y = y_; w = w_; h = h_;
    }
    double x, y, w, h;
};

Rect round_grid (const dRect &r, double w, double h) {
    double x  = r.x / w ;
    double y  = r.y / h;
    double x2 = (r.x + r.w-1) / w;
    double y2 = (r.y + r.h-1) / h;

    Rect s (round_down<int> (x), round_down<int> (y),
            round_down<int> (x2), round_down<int> (y2));
    s.w -= s.x-1;
    s.h -= s.y-1;
    return s;
}


/* -------------------- Local variables -------------------- */

namespace
{
    const int NTILESH = 20;     // Default game screen width in tiles
    const int NTILESV = 13;     // Default game screen height in tiles


    DisplayFlags  display_flags = SHOW_ALL;
    GameDisplay  *gamedpy       = 0;
    bool          ShowFPS       = false;

}


//======================================================================
// STATUS BAR
//======================================================================

StatusBarImpl::StatusBarImpl (const ScreenArea &area)
: Window(area),
  m_itemarea (),
  m_models(),
  m_changedp(false),
  m_textview (*enigma::GetFont("statusbarfont")),
  m_leveltime (0),
  m_showtime_p (true),
  m_counter (0),
  m_showcounter_p(false),
  m_showodometer_p(false),
  m_interruptible(true),
  m_text_active(false)
{
    const video::VMInfo *vminfo = video::GetInfo();
    m_itemarea = vminfo->sb_itemarea;
}

StatusBarImpl::~StatusBarImpl() {
    ecl::delete_sequence(m_models.begin(), m_models.end());
    m_models.clear();
}

void StatusBarImpl::set_time (double time) {
    double oldtime=m_leveltime;
    m_leveltime = time;
    if (m_showtime_p && floor(m_leveltime)-floor(oldtime) >= 1)
        m_changedp = true;      // update clock
}

void StatusBarImpl::hide_text() 
{ 
    if (m_text_active) {
        m_text_active = false;
        m_changedp = true;
    }
}

void StatusBarImpl::set_speed (double /*speed*/)
{
}

void StatusBarImpl::set_travelled_distance (double /*distance*/)
{
}

void StatusBarImpl::set_counter (int new_counter) 
{
    if (m_showcounter_p && new_counter != m_counter) {
        m_changedp = true;
        m_counter = new_counter;
    }
}

void StatusBarImpl::show_move_counter (bool active) 
{
    if (active != m_showcounter_p) {
        m_showcounter_p = active;
        m_changedp      = true;
    }
}

void StatusBarImpl::show_odometer (bool active) 
{
    if (active != m_showodometer_p) {
        m_showodometer_p = active;
        m_changedp = true;
    }
}

#ifdef _MSC_VER
#define snprintf _snprintf
#endif


void StatusBarImpl::redraw (ecl::GC &gc, const ScreenArea &r) {
    const video::VMInfo *vminfo = video::GetInfo();
    ScreenArea a = get_area();
    clip(gc, intersect(a, r));

    blit(gc, a.x, a.y, enigma::GetImage ("inventory", ".jpg"));


//     set_color (gc, 255, 0, 0);
//     frame (gc, vminfo->sb_timearea);
//     frame (gc, vminfo->sb_textarea);
//     set_color (gc, 0, 255, 0);
//     frame (gc, vminfo->sb_movesarea);
//     frame (gc, vminfo->sb_itemarea);

    if (m_showtime_p || m_showcounter_p) {
        const int  BUFSIZE     = 8;
        char       buf[BUFSIZE];
        int        xsize_time  = 0;
        int        xsize_moves = 0;
        Surface   *s_time      = 0;
        Surface   *s_moves     = 0;
        Font      *timefont    = enigma::GetFont ("timefont");
        Font      *movesfont   = enigma::GetFont ("smallfont");
        ScreenArea timearea    = vminfo->sb_timearea;
        ScreenArea movesarea   = vminfo->sb_movesarea;

        if (m_showtime_p) {
            double     abstime       = m_leveltime >= 0 ? m_leveltime : fabs(floor(m_leveltime));
            int        minutes       = static_cast<int>(abstime/60);
            int        seconds       = static_cast<int>(abstime) % 60;

            if (minutes >= 100) {
                minutes = 99;
                seconds = 59;
            }
            snprintf(buf, BUFSIZE,
                     m_leveltime >= 0 ? "%d:%02d" : "-%d:%02d",
                     minutes, seconds);
            s_time = timefont->render(buf);
            xsize_time = s_time->width();
        }

        if (m_showcounter_p) {
            int len = snprintf(buf, BUFSIZE, "%d", m_counter);
            s_moves     = movesfont->render(buf);
            xsize_moves = s_moves->width();
        }


        if (m_showtime_p) {
            if (m_showcounter_p) { // time + moves
                int x = timearea.x + (movesarea.x - timearea.x - xsize_time)/2;
                int y = timearea.y + (timearea.h - timefont->get_lineskip())/2;
                blit(gc, x, y, s_time);
                
                x = movesarea.x + (movesarea.w - xsize_moves)/2;
                y = movesarea.y + (movesarea.h + timefont->get_lineskip())/2 - movesfont->get_lineskip() - 4;
                blit(gc, x, y, s_moves);
            }
            else { // only time
                int x = timearea.x + (timearea.w - xsize_time)/2;
                int y = timearea.y + (timearea.h - timefont->get_lineskip())/2;
                blit(gc, x, y, s_time);
            }
        }
        else {                  // only moves            
            int x = timearea.x + (timearea.w - xsize_moves)/2;
            int y = timearea.y + (timearea.h - movesfont->get_lineskip())/2;
            blit(gc, x, y, s_moves);
        }

        delete s_moves;
        delete s_time;
    }

    if (m_text_active) {
        m_textview.draw (gc, r);
    }
    else {
        int itemsize = static_cast<int>(vminfo->tile_size * 1.25);
        int x = m_itemarea.x;
        for (unsigned i=0; i<m_models.size(); ++i) {
            Model *m = m_models[i];
            m->draw(gc, x, m_itemarea.y);
            x += itemsize;
        }
    }
    m_changedp = false;
}


void StatusBarImpl::set_inventory (const std::vector<std::string> &modelnames)
{
    if (m_text_active && m_interruptible) {
        hide_text();
    }

    ecl::delete_sequence(m_models.begin(), m_models.end());
    m_models.clear();

    for (size_t i=0; i<modelnames.size(); ++i) {
        m_models.push_back(MakeModel(modelnames[i]));
    }
    m_changedp = true;
}

void StatusBarImpl::show_text (const std::string &str, bool scrolling, double duration) 
{
    m_textview.set_text (str, scrolling, duration);
    m_interruptible = false;
    m_text_active = true;
    m_changedp = true;
}

void StatusBarImpl::tick (double dtime) {
    // Update text display
    if (m_text_active) {
        m_textview.tick (dtime);
        m_changedp = m_changedp || m_textview.has_changed();
        if (m_textview.has_finished()) {
            m_text_active = false;
            m_changedp = true;
        }
    }
}

void StatusBarImpl::new_world() {
    delete_sequence(m_models.begin(), m_models.end());
    m_models.clear();
    m_leveltime   = 0;
    m_text_active = false;
    m_changedp    = true;
}


/* -------------------- TextDisplay implementation -------------------- */

TextDisplay::TextDisplay (Font &f)
: area(), 
  text(), 
  changedp(false), finishedp(true),
  pingpong (false),
  showscroll(false),
  xoff(0), scrollspeed(200),
  textsurface(0), font(f)
{
    const video::VMInfo *vminfo = video::GetInfo();
    area = vminfo->sb_textarea;

    time = maxtime = 0;
}

void TextDisplay::set_text (const string &t, bool scrolling, double duration) 
{
    text = t;
    textsurface.reset(font.render(text.c_str()));
    

    time = 0;

    if (scrolling) {
        if (duration <= 0) {
            xoff = -area.w;
            scrollspeed = 160;
        } else {
            // Showscroll mode: first show string then scoll it out
            showscroll = true;
            scrollspeed = 0;
            if (area.w < textsurface->width()) {
                // start left adjusted for long strings
                xoff = 0;
            } else {
                // start centered for short strings
                xoff = -(area.w - textsurface->width())/2;
            }
        }
    }

    if (duration > 0)
        maxtime = duration;
    else
        maxtime = 1e20;       // "infinite" for all practical purposes

    if (!scrolling) {// centered text string
        if (area.w < textsurface->width()) {
            pingpong = true;
            scrollspeed = 4 * (textsurface->width() - area.w) / duration;
            xoff = 0;
        }
        else {
            pingpong = false;
            xoff = -(area.w - textsurface->width())/2;
            scrollspeed = 0;
        }
    }

    finishedp = false;
    changedp = true;
}

void TextDisplay::tick (double dtime) 
{
    time += dtime;
    if (time > maxtime) {
        if (showscroll) {
            showscroll = false;
            scrollspeed = 160;            
            maxtime = 1e20;       // "infinite" for all practical purposes
        } else {
            finishedp = true;
            changedp = true;
        }
    }
    else {
        int oldxoff = round_nearest<int>(xoff);
        xoff += dtime * scrollspeed;
        int newxoff = round_nearest<int> (xoff);
        changedp = newxoff != oldxoff;
        if (pingpong) {
            if (scrollspeed > 0 && area.w + newxoff >= textsurface->width() ) {
                scrollspeed = -scrollspeed;
            }
            else if (scrollspeed < 0 && newxoff <= 0) {
                scrollspeed = -scrollspeed;
            }
        }
        else if (xoff >= textsurface->width()) {
            finishedp = true;
            changedp = true;
        }
    }
}

void TextDisplay::draw (ecl::GC &gc, const ScreenArea &r) {
    clip(gc, intersect(area, r));
    set_color(gc, 0,0,0);
    box(gc, area);
    if (Surface *s = textsurface.get())
        blit(gc, area.x-round_nearest<int>(xoff), area.y, s);
}




//======================================================================
// DISPLAY ENGINE
//======================================================================

DisplayEngine::DisplayEngine (int tilew, int tileh)
: m_tilew(tilew), m_tileh(tileh),
  m_offset(), 
  m_new_offset(),
  m_area (),
  m_width(0), m_height(0),
  m_redrawp(0,0)
{
    m_area = video::GetScreen()->size();
    m_screenoffset[0] = m_screenoffset[1] = 0;
}

DisplayEngine::~DisplayEngine() 
{
    delete_sequence (m_layers.begin(), m_layers.end());
}

void DisplayEngine::set_tilesize (int w, int h) 
{ 
    m_tilew=w; 
    m_tileh=h; 
}


void DisplayEngine::add_layer (DisplayLayer *l) {
    l->set_engine (this);
    m_layers.push_back(l);
}

void DisplayEngine::set_offset (const V2 &off) {
    m_offset = m_new_offset = off;
    world_to_video (off, &m_screenoffset[0], &m_screenoffset[1]);
}

void DisplayEngine::move_offset (const ecl::V2 &off) {
    m_new_offset = off;
}

/*! Scroll the screen contents and mark the newly exposed regions for
  redraw.  This method assumes that the screen contents were not
  modified externally since the last call to update_offset(). */
void DisplayEngine::update_offset () 
{
    ecl::Screen *screen = video::GetScreen();

    int oldx = m_screenoffset[0];
    int oldy = m_screenoffset[1];
    int newx, newy;
    world_to_video (m_new_offset, &newx, &newy);

    if (newx != oldx || newy != oldy) {
        const Rect &a = get_area();
        Rect oldarea (a.x+oldx, a.y+oldy, a.w, a.h);
        Rect newarea (a.x+newx, a.y+newy, a.w, a.h);
        Rect common = intersect(newarea, oldarea);

        // Blit overlapping screen area from old to new position
        GC screengc (screen->get_surface());
        Rect blitrect (common.x-oldx, common.y-oldy, common.w, common.h);
        blit (screengc, common.x-newx, common.y-newy, screen->get_surface(), blitrect);
        blitrect.x = common.x-newx;
        blitrect.y = common.y-newy;
        screen->update_rect(blitrect);

        // Update offset
        set_offset (V2(newx/double(m_tilew), newy/double(m_tileh)));

        // Mark areas that could not be copied from old screen for redraw
        RectList rl;
        rl.push_back (get_area());
        rl.sub (blitrect);
        for (RectList::iterator i=rl.begin(); i!=rl.end(); ++i) {
            Rect r = screen_to_world(*i);
            mark_redraw_area (r);
        }
    }
}

void DisplayEngine::set_screen_area (const ecl::Rect & r) {
    m_area = r;
}

void DisplayEngine::new_world (int w, int h) 
{
    m_width = w;
    m_height = h;
    m_offset = m_new_offset = V2();
    m_screenoffset[0] = m_screenoffset[1] = 0;
    m_redrawp.resize(w, h, 1);

    for (unsigned i=0; i<m_layers.size(); ++i)
        m_layers[i]->new_world(w,h);
}


void DisplayEngine::tick (double dtime) {
    for_each (m_layers.begin(), m_layers.end(),
              bind2nd (mem_fun(&DisplayLayer::tick), dtime));
}

void DisplayEngine::world_to_screen (const V2 & pos, int *x, int *y) 
{
    *x = round_nearest<int>(pos[0]*m_tilew) - m_screenoffset[0] + get_area().x;
    *y = round_nearest<int>(pos[1]*m_tileh) - m_screenoffset[1] + get_area().y;
}


void DisplayEngine::world_to_video (const ecl::V2 &pos, int *x, int *y)
{
    *x = round_nearest<int>(pos[0]*m_tilew);
    *y = round_nearest<int>(pos[1]*m_tileh);
}

void DisplayEngine::video_to_screen (int x, int y, int *xx, int *yy)
{
    *xx = x - m_screenoffset[0] + get_area().x;
    *yy = y - m_screenoffset[1] + get_area().y;
}

/* Calculate the smallest rectangle `s' in world space aligned to
   tiles that contains a certain rectangle `r' in video space.  This
   function is used for calculating the region that needs to be
   updated when a sprite with extension `r' is moved on the screen. */
void DisplayEngine::video_to_world (const ecl::Rect &r, Rect &s) {
    dRect dr (r.x, r.y, r.w, r.h);
    s = round_grid (dr, get_tilew(), get_tileh());
}


ScreenArea DisplayEngine::world_to_screen (const WorldArea &a)
{
    int x, y;
    world_to_screen (V2(a.x, a.y), &x, &y);
    return ScreenArea (x, y, a.w*m_tilew, a.h*m_tileh);
}


WorldArea DisplayEngine::screen_to_world (const ScreenArea &a)
{
    int sx = m_screenoffset[0] + a.x - get_area().x;
    int sy = m_screenoffset[1] + a.y - get_area().y;

    int x1 = Max(0, sx / m_tilew);
    int y1 = Max(0, sy / m_tileh);
    int x2 = Min(m_width, (sx + a.w+m_tilew-1) / m_tilew);
    int y2 = Min(m_height, (sy + +a.h+m_tileh-1) / m_tileh);

    return WorldArea (x1, y1, x2-x1, y2-y1);
}


V2 DisplayEngine::to_world (const V2 &pos) {
    return m_offset + 
        V2((pos[0]-get_area().x)/m_tilew, (pos[1]-get_area().y)/m_tileh);
}

void DisplayEngine::mark_redraw_area (const WorldArea &wa, int delay) 
{
    int x2 = Min(m_width, wa.x+wa.w);
    int y2 = Min(m_height, wa.y+wa.h);
    for (int x=Max(0,wa.x); x <x2; x++)
        for (int y=Max(0,wa.y); y<y2; y++) {
            int d = m_redrawp(x, y);
            if (d == 0 || 1+delay < d)
                m_redrawp(x,y) = 1 + delay;
        }
}

void DisplayEngine::mark_redraw_screen() {
    mark_redraw_area(screen_to_world(m_area));
}

void DisplayEngine::draw_all (ecl::GC &gc) 
{
    WorldArea wa = screen_to_world (get_area());

    // Fill screen area not covered by world
    {
        RectList rl;
        rl.push_back (get_area());
        rl.sub (world_to_screen (WorldArea (0,0, m_width, m_height)));
        set_color (gc, 200, 0, 200);
        for (RectList::iterator i=rl.begin(); i!=rl.end(); ++i)
            box (gc, *i);
    }


    int xpos, ypos;
    world_to_screen (V2(wa.x, wa.y), &xpos, &ypos);
    for (unsigned i=0; i<m_layers.size(); ++i) {
        clip(gc, get_area());
        m_layers[i]->prepare_draw (wa);
        m_layers[i]->draw (gc, wa, xpos, ypos);
        m_layers[i]->draw_onepass(gc);
    }
}

void DisplayEngine::update_layer (DisplayLayer *l, WorldArea wa) 
{
    GC gc(video::GetScreen()->get_surface());

    int x2 = wa.x+wa.w;
    int y2 = wa.y+wa.h;

    clip(gc, get_area());
    int xpos0, ypos;
    world_to_screen (V2(wa.x, wa.y), &xpos0, &ypos);

    l->prepare_draw (wa);
    for (int y=wa.y; y<y2; y++, ypos += m_tileh) {
        int xpos = xpos0;
        for (int x=wa.x; x<x2; x++, xpos += m_tilew) {
            if (m_redrawp(x,y) == 1)
                l->draw (gc, WorldArea(x,y,1,1), xpos, ypos);
        }
    }
    l->draw_onepass (gc);
}

void DisplayEngine::update_screen() 
{
    ecl::Screen *screen = video::GetScreen();
    GC gc(screen->get_surface());

    if (m_new_offset != m_offset) {
        update_offset ();
        m_new_offset = m_offset;
    }

    Rect area=get_area();
    clip(gc, area);

    WorldArea wa = screen_to_world (area);
    for (unsigned i=0; i<m_layers.size(); ++i) {
        update_layer (m_layers[i], wa);
    }
    int x2 = wa.x+wa.w;
    int y2 = wa.y+wa.h;
    for (int x=wa.x; x<x2; x++)
        for (int y=wa.y; y<y2; y++)
            if (m_redrawp(x,y) >= 1) {
                if ((m_redrawp(x,y) -= 1) == 0)
                    screen->update_rect (world_to_screen (WorldArea (x, y, 1, 1)));
            } 
}


/* -------------------- ModelLayer -------------------- */

void ModelLayer::maybe_redraw_model(Model *m, bool immediately) 
{
    Rect videoarea;
    if (m->has_changed(videoarea)) {
        int delay = immediately ? 0 : enigma::IntegerRand (0, 2);
        WorldArea wa;
        get_engine()->video_to_world (videoarea, wa);
        get_engine()->mark_redraw_area(wa, delay);
    }
}

void ModelLayer::activate (Model *m) 
{
    list<Model*> &am = m_active_models_new;
    am.push_back(m);
}

void ModelLayer::deactivate (Model *m)
{
    list<Model*> &am = m_active_models;
    list<Model*>::iterator i = find(am.begin(), am.end(), m);
    if (i == am.end()) {
        m_active_models_new.remove(m);
    } else {
        *i = 0;
    }
}

void ModelLayer::new_world (int, int) {
    m_active_models.clear();
    m_active_models_new.clear();
}

void ModelLayer::tick (double dtime) 
{
    ModelList &am = m_active_models;

    am.remove(static_cast<Model*> (0));
    am.remove_if(mem_fun(&Model::is_garbage));

    // Append new active models to list
    am.splice (am.end(), m_active_models_new);

    /* for_each does not work; animation may remove itself during a
       tick.  This may happen for example when a model callback
       decides to replace the old model by another one.  */
    for (ModelList::iterator i=am.begin(); i!=am.end(); ++i) {
        if (Model *m = *i) {
            m->tick(dtime);

            // We have to check (*i) again because the list of active
            // models can change during a tick!
            if ((m = *i))
                maybe_redraw_model (m);
        }
    }
}


/* -------------------- GridLayer -------------------- */

DL_Grid::DL_Grid(int redrawsize)
: m_models (0, 0), m_redrawsize (redrawsize)
{
}

DL_Grid::~DL_Grid() {
    delete_sequence (m_models.begin(), m_models.end());
}

void DL_Grid::new_world (int w, int h) {
    ModelLayer::new_world (w, h);
    delete_sequence (m_models.begin(), m_models.end());
    m_models.resize (w, h, 0);
}

void DL_Grid::mark_redraw (int x, int y) {
    get_engine()->mark_redraw_area (WorldArea (x, y, m_redrawsize, m_redrawsize));
}

void DL_Grid::set_model (int x, int y, Model *m) {
    if (!(x >= 0 && y >= 0 &&
          (unsigned)x<m_models.width() &&
          (unsigned)y<m_models.height()))
    {
        delete m;	// model is owned by DL_Grid!
        return;
    }

    if (m_models(x,y) != m) {
        if (Model *oldm = m_models(x,y)) {
            oldm->remove(this);
            delete oldm;
        }
        mark_redraw (x, y);
        m_models(x,y) = m;
        if (m) {
            int vx, vy;
            get_engine()->world_to_video (V2 (x, y), &vx, &vy);
            m->expose (this, vx, vy);
        }
    }
}

Model * DL_Grid::get_model (int x, int y) {
    return m_models(x,y);
}

Model *DL_Grid::yield_model (int x, int y) {
    Model *m = get_model (x, y);
    if (m)
        m->remove (this);
    m_models(x,y) = 0;
    mark_redraw (x,y);
    return m;
}


void DL_Grid::draw (ecl::GC &gc, const WorldArea &a, int destx, int desty) {
    int x2 = a.x+a.w;
    int y2 = a.y+a.h;
    int tilew = get_engine()->get_tilew();
    int tileh = get_engine()->get_tileh();
    int xpos = destx;
    for (int x=a.x; x<x2; ++x) {
        int ypos = desty;
        for (int y=a.y; y<y2; ++y) {
            if (Model *m = m_models(x,y))
                m->draw(gc, xpos, ypos);
            ypos += tileh;
        }
        xpos += tilew;
    }
}


/* -------------------- Sprites -------------------- */

SpriteHandle::SpriteHandle (DL_Sprites *l, unsigned spriteid)
: layer(l), id(spriteid)
{}

SpriteHandle::SpriteHandle() 
: layer(0)
{
    id = DL_Sprites::MAGIC_SPRITEID;
}

void SpriteHandle::kill() {
    if (layer) {
	layer->kill_sprite (id);
	layer = 0;
	id = DL_Sprites::MAGIC_SPRITEID;
    }
}

void SpriteHandle::move (const ecl::V2 &newpos) const {
    if (layer)
        layer->move_sprite (id, newpos);
}

void SpriteHandle::replace_model (Model *m) const {
    if (layer)
        layer->replace_sprite (id, m);
    else
        delete m;
}

Model *SpriteHandle::get_model () const {
    return layer ? layer->get_model (id) : 0;
}

void SpriteHandle::set_callback (ModelCallback *cb) const {
    if (Model *m = get_model())
        m->set_callback(cb);
}

void SpriteHandle::hide() const {
    if (layer) {
        Sprite * s = layer->get_sprite(id);
        if(s->visible) {
            s->visible = false;
            layer->redraw_sprite_region(id);
        }
    }
}

void SpriteHandle::show() const {
    if (layer) {
        Sprite * s = layer->get_sprite(id);
        if(!s->visible) {
            s->visible = true;
            layer->redraw_sprite_region(id);
        }
    }
}


/* -------------------- Sprite layer -------------------- */

DL_Sprites::DL_Sprites()
: numsprites(0), maxsprites(1000)
{}

DL_Sprites::~DL_Sprites() {
    delete_sequence(sprites.begin(), sprites.end());
}

Sprite *DL_Sprites::get_sprite(SpriteId id) {
    return (id == MAGIC_SPRITEID) ? 0 : sprites[id];
}

void DL_Sprites::new_world (int w, int h) {
    ModelLayer::new_world (w,h);
    delete_sequence (sprites.begin(), sprites.end());
    sprites.clear();
    numsprites = 0;
}

void DL_Sprites::move_sprite (SpriteId id, const ecl::V2& newpos) 
{
    Sprite *sprite = sprites[id];

    int newx, newy;
    get_engine()->world_to_video (newpos, &newx, &newy);

    if (newx != sprite->screenpos[0] || newy != sprite->screenpos[1] ||
            sprite->mayNeedRedraw ) {
        redraw_sprite_region(id); // make sure old sprite is removed
        sprite->pos = newpos;
        sprite->screenpos[0] = newx;
        sprite->screenpos[1] = newy;
        if (Anim2d* anim = dynamic_cast<Anim2d*>(sprite->model))
            anim->move (newx, newy);
        redraw_sprite_region(id); // draw new sprite
    }
}

SpriteId DL_Sprites::add_sprite (Sprite *sprite) 
{
    if (numsprites >= maxsprites) {
        delete sprite;
        return MAGIC_SPRITEID;
    }

    SpriteList &sl = sprites;
    SpriteId id = 0;

    // Find the first empty slot
    SpriteList::iterator i = find(sl.begin(), sl.end(), static_cast<Sprite*>(0));
    if (i == sl.end()) {
        id = sl.size();
        sl.push_back(sprite);
    }
    else {
        id = distance(sl.begin(), i);
        *i = sprite;
    }
    get_engine()->world_to_video (sprite->pos, &sprite->screenpos[0], &sprite->screenpos[1]);
    if (Model *m = sprite->model)
        m->expose (this, sprite->screenpos[0], sprite->screenpos[1]);
    redraw_sprite_region(id);
    numsprites += 1;
    return id;
}

void DL_Sprites::replace_sprite (SpriteId id, Model *m) {
    Sprite *sprite = sprites[id];
    if (Model *old = sprite->model) {
        redraw_sprite_region(id);
        old->remove (this);
        delete old;
    }
    sprite->model = m;
    if (m) {
        m->expose (this, sprite->screenpos[0], sprite->screenpos[1]);
        redraw_sprite_region(id);
    }
}

void DL_Sprites::kill_sprite (SpriteId id) {
    if (Sprite *sprite = sprites[id]) {
        redraw_sprite_region(id);
        if (Model *m = sprite->model) {
            m->remove (this);
        }
        sprites[id] = 0;
        numsprites -= 1;
        delete sprite;
    }
}

void DL_Sprites::draw (ecl::GC &gc, const WorldArea &a, int /*x*/, int /*y*/)
{
    DisplayEngine *engine = get_engine();
    clip (gc, intersect (engine->get_area(), engine->world_to_screen(a)));
    draw_sprites (false, gc);
}


void DL_Sprites::draw_sprites (bool drawshadowp, GC &gc) {
    SpriteList &sl = sprites;

    for (unsigned i=0; i<sl.size(); ++i) {
        Sprite *s = sl[i];
        if (s && s->model && s->visible) {
            int sx, sy;
            get_engine()->world_to_screen(s->pos, &sx, &sy);
            if (drawshadowp)
                s->model->draw_shadow(gc, sx, sy);
            else
                s->model->draw(gc, sx, sy);
        }
    }
}

void DL_Sprites::draw_onepass (ecl::GC &gc)
{
//     draw_sprites (false, gc);
}

void DL_Sprites::redraw_sprite_region (SpriteId id) 
{
    Sprite *s = sprites[id];
    if (s && s->model) {
        Rect r, redrawr;
        s->model->get_extension (r);
        r.x += s->screenpos[0];
        r.y += s->screenpos[1];
        DisplayEngine *e = get_engine();
        e->video_to_world (r, redrawr);
        e->mark_redraw_area (redrawr);
    }
}

void DL_Sprites::tick (double dtime)
{
    SpriteList &sl = sprites;
    for (unsigned i=0; i<sl.size(); ++i) {
        Sprite *s = sl[i];
        if (!s || !s->model)
            continue;

        if (s->model->is_garbage()  && s->layer==SPRITE_EFFECT) {
            // Only remove effect sprites -- actor sprites remain in
            // the world all the time
            kill_sprite (i);
        }
    }
    ModelLayer::tick (dtime);
}



//----------------------------------------------------------------------
// RUBBER BANDS
//----------------------------------------------------------------------

void DL_Lines::draw_onepass (ecl::GC &gc)
{
    DisplayEngine *engine = get_engine();

    set_color (gc, 240, 140, 20, 255);
    set_flags (gc.flags, GS_ANTIALIAS);

    for (LineMap::iterator i=m_rubbers.begin(); i!= m_rubbers.end(); ++i)
    {
        int x1, y1, x2, y2;
        engine->world_to_screen (i->second.start, &x1, &y1);
        engine->world_to_screen (i->second.end, &x2, &y2);

        line (gc, x1, y1, x2, y2);
    }
}

/* Mark the screen region occupied by a rubber band for redraw.
   Problem is: what region is that exactly?  What pixels on the screen
   will the line rasterizer touch?  Hard to tell, especially when
   anti-aliasing is used.

   This function constructs a list of rectangles that completely
   enclose the line by subdividing the line into n segments and
   constructing the bounding box for each of these segments.  To
   account for the (effective) finite width of the line, these boxes
   need to be enlarged by a small amount to make them overlap a bit.

   The number n of subdivision depends on the length of the line.  n=1
   would of course do, but we want to redraw as little of the screen
   as possible.  `n' is therefore chosen in such a way that the line
   is covered with boxes of size not larger than `maxboxsize'.
*/
void DL_Lines::mark_redraw_line (const Line &r) {
    const double maxboxsize = 0.5;

    double w0 = r.start[0]-r.end[0];
    double h0 = r.start[1]-r.end[1];
    int n = int (max(abs(w0),abs(h0)) / maxboxsize)+1;

    double w = w0/n;
    double h = h0/n;

    double overlap = 0.1;

    double x = r.end[0];
    double y = r.end[1];

    double xoverlap = w<0 ? -overlap : overlap;
    double yoverlap = h<0 ? -overlap : overlap;

    for (int i=0; i<n; ++i) {
        dRect dr (x-xoverlap, y-yoverlap, w+2*xoverlap, h+2*yoverlap);
        WorldArea wa = round_grid (dr, 1, 1);

        if (wa.w < 0) {
            wa.x += wa.w;
            wa.w = -wa.w;
        }
        if (wa.h < 0) {
            wa.y += wa.h;
            wa.h = -wa.h;
        }
        wa.w++;
        wa.h++;

        get_engine()->mark_redraw_area (wa);

        x += w;
        y += h;
    }
}

RubberHandle
DL_Lines::add_line (const V2 &p1, const V2 &p2)
{
    m_rubbers[m_id] = Line(p1, p2);
    mark_redraw_line (m_rubbers[m_id]);
    return RubberHandle(this, m_id++);
}

void DL_Lines::set_startpoint (unsigned id, const V2 &p1)
{
    mark_redraw_line (m_rubbers[id]);
    m_rubbers[id].start = p1;
    mark_redraw_line (m_rubbers[id]);
}

void DL_Lines::set_endpoint (unsigned id, const V2 &p2)
{
    mark_redraw_line (m_rubbers[id]);
    m_rubbers[id].end = p2;
    mark_redraw_line (m_rubbers[id]);
}

void DL_Lines::kill_line (unsigned id) {
    mark_redraw_line (m_rubbers[id]);
    LineMap::iterator i=m_rubbers.find(id);
    if (i != m_rubbers.end())
        m_rubbers.erase(i);
}

RubberHandle::RubberHandle(DL_Lines *ll, unsigned id_)
: line_layer (ll), id(id_)
{
}

void RubberHandle::update_first(const V2 &p1)
{
    line_layer->set_startpoint (id, p1);
}

void RubberHandle::update_second(const V2 &p2)
{
    line_layer->set_endpoint (id, p2);
}

void RubberHandle::kill()
{
    line_layer->kill_line(id);
}


//----------------------------------------------------------------------
// SHADOWS
//----------------------------------------------------------------------

/*
** Drawing the shadows is a lot more difficult than drawing any of the
** other layers.  There are a couple of reasons for this:
**
** 1. Both Stones and actors cast a shadow.  Not a real problem, but
**    it makes the implementation more complex.
**
** 2. Shadows can overlap.  Not only can the shadows of stones and
**    actors overlap, but also the shadows of two adjacent stones can.
**    Since we are using alpha blending for the shadows, this means
**    that we cannot blit the invidual shadows to the screen, but we
**    have to use an intermediate buffer.
**
** 3. Performance is critical.  Drawing the shadows is time-consuming,
**    firstly because alpha blending is costly and secondly because of
**    the intermediate buffer.  So we should try to cache shadows *and*
**    avoid the buffer if possible.
**
** So, how do we approach these problems? We handle stone and actor
** shadows separately: The stone shadows do not change very often so
** it's easy to cache them, one tile at a time.  If there is no actor
** on this tile, we can blit the cached image directly to the screen,
** otherwise we have no choice but to use the buffer.
**
** The remaining problem is the shadow cache.  The easiest solution
** would be to use one huge image for the whole level and keep it in
** memory all the time.  This would consume roughly 20mb for a 100x100
** landscape, which is of course excessive, considering that there are
** rarely more than 40 different shadow tiles in each landscape.
**
** Instead, Enigma caches the most recently calculated shadow tiles in
** a linked list.  (If this should one day turn out to be too slow,
** it's still possible to resort to a hash table or something
** similar.)
*/

namespace display
{
    struct ImageQuad {
        Image *images[4];

        ImageQuad() { /* do not initialize fields. */ }

        ImageQuad (Image *i1, Image *i2, Image *i3, Image *i4) {
            images[0] = i1;
            images[1] = i2;
            images[2] = i3;
            images[3] = i4;
        }
        bool operator == (const ImageQuad &q) {
            return (images[0]==q.images[0] &&
                    images[1]==q.images[1] &&
                    images[2]==q.images[2] &&
                    images[3]==q.images[3]);
        }
        Image *operator[] (int idx) { return images[idx]; }
    };

    bool only_static_shadows (Model *models[4], ImageQuad &q) {
        int nimages=4;

        for (int i=0; i<4; ++i) {
            if (models[i] == 0) {
                // No model at all? -> static
                q.images[i] = 0;
	    }
            else if (Model *shadow = models[i]->get_shadow()) {
                if (ImageModel *im = dynamic_cast<ImageModel*>(shadow))
                    // We have a model with a static image shadow
                    q.images[i] = im->get_image();
                else
                    q.images[i] = 0, nimages--;
            }
            else
                q.images[i] = 0;
        }
        return nimages==4;
    }


    struct StoneShadow {
        ImageQuad  images;
        Surface   *image;
        bool       in_cache;

        StoneShadow (ImageQuad iq, bool cached)
        : images(iq), image(0), in_cache(cached)
        {}
    };

    class StoneShadowCache : public ecl::Nocopy {
    public:
        StoneShadowCache(int tilew, int tileh);
        ~StoneShadowCache();

        StoneShadow *retrieve (Model *models[4]);
        void release (StoneShadow *s);
        void clear();
    private:
        typedef std::list<StoneShadow*> CacheList;

        // Variables
        size_t            m_max_size; // Max. number of different shadow tiles to cache
        CacheList         m_cache;
        int               m_tilew, m_tileh;
        vector<Surface *> m_surface_avail;

        // Private methods.
        Surface *new_surface ();
        StoneShadow *find_in_cache (const ImageQuad &images);

        void fill_image (StoneShadow *s);
        void fill_image (StoneShadow *sh, Model *models[4]);
    };
}

StoneShadowCache::StoneShadowCache(int tilew, int tileh)
: m_max_size(50), m_cache()
{
    m_tilew=tilew; m_tileh=tileh;
}

StoneShadowCache::~StoneShadowCache()
{
    clear();
}

void StoneShadowCache::clear() {
    for (CacheList::iterator i = m_cache.begin(); i!=m_cache.end(); ++i)
        delete (*i)->image;
    delete_sequence (m_cache.begin(), m_cache.end());
    m_cache.clear();
    delete_sequence (m_surface_avail.begin(), m_surface_avail.end());
    m_surface_avail.clear();
}

void StoneShadowCache::fill_image (StoneShadow *sh) {
    // Special case: no shadows at all:
    if (sh->images[0] == 0 && sh->images[1] == 0 &&
        sh->images[2] == 0 && sh->images[3] == 0)
    {
        sh->image = 0;
        return;
    }

    Surface *s = new_surface();
    GC gc(s);
    set_color (gc, 255,255,255);
    box(gc, s->size());

    if (Image *i = sh->images[0])
        draw_image (i, gc, -m_tilew, -m_tileh);
    if (Image *i = sh->images[1])
        draw_image (i, gc, 0, -m_tileh);
    if (Image *i = sh->images[2])
        draw_image (i, gc, -m_tilew, 0);
    if (Image *i = sh->images[3])
        draw_image (i, gc, 0, 0);

    SDL_Surface *ss = s->get_surface();
    SDL_SetColorKey(ss, SDL_SRCCOLORKEY | SDL_RLEACCEL,
                    SDL_MapRGB(ss->format, 255,255,255));
    SDL_SetAlpha (ss, SDL_SRCALPHA | SDL_RLEACCEL, 128);

    sh->image = s;
}

void StoneShadowCache::fill_image (StoneShadow *sh, Model *models[4]) {
    Surface *s = new_surface();
    GC gc(s);
    set_color (gc, 255,255,255);
    box(gc, s->size());
    if (models[0]) models[0]->draw_shadow (gc, -m_tilew, -m_tileh);
    if (models[1]) models[1]->draw_shadow (gc, 0, -m_tileh);
    if (models[2]) models[2]->draw_shadow (gc, -m_tilew, 0);
    if (models[3]) models[3]->draw_shadow (gc, 0,0);
    SDL_Surface *ss = s->get_surface();
    SDL_SetColorKey(ss, SDL_SRCCOLORKEY | SDL_RLEACCEL,
                    SDL_MapRGB(ss->format, 255,255,255));
    SDL_SetAlpha (ss, SDL_SRCALPHA | SDL_RLEACCEL, 128);
    sh->image = s;
}


StoneShadow *
StoneShadowCache::find_in_cache (const ImageQuad &images)
{
    CacheList::iterator i=m_cache.begin();
    for (; i!=m_cache.end(); ++i) {
        if ((*i)->images == images) {
            StoneShadow *sh = *i;
            // Move entry to front of list
            m_cache.splice (m_cache.begin(), m_cache, i);
            return sh;
        }
    }
    return 0;
}

/* Try to lookup the shadow created by the four models in `models[]'
   in the shadow cache. */
StoneShadow *
StoneShadowCache::retrieve (Model *models[4])
{
    StoneShadow *shadow = 0;

    ImageQuad images;

    // Only cache static stone shadows, i.e., those consisting
    // only of Image models.
    if (only_static_shadows (models, images)) {
        shadow = find_in_cache(images);
        if (!shadow) {
            shadow = new StoneShadow (images, true);
            fill_image (shadow);
            m_cache.push_front (shadow);
        }
    }
    else {
        shadow = new StoneShadow (images, false);
        fill_image (shadow, models);
    }
    return shadow;
}

void StoneShadowCache::release (StoneShadow *s) {
    if (s->in_cache) {
        // Image is in cache, no need to free anything
    }
    else {
        m_surface_avail.push_back(s->image);
        delete s;
    }
}

Surface *StoneShadowCache::new_surface () {
    Surface *s = 0;
    if (m_surface_avail.empty()) {
        // WARNING: Always make sure the surface format here matches
        // the format of `buffer' in class DL_Shadows!!!
        SDL_Surface *ss = SDL_CreateRGBSurface(SDL_SWSURFACE,
                                               m_tilew, m_tileh, 32,
                                               0,0,0,0);
        s = Surface::make_surface(ss);
    } else {
        s = m_surface_avail.back();
        m_surface_avail.pop_back();
    }
    return s;
}


/* -------------------- Shadow layer -------------------- */

DL_Shadows::DL_Shadows (DL_Grid *grid, DL_Sprites *sprites)
: m_grid(grid), m_sprites(sprites), m_cache(0), buffer(0),
  m_hasactor (0,0)
{
}

DL_Shadows::~DL_Shadows() {
    delete m_cache;
    delete buffer;
}

void DL_Shadows::new_world(int w, int h) 
{
    m_hasactor.resize (w, h, false);

    DisplayEngine *e = get_engine();
    int tilew=e->get_tilew();
    int tileh=e->get_tileh();

    delete m_cache;
    m_cache = new StoneShadowCache(tilew, tileh);

    delete buffer;
    // WARNING: Always make sure the surface format here matches
    // the format in `StoneShadowCache::new_surface' !!!
    SDL_Surface *ss = SDL_CreateRGBSurface(SDL_SWSURFACE,
                                           tilew, tileh, 32,
                                           0,0,0,0);
    SDL_SetAlpha(ss, SDL_SRCALPHA, 128);
    SDL_SetColorKey(ss, SDL_SRCCOLORKEY ,
                    SDL_MapRGB(ss->format, 255,255,255));

    buffer = Surface::make_surface(ss);
}

void DL_Shadows::draw (ecl::GC &gc, const WorldArea &a, int destx, int desty) {
    int x2 = a.x+a.w;
    int y2 = a.y+a.h;
    int tilew = get_engine()->get_tilew();
    int tileh = get_engine()->get_tileh();
    int xpos = destx;
    for (int x=a.x; x<x2; ++x) {
        int ypos = desty;
        for (int y=a.y; y<y2; ++y) {
            draw (gc, xpos, ypos, x, y);
            ypos += tileh;
        }
        xpos += tilew;
    }
}

bool DL_Shadows::has_actor (int x, int y) 
{
    return m_hasactor(x, y);
}

/** Prepare the shadows layer for a redraw.  This routine
    pre-calculates the tiles that currently are partially covered by
    an actor.  */
void DL_Shadows::prepare_draw (const WorldArea &wa) 
{
    for (int i=0; i<wa.w; ++i) 
        for (int j=0; j<wa.h; ++j)
            m_hasactor(wa.x + i, wa.y + j) = false;

    for (unsigned k=0; k<m_sprites->sprites.size(); ++k) {
        Sprite *s = m_sprites->sprites[k];
        if (s && s->layer == SPRITE_ACTOR && s->model) {
            Rect r, redrawr;
            s->model->get_extension (r);
            r.x += s->screenpos[0];
            r.y += s->screenpos[1];
            DisplayEngine *e = get_engine();
            e->video_to_world (r, redrawr);
            redrawr.intersect (wa);

            for (int i=0; i<redrawr.w; ++i) 
                for (int j=0; j<redrawr.h; ++j)
                    m_hasactor(redrawr.x + i, redrawr.y + j) = true;
        }
    }
}

Model *DL_Shadows::get_shadow_model(int x, int y) {
    if (x >= 0 && y >= 0) {
        if (Model *m = m_grid->get_model(x,y))
            return m; //return m->get_shadow();
    }
    return 0;
}

void DL_Shadows::draw(GC &gc, int xpos, int ypos, int x, int y) {
    Model *models[4];
    models[0] = get_shadow_model (x-1, y-1);
    models[1] = get_shadow_model (x, y-1);
    models[2] = get_shadow_model (x-1, y);
    models[3] = get_shadow_model (x, y);

    StoneShadow *sh = m_cache->retrieve (models);

    int tilew = get_engine()->get_tilew();
    int tileh = get_engine()->get_tileh();

    bool hasActor = this->has_actor (x, y);
    if (hasActor || sh->image) {
        Surface *s = sh->image;
        if (hasActor) {
            GC gc2(buffer);
            if (s) {
                s->lock();
                buffer->lock();
                SDL_Surface *ss = s->get_surface();
                SDL_Surface *bs = buffer->get_surface();
                memcpy (bs->pixels, ss->pixels, ss->w * ss->h * ss->format->BytesPerPixel);
                buffer->unlock();
                s->unlock();
            }
            else {
                set_color (gc2, 255, 255, 255);
                box (gc2, buffer->size());
            }
            for (unsigned i=0; i<m_sprites->sprites.size(); ++i) {
                if (Sprite *sp = m_sprites->sprites[i]) {
                    if (sp->visible && sp->model) {
                        int sx = round_nearest<int>(sp->pos[0]*tilew) - x*tilew;
                        int sy = round_nearest<int>(sp->pos[1]*tileh) - y*tileh;
                        sp->model->draw_shadow(gc2, sx, sy);
                    }
                }
            }
            blit(gc, xpos, ypos, buffer);
        }
        else {
            blit (gc, xpos,ypos,s);
        }
    }

    m_cache->release (sh);
}


//----------------------------------------------------------------------
// Sprite following code
//----------------------------------------------------------------------

Follower::Follower (DisplayEngine *e)
: m_boundary (0.5),
  m_engine(e)
{}

double Follower::get_hoff() const 
{ 
    ScreenArea gamearea = m_engine->get_area();
    return gamearea.w / m_engine->get_tilew() -m_boundary*2; 
}

double Follower::get_voff() const 
{
    ScreenArea gamearea = m_engine->get_area();
    return gamearea.h / m_engine->get_tileh() -m_boundary*2;
}

void Follower::center(const ecl::V2 &point) 
{
    double borderh = m_boundary; 
    double borderv = m_boundary;
    double hoff = get_hoff();
    double voff = get_voff();

    V2 off = point;
    off[0] = floor((off[0] - borderh) / hoff) * hoff;
    off[1] = floor((off[1] - borderv) / voff) * voff;

    set_offset(off);
}

bool Follower::set_offset (V2 offs) 
{
    DisplayEngine *e = get_engine();
    offs[0] = max (offs[0], 0.0);
    offs[1] = max (offs[1], 0.0);
    offs[0] = min (offs[0], double(e->get_width()-get_hoff()-1));
    offs[1] = min (offs[1], double(e->get_height()-get_voff()-1));
    if (offs != e->get_offset()) {
	e->set_offset(offs);
	return true;
    }
    return false;
}

/* -------------------- Follower_Screen -------------------- */

Follower_Screen::Follower_Screen(DisplayEngine *e)
: Follower(e)
{}


/*! Determine whether the screen must be scrolled or not, and change
  the coordinate origin of the screen accordingly. */
void Follower_Screen::tick(double, const ecl::V2 &point) {
    DisplayEngine *engine = get_engine();
    V2 oldoff = engine->get_offset();
    Follower::center(point);
    if (oldoff != engine->get_offset())
        engine->mark_redraw_screen();
}



/* -------------------- Follower_Scrolling -------------------- */

Follower_Scrolling::Follower_Scrolling(DisplayEngine *e, bool screenwise_)
: Follower (e),
  currently_scrolling(false),
  scrollspeed(0), resttime(0),
  screenwise (screenwise_)
{}

void Follower_Scrolling::center(const ecl::V2 &point) 
{
    Follower::center(point);
    curpos = destpos = get_engine()->get_offset();
}

void Follower_Scrolling::tick(double dtime, const ecl::V2 &point) 
{
    DisplayEngine *engine   = get_engine();

    if (!currently_scrolling) {
        ScreenArea gamearea = engine->get_area();
        int        tilew    = engine->get_tilew();
        int        tileh    = engine->get_tileh();
        int        borderx  = tilew/2;
        int        bordery  = tileh/2;

        int sx, sy;
        engine->world_to_screen(point, &sx, &sy);

        bool scrollx_p = (sx < gamearea.x + borderx)
            || (sx >= gamearea.x + gamearea.w - borderx);

        bool scrolly_p = (sy < gamearea.y + bordery)
            || (sy >= gamearea.y + gamearea.h - bordery);

        if (scrollx_p || scrolly_p) {
            V2 olddest = destpos;
            V2 scrollpos = engine->get_offset();

            currently_scrolling = true;

            // Move `point' to center of the screen
            curpos = scrollpos;

            if (screenwise) {
                double hoff = get_hoff();
                double voff = get_voff();
                destpos[0] = floor((point[0]-m_boundary) / hoff) * hoff;
                destpos[1] = floor((point[1]-m_boundary) / voff) * voff;
            } else {
                destpos = point - V2(gamearea.w/tilew, gamearea.h/tileh)/2;
            }

            // Round to integer pixel offset
            destpos[0] = round_nearest<int>(destpos[0]*tilew)/tilew;
            destpos[1] = round_nearest<int>(destpos[1]*tileh)/tileh;

            // Don't scroll off the game area
            destpos[0] = Clamp (destpos[0], 0.0,
                                (double)engine->get_width()-gamearea.w/tilew);
            destpos[1] = Clamp (destpos[1], 0.0, 
                                (double)engine->get_height()-gamearea.h/tileh);
            if (!scrollx_p)
                destpos[0] = olddest[0];
            if (!scrolly_p)
                destpos[1] = olddest[1];
        }
    }

    if (currently_scrolling) {
        scrollspeed = 45.0;
        resttime = length(destpos - curpos)/scrollspeed;

        resttime -= dtime;
        if (resttime <= 0) {
            engine->move_offset (destpos);
            currently_scrolling = false;
        } else {
            dir = normalize(destpos - curpos);
            curpos += dir * scrollspeed*dtime;
            engine->move_offset (curpos);
        }
    }
}

/* -------------------- Follower_Smooth -------------------- */

Follower_Smooth::Follower_Smooth (DisplayEngine *e)
: Follower (e)
{
}

ecl::V2 Follower_Smooth::calc_offset (const ecl::V2 &point)
{
    DisplayEngine *engine   = get_engine();
    ScreenArea     gamearea = engine->get_area();
    int            tilew    = engine->get_tilew();
    int            tileh    = engine->get_tileh();

    V2 destpos = point - V2(double(gamearea.w)/tilew, double(gamearea.h)/tileh)/2;
    // Round to integer pixel offset
    destpos[0] = round_nearest<int>(destpos[0]*tilew)/double(tilew);
    destpos[1] = round_nearest<int>(destpos[1]*tileh)/double(tileh);
    destpos[0] = Clamp (destpos[0], 0.0,
                        (double)engine->get_width()-gamearea.w/tilew);
    destpos[1] = Clamp (destpos[1], 0.0, 
                        (double)engine->get_height()-gamearea.h/tileh);
    return destpos;
}

void Follower_Smooth::tick (double /*time*/, const ecl::V2 &point)
{
    DisplayEngine *engine   = get_engine();
    engine->move_offset (calc_offset (point));
}

void Follower_Smooth::center (const ecl::V2 &point)
{
    set_offset(calc_offset (point));
}


//----------------------------------------------------------------------
// Editor / game display engine
//----------------------------------------------------------------------

CommonDisplay::CommonDisplay (const ScreenArea &a)
{
    m_engine = new DisplayEngine;
    m_engine->set_screen_area (a);

    const video::VMInfo *vminfo = video::GetInfo();
    m_engine->set_tilesize (vminfo->tile_size, vminfo->tile_size);

    // Create and configure display layers
    floor_layer   = new DL_Grid;
    item_layer    = new DL_Grid;
    sprite_layer  = new DL_Sprites;
    stone_layer   = new DL_Grid (2);
    shadow_layer  = new DL_Shadows(stone_layer, sprite_layer);
    line_layer    = new DL_Lines;
    effects_layer = new DL_Sprites;
    effects_layer->set_maxsprites(50);

    // Register display layers
    m_engine->add_layer (floor_layer);
    m_engine->add_layer (item_layer);
    m_engine->add_layer (shadow_layer);
    m_engine->add_layer (sprite_layer);
    m_engine->add_layer (stone_layer);
    m_engine->add_layer (line_layer);
    m_engine->add_layer (effects_layer);
}

CommonDisplay::~CommonDisplay()
{
    delete m_engine;
}

Model * CommonDisplay::set_model (const GridLoc &l, Model *m)
{
    int x = l.pos.x, y=l.pos.y;

    switch (l.layer) {
    case GRID_FLOOR: floor_layer->set_model (x, y, m); break;
    case GRID_ITEMS: item_layer->set_model (x, y, m); break;
    case GRID_STONES:
        stone_layer->set_model (x, y, m);
//        shadow_layer->set_model (x, y, m);
//         shadow_layer->update (x, y);
        break;
    case GRID_COUNT: break;
    }
    return m;
}

Model *
CommonDisplay::get_model (const GridLoc &l)
{
    int x = l.pos.x, y=l.pos.y;
    switch (l.layer) {
    case GRID_FLOOR: return floor_layer->get_model (x, y);
    case GRID_ITEMS: return item_layer->get_model (x, y);
    case GRID_STONES: return stone_layer->get_model (x, y);
    case GRID_COUNT: return 0;
    }
    return 0;
}

Model *
CommonDisplay::yield_model (const GridLoc &l)
{
    int x = l.pos.x, y=l.pos.y;
    switch (l.layer) {
    case GRID_FLOOR: return floor_layer->yield_model (x, y);
    case GRID_ITEMS: return item_layer->yield_model (x, y);
    case GRID_STONES: return stone_layer->yield_model (x, y);
    case GRID_COUNT: return 0;
    }
    return 0;
}


RubberHandle
CommonDisplay::add_line (V2 p1, V2 p2)
{
    return line_layer->add_line (p1, p2);
}

SpriteHandle
CommonDisplay::add_effect (const V2& pos, Model *m)
{
    Sprite *spr = new Sprite (pos, SPRITE_EFFECT, m);
    return SpriteHandle (effects_layer, effects_layer->add_sprite(spr));
}

SpriteHandle
CommonDisplay::add_sprite (const V2 &pos, Model *m)
{
    Sprite *spr = new Sprite (pos, SPRITE_ACTOR, m);
    return SpriteHandle (sprite_layer, sprite_layer->add_sprite(spr));
}

void CommonDisplay::new_world (int w, int h) {
    get_engine()->new_world (w, h);
}

void CommonDisplay::redraw() {
    get_engine()->update_screen();
}

void CommonDisplay::set_floor (int x, int y, Model *m) {
    floor_layer->set_model (x, y, m);
}

void CommonDisplay::set_item (int x, int y, Model *m) {
    item_layer->set_model (x,y , m);
}

void CommonDisplay::set_stone (int x, int y, Model *m) {
    stone_layer->set_model (x,y , m);
}



//----------------------------------------------------------------------
// Game Display Engine
//----------------------------------------------------------------------

GameDisplay::GameDisplay (const ScreenArea &gamearea, 
                          const ScreenArea &inventoryarea_)
: CommonDisplay(gamearea),
  last_frame_time (0),
  redraw_everything(false),
  m_reference_point (),
  m_follower (0),
  inventoryarea (inventoryarea_)
{
    status_bar = new StatusBarImpl (inventoryarea);
}

GameDisplay::~GameDisplay()
{
    delete m_follower;
    delete status_bar;
}

void GameDisplay::tick(double dtime) {
    get_engine()->tick (dtime);
    status_bar->tick (dtime);

    if (m_follower)
        m_follower->tick (dtime, m_reference_point);
}

void GameDisplay::new_world (int w, int h) {
    CommonDisplay::new_world (w, h);
    status_bar->new_world();
    resize_game_area (NTILESH, NTILESV);
    set_follow_mode (FOLLOW_SCREEN);
    m_reference_point = V2();

//     shadow_layer->new_world(w,h);
}

StatusBar *
GameDisplay::get_status_bar() const
{
    return status_bar;
}


/* -------------------- Scrolling -------------------- */

void GameDisplay::set_follow_mode (FollowMode m) {
    switch (m) {
    case FOLLOW_NONE: 
        set_follower(0); 
        break;
    case FOLLOW_SCROLLING:
	set_follower (new Follower_Scrolling(get_engine(), false)); 
        break;
    case FOLLOW_SCREEN:
	set_follower (new Follower_Screen(get_engine())); 
        break;
    case FOLLOW_SCREENSCROLLING:
	set_follower (new Follower_Scrolling(get_engine(), true)); 
        break;
    case FOLLOW_SMOOTH:
        set_follower (new Follower_Smooth(get_engine()));
    };
}

void GameDisplay::set_follower (Follower *f) {
    delete m_follower;
    if ((m_follower = f))
        follow_center();
}

void GameDisplay::follow_center() {
    if (m_follower)
        m_follower->center (m_reference_point);
}

void GameDisplay::set_reference_point (const V2 &point) {
    m_reference_point = point;
}

void GameDisplay::get_reference_point_coordinates(int *x, int *y) {
    get_engine()->world_to_screen(m_reference_point, x, y);
}

void GameDisplay::set_scroll_boundary (double boundary) 
{
    if (m_follower)
        m_follower->set_boundary (boundary);
}

/* ---------- Screen updates ---------- */

void GameDisplay::redraw_all (Screen *scr) {
    get_engine()->mark_redraw_screen();
    redraw_everything = true;
    scr->update_all();
    redraw (scr);
}

void GameDisplay::redraw (ecl::Screen *screen) {
    GC gc(screen->get_surface());
    if (SDL_GetTicks() - last_frame_time > 10) {
        CommonDisplay::redraw();

        if (ShowFPS) {
            char fps[20];
            sprintf (fps,"fps: %d\n", int(1000.0/(SDL_GetTicks()-last_frame_time)));
            Font *f = enigma::GetFont("levelmenu");

            clip(gc);
            Rect area (0,0,80,20);
            set_color (gc, 0,0,0);
            box (gc, area);
            f->render (gc, 0,0, fps);

            screen->update_rect(area);
        }
        last_frame_time = SDL_GetTicks();
    }
    if (status_bar->has_changed() || redraw_everything) {
        status_bar->redraw (gc, inventoryarea);
        screen->update_rect(inventoryarea);
    }
    if (redraw_everything)
        draw_borders(gc);
    screen->flush_updates();
    redraw_everything = false;
}

void GameDisplay::draw_all (GC &gc) {
    get_engine()->draw_all(gc);
    status_bar->redraw (gc, inventoryarea);
    draw_borders(gc);
}

void GameDisplay::draw_borders (GC &gc) {
    RectList rl;
    rl.push_back (gc.drawable->size());
    rl.sub (get_engine()->get_area());
    rl.sub (inventoryarea);
    clip(gc);
    set_color (gc, 0, 0, 0);
    for (RectList::iterator i=rl.begin(); i!=rl.end(); ++i) {
        box (gc, *i);
    }
}

void GameDisplay::resize_game_area (int w, int h) 
{
    DisplayEngine *e = get_engine();
    int neww = w * e->get_tilew();
    int newh = h * e->get_tileh();

    const video::VMInfo *vidinfo = video::GetInfo();

    int screenw = vidinfo->width;
    int screenh = NTILESV * vidinfo->tile_size;
    if (neww > screenw || newh > screenh) {
        enigma::Log << "Illegal screen size ("<< neww << "," << newh
                    << "): larger than physical display\n";
        return;
    }
    Rect r ((screenw-neww)/2, (screenh-newh)/2, neww, newh);
    e->set_screen_area (r);
    follow_center();
}



/* -------------------- Global functions -------------------- */

void display::Init() {
    InitModels();

    const video::VMInfo *vminfo = video::GetInfo();
    gamedpy = new GameDisplay (vminfo->gamearea, 
                               vminfo->statusbararea);
}

void display::Shutdown() {
    delete gamedpy;
    ShutdownModels();
}

void display::Tick (double dtime) {
    gamedpy->tick(dtime);
}

StatusBar * display::GetStatusBar() {
    return gamedpy->get_status_bar();
}

void display::NewWorld(int w, int h) {
    gamedpy->new_world (w, h);
}

void display::FocusReferencePoint() {
    gamedpy->follow_center();
}

void display::SetReferencePoint (const ecl::V2 &point) {
    gamedpy->set_reference_point (point);
}

void display::SetFollowMode(FollowMode m) {
    gamedpy->set_follow_mode(m);
}

void display::SetScrollBoundary (double boundary) {
    gamedpy->set_scroll_boundary (boundary);
}


void display::GetReferencePointCoordinates(int *x, int *y) {
    gamedpy->get_reference_point_coordinates(x, y);
}

Model *display::SetModel (const GridLoc &l, Model *m) {
    return gamedpy->set_model (l, m);
}

Model *display::SetModel (const GridLoc &l, const string &modelname) {
    return SetModel(l, MakeModel(modelname));
}

void display::KillModel(const GridLoc & l) {
    delete YieldModel(l);
}

Model *display::GetModel(const GridLoc &l) {
    return gamedpy->get_model (l);
}

Model *display::YieldModel(const GridLoc &l) {
    return gamedpy->yield_model (l);
}

SpriteHandle display::AddEffect (const V2& pos, const char *modelname) {
    return gamedpy->add_effect (pos, MakeModel(modelname));
}

SpriteHandle
display::AddSprite (const V2& pos, const char *modelname)
{
    Model *m = modelname ? MakeModel(modelname) : 0;
    return gamedpy->add_sprite (pos, m);
}

void display::ToggleFlag(DisplayFlags flag)
{
    toggle_flags (display_flags, flag);
}

void display::DrawAll (GC &gc) {
    gamedpy->draw_all(gc);
}

void display::RedrawAll(Screen *screen) {
    gamedpy->redraw_all(screen);
}

void display::Redraw (Screen *screen) {
    gamedpy->redraw (screen);
}

void display::ResizeGameArea (int w, int h) {
    gamedpy->resize_game_area (w, h);
}
const Rect& display::GetGameArea () {
    return gamedpy->get_engine()->get_area();
}

RubberHandle
display::AddRubber (const V2 &p1, const V2 &p2)
{
    return gamedpy->add_line (p1, p2);
}
