/*
 * Copyright (C) May 2003 by Ralf Westram <amgine@rallysoft.de>
 * Copyright (C) 2004 Daniel Heck
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

#if defined(PLAIN_SPEC_ONLY)
// This is used from lib-src/oxydlib/EnigmaNames.cpp.
//
// defining PLAIN_SPEC_ONLY only shows xxx_floor_map, xxx_stone_map and xxx_item_map
// Note:  the xxx_item_map changes it's type (ItemID -> const char *)

#define ITEMSPEC(i) #i
#define ITEMMAPTYPE const char *

#else
// Standard enigma section :

#define ITEMSPEC(i) i
#define ITEMMAPTYPE world::ItemID

#include "oxyd_internal.hh"
using namespace world;

#endif // PLAIN_SPEC_ONLY
// only tables following!

const char *oxyd::oxydmag_floor_map[256] = {
    "fl-abyss",                 // OxydMagnum floor 0x00
    "fl-gray",                  // OxydMagnum floor 0x01 (common was 'fl-gray')
    "fl-metal",                 // OxydMagnum floor 0x02 (This should be "fl-stwood" for #112)
    "fl-metal3",                // OxydMagnum floor 0x03
    "fl-metal4",                // OxydMagnum floor 0x04
    0,                          // OxydMagnum floor 0x05 (common was 'fl-metal')(level 121 ?)
    "fl-metal5",                // OxydMagnum floor 0x06
    "fl-normal",                // OxydMagnum floor 0x07 (common was 'fl-metal')
    "fl-tigris",                // OxydMagnum floor 0x08 (common was 'fl-rough')
    "fl-tigris",                // OxydMagnum floor 0x09 (common was 'fl-normal')
    "fl-tigris",                // OxydMagnum floor 0x0a (common was 'fl-normal')
    "fl-tigris",                // OxydMagnum floor 0x0b (common was 'fl-stone')
    "fl-inverse",               // OxydMagnum floor 0x0c (common was 'fl-inverse')
    "fl-acblack",               // OxydMagnum floor 0x0d
    "fl-acwhite",               // OxydMagnum floor 0x0e
    "fl-swamp",                 // OxydMagnum floor 0x0f
    UNUSED,                     // OxydMagnum floor 0x10
    UNUSED,                     // OxydMagnum floor 0x11
    UNUSED,                     // OxydMagnum floor 0x12
    "fl-water",                 // OxydMagnum floor 0x13
    UNUSED,                     // OxydMagnum floor 0x14
    UNUSED,                     // OxydMagnum floor 0x15
    UNUSED,                     // OxydMagnum floor 0x16
    "fl-ice",                   // OxydMagnum floor 0x17
    UNUSED,                     // OxydMagnum floor 0x18
    UNUSED,                     // OxydMagnum floor 0x19
    UNUSED,                     // OxydMagnum floor 0x1a
    UNUSED,                     // OxydMagnum floor 0x1b
    "fl-space",                 // OxydMagnum floor 0x1c
    UNUSED,                     // OxydMagnum floor 0x1d
    UNUSED,                     // OxydMagnum floor 0x1e
    UNUSED,                     // OxydMagnum floor 0x1f
    UNUSED,                     // OxydMagnum floor 0x20
    UNUSED,                     // OxydMagnum floor 0x21
    UNUSED,                     // OxydMagnum floor 0x22
    "fl-gradient1",             // OxydMagnum floor 0x23
    "fl-gradient2",             // OxydMagnum floor 0x24
    "fl-gradient3",             // OxydMagnum floor 0x25
    "fl-gradient4",             // OxydMagnum floor 0x26
    "fl-gradient5",             // OxydMagnum floor 0x27
    "fl-gradient6",             // OxydMagnum floor 0x28
    "fl-gradient7",             // OxydMagnum floor 0x29
    "fl-gradient8",             // OxydMagnum floor 0x2a
    "fl-gradient9",             // OxydMagnum floor 0x2b
    "fl-gradient10",            // OxydMagnum floor 0x2c
    "fl-gradient11",            // OxydMagnum floor 0x2d
    "fl-gradient12",            // OxydMagnum floor 0x2e
    "fl-gradient13",            // OxydMagnum floor 0x2f
    "fl-gradient14",            // OxydMagnum floor 0x30
    "fl-gradient15",            // OxydMagnum floor 0x31
    "fl-gradient16",            // OxydMagnum floor 0x32
    "fl-brick",                 // OxydMagnum floor 0x33 (common was 'fl-rough')
    "fl-brick",                 // OxydMagnum floor 0x34 (common was 'fl-rough')
    UNUSED,                     // OxydMagnum floor 0x35
    UNUSED,                     // OxydMagnum floor 0x36
    "fl-brick",                 // OxydMagnum floor 0x37
    "fl-brick",                 // OxydMagnum floor 0x38
    UNUSED,                     // OxydMagnum floor 0x39
    UNUSED,                     // OxydMagnum floor 0x3a
    UNUSED,                     // OxydMagnum floor 0x3b
    "fl-rock",                  // OxydMagnum floor 0x3c
    UNUSED,                     // OxydMagnum floor 0x3d
    UNUSED,                     // OxydMagnum floor 0x3e
    0,                          // OxydMagnum floor 0x3f (Level 121 ?)(in normal Oxyd Magnum Levels unused!)
    "fl-stwood1",               // OxydMagnum floor 0x40
    "fl-wood",                  // OxydMagnum floor 0x41
    "fl-wood1",                 // OxydMagnum floor 0x42
    UNUSED,                     // OxydMagnum floor 0x43
    UNUSED,                     // OxydMagnum floor 0x44
    UNUSED,                     // OxydMagnum floor 0x45
    "fl-gravel",                // OxydMagnum floor 0x46
    "fl-gravel",                // OxydMagnum floor 0x47
    UNUSED,                     // OxydMagnum floor 0x48
    UNUSED,                     // OxydMagnum floor 0x49
    UNUSED,                     // OxydMagnum floor 0x4a
    UNUSED,                     // OxydMagnum floor 0x4b
    "fl-stone",                 // OxydMagnum floor 0x4c
    "fl-stone",                 // OxydMagnum floor 0x4d
    UNUSED,                     // OxydMagnum floor 0x4e
    UNUSED,                     // OxydMagnum floor 0x4f
    UNUSED,                     // OxydMagnum floor 0x50
    UNUSED,                     // OxydMagnum floor 0x51
    "fl-bridge",                // OxydMagnum floor 0x52 (common was 'fl-bridge')
    "fl-springboard",           // OxydMagnum floor 0x53
    "fl-light",                 // OxydMagnum floor 0x54
    "fl-lightgray",             // OxydMagnum floor 0x55
    "fl-darkgray",              // OxydMagnum floor 0x56
    UNUSED,                     // OxydMagnum floor 0x57
    "fl-darkgray",              // OxydMagnum floor 0x58
    "fl-leaves",                // OxydMagnum floor 0x59
    "fl-normal",                // OxydMagnum floor 0x5a (Only Oxyd Magnum Gold #119,120)
    "fl-plank",                 // OxydMagnum floor 0x5b
    // codes >= 0x5c are unused
};

const char *oxyd::oxydmag_stone_map[256] = {
    0,                          // OxydMagnum stone 0x00
    0,0,0,0,0,0,0,0,            // 0x01 -- 0x08  Oxyd stones
    0,0,0,0,0,0,0,0,            // 0x09 -- 0x10  Oxyd stones
    "st-likeoxydd",             // OxydMagnum stone 0x11
    "st-plain",                 // OxydMagnum stone 0x12
    "st-bigbrick-es",           // OxydMagnum stone 0x13
    "st-bigbrick-sw",           // OxydMagnum stone 0x14
    "st-bigbrick-ne",           // OxydMagnum stone 0x15
    "st-bigbrick-nw",           // OxydMagnum stone 0x16
    UNUSED,                     // OxydMagnum stone 0x17
    UNUSED,                     // OxydMagnum stone 0x18
    "st-glass1_hole",           // OxydMagnum stone 0x19
    "st-grate1",                // OxydMagnum stone 0x1a
    "st-grate2",                // OxydMagnum stone 0x1b
    "st-bug",                   // OxydMagnum stone 0x1c
    UNUSED,                     // OxydMagnum stone 0x1d
    UNUSED,                     // OxydMagnum stone 0x1e
    "st-brick",                 // OxydMagnum stone 0x1f
    "st-rock1",                 // OxydMagnum stone 0x20
    "st-rock1",                 // OxydMagnum stone 0x21 (Level 121 ?)(Oxyd Magnum Gold)
    "st-rock1",                 // OxydMagnum stone 0x22 (common was 'st-glass')(Level 73 only)(looks like st-rock1)(looks like 0x20, but not like 0x87)
    "st-woven",                 // OxydMagnum stone 0x23
    "st-marble",                // OxydMagnum stone 0x24
    "st-glass1",                // OxydMagnum stone 0x25
    "st-glass3",                // OxydMagnum stone 0x26
    "st-glass1",                // OxydMagnum stone 0x27
    "st-plain_break",           // OxydMagnum stone 0x28
    "st-plain_hole",            // OxydMagnum stone 0x29
    "st-plain_move",            // OxydMagnum stone 0x2a
    "st-wood",                  // OxydMagnum stone 0x2b
    "st-switch",                // OxydMagnum stone 0x2c
    "st-floppy",                // OxydMagnum stone 0x2d (in normal Oxyd Magnum Levels unused!)
    UNUSED,                     // OxydMagnum stone 0x2e
    "st-fourswitch",            // OxydMagnum stone 0x2f (Level 89 and 92)
    "st-key",                   // OxydMagnum stone 0x30
    "st-floppy",                // OxydMagnum stone 0x31
    "st-knight",                // OxydMagnum stone 0x32
    "st-fourswitch",            // OxydMagnum stone 0x33 (Level 51 and 100)
    "st-death",                 // OxydMagnum stone 0x34
    "st-death_invisible",       // OxydMagnum stone 0x35
    "st-oneway_black-w",        // OxydMagnum stone 0x36
    "st-oneway_black-e",        // OxydMagnum stone 0x37
    "st-oneway_black-n",        // OxydMagnum stone 0x38
    "st-oneway_black-s",        // OxydMagnum stone 0x39
    "st-oneway_white-w",        // OxydMagnum stone 0x3a
    "st-oneway_white-e",        // OxydMagnum stone 0x3b
    "st-oneway_white-n",        // OxydMagnum stone 0x3c
    "st-oneway_white-s",        // OxydMagnum stone 0x3d
    UNUSED,                     // OxydMagnum stone 0x3e
    UNUSED,                     // OxydMagnum stone 0x3f
    UNUSED,                     // OxydMagnum stone 0x40
    UNUSED,                     // OxydMagnum stone 0x41
    "st-actorimpulse",          // OxydMagnum stone 0x42
    "st-actorimpulse_invisible", // OxydMagnum stone 0x43
    "st-laser-1",               // OxydMagnum stone 0x44  The laser-names are fake names!
    "st-laser-2",               // OxydMagnum stone 0x45  Direction and state are generated by Enigma.
    "st-laser-3",               // OxydMagnum stone 0x46
    "st-mirror-p|",             // OxydMagnum stone 0x47
    "st-mirror-p/",             // OxydMagnum stone 0x48
    "st-mirror-p-",             // OxydMagnum stone 0x49
    "st-mirror-p\\",            // OxydMagnum stone 0x4a
    "st-mirror-p|m",            // OxydMagnum stone 0x4b
    "st-mirror-p/m",            // OxydMagnum stone 0x4c
    "st-mirror-p-m",            // OxydMagnum stone 0x4d
    "st-mirror-p\\m",           // OxydMagnum stone 0x4e
    "st-mirror-p|t",            // OxydMagnum stone 0x4f
    "st-mirror-p/t",            // OxydMagnum stone 0x50
    "st-mirror-p-t",            // OxydMagnum stone 0x51
    "st-mirror-p\\t",           // OxydMagnum stone 0x52
    "st-mirror-3>",             // OxydMagnum stone 0x53
    "st-mirror-3^",             // OxydMagnum stone 0x54
    "st-mirror-3<",             // OxydMagnum stone 0x55
    "st-mirror-3v",             // OxydMagnum stone 0x56
    "st-puzzle2-es",            // Oxyd1 stone 0x57
    "st-puzzle2-sw",            // Oxyd1 stone 0x58
    "st-puzzle2-nw",            // Oxyd1 stone 0x59
    "st-puzzle2-ne",            // Oxyd1 stone 0x5a
    "st-puzzle2-ew",            // Oxyd1 stone 0x5b
    "st-puzzle2-ns",            // Oxyd1 stone 0x5c
    "st-puzzle2-n",             // Oxyd1 stone 0x5d
    "st-puzzle2-s",             // Oxyd1 stone 0x5e
    "st-puzzle2-w",             // Oxyd1 stone 0x5f
    "st-puzzle2-e",             // Oxyd1 stone 0x60
    "st-puzzle2-nesw",          // Oxyd1 stone 0x61
    "st-metal_hole",            // OxydMagnum stone 0x62  (st-explosive: like st-metal, but explodes when touched ...)
    "st-plain_break",           // OxydMagnum stone 0x63
    "st-fakeoxyda",             // OxydMagnum stone 0x64
    UNUSED,                     // OxydMagnum stone 0x65
    UNUSED,                     // OxydMagnum stone 0x66
    UNUSED,                     // OxydMagnum stone 0x67
    "st-bombs",                 // OxydMagnum stone 0x68 (common was 'st-shogun-l')
    "st-flash",                 // OxydMagnum stone 0x69
    "st-coinslot",              // OxydMagnum stone 0x6a
    "st-thief",                 // OxydMagnum stone 0x6b
    "st-shogun-s",              // OxydMagnum stone 0x6c
    "st-stoneimpulse",          // OxydMagnum stone 0x6d
    "st-lasertimeswitch",       // OxydMagnum stone 0x6e
    "st-mail-n",                // OxydMagnum stone 0x6f
    "st-mail-w",                // OxydMagnum stone 0x70
    "st-mail-e",                // OxydMagnum stone 0x71
    "st-mail-s",                // OxydMagnum stone 0x72
    "st-door-h",                // OxydMagnum stone 0x73
    "st-door-v",                // OxydMagnum stone 0x74
    "st-metal",                 // OxydMagnum stone 0x75
    "st-invisible",             // OxydMagnum stone 0x76
    UNUSED,                     // OxydMagnum stone 0x77
    "st-door-v-open",           // OxydMagnum stone 0x78 (st-door-h-open was wrong, look at level #32)
    UNUSED,                     // OxydMagnum stone 0x79
    "st-timeswitch",            // OxydMagnum stone 0x7a (Level 66, this is a solid stone)
    UNUSED,                     // OxydMagnum stone 0x7b
    UNUSED,                     // OxydMagnum stone 0x7c
    UNUSED,                     // OxydMagnum stone 0x7d
    UNUSED,                     // OxydMagnum stone 0x7e
    UNUSED,                     // OxydMagnum stone 0x7f
    UNUSED,                     // OxydMagnum stone 0x80
    UNUSED,                     // OxydMagnum stone 0x81
    UNUSED,                     // OxydMagnum stone 0x82
    UNUSED,                     // OxydMagnum stone 0x83
    UNUSED,                     // OxydMagnum stone 0x84
    UNUSED,                     // OxydMagnum stone 0x85
    "st-rock3_hole",            // OxydMagnum stone 0x86
    "st-rock4",                 // OxydMagnum stone 0x87 (was st-rock1, but the pic has straight edges and surfaces)
    0,                          // OxydMagnum stone 0x88 (Level 121 ?)(in normal Oxyd Magnum Levels unused!)
    0,                          // OxydMagnum stone 0x89 (Level 121 ?)(in normal Oxyd Magnum Levels unused!)
    "st-stone1",                // OxydMagnum stone 0x8a
    "st-rock6",                 // OxydMagnum stone 0x8b
    "st-white1",                // OxydMagnum stone 0x8c
    "st-black1",                // OxydMagnum stone 0x8d
    "st-yinyang2",              // OxydMagnum stone 0x8e
    UNUSED,                     // OxydMagnum stone 0x8f
    "st-redrock",               // OxydMagnum stone 0x90 (Level 89 only)
    UNUSED,                     // OxydMagnum stone 0x91
    "st-fourswitch",            // OxydMagnum stone 0x92 (Level 51 only)
    "st-yinyang1",              // OxydMagnum stone 0x93
    "st-break_acwhite",         // OxydMagnum stone 0x94
    "st-break_acblack",         // OxydMagnum stone 0x95
    "st-chargeplus",            // OxydMagnum stone 0x96
    "st-chargeminus",           // OxydMagnum stone 0x97
    "st-chargezero",            // OxydMagnum stone 0x98
    0,                          // OxydMagnum stone 0x99 (common was 'st-blocker')(Level 121 ?)(in normal Oxyd Magnum Levels unused!)
    "st-turnstile",             // OxydMagnum stone 0x9a
    "st-turnstile-n",           // OxydMagnum stone 0x9b
    "st-turnstile-s",           // OxydMagnum stone 0x9c
    "st-turnstile-w",           // OxydMagnum stone 0x9d
    "st-turnstile-e",           // OxydMagnum stone 0x9e
    UNUSED,                     // OxydMagnum stone 0x9f
    UNUSED,                     // OxydMagnum stone 0xa0
    UNUSED,                     // OxydMagnum stone 0xa1
    UNUSED,                     // OxydMagnum stone 0xa2
    UNUSED,                     // OxydMagnum stone 0xa3
    "st-stoneimpulse-hollow",   // OxydMagnum stone 0xa4
    UNUSED,                     // OxydMagnum stone 0xa5
    UNUSED,                     // OxydMagnum stone 0xa6
    "st-lightpassenger",        // OxydMagnum stone 0xa7
    UNUSED,                     // OxydMagnum stone 0xa8
    UNUSED,                     // OxydMagnum stone 0xa9
    // codes >= 0xaa are unused
};

ITEMMAPTYPE oxyd::oxydmag_item_map[256] = {
    ITEMSPEC(it_none),          // OxydMagnum item 0x00
    ITEMSPEC(it_extralife),     // OxydMagnum item 0x01
    ITEMSPEC(it_EXTERNAL),      // OxydMagnum item 0x02 document 1
    ITEMSPEC(it_EXTERNAL),      // OxydMagnum item 0x03 document 2
    ITEMSPEC(it_hammer),        // OxydMagnum item 0x04
    ITEMSPEC(it_coffee),        // OxydMagnum item 0x05
    ITEMSPEC(it_cherry),        // OxydMagnum item 0x06
    ITEMSPEC(it_umbrella),      // OxydMagnum item 0x07
    ITEMSPEC(it_glasses),       // OxydMagnum item 0x08
    ITEMSPEC(it_glasses_broken), // OxydMagnum item 0x09
    ITEMSPEC(it_dynamite),      // OxydMagnum item 0x0a
    ITEMSPEC(it_blackbomb),     // OxydMagnum item 0x0b
    ITEMSPEC(it_whitebomb),     // OxydMagnum item 0x0c
    ITEMSPEC(it_crack0),        // OxydMagnum item 0x0d
    ITEMSPEC(it_crack1),        // OxydMagnum item 0x0e
    ITEMSPEC(it_crack2),        // OxydMagnum item 0x0f
    ITEMSPEC(it_crack3),        // OxydMagnum item 0x10
    ITEMSPEC(it_coin1),         // OxydMagnum item 0x11
    ITEMSPEC(it_coin2),         // OxydMagnum item 0x12
    ITEMSPEC(it_coin4),         // OxydMagnum item 0x13
    ITEMSPEC(it_key_a),         // OxydMagnum item 0x14
    ITEMSPEC(it_key_b),         // OxydMagnum item 0x15
    ITEMSPEC(it_key_c),         // OxydMagnum item 0x16
    ITEMSPEC(it_floppy),        // OxydMagnum item 0x17
    ITEMSPEC(it_sword),         // OxydMagnum item 0x18
    ITEMSPEC(it_flagwhite),     // OxydMagnum item 0x19
    ITEMSPEC(it_flagblack),     // OxydMagnum item 0x1a
    ITEMSPEC(it_ring),          // OxydMagnum item 0x1b
    ITEMSPEC(it_pipe_wn),       // OxydMagnum item 0x1c
    ITEMSPEC(it_pipe_sw),       // OxydMagnum item 0x1d
    ITEMSPEC(it_pipe_ne),       // OxydMagnum item 0x1e
    ITEMSPEC(it_pipe_es),       // OxydMagnum item 0x1f
    ITEMSPEC(it_pipe_v),        // OxydMagnum item 0x20
    ITEMSPEC(it_pipe_h),        // OxydMagnum item 0x21
    ITEMSPEC(it_spade),         // OxydMagnum item 0x22
    ITEMSPEC(it_UNUSED),        // OxydMagnum item 0x23
    ITEMSPEC(it_pin),           // OxydMagnum item 0x24
    ITEMSPEC(it_seed),          // OxydMagnum item 0x25
    ITEMSPEC(it_spring2),       // OxydMagnum item 0x26
    ITEMSPEC(it_spring1),       // OxydMagnum item 0x27
    ITEMSPEC(it_bag),           // OxydMagnum item 0x28
    ITEMSPEC(it_magnet_off),    // OxydMagnum item 0x29
    ITEMSPEC(it_inversesensor), // OxydMagnum item 0x2a
    ITEMSPEC(it_sensor),        // OxydMagnum item 0x2b
    ITEMSPEC(it_shogun_s),      // OxydMagnum item 0x2c
    ITEMSPEC(it_vortex_open),   // OxydMagnum item 0x2d
    ITEMSPEC(it_vortex_closed), // OxydMagnum item 0x2e
    ITEMSPEC(it_wormhole_on),   // OxydMagnum item 0x2f
    ITEMSPEC(it_hill),          // OxydMagnum item 0x30
    ITEMSPEC(it_tinyhill),      // OxydMagnum item 0x31
    ITEMSPEC(it_hollow),        // OxydMagnum item 0x32
    ITEMSPEC(it_tinyhollow),    // OxydMagnum item 0x33
    ITEMSPEC(it_vstrip),        // OxydMagnum item 0x34
    ITEMSPEC(it_hstrip),        // OxydMagnum item 0x35
    ITEMSPEC(it_springboard),   // OxydMagnum item 0x36
    ITEMSPEC(it_MISSING),       // OxydMagnum item 0x37
    ITEMSPEC(it_bridge_oxyd),   // OxydMagnum item 0x38
    ITEMSPEC(it_UNUSED),        // OxydMagnum item 0x39
    ITEMSPEC(it_UNUSED),        // OxydMagnum item 0x3a
    ITEMSPEC(it_UNUSED),        // OxydMagnum item 0x3b
    ITEMSPEC(it_UNUSED),        // OxydMagnum item 0x3c
    ITEMSPEC(it_UNUSED),        // OxydMagnum item 0x3d
    ITEMSPEC(it_UNUSED),        // OxydMagnum item 0x3e
    ITEMSPEC(it_UNUSED),        // OxydMagnum item 0x3f
    ITEMSPEC(it_UNUSED),        // OxydMagnum item 0x40
    ITEMSPEC(it_UNUSED),        // OxydMagnum item 0x41
    ITEMSPEC(it_UNUSED),        // OxydMagnum item 0x42
    ITEMSPEC(it_UNUSED),        // OxydMagnum item 0x43
    ITEMSPEC(it_UNUSED),        // OxydMagnum item 0x44
    ITEMSPEC(it_UNUSED),        // OxydMagnum item 0x45
    ITEMSPEC(it_UNUSED),        // OxydMagnum item 0x46
    ITEMSPEC(it_UNUSED),        // OxydMagnum item 0x47
    ITEMSPEC(it_UNUSED),        // OxydMagnum item 0x48
    ITEMSPEC(it_UNUSED),        // OxydMagnum item 0x49
    ITEMSPEC(it_UNUSED),        // OxydMagnum item 0x4a
    ITEMSPEC(it_UNUSED),        // OxydMagnum item 0x4b
    ITEMSPEC(it_springboard),   // OxydMagnum item 0x4c
    ITEMSPEC(it_UNUSED),        // OxydMagnum item 0x4d
    ITEMSPEC(it_UNUSED),        // OxydMagnum item 0x4e
    ITEMSPEC(it_UNUSED),        // OxydMagnum item 0x4f
    ITEMSPEC(it_UNUSED),        // OxydMagnum item 0x50
    ITEMSPEC(it_UNUSED),        // OxydMagnum item 0x51
    ITEMSPEC(it_UNUSED),        // OxydMagnum item 0x52
    ITEMSPEC(it_UNUSED),        // OxydMagnum item 0x53
    ITEMSPEC(it_UNUSED),        // OxydMagnum item 0x54
    ITEMSPEC(it_UNUSED),        // OxydMagnum item 0x55
    ITEMSPEC(it_UNUSED),        // OxydMagnum item 0x56
    ITEMSPEC(it_UNUSED),        // OxydMagnum item 0x57
    ITEMSPEC(it_UNUSED),        // OxydMagnum item 0x58
    ITEMSPEC(it_UNUSED),        // OxydMagnum item 0x59
    ITEMSPEC(it_UNUSED),        // OxydMagnum item 0x5a
    ITEMSPEC(it_UNUSED),        // OxydMagnum item 0x5b
    ITEMSPEC(it_UNUSED),        // OxydMagnum item 0x5c
    ITEMSPEC(it_UNUSED),        // OxydMagnum item 0x5d
    ITEMSPEC(it_UNUSED),        // OxydMagnum item 0x5e
    ITEMSPEC(it_oxyd5f),        // OxydMagnum item 0x5f
    ITEMSPEC(it_drop),          // OxydMagnum item 0x60 (drunk)
    ITEMSPEC(it_UNUSED),        // OxydMagnum item 0x61 (rev. breaking area)
    ITEMSPEC(it_UNUSED),        // OxydMagnum item 0x62 (player exchange)
    ITEMSPEC(it_trigger),       // OxydMagnum item 0x63
    ITEMSPEC(it_puller_n),      // OxydMagnum item 0x64
    ITEMSPEC(it_puller_s),      // OxydMagnum item 0x65
    ITEMSPEC(it_puller_w),      // OxydMagnum item 0x66
    ITEMSPEC(it_puller_e),      // OxydMagnum item 0x67
    // codes >= 0x68 are unused
};
