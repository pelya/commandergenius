#ifndef ITEMHOLDER_HH_INCLUDED
#define ITEMHOLDER_HH_INCLUDED

#include "items.hh"

namespace enigma
{
    /**
     * A base class for all entities that can hold multiple items
     * during the game.  Currently this is only the inventory that is
     * displayed at the bottom of the screen (there is one inventory
     * for every player) and the it-bag item.
     */
    class ItemHolder {
    public:
        virtual ~ItemHolder() {}

        //! Return true if not further object can be picked up
        virtual bool is_full() const = 0;
        virtual bool is_empty() const = 0;

        //! Add another item
        virtual void add_item (world::Item *it) = 0;
        virtual world::Item *yield_first() = 0;
        virtual void takeItemsFrom(ItemHolder *ih) {return;}
    };
}

#endif
