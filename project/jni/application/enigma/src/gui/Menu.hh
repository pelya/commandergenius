/*
 * Copyright (C) 2002,2003,2004,2005,2006 Daniel Heck
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
#ifndef GUI_MENU_HH_INCLUDED
#define GUI_MENU_HH_INCLUDED

#include "gui/widgets.hh"
#include "gui/Menu.hh"
#include "ecl_fwd.hh"
#include "ecl_geom.hh"
#include "SDL.h"
#include <cmath>
#include <vector>

namespace enigma { namespace gui {
/* -------------------- Menu -------------------- */

    class Menu : public Container {
    public:
        Menu();

        //! true: ok, false: menu aborted by user
        virtual bool manage();

        void add(Widget *w);
        void add(Widget *w, ecl::Rect r);
        void center();

        void draw (ecl::GC &gc, const ecl::Rect &r);

        virtual void quit();
        void abort();

    protected:
        void reset_active_widget()
        { active_widget = 0; }

        // Menu interface.
        virtual void draw_background(ecl::GC &/*gc*/) {}

    private:
        void handle_event(const SDL_Event &e);

        void switch_active_widget(Widget *to_activate);
        void track_active_widget( int x, int y ); // used by mouse
        void goto_adjacent_widget(int xdir, int ydir); // used by keyboard

        // Variables.
        Widget *active_widget;
        bool quitp, abortp;
    };

    class BuildVList {
        ecl::Rect r;
        Menu *container;
        int skip;
    public:
        BuildVList(Menu *cc, const ecl::Rect &rr, int s)
        : r(rr), container(cc), skip(s)
        {}

        Widget *add(Widget *w) {
            container->add(w, r);
            r.y += r.h+skip;
            return w;
        }

        ecl::Rect pos() const { return r; }
    };

    class BuildHList {
        ecl::Rect r;
        Menu *container;
        int skip;
    public:
        BuildHList(Menu *cc, const ecl::Rect &rr, int s)
        : r(rr), container(cc), skip(s)
        {}

        Widget * add(Widget *w) {
            container->add(w, r);
            r.x += r.w+skip;
            return w;
        }
        Widget *add (Widget *w, int width) {
            ecl::Rect rr(r.x, r.y, width, r.h);
            container->add(w, rr);
            r.x += width + skip;
            return w;
        }

        ecl::Rect pos() const { return r; }
    };

}} // namespace enigma::gui
#endif
