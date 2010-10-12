/*
 * Copyright (C) 2002,2003 Daniel Heck, Ralf Westram
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

#ifndef ENIGMA_HELP_HH
#define ENIGMA_HELP_HH


#include "gui/Menu.hh"
#include "ecl.hh"

namespace enigma { namespace gui {
    struct HelpMenuConfig {
        int         x0, x1;         // x coordinates of first and second column
        int         y0;             // y coordinate
        int         yskip;
        std::string fontname;
    
        HelpMenuConfig (int xoffset) {
            x0       = 40;
            x1       = x0 + xoffset;
            y0       = 40;
            yskip    = 30;
            fontname = "menufont";
        }
    };

    class HelpMenu : public Menu {
    public:
        HelpMenu (const char **helptext_, int xoffset);
    private:
        bool on_event           (const SDL_Event &e);
        void on_action          (gui::Widget *w);
        void draw_background    (ecl::GC &gc);
    
        const char     **helptext;
        gui::Widget     *ok;
        HelpMenuConfig   cfg;
    };
    
    void displayHelp (const char **helptext, int xoffset);

}} // namespace enigma::gui
#endif // ENIGMA_HELP_HH

