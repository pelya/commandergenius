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

#include "DOMSchemaResolver.hh"
#include "main.hh"
#include "file.hh"
#include "XMLtoLocal.hh"
#include "LocalToXML.hh"
#include <xercesc/dom/DOM.hpp>
#if _XERCES_VERSION >= 30000
#include <xercesc/framework/Wrapper4InputSource.hpp>
#include <xercesc/framework/LocalFileInputSource.hpp>

#else
#include <xercesc/framework/Wrapper4InputSource.hpp>
#include <xercesc/framework/LocalFileInputSource.hpp>
#endif

#include <iostream>

XERCES_CPP_NAMESPACE_USE

namespace enigma
{
    DOMSchemaResolver::DOMSchemaResolver():resolveStatus(false) {
    }
    
    DOMSchemaResolver::~DOMSchemaResolver() {
    }

    void DOMSchemaResolver::resetResolver() {
        substitutions.clear();
        resolveStatus = false;
    }
    
    bool DOMSchemaResolver::didResolveSchema() {
        return resolveStatus;
    }
    
    void DOMSchemaResolver::resetResolveStatus() {
        resolveStatus = false;
    }
    
    void DOMSchemaResolver::addSchemaId(const std::string &schemaSystemId, 
            const std::string &schemaFilename) {
        substitutions.insert(std::make_pair(schemaSystemId, schemaFilename));
    }
    
#if _XERCES_VERSION >= 30000
    DOMLSInput * DOMSchemaResolver::resolveResource (
            const XMLCh* const resourceType, const XMLCh* const namespaceUri,
            const XMLCh *const publicId, const XMLCh *const systemId,
            const XMLCh *const baseURI) {

        std::string schemaName = XMLtoLocal(systemId).c_str();
        std::map<std::string, std::string>::iterator i = substitutions.find(schemaName);
        if (i == substitutions.end()) {
            Log << "DOMSchemaResolver: no schema substitution found for '" 
                    << schemaName << "'\n";
            // let the parser try to resolve potential external entities
            return NULL;
        } else {
            std::string schemaPath;
            bool result = app.systemFS->findFile( std::string("schemas/") + 
                    i->second , schemaPath);
            if (result) {
                DOMLSInput * inSrc = new Wrapper4InputSource(new LocalFileInputSource(
                        LocalToXML(&schemaPath).x_str()));
// W3C implementation:
//                 DOMLSInput * inSrc = app.domImplementationLS->createLSInput();
//                 inSrc->setSystemId(LocalToXML(&schemaPath).x_str()));
                return inSrc;
            } else {
                Log << "DOMSchemaResolver: schema file '" 
                    << i->second << "' not found\n";
                // let the parser try to resolve the schema
                return NULL;
            }
        }
    }
#else
    DOMInputSource * DOMSchemaResolver::resolveEntity (
            const XMLCh *const publicId, const XMLCh *const systemId,
            const XMLCh *const baseURI) {
                
        std::string schemaName = XMLtoLocal(systemId).c_str();
        std::map<std::string, std::string>::iterator i = substitutions.find(schemaName);
        if (i == substitutions.end()) {
            Log << "DOMSchemaResolver: no schema substitution found for '" 
                    << schemaName << "'\n";
            // let the parser try to resolve potential external entities
            return NULL;
        } else {
            std::string schemaPath;
            bool result = app.systemFS->findFile( std::string("schemas/") + 
                    i->second , schemaPath);
            if (result) {
                DOMInputSource * inSrc = new Wrapper4InputSource(new LocalFileInputSource(
                        LocalToXML(&schemaPath).x_str()));
                return inSrc;
            } else {
                Log << "DOMSchemaResolver: schema file '" 
                    << i->second << "' not found\n";
                // let the parser try to resolve the schema
                return NULL;
            }
        }
    }
#endif
} // namespace enigma

