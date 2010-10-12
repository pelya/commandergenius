/*
 * Copyright (C) 2006 Daniel Heck
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
#include "enigma.hh"

namespace enigma
{
    /** 
     * This class maintains a list of time handlers that are either
     * invoked at every tick or after a specified amount of time.  If
     * the TimeHandler is registered using #activate, it is invoked at
     * every tick, the #set_alarm method can be used to register a
     * time handler that is invoked (either once or repeatedly) after
     * a specified time interval.
     */
    class Timer : public ecl::Nocopy {
    public:
        Timer();
        ~Timer();
        void activate (TimeHandler *th);
        void deactivate (TimeHandler* th);
        void set_alarm (TimeHandler* th, double interval, bool repeatp = false);
        void remove_alarm (TimeHandler *th);
        void clear();

        void tick(double dtime);
    private:
        struct Rep;
        Rep &self;
    };
}
