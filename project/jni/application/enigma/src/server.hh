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

/*! @file
  The client->server interface 
*/
#ifndef SERVER_HH_INCLUDED
#define SERVER_HH_INCLUDED

//#include "levels.hh"
#include "lev/Proxy.hh"

namespace enigma_server 
{
    using namespace enigma;
//    using levels::LevelPack;

/* -------------------- Global variables -------------------- */

    /*! Pointer to the current level pack. */
//    extern LevelPack *CurrentLevelPack;

    /*! Index of the current level. */
//    extern unsigned CurrentLevel;

    /*! True if the level is only being loaded for the purpose of
      making a preview image. */
    extern bool CreatingPreview;

/* -------------------- Cheats -------------------- */

    extern bool NoCollisions;

/* -------------------- Per-level settings -------------------- */

    // The number of seconds passed since the level game started.
    extern double LevelTime;

    // True: do not reset level when player dies
    extern bool     ConserveLevel;

    extern bool     TwoPlayerGame;

    extern bool     SingleComputerGame;

    // True: allow to control 2nd player (e.g. after first died)
    extern bool     AllowTogglePlayer;

    // True -> show move counter (Sokoban style)
    extern bool     ShowMoves;

    // Behave like Oxyd/Enigma version
    extern GameType GameCompatibility;

    // Default brittleness of the floor: 0 = stable..1=unstable.
    // Really: probability that a floor tile will crack when an actor
    // enters or leaves it.
    extern double   Brittleness;

    extern double   WaterSinkSpeed;

    extern double   SwampSinkSpeed;

/* --------------------  Force multipliers... -------------------- */

    // ...for floor tiles that look sloped
    extern double   SlopeForce;

    // ...for floor tiles that DON'T look sloped
    extern double   FlatForce;

    // ...for friction on certain floor types
    extern double   FrictionFactor;

    // ...for friction on ice
    extern double   IceFriction;

    // ...for electrostatic forces between actors (default: 15)
    extern double   ElectricForce;

    // ...for the bumber stones (st-actorimpulse*)
    extern double   BumperForce;

    // ...for magnets
    extern double   MagnetForce;
    extern double   MagnetRange;

    // ...for wormholes
    extern double   WormholeForce;
    extern double   WormholeRange;

    // ...for holes
    extern double   HoleForce;

/* -------------------- Functions -------------------- */

    /** Initialize the server at program start. */
    void Init();

    /** Shutdown the server at program end. */
    void Shutdown();

// TEST
    bool NetworkStart();

    /** Prepare the server for a new game. */
    void InitNewGame();

    void Tick (double dtime);
    void RestartLevel();
    bool IsRestartingLevel();
    void FinishLevel();

    void SetCompatibility(const char *version); // set compatibility (from lua)
    void SetCompatibility(lev::Proxy *levelProxy); // set compatibility from xml
    
    enigma::Difficulty GetDifficulty();

    // move counter
    void InitMoveCounter();
    int  IncMoveCounter(int increment = 1);
    int  GetMoveCounter();

    void RaiseError (const std::string &msg);

/* -------------------- Client -> Server messages -------------------- */

    void Msg_SetLevelPack (const std::string &name);

    void Msg_LoadLevel (lev::Proxy *levelProxy, bool isPreview);
     
    void Msg_JumpBack();

    /*! After loading the level, the server sends a "LevelLoaded"
      message to all clients.  The game only starts after they have
      answered with a "StartGame" message. */
    void Msg_StartGame();

    void Msg_RestartGame();

    void Msg_Command (const std::string &command);

    void Msg_Pause (bool onoff);
    
    void Msg_Panic (bool onoff);

    void Msg_MouseForce (const ecl::V2 &f);


    void Msg_ActivateItem();
}

namespace enigma
{
    namespace server = enigma_server;
}

#endif
