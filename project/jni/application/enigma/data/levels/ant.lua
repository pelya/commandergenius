------------------------------------------------------------------------
-- Copyright (C) 2002-2003 Petr Machata
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
-- $Id: ant.lua,v 1.20 2004/02/10 21:47:44 ant_39 Exp $
------------------------------------------------------------------------
--
-- This module contains All Neccessary Tools for designing enigma maps.
-- There is a documentation available, please read it if you want
-- to find out how to use this (see <enigma-dir>/doc/ant_lua.txt),
-- or look at how miscelaneous ant??.lua maps are done.
-- Any questions, bug reports or anything related to this module please send to:
--   my e-mail:              ant_39 at centrum.cz
--   enigma-devel ML:  enigma-devel at nongnu.org
--
-- revision history:
-- 2003-01-07 -- special floor types and train support
-- 2003-01-11 -- multiples support (groups of doors, bolders etc.)
-- 2003-01-14 -- bugfixes and error reporting, some new map-creating funcs
-- 2003-02-09 --
--   many changes. Fixes of rubberband functions, fixes in multiples support, some interfaces changed
--   These changes require also fixes in several levels, but it's more logic this way. Besides this, it's
--   finally possible to call functions from init.lua or any functions declared like func(x, y, args),
--   for example oxyd(), fakeoxyd(), laser(), ...  and others.
--   While I was rewriting, I finally implemented multichar maps, which I planned since the beginning...
-- 2003-02-11 -- fixes in filling functions
-- 2003-02-12 -- cell() now returns last created object; functional-drawing functions accept table; and fixes
-- 2003-02-20 --
--   major rewrite of cell() function. This require also changes in *all* (~30) levels so far
--   I'll do it now, until there is just a few things to change... Later it could be too late
--   and these changes are really necessary
--   These changes include rewrites in parent function handling, so that it now supports a kind
--   of curried functions from haskell. Also you can use the coordlists instead of simple coords and
--   place several elements at once. And so on...
--   Big changes in interfacing many parent functions, completely rewritten trains and puzzle generators,
--   changes in multiples (there are more kinds, and interfacing changed).
--   A bug in multichar cell key fixed
-- 2003-02-25 --
--   add_multi* now uses tinsert/tremove, functions that work with multiples adapted where necessary
--   some comments fixed, few minor fixes across the source. warning().
-- 2003-03-08 --
--   support for parents that do not accept coordinates, default cell key meanings, updates in default
--   parent handling, debug mode
-- 2003-03-13 -- several cosmetic changes plus add_multitag, spread_tag, wormholes generator and slope generator
-- 2003-03-21 -- warning may raise error if warning_raises_error is set to 1 (ralf)
-- 2003-03-30 --
--   some comments fixed, be_pedantic() added, add_multitag fix, draw_map and get_map_size
--   were broken up to several parts so that particular tasks may be solved as designer wishes.
--   cell0.tag removed - I'm not sure if it was ever used at all...
--   some improvements in handling default cell key meanings at multichar maps
-- 2003-04-12 -- interface of add_multiobject changed to match interface of other multiples
-- 2003-04-25 -- boolean tables
-- 2003-06-19 -- render_puzzles accepts a 'kind' argument. Thanks to ralf!
-- 2003-10-09 -- oxyd_custom, oxyd_col and oxyd_fla added.
-- 2004-02-09 -- add_multitag removed, many cosmetic changes across the code


-- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
-- MISCELANEOUS - -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
-- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
-- for warning messages
function warning(text)
   if (warning_raises_error) then
      error(" [ant.lua]: "..text)
   else
      print("warning: [ant.lua]: "..text)
   end
end

-- turn warning messages to errors
-- usage: be_pedantic() -> turn on raising errors instead of warnings
function be_pedantic(mode)
   warning_raises_error = ((mode==nil)or(mode~=0))
end

-- for debug messages
function debug(text)
   if (DEBUG_MODE) then
      print("debug: [ant.lua]: "..text)
   end
end

-- debug mode
-- in debug mode, ant.lua produces messages that inform you about the execution
DEBUG_MODE = nil
function debug_mode()
   DEBUG_MODE = 1
   debug("debug_mode: debug mode turned on")
end

-- turn off debug mode
function debug_mode_off()
   debug("debug_mode_off: turning debug mode off")
   DEBUG_MODE = nil
end

-- x,y should be numbers, although chars and strings are generally also the right choice
function getkey(...)
   local key=""
   for i = 1,getn(arg) do
      if (i > 1) then key = key.."\0" end
      key = key..arg[i]
   end
   return key
end

-- todo: deep clone - clone also nested tables
function clone_table(tab)
   local ntab = {}

   for key,val in pairs(tab) do
      ntab[key] = val
   end

   return ntab;
end

-- each ant.lua function, that accepts coordinates, may be
-- called with negative coordinates as well. That in fact
-- means to place the object relatively to lower right
-- corner.
--  location [-1,-1] means [levelw, levelh]
--  size [0,0] means [levelw, levelh]
function transform_coords(x,y,w,h)
   local x, y = (x or 0), (y or 0)
   local w, h = (w or 0), (h or 0)
   if (x <  0) then x = level_width  + x - 1 end
   if (y <  0) then y = level_height + y - 1 end
   if (w <= 0) then w = level_width  + w end
   if (h <= 0) then h = level_height + h end
   return x,y,w,h
end

-- this couple of functions takes care of converting common tile coordinates to real coordinates of placed
-- actor. It depends on the given actor_mode, which is being declared like this:
-- cells["O"]=cell{parent=cells["+"], actor={face="ac-blackball", attr={player=0}, mode=ACTOR_MODE}}
--
-- the meaning of actor mode is as follows:
-- value || x | y || meaning
-- ------++---+---++--------
--  0    || 0 | 0 || actor is placed to left top corner of tile
--  1    || 1 | 0 || actor is centered horizontally on tile
--  2    || 0 | 1 || actor is centered vertically on tile
--  3    || 1 | 1 || actor is centered in tile
--
function get_actor_x(x, actor_mode)
   if ((actor_mode == 0) or (actor_mode == 2)) then
      return x
   else
      return x + 0.5
   end
end

function get_actor_y(y, actor_mode)
   if ((actor_mode == 0) or (actor_mode == 1)) then
      return y
   else
      return y + 0.5
   end
end




-- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
-- VISUAL MAP CREATION  -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
-- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --

-- width of key in map definition
-- CELL_KEY_WIDTH means how many characters occupy single map cell. In several maps I strongly
-- wanted this functions (cannonball for example), and after all here it is. Usable in maps with
-- wide variety of different surfaces.
CELL_KEY_WIDTH = 1
function set_cell_key_width(w)
   if ( (type(w) == "number") and (w>0) and (floor(w) == w) ) then
      CELL_KEY_WIDTH = w
      debug("set_cell_key_width: CELL_KEY_WIDTH is: "..w)
      return w
   else
      warning("set_cell_key_width: CELL_KEY_WIDTH has to be positive whole number")
      return -1
   end
end

-- get a table with cell functions
-- with no argument given, global cells is looked for.
-- if none is present, warning occurs.
function get_cellfuncs(cellfuncs)
   local funcs = cellfuncs or cells;  -- hope for a global cells

   if (not(funcs)) then
      warning("get_cellfuncs: no cellfuncs declared and global variable 'cells' is absent.")
      funcs = {};
   else
      debug("get_cellfuncs: ok, found cellfuncs table.")
   end

   return funcs;
end

-- this function tries to guess the width of key.
-- It does so by looking for the most common width of keys in cells[key]
function guess_cell_key_width(cellfuncs)
   local funcs = get_cellfuncs(cellfuncs)
   local freq_table = {}

   for key,_ in pairs(funcs) do
      local ktype = type(key)
      if (ktype == "string") then
	 local n = strlen(key)
	 freq_table[n] = (freq_table[n] or 0)+1
      end
   end

   local maxw,maxfreq = 0,0
   for w,freq in pairs(freq_table) do
      if (freq > maxfreq) then
	 maxw = w
	 maxfreq = freq
      end
   end

   debug("guess_cell_key_width: CELL_KEY_WIDTH guessed "..maxw)
   return set_cell_key_width(maxw)
end

-- default cell parent is called before any of common cell parents
-- default parent is a table, where each element may be
--  + a function
--  + a string, in which case this is a key in cellfunc table
-- also set_default_parent can be called with a single function
-- or string argument. The table will be built from this.
DEFAULT_CELL_PARENT = nil
function set_default_parent(func)
   if (not(func)) then
      debug("set_default_parent: default parent turned off")
      DEFAULT_CELL_PARENT = nil
      return
   end

   if ((type(func) == "function") or (type(func) == "string")) then
      func = {func}
   end

   if (type(func) == "table") then
      for key,f in pairs(func) do
	 local ftype = type(f)
	 if ((ftype ~= "function") and (ftype ~= "string")) then
	    warning("set_default_parent: element "..key.." of DEFAULT_CELL_PARENT\n"..
		    "has to be function or string, not "..ftype.."!")
	    return
	 end
      end
      DEFAULT_CELL_PARENT = func
   else
      warning("set_default_parent: default parent has to be\n"..
	      "a function, table of functions or string key")
   end
end

-- each item:
--  + face - name of stone/floor/item object
--  + attr - set of attributes of given object
function cell_item(it)
   local tit = type(it)
   if (tit == "string") then
      it = {it}
   elseif (tit == "table") then
      -- this is O.K.
   else
      warning("cell_item: cell item may be string or table, not "..tit.."!")
   end

   local n_it = {}
   n_it.face = (it.face or it[1] or "")
   n_it.attr = (it.attr or it[2] or {})
   n_it.mode = (it.mode or it[3] or 3)

   return n_it
end

-- each cell
--  + stone - if present, the stone shall be added to defined position
--  + floor - the same, but for floor
--  + item  - the same, but for item
--  + actor - the same, but for actor
--    ++ mode - see get_actor_x() and _y() functions
--  + mode - parent arglist handling mode. Defaults to 0 and it tells us where to put (x,y) couple in arglist
--    ++ mode = -1 => do not place the (x,y) couple to arglist
--    ++ mode = +0 => place it before parent arguments

--  ATTENTION: there *has* to be a coordlist even in case of pmode=-1, or NO arguments may be passed:
--    -> func()
--    -> func(x,y)
--    -> func(x,y, arg1, arg2, ...)
--    -> func(arg1, arg2) --> INCORRECT!!

function cell(structure)
   local cell0 = {}

   cell0.floor = cell_item(structure.floor or {})
   cell0.stone = cell_item(structure.stone or {})
   cell0.item =  cell_item(structure.item or {})
   cell0.actor = cell_item(structure.actor or {})

   -- get parent
   -- parent is declared in this form: parent={{func1, arg1, arg2,...},...}
   -- if it's not, convert to this form
   local parent = (structure.parent or structure[1] or {})

   if (type(parent) == "function") then
      parent = {parent}
   elseif (type(parent) == "table") then
      -- this is O.K.
   else
      warning("cell{}: parent has to be a function, a table of functions\n"..
	      "or a table of curry-functions, not "..type(parent))
      parent = {}
   end

   for a = 1,getn(parent) do
      if (type(parent[a]) == "function") then
	 parent[a] = {parent[a]}
      elseif (type(parent[a]) == "table") then
	 if (type(parent[a][1]) == "function") then
	    -- this is O.K. => a curry function call {func, arg1, arg2, ...}
	 else
	    warning("cell{}: parent element number "..a.." looks like a curry-function\n"..
		    "construction, but its first element isn't a function, it's "..type(parent[a])..".\n"..
		    "Using empty function instead.")
	    parent[a] = {function() end}
	 end
      else
	 warning("cell{}: parent element number "..a.." should be a function\n"..
		 "or a curry-function construction, not "..type(parent[a])..".\n"..
		 "Using empty function instead.")
	 parent[a] = {function() end}
      end
   end

   --+ this functions manages calling various parent functions. Most functions are just func(x,y) or (x,y,something)
   --  but it's also possible to call ({{x1,y1},{x2,y2}}, something1, something2)
   --  This enables compatibility with and wrapping of functions form init.lua.
   --+ At the same moment, it's possible to declare some parametters in cell functions, like this:
   --     cell1 = cell{parent={{func, par1}}}
   --  and then call  --cell1(par2)--  instead of  --func(par1, par2)--
   --  It may be used as a sort of curried functions from haskell, as far as my poor haskell knowledge says...
   return function(...)
	     local xylist = tremove(arg, 1) or {}
	     local ret = nil

	     -- first, get rid of cell(x,y) notation and convert it to cell({{x0,y0}}) notation
	     local xtp = type(xylist);
	     if (xtp == "number") then
		xylist = {{xylist, tremove(arg, 1)}}
	     elseif ((xtp == "table") and (type(xylist[1]) == "number")) then
		xylist = {xylist}
	     end

	     for idx = 1,getn(parent) do
		local tab0 = parent[idx]
		--+ tab0 is table. first item is function, the rest are the the function arguments
		--+ first extract function from tab0, then include arguments from function call to the
		--  tab, add a place for x,y coordinates and call it. Eventually tab will look like this:
		--    {x, y, pfpar1, pfpar2, ..., arg1, arg2, ...}
		local func = tab0[1]
		local pmode = tab0.mode or parent.mode or 0

		local tab;
		if (pmode == 0) then
		   tab = {0,0}
		else
		   tab = {}
		end

		for a = 2,getn(tab0) do
		   tinsert(tab, tab0[a])
		end

		for a = 1,getn(arg) do
		   tinsert(tab, arg[a])
		end

		-- and call a function
		if (pmode == 0) then
		   for i = 1,getn(xylist) do
		      tab[1],tab[2] = transform_coords(xylist[i][1], xylist[i][2])
		      ret = func(unpack(tab))
		   end
		else
		   ret = func(unpack(tab))
		end
	     end

	     --process common map elements
	     for i = 1,getn(xylist) do
		local x,y = transform_coords(xylist[i][1], xylist[i][2])
		if (structure.stone) then ret = set_stone(cell0.stone.face, x, y, cell0.stone.attr) end
		if (structure.floor) then ret = set_floor(cell0.floor.face, x, y, cell0.floor.attr) end
		if (structure.item ) then ret = set_item (cell0.item.face , x, y, cell0.item.attr ) end
		if (structure.actor) then
		   local ax, ay = get_actor_x(x, cell0.actor.mode), get_actor_y(y, cell0.actor.mode)
		   ret = set_actor(cell0.actor.face, ax, ay, cell0.actor.attr)
		end
	     end
	     return ret
	  end
end

-- height is just number of lines
function get_map_height(map)
   return getn(map)
end

-- width of particular map row
function map_row_length(y, map)
   return floor( strlen( map[y]) / CELL_KEY_WIDTH);
end

-- width - this is width of the widest row of map
function get_map_width(map)
   local mapw = 0

   for y = 1,get_map_height(map) do
      local w = map_row_length(y, map)
      if w > mapw then
	 mapw = w
      end
   end

   return mapw
end

-- and size combined
function get_map_size(map)
   return get_map_width(map), get_map_height(map)
end

-- common map
-- if a key has no meaning declared in given 'cellfuncs', we'll look here
-- whether there is a 'default' meaning
-- look at the bottom of ant.lua to see the map
DEFAULT_KEY_MEANING = {}
function map_cell_meaning(key, func)
   DEFAULT_KEY_MEANING[key] = func
end

-- use cells from default library in cellfuncs table
-- this function accepts a table of cellfuncs and arbitrary number of string keys as arguments
-- it loads the default keys to given cellfuncs table
function use_cells(funcs, ...)
   for a = 1,getn(arg) do
      funcs[arg[a]] = DEFAULT_KEY_MEANING[arg[a]]
   end
end

-- function picks a function from table of default cell keys
-- and results it
function default_cell(key)
   return DEFAULT_KEY_MEANING[key]
end

-- this function accepts a cell key and cellfuncs table
-- it looks into that table and if there is no func with given key, it returns default
-- value instead and inserts the value to given cellfuncs table
function get_cell_func(key, cellfuncs)
   if (cellfuncs[key]) then
      return cellfuncs[key]
   elseif (DEFAULT_KEY_MEANING[key]) then
      debug("get_cell_func: using DEFAULT_KEY_MEANING of '"..key.."'")
      cellfuncs[key] = DEFAULT_KEY_MEANING[key]
      return DEFAULT_KEY_MEANING[key]
   else
      -- for multichar maps, try by the first char
      if (strlen(key)>1) then
	 local ret = get_cell_func(strsub(key, 1, 1), cellfuncs)
	 if (ret) then
	    debug("get_cell_func: as a meaning of the key '"..key.."'")
	    cellfuncs[key] = ret;
	    return ret
	 end
      end

      warning("get_cell_func: no function declared for map key '"..key.."'")
      return nil
   end

   --notreached
   return nil
end




-- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
-- MAP RENDERING FUNCTIONS -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
-- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --

-- render a map square by key
-- rx,ry are x,y coordinates of given cell on map
-- key is a cell descriptor
-- func is a table of cellfuncs
function render_key(rx, ry, key, cellfuncs)
   local cellfuncs = cellfuncs or get_cellfuncs(cellfuncs);

   -- call default parents
   if (DEFAULT_CELL_PARENT) then
      for _,val in pairs(DEFAULT_CELL_PARENT) do
	 local dpfunc = 0
	 if (type(val) == "string") then
	    dpfunc = get_cell_func(val, cellfuncs)
	 else
	    dpfunc = val
	 end

	 if (type(dpfunc) == "function") then
	    dpfunc(rx, ry)
	 else
	    warning("render_key: unknown default parent type for the key: '"..val.."'")
	 end
      end
   end

   -- call common cell function
   local func = get_cell_func(key, cellfuncs)
   local ftype = type(func)
   if (ftype == "function") then
      func(rx, ry)
   elseif (ftype == "nil") then
      warning("render_key: function doesn't exist for map element '"..key.."'.")
   else
      warning("render_key: cell element '"..key.."' is not a function, it's "..ftype..".")
   end
end

-- this function returns a cell key by its x,y coordinates
function get_cell_by_xy(mx, my, map)
   return strsub(map[my], (mx - 1) * CELL_KEY_WIDTH + 1, mx * CELL_KEY_WIDTH)
end

-- function renders a given cell of map
-- rx0, ry0 are coordinates of left top square of map
-- mx, my are coordinates of map square to be rendered
function render_map_cell(rx0, ry0, mx, my, map, cellfuncs)
   if (not(map)) then
      warning("render_map_cell: no map given!")
      return
   end
   local cellfuncs = cellfuncs or get_cellfuncs(cellfuncs)

   local rx = mx + rx0 - 1
   local ry = my + ry0 - 1
   local key = get_cell_by_xy(mx, my, map)
   render_key(rx, ry, key, cellfuncs)
end

-- this draws the map to the position [rx0,ry0]
-- map is array of strings. Each string is one line of result map, each char is one map square.
-- you may omit cellfuncs, default global 'cells' is used instead
function draw_map_portion(rx0, ry0, mxy0, mxy1, map, cellfuncs)
   if (not(map)) then
      warning("draw_map: no map given!")
      return
   end
   local cellfuncs = cellfuncs or get_cellfuncs(cellfuncs);

   for my = mxy0[2],mxy1[2] do
      for mx = mxy0[1],mxy1[1] do
	 render_map_cell(rx0, ry0, mx, my, map, cellfuncs);
      end
   end
end

-- this will draw whole map
-- rx0, ry0 are coordinates of left top map corner
function draw_map(rx0, ry0, map, cellfuncs)
   local mapw, maph = get_map_size(map)
   draw_map_portion(rx0, ry0, {1,1}, {mapw, maph}, map, cellfuncs)
end

-- this just prepares the world, but no map is created
function prepare_world_by_map(map)
   local mapw, maph = get_map_size(map)
   local flavor = oxyd_default_flavor

   debug("creating world ["..mapw.."x"..maph.."]")
   create_world(mapw, maph)
   oxyd_default_flavor = flavor or oxyd_default_flavor or "b"
end

-- this prepares enigma world and draws given map
-- you may omit cellfuncs, default global 'cells' is used instead
function create_world_by_map(map, cellfuncs)
   prepare_world_by_map(map)
   draw_map(0, 0, map, cellfuncs)
end







-- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
-- FUNCTIOAL MAP DRAWING - -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
-- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
-- functions in this section are much like those defined in init.lua. Major difference
-- is that these functions accept a function as an argument. So, it's possible to call
-- given function with several coordinates at once. Just one single prerequisity -
-- the function must require at most two arguments, and they have to be coordinates
--   func(x,y, non_required_arguments)

-- fill each square of map with given function
function fill_world_func(fillfunc, x0, y0, w, h)
   if (type(fillfunc) ~= "table") then
      fillfunc = {fillfunc}
   end
   local x0,y0,w,h = transform_coords(x0, y0, w, h)

   for _,func in pairs(fillfunc) do
      for x=x0, x0+w-1 do
	 for y=y0, y0+h-1 do
	    func(x, y)
	 end
      end
   end
end

-- to draw border of map by given function
function draw_border_func(fillfunc, x0, y0, w, h)
   if (type(fillfunc) ~= "table") then
      fillfunc = {fillfunc}
   end
   local x0,y0,w,h = transform_coords(x0, y0, w, h)

   for _,func in pairs(fillfunc) do
      for x=x0,x0+w-1 do
	 func(x, y0)
	 func(x, y0+h-1)
      end

      for y=y0,y0+h-1 do
	 func(x0, y)
	 func(x0+w-1, y)
      end
   end
end

-- draws into corners of given boundary
function draw_func_corners(fillfunc, x0, y0, w, h)
   if (type(fillfunc) ~= "table") then
      fillfunc = {fillfunc}
   end
   local x0,y0,w,h = transform_coords(x0, y0, w, h)

   for _,func in pairs(fillfunc) do
      func(x0,y0)
      func(x0,y0+h-1)
      func(x0+w-1,y0)
      func(x0+w-1,y0+h-1)
   end
end

-- like set_stones, but calling a function
-- like: set_funcs(oxyd, {{1,2},{3,4},...})
function set_funcs(fillfunc, poslist)
   if (type(fillfunc) ~= "table") then
      fillfunc = {fillfunc}
   end

   for _,func in pairs(fillfunc) do
      for i = 1,getn(poslist) do
          local x,y,w,h = transform_coords(poslist[i][1], poslist[i][2])
          func(x, y)
      end
   end
end


-- draw functions into a row -- like draw_stones, draw_floor and others, but calls a func
-- generator of coordinates
function get_draw_coords(xylist, dxdylist, steps)
   if (type(xylist[1]) ~= "table") then
      xylist = {xylist}
   end
   if (type(dxdylist[1]) ~= "table") then
      dxdylist = {dxdylist}
   end
   local ret = {}

   -- convert
   for i = 1,getn(xylist) do
      local x0,y0 = transform_coords(xylist[i][1], xylist[i][2])

      tinsert(ret, {x0,y0})

      for j = 1,getn(dxdylist) do
	 local x,y   = x0,y0
	 local dx,dy = dxdylist[j][1], dxdylist[j][2]

	 for i = 2,steps do
	    x = x+dx
	    y = y+dy
	    tinsert(ret, {x,y})
	 end
      end
   end
   --
   return ret
end

-- drawing function
function draw_func(fillfunc, ...)
   if (type(fillfunc) ~= "table") then
      fillfunc = {fillfunc}
   end

   local xylist = get_draw_coords(unpack(arg))

   for _,func in pairs(fillfunc) do
      for i = 1,getn(xylist) do
	 local x,y = transform_coords(xylist[i][1], xylist[i][2])
	 func(x,y)
      end
   end
end


-- draw a function into a regular n-gon. Particularly usable for
-- setting up arrangements of actors, as they accept real values,
-- but if proper roundfunc is given, also stones etc. may be
-- placed with this function.

-- generator of coordinates
function get_ngon_coords(xylist, radiuslist, count, alpha0, roundfunc)
   if (type(xylist[1]) ~= "table") then
      xylist = {xylist}
   end
   if (type(radiuslist) ~= "table") then
      radiuslist = {radiuslist}
   end

   local astep = 360 / count
   local roundfunc = roundfunc or (function(x) return x end)

   local ret = {}

   for i = 1,getn(xylist) do
      local x0, y0 = transform_coords(xylist[i][1], xylist[i][2])

      for j = 1,getn(radiuslist) do
	 local radius = radiuslist[j]
	 local alpha  = alpha0 or 0

	 for _ = 1,count do
	    local x00 = roundfunc(x0 + radius * sin(alpha))
	    local y00 = roundfunc(y0 + radius * cos(alpha))
	    tinsert(ret, {x00,y00})
	    alpha = alpha + astep
	 end
      end
   end

   return ret
end

-- drawing function
function ngon_funcs(fillfunc, ...)
   if (type(fillfunc) == "function") then
      fillfunc = {fillfunc}
   end

   local xylist = get_ngon_coords(unpack(arg))

   for _,func in pairs(fillfunc) do
      for i = 1,getn(xylist) do
	 local x,y = transform_coords(xylist[i][1], xylist[i][2])
	 func(x,y)
      end
   end
end






-- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
-- SPECIAL FLOOR TYPES  -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
-- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
-- see ant.lua documentation for extensive howto
-- (this is yet to be done)

-- draws checker floor
-- sidex: horizontal size of checker square
-- sidey: vertical size of checker square
-- count: number of checker textures
-- items: table of checker textures {1,2,3,...}
-- x,y:   coordinates of checker field
function mcheckerfloor(x, y, sidex, sidey, offsetx, offsety, count, items)
   local x0 = floor( (x - offsetx + sidex) / sidex)
   local y0 = floor( (y - offsety + sidey) / sidey)
   local remainder = mod(x0+y0, count)
   items[remainder + 1](x,y)
end

-- the checkerfloor parent
-- items: checkerfloor textures. Functions generated by cell({structure})
-- eg:
--   cells["."]=cell{floor={face="fl-metal"}}
--   cells[","]=cell{floor={face="fl-normal"}}
--   cells[" "]=cell{parent={{checkerfloor,{cells[","], cells["."]}}}}
-- optionally, at the end of table there may be additional checkerboard generating variables:
--   side - set size of checkerboard square
--   sidex - only set horizontal size (defaults to 1)
--   sidey - only set vertical size (defaults to 1)
--     cells[" "]=cell{parent={{checkerfloor,{cells[","], cells["."]; side=2}}}}
function checkerfloor(x, y, items)
   local count = getn(items)
   local remainder = mod(x+y, count)
   local sidex, sidey = 1, 1
   local offsetx, offsety = 0, 0

   if (items.side ~= nil) then
      sidex = items.side
      sidey = sidex
   end

   if (items.sidex ~= nil) then
      sidex = items.sidex
   end

   if (items.sidey ~= nil) then
      sidey = items.sidey
   end

   if (items.offset ~= nil) then
      offsetx = items.offset
      offsety = offsetx
   end

   if (items.offsetx ~= nil) then
      offsetx = items.offsetx
   end

   if (items.offsety ~= nil) then
      offsety = items.offsety
   end

   mcheckerfloor(x, y, sidex, sidey, offsetx, offsety, getn(items), items)
end

-- the random floor parent
--  every item may be:
--   texture -- parent function, for example cell[] function
--   occurence -- occurence factor for previous texture. Defaults to 1.
function randomfloor(x, y, ...)
   local count = 0
   local total = 0 --total occurences

   local items = {}
   if (getn(arg) == 1) then
      if (type(arg[1]) == "table") then
	 items = arg[1]    -- table is passed
      else
	 items = {arg[1]}  -- single function passed (hope it's a function)... who could do it?
      end
   else
      items = arg          -- a list of functions and their rarities is not passed in table, but as arguments
   end

   local itemlist = {}

   for i = 1,getn(items) do
      local item  = items[i]
      local titem = type(item)
      if (titem == "function") then
	 count = count + 1
	 itemlist[count] = {}
	 itemlist[count].func = item
	 itemlist[count].occf = 1
	 total = total + 1
      elseif (titem == "number") then
	 itemlist[count].occf = item
	 total = total + item-1
      end
   end

   local rand = total*random()
   local ctr  = 0

   for i=1,count do
      local move = itemlist[i].occf
      if ((rand>=ctr) and (rand<ctr+move)) then
	 itemlist[i].func(x,y)
      end
      ctr = ctr + move
   end
end





-- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
-- OBJECT GROUPS [MULTIPLES]  -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
-- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
-- see ant.lua documentation for extensive howto

-- add a stone to a group
-- face: this is stone face
-- group: object group table
-- attribs: object attributes
function add_multistone(x, y, face, group, attribs)
   local attribs = attribs or {}
   attribs["name"] = attribs.name or (face..(getn(group)+1))
   tinsert(group, set_stone(face, x, y, attribs))
end

-- add a floor to a group
-- face: this is floor face
-- group: object group table
-- attribs: object attributes
function add_multifloor(x, y, face, group, attribs)
   local attribs = attribs or {}
   attribs["name"] = attribs.name or (face..(getn(group)+1))
   tinsert(group, set_floor(face, x, y, attribs))
end

-- add an item to a group
-- face: this is an item kind
-- group: object group table
-- attribs: object attributes
function add_multiitem(x, y, face, group, attribs)
   local attribs = attribs or {}
   attribs["name"] = attribs.name or (face..(getn(group)+1))
   tinsert(group, set_item(face, x, y, attribs))
end

-- add an actor to a group
-- face: this is floor face
-- group: object group table
-- attribs: object attributes
function add_multiactor(x, y, face, group, attribs, actor_mode)
   local attribs = attribs or {}
   local actor_mode = actor_mode or 3
   attribs["name"] = attribs.name or (face..(getn(group)+1))
   tinsert(group, set_actor(face, get_actor_x(x, actor_mode), get_actor_y(y, actor_mode), attribs))
end

-- for generic multiples, this only stores cell coordinates and tag information
-- this is usable for example to construct puzzles and trains, see below
function add_multicell(x, y, group, tag)
   local key = getkey(x,y)
   group[key] = {}
   group[key].x = x
   group[key].y = y
   group[key].tag = tag
end

-- another generic multiple
-- this stores the result of given function(x,y) to table
function add_multiobject(x, y, group, func)
   tinsert(group, func(x, y))
end



-- couple of functions to mark all cells in given area with
-- given tagnumber. Used for rendering gradients. Area must
-- be closed, otherwise stack overflow occurs.
-- usage:
--   slopes={}
--   pivots={}
--   cells["*"]=cell{{{add_multicell, slopes, 1}}}
--   cells["&"]=cell{{{add_multicell, pivots, slopes}}}
-- at the end of levelfile:
--   spread_tag(pivots)
--   render_slopes(slopes)
-- BUGS:
--  stack overflows, if:
--  + the shape is not closed
--  + area is too large (for example very long corridor...)
function spread_tag_depth(x0, y0, tab, tag)
   if tab[getkey(x0, y0)] then
      return
   end

   add_multicell(x0, y0, tab, tag)
   spread_tag_depth(x0,   y0-1, tab, tag)
   spread_tag_depth(x0+1, y0  , tab, tag)
   spread_tag_depth(x0,   y0+1, tab, tag)
   spread_tag_depth(x0-1, y0  , tab, tag)
end

function spread_tag(tab, tag)
   local tag = tag or 2
   for _,val in pairs(tab) do
      local x0 = val.x
      local y0 = val.y
      local tab0 = val.tag
      spread_tag_depth(x0, y0, tab0, tag)
   end
end

-- add the rubber band between all objects in group 1 and all objects in group 2
-- that is, each object from gr1 will be connected with each object from gr2
function add_rubber_bands(gr1, gr2, strength, length)
   for i = 1,getn(gr1) do
      for j = 1,getn(gr2) do
	 AddRubberBand(gr1[i], gr2[j], strength, length)
      end
   end
end

-- add pairs: 1st object from gr1 will be connected with 1st from gr2 and so on
-- requires same sized groups, of course...
function add_rubber_band_pairs(gr1, gr2, strength, length)
   for i = 1,getn(gr1) do
      AddRubberBand(gr1[i], gr2[i], strength, length)
   end
end

-- to rubber-band given objects [1,2,3,...,n] like that:
--  1=2=3=...=n=1
function rubber_band_circle(gr1, strength, length)
   local remember = nil
   local first = nil
   for a = 1,getn(gr1) do
      local obj1 = gr1[a]
      if (remember) then
	 AddRubberBand(obj1, remember, strength, length)
      else
	 first = obj1
      end

      remember = obj1
   end

   if (first) then
      AddRubberBand(first, remember, strength, length)
   end
end

-- send message to each object in group
function send_group_message(group, message, third)
   for i = 1,getn(group) do
       SendMessage(group[i], message, third)
   end
end

-- send a message to named object
function send_message_named(objname, message, third)
    SendMessage(objname, message, third)
end

-- set attribute to each of the objects in group
function set_group_attribs(group, attribs)
   for i = 1,getn(group) do
      set_attribs(group[i], attribs)
   end
end





-- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
-- RAILWAY GENERATOR -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
-- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
-- see ant.lua documentation for extensive howto

function new_rail(engines, path)
   local func0 = function()
		    for _,engine in pairs(engines) do
		       local ekey = getkey(engine.x, engine.y)

		       for _, dir in pairs({engine.dir,{1,0},{0,1},{-1,0},{0,-1}}) do
			  local x0 = engine.x + dir[1]
			  local y0 = engine.y + dir[2]
			  local key = getkey(x0, y0)

			  if ((path[key]) and (path[key].tag ~= path[ekey].tag)) then
			     path[key].tag = path[ekey].tag
			     engine.x, engine.y = x0, y0
			     engine.dir = dir -- remember direction
			     engine.tag(x0, y0)
			     break
			  end
		       end--for directions

		    end--for engines
		 end--function
   return func0
end




-- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
-- PUZZLE GENERATOR  -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
-- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
-- see ant.lua documentation for extensive howto
function render_puzzles(tab, kind, generatorfunc)
   for _,val in pairs(tab) do
      local kind = kind or puzzle
      local x,y = val.x, val.y

      local up   = 0;
      local down = 0;
      local left = 0;
      local right= 0;
      if (tab[getkey(x, y-1)] ~= nil) then up = 1 end
      if (tab[getkey(x, y+1)] ~= nil) then down = 1 end
      if (tab[getkey(x-1, y)] ~= nil) then left = 1 end
      if (tab[getkey(x+1, y)] ~= nil) then right = 1 end

      if (generatorfunc) then
	 generatorfunc(val);
      end

      if ((val.tag ~= 2) and (kind)) then
	 kind(x, y, getglobal("PUZ_"..up..right..down..left))
      end
   end
end




-- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
-- BLACK HOLES GENERATOR - -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
-- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
-- see ant.lua documentation for extensive howto

function render_wormholes(holes, targets, whole_attribs, actor_mode)
   local whole_attribs = whole_attribs or {}
   local actor_mode = actor_mode or 3

   for _,hval in pairs(holes) do
      local target = 0
      for _,tval in pairs(targets) do
	 if (tval.tag == hval.tag) then
	    target = tval;
	    break
	 end
      end

      if (target ~= 0) then
	 local targetx = get_actor_x(target.x, actor_mode)
	 local targety = get_actor_y(target.y, actor_mode)
	 wormhole(hval.x, hval.y, targetx, targety, whole_attribs)
      end
   end
end

function worm_hole_pair(cellfuncs, whole_cell, tgt_cell, whole_parent, tgt_parent, whole_grp, tgt_grp, tagnumber)
   cellfuncs[whole_cell] = cell{{whole_parent, {add_multicell, whole_grp, tagnumber}}}
   cellfuncs[tgt_cell]   = cell{{tgt_parent,   {add_multicell, tgt_grp,   tagnumber}}}
end






-- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
-- SLOPE GENERATOR - -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
-- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
-- see ant.lua documentation for extensive howto

__slopemap = {}
__slopemap["x0x11x1x"]=SLOPE_N
__slopemap["x1x01x1x"]=SLOPE_W
__slopemap["x1x10x1x"]=SLOPE_E
__slopemap["x1x11x0x"]=SLOPE_S
__slopemap["x1x00x0x"]=SLOPE_S
__slopemap["x0x10x0x"]=SLOPE_E
__slopemap["x0x01x0x"]=SLOPE_W
__slopemap["x0x00x1x"]=SLOPE_N

-- soft conditions
__slopemap["x11x1xxx"]=SLOPE_SMALL_SW
__slopemap["x1x1111x"]=SLOPE_LARGE_NE
__slopemap["xxx1x11x"]=SLOPE_SMALL_NE
__slopemap["x1111x1x"]=SLOPE_LARGE_SW
__slopemap["xxxx1x11"]=SLOPE_SMALL_NW
__slopemap["11x11x1x"]=SLOPE_LARGE_SE
__slopemap["11x1xxxx"]=SLOPE_SMALL_SE
__slopemap["x1x11x11"]=SLOPE_LARGE_NW

-- hard conditions -- these don't match always, but help to break ambiguous matches
__slopemap["x1101x0x"]=SLOPE_SMALL_SW
__slopemap["x101111x"]=SLOPE_LARGE_NE
__slopemap["x0x1011x"]=SLOPE_SMALL_NE
__slopemap["x111101x"]=SLOPE_LARGE_SW
__slopemap["x0x01x11"]=SLOPE_SMALL_NW
__slopemap["11x11x10"]=SLOPE_LARGE_SE
__slopemap["11x10x0x"]=SLOPE_SMALL_SE
__slopemap["01x11x11"]=SLOPE_LARGE_NW

--inverses
__grad_inverse={}
__grad_inverse[SLOPE_S]=SLOPE_N
__grad_inverse[SLOPE_N]=SLOPE_S
__grad_inverse[SLOPE_E]=SLOPE_W
__grad_inverse[SLOPE_W]=SLOPE_E
__grad_inverse[SLOPE_LARGE_SE]=SLOPE_SMALL_NW
__grad_inverse[SLOPE_LARGE_SW]=SLOPE_SMALL_NE
__grad_inverse[SLOPE_LARGE_NE]=SLOPE_SMALL_SW
__grad_inverse[SLOPE_LARGE_NW]=SLOPE_SMALL_SE
__grad_inverse[SLOPE_SMALL_SE]=SLOPE_LARGE_NW
__grad_inverse[SLOPE_SMALL_NE]=SLOPE_LARGE_SW
__grad_inverse[SLOPE_SMALL_SW]=SLOPE_LARGE_NE
__grad_inverse[SLOPE_SMALL_NW]=SLOPE_LARGE_SE

-- return correct gradient type based on neighbors
function map_slope(x, y, node)
   local map = __slopemap;
   local re  = ""..node[1]..node[2]..node[3]..node[4]..node[5]..node[6]..node[7]..node[8]
   --
   local longest = 0;
   local longkey = 0;
   local longval = 0;
   local warns = {}
   --
   for key,val in pairs(map) do
      local okay = 1
      local count = 0
      -- okay turns to 0, if any char, that is not 'x', doesn't match
      for i = 1,strlen(key) do
	 local char1 = strsub(key, i, i)
	 local char2 = strsub(re, i, i)
	 --
	 if ((char1 ~= 'x') and (char2 ~= 'x') and (char1 ~= char2)) then
	    okay = 0
	    break
	 else
	    if ((char1 ~= 'x') and (char2 ~= 'x')) then
	       count = count +1
	    end
	 end
      end
      --
      if (okay == 1) then
	 if (count > longest) then
	    longest = count;
	    longkey = key;
	    longval = val;
	    warns = {}
	 elseif (count == longest) then
	    tinsert(warns, "map_slope: ambiguous match: '"..longkey.."' vs. '"..key.."' for re '"..re.."'")
	 end
      end
   end
   --
   for i = 1,getn(warns) do
      debug(warns[i])
   end

   if (longest == 0) then
      warning("map_slope: no match for mask '"..re.."' at ["..x..","..y.."].")
   end

   return longval;
end

function render_slopes(tab, invert)
   for _,val in pairs(tab) do
      local x,y = val.x, val.y

      local node = {}
      if (tab[getkey(x-1,y-1)] ~= nil) then tinsert(node, 1) else  tinsert(node, 0) end
      if (tab[getkey(x  ,y-1)] ~= nil) then tinsert(node, 1) else  tinsert(node, 0) end
      if (tab[getkey(x+1,y-1)] ~= nil) then tinsert(node, 1) else  tinsert(node, 0) end
      if (tab[getkey(x-1,y  )] ~= nil) then tinsert(node, 1) else  tinsert(node, 0) end
      if (tab[getkey(x+1,y  )] ~= nil) then tinsert(node, 1) else  tinsert(node, 0) end
      if (tab[getkey(x-1,y+1)] ~= nil) then tinsert(node, 1) else  tinsert(node, 0) end
      if (tab[getkey(x  ,y+1)] ~= nil) then tinsert(node, 1) else  tinsert(node, 0) end
      if (tab[getkey(x+1,y+1)] ~= nil) then tinsert(node, 1) else  tinsert(node, 0) end

      if (val.tag ~= 2) then
	 if( (invert ~= nil) ~= (val.tag == -1)) then   -- this is !(invert xor (val.tag == -1))
	    gradient(x, y, __grad_inverse[map_slope(x, y, node)])
	 else
	    gradient(x, y, map_slope(x, y, node))
	 end
      end
   end
end





-- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
-- BOOLEAN TABLES -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
-- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
-- see ant.lua documentation for extensive howto

-- bool_and tests table for and: all elements have to be '1' to succeed
function bool_and(tab)
   return tab.count == tab.value
end


-- bool_or tests table for or: at least one element has to be '1' to succeed
function bool_or(tab)
   return tab.value > 0
end


-- bool_xor tests table for xor: odd number of elements has to be '1' to succeed
function bool_xor(tab)
   return mod(tab.value, 2) ~= 0
end


-- bool_table initialization
-- count_tot  is number of cells in table (that is the number of triggers)
-- count_init is number of cells to init (number of negset triggers)
-- test_func  is a function to get called each set and negset - a test function
-- true_func  is a function to be called if test passed
-- false_func is a function to be called if test failed
function bool_table(count_tot, count_init, test_func, true_func, false_func)
   local tab   = {};

   tab.test    = test_func  or function() return nil end;
   tab.ontrue    = true_func  or function() return nil end;
   tab.onfalse   = false_func or function() return nil end;
   tab.count   = count_tot  or 1;
   tab.value   = count_init or 0;
   tab.remember= -1;

   return tab;
end


-- bool val set
-- this gets called upon every trigger/switcher state change
-- it changes value of one element of given table
function bool_set(value, omit, tab)
   if (value == 0) then
      tab.value = tab.value -1;
   else
      tab.value = tab.value +1;
   end

   local res = tab.test(tab);
   if (tab.remember ~= res) then
      tab.remember = res;

      if (res) then
	 debug("bool_set: true")
	 tab.ontrue()
      else
	 debug("bool_set: false")
	 tab.onfalse()
      end
   end
end


-- bool val negative set
-- this is similar to bool_val, except that it sets '1' if triggered off and vice versa
function bool_negset(value, omit, tab)
   if (value == 0) then
      bool_set(1, nil, tab)
   else
      bool_set(0, nil, tab)
   end
end





-- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
-- WRAPPED init.lua FUNCTIONS -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
-- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --

-- This somehow generalizes oxyd() from init.lua. You can pick your
-- own color and flavor of oxyd stone. If you omit 'acolor', or pass a
-- '' (empty string) instead, default value (or last time used value)
-- is used instead. Omitting aflavor or passing empty string as
-- aflavor results in using default or last used flavor.

function oxyd_custom(x,y,aflavor,acolor)
   if ((not aflavor) or (aflavor=='')) then
      aflavor = oxyd_default_flavor
   end

   if ((not acolor) or (acolor=='')) then
      acolor = oxyd_current_color
   end

   oxyd_default_flavor = aflavor;
   oxyd_current_color = acolor;

   oxyd(x,y)
end


-- these functions override common init.lua functions, giving them the power
-- of ant.lua syntax -- that is, calling with list of coordinates,
-- using negative coordinates for positions relative to lower-right corner
-- and maybe also some others that I do not recall now :)
-- Note, that basic syntax remain intact, so at first glance no changes are visible!
-- Wrapped functions are a lot slower, empirically nearly 3.5 times. Not a great deal,
-- as LUA is really fast, and the same for enigma. Tested on rather slow 166MHz AMD,
-- 80 megs of memory, Win95 - you won't notice extra time spent by loading a level.

-- stones
oxyd =     cell{oxyd}
fakeoxyd = cell{fakeoxyd}
oneway =   cell{oneway}
laser =    cell{laser}
mirrorp =  cell{mirrorp}
mirror3 =  cell{mirror3}
puzzle =   cell{puzzle}
switch =   cell{switch}

-- floors
abyss =    cell{abyss}
hollow =   cell{hollow}
hill=      cell{item="it-hill"}

-- items
Document = cell{Document}
hammer =   cell{hammer}
dynamite = cell{dynamite}
bomb =     cell{bomb}
shogundot= cell{shogundot}
keya =     cell{keya}
keyb =     cell{keyb}
keyc =     cell{keyc}
shogundot1=cell{{{shogundot, 1}}}
shogundot2=cell{{{shogundot, 2}}}
shogundot3=cell{{{shogundot, 3}}}
Wormhole = cell{Wormhole}
doorh =    cell{doorh}
doorv =    cell{doorv}
gradient = cell{gradient}

-- lower case equivalents
document = Document
wormhole = Wormhole

-- 'andvanced' oxyd functions
oxyd_col = cell{{{oxyd_custom, ''}}}
oxyd_fla = cell{{{oxyd_custom}}}




-- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
-- MEANINGS FOR COMMON CELL KEYS -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
-- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
-- here are declarations for default cell bindings

-- bindings based on init.lua functions
map_cell_meaning(".", abyss)
map_cell_meaning("0", oxyd)

-- common constructions
map_cell_meaning("W", cell{stone="st-wood"})
map_cell_meaning("B", cell{stone="st-block"})
map_cell_meaning("D", cell{stone="st-death"})
map_cell_meaning("=", cell{stone="st-glass"})
map_cell_meaning("X", cell{stone="st-grate1"})

-- actors
map_cell_meaning("o", cell{actor={"ac-whiteball-small", {player=0, mouseforce=1}}})
map_cell_meaning("O", cell{actor={"ac-blackball", {mouseforce=1}}})

-- presets
function meditation_mode()
   map_cell_meaning("O", hollow)
end

function multiplayer_mode()
   map_cell_meaning("1", cell{item="it-yinyang", actor={"ac-blackball", {mouseforce=1}}})
   map_cell_meaning("2", cell{item="it-yinyang", actor={"ac-whiteball", {mouseforce=1}}})
end

-- level mood
function grass_mode()
   map_cell_meaning(" ", cell{floor="fl-leaves"})
   map_cell_meaning("#", cell{stone="st-rock1"})
end

function metal_mode()
   map_cell_meaning(" ", cell{floor="fl-metal"})
   map_cell_meaning("#", cell{stone="st-rock2"})
end

-- maybe in a future, there will be a possibility to integrate Nat's mazes into maps
function maze_mode()
   Require("levels/lib/natmaze.lua")
   --map_cell_meaning("!", )
end












