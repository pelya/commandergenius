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
-- $Id: models-2d.lua,v 1.124 2004/05/27 20:30:47 dheck Exp $
------------------------------------------------------------------------

-- This file defines the models used in Enigma.

dofile(FindDataFile("models.lua"))

def_image("dummy")
def_image("invisible")

TRUE, FALSE = 1, 0

function SpriteImages(spriteimg, n, offsetfactor, padding)
    local factor = offsetfactor or 0.5
    local offset = -SpriteSize * factor
    local pad = (padding or 0) * TileSize
    return def_subimages(spriteimg,
                         {h = n, imgw=SpriteSize, imgh=SpriteSize,
                             xoff = offset, yoff = offset, padding=pad})
end

function SpriteImage(spriteimg, offsetfactor, padding)
    local factor = offsetfactor or 0.5
    local offset = -SpriteSize * factor
    local padding = (padding or 0) * TileSize
    return def_image(spriteimg, {xoff = offset, yoff = offset, padding=padding})
end

function SpriteWithShadow()

end

function SpriteAnim(name, images, shadows, framelen, loop)
    local frames={}
    local nframes = getn(images)
    for i=1,getn(images) do
        def_shmodel(name..i, images[i], shadows[mod (i, getn(shadows))] )
        tinsert(frames, name..i)
    end
    def_anim (name, buildframes(frames, framelen), loop)
end

function Sprite(descr)
    local imgfile = descr.imgfile or descr.name
    local loop    = descr.loop or FALSE
    local img     = SpriteImages(imgfile, descr.nimages, 0.5, descr.padding)
    def_anim(descr.name, buildframes(img, descr.framelen), loop)
end

--------------------------------------------------------------------
--                           ACTOR MODELS                         --
--------------------------------------------------------------------
Progress(0, "Loading sprite models")

------------------
-- Spinning top --
------------------
do
    local img=SpriteImages("fg-top", 9, 0.5, 0.3)
    SpriteImage("sh-top", 0.4, 0.3)

    frames={}
    for i=1,9 do
        def_shmodel("ac-top"..i, img[i], "sh-top")
        tinsert(frames, "ac-top"..i)
    end
    def_anim("ac-top", buildframes(frames, 25), TRUE)
end

-----------
-- Rotor --
-----------
do
    local fg=SpriteImages("fg-rotor",9)
    local bg=SpriteImages("sh-rotor", 9, 0.4)

    frames={}
    for i=1,9 do
        def_shmodel("ac-rotor"..i, fg[i], bg[i])
        tinsert(frames, "ac-rotor"..i)
    end
    def_anim("ac-rotor", buildframes(frames, 30), TRUE)
end

-----------
-- Horse --
-----------
do
    SpriteImage ("fg-horse")
    SpriteImage ("sh-horse", 0.4)
    def_shmodel ("ac-horse", "fg-horse", "sh-horse")
end

----------------------
-- Small white ball --
----------------------
do
    local fg,img

    -- Normal
    SpriteImage ("sh-whiteball-small", 0.4, 0.3)
    SpriteImage ("fg-whiteball-small", 0.5, 0.3) 
    def_shmodel("ac-whiteball-small", "fg-whiteball-small", "sh-whiteball-small")
    def_alias ("ac-whiteball-small-shine", "ac-whiteball-small")

    -- Falling
    img=SpriteImages ("ac-whiteball-small-fall", 5)
    tinsert(img, "invisible")
    def_anim("ac-whiteball-small-fall", composeframes(img,{70,65,60,55,50,30}))
    def_alias("ac-whiteball-small-fallen", "invisible")

    -- Appearing / disappearing
    def_anim("ac-whiteball-small-appear", reverseframes(buildframes(img, 25)))
    def_anim("ac-whiteball-small-disappear", buildframes(img, 25))

    -- Shattering
    img=SpriteImages ("ac-whiteball-small-shatter", 5)
    def_anim("ac-whiteball-small-shatter", buildframes(img, 60))
    SpriteImage ("ac-whiteball-small-shattered")


    -- sinking
    def_alias ("ac-whiteball-small-sink0", "ac-whiteball-small-fall1")
    def_alias ("ac-whiteball-small-sink1", "ac-whiteball-small-fall2")
    def_alias ("ac-whiteball-small-sink2", "ac-whiteball-small-fall3")
    def_alias ("ac-whiteball-small-sink3", "ac-whiteball-small-fall4")
    def_alias ("ac-whiteball-small-sink4", "ac-whiteball-small-fall5")
    def_alias ("ac-whiteball-small-sink5", "ac-whiteball-small-fall5")
    def_alias ("ac-whiteball-small-sink6", "ac-whiteball-small-fall5")
    def_alias ("ac-whiteball-small-sunk", "invisible")
end

-- Jumping small balls --
do
    namelist = SpriteImages("ac-whiteball-small-jump", 4)
    shadows  = SpriteImages("sh-whiteball-small-jump", 4, 0.4)
    frames   = {}
    for i=1,4 do
        def_shmodel("sb-jump"..i, namelist[i], shadows[i])
        tinsert(frames, "sb-jump"..i)
    end
    def_anim("ac-whiteball-small-jump", pingpong(buildframes(frames, 70)))
end

def_alias("ac-killerball", "ac-whiteball-small")

-------------------------
-- Balls (both colors) --
-------------------------

-- Shattering
Sprite{name="ac-blackball-shatter", nimages=5, framelen=60}
Sprite{name="ac-whiteball-shatter", nimages=5, framelen=60}

-- Sinking
do
    local img, sh

    sh = SpriteImages("sh-blackball-sink", 7, 0.4)
    img = SpriteImages("fg-blackball-sink", 7)
    for i=1,getn(img) do
        def_shmodel("ac-blackball-sink"..(i-1), img[i], sh[i])
    end
    def_alias("ac-blackball-sunk", "invisible")

    img=SpriteImages("fg-whiteball-sink", 7)
    for i=1,getn(img) do
        def_shmodel("ac-whiteball-sink"..(i-1), img[i], sh[i])
    end
    def_alias("ac-whiteball-sunk", "invisible")
end


----------------
-- Black ball --
----------------
do
    local img,f,sh

    -- Normal
    sh = SpriteImage("sh-blackball", 0.4, 0.2)
    img = SpriteImages("fg-blackball", 2, 0.5, 0.25)
    def_shmodel("ac-blackball", "fg-blackball1", "sh-blackball")
    def_shmodel("ac-blackball-shine", "fg-blackball2", "sh-blackball")

    -- Falling
    img=SpriteImages("ac-blackball-fall", 10)
    f=composeframes(img,{70,65,60,55,50,50,50,50,50,50,50})
    def_anim("ac-blackball-fall", f)
    def_alias("ac-blackball-fallen", "invisible")

    -- Appearing / disappearing
    def_anim("ac-blackball-appear", reverseframes(buildframes(img, 25)))
    def_anim("ac-blackball-disappear", buildframes(img, 25))

    def_alias("ac-blackball-shattered", "ac-blackball-shatter5")
end



----------------
-- White ball --
----------------

do
    local img,f

    -- Normal
    def_alias("sh-whiteball", "sh-blackball")
    img=SpriteImage ("fg-whiteball")
--    def_image("fg-wb", {filename="ac-whiteball", xoff=-9, yoff=-9})
    def_shmodel("ac-whiteball", "fg-whiteball", "sh-whiteball")
    def_alias("ac-whiteball-shine", "ac-whiteball")

    -- Falling
    img=SpriteImages("ac-whiteball-fall", 10)
    f=composeframes(img,{70,65,60,55,50,50,50,50,50,50,50})
    def_anim("ac-whiteball-fall", f)
    def_alias("ac-whiteball-fallen", "invisible")

    -- Appearing / disappearing
    def_anim("ac-whiteball-appear", reverseframes(buildframes(img, 25)))
    def_anim("ac-whiteball-disappear", buildframes(img, 25))

    -- Shattering

    def_alias("ac-whiteball-shattered", "ac-whiteball-shatter5")
end

-- Jumping normal balls --
do
    namelist = SpriteImages("ac-blackball-jump", 4)
    shadows  = SpriteImages("sh-blackball-jump", 4, 0.4)
    frames   = {}
    for i=1,4 do
        def_shmodel("bb-jump"..i, namelist[i], shadows[i])
        tinsert(frames, "bb-jump"..i)
    end
    def_anim("ac-blackball-jump", pingpong(buildframes(frames, 70)))

    namelist = SpriteImages("ac-whiteball-jump", 4)
    frames   = {}
    for i=1,4 do
        def_shmodel("wb-jump"..i, namelist[i], shadows[i])
        tinsert(frames, "wb-jump"..i)
    end
    def_anim("ac-whiteball-jump", pingpong(buildframes(frames, 70)))
end

----------------
-- Cannonball --
----------------

def_alias ("ac-cannonball", "ac-blackball-jump")



---------
-- Bug --
---------
do
    SpriteImage("fg-bug")
    def_shmodel("ac-bug", "fg-bug", "sh-whiteball-small")
end

-- Marbles in inventory  --
def_image("inv-blackball")
def_image("inv-whiteball")


------------------
-- Floor models --
------------------
Progress(10, "Loading floor models")

function def_floors(floorlist)
    for i,name in pairs(floorlist) do
	def_image(name)
    end
end

function def_randfloor(name, imagelist)
    def_floors(imagelist)
    display.DefineRandModel(name, getn(imagelist), imagelist)
end

-- _si for _subimage
function def_randfloor_si(name, height, width)
    imagelist=def_subimages(name, {h=height, w=width})
    display.DefineRandModel(name, getn(imagelist), imagelist)
end

--if options.WizardMode > 0 then
--    def_subimages("fl-gradient-wiz", {modelname="fl-gradient",w=6, h=4})
--else
def_subimages("fl-gradient2", {w=6, h=4, modelname="fl-gradient"})
--end

do
    def_subimages("fl-leaves", {w=10, h=2, modelname="fl-leavesx"})
    display.DefineRandModel("fl-leaves", 4, {"fl-leavesx1", "fl-leavesx2", "fl-leavesx3", "fl-leavesx4"})
    display.DefineRandModel("fl-leavesb", 4, {"fl-leavesx5", "fl-leavesx6", "fl-leavesx7", "fl-leavesx8"})
    def_alias("fl-leavesc1", "fl-leavesx9")
    def_alias("fl-leavesc2", "fl-leavesx10")
    def_alias("fl-leavesc3", "fl-leavesx11")
    def_alias("fl-leavesc4", "fl-leavesx12")
    def_alias("fl-leavesd1", "fl-leavesx13")
    def_alias("fl-leavesd2", "fl-leavesx14")
    def_alias("fl-leavesd3", "fl-leavesx15")
    def_alias("fl-leavesd4", "fl-leavesx16")
    def_alias("fl-leavese1", "fl-leavesx17")
    def_alias("fl-leavese2", "fl-leavesx18")
    def_alias("fl-leavese3", "fl-leavesx19")
    def_alias("fl-leavese4", "fl-leavesx20")
end

-- To get different wood tiles with horizontal or vertical slats.
-- Each floortype has got 2 random images.
do
    def_subimages("fl-wood", {w=1, h=4, modelname="fl-woodx"})
    display.DefineRandModel("fl-wood", 4, {"fl-woodx1", "fl-woodx2", "fl-woodx3", "fl-woodx4"})
    display.DefineRandModel("fl-wood1", 2, {"fl-woodx3", "fl-woodx4"})
    display.DefineRandModel("fl-wood2", 2, {"fl-woodx1", "fl-woodx2"})
end

def_randfloor("fl-rock", {"fl-rock1", "fl-rock2"})
def_floors{"fl-abyss"}
def_alias("fl-abyss_fake", "fl-abyss")
def_floors{"fl-dunes", "fl-sand", "fl-bluegreen", "fl-bluegreenx"}
def_floors{"fl-inverse", "fl-inverse2", "fl-white", "fl-black", "fl-acwhite", "fl-acblack"}
def_floors{"fl-springboard"}
def_image("fl-normal")
def_randfloor_si("fl-hay", 4)
def_floors{"fl-floor_001"}
def_floors{"fl-ice"}
def_floors{"fl-stone"}
def_alias("fl-normal_x", "fl-normal")
def_floors{"fl-dummy"}
def_floors{"fl-light", "fl-lightgray", "fl-darkgray"}
def_floors{"fl-trigger"}
def_randfloor_si("fl-concrete", 4)
def_randfloor_si("fl-gravel", 4)
def_randfloor_si("fl-bumps", 2,2)
def_randfloor_si("fl-mortar", 2, 2)

def_randfloor_si("fl-brick", 3)

def_randfloor_si("fl-space", 3,2)
def_alias("fl-space-force", "fl-space")

def_randfloor_si("fl-rough", 5)
def_alias("fl-rough_medium", "fl-rough")
def_alias("fl-rough_slow", "fl-rough")
def_randfloor_si("fl-gray", 5)
def_randfloor_si("fl-metal", 6)
def_randfloor_si("fl-plank", 4)
def_randfloor_si("fl-water", 4)
def_randfloor_si("fl-swamp", 4)
def_randfloor_si("fl-woven", 5)
def_randfloor_si("fl-marble", 4)
--def_subimages("fl-stwood", {h=2})
def_randfloor_si("fl-stwood", 2)
def_randfloor_si("fl-bluegray", 4)
def_randfloor_si("fl-red", 4)
def_randfloor_si("fl-rough-blue", 4)
def_randfloor_si("fl-rough-red", 4)
def_randfloor_si("fl-sahara", 4)
def_randfloor_si("fl-tigris", 4)
def_randfloor_si("fl-samba", 2)
def_randfloor_si("fl-himalaya", 4)
def_floors{"fl-woven_orange"}

--
-- Bridges
--
def_image("fl-bridgea-open")
def_image("fl-bridgea-closed")
namelist=def_subimages("fl-bridgea", {h=10})
frames = buildframes(namelist,70)
def_anim("fl-bridgea-opening", reverseframes(frames))
def_anim("fl-bridgea-closing", frames)


def_image("fl-bridgex-open")
def_image("fl-bridgex-closed")
namelist=def_subimages("fl-bridgex", {h=9})
frames = buildframes(namelist,70)
def_anim("fl-bridgex-opening", frames)
def_anim("fl-bridgex-closing", reverseframes(frames))


def_image("fl-bridgey-open")
def_image("fl-bridgey-closed")
namelist=def_subimages("fl-bridgey", {h=9})
frames = buildframes(namelist,70)
def_anim("fl-bridgey-opening", frames)
def_anim("fl-bridgey-closing", reverseframes(frames))

------------------------
-- Heating animations --
------------------------

function heating_animation(basemodel)
    local img = def_subimages(basemodel.."-heating", {h=8});    
    local f = buildframes(img, 240)
    def_anim(basemodel.."-heating", f);
end

heating_animation("fl-ice")
heating_animation("fl-water")
heating_animation("fl-swamp")


--------------------------------------------------------------------------
--                           ITEM MODELS                                --
--------------------------------------------------------------------------
Progress(20, "Loading item models")

do
--    def_image("items")
    local itemtiles={
        "it-brush", "it-floppy",
        "it-document", "it-hammer",
        "it-key", "it-spade",
        "it-umbrella", "it-extralife",
        "it-hill", "it-hollow", "it-tinyhill", "it-tinyhollow",
        "it-laserh", "it-laserv", "it-laserhv",
        "it-magicwand",
        "it-spring1", "it-spring2",
        "it-sword",
        "it-surprise",
        "it-trigger", "it-trigger1",
        "it-yinyang", "it-yanying",
        "it-puller-n", "it-puller-e", "it-puller-s", "it-puller-w",
        "it-odometer",
        "it-flagblack", "it-flagwhite",
        "it-ring", "it-pin", "it-bag", "it-drop", "it-rubberband"
    }
--    def_tiles("items", itemtiles)
    DefineTiles ("items", itemtiles)
end

-- Puller animation
do
    local models={"it-puller-n", "it-puller-e", "it-puller-s", "it-puller-w"}
    local frames=buildframes(models, 100)
    def_anim("it-puller-active", repeatanim(frames, 4), FALSE)
end

do
    local n=def_subimages("it-coffee", {h=4})
    local f=buildframes(n,150)
    def_anim("it-coffee", f, TRUE)
end

-- Some more Items

def_images{
    "it-blackbomb",
    "it-whitebomb",
    "it-dynamite",
    "it-dummy",
    "it-blocker",
    "it-wrench",
    "it-cherry",
    "it-glasses",
    "it-glasses-broken",
    "it-weight",
    "it-landmine",
    "it-hstrip",
    "it-vstrip",
    "it-booze",
    "it-banana", 
    "it-spoon",
    "it-cross",
    "it-pencil",
    "it-squashed",
    "it-booze-broken",
}

def_alias("it-key_a", "it-key")
def_alias("it-key_b", "it-key")
def_alias("it-key_c", "it-key")

-- Seed --
do
    local n = def_subimages("it-seed", {h=5})
    def_alias("it-seed", "it-seed1")
    def_alias("it-seed_nowood", "it-seed")
    def_alias("it-seed_volcano", "it-seed")
    local f={
        "it-seed1", "it-seed2", "it-seed1", "it-seed3", "it-seed1", "it-seed2",
        "it-seed1", "it-seed4", "it-seed5", "it-seed4", "it-seed1",
        "it-seed4", "it-seed5",
    }
    def_anim("it-seed-growing", buildframes(f, 120))
end

-- Cracks --
namelist=def_subimages("it-crack", {h=8})
--frames=buildframes({"it-crack4", "it-crack6", "it-crack7", "it-crack8"},90)
frames=buildframes(namelist,50)
def_anim("it-debris", frames)

frames=buildframes({"it-crack4", "it-crack5"},120)
def_anim("it-crack_anim1", frames)

frames=buildframes({"it-crack6", "it-crack7", "it-crack8"},120)
def_anim("it-crack_anim2", frames)

-- Burning Floor --
frames=buildframes(def_subimages("it-burnable_ignite", {h=8}), 100)
def_anim("it-burnable_ignited", frames)

frames=buildframes(def_subimages("it-burnable_burning", {h=8}), 100)
def_anim("it-burnable_burning", frames)

def_images{ "it-burnable_ash", "it-burnable_fireproof"}
def_alias("it-burnable", "invisible")

do
   local img = def_subimages("it-extinguisher", {h=3})
   def_alias("it-extinguisher", img[1])
   def_alias("it-extinguisher_medium", img[2])
   def_alias("it-extinguisher_empty", img[3])
end

-- Oil --
def_subimages("it-burnable_oil", {h=4})

-- Magnet --
def_image("it-magnet-off")
frames=buildframes(def_subimages("it-magnet-on", {h=5}), 100)
def_anim("it-magnet-on", frames, 1)

-- Wormhole --
do
    local f = buildframes(def_subimages("it-wormhole", {h=2}), 100)
    def_anim("it-wormhole", f, TRUE)
    def_alias("it-wormhole-off", "it-wormhole1")
end

-- Vortex --
do
   local img = def_subimages("it-vortex", {h=4})
   def_alias("it-vortex-open", img[1])
   def_alias("it-vortex-closed", img[4])

   def_anim("it-vortex-opening", reverseframes(buildframes(img, 100)))
   def_anim("it-vortex-closing", buildframes(img, 100))
end

-- Coins --
def_images{"it-coin1", "it-coin2", "it-coin4"}

-- Burning dynamite --
dyn_frames=buildframes(def_subimages("it-dynamite-burning", {h=15}), 100)
def_anim("it-dynamite-burning", dyn_frames) --repeat_frames(dyn_frames,3,2))

-- Burning bomb --
frames=buildframes(def_subimages("it-blackbomb-burning", {h=9}), 100)
def_anim("it-blackbomb-burning", frames) --repeat_frames(frames,3,2))

frames=buildframes(def_subimages("it-whitebomb-burning", {h=9}), 100)
def_anim("it-whitebomb-burning", frames) --repeat_frames(frames,3,2))

-- Explosion --
def_anim_images("expl", {{"expl", 50}})
def_alias("it-explosion1", "expl")
def_alias("it-explosion2", "expl")
def_alias("it-explosion3", "expl")

-- Pipes --
DefineTiles("it-pipe", {
                "it-pipe-e", "it-pipe-s", "it-pipe-es", "it-pipe-sw",
                "it-pipe-h", "it-pipe-w", "it-pipe-n", "it-pipe-ne",
                "it-pipe-wn", "it-pipe-v"})

-- broken Bottle --
-- Should get more models like oil!
--def_subimages("it-brokenbottle", {h=1})

-----------------
-- Shogun dots --
-----------------

NewAnim("it-shogun-s", {img="it-shogun-small", h=3, speed=160, pingpong=1, loop=1})
NewAnim("it-shogun-m", {img="it-shogun-med",   h=3, speed=160, pingpong=1, loop=1})
NewAnim("it-shogun-l", {img="it-shogun-big",   h=3, speed=160, pingpong=1, loop=1})

-- Springboard --
do
    local n = {"it-springboard1", "it-springboard2"}
    DefineTiles ("it-springboard", n)
    def_alias ("it-springboard", n[1])
    def_anim("it-springboard_anim", buildframes(reverseframes(n),120))
end

-- def_alias("it-bridge-oxyd", "invisible")
-- def_alias("it-sensor", "invisible")
-- def_alias("it-inversesensor", "invisible")

-- it-death --
do
   local img = def_subimages("it-death", {h=4})
   def_alias("it-death", img[1])
   def_anim("it-death-anim", buildframes(img, 100))
end

------------------
-- STONE MODELS --
------------------
Progress(30, "Loading stone models")

-------------------
-- Stone shadows --
-------------------
def_image("sh-solid")
def_image("sh-round")
def_image("sh-round2")
def_image("sh-grate1")
def_image("sh-grate2")
def_image("sh-grate3")
def_image("sh-glass")
def_image("sh-white4")
def_image("sh-puzzle1")
def_image("sh-brake")
def_image("sh-floating")

-- stone models
def_stone("st-glass", "sh-glass");

def_stone("st-bug")

-- Scissors
do
    def_stone("st-scissors")
    local n=def_subimages("st-scissors-snip", {h=1})
    def_anim("st-scissors-snip-anim", buildframes(n, 130))
    def_solidstone("st-scissors-snip", "st-scissors-snip-anim")
end

def_stone("st-rubberband")


do
    local tiles = {
        "st-disco-light",
        "st-disco-medium",
        "st-disco-dark"
    }
    map_tiles({h=3},
              function (i, rect)
                  local model = tiles[i]
                  display.DefineSubImage(model, "st-disco", 0,0, rect)
              end)
end

-- other

do
--    def_image("stones")
    local stonetiles={
        "st-black1#",
        "st-black2#",
        "st-black3#",
        "st-black4#",
        "st-block#",
        "st-idontknowwhattonameit2",
        "st-beads#"
    }
    DefineTiles("stones", stonetiles)
end

def_alias("st-explosion", "expl")

def_stone2("st-black1")
def_stone2("st-black2")
def_stone2("st-black3", "sh-grate1")
def_stone2("st-black4", "sh-white4")
def_stone2("st-block")
def_stone2("st-beads", "sh-glass")


def_stone("st-glass1", "sh-glass")
def_stone("st-glass2", "sh-glass")
def_stone("st-glass3", "sh-glass")
def_stone("st-rock1", "sh-round")
def_stone("st-rock2")
def_stone("st-rock3")
def_stone("st-rock4")
def_stone("st-rock5")
def_stone("st-rock6")
def_stone("st-rock7")
def_stone("st-rock8", "sh-round")
def_stone("st-rock9", "sh-round")
def_stone("st-rock10")
def_stone("st-redrock")
def_stone("st-brownie", "sh-round")
def_stone("st-stone1")
def_stone("st-stone2")
def_stone("st-bumps")

def_stone("st-camouflage")
def_alias("st-camouflage_move", "st-camouflage")
def_alias("st-camouflage_hole", "st-camouflage")

def_alias("st-laserbreak", "st-rock3")
def_alias("st-laserbreak-anim", "st-rock3_break-anim")

def_alias("st-rock1_hole", "st-rock1")
def_alias("st-rock1_move", "st-rock1")

def_alias("st-rock2_hole", "st-rock2")

def_alias("st-rock3_hole", "st-rock3")
def_alias("st-rock3_move", "st-rock3")
def_alias("st-rock3_break", "st-rock3")

def_stone("st-marble", "sh-round")
def_alias("st-marble_hole", "st-marble")
def_alias("st-marble_move", "st-marble")

def_alias("st-glass_move", "st-glass");
def_alias("st-glass1_move", "st-glass1")
def_alias("st-glass1_hole", "st-glass1")
def_alias("st-glass2_move", "st-glass2")
def_alias("st-glass2_hole", "st-glass2")

def_stone("st-metal")
def_alias("st-metal_hole", "st-metal")
def_stone("st-blue-sand")
def_stone("st-flrock")
def_stone("st-flhay")

def_stone("st-firebreak")
def_alias("st-firebreak_move", "st-firebreak")

do
   local sh=def_subimages("sh-round2-growing", {h=3,imgw=ShadowSize,imgh=ShadowSize})

   -- Wooden stones --
   do
      def_subimages("st-wood", {modelname="st-wood-fg",h=2})
      def_shmodel("st-wood1", "st-wood-fg1", "sh-round")
      def_shmodel("st-wood2", "st-wood-fg2", "sh-round")

      local n=def_subimages("st-wood-growing", {h=3})
      def_anim("wood-growing-fg", buildframes(n, 130))
      def_anim("wood-growing-bg", buildframes(sh, 130))
      def_shmodel("st-wood-growing", "wood-growing-fg", "wood-growing-bg")
      def_shmodel("st-greenbrown-growing", "wood-growing-fg", "wood-growing-bg")
   end

   -- Blocker stone
   do
      local n=def_subimages("st-blocker", {h=4})
      def_roundstone("st-blocker", n[1])
      frames={}
      for i=4,2,-1 do
         tinsert(frames, "st-blocker"..i)
      end
      def_anim("blocker-growing-fg", buildframes(frames, 60))
      def_anim("blocker-growing-bg", buildframes(sh, 60))
      def_anim("blocker-shrinking-fg", reverseframes(buildframes(frames, 60)))
      def_anim("blocker-shrinking-bg", reverseframes(buildframes(sh, 60)))
      def_shmodel("st-blocker-shrinking", "blocker-shrinking-fg", "blocker-shrinking-bg");
      def_shmodel("st-blocker-growing", "blocker-growing-fg", "blocker-growing-bg");
   end
end

def_alias("st-volcano-growing", "st-blocker-growing")

-- Rotator stones
do
    local n=def_subimages("st-rotator-left", {h=8})
    def_anim("st-rotator-left-anim", buildframes(n, 70), FALSE)
    def_roundstone("st-rotator-left", "st-rotator-left-anim")
    n=def_subimages("st-rotator-right", {h=8})
    def_anim("st-rotator-right-anim", buildframes(n, 70), FALSE)
    def_roundstone("st-rotator-right", "st-rotator-right-anim")
end

def_stone("st-grate1", "sh-grate1")
def_stone("st-grate2", "sh-grate2")
def_stone("st-grate3", "sh-grate3")

def_stone("st-brake", "sh-brake")
def_image("it-brake", {filename="st-brake"})

def_stone("st-greenbrown", "sh-round")
def_alias("st-greenbrown_hole", "st-greenbrown")
def_alias("st-greenbrown_move", "st-greenbrown")

--Progress(40)


--def_stone("st-block")
def_stone("st-brick")
def_stone("st-woven")
def_stone("st-brick_magic", nil, {filename="st-brick"})

-- Switches --
do
    function mkswitch(modelname, basename)
	local n = def_subimages(modelname, {h=3, modelname=basename.."-fg"})
	local f = buildframes(n, 60)
	def_roundstone(modelname.."-off", n[1])
	def_roundstone(modelname.."-on", n[3])
	def_anim(basename.."-turnon", f)
	def_anim(basename.."-turnoff", reverseframes(f))
	def_roundstone(modelname.."-turnon", basename.."-turnon")
	def_roundstone(modelname.."-turnoff", basename.."-turnoff")
    end
    mkswitch("st-switch", "switch")
    mkswitch("st-switch_black", "switch_black")
    mkswitch("st-switch_white", "switch_white")
end

-- 4-Switches --
do
   def_subimages("st-fourswitch",{h=4})

   def_shmodel("st-fourswitch-n", "st-fourswitch1", "sh-round")
   def_shmodel("st-fourswitch-e", "st-fourswitch2", "sh-round")
   def_shmodel("st-fourswitch-s", "st-fourswitch3", "sh-round")
   def_shmodel("st-fourswitch-w", "st-fourswitch4", "sh-round")
end

def_stone("st-floppy0", "sh-round", {filename="st-floppy1"})
def_stone("st-floppy1", "sh-round", {filename="st-floppy2"})

def_image("st-easymode")


def_stone("st-laserswitch0", nil, {filename="st-oxydb"})
def_anim_images("laserswitch-blink",
                repeatanim(pingpong(
                            buildframes(framenames("st-fakeoxyd-blink", 1,4),
                            50))), {loop=1})
def_solidstone("st-laserswitch1", "laserswitch-blink")

def_stone("st-wood_001")
def_stone("st-key0", "sh-round", {filename="st-key1"})
def_stone("st-key1", "sh-round", {filename="st-key2"})
def_stone("st-white1")
def_stone("st-white2")
def_stone("st-white3", "sh-grate1")
def_stone("st-white4", "sh-white4")
def_stone("st-yinyang1")
def_stone("st-yinyang2")
def_alias("st-yinyang3", "st-yinyang1")
def_stone("st-bluegray", "sh-round")
def_stone("st-bluegray_hole", "sh-round", {filename="st-bluegray"})
def_stone("st-yellow")
def_stone("st-dummy")
def_stone("st-blackballs")
def_stone("st-whiteballs")


def_stone("st-rock3_cracked")

---------------
-- st-plain* --
---------------
def_alias("st-plain", "st-rock3")
def_alias("st-plain_hole", "st-rock3")
def_alias("st-plain_breaking", "st-rock3_break-anim")
def_alias("st-plain_break", "st-rock3")
def_alias("st-plain_cracked", "st-rock3_cracked")
def_alias("st-plain_move", "st-rock3")
def_alias("st-plain_falling", "st-rock3-falling")

--------------------------
-- pull and swap stones --
--------------------------

--def_stone("st-swap", "sh-round")

do
   def_subimages("st-pull",{modelname="st-pull-fg",h=5})
   def_subimages("st-swap",{modelname="st-swap-fg",h=5})
   def_subimages("sh-pull",{h=4,imgw=ShadowSize,imgh=ShadowSize})

   def_shmodel("st-pull",   "st-pull-fg1", "sh-glass")
   def_shmodel("st-pull-n", "st-pull-fg2", "sh-pull1")
   def_shmodel("st-pull-s", "st-pull-fg3", "sh-pull2")
   def_shmodel("st-pull-e", "st-pull-fg4", "sh-pull3")
   def_shmodel("st-pull-w", "st-pull-fg5", "sh-pull4")

   def_shmodel("st-swap",   "st-swap-fg1", "sh-round")
   def_shmodel("st-swap-n", "st-swap-fg2", "sh-pull1")
   def_shmodel("st-swap-s", "st-swap-fg3", "sh-pull2")
   def_shmodel("st-swap-e", "st-swap-fg4", "sh-pull3")
   def_shmodel("st-swap-w", "st-swap-fg5", "sh-pull4")
end

-----------------
-- Oxyd Stones --
-----------------

-- Oxyd stones occur in different colors and different flavors and
-- for each combination of these two attributes we need animations
-- for the following internal states or state transitions:
--
--   (a) opening and closing
--   (b) blinking
--   (c) opened
--
-- Creating the required image files by hand would be tedious, so
-- we use a couple of specialized functions and overlays to create
-- them automatically.

do
    local colorspots = framenames("st-oxydbtempl", 2,9)
    local openovls = framenames("st-oxydbtempl", 10,14)

-- Define "fading in" and "fading out" animations for oxyd stones.
-- These two animations are combined with the stone images to
-- produce the opening and closing animations for oxyd stones.

    local baseimg = {
        a="st-oxyda-open",
        b="st-oxydb-open",
        c="st-oxydc-open",
        d="st-oxydd-open"
    }
    local shadow = {
       a="sh-round",
       b="sh-round",
       c="sh-solid",
       d="sh-solid",
    }
    local fopening = {
        a = buildframes(def_subimages("st-oxyda-opening", {h=9}), 60),
        b = buildframes(def_subimages("st-oxydb-opening", {h=14}), 40),
        c = buildframes(def_subimages("st-oxydc-opening", {h=5}), 70),
        d = buildframes(def_subimages("st-oxydd-opening", {h=5}), 70),
    }
    local fclosing = {
        a = reverseframes(fopening["a"]),
        b = reverseframes(fopening["b"]),
        c = reverseframes(fopening["c"]),
        d = reverseframes(fopening["d"]),
    }

    function mkopenclose(flavor, color)
        local n = "st-oxyd" .. flavor .. color
        local fadein = "oxyd"..flavor.."-fadein"
        local fadeout= "oxyd"..flavor.."-fadeout"

        def_overlay(n.."-base", {baseimg[flavor], colorspots[color+1]})
        display.DefineComposite(n.."-opening-fg", n.."-base", fadein)
        display.DefineComposite(n.."-closing-fg", n.."-base", fadeout)
        def_shmodel (n.."-opening", n.."-opening-fg", shadow[flavor])
        def_shmodel (n.."-closing", n.."-closing-fg", shadow[flavor])
    end

    function mkblink(flavor, color)
        local n = "st-oxyd"..flavor..color.."-blink"
        local img={baseimg[flavor],colorspots[color+1], "st-oxyd-questmark"}
        def_overlay(n..1, img)
        def_overlay(n..2, {baseimg[flavor], colorspots[color+1]})
        def_anim(n.."-anim", buildframes({n..1,n..2}, 500), 1)
        def_shmodel(n, n.."-anim", shadow[flavor])
    end

    function mkopened(flavor, color)
        local n = "st-oxyd" .. flavor .. color .. "-open"
        local names = {}

        for i=1,getn(openovls) do
            local images={baseimg[flavor],colorspots[color+1],openovls[i]}
            names[i] = n .. format("_%04d", i)
            def_overlay(names[i], images)
        end

        -- compose these images into an animation
        frames = pingpong(buildframes(names, 100))
        def_anim(n.."-anim", frames, TRUE)

        -- and finally add a shadow to make the model complete
        def_shmodel(n, n.."-anim", shadow[flavor])
    end

    function mkoxyd(flavor)
        def_stone("st-oxyd"..flavor, shadow[flavor])
        def_shmodel("st-likeoxyd"..flavor, "st-oxyd"..flavor, shadow[flavor])
--        def_solidstone("st-likeoxyd"..flavor, "st-oxyd"..flavor)
        img=def_image("st-oxyd"..flavor.."-open")
        def_shmodel("st-likeoxyd"..flavor.."-open", img, shadow[flavor])

        local fadein = "oxyd"..flavor.."-fadein"
        local fadeout= "oxyd"..flavor.."-fadeout"
        def_anim(fadein, fopening[flavor])
        def_anim(fadeout, fclosing[flavor])

        for color=0,7 do
            mkopenclose(flavor, color)
            mkblink(flavor, color)
            mkopened(flavor, color)
        end
    end
    mkoxyd("a")
    mkoxyd("b")
--    Progress(50)
    mkoxyd("d")
    mkoxyd("c")
end

def_alias("st-coffee", "st-oxydc")

--Progress(60)

-----------------
-- Laser stone --
-----------------
function make_laser(dir)
    laseron=framenames("st-laser"..dir, 1, 9)

    -- deactivated laser
    def_overlay("laser"..dir, {"st-laser-base", laseron[1]})
    def_roundstone("st-laser"..dir, "laser"..dir)

    -- activated laser
    names = {}
    for i=1,getn(laseron) do
	names[i] = "st-laseron"..dir .. format("_%04d", i)
	def_overlay (names[i], {"st-laser-base", laseron[i]})
    end
    frames = buildframes(names, 100)
    def_anim("st-laseron-anim"..dir, frames, 1)
    def_roundstone("st-laseron"..dir, "st-laseron-anim"..dir)
end

make_laser("-e")
make_laser("-s")
make_laser("-w")
make_laser("-n")

---------------------
-- Fake oxyd stone --
---------------------
def_stone("st-fakeoxyd", "sh-round", {filename="st-oxydb"})
def_anim_images("fakeoxyd-blink",
                repeatanim(pingpong(
                            buildframes(framenames("st-fakeoxyd-blink", 1,4),
                                        50))))
def_roundstone("st-fakeoxyd-blink", "fakeoxyd-blink")

def_alias("st-fakeoxyda", "st-oxyda")

-- Fart stone
def_stone("st-fart", "sh-round", {filename="st-oxydb"})
def_anim_images("farting",
                repeatanim(pingpong(
                            buildframes(framenames("st-fakeoxyd-blink", 1,4),
                                        50))))
def_roundstone("st-farting", "farting")

namelist = def_subimages("st-fart-break",{h=6})
def_anim("fartbreak-anim", buildframes(namelist,50))
def_roundstone("st-fartbreak-anim", "fartbreak-anim")

-- st-rock3_movebreak

namelist = def_subimages("st-rock3-break", {h=6})
def_anim("rock3_break-anim",buildframes(namelist,50))
def_solidstone("st-rock3_break-anim", "rock3_break-anim")
def_alias("st-rock3_movebreak", "st-rock3")

-- st-rock3-falling

do
    local n = def_subimages("st-rock3-falling", {h=4})
    def_anim("st-rock3-falling", buildframes(n, 100))
end

-- Breaking stone

def_alias("st-breaking", "st-rock3_break-anim")

-- Actor impulse stone
do
   namelist = def_subimages("st-actorimpulse", {h=3})
   shnamelist = def_subimages("sh-actorimpulse", {h=3,imgw=ShadowSize,imgh=ShadowSize})

   def_anim("st-ai-fg", pingpong(buildframes(namelist, 30)))
   def_anim("st-ai-sh", pingpong(buildframes(shnamelist, 30)))
   def_shmodel("st-actorimpulse-anim", "st-ai-fg", "st-ai-sh")

   def_shmodel("st-actorimpulse", namelist[1], shnamelist[1])
end

-- Stone impulse stone
--
-- Note: It's important that the duration of the closing animation
-- (anim2) is longer than the opening animation (anim1). Otherwise
-- impulse stones do not work properly!

do
   namelist = def_subimages("st-stoneimpulse", {h=4})
   def_roundstone("st-stoneimpulse", namelist[1])
   frames={}
   for i=1,4 do tinsert(frames, namelist[i]) end
   def_anim("stoneimpulse-anim1", buildframes(frames, 55))
   def_roundstone("st-stoneimpulse-anim1", "stoneimpulse-anim1")
   tinsert(frames, namelist[4]) -- add 1 frame to make closing anim longer!
   def_anim("stoneimpulse-anim2", reverseframes(buildframes(frames, 55)))
   def_roundstone("st-stoneimpulse-anim2", "stoneimpulse-anim2")
end

do
   namelist = def_subimages("st-stoneimpulse-hollow", {h=4})
   def_shmodel("st-stoneimpulse-hollow", namelist[1], "sh-floating")
   frames={}
   for i=1,4 do tinsert(frames, namelist[i]) end
   def_anim("stoneimpulse-hollow-anim1", buildframes(frames, 55))
   def_shmodel("st-stoneimpulse-hollow-anim1", "stoneimpulse-hollow-anim1", "sh-floating")
   tinsert(frames, namelist[4]) -- add 1 frame to make closing anim longer!
   def_anim("stoneimpulse-hollow-anim2", reverseframes(buildframes(frames, 55)))
   def_shmodel("st-stoneimpulse-hollow-anim2", "stoneimpulse-hollow-anim2", "sh-floating")
end

---------------------------
-- Thief stone and floor --
---------------------------
do
    -- Base images

    local stonebase = "st-bluegray"
    local floorbase = "fl-bluegray"
    local thiefovl = def_subimages("thief-template", {h = 7})
    local capturedovl = def_subimages("thief-captured-template", {h = 12})

    -- Creating st-thief

    local names = {}
    for j = 1, getn(thiefovl) do
        names[j] = "st-thief"..format("_%04d", j)
        display.DefineComposite(names[j], stonebase, thiefovl[j])
    end

    local f1 = buildframes(names, 80)
    def_anim("pre-st-thief-emerge", f1)
    def_anim("pre-st-thief-retreat", reverseframes(f1))
    def_roundstone("st-thief", stonebase)
    def_roundstone("st-thief-emerge", "pre-st-thief-emerge")
    def_roundstone("st-thief-retreat", "pre-st-thief-retreat")

    -- Creating st-thief-captured
    --
    --   Note that this is done without template, as the whole
    --   stone has to disappear (e.g. via shrinking)

    local img2 = def_subimages("st-thief-captured", {h = 12})
    local f2 = buildframes(img2, 80)
    def_anim("st-thief-captured", f2)

    -- Creating fl-thief

    local floornames = {}
    local floorcaptured = {}
    local floorbases = def_subimages(floorbase, {h=4, modelname="fl-thief-base"})
    for k = 1,4 do
      floornames[k] = {}
      for j = 1, getn(thiefovl) do
        floornames[k][j] = "fl-thief"..k..format("_%04d", j)
        display.DefineComposite(floornames[k][j], floorbases[k], thiefovl[j])
      end
      floorcaptured[k] = {}
      for j = 1, getn(capturedovl) do
        floorcaptured[k][j] = "fl-thief"..k.."-captured"..format("_%04d", j)
        display.DefineComposite(floorcaptured[k][j], floorbases[k], capturedovl[j])
      end
      local f3 = buildframes(floornames[k], 80)
      local f4 = buildframes(floorcaptured[k], 80)
      def_alias("fl-thief"..k, floorbases[k])
      def_anim("fl-thief"..k.."-emerge", f3)
      def_anim("fl-thief"..k.."-retreat", reverseframes(f3))
      def_anim("fl-thief"..k.."-captured", f4)
    end

--do
--    local img = def_subimages("fl-thief", {h=7})
--    local f = buildframes(img, 80)
--    def_anim("thief-emerge", f)
--    def_anim("thief-retreat", reverseframes(f))
--    def_roundstone("fl-thief", img[1])
--    def_roundstone("fl-thief-emerge", "thief-emerge")
--    def_roundstone("fl-thief-retreat", "thief-retreat")
--end

end

-----------------
-- Chess stone --
-----------------
function make_chess(colour)
    local img1 = def_subimages("st-chess"..colour, {h=5});
    --local img2 = def_subimages("st-chess"..colour.."-captured", {h=5});
    local f1 = buildframes(img1, 120)
    --local f2 = buildframes(img2, 40)
    local f2 = f1
    local f3 = buildframes(img1, 500)
    def_anim("st-chess"..colour.."-disappearing", f1);
    def_anim("st-chess"..colour.."-appearing", reverseframes(f1))
    def_anim("st-chess"..colour.."-captured", f2);
    def_anim("st-chess"..colour.."-swamp", f3);
    def_roundstone("st-chess"..colour, img1[1])
end

make_chess("_black")
make_chess("_white")

-----------------
-- Timer stone --
-----------------
do
    local img = def_subimages("st-timer", {h=4})
    def_anim("timer-anim", buildframes(img, 120), 1)
    def_roundstone("st-timer", "timer-anim")
    def_roundstone("st-timeroff", "st-timer1")
end

-----------------
-- Bomb Stones --
-----------------

function make_bombstone(name)
    local n=def_subimages(name, {h=7})
    def_roundstone(name, name.."1")
    def_anim(name.."-anim", buildframes(n,50))
end

make_bombstone("st-bombs")
make_bombstone("st-dynamite")
make_bombstone("st-whitebombs")

----------------
-- Mail stone --
----------------
do
    local n=def_subimages("st-mail", {h=4})
    def_solidstone("st-mail-n", "st-mail1")
    def_solidstone("st-mail-e", "st-mail2")
    def_solidstone("st-mail-s", "st-mail4")
    def_solidstone("st-mail-w", "st-mail3")
end

function def_solidstone_with_anim(name, npictures, frametime)
    local n=def_subimages(name, {h=npictures})
    def_anim(name.."-animfg", buildframes(n,frametime))
    def_solidstone(name.."-anim", name.."-animfg")
    def_solidstone(name, n[1])
end
function def_roundstone_with_anim(name, npictures, frametime)
    local n=def_subimages(name, {h=npictures})
    def_anim(name.."-animfg", buildframes(n,frametime))
    def_roundstone(name.."-anim", name.."-animfg")
    def_roundstone(name, n[1])
 end

def_solidstone_with_anim ("st-window", 4, 80)
def_solidstone_with_anim ("st-stone_break", 10, 50)
def_solidstone_with_anim ("st-break_bolder", 10, 50)
def_roundstone_with_anim ("st-break_acwhite", 10, 50)
def_roundstone_with_anim ("st-break_acblack", 10, 50)
def_roundstone_with_anim ("st-break_gray", 10, 50)
def_roundstone_with_anim ("st-death", 3, 140)


-- Flash stone --
do
    local n=def_subimages("st-flash", {h=1})
    def_solidstone("st-flash", "st-flash1")
end

-- Surprise stone --
do
    def_stone ("st-surprise")
end

-- Charge stones --
do
    function def_chargestone(basename)
        local n=def_subimages(basename, {h=2})
        def_solidstone(basename, n[1])
        def_anim(basename.."-animfg", {{n[2], 140}})
        def_solidstone(basename.."-anim", basename.."-animfg")
    end
    def_chargestone("st-chargeplus")
    def_chargestone("st-chargeminus")
    def_chargestone("st-chargezero")
end

---------------
-- Turnstile --
---------------
do
   local img = def_subimages("st-turnstile", {h=2})
   def_anim("turnstile-anim", reverseframes(buildframes(img, 30)))
   def_solidstone("st-turnstile", img[1])
   def_solidstone("st-turnstile-anim", "turnstile-anim")
end

do
   local img = def_subimages("st-turnstile-green", {h=2})
   def_anim("turnstile-green-anim", reverseframes(buildframes(img, 30)))
   def_solidstone("st-turnstile-green", img[1])
   def_solidstone("st-turnstile-green-anim", "turnstile-green-anim")
end

----------------
-- Timeswitch --
----------------
do
    def_stone("st-timeswitch")
    local img = def_subimages("st-time1switch", {h=2})
    def_anim("st-time1switch-anim", buildframes (img, 180), 1)
    def_solidstone("st-time1switch", "st-time1switch-anim")
end

--Progress(70)

-------------
-- Mirrors --
-------------

-- naming scheme for mirror models:
--
--	st-{3mirror,pmirror}-{m,s}{o,t}[1234]
--
-- {m,s} -> movable or static
-- {o,t} -> opaque or transparent
--
-- The numbers map to actual orientations as follows:
--
--   NUMBER    TRIANG.M.   PLANE M.
--	1	south	  "v"        "-"
--	2	west	  "<"        "\"
--	3	north	  "^"        "|"
--	4	east	  ">"        "/"

function make_mirror(basename, baseimg, overlays)
    for i=1,4 do
	mname = basename .. i
	def_overlay (mname .. "-ovl", {baseimg, overlays[i]})
	def_shmodel(mname, mname .. "-ovl", "sh-round2")
    end
end

mirror3_opaque = framenames("st-mirrortempl", 1, 4)
mirror3_transp = framenames("st-mirrortempl", 5, 8)
mirrorp_opaque = framenames("st-mirrortempl", 9, 12)
mirrorp_transp = framenames("st-mirrortempl", 13, 16)

make_mirror("st-3mirror-mo", "st-mirror-movable", mirror3_opaque)
make_mirror("st-3mirror-so", "st-mirror-static",  mirror3_opaque)
make_mirror("st-3mirror-mt", "st-mirror-movable", mirror3_transp)
make_mirror("st-3mirror-st", "st-mirror-static",  mirror3_transp)

make_mirror("st-pmirror-mo", "st-mirror-movable", mirrorp_opaque)
make_mirror("st-pmirror-so", "st-mirror-static",  mirrorp_opaque)
make_mirror("st-pmirror-mt", "st-mirror-movable", mirrorp_transp)
make_mirror("st-pmirror-st", "st-mirror-static",  mirrorp_transp)

-- OneWay --
do
   local model_names = {  
       "st-oneway-e", "st-oneway-n", "st-oneway-s", "st-oneway-w",
       "st-oneway_black-e", "st-oneway_black-n", "st-oneway_black-s", "st-oneway_black-w",
       "st-oneway_white-e", "st-oneway_white-n", "st-oneway_white-s", "st-oneway_white-w"
   }
   def_subimages("st-oneway", {modelname="st-onewayx",w=3,h=4})
   for i=1,12 do def_floatingstone(model_names[i], "st-onewayx"..i) end
end

---------------
-- Coin slot --
---------------
do
    def_stone("st-coinslot")
    local img=def_subimages("st-coin2slot", {h=18})
    def_anim("st-coin2slot-anim", buildframes(img, 20))
    def_solidstone("st-coin2slot", "st-coin2slot-anim")
    def_solidstone("st-coinslot-active", "st-coin2slot18")
end


--Progress(80)

-------------------
-- Puzzle stones --
-------------------
do
    def_subimages("st-puzzle", {modelname="st-puzzlex",w=8,h=4})
    for i=2,16  do def_solidstone("st-puzzle"..i, "st-puzzlex"..i) end
    for i=18,32 do def_solidstone("st-puzzle"..i, "st-puzzlex"..i) end
    def_shmodel("st-puzzle1", "st-puzzlex1", "sh-puzzle1")
    def_shmodel("st-puzzle17", "st-puzzlex17", "sh-puzzle1")
end

def_alias("st-turnstile-e", "st-puzzle2")
def_alias("st-turnstile-w", "st-puzzle5")
def_alias("st-turnstile-s", "st-puzzle9")
def_alias("st-turnstile-n", "st-puzzle3")
--def_stone("st-turnstile")

---------------
-- Big brick --
---------------

do
   def_subimages("st-bigbrick", {modelname="st-bigbrickx",w=4,h=4})
   for i=1,16 do def_solidstone("st-bigbrick"..i, "st-bigbrickx"..i) end
end

--def_alias("st-bigbrick", "st-bigbrick1");
--def_alias("st-bigbrick-n", "st-bigbrick9");
--def_alias("st-bigbrick-e", "st-bigbrick5");
--def_alias("st-bigbrick-s", "st-bigbrick3");
--def_alias("st-bigbrick-w", "st-bigbrick2");
--def_alias("st-bigbrick-ne", "st-bigbrick13");
--def_alias("st-bigbrick-nw", "st-bigbrick10");
--def_alias("st-bigbrick-es", "st-bigbrick7");
--def_alias("st-bigbrick-sw", "st-bigbrick4");
--def_alias("st-bigbrick-ns", "st-bigbrick11");
--def_alias("st-bigbrick-ew", "st-bigbrick6");
--def_alias("st-bigbrick-nes", "st-bigbrick15");
--def_alias("st-bigbrick-new", "st-bigbrick14");
--def_alias("st-bigbrick-nsw", "st-bigbrick12");
--def_alias("st-bigbrick-esw", "st-bigbrick8");
--def_alias("st-bigbrick-nesw", "st-bigbrick16");

-----------------
-- BigBlueSand --
-----------------

do
   def_subimages("st-bigbluesand", {modelname="st-bigbluesandx",w=4,h=4})
   for i=1,16 do def_solidstone("st-bigbluesand"..i, "st-bigbluesandx"..i) end
end

-----------
-- Doors --
-----------
do
    local f,img,sh

    img=def_subimages("st-doorh", {h=7})
    sh=def_subimages("sh-doorh", {h=7,imgw=ShadowSize,imgh=ShadowSize})
    def_shmodel("st-doorh-open", img[1], sh[1])
    def_shmodel("st-doorh-closed", img[7], sh[7])
    def_anim("doorh-opening-fg", reverseframes(buildframes(img, 60)))
    def_anim("doorh-opening-bg", reverseframes(buildframes(sh, 60)))
    def_shmodel("st-doorh-opening", "doorh-opening-fg", "doorh-opening-bg")
    def_anim("doorh-closing-fg", buildframes(img, 60))
    def_anim("doorh-closing-bg", buildframes(sh, 60))
    def_shmodel("st-doorh-closing", "doorh-closing-fg", "doorh-closing-bg")


    img=def_subimages("st-doorv", {w=7})
    sh=def_subimages("sh-doorv", {w=7,imgw=ShadowSize,imgh=ShadowSize})
    def_shmodel("st-doorv-open", img[1], sh[1])
    def_shmodel("st-doorv-closed", img[7], sh[7])
    def_anim("doorv-opening-fg", reverseframes(buildframes(img, 60)))
    def_anim("doorv-opening-bg", reverseframes(buildframes(sh, 60)))
    def_shmodel("st-doorv-opening", "doorv-opening-fg", "doorv-opening-bg")
    def_anim("doorv-closing-fg", buildframes(img, 60))
    def_anim("doorv-closing-bg", buildframes(sh, 60))
    def_shmodel("st-doorv-closing", "doorv-closing-fg", "doorv-closing-bg")
end

-- Door_a --
do
    def_alias("st-door_a-open", "st-grate1")
    def_alias("st-door_a-closed", "st-oxyda")
    local f = buildframes({"st-door_a-closed", "st-door_a-open"},60)
    def_anim("st-door_a-opening", f)
    def_anim("st-door_a-closing", reverseframes(f))
end

-- Door_b --
do
    def_alias("st-door_b-open", "invisible")
    def_alias("st-door_b-closed", "st-plain")
    local img=def_subimages("st-doorb", {modelname="doorb", h=8})
    local f = buildframes(img,60)
    def_anim("st-door_b-opening", f)
    def_anim("st-door_b-closing", reverseframes(f))
end

-- Door_c --
do
    def_alias("st-door_c-open", "st-grate2")
    def_alias("st-door_c-closed", "st-plain")
    frames=buildframes({"st-door_c-closed","st-door_c-open"},60)
    def_anim("st-door_c-opening", frames)
    def_anim("st-door_c-closing", reverseframes(frames))
end

------------------
-- Knight stone --
------------------
do
    local n = def_subimages("st-knight", {modelname="knight", h=5})
    for i=1,getn(n) do
        def_solidstone("st-knight"..i, n[i])
    end
end

-------------------
-- Shogun stones --
-------------------
do
    def_images{"sh-shogun1","sh-shogun2", "sh-shogun4"}
    def_subimages("st-shogun", {modelname="st-shogun-fg",h=7})

    def_shmodel("st-shogun1", "st-shogun-fg1", "sh-shogun1")
    def_shmodel("st-shogun2", "st-shogun-fg2", "sh-shogun2")
    def_shmodel("st-shogun3", "st-shogun-fg3", "sh-shogun1")
    def_shmodel("st-shogun4", "st-shogun-fg4", "sh-shogun4")
    def_shmodel("st-shogun5", "st-shogun-fg5", "sh-shogun1")
    def_shmodel("st-shogun6", "st-shogun-fg6", "sh-shogun2")
    def_shmodel("st-shogun7", "st-shogun-fg7", "sh-shogun1")
end

--Progress(90)

-------------------
-- Bolder stones --
-------------------

do
    local img=def_subimages("st-bolder", {w=4,h=3})
    local imgfall=def_subimages("st-bolder-fall", {w=4,h=3})

    function def_bolder(orient, start)
       local animname="st-bolder"..orient.."a"
       local frames={img[start], img[start+1], img[start+2]}
       def_anim(animname, buildframes(frames, 120), FALSE)
       def_shmodel("st-bolder-"..orient, animname, "sh-round")

       animname="st-bolder-"..orient.."-fall-anim"
       frames={imgfall[start],imgfall[start+1],imgfall[start+2]}
       def_anim(animname, buildframes(frames, 120), FALSE)
    end

    def_bolder("n",1)
    def_bolder("e",4)
    def_bolder("s",7)
    def_bolder("w",10)
end

-- Invisible stone --
def_alias("st-invisible", "invisible")
def_alias("st-invisible_magic", "invisible")
def_alias("st-stonebrush", "invisible")
def_alias("st-break_invisible", "invisible")
def_alias("st-actorimpulse_invisible", "invisible")
def_alias("st-death_invisible", "invisible")

----------------------
-- Magic stones :-) --
----------------------
do
    local img = def_subimages("st-magic", {h=4, modelname="st-magic-fg"})
    local nlist = {}
    for i=1,getn(img) do
        nlist[i] = "st-magic"..i
        def_roundstone(nlist[i], img[i])
    end
    display.DefineRandModel("st-magic", getn(nlist), nlist)
end

-------------------
-- Spitter stone --
-------------------
do
    def_alias ("st-spitter", "st-timeswitch")
    def_alias ("st-spitter-loading", "st-fakeoxyd-blink")
    def_alias ("st-spitter-spitting", "st-fakeoxyd-blink")
end

---------------------------
-- Light Passenger Stone --
---------------------------
do
    local img = def_subimages("st-lightpassenger", {h=7})
    def_shmodel("st-lightpassenger", img[1], "sh-glass")
    def_alias("st-lightpassenger_off", "st-glass2")
    local frames={img[2], img[3], img[4], img[5], img[6], img[7]}
    def_anim("st-lightpassenger-blink1", pingpong(buildframes(frames, 75)), TRUE)
    def_shmodel("st-lightpassenger-blink", "st-lightpassenger-blink1", "sh-glass")
    img = def_subimages("st-lightpassenger-break-v", {h=7})
    def_anim("st-lightpassenger-break-v", buildframes(img, 50), FALSE)
    img = def_subimages("st-lightpassenger-break-h", {h=7})
    def_anim("st-lightpassenger-break-h", buildframes(img, 50), FALSE)
    img = def_subimages("st-lightpassenger-break-hv", {h=7})
    def_anim("st-lightpassenger-break-hv", buildframes(img, 50), FALSE)
end

-------------
-- Effects --
-------------

Sprite{
    name     = "ring-anim", 
    nimages  = 8,
    framelen = 50,
    padding = 0.3
}

Sprite{
    name    = "ac-drowned",
    nimages = 5,
    framelen = 80,
    padding = 0.3
}

-- Halo --
Sprite{
    name     = "halo",
    nimages  = 2,
    framelen = 30,
    loop     = TRUE
}
Sprite{
    name     = "halo-blink",
    nimages  = 2,
    framelen = 30,
    loop     = TRUE
}

-- Halo for small balls --
Sprite{
    name     = "halo-small",
    nimages  = 2,
    framelen = 30,
    loop     = TRUE
}
Sprite{
    name     = "halo-small-blink",
    nimages  = 2,
    framelen = 30,
    loop     = TRUE
}

Progress(100, "Loading Enigma levels")
