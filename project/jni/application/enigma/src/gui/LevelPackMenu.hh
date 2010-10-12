/*
 * Copyright (C) 2002,2003,2004,2005,2006 Daniel Heck, Ronald Lamprecht
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
#ifndef GUI_LEVELPACKMENU_HH_INCLUDED
#define GUI_LEVELPACKMENU_HH_INCLUDED

#include "gui/Menu.hh"
#include "video.hh"
#include "lev/Index.hh"

#include <map>
#include <string>

namespace enigma { namespace gui {
/* -------------------- LevelPackMenu -------------------- */

    /**
     * The levelpack selection menu.
     * 
     * The used strategy for horizontal scrolling of the levelpack columns
     * is to avoid scrolling as long as the target levelpack is still visible.
     * The current levelpack may be scrolled out of visibility by using the
     * scoll buttons within this menu. But if the user changes the current
     * levelpack via indirect methods besides
     * using the levelpack menu, f.e. the jumpto command, we ensure that
     * the new current levelpack is visible when the menu is redisplayed.
     * If it is necessary to scroll we scroll only the minimum amount of columns
     * needed to display the current levelpack. Even on screen resolution changes
     * we try to keep the column of the current levelpack unchanged.
     */
    class LevelPackMenu : public Menu {
    public:
        LevelPackMenu();
        void manageLevelMenu();
        virtual bool manage();

        bool on_event (const SDL_Event &e);
        void on_action(Widget *w);
        void draw_background(ecl::GC &gc);

    private:
        static std::map<std::string, std::string> groupLastSelectedIndex;
        static std::map<std::string, int> groupLastSelectedColumn;
        static std::string lastGroupName;
        static int firstDisplayedGroup;
        
        video::VideoModes vm;
        std::vector<TextButton *> packButtons;
        std::vector<TextButton *> groupButtons;

        HList       *packsHList;
        ImageButton *scrollLeft;
        ImageButton *scrollRight;        
        ImageButton *scrollUp;        
        ImageButton *scrollDown;        
        VList       *groupsVList;
        HList       *commandHList;
        Widget      *but_search;
        Widget      *but_new;
        Widget      *but_level;
        Widget      *but_main;
        
        bool         isLevelMenuSubmenu;
        
        void setupMenu();
        void updateHighlight();
        int getGroupPosition(std::vector<std::string> * groups, std::string groupName);
        int getIndexPosition(std::vector<lev::Index *> * group, std::string indexName);
        int checkColumn(int rows, int columns, int size, int position, int oldColumn);
    };
}} // namespace enigma::gui
#endif
