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
#include "game.hh"
#include "errors.hh"
#include "main.hh"
#include "options.hh"
#include "video.hh"
#include "client.hh"
#include "server.hh"
#include "world.hh"
#include "sound.hh"
#include "lev/PersistentIndex.hh"

#include "ecl_sdl.hh"
#include <cassert>

using namespace enigma;
using namespace std;


/* -------------------- Global variables -------------------- */

namespace
{
    Uint32      last_tick_time;
}


/* -------------------- Level previews -------------------- */

bool game::DrawLevelPreview (ecl::GC &gc, lev::Proxy *levelProxy)
{
    bool success = false;

    sound::TempDisableSound();
    try {
        server::Msg_LoadLevel (levelProxy, true);

        display::DrawAll(gc);
        success = true;
    }
    catch (XLevelLoading &err) {
        // log the message as we cannot display it on the screen
        Log << "DrawLevelPreview load error:\n" << err.what();
        success = false;
    }
    sound::TempReEnableSound();
    return success;
}



/* -------------------- Functions -------------------- */

void game::StartGame ()
{
    lev::Index *ind = lev::Index::getCurrentIndex();
    server::InitNewGame();

    video::HideMouse();
    sdl::TempInputGrab grab(enigma::Nograb ? SDL_GRAB_OFF : SDL_GRAB_ON);

    server::Msg_LoadLevel(ind->getCurrent(), false);

    double dtime = 0;
    last_tick_time=SDL_GetTicks();
    while (!client::AbortGameP()) {
        try {
            client::Tick (dtime);
            server::Tick (dtime);
        }
        catch (XLevelRuntime& err) {        
            client::Msg_Error (string("Server Error: level runtime error:\n")
                               + err.what());
            server::Msg_Panic(true);
        }

        int sleeptime = 10 - (SDL_GetTicks()-last_tick_time);
        if (sleeptime >= 3)      // only sleep if relatively idle
            SDL_Delay(sleeptime);

        Uint32 current_tick_time = SDL_GetTicks();
        dtime = (current_tick_time - last_tick_time)/1000.0;

        if (abs(1-dtime/0.01) < 0.2) {
            // less than 20% deviation from desired frame time?
            dtime = 0.01;
            last_tick_time += 10;
        } else {
            last_tick_time = current_tick_time;
        }

	if (dtime > 500.0) /* Time has done something strange, perhaps
			      run backwards */
            dtime = 0.0;
// 	else if (dtime > 0.5)
//             dtime = 0.5;
    }
    // add last played level
    lev::PersistentIndex::addCurrentToHistory();

    video::ShowMouse();
}

void game::ResetGameTimer() 
{
    last_tick_time=SDL_GetTicks();
}


