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
 
#ifndef GUI_SEARCHMENU_HH_INCLUDED
#define GUI_SEARCHMENU_HH_INCLUDED

#include "gui/Menu.hh"
#include "gui/TextField.hh"

namespace enigma { namespace gui {

    class SearchMenu : public gui::Menu {
    public:
        SearchMenu ();
        
        void on_action(Widget *w);
        void draw_background(ecl::GC &gc);
        bool isSearchQuit();
        
    private:
        TextField   *shallowSearch;
        Widget      *but_ignore;
        Widget      *but_search;
        bool         didSearch;
    };

}} // namespace enigma::gui
#endif
