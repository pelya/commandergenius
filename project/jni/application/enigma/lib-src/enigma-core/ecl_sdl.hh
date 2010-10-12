/*
 * Copyright (C) 2002,2003,2005 Daniel Heck
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

/*
** SDL wrappers & helper classes
*/

#ifndef ECL_SDL_HH
#define ECL_SDL_HH

#include "ecl_geom.hh"
#include "SDL.h"

namespace sdl
{
    class EventHandler {
    public:
        virtual ~EventHandler() {}

        /* Dispatch an event to the suitable virtual function. Returns
           true if event was handled. */
        bool dispatch_event (SDL_Event &e);

        /*
        ** The following function can be overriden to receive
        ** particular events.  They should return true if the event
        ** was handled.
        */
        virtual bool on_mousemotion (SDL_Event &/*e*/) { return false; }
        virtual bool on_mousebutton (SDL_Event &/*e*/) { return false; }
        virtual bool on_keydown (SDL_Event &/*e*/) { return false; }
        virtual bool on_keyup (SDL_Event &/*e*/) { return false; }
        virtual bool on_quit (SDL_Event &/*e*/) { return false; }

        /* The generic event handler; this method is called for events
           that none of the specialized methods above decided to handle. */
        virtual bool on_event (SDL_Event &/*e*/) { return false; }
    };

    class TempInputGrab {
    public:
        TempInputGrab(SDL_GrabMode m)
        : grabmode(SDL_WM_GrabInput(SDL_GRAB_QUERY))
        {
            SDL_WM_GrabInput(m);
        }
        ~TempInputGrab() { SDL_WM_GrabInput(grabmode); }
    private:
        SDL_GrabMode grabmode;
    };

    inline void copy_rect (SDL_Rect &s, const ecl::Rect &from) {
        s.x = from.x; s.y = from.y;
        s.w = from.w; s.h = from.h;
    }

    void FlushEvents();
}
#endif
