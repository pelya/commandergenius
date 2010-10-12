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

#include "PreferenceManager.hh"
#include "main.hh"
#include "DOMErrorReporter.hh"
#include "DOMSchemaResolver.hh"
#include "LocalToXML.hh"
#include "nls.hh"
#include "Utf8ToXML.hh"
#include "utilXML.hh"
#include "options.hh"
#include "XMLtoLocal.hh"
#include "XMLtoUtf8.hh"
#include "ecl_system.hh"
#include "gui/ErrorMenu.hh"

#include <iostream>
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
    PreferenceManager *PreferenceManager::theSingleton = 0;
    
    PreferenceManager* PreferenceManager::instance() {
        if (theSingleton == 0) {
            theSingleton = new PreferenceManager();
        }
        return theSingleton;
    }
    
    PreferenceManager::PreferenceManager() {
        std::string prefTemplatePath;
        bool haveXMLProperties = (ecl::FileExists(app.prefPath)) ? true : false;
        
        if (!app.systemFS->findFile( std::string("schemas/") + PREFFILENAME , prefTemplatePath)) {
            cerr << "Preferences: no template found\n";
            exit(-1);
        }

        try {
            app.domParserErrorHandler->resetErrors();
            app.domParserErrorHandler->reportToErr();
            app.domParserSchemaResolver->resetResolver();
            app.domParserSchemaResolver->addSchemaId("preferences.xsd","preferences.xsd");

            if (haveXMLProperties) {
                // update existing XML prefs from possibly newer template:
                // use user prefs and copy new properties from template
                doc = app.domParser->parseURI(app.prefPath.c_str());
                propertiesElem = dynamic_cast<DOMElement *>(doc->getElementsByTagName(
                        Utf8ToXML("properties").x_str())->item(0));
                // The following algorithm is not optimized - O(n^2)!
                DOMDocument * prefTemplate = app.domParser->parseURI(prefTemplatePath.c_str());
                DOMNodeList * tmplPropList = prefTemplate->getElementsByTagName(
                        Utf8ToXML("property").x_str());
                for (int i = 0, l = tmplPropList-> getLength(); i < l; i++) {
                    DOMElement *tmplProperty = dynamic_cast<DOMElement *>(tmplPropList->item(i));
                    const XMLCh * key = tmplProperty->getAttribute(Utf8ToXML("key").x_str());
                    DOMElement * lastUserProperty;
                    if ((key[0] != chUnderscore) && !hasProperty(key, &lastUserProperty)) {
                        // a new property in the template
                        Log << "Preferences: add new Property \"" << XMLtoLocal(key) << "\"\n";
                        // make a copy
                        DOMNode * newProperty =  doc->importNode(tmplProperty, false);
                        if (newProperty == NULL) {
                            Log << "Preferences: copy new Property failed!\n";
                        } else {
                            // insert it at the end of the existing user properties
                            propertiesElem->appendChild(dynamic_cast<DOMElement *>(newProperty));
                        }
                    }
                }
                prefTemplate->release();
            } else {
                // update from LUA options to XML preferences:
                // use the template, copy LUA option values and save it later as prefs
                doc = app.domParser->parseURI(prefTemplatePath.c_str());
                propertiesElem = dynamic_cast<DOMElement *>(doc->getElementsByTagName(
                        Utf8ToXML("properties").x_str())->item(0));
                DOMNodeList * propList = propertiesElem->getElementsByTagName(Utf8ToXML("property").x_str());
                for (int i = 0, l = propList-> getLength(); i < l; i++) {
                    DOMElement * property  = dynamic_cast<DOMElement *>(propList->item(i));
                    const XMLCh * key = property->getAttribute(Utf8ToXML("key").x_str());
                    std::string optionValue;
                    if (options::HasOption(XMLtoLocal(key).c_str(), optionValue)) {
                        Log << "Preferences: copy LUA option \"" << XMLtoLocal(key) << "\"\n";
                        property->setAttribute(Utf8ToXML("value").x_str(), 
                            LocalToXML(&optionValue).x_str());
                    } else {
                        Log << "Preferences: no LUA option \"" << XMLtoLocal(key) << "\"\n";
                    }
                }
            }
        }
        catch (const XMLException& toCatch) {
            char* message = XMLString::transcode(toCatch.getMessage());
            cerr << "Exception on load of preferences: "
                 << message << "\n";
            XMLString::release(&message);
            exit (-1);
        }
        catch (const DOMException& toCatch) {
            char* message = XMLString::transcode(toCatch.msg);
            cerr << "Exception on load of preferences: "
                 << message << "\n";
            XMLString::release(&message);
            exit (-1);
        }
        catch (...) {
            cerr << "Unexpected Exception on load of preferences\n" ;
            exit (-1);
        }        
    }
     
    PreferenceManager::~PreferenceManager() {
        if (doc != NULL)
            shutdown();
    }
    
    bool PreferenceManager::save() {
        bool result = true;
        std::string errMessage;
        
        if (doc == NULL)
            return true;

        stripIgnorableWhitespace(doc->getDocumentElement());
        
        try {
#if _XERCES_VERSION >= 30000
            result = app.domSer->writeToURI(doc, LocalToXML(& app.prefPath).x_str());
#else
            XMLFormatTarget *myFormTarget = new LocalFileFormatTarget(app.prefPath.c_str());
            result = app.domSer->writeNode(myFormTarget, *doc);            
            delete myFormTarget;   // flush
#endif
        } catch (const XMLException& toCatch) {
            errMessage = std::string("Exception on save of preferences: \n") + 
                    XMLtoUtf8(toCatch.getMessage()).c_str() + "\n";
            result = false;
        } catch (const DOMException& toCatch) {
            errMessage = std::string("Exception on save of preferences: \n") + 
                    XMLtoUtf8(toCatch.getMessage()).c_str() + "\n";
            result = false;
        } catch (...) {
            errMessage = "Unexpected exception on save of preferences\n" ;
            result = false;
        }
        
        if (!result) {
            cerr << errMessage;
            gui::ErrorMenu m(errMessage, N_("Continue"));
            m.manage();          
        } else
            Log << "Preferences save o.k.\n";
        
        return result;
    }

    void PreferenceManager::shutdown() {
        save();
        if (doc != NULL)
            doc->release();
        doc = NULL;
    }
    
} // namespace enigma

