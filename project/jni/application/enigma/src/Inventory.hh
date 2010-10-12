/*
 * Copyright (C) 2006 Daniel Heck
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

#ifndef INVENTORY_HH_INCLUDED
#define INVENTORY_HH_INCLUDED

#include "ItemHolder.hh"
#include <string>
#include <vector>
#include <stdlib.h>

namespace enigma
{
    using world::Item;          // TODO: remove this after moving Item into enigma namespace

    class Inventory : public ItemHolder {
    public:
        Inventory();
        ~Inventory();

        // ---------- ItemHolder interface ----------
        bool is_full() const;
        virtual bool is_empty() const;
        void add_item (Item *i);
        virtual void takeItemsFrom(ItemHolder *ih);
        virtual Item *yield_first();

        // ---------- Methods ----------

        //! The number of items currently in the inventory
        size_t size() const;
        void clear();

        void rotate_left();
        void rotate_right();
        Item *get_item (size_t idx) const;
        Item *yield_item (size_t idx);
	bool willAddItem(Item *it);

        int find(const std::string& kind, size_t start_idx = 0) const;

    private:
        // Private methods.
        Item *remove_item(Item *it);

        // Private variables.
        static const unsigned max_items;
        std::vector<Item*> m_items;
    };
}

#endif
