------------------------------------------------------------------------
-- Copyright (C) 2002,2003,2004,2005 Daniel Heck
-- Copyright (C) 2007 Andreas Lochmann
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

----------------------------------------------------------------------
-- Sound handling
----------------------------------------------------------------------

default_sound = { name = "",
                  file = "",
                  volume = 1.0,
                  loop = false,
                  global = false,
                  priority = 1,
                  damp_max = 10.0,
                  damp_inc = 1.0,
                  damp_mult = 1.0,
                  damp_min = 0.5,
                  damp_tick = 0.9,
                  silence_string = "$default$" }

-- Complete a sound definition
function complete_sound(k, t)
    local tt = {}
    if type(t) == "string" then
        for key, value in pairs(default_sound) do
            tt[key] = value
        end
        tt.file = t
    elseif type(t) == "table" then
        for key, value in pairs(default_sound) do
            tt[key] = t[key] or value
            -- Special fix for "global" and "loop" values of 1.00 sound sets
            if (key == "global") or (key == "loop") then
                if tt[key] == 1 then
                    tt[key] = true
                elseif tt[key] == 0 then
                    tt[key] = false
                end
            end
        end
    else
        error("Syntax error in sound table, effect name "..k..".")
        return default_sound
    end
    -- Fill in the silence string if requested
    if (tt.silence_string == "$default$") then
        tt.silence_string = soundtable_silent[k].silence_string or k
    end
    return tt
end

-- Copy all key/value pairs only present in t1 to t2.
function copy_missing (t1, t2)
    for k,v in pairs(t1) do
        t2[k] = t2[k] or v
    end
end

-- Add a new sound set
function AddSoundSet (tablename, t)
    for key, value in pairs(t) do
        r = complete_sound(key, value)  -- completed data set
        sound.DefineSoundEffect(tablename, key, r.file, r.volume, r.loop,
            r.global, r.priority, r.damp_max, r.damp_inc, r.damp_mult,
            r.damp_min, r.damp_tick, r.silence_string)
    end
end

-- Activate a sound set by name
function ActivateSoundSet (name)
    sound.SetActiveSoundSet(name)
end

-- Compatibility with 1.00 sound.lua's
function Sound (t)  return t  end
AddSoundTable = AddSoundSet

---------------------------------
-- Definition of sound effects --
---------------------------------

------------------------
-- Enigma Sound table --
------------------------

-- Note that no "#" is allowed within a sound event name!

soundtable_enigma = {
    [""]           = "",        -- empty sound
    ballcollision  = "enigma/ballcollision",
    blackbomb      = "enigma/explosion1",
    blockerdown    = "",
    blockerup      = "",
    booze          = "",
    bumper         = "enigma/bumper",
    cloth          = { file="enigma/st-thud", damp_max = 4.0 },
    coinslotoff    = "",
    coinsloton     = "enigma/st-coinslot",
    crack          = "",
    doorclose      = "enigma/doorclose",  -- missing
    dooropen       = "enigma/dooropen",   -- missing
    drown          = "enigma/drown",
    dynamite       = "enigma/explosion2", -- replace?
    electric       = "enigma/st-stone",   -- replace
    exit           = { file="enigma/exit", global=true },  -- missing
    extinguish     = "",
    fakeoxyd       = { file="enigma/st-fakeoxyd", volume=0.3 },
    falldown       = "enigma/falldown",   -- missing, unused (falling is shatter!)
    fart           = "enigma/fart",
    finished       = { file="enigma/finished", global=true },  -- missing
    floordestroy   = "",
    fourswitch     = "enigma/st-switch", 
    glass          = "enigma/st-metal",   -- replace
    hitfloor       = "enigma/stone",      -- missing, used by it-vortex
    impulse        = "",
    intro          = { file="enigma/intro", global=true },  -- missing
    invrotate      = { file="enigma/invrotate", global=true },
    itemtransform  = "enigma/st-magic",
    jump           = "enigma/boink",
    jumppad        = "",
    landmine       = "enigma/explosion2",  -- replace?
    laserloop      = "",
    laseron        = { file="enigma/st-laser", damp_max = 20.0, damp_inc = 2.0 },
    laseroff       = "",
    lock           = "",
    magneton       = "",
    magnetoff      = "",
    mail           = "",
    menuexit        = { file="enigma/menuexit", global=true },
    menumove        = { file="enigma/menumove", global=true },
    menuok          = { file="enigma/menuok", global=true },
    menustop        = { file="enigma/menustop", global=true },
    menuswitch      = { file="enigma/menuswitch", global=true },
    metal          = "enigma/st-metal",
    mirrorturn     = "enigma/st-mirrorturn",
    movebig        = "enigma/st-move",
    moveslow       = "enigma/st-move",
    movesmall      = { file="enigma/st-move", damp_max = 10.0, damp_inc = 2.0 },
    oxydclose      = "enigma/st-oxydclose",  -- missing, not neccessary I think
    oxydopen       = { file="enigma/st-oxydopen", damp_max = 2000.0, damp_inc = 50.0 },
    oxydopened     = "enigma/st-oxydopened",
    pickup         = { file="enigma/pickup", global=true },
    puller         = "",
    puzzlerotate   = "enigma/st-move",
    rubberband     = "enigma/boing",
    scissors       = "",
    seedgrow       = "enigma/seedgrow",  -- missing
    shatter        = "enigma/shatter",
    shattersmall   = "enigma/shatter",   -- replace, "shattersmall" is missing
    shogunoff      = "",
    shogunon       = "",
    skull          = "",
    spade          = "",
    squish         = "",
    stone          = "enigma/st-stone",
    stonedestroy   = "enigma/explosion0",
    stonepaint     = "enigma/st-magic",
    stonetransform = "enigma/st-magic",
    swamp          = "enigma/swamped",
    switchmarbles  = "enigma/warp",      -- missing
    switchoff      = "",
    switchon       = "enigma/st-switch",
    switchplayer   = "enigma/switch",
    sword          = "",
    thief          = "enigma/thief",     -- missing
    triggerdown    = "enigma/it-triggerdown",
    triggerup      = "enigma/it-triggerup",
    turnstileleft  = "enigma/turnstileleft",  -- missing
    turnstileright = "enigma/turnstileright", -- missing
    umbrellaoff    = "",
    umbrellaon     = "",
    umbrellawarn   = "",
    unlock         = "enigma/unlock",    -- missing
    vortexclose    = "enigma/doorclose", -- missing
    vortexopen     = "enigma/dooropen",  -- missing
    warp           = "enigma/warp",      -- missing, maybe suck2?
    whitebomb      = "enigma/explosion1",
    wood           = "enigma/wood",
    yinyang        = "enigma/st-magic",
}

soundtable_silent = {
    [""]           = "",        -- empty sound
    ballcollision  = { silence_string = "tack" },
    blackbomb      = { silence_string = "KABOOM!" },
    blockerdown    = "",
    blockerup      = "",
    booze          = "",
    bumper         = { silence_string = "Doinc" },
    cloth          = { silence_string = "bump" },
    coinslotoff    = "",
    coinsloton     = { silence_string = "kachink" },
    crack          = { silence_string = "Crrr..." },
    doorclose      = "",
    dooropen       = "",
    drown          = { silence_string = "blupp" },
    dynamite       = { silence_string = "Boom!" },
    electric       = { silence_string = "cling" },
    exit           = "",
    extinguish     = { silence_string = "pfff..." },
    fakeoxyd       = { silence_string = "ding-dang" },
    falldown       = "",
    fart           = { silence_string = "Prfft!" },
    finished       = { silence_string = "Finished!" },
    floordestroy   = { silence_string = "CrraaACK!" },
    fourswitch     = { silence_string = "Clihick" }, 
    glass          = { silence_string = "Cliorr" },
    hitfloor       = { silence_string = "Bang" },
    impulse        = "",
    intro          = "",
    invrotate      = "",
    itemtransform  = { silence_string = "Tatam!" },
    jump           = { silence_string = "Doink" },
    jumppad        = { silence_string = "da-Doink" },
    landmine       = { silence_string = "click-BANG!" },
    laserloop      = "",
    laseron        = { silence_string = "wuuiim!" },
    laseroff       = "",
    lock           = "",
    magneton       = "",
    magnetoff      = "",
    mail           = "",
    menuexit       = "",
    menumove       = "",
    menuok         = "",
    menustop       = "",
    menuswitch     = "",
    metal          = { silence_string = "clang" },
    mirrorturn     = { silence_string = "wrrr" },
    movebig        = { silence_string = "swosh" },
    moveslow       = { silence_string = "swosh" },
    movesmall      = { silence_string = "swosh" },
    oxydclose      = "",
    oxydopen       = { silence_string = "da-Ding" },
    oxydopened     = { silence_string = "da-Dang!" },
    pickup         = { silence_string = "pick" },
    puller         = "",
    puzzlerotate   = { silence_string = "rot-rot" },
    rubberband     = { silence_string = "Boing!" },
    scissors       = "",
    seedgrow       = { silence_string = "krk...krk" },
    shatter        = { silence_string = "CLIRR!!" },
    shattersmall   = { silence_string = "CLIRR!" },
    shogunoff      = "",
    shogunon       = "",
    skull          = "",
    spade          = "",
    squish         = "",
    stone          = { silence_string = "dopp" },
    stonedestroy   = { silence_string = "BANG!" },
    stonepaint     = { silence_string = "flatsh" },
    stonetransform = { silence_string = "Tradam!" },
    swamp          = { silence_string = "gulp" },
    switchmarbles  = "",
    switchoff      = "",
    switchon       = { silence_string = "Click" },
    switchplayer   = { silence_string = "switch-switch" },
    sword          = "",
    thief          = { silence_string = "hehee!" },
    triggerdown    = { silence_string = "ca-lik" },
    triggerup      = { silence_string = "ca-lak" },
    turnstileleft  = "",
    turnstileright = "",
    umbrellaoff    = "",
    umbrellaon     = "",
    umbrellawarn   = "",
    unlock         = "",
    vortexclose    = "",
    vortexopen     = "",
    warp           = "",
    whitebomb      = { silence_string = "KABOOOOM!!" },
    wood           = { silence_string = "dok" },
    yinyang        = { silence_string = "Tradim!" },
}

soundtable_oxyd = {
    [""]           = "",        -- empty sound
    ballcollision  = "OXKLICK3.SDD",
    blackbomb      = "OXCRASH2.SDD",
    blockerdown    = "",
    blockerup      = "",
    booze          = "",
    bumper         = "OXWOUOU.SDD",
    cloth          = "OXKLICK4.SDD",
    coinslotoff    = "",
    coinsloton     = "OXMONEY.SDD",
    crack          = { file="OXCRACK.SDD", volume=0.7 },
    doorclose      = "OXMOTOR.SDD",
    dooropen       = "OXMOTOR.SDD",
    drown          = "OXBLOOP.SDD",
    dynamite       = "OXCRASH1.SDD",
    electric       = "OXKLICK6.SDD",
    exit           = { file="OXEXIT.SDD", global=true },
    extinguish     = "",
    fakeoxyd       = "",
    falldown       = "",
    fart           = "OXUNTITL.SDD",
    finished       = { file="OXFINITO.SDD", global=true },
    floordestroy   = "OXCRASH2",
    fourswitch     = "",
    glass          = "OXKLICK1.SDD",
    hitfloor       = "OXKLICK1.SDD",
    impulse        = "OXWOUOU.SDD",
    intro          = { file="OXINTRO.SDD", global=true },
    invrotate      = { file="OXINVROT.SDD", global=true },
    itemtransform  = "OXMAGIC1.SDD",
    jump           = "OXJUMP.SDD",
    jumppad        = "",
    landmine       = "explosion1",
    laserloop      = "",
    laseron        = "OXLASER.SDD",
    laseroff       = "",
    lock           = "",
    magneton       = "",
    magnetoff      = "",
    mail           = "",
    metal          = "OXKLICK2.SDD",
    mirrorturn     = "OXTURN.SDD",
    movebig        = "OXMOVE.SDD",
    moveslow       = "OXMOVE.SDD",
    movesmall      = "OXMOVE.SDD",
    oxydclose      = "OXMEMCL.SDD",
    oxydopen       = "OXMEMOP.SDD",
    oxydopened     = "OXMEMOK.SDD",
    pickup         = "OXINVENT.SDD",
    puller         = "OXPULLER.SDD",
    puzzlerotate   = "OXMOVE.SDD",
    rubberband     = "OXBOING.SDD",
    scissors       = "OXCUT.SDD",
    seedgrow       = "",
    shatter        = "OXKLIRR.SDD",
    shattersmall   = "OXKLIRR.SDD",
    shogunoff      = "",
    shogunon       = "",
    skull          = "",
    spade          = "",
    squish         = "OXMATSCH.SDD",
    stone          = "OXKLICK1.SDD",
    stonedestroy   = "OXCRASH2.SDD",
    stonepaint     = "OXMAGIC.SDD",
    stonetransform = "OXMAGIC.SDD",
    swamp          = "OXBLOOP.SDD",
    switchmarbles  = "",
    switchoff      = "",
    switchon       = "",
    switchplayer   = "",
    sword          = "",
    thief          = "OXTHIEF.SDD",
    triggerdown    = "OXSWON.SDD",
    triggerup      = "OXSWOFF.SDD",
    turnstileleft  = "OXMAGIC4.SDD",
    turnstileright = "OXMAGIC3.SDD",
    umbrellaoff    = "",
    umbrellaon     = "",
    umbrellawarn   = "",
    unlock         = "",
    vortexclose    = "OXMOTOR.SDD",
    vortexopen     = "OXMOTOR.SDD",
    warp           = "OXTRANS.SDD",
    whitebomb      = "OXCRASH2.SDD",
    wood           = "OXKLICK1.SDD",
    yinyang        = "OXMAGIC.SDD",
}    
copy_missing (soundtable_enigma, soundtable_oxyd)

soundtable_oxm = {
    turnstileleft = "OXTURNL.SDD",
    turnstileright = "OXTURNL.SDD"
}
copy_missing (soundtable_oxyd, soundtable_oxm)

--------------------------------------
-- Register predefined sound tables --
--------------------------------------

AddSoundSet ("Oxyd*", soundtable_oxyd)
AddSoundSet ("Magnum*", soundtable_oxm)
--AddSoundSet ("Silent", soundtable_silent)
AddSoundSet ("Enigma", soundtable_enigma)

ActivateSoundSet ("Enigma")


