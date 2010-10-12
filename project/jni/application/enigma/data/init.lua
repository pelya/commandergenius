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
------------------------------------------------------------------------

TRUE = 1
FALSE = 0

EASY = 1
HARD = 2

difficult = options.Difficulty==2

function Tick (deltatime)
    -- do nothing by default
end


----------------------
-- Global variables --
----------------------

level_width = 0
level_height = 0

oxyd_default_flavor = "b"       -- Default flavor for oxyd stones.
oxyd_count = 0
oxyd_current_color = 0

DefaultAttribs = {}

---------------------
-- Helper routines --
---------------------

function PrintTable(t)
    for i,v in pairs(t) do
        if type(v)=="table" then
            print (i.." - "..v[1]..","..v[2])
        else
            print (i.." - "..v)
        end
    end
end


function Require(filename)
    enigma.LoadLib(string.sub(filename,8,string.len(filename)-4))
end

-- randseed is issued by Enigma application and must not be disturbed
math.randomseed = function () end

----------------------------------
-- Routines for creating levels --
----------------------------------

function CreateWorld(w, h)
    oxyd_default_flavor = "b"
    oxyd_count = 0
    oxyd_current_color = 0
    DefaultAttribs = {}

    level_width = w
    level_height = h
    world.Resize(w, h)
end


function SetDefaultAttribs(objname, attribs)
    local list = DefaultAttribs[objname]
    if list then
        for key,val in pairs(attribs) do list[key] = val end
    else
        DefaultAttribs[objname]= attribs
    end
end

function SetAttrib(obj, key,val)
    if key == "name" then
        enigma.NameObject(obj, val)
    end
    enigma.SetAttrib(obj, key, val)
end

GetAttrib = enigma.GetAttrib
GetKind   = enigma.GetKind

function set_attribs(obj, attrs)
    if not attrs then return end
    for key,val in pairs(attrs) do
	set_attrib(obj, key, val)
    end
end

function MakeObject(name, attrs)
    local obj = enigma.MakeObject(name)
    local a=DefaultAttribs[name]
    if a then set_attribs(obj, a) end
    set_attribs(obj, attrs)
    return obj
end

function set_floor(name, x, y, attrs)
    local obj = make_object(name, attrs)
    enigma.SetFloor(x,y,obj)
    return obj
end
function set_item(name, x, y, attrs)
    local obj = make_object(name, attrs)
    enigma.SetItem(x,y,obj)
    return obj
end
function set_stone(name, x,y, attrs)
    local obj=make_object(name, attrs)
    enigma.SetStone(x,y,obj)
    return obj
end

kill_stone = enigma.KillStone
kill_item = enigma.KillItem

function SendMessage (target, msg, arg)
    if type(target) == "string" then
        enigma.SendMessage (enigma.GetNamedObject (target), msg, arg)
    else
        enigma.SendMessage (target, msg, arg)
    end
end

function SetAttribs (target, attribs)
    if type (target) == "string" then
        set_attribs(enigma.GetNamedObject(target), attribs)
    else
        set_attribs(target, attribs)
    end
end


function AddRubberBand(obj1,obj2, strength,length, minlen)
    enigma.AddRubberBand(obj1,obj2, strength or 10, length or 2, minlen or 0)
end

function fill_floor(name, x0,y0, w,h)
    if x0 == nil then x0 = 0 end
    if y0 == nil then y0 = 0 end
    if w  == nil then w  = level_width end
    if h  == nil then h  = level_height end
    for y=y0,y0+h-1 do
	for x=x0,x0+w-1 do
            set_floor(name, x, y) 
        end
    end
end

function fill_items(name, x0,y0,w,h)
    for y=y0,y0+h-1 do
	for x=x0,x0+w-1 do set_item(name, x, y) end
    end
end

function fill_stones(name, x0,y0, w,h)
    for y=y0,y0+h-1 do
        for x=x0,x0+w-1 do set_stone(name, x, y) end
    end
end

function draw_floor(name, xy0, xystep, n, attrs)
    local x,y = xy0[1],xy0[2]
    for i=1,n do
	set_floor(name, x, y, attrs)
	x = x+xystep[1]
	y = y+xystep[2]
    end
end

function draw_checkerboard_floor (name1, name2, x, y, w, h, attrs)
    for i=1,h do
        for j=1,w do
             if mod(i,2) == mod(j,2) then
                 set_floor (name1, x+j-1, y+i-1, attrs )
             else
                 set_floor (name2, x+j-1, y+i-1, attrs )
            end
        end
    end
end

function draw_items(name, xy0, xystep, n, attrs)
    local x,y = xy0[1],xy0[2]
    for i=1,n do
	set_item(name, x, y, attrs)
	x = x+xystep[1]
	y = y+xystep[2]
    end
end

function draw_stones(name, xy0, xystep, n, attrs)
    local x,y = xy0[1],xy0[2]
    for i=1,n do
	set_stone(name, x, y, attrs)
	x = x+xystep[1]
	y = y+xystep[2]
    end
end

function draw_border(stonename, x0, y0, w, h)
    if x0 == nil then x0 = 0 end
    if y0 == nil then y0 = 0 end
    if w  == nil then w  = level_width end
    if h  == nil then h  = level_height end
    draw_stones(stonename, {x0,y0}, {1,0}, w)
    draw_stones(stonename, {x0,y0+h-1},{1,0}, w)
    draw_stones(stonename, {x0,y0}, {0,1}, h)
    draw_stones(stonename, {x0+w-1,y0},{0,1}, h)
end

function set_stones(name, poslist, attrs)
    for i,xy in pairs(poslist) do
	set_stone(name, xy[1], xy[2], attrs)
    end
end

function SetActor(name, x, y, attrs)
    local obj=make_object(name, attrs)
    enigma.SetActor(x, y,obj)
    return obj
end

---------------------------------------------
-- Creation of particular kinds of objects --
---------------------------------------------
function fakeoxyd(x,y) 
    return set_stone("st-fakeoxyd",x,y) 
end


-- Create an oxyd stone with the current default flavor.
function oxyd(x, y, flavor, color)
    local f = flavor or oxyd_default_flavor
    local c = color or oxyd_current_color
    local a = {flavor=f, color=""..c}
    local obj = set_stone("st-oxyd", x, y, a)

    -- if "color" argument not provided, use next available color
    if not color then
        oxyd_count = oxyd_count +1

        if oxyd_count == 2 then
            oxyd_count = 0
            oxyd_current_color = oxyd_current_color + 1
        end
    end

    return obj
end

-- Shuffle the colors of all oxyd stones in the current landscape.
function oxyd_shuffle()
    enigma.SendMessage(enigma.GetObjectTemplate("st-oxyd"), "shuffle", nil)
end

-- Close all oxyd stones in the current landscape
function oxyd_closeall()
    enigma.SendMessage(enigma.GetObjectTemplate("st-oxyd"), "closeall", nil)
end

function oneway(x,y,orient)
    return set_stone("st-oneway",x,y,{orientation=orient})
end
function laser(x,y,is_on,dir)
    return set_stone("st-laser",x,y,{on=is_on, dir=dir})
end

function mirrorp(x,y,movable, transp,orient)
    return set_stone("st-pmirror", x, y, {movable=movable, transparent=transp,
                         orientation=orient})
end
function mirror3(x,y,movable, transp, orient)
    return set_stone("st-3mirror", x, y, {movable=movable, transparent=transp,
                         orientation = orient})
end

PUZ_0000=1                      -- hollow
PUZ_0001=2                      -- w
PUZ_0010=3                      -- s
PUZ_0011=4                      -- sw
PUZ_0100=5                      -- e
PUZ_0101=6                      -- ew
PUZ_0110=7                      -- es
PUZ_0111=8                      -- esw
PUZ_1000=9                      -- n
PUZ_1001=10                     -- nw
PUZ_1010=11                     -- ns
PUZ_1011=12                     -- nsw
PUZ_1100=13                     -- ne
PUZ_1101=14                     -- new
PUZ_1110=15                     -- nes
PUZ_1111=16                     -- nesw

-- functions using puzzle-style tiles:

function puzzle(x, y, conn)
    return set_stone("st-puzzle", x,y, {connections=conn})
end
function puzzle2(x, y, conn)
    return set_stone("st-puzzle", x,y, {connections=conn,oxyd=1})
end
function bigbrick(x, y, conn)
   return set_stone("st-bigbrick-n",x,y,{connections=conn})
end

--

function switch(x,y,target,action)
    return set_stone("st-switch", x,y, {target=target, action=action})
end

function abyss(x,y) set_floor("fl-abyss",x,y) end




-----------
-- ITEMS --
-----------
function hollow(x,y) set_item("it-hollow", x,y) end
function Document(x,y,t) set_item("it-document", x, y, {text=t}) end
function hammer(x,y) set_item("it-hammer",x,y) end
function dynamite(x,y) set_item("it-dynamite",x,y) end
function bomb(x,y) set_item("it-blackbomb",x,y) end
function keya(x,y) set_item("it-key", x,y, {keycode=1.0}) end
function keyb(x,y) set_item("it-key", x,y, {keycode=2.0}) end
function keyc(x,y) set_item("it-key", x,y, {keycode=3.0}) end

function shogundot1(x,y,attrs) set_item("it-shogun-s", x, y, attrs) end
function shogundot2(x,y,attrs) set_item("it-shogun-m", x, y, attrs) end
function shogundot3(x,y,attrs) set_item("it-shogun-l", x, y, attrs) end

function Wormhole(x,y,targetx, targety, attribs)
    local attrs = attribs or {}
    attrs.targetx = targetx
    attrs.targety = targety

    set_item("it-wormhole", x,y, attrs)
end

function Doorh(x,y,attrs)
    local attrs = attrs or {}
    attrs.type="h"
    set_stone("st-door",x,y,attrs)
end
function Doorv(x,y,attrs)
    local attrs = attrs or {}
    attrs.type="v"
    set_stone("st-door",x,y,attrs)
end

EAST = enigma.EAST
NORTH = enigma.NORTH
SOUTH = enigma.SOUTH
WEST = enigma.WEST

---------------
-- GRADIENTS --
---------------

SLOPE_S         = 1
SLOPE_N         = 2
SLOPE_E         = 3
SLOPE_W         = 4
SLOPE_LARGE_SE  = 5
SLOPE_LARGE_SW  = 6
SLOPE_LARGE_NE  = 7
SLOPE_LARGE_NW  = 8
SLOPE_SMALL_SE  = 9
SLOPE_SMALL_NE  = 10
SLOPE_SMALL_SW  = 11
SLOPE_SMALL_NW  = 12
SLOPE_S_FORCE_W = 13
SLOPE_N_FORCE_W = 14
SLOPE_S_FORCE_E = 15
SLOPE_N_FORCE_E = 16
SLOPE_E_FORCE_N = 17
SLOPE_W_FORCE_N = 18
SLOPE_E_FORCE_S = 19
SLOPE_W_FORCE_S = 20
FLAT_FORCE_S    = 21
FLAT_FORCE_N    = 22
FLAT_FORCE_E    = 23
FLAT_FORCE_W    = 24

function Gradient( x, y, type )
   if (type==nil) then error("Illegal gradient type"); end
   if (type>=1 and type<=24) then
      set_floor( "fl-gradient", x, y, {type=type} )
   else
      error("Unknown gradient type '"..type.."'");
   end
end


Signal = enigma.AddSignal
SetItem = set_item

----------------------------------------------------
-- Define lowercase aliases for various functions --
----------------------------------------------------

create_world = CreateWorld
get_attrib = GetAttrib
get_kind = GetKind
set_attrib = SetAttrib
make_object = MakeObject
set_actor = SetActor

document = Document
doorh    = Doorh
doorv    = Doorv
gradient = Gradient
wormhole = Wormhole

