/*
 * Copyright (C) 2002,2003,2004,2005 Daniel Heck
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
#ifndef PLAYER_HH_INCLUDED
#define PLAYER_HH_INCLUDED

/**
 * @file
 *
 * Player management.  Deals mostly with inventory management,
 * switching between players, respawning, etc.  Players are numbered
 * from 0..N-1, where N is the number of players in the current game
 * (currently always 1 or 2).
 */

#include "objects.hh"

namespace enigma
{
    class Inventory;

    namespace player {

        /*! Start a new game for two virtual players. */
        void NewGame(bool isRestart);

        /*! This is called whenever a new level is reached in a running
          game.  The inventories of all players are cleaned up, i.e., all
          items except for extra lifes are removed. */
        void PrepareLevel();

        /*! Add a yinyang item to all players' inventories. */
        void AddYinYang ();

        /*! Called after the loading a level but before starting the
          game.  It initializes the inventory. */
        void LevelLoaded(bool isRestart);

        /*! Called as soon as the current level is finished; it removes
          the actors of all players from the level. */
        void LevelFinished();

        /* This function is only used by the YinYang items to exchange the
           two players. */
        void SwapPlayers();

        void RedrawInventory();
        void RedrawInventory(Inventory *inv);

        // set/remove respawn positions for all black or all white actors
        // (used when it-flagwhite/black is dropped)
        void SetRespawnPositions(enigma::GridPos pos, bool black);
        void RemoveRespawnPositions(bool black);

        int      CurrentPlayer();
        void     SetCurrentPlayer(unsigned iplayer);
        unsigned NumberOfRealPlayers();


        Inventory *MayPickup(Actor *a, Item *it);
        Inventory *GetInventory(int iplayer);
        Inventory *GetInventory(Actor *a);
        bool       WieldedItemIs(Actor *a, const std::string &kind);

        void Suicide();

        void AddActor (unsigned iplayer, Actor *a);
        void AddUnassignedActor (Actor *a);  // actors not assigned to a player
        bool HasActor(unsigned iplayer, Actor *a);
        Actor *ReplaceActor (unsigned iplayer, Actor *old, Actor *a);
        Actor *GetMainActor (unsigned iplayer);

        bool AllActorsDead();

        void InhibitPickup (bool yesno);
        void PickupItem (Actor *a, enigma::GridPos p);
        void PickupStoneAsItem (Actor *a, enigma::GridPos p);
        void RotateInventory (int dir=1);

        void ActivateFirstItem();
        world::ItemAction ActivateItem (Item *it);

        void Tick (double dtime);

    } // namespace player
} // namespace enigma
#endif
