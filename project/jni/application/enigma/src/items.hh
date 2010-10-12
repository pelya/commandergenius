/*
 * Copyright (C) 2002,2003,2004 Daniel Heck
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
#ifndef ITEMS_HH_INCLUDED
#define ITEMS_HH_INCLUDED

#include "objects_decl.hh"

namespace world
{
    enum ItemID {
        it_INVALID = -1,
        it_FIRST = 0,
        it_none = 0,
        it_1pkillstone,
        it_2pkillstone,
        it_abyss,
        it_bag,
        it_banana,
        it_blackbomb,
        it_blackbomb_burning,
        it_blocker,
        it_booze,
        it_brake,
        it_bridge_oxyd,
        it_bridge_oxyd_active,
        it_booze_broken,
        it_brush,
        it_burnable,
        it_burnable_fireproof,
        it_burnable_ignited,
        it_burnable_burning,
        it_burnable_ash,
        it_burnable_oil,
        it_changefloor,
        it_cherry,
        it_cherry_crushed,
        it_coffee,
        it_coin1,
        it_coin2, 
        it_coin4,
        it_crack0,
        it_crack1,
        it_crack2,
        it_crack3,
        it_cross,
        it_death,
        it_debris,
        it_document,
        it_drop,
        it_dynamite,
        it_dummy,
        it_easykeepstone,
        it_easykillstone,
        it_explosion1,
        it_explosion2,
        it_explosion3,
        it_extinguisher,
        it_extinguisher_medium,
        it_extinguisher_empty,
        it_extralife,
        it_flagblack,
        it_flagwhite,
        it_floppy,
        it_glasses,
        it_glasses_broken,
        it_hammer,
        it_hill,
        it_hollow,
        it_hstrip,
        it_inversesensor,
        it_key_a,
        it_key_b,
        it_key_c,
        it_landmine,
        it_laserbeam,
        it_magicwand,
        it_magnet_off,
        it_magnet_on,
        it_odometer,
        it_oxyd5f,
        it_pencil,
        it_pin,
        it_pipe_e, it_pipe_w, it_pipe_s, it_pipe_n,
        it_pipe_es, it_pipe_ne, it_pipe_sw, it_pipe_wn,
        it_pipe_h, it_pipe_v,
        it_puller_n,
        it_puller_e,
        it_puller_s,
        it_puller_w,
        it_ring,
        it_rubberband,
        it_seed,
        it_seed_nowood,
        it_seed_volcano,
        it_sensor,
        it_shogun_s,
        it_shogun_m,
        it_shogun_l,
        it_signalfilter0,
        it_signalfilter1,
        it_spade,
        it_spoon,
        it_spring1,
        it_spring2,
        it_springboard,
        it_squashed,
        it_surprise,
        it_sword,
        it_tinyhill,
        it_tinyhollow,
        it_trigger,
        it_umbrella,
        it_vortex_open,
        it_vortex_closed,
        it_vstrip,
        it_weight,
        it_whitebomb,
        it_wormhole_off,
        it_wormhole_on,
        it_wrench,
        it_yinyang,
        it_LAST,
        it_COUNT
    };

    /*! What may happen to an item _after_ it was activated? */
    enum ItemAction {
        ITEM_DROP,              //!< Drop it to the floor
        ITEM_KILL,              //!< Remove it from the inventory and dispose it
        ITEM_KEEP,              //!< Keep it in the inventory; do nothing further
    };

    enum ItemFlags {
        itf_none   = 0,
        itf_static = 1,         //!< Cannot be picked up
        itf_indestructible = 2, //!< Cannot be destroyed by explosions etc.
        itf_animation = 4,      //!< Use set_anim() instead of set_model()
        itf_invisible = 8,      //!< Item has no visible model
        itf_inflammable = 16,   //!< Burns when hit by laser beam
        itf_norespawn = 32,     //!< Don't respawn balls on top of this item
        itf_fireproof = 64,     //!< This item can't burn by fire
    };

    struct ItemTraits {
        const char *name;       //!< Name of the item, e.g., "it-hammer"
        ItemID      id;
        int         flags;      //!< Combination of ItemFlags
        float       radius;     //!< Radius, 0.0 = default
    };

    class Item : public GridObject {
    public:
        Item();

        /* ---------- Public methods ---------- */
        void kill();
        void replace (ItemID id);

        /* ---------- Virtual functions ---------- */
        const char *get_kind() const;
        void init_model();
        void on_laserhit(Direction);

        /* ---------- Item interface ---------- */

        virtual Item *clone() = 0;

        virtual const ItemTraits &get_traits() const = 0;

        /*! Return true if item completely covers the floor. In this
          case the Floor::actor_contact() will not be called
          automatically; this must be done from `Item::actor_hit' (if
          at all). */
        virtual bool covers_floor(ecl::V2 pos) const { return false; }

        /*! Return the force an item exerts on actor `a'.  This is
          used by sloped and hills for force fields that are local to
          the current field.  For global force fields you have to
          register a ForceField in the world. */
        virtual void add_force(Actor *a, ecl::V2 &f);

        virtual bool can_drop_at (GridPos p);

        virtual void drop (Actor *a, GridPos p);

        /*! Called when item is dropped by actor `a' */
        virtual void on_drop(Actor *a);

        /*! Called when item is picked up by actor `a' */
        virtual void on_pickup(Actor *a);

        /*! Called when stone above item changes. */
        virtual void stone_change(Stone *st);

        /*! Called when item is ``hit'' by a moving stone. */
        virtual void on_stonehit(Stone *st);

        /*! Called when item is ``hit'' by an actor.  Return true if
          the item should be picked up. */
        virtual bool actor_hit(Actor *a);

        /*! The model used for displaying this item in an
          inventory. */
        virtual string get_inventory_model();

        /* Called when item is activated by the owner of `a'. */
        virtual ItemAction activate(Actor* a, GridPos p);
    protected:
        // GridObject interface
        virtual void set_model (const std::string &mname) {
            display::SetModel(GridLoc(GRID_ITEMS, get_pos()), mname);
        }

        virtual display::Model *get_model () {
            return display::GetModel(GridLoc(GRID_ITEMS, get_pos()));
        }

        virtual void kill_model (GridPos p) {
            display::KillModel (GridLoc (GRID_ITEMS, p));
        }
        // replace template method hook
        virtual void setup_successor(Item *newitem) {}
    };

/* -------------------- Inline functions -------------------- */

    /*! Return unique item type identifier. */
    inline ItemID get_id (Item *it) { 
        if (it)
            return it->get_traits().id;
        return it_none;
    }

    inline bool has_flags (Item *it, ItemFlags flags) {
        return (it->get_traits().flags & flags) == flags;
    }

/* -------------------- Functions -------------------- */

    void InitItems();
}

#endif
