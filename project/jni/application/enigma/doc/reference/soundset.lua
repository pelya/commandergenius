--
-- This file demonstrates how to add a sound set to Enigma.
--
-- Sound effects are triggered by so-called "sound events". These sound
-- events usually have a name (like "dooropen") and an associated location
-- (the coordinates of the door) which affects the way a sound effect is
-- played.
--
-- The sound event is converted into a real sound effect using tables
-- similar to the one below.  Each entry in the table is either a string
-- like "enigma/st-coinslot", which is interpreted as the file
-- "soundsets/enigma/st-coinslot.wav" with some default properties, or a
-- list of sound attributes enclosed in curly braces "{ ... }".
--
-- Here is a complete example of such an attribute list:
--
--      dooropen = { file="my_soundset/open-door", volume=0.9, priority=4 },
--
-- The meaning of these attributes is as follows:
--
--      file     - Path and name of the sound file for this event, without
--                 the".wav" extension.
--
--      volume   - The sound volume: 1.0 is standard, 0.0 is silent.
--
--      priority - If many effects are active at the same time, high-priority
--                 effects can replace lower-priority effects. Use an integer
--                 between 1 and 10 (default 1). This property does not yet
--                 work within Enigma 1.01.
--
--      global   - Either "true" or "false".  If true, no stereo effects are
--                 applied and there is no attenuation.  Used for menu sound,
--                 level end sounds, etc. Default is "false".
--
--      loop     - "true" or "false".  If true, the sound repeats infinitely
--                 until canceled. Default is "false". 
--
--      damp_max, damp_inc, damp_mult, damp_min, damp_tick
--               - Parameters for sound damping.  Sounds from noisy objects
--                 like light passengers are damped to reduce the noise.
--                 For this, the sound event's frequency is estimated.
--                 damp_max calibrates the maximal damping factor (high means
--                 quiet), damp_inc how fast the damping accumulates,
--                 damp_mult is an overall factor, damp_min defines a lower
--                 bound for the damping entries (beyond which they are
--                 removed from memory) and damp_tick the factor that's
--                 applied all 0.1 seconds. See sound.hh for details.
--                 Defaults: 10.0, 1.0, 1.0, 0.5, 0.9.
--
-- To design a new sound set, proceed as follows.
--
-- 1) Create a new folder containing this file (named "soundset.lua")
--    and the wav files you want to use.  
--
-- 2) Move this new folder into Enigma's "soundsets" folder in your user path.
--    (Possibly you have to create it.) The directory structure should look
--    something like this:
--
--      (user path)/soundsets/my_sounds/
--                                     /soundset.lua
--                                     /a.wav
--                                     /b.wav
--                                     ...
--
-- 3) Run Enigma.  Since this file's sound set does not map any sound effect
--    to a wav file, you should hear nothing.
--
-- 4) Edit the contents of this file to your liking.  You can access the
--    default sound files in the "soundsets/enigma" directory, e.g.:
--        ...
--        coinsloton = { file="enigma/st-coinslot" },
--        ...
--    When using own sound files, remember to add the subfolder, like in
--        ...
--        coinsloton = { file="my_sounds/b" },
--        ...
--    No extension ".wav"! It's added automatically. Make sure that the
--    extension is in lower case letters.
--
-- 5) Replace "MY_SOUNDSET" by a suitable variable name, and "My Soundset"
--    by the name you want to see in the sound options menu.  Remember to
--    make it short enough to fit on the button.
--
-- If you need inspiration, take a look at "sound-defaults.lua" shipped with
-- Enigma, which contains the default sound table.
--
-- If you have questions, don't hesitate to ask.  Have fun!
--

soundtable_MY_SOUNDSET = {
    [""]           = "",        -- empty sound
    ballcollision  = "",
    blackbomb      = "",
    blockerdown    = "",
    blockerup      = "",
    booze          = "",
    bumper         = "",
    cloth          = "",
    coinslotoff    = "",
    coinsloton     = "",
    crack          = "",
    doorclose      = "",
    dooropen       = "",
    drown          = "",
    dynamite       = "",
    electric       = "",
    exit           = "",
    extinguish     = "",
    fakeoxyd       = "",
    falldown       = "",
    fart           = "",
    finished       = "",
    floordestroy   = "",
    fourswitch     = "",
    glass          = "",
    hitfloor       = "",
    impulse        = "",
    intro          = "",
    invrotate      = "",
    itemtransform  = "",
    jump           = "",
    jumppad        = "",
    landmine       = "",
    laserloop      = "",
    laseron        = "",
    laseroff       = "",
    lock           = "",
    magneton       = "",
    magnetoff      = "",
    mail           = "",
    menuexit        = "",
    menumove        = "",
    menuok          = "",
    menustop        = "",
    menuswitch      = "",
    metal          = "",
    mirrorturn     = "",
    movebig        = "",
    moveslow       = "",
    movesmall      = "",
    oxydclose      = "",
    oxydopen       = "",
    oxydopened     = "",
    pickup         = "",
    puller         = "",
    puzzlerotate   = "",
    rubberband     = "",
    scissors       = "",
    seedgrow       = "",
    shatter        = "",
    shattersmall   = "",
    shogunoff      = "",
    shogunon       = "",
    skull          = "",
    spade          = "",
    squish         = "",
    stone          = "",
    stonedestroy   = "",
    stonepaint     = "",
    stonetransform = "",
    swamp          = "",
    switchmarbles  = "",
    switchoff      = "",
    switchon       = "",
    switchplayer   = "",
    sword          = "",
    thief          = "",
    triggerdown    = "",
    triggerup      = "",
    turnstileleft  = "",
    turnstileright = "",
    umbrellaoff    = "",
    umbrellaon     = "",
    umbrellawarn   = "",
    unlock         = "",
    vortexclose    = "",
    vortexopen     = "",
    warp           = "",
    whitebomb      = "",
    wood           = "",
    yinyang        = "",
}

-- Remove "--" from the line below to add missing sound entries
-- from the default sound set.
--copy_missing (soundtable_enigma, soundtable_MY_SOUNDSET)

AddSoundSet ("My Soundset", soundtable_MY_SOUNDSET)
