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
#ifndef GUI_GAMEMENU_HH_INCLUDED
#define GUI_GAMEMENU_HH_INCLUDED

#include "gui/Menu.hh"

namespace enigma { namespace gui {
/* -------------------- GameMenu -------------------- */
    class GameMenu : public Menu {
    public:
        GameMenu(int zoomxpos_, int zoomypos_);
        virtual ~GameMenu();
    private:
        bool on_event (const SDL_Event &e);
        void on_action(gui::Widget *w);
        void draw_background(ecl::GC &gc);

        gui::Widget *resume, *restart, *options, *info, *abort;
        ecl::Surface *zoomed;
        int          zoomxpos, zoomypos; // position to be zoomed
    };
}} // namespace enigma::gui
#endif
