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
 *
 */
 
#include "gui/LPGroupConfig.hh"
#include "gui/LevelPackConfig.hh"
#include "ecl.hh"
#include "errors.hh"
#include "nls.hh"
#include "video.hh"
#include "lev/Index.hh"

#include "main.hh"

using namespace ecl;
using namespace std;

namespace enigma { namespace gui {
    LPGroupConfig::LPGroupConfig(std::string groupName) : oldGroupName (groupName) {
        const video::VMInfo &vminfo = *video::GetInfo();
        groups = lev::Index::getGroupNames();
        
        position = -1;
        for (int i = 0; i < groups.size(); i++) {
            if (groups[i] == groupName) {
                position = i;
                break;
            } 
        }
        oldPosition = position;
        if (position < 0) {
            groups.push_back(groupName);
            position = groups.size() - 1;
        }

        VList * titleVList = new VList;
        titleVList->set_spacing(12);
        titleVList->set_alignment(HALIGN_LEFT, VALIGN_CENTER);
        titleVList->set_default_size(160, 35);
        Label * groupLabel = new Label(N_("Group:"), HALIGN_RIGHT);
        titleVList->add_back(groupLabel);
        
        VList * groupsVList = new VList;
        groupsVList->set_spacing(12);
        groupsVList->set_alignment(HALIGN_LEFT, VALIGN_CENTER);
        groupsVList->set_default_size(160, 35);
        
        Label * positionLabel = new Label(N_("Position:"), HALIGN_LEFT);
        pre2Group = new UntranslatedLabel("");
        pre1Group = new UntranslatedLabel("");
        tf_groupName = new TextField(groupName);
        post1Group = new UntranslatedLabel("");
        post2Group = new UntranslatedLabel("");
        Label * dummyLabel = new Label("");

        groupsVList->add_back(positionLabel);
        groupsVList->add_back(pre2Group);
        groupsVList->add_back(pre1Group);
        groupsVList->add_back(tf_groupName);
        groupsVList->add_back(post1Group);
        groupsVList->add_back(post2Group);
        groupsVList->add_back(dummyLabel);

        VList * scrollVList = new VList;
        scrollVList->set_spacing(12);
        scrollVList->set_alignment(HALIGN_LEFT, VALIGN_CENTER);
        scrollVList->set_default_size(40, 35);

        scrollUp = new ImageButton("ic-up", "ic-up1", this);
        scrollDown = new ImageButton("ic-down", "ic-down1", this);
        scrollVList->add_back(scrollUp);
        scrollVList->add_back(scrollDown);

        this->add(titleVList, Rect(vminfo.width/2 - 290, 0, 160, vminfo.height-100));
        this->add(groupsVList, Rect(vminfo.width/2 - 80, 0, 160, vminfo.height-100));
        this->add(scrollVList, Rect(vminfo.width/2 + 130, 0, 40, vminfo.height-100));

        errorLabel = new Label("", HALIGN_CENTER);
        this->add(errorLabel, Rect(10, vminfo.height-100, vminfo.width-20, 35));
        
        // Create buttons - positioning identical to Levelmenu
        but_newPack = new StaticTextButton(N_("New Pack"), this);
        but_delete = new StaticTextButton(N_("Delete Group"), this);
        but_ignore = new StaticTextButton(N_("Undo"), this);
        but_ok = new StaticTextButton(N_("Ok"), this);
        
        HList * commandHList = new HList;
        commandHList->set_spacing(10);
        commandHList->set_alignment(HALIGN_CENTER, VALIGN_TOP);
        commandHList->set_default_size(140, 35);
        commandHList->add_back(but_newPack);
        commandHList->add_back(but_delete);
        commandHList->add_back(but_ignore);
        commandHList->add_back(but_ok);
        this->add(commandHList, Rect(10, vminfo.height-50, vminfo.width-20, 35));
        
        updateGroupList();
    }
    
    void LPGroupConfig::updateGroupList() {
        pre2Group->set_text((position > 1) ? groups[position - 2] : "");
        pre1Group->set_text((position > 0) ? groups[position - 1] : "");
        post1Group->set_text((position < groups.size() - 1) ? groups[position + 1] : "");
        post2Group->set_text((position < groups.size() - 2) ? groups[position + 2] : "");        
    }
    
    bool LPGroupConfig::doChanges() {
        // rename first for consistency
        std::string newName = tf_groupName->getText();
        std::string::size_type lastChar = newName.find_last_not_of(" ");
        if (lastChar == std::string::npos) {
            // the name is effectively an empty string
            errorLabel->set_text(N_("Error: empty group name not allowed - press \"Undo\" to exit without modifications"));
            return false;
        }

        // strip off trailing and leading spaces
        newName = newName.substr(0 , lastChar + 1);
        newName = newName.substr(newName.find_first_not_of(" "));
        
        // check if new group name is unique
        for (int i = 0; i < groups.size(); i++) {
            if (i != position && groups[i] == newName) {
                errorLabel->set_text(N_("Error: group name is a duplicate of an existing group"));
                return false;
            }
        }
        if (newName == INDEX_EVERY_GROUP) {
            errorLabel->set_text(N_("Error: \"Every group\" is a reserved group name"));
            return false;
        }
        if (newName.size() > 2 && newName[0] == '[' 
                && newName[newName.size() -1] == ']') {
            errorLabel->set_text(N_("Error: group name must not be enclosed in square brackets"));
            return false;
        }
            
        
        if (oldGroupName.empty()) {
            // menu called without an existing group
            lev::Index::insertGroup(newName, position);
        } else {
            // menu called with an existing group
            if (newName != oldGroupName)
                lev::Index::renameGroup(oldGroupName, newName);
            
            if (oldPosition >= 0 && position != oldPosition) {
                // move the group to the new position
                lev::Index::moveGroup(newName, position);
            }
        }
        return true;
    }
    
    void LPGroupConfig::on_action(Widget *w) {
        if (w == but_ok) {
            if (doChanges())
                Menu::quit();
            else
                invalidate_all();
        } else if (w == but_ignore) {
            Menu::quit();
        } else if (w == scrollUp) {
            if (position > 0) {
                std::string tmp = groups[position];
                groups[position] = groups[position - 1];
                groups[position - 1] = tmp;
                position--;
                updateGroupList();
                invalidate_all();
            }
        } else if (w == scrollDown) {
            if (position < groups.size() - 1) {
                std::string tmp = groups[position];
                groups[position] = groups[position + 1];
                groups[position + 1] = tmp;
                position++;
                updateGroupList();
                invalidate_all();
            }
        } else if (w == but_delete) {
            std::vector<lev::Index *> * indices = lev::Index::getGroup(oldGroupName);
            if (indices != NULL) {
                // reassign remaining indices from back to front to keep the
                // group vector valid
                for (int i = indices->size() - 1; i >= 0; i--) {
                    if ((*indices)[i]->getGroupName() != INDEX_EVERY_GROUP) {
                        LevelPackConfig m((*indices)[i]->getName(), oldGroupName, true);
                        if (!m.manage() || m.isUndoQuit()) {
                            errorLabel->set_text(N_("Error: group not empty"));
                            invalidate_all();
                            return;
                        } 
                    }
                }
                lev::Index::deleteGroup(oldGroupName);
            }
            Menu::quit();
        } else if (w == but_newPack) {
            if (doChanges()) {
                LevelPackConfig m("");
                m.manage();
                Menu::quit();
            } else {
                invalidate_all();
            }
        }
    }
    
    void LPGroupConfig::draw_background(ecl::GC &gc) {
        video::SetCaption(("Enigma - Level Pack Group Configuration"));
        blit(gc, 0,0, enigma::GetImage("menu_bg", ".jpg"));
    }
    

}} // namespace enigma::gui
