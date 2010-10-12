/*
 * Copyright (C) 2006, 2007 Ronald Lamprecht
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
 
#include "gui/LevelPackConfig.hh"
#include "gui/LevelPackComposer.hh"
#include "ecl.hh"
#include "errors.hh"
#include "nls.hh"
#include "video.hh"
#include "lev/Index.hh"

#include "main.hh"

using namespace ecl;
using namespace std;

namespace enigma { namespace gui {
    GroupButton::GroupButton(std::vector<std::string> groups, int pos) : 
            ValueButton(0, groups.size() - 1), 
            position (pos), groupNames (groups) {
        init();
    }
    
    int GroupButton::get_value() const  {
        return position; 
    }
    
    void GroupButton::set_value(int value) {
        position = value;
    }
    
    std::string GroupButton::get_text(int value) const {
        return groupNames[value];
    }
    
    /* ------------------- LevelmodeButton -------------------- */
    
    LevelmodeButton::LevelmodeButton(bool initialMode) : 
            ImageButton("ic-link_copy","ic-link_copy",this), mode (initialMode) {
        update();
    }
    
    bool LevelmodeButton::isLinkOnly() {
        return mode;
    }
    
    void LevelmodeButton::update() {
        if (mode)
            ImageButton::set_images("ic-link","ic-link_copy");
        else
            ImageButton::set_images("ic-link_copy","ic-link");
    }
    
    void LevelmodeButton::on_action(Widget *) 
    {
        mode = !mode;
        update();
        invalidate();
    }

    void LevelmodeButton::draw(ecl::GC &gc, const ecl::Rect &r) {
        update();
        ImageButton::draw(gc, r);
    }
    
    
    
    LevelPackConfig::LevelPackConfig(std::string indexName, std::string groupName,
            bool forceGroupReasign) : isReasignOnly (forceGroupReasign), 
            undo_quit (false), didEditMetaData (false), titleTF (NULL) {
        const video::VMInfo &vminfo = *video::GetInfo();
        
        if (indexName.empty())
            // new levelpack
            packIndex = new lev::PersistentIndex(" ", false,
                    INDEX_DEFAULT_PACK_LOCATION,  "", 
                    INDEX_STD_FILENAME, lev::Index::getCurrentGroup()); // mark as incomplete
        else
            packIndex = lev::Index::findIndex(indexName);
        ASSERT (packIndex != NULL, XFrontend, "not existing index Name");
        persIndex = dynamic_cast<lev::PersistentIndex *>(packIndex);
        isPersistent = (persIndex != NULL);
        isEditable = isPersistent ? persIndex->isUserEditable() : false;
        
        // build a list of allowed group
        std::vector<std::string> groups = lev::Index::getGroupNames();
        // eliminate pseudo group "All Packs"
        std::vector<std::string>::iterator itg = groups.begin();
        while (itg != groups.end()) {
            if (*itg == INDEX_ALL_PACKS) {
                itg = groups.erase(itg);
                break;
            }
            if (itg != groups.end())
                itg++;
        }
        // add pseudo group "[Every Group]"
        groups.push_back(std::string("[") + INDEX_EVERY_GROUP +"]");
        intialGroupPosition = groups.size() - 1;   // INDEX_EVERY_GROUP as default
        // mark index's default group with square brackets and find current group
        bool defaultGroupFound = false;
        for (int i = 0; i < groups.size(); i++) {
            if (groups[i] == packIndex->getGroupName()) {
                intialGroupPosition = i;
            }
            if (groups[i] == packIndex->getDefaultGroupName()) {
                groups[i] = std::string("[") + groups[i] +"]";
                defaultGroupFound = true;
            }
        }
        if (!defaultGroupFound)  {
            groups.push_back(std::string("[") + packIndex->getDefaultGroupName() +"]");
        }
        groupButton = new GroupButton(groups, intialGroupPosition);

        // index location list setup
        std::vector<lev::Index *> * allIndices = lev::Index::getGroup(INDEX_ALL_PACKS);
        for (int i = 0; i < allIndices->size(); i++)
            locationList.push_back((*allIndices)[i]->getName());
        position = -1;
        for (int i = 0; i < locationList.size(); i++) {
            if (locationList[i] == indexName) {
                position = i;
                break;
            } 
        }
        oldPosition = position;
        if (position < 0) { 
            // append new levelpack as last
            locationList.push_back(indexName);
            position = locationList.size() - 1;
        }

        VList * titleLeftVList = new VList;
        titleLeftVList->set_spacing(11);
        titleLeftVList->set_alignment(HALIGN_LEFT, VALIGN_CENTER);
        titleLeftVList->set_default_size(140, 35);
        Label * titleLabel = new Label(N_("Levelpack:"), HALIGN_RIGHT);
        Label * ownerLabel = new Label(N_("Owner:"), HALIGN_RIGHT);
        Label * groupLabel = new Label(N_("Group:"), HALIGN_RIGHT);
        Label * loactionLabel1 = new Label(N_("Location"), HALIGN_LEFT);
        Label * loactionLabel2 = new Label(N_("in [All Packs]:"), HALIGN_RIGHT);
        titleLeftVList->add_back(titleLabel);
        if (!isReasignOnly) {
           titleLeftVList->add_back(ownerLabel);
        }
        titleLeftVList->add_back(groupLabel);
        if (!isReasignOnly) {        
            titleLeftVList->add_back(new Label());
            titleLeftVList->add_back(loactionLabel1);
            titleLeftVList->add_back(loactionLabel2);
            titleLeftVList->add_back(new Label());
            titleLeftVList->add_back(new Label());
        }
        
        valueLeftVList = new VList;
        valueLeftVList->set_spacing(11);
        valueLeftVList->set_alignment(HALIGN_LEFT, VALIGN_CENTER);
        valueLeftVList->set_default_size(160, 35);

        titleValueLabel = new UntranslatedLabel(indexName, HALIGN_CENTER);
        ownerValueLabel = new UntranslatedLabel(isPersistent ? persIndex->getOwner() : "System");
        
        pre2Index = new UntranslatedLabel();
        pre1Index = new UntranslatedLabel();
        thisIndex = new UntranslatedLabel();
        post1Index = new UntranslatedLabel();
        post2Index = new UntranslatedLabel();

        valueLeftVList->add_back(titleValueLabel);
        if (!isReasignOnly) {        
            valueLeftVList->add_back(ownerValueLabel);
        }
        valueLeftVList->add_back(groupButton);
        if (!isReasignOnly) {        
            valueLeftVList->add_back(pre2Index);
            valueLeftVList->add_back(pre1Index);
            valueLeftVList->add_back(thisIndex);
            valueLeftVList->add_back(post1Index);
            valueLeftVList->add_back(post2Index);
        }

        VList * scrollVList = new VList;
        scrollVList->set_spacing(12);
        scrollVList->set_alignment(HALIGN_LEFT, VALIGN_CENTER);
        scrollVList->set_default_size(30, 35);

        scrollUp = new ImageButton("ic-up", "ic-up1", this);
        scrollDown = new ImageButton("ic-down", "ic-down1", this);
        scrollVList->add_back(scrollUp);
        scrollVList->add_back(scrollDown);

        VList * metaVList = new VList;
        metaVList->set_spacing(12);
        metaVList->set_alignment(HALIGN_LEFT, VALIGN_CENTER);
        metaVList->set_default_size(140, 35);
        
        if (isEditable)
            but_metadata = new StaticTextButton(N_("Edit Metadata"), this);
        else
            but_metadata = new Label();
        Label * releaseLabel = new Label(N_("Release:"), HALIGN_RIGHT);
        Label * revisionLabel = new Label(N_("Revision:"), HALIGN_RIGHT);
        Label * compatibilityLabel = new Label(N_("Compatibility:"), HALIGN_RIGHT);
        Label * defLocationLabel = new Label(N_("Default Location:"), HALIGN_RIGHT);
        Label * crossmodeLabel = new Label(N_("Level types:"), HALIGN_RIGHT);
        
        if (!isReasignOnly) {
            metaVList->add_back(but_metadata);
            metaVList->add_back(new Label());
	    if (WizardMode) {
		metaVList->add_back(releaseLabel);
		metaVList->add_back(revisionLabel);
	    } else {
		metaVList->add_back(new Label());
		metaVList->add_back(new Label());
	    }
            metaVList->add_back(crossmodeLabel);
	    if (WizardMode) {
		metaVList->add_back(compatibilityLabel);
	    } else {
		metaVList->add_back(new Label());
	    }
            metaVList->add_back(defLocationLabel);
            metaVList->add_back(new Label());
        }

        valueMetaVList = new VList;
        valueMetaVList->set_spacing(12);
        valueMetaVList->set_alignment(HALIGN_CENTER, VALIGN_CENTER);
        valueMetaVList->set_default_size(75, 35);
        Widget * levelmodeWidget;
        if (indexName.empty()){
            levelmode = new LevelmodeButton(false);
            levelmodeWidget = levelmode;
        } else {
            levelmodeWidget = new Image(isPersistent && !(persIndex->isCross()) ?
                    "ic-link_copy" : "ic-link");
        }
        defLocationValueLabel = new Label(ecl::strf("%g", packIndex->getDefaultLocation()));
        releaseValueLabel = new Label(isPersistent ? ecl::strf("%d", persIndex->getRelease()) : "-");
        revisionValueLabel = new Label(isPersistent ? ecl::strf("%d", persIndex->getRevision()) : "-");
        compatibilityValueLabel = new Label(isPersistent ? ecl::strf("%.2f", persIndex->getCompatibility()) : "-");
        
        if (!isReasignOnly) {
            valueMetaVList->add_back(new Label());
            valueMetaVList->add_back(new Label());
	    if (WizardMode) {
            valueMetaVList->add_back(releaseValueLabel);
            valueMetaVList->add_back(revisionValueLabel);
	    } else {
            valueMetaVList->add_back(new Label());
            valueMetaVList->add_back(new Label());
	    }
            valueMetaVList->add_back(levelmodeWidget);
	    if (WizardMode) {
            valueMetaVList->add_back(compatibilityValueLabel);
	    } else {
            valueMetaVList->add_back(new Label());
	    }
            valueMetaVList->add_back(defLocationValueLabel);
            valueMetaVList->add_back(new Label());
        }
        

        if (isReasignOnly) {        
            this->add(titleLeftVList, Rect(vminfo.width/2 - 270, 15, 140, vminfo.height-97));
            this->add(valueLeftVList, Rect(vminfo.width/2 - 80, 15, 160, vminfo.height-97));
        } else {
            this->add(titleLeftVList, Rect(vminfo.width/2 - 300, 15, 140, vminfo.height-97));
            this->add(valueLeftVList, Rect(vminfo.width/2 - 140, 15, 160, vminfo.height-97));
            this->add(scrollVList, Rect(vminfo.width/2 + 30, 15+3*(35+12) + (vminfo.height-480)/2, 30, 5*35+4*12));
            this->add(metaVList, Rect(vminfo.width/2 + 80, 15, 140, vminfo.height-97));
            this->add(valueMetaVList, Rect(vminfo.width/2 + 235, 15, 75, vminfo.height-97));
        }
    
        errorLabel = new Label("", HALIGN_CENTER);
        this->add(errorLabel, Rect(10, vminfo.height-97, vminfo.width-20, 35));
        
        if (isReasignOnly) 
            errorLabel->set_text(N_("Please reasign levelpack to another group for group deletion"));
       
        // Create buttons - positioning identical to Levelmenu
        but_edit = new StaticTextButton(N_("Compose Pack"), this);
        if (isPersistent && persIndex->isUpdatable() && persIndex->isCross()) {
            but_update = new StaticTextButton(N_("Update Pack"), this);
        } else {
            but_update = new Label();
        }
        but_ignore = new StaticTextButton(N_("Undo"), this);
        but_back = new StaticTextButton(N_("Ok"), this);
        
        HList * commandHList = new HList;
        commandHList->set_spacing(10);
        commandHList->set_alignment(HALIGN_CENTER, VALIGN_TOP);
        commandHList->set_default_size(140, 35);
        if (isReasignOnly) {
            commandHList->add_back(new Label());
            commandHList->add_back(new Label());
        } else {
            commandHList->add_back(but_edit);
            commandHList->add_back(but_update);
        }
        commandHList->add_back(but_ignore);
        commandHList->add_back(but_back);
        this->add(commandHList, Rect(10, vminfo.height-50, vminfo.width-20, 35));
        
        updateLocationList();
        if (indexName.empty())
            // new levelpack
            switchToMetadataEdit();
    }
    
    void LevelPackConfig::updateLocationList() {
        pre2Index->set_text((position > 1) ? locationList[position - 2] : "");
        pre1Index->set_text((position > 0) ? locationList[position - 1] : "");
        thisIndex->set_text(didEditMetaData ? titleTF->getText() : packIndex->getName());
        post1Index->set_text((position < locationList.size() - 1) ? locationList[position + 1] : "");
        post2Index->set_text((position < locationList.size() - 2) ? locationList[position + 2] : "");        
    }
    
    void LevelPackConfig::switchToMetadataEdit() {
        if (!didEditMetaData) {
            didEditMetaData = true;
            titleTF = new TextField(titleValueLabel->getText(), this); 
            valueLeftVList->exchange_child(titleValueLabel, titleTF);
            delete titleValueLabel;
            ownerTF = new TextField(ownerValueLabel->getText()); 
            valueLeftVList->exchange_child(ownerValueLabel, ownerTF);
            delete ownerValueLabel;
            defLocationTF = new TextField(defLocationValueLabel->getText()); 
            valueMetaVList->exchange_child(defLocationValueLabel, defLocationTF);
            delete defLocationValueLabel;
	    if (WizardMode) {
		releaseTF = new TextField(releaseValueLabel->getText()); 
		valueMetaVList->exchange_child(releaseValueLabel, releaseTF);
		delete releaseValueLabel;
		revisionTF = new TextField(revisionValueLabel->getText()); 
		valueMetaVList->exchange_child(revisionValueLabel, revisionTF);
		delete revisionValueLabel;
		compatibilityTF = new TextField(compatibilityValueLabel->getText()); 
		valueMetaVList->exchange_child(compatibilityValueLabel, compatibilityTF);
		delete compatibilityValueLabel;
	    }
        }
    }
    
    bool LevelPackConfig::isUndoQuit() {
        return undo_quit;
    }
    
    bool LevelPackConfig::doChanges() {
        // change metadata
        if (didEditMetaData) {
            // the Index is persistent, user editabel and the user did switch to edit mode
            bool needSave = false;
            bool isNewIndex = persIndex->getName().empty();
            
            // check for valid input 
            // title
            std::string newtitle = titleTF->getText();
            std::string::size_type lastChar = newtitle.find_last_not_of(" ");
            if (lastChar == std::string::npos) {
                // the title is effectively an empty string
                errorLabel->set_text(N_("Error: empty title not allowed - press \"Undo\" to exit without modifications"));
                return false;
            }   
            // strip off trailing and leading spaces
            newtitle = newtitle.substr(0 , lastChar + 1);
            newtitle = newtitle.substr(newtitle.find_first_not_of(" "));
            if (newtitle != persIndex->getName()) {
                if (isNewIndex) {
                    // check for filename usability of title
                    const std::string validChars("_- .#0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ");
                    if (newtitle.find_first_not_of(validChars, 0) != std::string::npos ||
			    (newtitle.length() >= 1 && newtitle[0] == '.')) {
                        errorLabel->set_text(N_("Error: use only \"a-zA-Z0-9 _-#\" for levelpack title"));
                        return false;               
                    }
                    
                    // set packPath to cross if link only
                    if (levelmode->isLinkOnly())
                        persIndex->markNewAsCross();
                }
                if (!persIndex->setName(newtitle)) {
                    errorLabel->set_text(N_("Error: title already in use - choose another title"));
                    return false;
                }
                needSave = true;                
            }

            // check rest for need of save
            if (ownerTF->getText() != persIndex->getOwner()) {
                persIndex->setOwner(ownerTF->getText());
                needSave = true;
            }
            if (defLocationTF->getText() != ecl::strf("%g", packIndex->getDefaultLocation())) {
                double d = 0;
                // check value - keep old value on error 
                if ((sscanf(defLocationTF->getText().c_str(),"%lg", &d) == 1) &&
                        d > 0) {
                    packIndex->setDefaultLocation(d);
                    needSave = true;
                }
            }
            
            if (WizardMode) {
                if (releaseTF->getText() != ecl::strf("%d", persIndex->getRelease())) {
                    int i = 0;
                    // check value - keep old value on error 
                    if ((sscanf(releaseTF->getText().c_str(),"%d", &i) == 1) &&
                	    i > 0) {
                	persIndex->setRelease(i);
                	needSave = true;
                    }
                }
                if (revisionTF->getText() != ecl::strf("%d", persIndex->getRevision())) {
                    int i = 0;
                    // check value - keep old value on error 
                    if ((sscanf(revisionTF->getText().c_str(),"%d", &i) == 1) &&
                	    i > 0) {
                	persIndex->setRevision(i);
                	needSave = true;
                    }
                }
                if (compatibilityTF->getText() != ecl::strf("%.2f", persIndex->getCompatibility())) {
                    double d = 0;
                    // check value - keep old value on error 
                    if ((sscanf(compatibilityTF->getText().c_str(),"%lg", &d) == 1) &&
                	    d >= 1) {
                	persIndex->setCompatibility(d);
                	needSave = true;
                    }
                }
            }
            
            // save
            if (needSave)
                persIndex->save();
            if (isNewIndex) {
                lev::Index::registerIndex(persIndex);
                lev::Index::setCurrentIndex(persIndex->getName());
            }
        }
        // regroup
        if (groupButton->get_value() != intialGroupPosition) {
            std::string newGroupName = groupButton->get_text(groupButton->get_value());
            // strip off square brackets used to mark default and pseudo groups
            if (newGroupName.size() > 2 && newGroupName[0] == '[' && 
                    newGroupName[newGroupName.size() -1] == ']') {
                newGroupName = newGroupName.substr(1, newGroupName.size() - 2);
            }
            packIndex->moveToGroup(newGroupName);
        } else if (isReasignOnly) {
            // the user did not reasign - take as an undo request
            undo_quit = true;
        }
        // relocate
        if (position != oldPosition)
            packIndex->locateBehind(position > 0 ? locationList[position - 1] : "");
        
        return true;
    }
    
    void LevelPackConfig::on_action(Widget *w) {
        if (w == but_back) {
            if (doChanges())
                Menu::quit();
            else
                invalidate_all();
        } else if (w == but_ignore) {
            if (packIndex->getName().empty()) {
                delete packIndex;
            }
            undo_quit = true;
            Menu::quit();
        } else if (w == but_update) {
            if (isPersistent && doChanges()) {
                persIndex->load(false, true);
                persIndex->save(true);
                Menu::quit();
            }
            invalidate_all();
        } else if (w == but_edit) {
            if (doChanges()) {
                LevelPackComposer m(isEditable);
                m.manage();
                Menu::quit();
            } else {
                invalidate_all();
            }
        } else if (w == scrollUp) {
            if (position > 0) {
                std::string tmp = locationList[position];
                locationList[position] = locationList[position - 1];
                locationList[position - 1] = tmp;
                position--;
                updateLocationList();
                invalidate_all();
            }
        } else if (w == scrollDown) {
            if (position < locationList.size() - 1) {
                std::string tmp = locationList[position];
                locationList[position] = locationList[position + 1];
                locationList[position + 1] = tmp;
                position++;
                updateLocationList();
                invalidate_all();
            }
        } else if (w == but_metadata && !didEditMetaData) {
            switchToMetadataEdit();
            invalidate_all();
        } else if (w == titleTF && w != NULL) {
            thisIndex->set_text(titleTF->getText());
        }
    }
    
    void LevelPackConfig::draw_background(ecl::GC &gc) {
        video::SetCaption(("Enigma - Level Pack Configuration"));
        blit(gc, 0,0, enigma::GetImage("menu_bg", ".jpg"));
    }
    

}} // namespace enigma::gui
