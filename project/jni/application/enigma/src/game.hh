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
#ifndef ENIGMA_GAME_HH
#define ENIGMA_GAME_HH

#include "fwd.hh"
#include "ecl_fwd.hh"
#include "lev/Proxy.hh"

namespace enigma_game
{
    using enigma_levels::LevelPack;
    using enigma_levels::Level;

    void ResetGameTimer();

    void StartGame();

    /*! Draw a level preview into video::BackBuffer().  Return true if
      successful. */
    bool DrawLevelPreview (ecl::GC &, enigma::lev::Proxy *levelProxy);

}

namespace enigma
{
    namespace game = enigma_game;
}
#endif
