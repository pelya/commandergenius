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
#ifndef STONES_HH_INCLUDED
#define STONES_HH_INCLUDED

#include "objects_decl.hh"

namespace world
{
    using namespace enigma;

    enum StoneID {
        st_INVALID = -1,
        st_FIRST = 0,
        st_none = 0,

        st_borderstone,
	st_black1,
        st_black2,
        st_black3,
        st_black4,
        st_chameleon,
        st_death,
        st_death_invisible,
        st_easymode,
        st_fart,
        st_knight,
        st_magic,
        st_rubberband,
        st_scissors,
        st_thief,
        st_white1,
        st_white2,
        st_white3,
        st_white4,
        st_window,
        st_lightpassenger,
        st_camouflage,
        st_polarswitch,

        st_LAST,
        st_COUNT = st_LAST
    };

    enum StoneFlags {
        stf_none = 0x0,
        stf_transparent = 0x1,  //< Laser beams can pass
    };

    enum Material {
        material_FIRST = 0,
        material_stone = 0,
        material_glass,
        material_metal,
        material_wood,
        material_cloth,
        material_LAST
    };

    struct StoneTraits {
        const char *name;
        StoneID     id;
        int         flags;
        Material    material;
        double      restitution;
        // Note that many properties of stones are implemented as functions.
    };

    /*! Things that may happen when an actor hits a stone. */
    enum StoneResponse {
        STONE_PASS,             // Actor may pass stone
        STONE_REBOUND           // Actor bounces off the stone
    };

    class Stone : public GridObject {
    public:
        Stone();
        Stone(const char *kind);
        ~Stone();

        /* ---------- Virtual functions ---------- */
        virtual Stone *clone() = 0;
        const char *get_kind() const;

        /* ---------- Stone interface (properties) ---------- */

        virtual const StoneTraits &get_traits() const;

        virtual const char *collision_sound();

        virtual StoneResponse collision_response(const StoneContact &sc);

        /*! Is this stone movable? Affects impulse-stones, fire, ordinary pushes... */
        virtual bool   is_movable() const { return false;}

        /*! Can a swap-stone or pull-stone swap this stone? */
        virtual bool   is_removable() const { return true; }

        /*! Is this stone floating above the floor (e.g. actors may pass)? */
        virtual bool   is_floating() const { return false; }
        
        /*! Can laser beams pass through stone? Return is_floating() by default. */
        virtual bool   is_transparent (Direction) const { 
            return is_floating();
        }
        
        /*! Do actors get stuck in this stone? */
        virtual bool is_sticky(const Actor *) const { 
            return !is_floating(); 
        }

        /* ---------- Stone interface (events) ---------- */

        virtual void   actor_hit (const StoneContact &sc);
        virtual void   actor_touch (const StoneContact &sc);
        virtual void   actor_inside (Actor * /*a*/) {}
        virtual void   actor_contact (Actor * /*a*/) {}

        virtual void   on_move();
        virtual void   on_floor_change() {}
        virtual void   on_impulse(const Impulse& impulse);

    protected:
        bool move_stone(GridPos newPos, const char *soundevent);
        bool move_stone(Direction dir);
        ecl::V2 distortedVelocity (ecl::V2 vel, double defaultfactor);

    protected:
        // GridObject interface
        virtual void set_model (const std::string &mname) {
            display::SetModel(GridLoc(GRID_STONES, get_pos()), mname);
        }

        virtual display::Model *get_model () {
            return display::GetModel(GridLoc(GRID_STONES, get_pos()));
        }

        virtual void kill_model (GridPos p) {
            display::KillModel (GridLoc (GRID_STONES, p));
        }
    };

    inline StoneID get_id(Stone *st) {
        return st->get_traits().id;
    }
}

namespace stones
{
    void Init();
}

#endif
