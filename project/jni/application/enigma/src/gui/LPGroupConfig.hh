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
 
#ifndef GUI_LPGROUPCONFIG_HH_INCLUDED
#define GUI_LPGROUPCONFIG_HH_INCLUDED

#include "gui/Menu.hh"
#include "gui/TextField.hh"

namespace enigma { namespace gui {
    class LPGroupConfig : public gui::Menu {
    public:
        LPGroupConfig (std::string groupName);
        
        void on_action(Widget *w);
        void draw_background(ecl::GC &gc);
    private:
        void updateGroupList();
        bool doChanges();
        
        TextField   *tf_groupName;
        Label       *pre2Group;
        Label       *pre1Group;
        Label       *post1Group;
        Label       *post2Group;
        Widget      *scrollUp;
        Widget      *scrollDown;
        Label       *errorLabel;
        Widget      *but_newPack;
        Widget      *but_delete;
        Widget      *but_ignore;
        Widget      *but_ok;
        std::vector<std::string> groups;
        int         position;       // new position of group that the user selected
        int         oldPosition;    // position of group when entering menu, -1 for new group
        std::string oldGroupName;
    };

}} // namespace enigma::gui
#endif

