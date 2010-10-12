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

#include "StateManager.hh"
#include "errors.hh"
#include "main.hh"
#include "DOMErrorReporter.hh"
#include "DOMSchemaResolver.hh"
#include "LocalToXML.hh"
#include "nls.hh"
#include "options.hh"
#include "Utf8ToXML.hh"
#include "utilXML.hh"
#include "XMLtoLocal.hh"
#include "XMLtoUtf8.hh"
#include "ecl_system.hh"
#include "gui/ErrorMenu.hh"
#include <cstdio>
#include <iostream>
#include <sstream>
#include <xercesc/dom/DOM.hpp>
#include <xercesc/util/XMLDouble.hpp>
#include <xercesc/util/XMLString.hpp>
#include <xercesc/util/XMLUniDefs.hpp>
#include <xercesc/util/PlatformUtils.hpp>
#include <xercesc/util/XercesVersion.hpp>
#if _XERCES_VERSION < 30000
#include <xercesc/framework/LocalFileFormatTarget.hpp>
#endif


using namespace std;
using namespace enigma;
XERCES_CPP_NAMESPACE_USE 

namespace enigma {
    StateManager *StateManager::theSingleton = 0;
    
    StateManager* StateManager::instance() {
        if (theSingleton == 0) {
            theSingleton = new StateManager();
        }
        return theSingleton;
    }
    
    StateManager::StateManager() {
        std::string statePath;
        std::string errMessage;
        
        if (!app.resourceFS->findFile( "state.xml" , statePath)) {
            if (!app.systemFS->findFile( "schemas/state.xml" , statePath)) {
                throw XFrontend("Cannot load application state template!");
            }
        }

        try {
            std::ostringstream errStream;
            app.domParserErrorHandler->resetErrors();
            app.domParserErrorHandler->reportToOstream(&errStream);
            app.domParserSchemaResolver->resetResolver();
            app.domParserSchemaResolver->addSchemaId("state.xsd","state.xsd");

            doc = app.domParser->parseURI(statePath.c_str());
            if (doc != NULL && !app.domParserErrorHandler->getSawErrors()) {
                propertiesElem = dynamic_cast<DOMElement *>(doc->getElementsByTagName(
                        Utf8ToXML("properties").x_str())->item(0));
                groupsElem = dynamic_cast<DOMElement *>(doc->getElementsByTagName(
                        Utf8ToXML("groups").x_str())->item(0));
                groupList = groupsElem->getElementsByTagName(
                        Utf8ToXML("group").x_str());
                indicesElem = dynamic_cast<DOMElement *>(doc->getElementsByTagName(
                        Utf8ToXML("indices").x_str())->item(0));
                indexList = indicesElem->getElementsByTagName(
                        Utf8ToXML("index").x_str());
                levelsElem = dynamic_cast<DOMElement *>(doc->getElementsByTagName(
                        Utf8ToXML("levels").x_str())->item(0));
            }
            if(app.domParserErrorHandler->getSawErrors()) {
                errMessage = errStream.str();
            }
            app.domParserErrorHandler->reportToNull();  // do not report to errStream any more
        }
        catch (...) {
            errMessage = "Unexpected XML Exception on load of state\n";
        }
        if (!errMessage.empty()) {
            throw XFrontend("Cannot load application state file: " + statePath +
                    "\nError: " + errMessage);
        }
    }
     
    StateManager::~StateManager() {
        if (doc != NULL)
            shutdown();
    }
    
    bool StateManager::save() {
        bool result = true;
        std::string errMessage;
        
        if (doc == NULL)
            return true;

        int count = getInt("Count");
        setProperty("Count", ++count);

        stripIgnorableWhitespace(doc->getDocumentElement());
        std::string path = app.userPath + "/state.xml";
        std::string pathBackup = app.userPath + "/backup/state.xml";
        
        // backup state every 10th save 
        if (count%10 == 0) {
            std::remove((pathBackup + "~2").c_str());
            std::remove((path + "~2").c_str()); // 1.00 bakups
            if (ecl::FileExists(path + "~1")) {
                if (std::difftime(ecl::FileModTime(path + "~1"),
                        ecl::FileModTime(pathBackup + "~1")) > 0) {
                    // backup 1 from 1.00 is newer than backup 1 on backup path
                    if (Copyfile(path + "~1", pathBackup + "~2"))
                        std::remove((path + "~1").c_str()); // 1.00 bakup
                } else {
                    // just in case off previous copy failure
                    std::rename((pathBackup + "~1").c_str(), (pathBackup + "~2").c_str());
                    std::remove((path + "~1").c_str()); // 1.00 bakup
                }
            } else {
                std::rename((pathBackup + "~1").c_str(), (pathBackup + "~2").c_str());
            }
            Copyfile(path, pathBackup + "~1");
        }

        try {
#if _XERCES_VERSION >= 30000
            result = app.domSer->writeToURI(doc, LocalToXML(& path).x_str());
#else
            XMLFormatTarget *myFormTarget = new LocalFileFormatTarget(path.c_str());
            result = app.domSer->writeNode(myFormTarget, *doc);            
            delete myFormTarget;   // flush
#endif
        } catch (const XMLException& toCatch) {
            errMessage = std::string("Exception on save of state: \n") + 
                    XMLtoUtf8(toCatch.getMessage()).c_str() + "\n";
            result = false;
        } catch (const DOMException& toCatch) {
            errMessage = std::string("Exception on save of state: \n") + 
                    XMLtoUtf8(toCatch.getMessage()).c_str() + "\n";
            result = false;
        } catch (...) {
            errMessage = "Unexpected exception on save of state\n" ;
            result = false;
        }

        if (!result) {
            if (count%10 == 0) {
                // restore backup in case of error
                if (Copyfile(pathBackup + "~1", path)) {
                    std::remove((pathBackup + "~1").c_str());
                    std::rename((pathBackup + "~2").c_str(), (pathBackup + "~1").c_str());
                }
            }
            cerr << XMLtoLocal(Utf8ToXML(errMessage.c_str()).x_str()).c_str();
            gui::ErrorMenu m(errMessage, N_("Continue"));
            m.manage();          
        } else
            Log << "State save o.k.\n";
        
        return result;
    }

    void StateManager::shutdown() {
        save();
        if (doc != NULL)
            doc->release();
        doc = NULL;
    }

    void StateManager::getGroupNames(std::vector<std::string> *names) {
        for (int i = 0, l = groupList-> getLength(); i < l; i++) {
            DOMElement * group = dynamic_cast<DOMElement *>(groupList->item(i));
            names->push_back(XMLtoUtf8(group->getAttribute(Utf8ToXML("title").x_str())).c_str());
        }
    }
    
    std::string StateManager::getGroupSelectedIndex(std::string groupName) {
        for (int i = 0, l = groupList-> getLength(); i < l; i++) {
            DOMElement * group = dynamic_cast<DOMElement *>(groupList->item(i));
            if (groupName == XMLtoUtf8(group->getAttribute(Utf8ToXML("title").x_str())).c_str())
                return XMLtoUtf8(group->getAttribute(Utf8ToXML("curindex").x_str())).c_str();
        }
        return "";
    }
    
    std::string StateManager::getGroupSelectedColumn(std::string groupName) {
        for (int i = 0, l = groupList-> getLength(); i < l; i++) {
            DOMElement * group = dynamic_cast<DOMElement *>(groupList->item(i));
            if (groupName == XMLtoUtf8(group->getAttribute(Utf8ToXML("title").x_str())).c_str())
                return XMLtoUtf8(group->getAttribute(Utf8ToXML("curcolumn").x_str())).c_str();
        }
        return 0;
    }

    void StateManager::setGroupSelectedIndex(std::string groupName, std::string indexName) {
        for (int i = 0, l = groupList-> getLength(); i < l; i++) {
            DOMElement * group = dynamic_cast<DOMElement *>(groupList->item(i));
            if (groupName == XMLtoUtf8(group->getAttribute(Utf8ToXML("title").x_str())).c_str()) {
                group->setAttribute(Utf8ToXML("curindex").x_str(), Utf8ToXML(indexName).x_str());
                return;
            }
        }
    }
    
    void StateManager::setGroupSelectedColumn(std::string groupName, std::string column) {
        for (int i = 0, l = groupList-> getLength(); i < l; i++) {
            DOMElement * group = dynamic_cast<DOMElement *>(groupList->item(i));
            if (groupName == XMLtoUtf8(group->getAttribute(Utf8ToXML("title").x_str())).c_str()) {
                group->setAttribute(Utf8ToXML("curcolumn").x_str(), Utf8ToXML(column).x_str());
                return;
            }
        }
    }
    
    void StateManager::addGroup(std::string groupName, std::string indexName, int column) {
        // check if group exists - update attributes only
        for (int i = 0, l = groupList->getLength(); i < l; i++) {
            DOMElement * group = dynamic_cast<DOMElement *>(groupList->item(i));
            if (groupName == XMLtoUtf8(group->getAttribute(Utf8ToXML("title").x_str())).c_str()) {
                group->setAttribute(Utf8ToXML("curcolumn").x_str(), 
                        Utf8ToXML(ecl::strf("%d",column)).x_str());
                group->setAttribute(Utf8ToXML("curindex").x_str(), Utf8ToXML(indexName).x_str());
                return;
            }
        }
        // if group does not exist add a new element
        DOMElement * group = doc->createElement (Utf8ToXML("group").x_str());
        group->setAttribute(Utf8ToXML("title").x_str(), Utf8ToXML(groupName).x_str());
        group->setAttribute(Utf8ToXML("curindex").x_str(), Utf8ToXML(indexName).x_str());
        group->setAttribute(Utf8ToXML("curcolumn").x_str(), 
                Utf8ToXML(ecl::strf("%d",column)).x_str());
        groupsElem->appendChild(group);
    }

    void StateManager::insertGroup(int pos, std::string groupName, 
            std::string indexName, std::string column) {
        DOMElement * group = doc->createElement (Utf8ToXML("group").x_str());
        group->setAttribute(Utf8ToXML("title").x_str(), Utf8ToXML(groupName).x_str());
        group->setAttribute(Utf8ToXML("curindex").x_str(), Utf8ToXML(indexName).x_str());
        group->setAttribute(Utf8ToXML("curcolumn").x_str(), Utf8ToXML(column).x_str());
        if (pos < 0 || pos >= groupList->getLength())
            groupsElem->appendChild(group);
        else  {
            DOMElement * nextGroup = dynamic_cast<DOMElement *>(groupList->item(pos));
            groupsElem->insertBefore(group, nextGroup);
        }
    }
    
    void StateManager::deleteGroup(std::string groupName) {
        for (int i = 0, l = groupList->getLength(); i < l; i++) {
            DOMElement * group = dynamic_cast<DOMElement *>(groupList->item(i));
            if (groupName == XMLtoUtf8(group->getAttribute(Utf8ToXML("title").x_str())).c_str()) {
                groupsElem->removeChild(group);
                return;
            }
        }
    }

    void StateManager::renameGroup(std::string oldName, std::string newName) {
        // rename group element
        for (int i = 0, l = groupList->getLength(); i < l; i++) {
            DOMElement * group = dynamic_cast<DOMElement *>(groupList->item(i));
            if (oldName == XMLtoUtf8(group->getAttribute(Utf8ToXML("title").x_str())).c_str()) {
                group->setAttribute(Utf8ToXML("title").x_str(), Utf8ToXML(newName).x_str());
                break;
            }
        }
    }
    
    
    void StateManager::addIndex(std::string indexName, std::string &groupName, 
            double &location, int &curpos, int &curfirst) {
        // check if index exists - get user attributes
        for (int i = 0, l = indexList-> getLength(); i < l; i++) {
            DOMElement * index = dynamic_cast<DOMElement *>(indexList->item(i));
            if (indexName == XMLtoUtf8(index->getAttribute(Utf8ToXML("title").x_str())).c_str()) {
                groupName = XMLtoUtf8(index->getAttribute(Utf8ToXML("group").x_str())).c_str();
                XMLDouble * result = new XMLDouble(index->getAttribute(Utf8ToXML("location").x_str()));
                location = result->getValue();
                delete result;
                curpos = XMLString::parseInt(index->getAttribute(Utf8ToXML("curposition").x_str()));
                curfirst = XMLString::parseInt(index->getAttribute(Utf8ToXML("curfirst").x_str()));
                return;
            }
        }
        // if index does not exist add a new element with default values
        DOMElement * index = doc->createElement (Utf8ToXML("index").x_str());
        index->setAttribute(Utf8ToXML("title").x_str(), Utf8ToXML(indexName).x_str());
        index->setAttribute(Utf8ToXML("group").x_str(), Utf8ToXML(groupName).x_str());
        index->setAttribute(Utf8ToXML("location").x_str(), Utf8ToXML(ecl::strf("%g",location)).x_str());
        index->setAttribute(Utf8ToXML("curfirst").x_str(), Utf8ToXML(ecl::strf("%d",0)).x_str());
        index->setAttribute(Utf8ToXML("curposition").x_str(), Utf8ToXML(ecl::strf("%d",0)).x_str());
        indicesElem->appendChild(index);
    }
    
    void StateManager::setIndexName(std::string oldName, std::string newName) {
        // search index and set attribute
        for (int i = 0, l = indexList-> getLength(); i < l; i++) {
            DOMElement * index = dynamic_cast<DOMElement *>(indexList->item(i));
            if (oldName == XMLtoUtf8(index->getAttribute(Utf8ToXML("title").x_str())).c_str()) {
                index->setAttribute(Utf8ToXML("title").x_str(), Utf8ToXML(newName).x_str());
                return;
            }
        }        
    }
    
    void StateManager::setIndexLocation(std::string indexName, double location) {
        // search index and set attribute
        for (int i = 0, l = indexList-> getLength(); i < l; i++) {
            DOMElement * index = dynamic_cast<DOMElement *>(indexList->item(i));
            if (indexName == XMLtoUtf8(index->getAttribute(Utf8ToXML("title").x_str())).c_str()) {
                index->setAttribute(Utf8ToXML("location").x_str(), Utf8ToXML(ecl::strf("%.15g",location)).x_str());
                return;
            }
        }
    }
    
    void StateManager::setIndexCurpos(std::string indexName, int curpos) {
        // search index and set attribute
        for (int i = 0, l = indexList-> getLength(); i < l; i++) {
            DOMElement * index = dynamic_cast<DOMElement *>(indexList->item(i));
            if (indexName == XMLtoUtf8(index->getAttribute(Utf8ToXML("title").x_str())).c_str()) {
                index->setAttribute(Utf8ToXML("curposition").x_str(), Utf8ToXML(ecl::strf("%d",curpos)).x_str());
                return;
            }
        }
    }
    
    void StateManager::setIndexCurfirst(std::string indexName, int curfirst) {
        // search index and set attribute
        for (int i = 0, l = indexList-> getLength(); i < l; i++) {
            DOMElement * index = dynamic_cast<DOMElement *>(indexList->item(i));
            if (indexName == XMLtoUtf8(index->getAttribute(Utf8ToXML("title").x_str())).c_str()) {
                index->setAttribute(Utf8ToXML("curfirst").x_str(), Utf8ToXML(ecl::strf("%d",curfirst)).x_str());
                return;
            }
        }
    }
           
    void StateManager::setIndexGroup(std::string indexName, std::string groupName) {
        // search index and set attribute
        for (int i = 0, l = indexList-> getLength(); i < l; i++) {
            DOMElement * index = dynamic_cast<DOMElement *>(indexList->item(i));
            if (indexName == XMLtoUtf8(index->getAttribute(Utf8ToXML("title").x_str())).c_str()) {
                index->setAttribute(Utf8ToXML("group").x_str(), Utf8ToXML(groupName).x_str());
                return;
            }
        }        
    }
    
    
    std::string StateManager::getAnnotation(std::string id) {
        DOMElement * level = getLevel(id);
        if (level != NULL)
            return XMLtoUtf8(level->getAttribute(Utf8ToXML("annotation").x_str())).c_str(); 
        else
            return "";
    }
    
    void StateManager::setAnnotation(std::string id, std::string annotation) {
        DOMElement * level = getLevel(id);
        if (level == NULL) {
            level = doc->createElement (Utf8ToXML("level").x_str());
            level->setAttribute(Utf8ToXML("id").x_str(), Utf8ToXML(id).x_str());
            levelsElem->appendChild(level);
        }   
        level->setAttribute(Utf8ToXML("annotation").x_str(), Utf8ToXML(annotation).x_str());
    }
    
    DOMElement * StateManager::getLevel(std::string id) {
        XMLCh * xmlId = XMLString::replicate(Utf8ToXML(id).x_str());
        bool levelFound = false;
        DOMElement * level;
        DOMNodeList * levelList = levelsElem->getElementsByTagName(Utf8ToXML("level").x_str());
        for (int i = 0, l = levelList-> getLength(); i < l && !levelFound; i++) {
            level  = dynamic_cast<DOMElement *>(levelList->item(i));
            if (XMLString::equals(xmlId, 
                    level->getAttribute(Utf8ToXML("id").x_str()))) {
                levelFound = true;
            }
        }
        XMLString::release(&xmlId);
        return levelFound ? level : NULL;
    }
} // namespace enigma
