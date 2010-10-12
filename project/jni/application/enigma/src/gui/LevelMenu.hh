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
#ifndef GUI_LEVELMENU_HH_INCLUDED
#define GUI_LEVELMENU_HH_INCLUDED

#include "gui/Menu.hh"
#include "gui/LevelWidget.hh"
#include "lev/Proxy.hh"
#include "lev/Index.hh"


namespace enigma { namespace gui {
/* -------------------- LevelMenu -------------------- */

    class LevelMenu : public Menu {
    public:
        LevelMenu();

        // Rotate through levels and packs
        void next_levelpack();
        void previous_levelpack();
        void show_text(const string& text);
        bool isMainQuit();

    private:
        void update_info();
        void next_unsolved();

        void updateIndex();

        // Menu interface.
        void tick (double time);
        void draw_background(ecl::GC &gc);

        // Widget interface.
        bool on_event (const SDL_Event &e);

        // ActionListener interface.
        void on_action(Widget *w);

        // Variables.


        Widget *pgup, *pgdown, *start, *end;
        Widget      *but_advancemode;      // Next unsolved level button
        Widget      *but_next;
        Widget      *but_back;          // "Back" button
        Widget      *but_difficulty;        // "Difficulty" button
        TextButton  *but_levelpack;     // "Levelpack" button
        HList       *hl_info_stat;
        Label       *lbl_lpinfo;        // Levelpack information
        Label       *lbl_statistics;        // percentage solved
        Label       *lbl_levelname;
        Label       *lbl_levelinfo;
        LevelWidget *levelwidget;
        string       shown_text; // info text (disappears automatically)
        double       shown_text_ttl; // rest duration for shown_text
        bool         main_quit;
    };

/* -------------------- Buttons -------------------- */

    class DifficultyButton : public ImageButton {
        // ActionListener interface.
        void on_action(Widget *);
    public:
        DifficultyButton();
        virtual void draw(ecl::GC &gc, const ecl::Rect &r);
    private:
        void update();
    };

    class AdvanceModeButton : public ImageButton {
        // ActionListener interface.
        void on_action(Widget *);
    public:
        AdvanceModeButton();
    private:
        void update();
    };

}} // namespace enigma::gui
#endif
