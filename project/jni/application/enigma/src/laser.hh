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
#ifndef LASER_HH
#define LASER_HH

/* This file contains the declarations for (almost) everything that
   has to do with lasers. */

#include "objects.hh"

namespace world
{
    /**
     * This interface must be implemented by all items and stones that
     * are capable of emitting light.
     */
    class LaserEmitter {
    public:
	virtual ~LaserEmitter() {}
	virtual DirectionBits emission_directions() const = 0;
    };

/* -------------------- PhotoCell -------------------- */

    /**
     * PhotoCells are objects (not necessarily stones) that are
     * sensitive to laser light.  Whenever the game engine
     * recalculates the laser beams, instances of this class are
     * notified about the beginning and the end of a recalculation.
     */
    class PhotoCell {
    public:
        virtual ~PhotoCell();

        // ---------- Static functions ----------
        static void notify_start();
        static void notify_finish();

        // ---------- PhotoCell interface ----------
        virtual void on_recalc_start() = 0;
        virtual void on_recalc_finish() = 0;
    protected:

        /*! Derived classes must call this method to register
          themselves for the on_recalc_start() and on_recalc_finish()
          events. */
        void photo_activate();

        /*! Derived classes must call this method to unregister
          themselves.  It is automatically called by ~PhotoCell(), but
          objects may have to call it explicitly if they are not
          interested in PhotoCell events. */
        void photo_deactivate();
    private:
        static std::vector<void*> instances;
    };

/* -------------------- PhotoStone -------------------- */

    /*! Most stones are indifferent to laser beams: They either block
      the light completely or they let it pass, but they do not change
      their internal state when they are hit by light.  Certain kinds
      of stones need to be notified whenever the `light' goes on or off
      -- these can be derived from this class.
    
      The most prominent example are Oxyd stones -- they open when
      they are hit by a laser beam.  See the remarks at the beginning
      of this file to understand why overriding `on_laserhit' is not
      sufficient for a proper implementation of Oxyd stones.
    */

    class PhotoStone : public Stone, public PhotoCell {
    protected:
        PhotoStone (const char *kind);

    private:
        bool illuminated;

        // PhotoCell interface
        void on_recalc_start();
        void on_recalc_finish();

        // PhotoStone interface
        virtual void notify_laseron() = 0;
        virtual void notify_laseroff() = 0;
    };
}

/* -------------------- Functions -------------------- */
namespace lasers
{
    void Init();

    /*! This function must be called at the end of each tick; it
      recalculates the laser beams if necessary. */
    void RecalcLightNow();

    /*! Force all light beams to be recalculated at the end of the
      current tick.  So far, this is only used by laser stones and in
      world::InitWorld().  */
    void RecalcLight();

    /*! If position `p' is inside a laser beam, force all laser beams
      to be recalculated.  This is mainly used when items and stones
      are created or removed, but it can be also used for objects
      (like doors) that sometimes shut off a light beam (when the door
      is closed) and sometimes don't (when the door is open). */
    void MaybeRecalcLight (enigma::GridPos p);

    /*! Return true iff a stone or an item at position `p' it hit by
      light coming from direction `dir'. */
    bool LightFrom (enigma::GridPos p, enigma::Direction dir);
}
#endif
