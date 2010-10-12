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

#include "lev/Index.hh"
#include "lev/VolatileIndex.hh"
#include "errors.hh"
#include "main.hh"
#include "options.hh"
#include "sound.hh"
#include "PreferenceManager.hh"
#include "StateManager.hh"
#include "lev/ScoreManager.hh"
#include "lev/RatingManager.hh"

#include <cstdio>

namespace enigma { namespace lev {

    std::map<std::string, Index *> Index::indices;
    std::map<std::string, std::vector<Index *> *> Index::indexGroups;

    Index * Index::currentIndex = NULL;
    std::string Index::currentGroup;
    std::map<std::string, std::string> Index::nullExtensions;
            
    void Index::initGroups() {
        ASSERT(indexGroups.empty(), XFrontend, "Reinitialization of groups");
        std::vector<std::string> groupNames = getGroupNames();
        for (int i = 0; i < groupNames.size(); i++) {
            std::vector<Index *> *group = new std::vector<Index *>;
            indexGroups.insert(std::make_pair(groupNames[i], group));
        }
        currentGroup = app.state->getString("CurrentGroup");
    }
    
    void Index::registerIndex(Index *anIndex) {
        if (anIndex == NULL)
            return;
        
        // check for uniqueness of index name
        if (findIndex(anIndex->getName()) != NULL)
            return;
            
        indices.insert(std::make_pair(anIndex->getName(), anIndex));
        
        
        // register index in state.xml and update current position, first with last values        
        std::string groupName = "";
        double stateLocation = 0;
        app.state->addIndex(anIndex->getName(), groupName, stateLocation, 
                anIndex->currentPosition, anIndex->screenFirstPosition);

        // user location for index?
        if (stateLocation > 0)
            anIndex->indexLocation = stateLocation;
            
        // reset positions that are out of range - this may happen due to
        // modified levelpacks (updates, deleted levels in auto, new commandline)
        if (anIndex->currentPosition < 0 || anIndex->currentPosition >= anIndex->size())
            anIndex->currentPosition = 0;

        // check user preferences for assigned group
        if (!groupName.empty())
            anIndex->indexGroup = groupName;  // use users preference
        else
            groupName = anIndex->indexGroup;  // use index default group
            
        std::vector<Index *> * group;
        
        // if no prefs ask for index default group
        
        // make new group if not existing
        if (groupName != INDEX_EVERY_GROUP) {
            std::map<std::string, std::vector<Index *> *>::iterator i = indexGroups.find(groupName);
            if (i != indexGroups.end()) {
                group = i->second;
            } else {
                // make the group
                group = new std::vector<Index *>;
                indexGroups.insert(std::make_pair(groupName, group));
                app.state->addGroup(groupName, anIndex->getName(), 0);
                
                // fill group with indices that appear in every group
                std::map<std::string, Index *>::iterator iti;
                for (iti = indices.begin(); iti != indices.end(); iti++)
                    if ((*iti).second->getGroupName() == INDEX_EVERY_GROUP)
                        addIndexToGroup((*iti).second, group);
            }
        }
        
        if (groupName != INDEX_EVERY_GROUP) {
            // insert according to user prefs or index defaults
            addIndexToGroup(anIndex, group);
            addIndexToGroup(anIndex, getGroup(INDEX_ALL_PACKS));        
        } else {
            // add index to all groups inclusive INDEX_ALL_PACKS
            std::map<std::string, std::vector<Index *> *>::iterator itg;
            for (itg = indexGroups.begin(); itg != indexGroups.end(); itg++)
                addIndexToGroup(anIndex, (*itg).second);
        }
        return;
    }
    
    void Index::addIndexToGroup(Index *anIndex, std::vector<Index *> * aGroup) {
        std::vector<Index *>::iterator itg;
        for (itg = aGroup->begin(); itg != aGroup->end() && 
                (*itg)->indexLocation <= anIndex->indexLocation; 
                itg++) {
        }
        aGroup->insert(itg, anIndex);
    }
    
    void Index::removeIndexFromGroup(Index *anIndex, std::string groupName) {
        std::vector<Index *> *theGroup = indexGroups[groupName];
        std::vector<Index *>::iterator itg;
        for (itg = theGroup->begin(); itg != theGroup->end(); itg++) {
            if ((*itg) == anIndex) {
                 theGroup->erase(itg);
                 return;
            }
        }   
    }
     
    Index * Index::findIndex(std::string anIndexName) {
        std::string::size_type lastChar = anIndexName.find_last_not_of(" ");
        if (lastChar == std::string::npos)
            // the name is effectively an empty string
            return NULL;

        // stip of trailing and leading spaces
        std::string name = anIndexName.substr(0 , lastChar + 1);
        name = name.substr(anIndexName.find_first_not_of(" "));
        
        std::map<std::string, Index *>::iterator i = indices.find(name);
        if (i != indices.end())
            return i->second;
        else 
            return NULL;
    }

    std::string Index::getCurrentGroup() {
        if (currentIndex == NULL)
            // initialize current group
            getCurrentIndex();
        return currentGroup;
    }
    
    void Index::setCurrentGroup(std::string groupName) {
        // set group - even "All Packs"
        app.state->setProperty("CurrentGroup", groupName);
        currentGroup = groupName;
        
        // set current index for desired group
        std::string indexName = getGroupSelectedIndex(groupName);
        Index * newIndex = findIndex(indexName);
        std::string indexGroupName;
        
        if (newIndex != NULL)
            indexGroupName = newIndex->getGroupName();
        
        if (newIndex != NULL && (indexGroupName == groupName ||
                indexGroupName == INDEX_EVERY_GROUP || 
                groupName == INDEX_ALL_PACKS)) {
            // set the groups current index as main current index
            setCurrentIndex(indexName);
        } else {
            // the groups current index is no longer available or did change the 
            // group -- reset the groups current index
            std::vector<Index *> * group  = getGroup(groupName);
            if (group->size() > 0) {
                setCurrentIndex((*group)[0]->getName());
            } else {
                // the group is empty -- delete group current index entry and
                // leave the apps current index unchanged
                setGroupSelectedIndex(groupName,"");
            }
        }
//        Log << "Index setCurrentGroup: wanted " << groupName << " - got " << currentGroup << " - idxGroup " << indexGroupName <<"\n";
    }
    
    std::vector<std::string> Index::getGroupNames() {
        std::vector<std::string> names;
        app.state->getGroupNames(&names);
        return names;
    }
    
    void Index::deleteGroup(std::string groupName) {
        std::vector<Index *> * theGroup = getGroup(groupName);
        if (theGroup != NULL) {
            indexGroups.erase(groupName);
            delete theGroup;
        }

        if (currentGroup == groupName) {
            std::vector<std::string> groups = getGroupNames();
            for (int i = 0; i < groups.size(); i++) {
                if (groups[i] == groupName) {
                    if (i > 0) {
                        setCurrentGroup(groups[i-1]);
                    } else {
                        ASSERT (groups.size() > 1, XFrontend, "Delete of last existing group.");
                        setCurrentGroup(groups[1]);
                    }
                    break;
                } 
            }
        }
        app.state->deleteGroup(groupName);
    }
    
    void Index::moveGroup(std::string groupName, int newPos) {
        std::string indexName = app.state->getGroupSelectedIndex(groupName);
        std::string column = app.state->getGroupSelectedColumn(groupName);
        app.state->deleteGroup(groupName);
        app.state->insertGroup(newPos, groupName, indexName, column);
    }
    
    void Index::renameGroup(std::string oldName, std::string newName) {
        // rename state group element
        app.state->renameGroup(oldName, newName);

        // rename map of groups
        std::vector<Index *> * group = getGroup(oldName);
        indexGroups.erase(oldName);
        indexGroups.insert(std::make_pair(newName, group));
        
        // rename group name in indices
        for (int i = 0; i < group->size(); i++) {
            if ((*group)[i]->getGroupName() == oldName) {
                (*group)[i]->indexGroup = newName;
                // set group as users choice for index in state
                app.state->setIndexGroup((*group)[i]->getName(), newName);
            }
        }
        
        // handle currentGroup
        if (currentGroup == oldName) {
            currentGroup = newName;
            app.state->setProperty("CurrentGroup", newName);
        }
    }

    void Index::insertGroup(std::string groupName, int newPos) {
        // make the group
        std::vector<Index *> *group = new std::vector<Index *>;
        indexGroups.insert(std::make_pair(groupName, group));
        app.state->insertGroup(newPos, groupName, "", "");
        
        // fill group with indices that appear in every group
        std::map<std::string, Index *>::iterator iti;
        for (iti = indices.begin(); iti != indices.end(); iti++)
            if ((*iti).second->getGroupName() == INDEX_EVERY_GROUP)
                addIndexToGroup((*iti).second, group);
        
        setCurrentGroup(groupName);
    }
    
    std::string Index::getGroupSelectedIndex(std::string groupName) {
        return app.state->getGroupSelectedIndex(groupName);
    }
    
    int Index::getGroupSelectedColumn(std::string groupName) {
        std::string columnString = app.state->getGroupSelectedColumn(groupName);
        if (columnString.empty())
            return INDEX_GROUP_COLUMN_UNKNOWN;
        else {
            int col = INDEX_GROUP_COLUMN_UNKNOWN;
            std::sscanf(columnString.c_str(), "%i", &col);
            return col;
        }
    }
    
    void Index::setGroupSelectedIndex(std::string groupName, std::string indexName) {
        app.state->setGroupSelectedIndex(groupName, indexName);
    }
    
    void Index::setGroupSelectedColumn(std::string groupName, int column) {
        if (column == INDEX_GROUP_COLUMN_UNKNOWN)       
            app.state->setGroupSelectedColumn(groupName, "");
        else
            app.state->setGroupSelectedColumn(groupName, ecl::strf("%d",column));
    }
    
    Index * Index::getCurrentIndex() {
        if (currentIndex == NULL) {
            // first look for user preference
            if (setCurrentIndex(app.state->getGroupSelectedIndex(
                    app.state->getString("CurrentGroup"))))
                ;
            
            // fallback to "Tutorial" pack
            else if (setCurrentIndex("Tutorial"))
                ;
                        
            // fallback to any pack
            else if (!indices.empty()) {
                setCurrentIndex(indices.begin()->second->getName());
            }
            // add empty pack
            else {
                std::vector<std::string> emptyList;
                registerIndex(new lev::VolatileIndex("Empty Index",
                        INDEX_DEFAULT_GROUP, emptyList));
                setCurrentIndex("Empty Index");
            }
        }
        return currentIndex;
    }

    bool Index::setCurrentIndex(std::string anIndexName) {
        Index * newIndex = findIndex(anIndexName);
        if (newIndex != NULL) {
            if (newIndex != currentIndex) {
                sound::SetDefaultSoundSet(newIndex->get_default_SoundSet());
                currentIndex = newIndex;
                std::string group = currentIndex->getGroupName();
                if (group != INDEX_EVERY_GROUP && 
                        app.state->getString("CurrentGroup") != INDEX_ALL_PACKS) {
                    app.state->setProperty("CurrentGroup", group);
                    currentGroup = group;
                }
                if (getGroupSelectedIndex(currentGroup) != currentIndex->getName()) {
                    setGroupSelectedIndex(currentGroup, currentIndex->getName());
                    setGroupSelectedColumn(currentGroup, INDEX_GROUP_COLUMN_UNKNOWN);
                }
            }
            return true;
        }
        return false;
    }
    
    Index * Index::nextGroupIndex() {
        std::vector<Index *> * curGroup = getGroup(currentGroup);
        ASSERT(curGroup != NULL, XFrontend, "");
        
        for (int i = 0; i < curGroup->size() - 1; i++) {
            if ((*curGroup)[i] == currentIndex)
                return (*curGroup)[i+1];
        }
        return currentIndex;
    }
    
    Index * Index::previousGroupIndex() {
        std::vector<Index *> * curGroup = getGroup(currentGroup);
        ASSERT(curGroup != NULL, XFrontend, "");

        for (int i = 1; i < curGroup->size(); i++) {
            if ((*curGroup)[i] == currentIndex)
                return (*curGroup)[i-1];
        }
        return currentIndex;
    }

    Proxy * Index::getCurrentProxy() {
        return getCurrentIndex()->getCurrent();
    }
    
    std::vector<Index *> * Index::getGroup(std::string groupName) {
        std::map<std::string, std::vector<Index *> *>::iterator i = indexGroups.find(groupName);
        if (i != indexGroups.end()) 
            return i->second;
        else 
            return NULL;
    }

    double Index::getNextUserLocation() {
        double lastUsed = INDEX_USER_PACK_LOCATION;
        std::map<std::string, Index *>::iterator iti;
        for (iti = indices.begin(); iti != indices.end(); iti++) {
            double idxLocation = (*iti).second->indexLocation;
            if (idxLocation > lastUsed && idxLocation < INDEX_DEFAULT_PACK_LOCATION) {
                lastUsed = idxLocation;
            }
        }
        if (lastUsed + 999 < INDEX_DEFAULT_PACK_LOCATION)
            return lastUsed + 100;
        else
            return 0.9 * lastUsed + INDEX_DEFAULT_PACK_LOCATION / 10;
    }
    
    
    Index::Index(std::string anIndexName, std::string aGroupName, double defaultLocation) : 
            indexName (anIndexName), indexGroup (aGroupName), defaultGroup (aGroupName),
            indexLocation (defaultLocation), indexDefaultLocation (defaultLocation),
            currentPosition (0), screenFirstPosition (0) {
    }
    
    Index::~Index() {}
    
    std::string Index::getName() {
        return indexName;
    }

    std::string Index::getGroupName() {
        return indexGroup;
    }
    
    std::string Index::getDefaultGroupName() {
        return defaultGroup;
    }
    
    double Index::getLocation() {
        return indexLocation;
    }
    
    double Index::getDefaultLocation() {
        return indexDefaultLocation;
    }
    
    void Index::setDefaultLocation(double defLocation) {
        indexDefaultLocation = defLocation;
    }

    void Index::moveToGroup(std::string newGroupName) {
        // remove from old group
        if (indexGroup != INDEX_EVERY_GROUP) {
            // remove index from the unique group
            removeIndexFromGroup(this, indexGroup);
            removeIndexFromGroup(this, INDEX_ALL_PACKS);      
        } else {
            // remove index from all groups inclusive INDEX_ALL_PACKS
            std::vector<std::string> groupNames = getGroupNames();
            for (int i = 0; i < groupNames.size(); i++)
                removeIndexFromGroup(this, groupNames[i]);
            // declare this index as not belonging to any group
            indexGroup = "";
        }
        
        // create group if not existing
        if (newGroupName != INDEX_EVERY_GROUP) {
            std::map<std::string, std::vector<Index *> *>::iterator i = indexGroups.find(newGroupName);
            if (i == indexGroups.end()) {
                // make the group
                std::vector<Index *> *group = new std::vector<Index *>;
                indexGroups.insert(std::make_pair(newGroupName, group));
                app.state->addGroup(newGroupName, indexName, 0);
                
                // fill group with indices that appear in every group
                std::map<std::string, Index *>::iterator iti;
                for (iti = indices.begin(); iti != indices.end(); iti++)
                    if ((*iti).second->getGroupName() == INDEX_EVERY_GROUP)
                        addIndexToGroup((*iti).second, group);
            }
        }
        
        
        indexGroup = newGroupName;
        
        // add to new group
        if (newGroupName != INDEX_EVERY_GROUP) {
            // insert according to user prefs or index defaults
            addIndexToGroup(this, getGroup(newGroupName));
            addIndexToGroup(this, getGroup(INDEX_ALL_PACKS));        
        } else {
            // add index to all groups inclusive INDEX_ALL_PACKS
            std::map<std::string, std::vector<Index *> *>::iterator itg;
            for (itg = indexGroups.begin(); itg != indexGroups.end(); itg++)
                addIndexToGroup(this, (*itg).second);
        }
               
        // store new group as users state
        app.state->setIndexGroup(indexName,
                newGroupName == defaultGroup ? "" : newGroupName);

        // select this index with its new group if it is the current Index
        if (this == currentIndex) {
            if (indexGroup != INDEX_EVERY_GROUP && 
                    app.state->getString("CurrentGroup") != INDEX_ALL_PACKS) {
                app.state->setProperty("CurrentGroup", indexGroup);
                    currentGroup = indexGroup;
                }
                if (getGroupSelectedIndex(currentGroup) != currentIndex->getName()) {
                    setGroupSelectedIndex(currentGroup, currentIndex->getName());
                    setGroupSelectedColumn(currentGroup, INDEX_GROUP_COLUMN_UNKNOWN);
                }
        }
    }
    
    int indexLocationCompare(Index * first, Index * second) {
        return first->getLocation() < second->getLocation();
    }
    
    void Index::locateBehind(std::string predName) {
        double predLocation = 0;
        double succLocation = 0;
        double newLocation;
        std::string predGroup;
        std::string succGroup;
        std::vector<Index *> * allGroup = getGroup(INDEX_ALL_PACKS);
        if (predName.empty()) {
            succLocation = (*allGroup)[0]->indexLocation;
            succGroup = (*allGroup)[0]->indexGroup;
        } else {
            for (int i = 0; i < allGroup->size(); i++) {
                if ((*allGroup)[i]->getName() == predName) {
                    predLocation = (*allGroup)[i]->indexLocation;
                    predGroup = (*allGroup)[i]->indexGroup;
                    int succ = 0;
                    if ((i+1 < allGroup->size()) && ((*allGroup)[i+1] != this)) {
                        succ = i + 1;
                    } else if (i+2 < allGroup->size()) {
                        succ = i + 2;
                    }
                    if (succ > 0) {
                        succLocation = (*allGroup)[succ]->indexLocation;
                        succGroup = (*allGroup)[succ]->indexGroup;
                    }
                    break;
                }
            }
        }
        ASSERT (!(predLocation == 0 && succLocation == 0), XFrontend, "");
        if (predLocation == 0) {
            if (succGroup == indexGroup) {
                newLocation = succLocation * 0.98;
            } else {
                newLocation = succLocation * 0.75;
            }
        } else if (succLocation == 0) {
            if (predGroup == indexGroup) {
                newLocation = predLocation + 100;
            } else {
                newLocation = predLocation + 10000;
            }
        } else if ((predGroup == indexGroup && succGroup == indexGroup) ||
                (predGroup != indexGroup && succGroup != indexGroup)){
            newLocation = (predLocation + succLocation) / 2;
        } else if (predGroup == indexGroup) {
            newLocation = 0.95 * predLocation + 0.05 * succLocation;
        } else {
            newLocation = 0.05 * predLocation + 0.95 * succLocation;
        }
//        Log << "newLocation " << newLocation << "\n";
        indexLocation = newLocation;
        app.state->setIndexLocation(indexName, indexLocation);
        
        // reorder all groups according to new location
        std::map<std::string, std::vector<Index *> *>::iterator itg;
        for (itg = indexGroups.begin(); itg != indexGroups.end(); itg++) 
            std::sort((*itg).second->begin(), (*itg).second->end(), indexLocationCompare);
    }
    
    void Index::renameIndex(std::string newName) {
        indices.erase(indexName);
        indices[newName] = this;
        app.state->setIndexName(indexName, newName);
        indexName = newName;
    }
    
    bool Index::isSource(Proxy *) {
        return false;
    }
    
    int Index::getCurrentPosition() {
        return currentPosition;
    }
    
    int Index::getCurrentLevel() {
        return currentPosition + 1;
    }
    
    Proxy * Index::getCurrent() {
        return getProxy(currentPosition);
    }
    
    void Index::setCurrentPosition(int newPos) {
        // reset positions that are out of range - this may happen due to
        // editable Indices
        if (newPos < 0 || newPos > size())
            newPos = 0;

        //
        currentPosition = newPos;
        app.state->setIndexCurpos(getName(), currentPosition);
    }

    int Index::getScreenFirstPosition() {
        return screenFirstPosition;
    }
    
    void Index::setScreenFirstPosition(int iFirstPos) {
        screenFirstPosition = iFirstPos;
        app.state->setIndexCurfirst(getName(), screenFirstPosition);
    }
    
    bool Index::mayPlayLevel(int levelNumber) {
        return true;
    }
    
    Proxy * Index::getProxy(int pos) {
        if (pos >= 0 && pos < proxies.size())
            return proxies[pos];
        else
            return NULL;
    }
    
    bool Index::containsProxy(Proxy * aProxy) {
        for (int i = 0; i < proxies.size(); i++) {
            if (proxies[i] == aProxy)
                return true;
        }
        return false;
    }
    
    bool Index::hasNormLevelPath(std::string path) {
        for (int i = 0; i < proxies.size(); i++) {
            if (proxies[i]->getNormLevelPath() == path)
                return true;
        }
        return false;
    }
    
    bool Index::advanceLevel(LevelAdvanceMode advMode) {
        NextLevelMode nextMode = static_cast<NextLevelMode>(app.state->getInt("NextLevelMode"));
    
        switch (advMode) {
        case ADVANCE_STRICTLY:
            nextMode = NEXT_LEVEL_STRICTLY;
            break;
        case ADVANCE_UNSOLVED:
            nextMode = NEXT_LEVEL_UNSOLVED;
            break;
        default:
            break;
        };

        bool found = false;
        const int max = size();
        int newPos = currentPosition;
        lev::ScoreManager *scm = lev::ScoreManager::instance();
        lev::RatingManager *ratingMgr = lev::RatingManager::instance();
        int    difficulty     = app.state->getInt("Difficulty");
        
        while (newPos < max - 1 && !found) {
            ++newPos;
    
            if (nextMode == NEXT_LEVEL_UNSOLVED || nextMode == NEXT_LEVEL_NOT_BEST ||
                    nextMode ==  NEXT_LEVEL_OVER_PAR) {
                bool solved = scm->isSolved(proxies[newPos], difficulty);
                if (!solved) // always play unsolved levels
                    found = true;
                else { // solved levels
                    if (nextMode == NEXT_LEVEL_NOT_BEST) {
                        int  par_time = ratingMgr->getBestScore(proxies[newPos], difficulty);
                        int  best_user_time = scm->getBestUserScore(proxies[newPos], difficulty);
                        if (best_user_time<0 || (par_time>0 && best_user_time>par_time))
                            found = true;
                    } else if (nextMode == NEXT_LEVEL_OVER_PAR) {
                        int  par_time = ratingMgr->getParScore(proxies[newPos], difficulty);
                        int  best_user_time = scm->getBestUserScore(proxies[newPos], difficulty);
                        if (best_user_time<0 || (par_time>0 && best_user_time>par_time))
                            found = true;
                    }
               }
            }
            else
                found = true;
        }
        if (!found)
            newPos = 0; // ?
            
        currentPosition = newPos;
        return found;        
    }
    
    int Index::size() const { 
        return proxies.size();
    }

    void Index::appendProxy(Proxy * newLevel, controlType varCtrl,
                scoreUnitType varUnit, std::string varTarget,
                std::map<std::string, std::string> varExtensions) {
        proxies.push_back(newLevel);
    }

    void Index::clear() {
//        proxies.clear();
    }
    
    void Index::updateFromProxies() {
        for (int i = 0, l = proxies.size();  i < l; i++) {
            try {
                proxies[i]->loadMetadata(true);
            } catch (XLevelLoading &err) {
                // silently ignore errors
            }
        }
    }

    /* ---------- LevelPack interface ---------- */
    

    /*! Return the default SoundSet (see options::SoundSet for meaning) */
    const char* Index::get_default_SoundSet() const { 
        return "Enigma";
    }

    /*! Returns true if it's a twoplayer levelpack, but has no
      it-yinyang (needed to add it-yinyang to inventory if
      oxyd-linkgame is played as single-player) */
    bool Index::needs_twoplayers() const { 
        return false;
    }

}} // namespace enigma::lev
