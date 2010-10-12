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
 
#ifndef PROPERTYMGR_HH_INCLUDED
#define PROPERTYMGR_HH_INCLUDED

#include <string>
#include <xercesc/dom/DOMDocument.hpp>
#include <xercesc/dom/DOMElement.hpp>

namespace enigma
{
    /**
     * An abstact Superclass for key/value property access.
     */
    class PropertyManager {
    public:
        ~PropertyManager();
        virtual bool save() = 0;    // mark the class as abstract
        
        void setProperty(const char *prefName, const std::string &value);
        void setProperty(const char *prefName, const char *value);
        void getProperty(const char *prefName, std::string &value);
        std::string getString(const char *prefName);
        void setProperty(const char *prefName, const double &value);
        void getProperty(const char *prefName, double &value);
        double getDouble(const char *prefName);
        void setProperty(const char *prefName, const int &value);
        void getProperty(const char *prefName, int &value);
        int  getInt(const char *prefName);
        void setProperty(const char *prefName, const bool &value);
        void getProperty(const char *prefName, bool &value);
        bool getBool(const char *prefName);
        
    protected:
        PropertyManager();
        XERCES_CPP_NAMESPACE_QUALIFIER DOMDocument *doc;
        XERCES_CPP_NAMESPACE_QUALIFIER DOMElement *propertiesElem;
        
        /**
         * force a return of a property element with the given name as key.
         * @param prefName  the name of the searched or new property
         * @return  the property element with the given key name.
         */
        XERCES_CPP_NAMESPACE_QUALIFIER DOMElement * getPropertyElement(const char *prefName);

        /**
         * checks if a property exists.
         * @param prefName the preference name.
         * @param element  the searched property element or the last property 
         *                 element found in the preference list (for append usage).
         * @return validity of element.
         */
         bool hasProperty(const char *prefName, XERCES_CPP_NAMESPACE_QUALIFIER DOMElement ** element);
        /**
         * checks if a property exists.
         * @param key     the preference name.
         * @param element the searched property element or the last property 
         *                element found in the preference list (for append usage).
         * @return validity of element.
         */
        bool hasProperty(const XMLCh * key, XERCES_CPP_NAMESPACE_QUALIFIER DOMElement ** element);
    };
} // namespace enigma
#endif

