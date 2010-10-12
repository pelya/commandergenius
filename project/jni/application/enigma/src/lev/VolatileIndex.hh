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
 
#ifndef LEV_VOLATILEINDEX_HH_INCLUDED
#define LEV_VOLATILEINDEX_HH_INCLUDED

#include "lev/Index.hh"

#include <string>
#include <vector>

namespace enigma { namespace lev {    
    /**
     * 
     */
    class VolatileIndex : public Index  {
    public:
        /**
         * Convention: method names *Level() can take int pos or Proxy as arg.
         */
        VolatileIndex(std::string anIndexName, std::string aGroupName,
                const std::vector<string> levelpaths, 
                double defaultLocation = INDEX_DEFAULT_PACK_LOCATION);
        ~VolatileIndex();
        virtual void clear();
    private:
        static int levelCount;  // used for volatile level ids - necessary for lua levels
    };

}} // namespace enigma::lev
#endif
