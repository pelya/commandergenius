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
#ifndef GUI_OPTIONSMENU_HH_INCLUDED
#define GUI_OPTIONSMENU_HH_INCLUDED

#include "gui/Menu.hh"
#include "gui/TextField.hh"

namespace enigma { namespace gui {
/* -------------------- OptionsMenu -------------------- */

    class OptionsMenu : public Menu {
    public:
        OptionsMenu(ecl::Surface *background_);
        ~OptionsMenu();
        virtual void quit();
    private:
        void update_info();

        // ActionListener interface.
        bool on_event (const SDL_Event &e);
        void on_action(gui::Widget *w);

        // Menu interface.
        void draw_background(ecl::GC &gc);
        void tick(double dtime);

        // Variables.
        gui::Widget *back, *fullscreen, *language;
        gui::TextField *userNameTF;
        gui::TextField *userPathTF;
        gui::TextField *userImagePathTF;
        gui::Label  *m_restartinfo;
        ecl::Surface *background;
        std::string  previous_caption;
    };

/* -------------------- Options Buttons -------------------- */

    struct FullscreenButton : public BoolOptionButton {
        FullscreenButton();
    };


    class StereoButton : public ValueButton {
        int get_value() const;
        void set_value(int value);
        std::string get_text(int value) const;
    public:
        StereoButton();
    };


    class SoundSetButton : public ValueButton {
    public:
        SoundSetButton();
        int get_value() const;
        void set_value(int value);
        std::string get_text(int value) const;
    };

    class LanguageButton : public ValueButton {
        int get_value() const;
        void set_value(int value);
        std::string get_text(int value) const;
        bool inInit;
        ActionListener *myListener;
    public:
        // second user action listener: first one is misused by ValueButton 
        LanguageButton (ActionListener *al = 0);
        
    };

    class GammaButton : public ValueButton {
        int get_value() const;
        void set_value(int value);
        std::string get_text(int value) const;
    public:
        GammaButton ();
        
    };
    
/* -------------------- Functions -------------------- */

    void ShowOptionsMenu(ecl::Surface *background);

}} // namespace enigma::gui
#endif
