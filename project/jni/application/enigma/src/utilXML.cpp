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

#include "utilXML.hh"
#include "XMLtoUtf8.hh"

#include <xercesc/dom/DOM.hpp>

XERCES_CPP_NAMESPACE_USE

namespace enigma
{
    bool boolValue(const XMLCh * const string) {
        std::string boolString = XMLtoUtf8(string).c_str();
        if (boolString == "true" ||  boolString == "1")
            return true;
        else
            // we need no further investigation due to XML validation
            return false;
    }
    
    void stripIgnorableWhitespace(DOMElement * elem) {
        DOMNode * child = elem->getFirstChild();
        DOMNode * next;

        for (child = elem->getFirstChild(); child != NULL; child = next) {
            next = child->getNextSibling();
            if (child->getNodeType() == DOMNode::TEXT_NODE) {
                if (dynamic_cast<DOMText *>(child)->isIgnorableWhitespace()) {
                    elem->removeChild(child);
                }
            } else if (child->getNodeType() == DOMNode::ELEMENT_NODE) {
                DOMElement * childElem = dynamic_cast<DOMElement *>(child);
                if (childElem != NULL)
                    stripIgnorableWhitespace(childElem);
            }
        }
    }
    
} //namespace enigma
