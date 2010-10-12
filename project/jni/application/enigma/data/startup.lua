------------------------------------------------------------------------
-- Copyright (C) 2002,2003,2004,2005 Daniel Heck
--
-- This program is free software; you can redistribute it and/or
-- modify it under the terms of the GNU General Public License
-- as published by the Free Software Foundation; either version 2
-- of the License, or (at your option) any later version.
--
-- This program is distributed in the hope that it will be useful,
-- but WITHOUT ANY WARRANTY; without even the implied warranty of
-- MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
-- GNU General Public License for more details.
--
-- You should have received a copy of the GNU General Public License along
-- with this program; if not, write to the Free Software Foundation, Inc.,
-- 59 Temple Place, Suite 330, Boston, MA 02111-1307, USA.
--
-- $Id: startup.lua,v 1.21 2004/02/15 11:54:36 dheck Exp $
------------------------------------------------------------------------

function Require(filename)
    dofile(enigma.FindDataFile(filename))
end

---------------------
-- Option handling --
---------------------

-- Set default values for options
options = {
    MouseSpeed        = 7.0,
    FullScreen        = 0,
    Nozoom            = 1,
    InGameMusic       = 0,                  -- no music during the game
    Difficulty        = 2,                  -- hard
    Tournament        = 0,                  -- tournament mode off
    VideoMode         = 0,
    Gamma             = 1.0,  
    LevelMenuPosition = 0.0,

    SoundSet          = 0,
    -- 0   = 'enigma' for enigma, appropriate oxyd sound sets for oxyd versions
    -- 1   = 'enigma'
    -- 2.. = OxydVersion-2

    SkipSolvedLevels = 0,
    TimeHunting      = 0,

    SoundVolume      = 1.0,
    MusicVolume      = 1.0,
    StereoSeparation = 10.0,

    MenuMusicFile  = "sound/menu.ogg",
    LevelMusicFile = "sound/Emilie.xm",

    Language = "",
    History = "",
}

-- Table of level stats is initially empty
stats = {}


-- For backwards compatibility only --
function options.LevelStat2 (key, stat)
    if stat[1] < 0 then stat[1] = -1 end
    if stat[2] < 0 then stat[2] = -1 end
    if stat[3] < 0 then stat[3] = 0 end

    gsub(key, "([^#]*)#(.+)", function (pack,lev) stats[lev] = stat end)
end

-- For backwards compatibility only --
function options.LevelStat (key, stat)
    -- old level status (containing modtime of .lua file)
    -- [keep this function here for compatibility with old config files]

    -- Enigma accidentally used to save times in the wrong numeric format.
    -- The following two lines work around this problem
    if stat[1] < 0 then stat[1] = -1 end
    if stat[2] < 0 then stat[2] = -1 end
    if stat[3] < 0 then stat[3] = 0 end

    stat[4] = stat[4] or 0
    if stat[4] > 0 then 
        stat[4] = 1 
    end -- level status has 'solved_at' time -> default to revision 1

    gsub(key, "([^#]*)#(.+)", function (pack,lev) stats[lev] = stat end)
end

function LoadOptions (filename)
    dofile(filename)
end


----------------------------------
-- Define some new object types --
----------------------------------
function def_stone(name, sound)
    world.DefineSimpleStone(name,sound or "stone",0,0)
end
function def_stone_hollow(name)
    world.DefineSimpleStone(name,"",1,0)
end
function def_stone_glass(name, sound)
    world.DefineSimpleStone(name,sound or "stone",0,1)
end
function def_stone_movable_glass(name, sound)
    world.DefineSimpleStoneMovable(name,sound or "stone",1)
end
function def_stone_movable(name, sound)
    world.DefineSimpleStoneMovable(name,sound or "stone",0)
end
function def_floor(name, frict, mfactor, burnable, firetransform)
    world.DefineSimpleFloor(name, frict, mfactor, burnable, firetransform)
end

def_stone_movable("st-brownie", "cloth")
def_stone("st-invisible")
def_stone("st-marble")
def_stone_movable("st-marble_move")
def_stone_hollow("st-marble_hole")
def_stone("st-rock1")
def_stone_movable("st-rock1_move")
def_stone_hollow("st-rock1_hole")
def_stone("st-rock2")
def_stone_hollow("st-rock2_hole")
def_stone("st-rock3")
def_stone_movable("st-rock3_move")
def_stone_hollow("st-rock3_hole")
def_stone("st-rock4")
def_stone("st-rock5")
def_stone("st-rock6")
def_stone("st-rock7")
def_stone("st-rock8")
def_stone("st-rock9")
def_stone("st-rock10")
def_stone("st-redrock")
def_stone("st-camouflage", "cloth")
def_stone_movable("st-camouflage_move", "cloth")
def_stone_hollow("st-camouflage_hole")
def_stone("st-greenbrown")
def_stone_movable("st-greenbrown_move", "cloth")
def_stone_hollow("st-greenbrown_hole")
def_stone("st-brick")
def_stone("st-woven")
def_stone("st-wood_001")
def_stone("st-bluegray")
def_stone_hollow("st-bluegray_hole")
def_stone("st-stone1")
def_stone("st-stone2")
def_stone("st-bumps")
def_stone("st-beads")
def_stone("st-metal")
def_stone_hollow("st-metal_hole")
def_stone("st-blue-sand")
def_stone("st-yellow")

function mkoxydlikes(flavor)
   def_stone("st-likeoxyd"..flavor)
   def_stone("st-likeoxyd"..flavor.."-open")
end
mkoxydlikes("a")
mkoxydlikes("b")
mkoxydlikes("c")
mkoxydlikes("d")

def_stone_glass("st-glass")
def_stone_movable_glass("st-glass_move")
def_stone_glass("st-glass1")
def_stone_movable_glass("st-glass1_move")
def_stone_hollow("st-glass1_hole")
def_stone_glass("st-glass2")
def_stone_movable_glass("st-glass2_move")
def_stone_hollow("st-glass2_hole")
def_stone_glass("st-glass3")

-- Floor(kind, friction, mousefactor, burnable, firetransform)
def_floor("fl-bluegray",     4.5,   1.5,    true,   "")
def_floor("fl-bluegreen",    6.0,   2.0,    true,   "")
def_floor("fl-bluegreenx",   0.4,   1.0,    true,   "")
def_floor("fl-brick",        3.5,   2.0,   false,   "")
def_floor("fl-bumps",        5.0,   1.2,   false,   "")
def_floor("fl-concrete",     4.5,   1.3,   false,   "")
def_floor("fl-dunes",        1.3,   1.0,   false,   "")
def_floor("fl-floor_001",    1.5,   2.5,    true,   "fl-abyss")
def_floor("fl-gravel",       3.2,   1.5,   false,   "")
def_floor("fl-gray",         5.0,   3.0,   false,   "")
def_floor("fl-hay",          5.0,   1.5,    true,   "")
def_floor("fl-himalaya",     5.0,   2.0,   false,   "")
def_floor("fl-inverse",      3.0,  -2.0,   false,   "")
def_floor("fl-inverse2",     3.0,  -2.0,   false,   "")
def_floor("fl-leaves",       4.0,   1.5,    true,   "fl-dunes")
def_floor("fl-leavesb",      4.0,   1.5,    true,   "fl-dunes")
def_floor("fl-leavesc1",     4.0,   1.5,    true,   "fl-dunes")
def_floor("fl-leavesc2",     4.0,   1.5,    true,   "fl-dunes")
def_floor("fl-leavesc3",     4.0,   1.5,    true,   "fl-dunes")
def_floor("fl-leavesc4",     4.0,   1.5,    true,   "fl-dunes")
def_floor("fl-leavesd1",     4.0,   1.5,    true,   "fl-dunes")
def_floor("fl-leavesd2",     4.0,   1.5,    true,   "fl-dunes")
def_floor("fl-leavesd3",     4.0,   1.5,    true,   "fl-dunes")
def_floor("fl-leavesd4",     4.0,   1.5,    true,   "fl-dunes")
def_floor("fl-leavese1",     4.0,   1.5,    true,   "fl-dunes")
def_floor("fl-leavese2",     4.0,   1.5,    true,   "fl-dunes")
def_floor("fl-leavese3",     4.0,   1.5,    true,   "fl-dunes")
def_floor("fl-leavese4",     4.0,   1.5,    true,   "fl-dunes")
def_floor("fl-light",        2.2,   1.6,    true,   "")
def_floor("fl-lightgray",    3.0,   1.6,    true,   "")
def_floor("fl-darkgray",     3.0,   1.6,   false,   "")
def_floor("fl-marble",       6.4,   2.0,    true,   "")
def_floor("fl-metal",        3.0,   2.0,   false,   "")
def_floor("fl-metal1",       3.0,   2.0,   false,   "")
def_floor("fl-metal2",       3.0,   2.0,   false,   "")
def_floor("fl-metal3",       3.0,   2.0,   false,   "")
def_floor("fl-metal4",       3.0,   2.0,   false,   "")
def_floor("fl-metal5",       3.0,   2.0,   false,   "")
def_floor("fl-metal6",       3.0,   2.0,   false,   "")
def_floor("fl-mortar",       7.2,   1.8,   false,   "")
def_floor("fl-normal",       4.0,   2.0,   false,   "")
def_floor("fl-normal_x",     3.0,   2.0,   false,   "")
def_floor("fl-plank",        5.5,   2.0,    true,   "fl-abyss")
def_floor("fl-red",          0.9,   1.0,    true,   "")
def_floor("fl-rock",         6.5,   2.2,   false,   "")
def_floor("fl-rough",        7.0,   2.0,    true,   "")
def_floor("fl-rough_medium", 5.0,   1.2,    true,   "")
def_floor("fl-rough_slow",   7.0,   0.5,    true,   "")
def_floor("fl-rough-blue",   7.0,   2.0,    true,   "")
def_floor("fl-rough-red",    7.0,   2.0,    true,   "")
def_floor("fl-sahara",       6.4,   2.0,   false,   "")
def_floor("fl-samba",        6.0,   2.0,    true,   "fl-abyss")
def_floor("fl-samba1",       6.0,   2.0,    true,   "fl-abyss")
def_floor("fl-samba2",       6.0,   2.0,    true,   "fl-abyss")
def_floor("fl-sand",         6.0,   2.0,   false,   "")
def_floor("fl-space",        0.0,   0.0,   false,   "")
def_floor("fl-springboard",  4.0,   2.0,   false,   "")
def_floor("fl-stone",        1.4,   1.0,   false,   "")
def_floor("fl-tigris",       6.0,   2.0,    true,   "")
def_floor("fl-wood",         6.4,   2.0,    true,   "fl-abyss")
def_floor("fl-wood1",        6.4,   2.0,    true,   "fl-abyss")
def_floor("fl-wood2",        6.4,   2.0,    true,   "fl-abyss")
def_floor("fl-woven",        6.5,   3.0,    true,   "")
def_floor("fl-woven_orange", 2.5,   0.0,    true,   "")
def_floor("fl-black",        3.0,   1.5,   false,   "")
def_floor("fl-white",        3.0,   1.5,   false,   "")
def_floor("fl-trigger",      3.0,   1.5,    true,   "")
def_floor("fl-abyss_fake",   3.0,   2.0,   false,   "")

