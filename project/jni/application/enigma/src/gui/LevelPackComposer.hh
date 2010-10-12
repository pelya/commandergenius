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
 
#ifndef GUI_LEVELPACKCOMPOSER_HH_INCLUDED
#define GUI_LEVELPACKCOMPOSER_HH_INCLUDED

#include "gui/Menu.hh"
#include "gui/LevelWidget.hh"
#include "lev/Proxy.hh"
#include "lev/Index.hh"
#include "lev/PersistentIndex.hh"

#include <set>

namespace enigma { namespace gui {
    
    class LevelPackComposer : public gui::Menu {
    public:
        LevelPackComposer(bool enableEdit);
        
        void tick(double dtime);
        bool on_event (const SDL_Event &e);
        void on_action(Widget *w);
        void draw_background(ecl::GC &gc);
    private:
        void update_info();

        static lev::PersistentIndex * clipboard;
        bool isEditable;
        bool isModified;
        lev::PersistentIndex * curIndex;
        std::set<std::string> deletions; // normLevelPath
        std::set<std::string> backups;   // normLevelPath
        Widget *pgup, *pgdown, *start, *end;
        LevelWidget *levelwidget;
        Label       *lbl_lpinfo;        // Levelpack information
        Label       *lbl_levelname;
        Label       *lbl_clipinfo;
        Label       *lbl_clipcontent;
        int          clipContentWidth;
        Widget      *but_ignore;
        Widget      *but_back;
    };

}} // namespace enigma::gui
#endif
