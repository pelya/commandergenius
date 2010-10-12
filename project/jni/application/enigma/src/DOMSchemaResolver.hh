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

#ifndef ENIGMA_DOMSCHEMARESOLVER_HH
#define ENIGMA_DOMSCHEMARESOLVER_HH

#include <map>
#include <string>
#include <xercesc/util/XercesVersion.hpp>
#if _XERCES_VERSION >= 30000
#include <xercesc/dom/DOMLSParser.hpp>
#include <xercesc/dom/DOMLSResourceResolver.hpp>
#include <xercesc/dom/DOMLSInput.hpp>
#else    
#include <xercesc/dom/DOMBuilder.hpp>
#include <xercesc/dom/DOMEntityResolver.hpp>
#include <xercesc/dom/DOMInputSource.hpp>
#endif

namespace enigma
{
    /**
     * An XML schema system id to local schema storage path resolver. 
     * The resolver can be configured with several pairs of system id and
     * paths. This should allow usage of versioned schema system ids like
     * http://enigma/1.0/levels.xsd . All entities without local substitution
     * are resolved by the parser using the given system id. A flag allows to
     * check if the XML file did adhere to a given schema.
     */
#if _XERCES_VERSION >= 30000
    class DOMSchemaResolver : public XERCES_CPP_NAMESPACE_QUALIFIER DOMLSResourceResolver {
    public:
        XERCES_CPP_NAMESPACE_QUALIFIER DOMLSInput * resolveResource (
            const XMLCh* const resourceType, const XMLCh* const namespaceUri,
            const XMLCh *const publicId, const XMLCh *const systemId,
            const XMLCh *const baseURI);
#else    
    class DOMSchemaResolver : public XERCES_CPP_NAMESPACE_QUALIFIER DOMEntityResolver {
    public:
        XERCES_CPP_NAMESPACE_QUALIFIER DOMInputSource * resolveEntity (
            const XMLCh *const publicId, const XMLCh *const systemId,
            const XMLCh *const baseURI);
#endif
        DOMSchemaResolver();
        ~DOMSchemaResolver();
        
        /**
         * clear system id substitution paths and reset result flag.
         */
        void resetResolver();
        
        /**
         * add a system id substituion path.
         * @param schemaSystemId  as used in XML files
         * @param schemaFilename  the local substitution filename of the schema.
         *                        Just the filename without any path components.
         */
        void addSchemaId(const std::string &schemaSystemId, const std::string &schemaFilename);
        
        /**
         * true if at least one local substitution occured since last reset.
         */
        bool didResolveSchema();
        
        /**
         * just reset status without deletion of substitution info.
         */
        void resetResolveStatus();
    private:
        std::map<std::string, std::string> substitutions;
        bool resolveStatus;
    };
} // namespace enigma
#endif

