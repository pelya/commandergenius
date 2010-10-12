/*
 * Copyright (C) 2003,2004,2005 Daniel Heck
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
#ifndef CLIENT_HH_INCLUDED
#define CLIENT_HH_INCLUDED

#include "lev/Index.hh"

namespace enigma_client 
{
    using namespace enigma;

/* -------------------- Functions -------------------- */

    bool NetworkStart();


    void Tick (double dtime);

    bool AbortGameP();

    void Stop();

/* -------------------- Server->Client messages -------------------- */

    void Msg_Command(const std::string& cmd);
    
    void Msg_AdvanceLevel (lev::LevelAdvanceMode mode);
    void Msg_JumpBack();

    void Msg_LevelLoaded(bool isRestart);

    void Msg_PlayerPosition (unsigned iplayer, const ecl::V2 &pos);

    void Msg_Sparkle (const ecl::V2 &pos);

    void Msg_ShowText (const std::string &text, bool scrolling, double duration=-1);

    //! Play sound emanating from an absolute position in the world
    void Msg_PlaySound (const std::string &soundname, 
                        const ecl::V2 &pos,
                        double relative_volume);

    //! Play a  sound emanating from the player's current position
    void Msg_PlaySound (const std::string &soundname, double relative_volume);

    //! Stop the game and display an error message instead
    void Msg_Error (const std::string &text);
}

namespace enigma
{
    namespace client = enigma_client;
}

#endif
