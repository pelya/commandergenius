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
 
#ifndef PREFMGR_HH_INCLUDED
#define PREFMGR_HH_INCLUDED

#include "PropertyManager.hh"

namespace enigma
{
    /**
     * A singelton manager for preferences stored in XML format. 
     * <p>The singleton can be accessed via standard static instance() method or
     * the application public ivar <code>app.prefs</code>.</p>
     * <p> During initialization old LUA stored options are convereted. New 
     * preferences with given default values are introduced to the user via the 
     * template in the systen data directory (data/enigmarc.xml).</p>
     * <p>The storage location of the users preferences is determined by the
     * Application object.</p>
     * <p>The set and get methods for preferences take utf8 encoded preference
     * names and operate with utf8 encoded string values. Set methods create new
     * preference property elements if necessary. Get methods do not modify
     * reference values and return C++ default values if the named preference
     * does not exist.</p>
     */
    class PreferenceManager : public PropertyManager {
    public:
        static PreferenceManager *instance();
        ~PreferenceManager();
        virtual bool save();
        void shutdown();
        
    protected:
        PreferenceManager();
    private:
        static PreferenceManager *theSingleton;
    };
} // namespace enigma
#endif

