------------------------------------------------------------------------
-- Copyright (C) 2002,2003 Daniel Heck
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
-- $Id: models.lua,v 1.11 2004/05/27 20:30:44 dheck Exp $
------------------------------------------------------------------------

-- This file contains common routines for defining new Enigma models.
-- It is used by all other model*.lua files.

---------------
-- FUNCTIONS --
---------------
function Progress(percent, text)
    local fontname = "levelmenu"
    local scr      = video.GetScreen();
    local d = scr:get_surface()

    local background = enigma.GetImage ("menu_bg", ".jpg")
    local logo = enigma.GetImage("enigma_logo3")
    local x = (d:width()  - logo:width())/2
    local y = (d:height() - logo:height())/2
    local gs = ecl.GS:new(d:size())
    local font2 = enigma.GetFont("menufontsel")

    d:blit(gs, 0, 0, background)
    d:blit(gs, x , y-logo:height(), logo)

    if text then
        font2:render(d, (d:width() - font2:get_width(text))/2, y, text)
    end
--     font:render(d, x, y-10, strrep(".", 50))
--     font2:render(d, x, y-10, strrep(".", percent/2))
    scr:update_all();
    scr:flush_updates();
    gs:delete()
end

modeltag = 0
function unique_modelname()
    modeltag = modeltag + 1
    return "xmodel" .. modeltag
end

-- Define a new image model.  'name' is the name of the model being created
-- and 'opt' contains optional information about the image file.  The following
-- fields can be set in 'opt':
--
--     * filename
--     * xoff,yoff (hotspot coordinates inside the image)
--
-- Suitable default values are used for all options
function def_image(name, opt)
    opt = opt or {}
    name = name or unique_modelname()
    local fname = opt.filename or name
    local err = display.DefineImage(name, fname, opt.xoff or 0, opt.yoff or 0, opt.padding or 0)
    if err ~= 0 then
        error ("Could not define model "..name..": error loading "..fname)
    end
    return name
end

-- Define multiple image models at once.  Use the same options for all
-- of them.
function def_images(names, opt)
    opt = opt or {}
    for i,name in pairs(names) do
	def_image(name,opt)
    end
end

-- Define many image models from one single big image file.

function def_subimages(name, options)
    local opts = options or { }
    local w = opts.w or 1
    local h = opts.h or 1
    local imgw = opts.imgw or TileSize
    local imgh = opts.imgh or TileSize
    local xoff = opts.xoff or 0
    local yoff = opts.yoff or 0
    local modelname = opts.modelname or name
    local padding = options.padding or 0

    local imagelist={}
    local cnt = 1
    local r=ecl.Rect:new(0,0,0,0)
    for x=1,w do
        for y=1,h do
            r.x,r.y,r.w,r.h = imgw*(x-1),imgh*(y-1),imgw,imgh
            r.x = r.x + padding
            r.y = r.y + padding
            r.w = r.w - 2*padding
            r.h = r.h - 2*padding
            tinsert(imagelist, modelname..cnt)
            err = display.DefineSubImage(modelname..cnt, name, xoff+padding,yoff+padding, r)
            if err ~= 0 then
                error ("Could not define "..modelname..cnt..": error loading "..name)
            end
            cnt = cnt+1
        end
    end
    r:delete()
    return imagelist
end

function map_tiles (imginfo, func)
    local w = imginfo.w or 1
    local h = imginfo.h or 1
    local tilew = imginfo.tilew or TileSize
    local tileh = imginfo.tileh or TileSize
    local r=ecl.Rect:new(0,0,0,0)
    local n=1
    for y=0,h-1 do
        for x=0,w-1 do
            r.x,r.y = x*tilew,y*tileh
            r.w,r.h = tilew, tileh
            func(n, r)
            n=n+1
        end
    end
    r:delete()
end


function DefineTile (imagename, modelname, x, y)
    local image = GetSurface (imagename)
    local r=ecl.Rect:new(x * TileSize, y*TileSize, TileSize, TileSize)
    local subsurface = CropSurface (image, r)
    DefineImageModel (modelname, subsurface)
    r:delete()
end


-- Generate multiple image models by tiling a big image into many
-- smaller subimages.  The parameters are currently hardcoded, see
-- "items.png" for an example image.
function DefineTiles(imagename, modelnames)
    local xoff = 0
    local yoff = 0
    local tilew = TileSize
    local tileh = TileSize

    local image = GetSurface (imagename)
    local imgw = image:width()
    local r=ecl.Rect:new(0,0,TileSize,TileSize)
    for i,mname in pairs(modelnames) do
        r.x,r.y,r.w,r.h = xoff,yoff,tilew,tileh
--        local subsurface = CropSurface (image, r)
--        DefineImageModel (mname, subsurface)
        DefineTile (imagename, mname, xoff/TileSize, yoff/TileSize)
        xoff = xoff + tilew
        if xoff >= imgw then
            xoff = 0
            yoff = yoff + tileh
        end
    end
    r:delete()
end


function def_stone(name, shmodel, opt)
    opt = opt or {}
    shmodel = shmodel or "sh-solid"
    opt.filename = opt.filename or name
    display.DefineShadedModel(name, def_image(nil, opt), shmodel)
end

function def_stone2(model, shmodel)
    shmodel = shmodel or "sh-solid"
    display.DefineShadedModel (model, model.."#", shmodel)
end

function def_stones(names)
    for i,name in names do def_stone(name) end
end
function def_roundstones(names)
    for i,name in names do def_stone(name, "sh-round") end
end

-- Define a shaded model named `name' with texture model `fg' and
-- shadow model `bg'.  Both are real models (not names of image files), so
-- you can combine animated images with shadows etc.
function def_shmodel(name, fg, bg)
    display.DefineShadedModel(name, fg, bg)
end

function def_overlay(name, imglist)
    display.DefineOverlayImage(name, getn(imglist), imglist)
end

function def_solidstone(name, front)
    def_shmodel(name, front, "sh-solid")
end
function def_roundstone(name, front)
    def_shmodel(name, front, "sh-round")
end

function def_floatingstone(name, front)
    def_shmodel(name, front, "sh-floating")
end

function def_alias(name, othername)
    display.DefineAlias(name,othername)
end

----------------
-- ANIMATIONS --
----------------

-- Generate a list of frame names by appending increasing numerical
-- prefixes to a base name.  For example, 'framenames("hello", 1,2)'
-- yields {"hello_0001", "hello_0002"}.
-- [Filenames like this are created by Gimp's "Video/Split Image.."
-- tool.]

function framenames(prefix, first, last)
    local fn = {}
    for i=first,last do
	tinsert(fn, prefix .. format("_%04d", i))
    end
    return fn
end

-- Build a list of frames from a list of model names and a constant
-- duration.
function buildframes(names, msec)
    local a={}
    for i,n in pairs(names) do a[i] = {n, msec} end
    return a
end

-- Build a list of frames from (1) a list of names and (2) a list of
-- frame durations.  These two list are assumed to have the same
-- number of entries, or, more generally, to have the same index set.
function composeframes(namelist, mseclist)
    local a={}
    for i=1,getn(namelist) do a[i] = {namelist[i], mseclist[i]} end
    return a
end


-- Given a list of frames, this function generates a new framelist
-- with for a ping-pong style animation.  (For example, an "abcd"
-- style animation would result in an "abcddcba"-style one.)

function pingpong(framelist)
    local a=framelist
    local n=getn(framelist)
    for i = 0,n-1 do
	a[n+i+1] = framelist[n-i]
    end
    return a
end

function repeat_frames(framelist, blocksize, cnt)
    local a={}
    for i=1,getn(framelist),blocksize do
	for j=1,cnt do
	    for k=i,i+blocksize-1 do
		tinsert(a,framelist[k])
	    end
	end
    end
    return a
end

function repeatanim(framelist, cnt)
    return repeat_frames(framelist, getn(framelist), cnt or 2)
end

function reverseframes(framelist)
    local a={}
    for i=getn(framelist),1,-1 do
	tinsert(a, framelist[i])
    end
    return a
end

-- Define an animation from a list of images.  `frames' is a
-- framelist, as built with `framenames()', but the model names in
-- this list are interpreted as image filenames.

function def_anim_images(name, frames, opt)
    opt = opt or {}
    local loopbool = opt.loop and (opt.loop == 1 or opt.loop == true) or false
    display.DefineAnim(name, loopbool)
    for i=1,getn(frames) do
	local frame=frames[i]
	opt.filename = frame[1]
	local immodel = def_image(nil, opt)
	display.AddFrame(name, immodel, frame[2])
    end
end


-- Define an animation from a list of models.

function def_anim(name, frames, loop)
    local loopbool = loop and (loop == 1 or loop == true) or false
    display.DefineAnim(name,loopbool)
    for i=1,getn(frames) do
	local frame = frames[i]
	display.AddFrame(name, frame[1], frame[2])
    end
end

function NewAnim(name,opts) -- name, img, h,w,speed,pingpong, loop)
    local imagefile = opts.img or name
    local h=opts.h or 1
    local w=opts.w or 1
    local speed = opts.speed or 100
    local loop=opts.loop or 0

    local frames = def_subimages(imagefile, {["h"]=h, ["w"]=w})
    if opts.pingpong then frames=pingpong(frames) end
    def_anim(name, buildframes(frames, speed), loop)
end
