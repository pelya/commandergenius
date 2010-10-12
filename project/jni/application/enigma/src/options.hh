/*
 * Copyright (C) 2002, 2003 Daniel Heck
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
#ifndef OPTIONS_HH
#define OPTIONS_HH

/*
** Options are ``persistent'' settings that can saved to a Lua file or
** loaded from it.  This file defined the C++ interface to options;
** loading and saving are handled by Lua functions defined in
** `startup.lua'.
*/

#include <ctime>

namespace enigma_options
{
    using namespace enigma;

/* -------------------- LevelStatus -------------------- */

    /*! 
     * This class stores information about the user's progress with a
     * particular level.  This is the information that is stored in
     * the .enigmarc files.  See for options.cc and startup.lua for
     * the relevant I/O code.
     */
    struct LevelStatus {
        LevelStatus(int easy=-1, int hard=-1, int fin=0, int solved_rev = 0);
        bool operator == (const LevelStatus& other) const;

        // Variables.

        int time_easy;         // user's best time in seconds (-1: NA)
        int time_hard;         // user's best time in seconds (-1: NA)
        int finished;          // Level solved? 0 = no,1=easy,2=hard,3=easy&hard
        int solved_revision;   // Revision #  that was solved
    };

/* -------------------- Constants -------------------- */

    const int MIN_MouseSpeed = 1;
    const int MAX_MouseSpeed = 15;

/* -------------------- Variables -------------------- */

    /*! An option was changed that will not take effect until Enigma is
      restarted. */
//    extern bool MustRestart;

    /*! An option was changed that makes it necessary to restart the
      current level (e.g. Difficulty changed during the game).  When
      'MustRestartLevel' is set to true, the current level will
      restart w/o any further questions. */
//    extern bool MustRestartLevel;

/* -------------------- Functions -------------------- */

    bool HasOption (const char *name, std::string &value);
    void SetOption (const char *name, double value);
    void SetOption (const char *name, const std::string &value);
    void GetOption (const char *name, double &value);
    void GetOption (const char *name, std::string &value);

    bool        GetBool (const char *name);
    double      GetDouble (const char *name);
    int         GetInt (const char *name);
    std::string GetString (const char *name);


    double SetMouseSpeed (double speed);
    double GetMouseSpeed ();


    /*! Get the status of a particular level.
      Returns false if no record for this level exists. */
    bool GetLevelStatus (const std::string &levelname,
                         LevelStatus &stat);


    /*! Set the sound and music volume to the values in SoundVolume
      and MusicVolume. */
    void UpdateVolume();

    /*! Try to load the user's configuration file.  Returns true if
      successful. */
    bool Load ();
}

namespace enigma
{
    namespace options = enigma_options;
}
#endif
