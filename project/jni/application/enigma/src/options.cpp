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
#include "errors.hh"
#include "lua.hh"
#include "main.hh"
#include "options.hh"
#include "PreferenceManager.hh"
#include "sound.hh"
#include "ecl_system.hh"

#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <set>

#ifndef CXXLUA
extern "C" {
#include <lua.h>
#include <lauxlib.h>
}
#else
#include <lua.h>
#include <lauxlib.h>
#endif 

#ifdef __MINGW32__
#       include <windows.h>
#endif

using namespace enigma;


/* -------------------- LevelStatus implementation -------------------- */

namespace enigma_options
{
    LevelStatus::LevelStatus(int easy, int hard, int finished_, int solved_rev)
        : time_easy (easy),
          time_hard (hard),
          finished (finished_),
          solved_revision (solved_rev) {
    }

    bool LevelStatus::operator == (const LevelStatus& other) const  {
        return (time_easy == other.time_easy &&
                time_hard == other.time_hard &&
                finished == other.finished &&
                solved_revision == other.solved_revision);
    }
}

/* -------------------- Variables -------------------- */

namespace enigma_options
{
    bool LevelStatusChanged = false;
//    bool MustRestart        = false;
//    bool MustRestartLevel   = false;
}


/* -------------------- Functions -------------------- */

bool options::HasOption (const char *name, std::string &value) {
    bool hasOption;
    const char * result;
    lua_State *L = lua::GlobalState();
    lua_getglobal (L, "options");
    lua_pushstring (L, name);
    lua_rawget (L, -2);
    result = lua_tostring (L, -1);
    if (result != NULL) {
        hasOption = true;
        value = result;
    } else {
        hasOption = false;
    }
    lua_pop (L, 2);
    return hasOption;
} 

void options::SetOption (const char *name, double value)
{
    app.prefs->setProperty(name, value);
}

void options::SetOption (const char *name, const std::string &value)
{
    app.prefs->setProperty(name, value);
}

void options::GetOption (const char *name, double &value)
{
    app.prefs->getProperty(name, value);
}

void options::GetOption (const char *name, std::string &value) {
    app.prefs->getProperty(name, value);
} 

bool options::GetBool (const char *name) {
    double val = 0;
    GetOption (name, val);
    return val != 0;
}

double options::GetDouble (const char *name) {
    double val = 0;
    GetOption (name, val);
    return val;
}

int options::GetInt (const char *name) {
    double val = 0;
    GetOption (name, val);
    return static_cast<int>(val);
}


double options::SetMouseSpeed (double speed) 
{
    double oldspeed = GetMouseSpeed();
    double newspeed = ecl::Clamp<double>(speed, MIN_MouseSpeed, MAX_MouseSpeed);
    SetOption("MouseSpeed", newspeed);
    return oldspeed;
}

double options::GetMouseSpeed () 
{
    return GetDouble ("MouseSpeed");
}

void options::UpdateVolume() 
{
    sound::SetSoundVolume (GetDouble("SoundVolume"));
    sound::SetMusicVolume (GetDouble("MusicVolume"));
}

std::string options::GetString (const char *name) 
{
    std::string val;
    GetOption (name, val);
    return val;
}

static void UpdateLevelStatus(const std::string &levelname,
                              const options::LevelStatus &stat)
{
    lua_State *L = lua::GlobalState();

    int oldtop = lua_gettop(L);

    lua_getglobal(L, "stats");
    lua_pushstring (L, levelname.c_str());
    lua_newtable (L);
    lua_pushnumber (L, stat.time_easy);       lua_rawseti (L, -2, 1);
    lua_pushnumber (L, stat.time_hard);       lua_rawseti (L, -2, 2);
    lua_pushnumber (L, stat.finished);        lua_rawseti (L, -2, 3);
    lua_pushnumber (L, stat.solved_revision); lua_rawseti (L, -2, 4);
    lua_rawset (L, -3);         // stats[levename] =

    lua_settop(L, oldtop);
}

bool options::GetLevelStatus (const std::string &levelname,
                              LevelStatus &stat)
{
    lua_State *L = lua::GlobalState();

    int oldtop = lua_gettop(L);

    lua_getglobal(L, "stats");
    lua_pushstring (L, levelname.c_str());
    lua_rawget (L, -2);
    if (!lua_istable(L, -1) || luaL_getn(L, -1)!=4) {
        lua_settop(L, oldtop);
        return false;
    }

    lua_rawgeti (L, -1, 1); stat.time_easy       = (int) lua_tonumber (L, -1); lua_pop(L, 1);
    lua_rawgeti (L, -1, 2); stat.time_hard       = (int) lua_tonumber (L, -1); lua_pop(L, 1);
    lua_rawgeti (L, -1, 3); stat.finished        = (int) lua_tonumber (L, -1); lua_pop(L, 1);
    lua_rawgeti (L, -1, 4); stat.solved_revision = (int) lua_tonumber (L, -1);

    lua_settop(L, oldtop);
    
    if (stat.solved_revision == 0 && stat.finished>0) {
        // revision 0 is an error (lowest allowed revision number is 1)
        printf("Auto-increasing revision number of '%s'\n", levelname.c_str());
        stat.solved_revision = 1;
        UpdateLevelStatus(levelname, stat); // update this correction
    }

    return true;
}


/* Determine name of the user's personal configuration file. */
static std::string
Personal_ConfigurationFileName()
{
    std::string fname = "enigmarc.lua";

    if (getenv ("HOME") != 0)
        fname = ecl::ExpandPath ("~/.enigmarc");

    return fname;
}

static std::string
System_ConfigurationFileName()
{
    return app.systemFS->findFile ("enigma_conf.lua");
}

#ifdef __MINGW32__

static std::string
Windows_ConfigurationFileName()
{
    std::string result = ecl::ApplicationDataPath();
    
    if (!result.empty()) 
        return result + "/enigmarc.lua";
    else
        return Personal_ConfigurationFileName();
    
}

#endif


static bool
load_from_file (const std::string &fname)
{
    lua::Error errcode = lua::DoAbsoluteFile(lua::GlobalState(), fname.c_str());
    bool success =  (errcode == 0 || errcode == lua::ERRFILE);
    if (!success) {
        enigma::Log << "error in file `" << fname <<"'\n";
    }
    return success;
}

static bool
load_options (const std::string &fname)
{
    bool success1 = load_from_file (fname);
    bool success2 = load_from_file (fname + "2");
    return success1 && success2;
}


bool options::Load ()
{
    std::string fname;
    bool success = true;

    fname = System_ConfigurationFileName();
    success &= load_options (fname);

#ifdef __MINGW32__
    fname  = Windows_ConfigurationFileName();
    success &= load_options (fname);
#endif

    // personal config (in $HOME or current directory) overides all!
    fname  = Personal_ConfigurationFileName();
    success &= load_options (fname);

    LevelStatusChanged = false;

    return success;
}

