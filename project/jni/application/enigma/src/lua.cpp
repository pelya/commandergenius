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
#include "lua.hh"
#include "errors.hh"
#include "main.hh"
#include "world.hh"
#include "config.h"
#include "video.hh"
#include "server.hh"
#include "sound.hh"
#include "options.hh"
#include "lev/Index.hh"
#include "lev/Proxy.hh"

#ifndef CXXLUA
extern "C" {
#include "lualib.h"
#include "tolua++.h"
}
#else
#include "lualib.h"
#include "tolua++.h"
#endif 


#include "lua-display.hh"
#include "lua-enigma.hh"
#include "lua-ecl.hh"

#include "ecl.hh"
#include <cassert>

#include "nls.hh"

using namespace std;
using namespace enigma;
using namespace lua;

using ecl::round_down;
using ecl::strf;

using enigma::GridPos;
using world::Object;
using world::GridObject;
using world::ForceField;

namespace lua
{
    int EmitSound (lua_State *L);
    int EmitSoundGlobal (lua_State *L);
    int MakeObject (lua_State *L);
}

namespace 
{
    lua_State *level_state = 0;  // level-local state
    lua_State *global_state = 0; // global Lua state

    lua::Error _lua_err_code (int i)
    {
        switch (i) {
	case 0: return NO_LUAERROR;
	case LUA_ERRRUN: return ERRRUN;
	case LUA_ERRFILE: return ERRFILE;
	case LUA_ERRSYNTAX: return ERRSYNTAX;
	case LUA_ERRMEM: return ERRMEM;
	case LUA_ERRERR: return ERRERR;
	}
	assert (!"Should never get there!");
    }

    void throwLuaError(lua_State * L, const char * message) {
        std::string backtrace = message;
        backtrace += "\nBacktrace:\n";
        lua_Debug dbgInfo;
        int frame = 0;
        while (lua_getstack(L, frame, &dbgInfo)) {
            lua_getinfo(L, "Sl", &dbgInfo);
            if (dbgInfo.source[0] == '@')
                // lua code loaded from file
                backtrace += ecl::strf("#%d %s: %d\n", frame, dbgInfo.source,
                                dbgInfo.currentline);
            else if (dbgInfo.source[0] == '-' && dbgInfo.source[1] == '-' &&
                dbgInfo.source[2] == '@') {
                // lua code loaded from string
                std::string code = dbgInfo.source;
                std::string::size_type slashPosFilenameEnd = code.find('\n');
                std::string::size_type slashPosLineStart;
                std::string::size_type slashPosLineEnd = slashPosFilenameEnd;
                for (int i = 1; i < dbgInfo.currentline; i++) {
                    slashPosLineStart = slashPosLineEnd;
                    slashPosLineEnd = code.find('\n', slashPosLineStart + 1);
                }
                backtrace += ecl::strf("#%d %s: %d\n  (%s)\n", frame, 
                        code.substr(2, slashPosFilenameEnd - 2).c_str(),
                        dbgInfo.currentline - 1,
                        code.substr(slashPosLineStart + 1, slashPosLineEnd - 
                                slashPosLineStart - 1).c_str());
            }
            frame++;
        }
        luaL_error(L, backtrace.c_str());
        
    }
}

/* -------------------- Helper routines -------------------- */

using enigma::Value;

void lua::SetTableVar (lua_State *L,
                       const char *tablename, 
                       const char *name, 
                       double value)
{
    lua_getglobal (L, tablename);
    lua_pushstring (L, name);
    lua_pushnumber (L, value);
    lua_rawset (L, -3);
    lua_pop (L, 1);
}

static void
push_value(lua_State *L, const Value &val)
{
    switch (val.get_type()) {
    case Value::NIL: lua_pushnil(L); break;
    case Value::DOUBLE: lua_pushnumber(L, to_double(val)); break;
    case Value::STRING: lua_pushstring(L, to_string(val)); break;
    }
}

static Value
to_value(lua_State *L, int idx)
{
    switch (lua_type(L, idx)) {
    case LUA_TNIL: return Value();
    case LUA_TNUMBER: return Value(lua_tonumber(L,idx));
    case LUA_TSTRING: return Value(lua_tostring(L,idx));
    case LUA_TBOOLEAN: return (lua_toboolean(L,idx)) ? Value(1) : Value();
    default: throwLuaError(L,"Cannot convert type to Value.");
    }
    return Value();
}

static bool
is_object(lua_State *L, int idx)
{
    return lua_isuserdata(L,idx) && luaL_checkudata(L,idx,"_ENIGMAOBJECT");
}

static Object *
to_object(lua_State *L, int idx)
{
    if (lua_isnil(L,idx))
        return 0;

    if (!is_object(L,idx)) {
        throwLuaError(L, "Cannot convert type to an Object");
        return 0;
    }
    return static_cast<Object*>(*(static_cast<void**>(lua_touserdata(L,idx))));
}

static void
pushobject (lua_State *L, Object *obj)
{
    Object **udata;
    /* Lua does not allow NULL pointers in userdata variables, so
       convert them manually to `nil' values. */
    if (obj == 0)
        lua_pushnil(L);
    else
      {
        udata=(Object**)lua_newuserdata(L,sizeof(int));
	*udata=obj;
        luaL_getmetatable(L, "_ENIGMAOBJECT");
        lua_setmetatable(L, -2);
      }
}


/* -------------------- Interface routines -------------------- */

int lua::MakeObject (lua_State *L)
{
    const char *name = lua_tostring(L, 1);
    if (!name) {
        throwLuaError(L, "MakeObject: string expected as argument");
    }
    Object *obj = world::MakeObject(name);
    if (obj == NULL)
        throwLuaError(L, ecl::strf("MakeObject: unknown object name '%s'", name).c_str());
    pushobject(L, obj);
    return 1;
}

static int
en_get_object_template(lua_State *L)
{
    Object *obj = world::GetObjectTemplate(lua_tostring(L, 1));
    pushobject(L, obj);
    return 1;
}

static int
en_set_attrib(lua_State *L)
{
    Object *obj = to_object(L,1);
    const char *key = lua_tostring(L,2);
    if (obj && key)
        obj->set_attrib(key, to_value(L, 3));
    else
        throwLuaError(L, strf("SetAttrib: invalid object or attribute name '%s'", key).c_str());
    return 0;
}

static int
en_get_attrib(lua_State *L)
{
    Object *obj = to_object(L,1);
    const char *key = lua_tostring(L,2);

    if (!obj) {
        throwLuaError(L, "GetAttrib: invalid object");
        return 0;
    }
    if (!key) {
        throwLuaError(L, "GetAttrib: invalid key");
        return 0;
    }

    if (0 == strcmp(key, "kind")) {
        throwLuaError(L, "GetAttrib: illegal attribute, use GetKind()");
        return 0;
    }

    const Value *v =  obj->get_attrib(key);
    if (!v) {
        // unknown attribute
        lua_pushnil(L);
    }
    else
        push_value(L, *v);
    return 1;
}

static int
en_get_kind(lua_State *L)
{
    Object *obj = to_object(L,1);

    if (!obj) {
        throwLuaError(L, "GetKind: invalid object");
        return 0;
    }

    push_value(L, Value(obj->get_kind()));
    return 1;
}

static int
en_is_same_object(lua_State *L)
{
    Object *obj1 = to_object(L,1);
    Object *obj2 = to_object(L,2);

    lua_pushboolean(L, obj1 == obj2);
    return 1;
}

static int
en_set_floor(lua_State *L)
{
    int x = round_down<int>(lua_tonumber(L, 1));
    int y = round_down<int>(lua_tonumber(L, 2));
    Floor *fl=0;

    if (lua_isnil(L, 3))
        fl = 0;
    else if (is_object(L,3)) {
        fl = static_cast<Floor*>(*(static_cast<void**> (lua_touserdata(L,3))));
    	if( ! fl)
	    throwLuaError(L, "object is no valid floor");
    } else
        throwLuaError(L, "argument 3 must be an Object or nil");
    world::SetFloor(GridPos(x,y), fl);
    return 0;
}

static int
en_set_item(lua_State *L)
{
    int x = round_down<int>(lua_tonumber(L, 1));
    int y = round_down<int>(lua_tonumber(L, 2));
    Item *it = dynamic_cast<Item*>(to_object(L, 3));
    if( ! it) {
        throwLuaError(L, "object is no valid item");
    }
    world::SetItem(GridPos(x,y), it);
    return 0;
}

static int
en_set_stone(lua_State *L)
{
    int x = round_down<int>(lua_tonumber(L, 1));
    int y = round_down<int>(lua_tonumber(L, 2));
    Stone *st = dynamic_cast<Stone*>(to_object(L, 3));
    if( ! st)
        throwLuaError(L, "object is no valid stone");
    world::SetStone(GridPos(x,y), st);
    return 0;
}

static int en_kill_stone(lua_State *L)
{
    int x = round_down<int>(lua_tonumber(L, 1));
    int y = round_down<int>(lua_tonumber(L, 2));
    world::KillStone(GridPos(x,y));
    return 0;
}

static int en_kill_item(lua_State *L)
{
    int x = round_down<int>(lua_tonumber(L, 1));
    int y = round_down<int>(lua_tonumber(L, 2));
    world::KillItem(GridPos(x,y));
    return 0;
}

static int
en_set_actor(lua_State *L)
{
    double x = lua_tonumber(L,1);
    double y = lua_tonumber(L,2);
    Actor *ac = dynamic_cast<Actor*>(to_object(L, 3));
    if( ! ac)
        throwLuaError(L, "object is no valid actor");
    if (world::IsInsideLevel(GridPos(round_down<int>(x), round_down<int>(y))))
        world::AddActor(x, y, ac);
    else
        throwLuaError(L, "position is outside of world");
    return 0;
}


static int
en_send_message(lua_State *L)
{
    Object     *obj = to_object(L, 1);
    const char *msg = lua_tostring(L, 2);
    Value v;
    if (!msg)
        throwLuaError(L,"Illegal message");
    else if (obj) {
        try {
            v = world::SendMessage (obj, msg, to_value(L, 3));
        }
        catch (const XLevelRuntime &e) {
            throwLuaError (L, e.what());
        }
        catch (...) {
            throwLuaError (L, "uncaught exception");
        }
    }
    push_value(L, v);
    return 0;
}

int lua::EmitSound (lua_State *L)
{
    Object     *obj       = to_object(L, 1);
    const char *soundname = lua_tostring(L, 2);
    double vol = 1.0;

    if (lua_isnumber(L, 3)) 
        vol  = lua_tonumber(L, 3);
    if (!soundname)
        throwLuaError(L,"Illegal sound");
    else if (obj) {
        GridObject *gobj = dynamic_cast<GridObject*>(obj);
        if (gobj) {
            if (!gobj->sound_event (soundname, vol)) {
                //throwLuaError(L, strf("Can't find sound '%s'", soundname).c_str());
                // Don't throw an error when no sound file was found.
                // Remember that user sound sets might be incomplete, and
                // absolutely correct levels could throw an error here.
                // Instead, write the "silence string" to the command line:
                sound::WriteSilenceString(soundname);
            }
        }
    }
    else
        throwLuaError(L, "EmitSound: invalid object");

    return 0;
}

int lua::EmitSoundGlobal (lua_State *L)
{
    const char *soundname = lua_tostring(L, 1);
    double vol = 1.0;

    if (lua_isnumber(L, 3)) 
        vol  = lua_tonumber(L, 3);
    if (!soundname)
        throwLuaError(L,"Illegal sound");
    else
        sound::EmitSoundEventGlobal(soundname, vol);

    return 0;
}

static int
en_name_object(lua_State *L)
{
    Object     *obj  = to_object(L, 1);
    const char *name = lua_tostring(L, 2);

    if (!obj) 
        throwLuaError(L, "NameObject: Illegal object");
    else if (!name) 
        throwLuaError(L, "NameObject: Illegal name");
    else
        world::NameObject(obj, name);

    return 0;
}

static int
en_get_named_object(lua_State *L)
{
    Object *o = world::GetNamedObject(lua_tostring(L,1));
    pushobject(L, o);
    return 1;
}

static int
en_get_floor(lua_State *L)
{
    int x = round_down<int>(lua_tonumber(L, 1));
    int y = round_down<int>(lua_tonumber(L, 2));
    Object *o = world::GetFloor(GridPos(x, y));
    pushobject(L, o);
    return 1;
}
static int
en_get_item(lua_State *L)
{
    int x = round_down<int>(lua_tonumber(L, 1));
    int y = round_down<int>(lua_tonumber(L, 2));
    Object *o = world::GetItem(GridPos(x, y));
    pushobject(L, o);
    return 1;
}
static int
en_get_stone(lua_State *L)
{
    int x = round_down<int>(lua_tonumber(L, 1));
    int y = round_down<int>(lua_tonumber(L, 2));
    Object *o = world::GetStone(GridPos(x, y));
    pushobject(L, o);
    return 1;
}

static int
en_get_pos(lua_State *L)
{
    Object  *obj = to_object(L, 1);
    GridPos  p;

    if (!obj) {
        throwLuaError(L, "GetPos: Illegal object");
        return 0;
    }

    if (GridObject *gobj = dynamic_cast<GridObject*>(obj))
        p = gobj->get_pos();
    else if (Actor *a = dynamic_cast<Actor*>(obj)) {
        p = GridPos(a->get_pos());
    }
    else
        p = GridPos(-1, -1);

    lua_pushnumber(L, double(p.x));
    lua_pushnumber(L, double(p.y));
    return 2;
}

static int en_add_constant_force(lua_State *L) {
    ecl::V2 v;
    v[0] = lua_tonumber(L, 1);
    v[1] = lua_tonumber(L, 2);
    world::SetConstantForce (v);
    return 0;
}

static int
en_add_rubber_band (lua_State *L)
{
    Actor  *a1       = dynamic_cast<Actor*> (to_object(L, 1));
    Object *o2       = to_object(L, 2);
    Actor  *a2       = dynamic_cast<Actor*>(o2);
    Stone  *st       = dynamic_cast<Stone*>(o2);
    world::RubberBandData d;
    d.strength  = lua_tonumber (L, 3);
    d.length    = lua_tonumber (L, 4);
    d.minlength = lua_tonumber (L, 5);

    if (!a1)
        throwLuaError(L, "AddRubberBand: First argument must be an actor\n");
    else {
        if (a2)
            world::AddRubberBand (a1, a2, d);
        else if (st)
            world::AddRubberBand (a1, st, d);
        else
            throwLuaError(L, "AddRubberBand: Second argument must be actor or stone\n");
    }
    return 0;
}

static int
en_get_ticks(lua_State *L)
{
    lua_pushnumber(L, SDL_GetTicks());
    return 1;
}

static int
en_is_solved(lua_State *L)
{
    // Function depreceated
    // - filename is no longer a useful reference for levels
    // - levels should not depend on external data for reasons of
    //   network compatibility and level journaling
    throwLuaError(L, "Usage of depreceated function \"IsSolved()\"");

//    const char  *levelname = lua_tostring(L,1);
//    int          solved    = 0;
//    
//    levels::Level level (0, 0);
//    if (levels::FindLevel (levelname, level)) {
//        solved = level.is_solved(options::GetInt("Difficulty"));
//    } else 
//    if (solved)
//        lua_pushnumber(L, solved);
//    else
//        lua_pushnil(L);
    return 1;
}

static int
en_add_scramble(lua_State *L)
{
    int         x       = round_down<int>(lua_tonumber(L, 1));
    int         y       = round_down<int>(lua_tonumber(L, 2));
    const char *dir     = lua_tostring(L,3);
    const char *allowed = "wsen";
    const char *found   = strchr(allowed, dir[0]);

    if (found && found[0]) 
        world::AddScramble(GridPos(x,y), enigma::Direction(found-allowed));
    else 
        throwLuaError(L, "AddScramble: Third argument must be one character of \"wsen\"");

    return 0;
}

static int
en_set_scramble_intensity(lua_State *L)
{
    world::SetScrambleIntensity(int(lua_tonumber(L, 1)));
    return 0;
}

static int
en_add_signal (lua_State *L) {
    const char *sourcestr = lua_tostring (L, 1);
    const char *targetstr = lua_tostring (L, 2);
    const char *msg = lua_tostring (L, 3);

    using namespace enigma;
    GridLoc source, target;

    if (sourcestr == 0 || !to_gridloc(sourcestr, source))
        throwLuaError (L, "AddSignal: invalid source descriptor");
    if (targetstr == 0 || !to_gridloc(targetstr, target)) 
        throwLuaError (L, "AddSignal: invalid target descriptor");
    if (msg == 0)
        msg = "signal";

    world::AddSignal (source, target, msg);
    return 0;
}

int loadLib(lua_State *L) 
{
    const char *id = lua_tostring(L, 1);
    lev::Proxy * curProxy = lev::Index::getCurrentProxy();
    try {
        curProxy->loadDependency(id);
    } catch (XLevelLoading &err) {
        throwLuaError(L, err.what());
    }
    return 1;
}

static CFunction globalfuncs[] = {
    {FindDataFile,          "FindDataFile"},
//    {lua::PlaySoundGlobal,  "PlaySoundGlobal"},
//    {lua::PlaySound,        "PlaySound"},
    {en_get_ticks,             "GetTicks"},
    {0,0}
};

static CFunction levelfuncs[] = {

    // internal functions

    {FindDataFile,          "FindDataFile"},
    {loadLib,               "LoadLib"},
    {en_get_object_template,"GetObjectTemplate"},
    {lua::MakeObject,       "MakeObject"},
    {en_set_actor,          "SetActor"},

    // finding objects

    {en_get_named_object,   "GetNamedObject"},

    {en_get_floor,          "GetFloor"},
    {en_get_item,           "GetItem"},
    {en_get_stone,          "GetStone"},

    // information from objects

    {en_get_pos,            "GetPos"},
    {en_get_attrib,         "GetAttrib"},
    {en_get_kind,           "GetKind"},
    {en_is_same_object,     "IsSameObject"},

    // manipulating objects

    {en_set_attrib,         "SetAttrib"},
    {en_send_message,       "SendMessage"},
    {en_name_object,        "NameObject"},

    // sound effects

    {lua::EmitSound,        "EmitSound"},

    // manipulating level

    {en_set_floor,          "SetFloor"},
    {en_set_item,           "SetItem"},
    {en_set_stone,          "SetStone"},

    {en_kill_stone,         "KillStone"},
    {en_kill_item,          "KillItem"},


    // signals
    {en_add_signal,         "AddSignal"},

    // access/modify global data

    {en_get_ticks,          "GetTicks"},
    {en_is_solved,          "IsSolved"},

    {en_add_constant_force,     "AddConstantForce"},
    {en_add_rubber_band,        "AddRubberBand"},
    {en_add_scramble,           "AddScramble"},
    {en_set_scramble_intensity, "SetScrambleIntensity"},


    {0,0}
};


/* -------------------- lua:: functions -------------------- */

int lua::FindDataFile (lua_State *L) 
{
    const char *filename = lua_tostring(L, 1);
    string absfile;
    if (app.resourceFS->findFile(filename, absfile)) 
        lua_pushstring(L, absfile.c_str());
    else
        lua_pushnil (L);
    return 1;
}

void lua::RegisterFuncs(lua_State *L, CFunction *funcs) 
{
    lua_getglobal(L, "enigma");
    for (unsigned i=0; funcs[i].func; ++i) {
        lua_pushstring(L, funcs[i].name);
        lua_pushcfunction(L, funcs[i].func);
        lua_settable(L, -3);
    }
    lua_pop(L, 1);
}

Error lua::CallFunc(lua_State *L, const char *funcname, const Value& arg, world::Object *obj) {
    int retval;
    lua_getglobal(L, funcname);
    push_value(L, arg);
    pushobject(L, obj);
    retval=lua_pcall(L,2,0,0);
    if (retval!=0) // error
      {
	lua_setglobal (L, "_LASTERROR") ; //Set _LASTERROR to returned error message
      }
    return _lua_err_code(retval);
}

Error lua::CallFunc(lua_State *L, const char *funcname, const ByteVec& arg) {
  int retval;
    lua_getglobal(L, funcname);
    lua_pushlstring (L, &arg[0], arg.size());
    retval=lua_pcall(L,1,0,0);
    if (retval!=0) // error
      {
	lua_setglobal (L, "_LASTERROR") ; //Set _LASTERROR to returned error message
      }
    return _lua_err_code(retval);
}

Error lua::DoAbsoluteFile(lua_State *L, const string &filename)
{
    int oldtop = lua_gettop(L);
    int retval = luaL_loadfile(L, filename.c_str());
    if (retval!=0) // error
    {
      lua_setglobal (L, "_LASTERROR") ; //Set _LASTERROR to returned error message
    }
    else 
    {
      retval= lua_pcall(L, 0, 0, 0);
      if (retval!=0) // error
      {
        lua_setglobal (L, "_LASTERROR") ; //Set _LASTERROR to returned error message
      }
    }
    lua_settop(L, oldtop);
    return _lua_err_code(retval);
}
    
Error lua::DoGeneralFile(lua_State *L, GameFS * fs, const string &filename)
{
    string completefn;
    if (fs->findFile(filename, completefn)) {
        return lua::DoAbsoluteFile(L, completefn);
    }
    else {
        return _lua_err_code(LUA_ERRFILE);
    }
}

Error lua::Dofile(lua_State *L, const string &filename) 
{
    return lua::DoGeneralFile(L, app.resourceFS, filename);
}

Error lua::DoSysFile(lua_State *L, const string &filename) 
{
    return lua::DoGeneralFile(L, app.systemFS, filename);
}

void lua::CheckedDoFile (lua_State *L, GameFS * fs, std::string const& fname)
{
    string completefn;
    if (!fs->findFile(fname, completefn))
    {
        fprintf(stderr, _("Cannot find '%s'.\n"), fname.c_str());
        fprintf(stderr, _("Your installation may be incomplete or invalid.\n"));
        exit (1);
    }

    lua::Error status = lua::DoAbsoluteFile(L, completefn);
    if (status != lua::NO_LUAERROR) {
        fprintf(stderr, _("There was an error loading '%s'.\n"), completefn.c_str());
        fprintf(stderr, _("Your installation may be incomplete or invalid.\n"));
	fprintf(stderr, _("Error: '%s'\n"), lua::LastError(L).c_str());
        exit (1);
    }
}


Error lua::Dobuffer (lua_State *L, const ByteVec &luacode) {
  int retval;
    const char *buffer = reinterpret_cast<const char *>(&luacode[0]);
    
    retval=luaL_loadbuffer(L, buffer, luacode.size(), "buffer");
    if (retval!=0) // error
      {
	lua_setglobal (L, "_LASTERROR") ; //Set _LASTERROR to returned error message
      }
      else 
	{
	  retval= lua_pcall(L, 0, 0, 0);
	  if (retval!=0) // error
          {
	    lua_setglobal (L, "_LASTERROR") ; //Set _LASTERROR to returned error message
          }
	}
    return _lua_err_code(retval);
}

string lua::LastError (lua_State *L)
{
    lua_getglobal (L, "_LASTERROR");
    if (lua_isstring(L,-1)){
      return string (lua_tostring (L, -1));
    }
    else {
      return "Lua Error. No error message available.";
    }
}


Error lua::DoSubfolderfile(lua_State *L, const string &basefolder, const string &filename) {
    std::list <string> fnames = app.resourceFS->findSubfolderFiles(basefolder, filename);
    int retval = 0;
    while (fnames.size() > 0) {
        int oldtop = lua_gettop(L);
	string fname = fnames.front();
        retval = luaL_loadfile(L, fname.c_str());
        if (retval!=0) // error
        {
	  lua_setglobal (L, "_LASTERROR") ; //Set _LASTERROR to returned error message
        }
	else 
	{
	  retval= lua_pcall(L, 0, 0, 0);
	  if (retval!=0) // error
          {
	    lua_setglobal (L, "_LASTERROR") ; //Set _LASTERROR to returned error message
          }
	}
	fnames.pop_front();
        lua_settop(L, oldtop);
    }
    return _lua_err_code(retval);
}

lua_State *lua::GlobalState() 
{
    if (global_state == 0) {
        lua_State *L = global_state = lua_open();

        luaL_openlibs(L);
        CheckedDoFile(L, app.systemFS, "compat.lua");

        tolua_open(L);
        tolua_enigma_open(L);

        RegisterFuncs(L, globalfuncs);
    }
    return global_state;
}

void lua::ShutdownGlobal() 
{
    assert (global_state);
    lua_close(global_state);
    global_state = 0;
}

#ifdef _MSC_VER
#define snprintf _snprintf
#endif



lua_State *lua::InitLevel() 
{
    char buffer[255];

    lua_State *L = level_state = lua_open();
    luaL_dostring (L, "options={}");
    snprintf (buffer, sizeof(buffer), "options.Difficulty = %d", 
              server::GetDifficulty());
    luaL_dostring (L, buffer);

    luaL_openlibs(L);

    tolua_open(L);
    tolua_enigma_open(L);
    tolua_px_open(L);
    tolua_display_open(L);

    RegisterFuncs(L, levelfuncs);

    // Create a new metatable for world::Object objects
    luaL_newmetatable(L,"_ENIGMAOBJECT");
    return L;
}

lua_State *lua::LevelState ()
{
    return level_state;
}

void lua::ShutdownLevel() {
    if (level_state) {
        lua_close(level_state);
        level_state = 0;
    }
}

