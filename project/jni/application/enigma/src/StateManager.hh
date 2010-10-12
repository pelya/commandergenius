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
 
#ifndef STATEMGR_HH_INCLUDED
#define STATEMGR_HH_INCLUDED

#include "PropertyManager.hh"
#include <string>
#include <vector>
#include <xercesc/dom/DOMElement.hpp>

namespace enigma
{
    /**
     * A singelton manager for state info stored in XML format. 
     * <p>The singleton can be accessed via standard static instance() method or
     * the application public ivar <code>app.state</code>.</p>
     *
     * <p>The storage location of the state is determined by the
     * Application object.</p>
     * <p>The set and get methods for properties take utf8 encoded property
     * names and operate with utf8 encoded string values. Set methods create new
     * state property elements if necessary. Get methods do not modify
     * reference values and return C++ default values if the named property
     * does not exist.</p>
     */
    class StateManager : public PropertyManager {
    public:
        static StateManager *instance();
        ~StateManager();
        virtual bool save();
        void shutdown();
        
        void getGroupNames(std::vector<std::string> *names);
        std::string getGroupSelectedIndex(std::string groupName);
        std::string getGroupSelectedColumn(std::string groupName);
        void setGroupSelectedIndex(std::string groupName, std::string indexName);
        void setGroupSelectedColumn(std::string groupName, std::string column);
        void addGroup(std::string groupName, std::string indexName, int column); // update or append
        void insertGroup(int pos, std::string groupName, std::string indexName, std::string column); // no duplicate check, pos -1 is append
        void deleteGroup(std::string groupName);
        void renameGroup(std::string oldName, std::string newName);
        
        void addIndex(std::string indexName, std::string &groupName, double &location,
                int &curpos, int &curfirst);
        void setIndexName(std::string oldName, std::string newName);
        void setIndexLocation(std::string indexName, double location);
        void setIndexCurpos(std::string indexName, int curpos);
        void setIndexCurfirst(std::string indexName, int curfirst);
        void setIndexGroup(std::string indexName, std::string groupName);
        
        std::string getAnnotation(std::string id);
        void setAnnotation(std::string id, std::string annotation);
        
    protected:
        StateManager();
    private:
        static StateManager *theSingleton;
        XERCES_CPP_NAMESPACE_QUALIFIER DOMElement *groupsElem;
        XERCES_CPP_NAMESPACE_QUALIFIER DOMNodeList *groupList;
        XERCES_CPP_NAMESPACE_QUALIFIER DOMElement *indicesElem;
        XERCES_CPP_NAMESPACE_QUALIFIER DOMNodeList *indexList;
        XERCES_CPP_NAMESPACE_QUALIFIER DOMElement *levelsElem;
        
        XERCES_CPP_NAMESPACE_QUALIFIER DOMElement *getLevel(std::string id);
    };
} // namespace enigma
#endif
