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
 
#ifndef GUI_LEVELPACKCONFIG_HH_INCLUDED
#define GUI_LEVELPACKCONFIG_HH_INCLUDED

#include "gui/Menu.hh"
#include "gui/TextField.hh"
#include "lev/Index.hh"
#include "lev/PersistentIndex.hh"

namespace enigma { namespace gui {

    class GroupButton : public ValueButton {
    public:
        GroupButton(std::vector<std::string> groups, int pos);
        int get_value() const;
        void set_value(int value);
        std::string get_text(int value) const;
    private:
        std::vector<std::string> groupNames;
        int position;
    };

    class LevelmodeButton : public ImageButton {
        // ActionListener interface.
        void on_action(Widget *);
    public:
        LevelmodeButton(bool initialMode = false);
        bool isLinkOnly();
        virtual void draw(ecl::GC &gc, const ecl::Rect &r);
    private:
        void update();
        bool mode;
    };


    class LevelPackConfig : public gui::Menu {
    public:
        LevelPackConfig (std::string indexName, std::string groupName = "",
            bool forceGroupReasign = false);
        
        void on_action(Widget *w);
        void draw_background(ecl::GC &gc);
        bool isUndoQuit();
    private:
        void updateLocationList();
        void switchToMetadataEdit();
        bool doChanges();
        
        lev::Index  *packIndex;
        lev::PersistentIndex *persIndex;
        bool         isPersistent;
        bool         isEditable;
        TextField   *tf_packName;
        GroupButton *groupButton;
        int          intialGroupPosition;
        VList       *valueLeftVList;
        Label       *titleValueLabel;
        TextField   *titleTF;
        Label       *ownerValueLabel;
        TextField   *ownerTF;
        VList       *valueMetaVList;
        LevelmodeButton *levelmode;
        Label       *defLocationValueLabel;
        TextField   *defLocationTF;
        Label       *releaseValueLabel;
        TextField   *releaseTF;
        Label       *revisionValueLabel;
        TextField   *revisionTF;
        Label       *compatibilityValueLabel;
        TextField   *compatibilityTF;
        Label       *pre2Index;
        Label       *pre1Index;
        Label       *thisIndex;
        Label       *post1Index;
        Label       *post2Index;
        Widget      *scrollUp;
        Widget      *scrollDown;
        Label       *errorLabel;
        Widget      *but_metadata;
        Widget      *but_up;
        Widget      *but_down;
        Widget      *but_edit;
        Widget      *but_update;
        Widget      *but_ignore;
        Widget      *but_back;
        bool         isReasignOnly;
        bool         didEditMetaData;
        bool         undo_quit;
        std::vector<std::string> locationList;
        int         position;       // new position of index in locationList that the user selected
        int         oldPosition;    // position of index when entering menu, -1 for new index
    };

}} // namespace enigma::gui
#endif
