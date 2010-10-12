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

const char *oxyd::oxydextra_floor_map[256] = {
    "fl-abyss",          // OxydExtra floor 0x00
    "fl-gray",           // OxydExtra floor 0x01
    "fl-metal",          // OxydExtra floor 0x02
    UNUSED,              // OxydExtra floor 0x03
    UNUSED,              // OxydExtra floor 0x04
    UNUSED,              // OxydExtra floor 0x05
    UNUSED,              // OxydExtra floor 0x06
    "fl-light",          // OxydExtra floor 0x07
    "fl-rough-red",      // OxydExtra floor 0x08
    "fl-rough-blue",     // OxydExtra floor 0x09
    "fl-rough-blue",     // OxydExtra floor 0x0a
    "fl-rough-red",      // OxydExtra floor 0x0b
    "fl-inverse",        // OxydExtra floor 0x0c
    "fl-black",          // OxydExtra floor 0x0d
    UNUSED,              // OxydExtra floor 0x0e
    UNUSED,              // OxydExtra floor 0x0f
    UNUSED,              // OxydExtra floor 0x10
    UNUSED,              // OxydExtra floor 0x11
    UNUSED,              // OxydExtra floor 0x12
    "fl-water",          // OxydExtra floor 0x13
    UNUSED,              // OxydExtra floor 0x14
    UNUSED,              // OxydExtra floor 0x15
    UNUSED,              // OxydExtra floor 0x16
    "fl-ice",            // OxydExtra floor 0x17
    UNUSED,              // OxydExtra floor 0x18
    UNUSED,              // OxydExtra floor 0x19
    UNUSED,              // OxydExtra floor 0x1a
    UNUSED,              // OxydExtra floor 0x1b
    "fl-space",          // OxydExtra floor 0x1c
    UNUSED,              // OxydExtra floor 0x1d
    UNUSED,              // OxydExtra floor 0x1e
    UNUSED,              // OxydExtra floor 0x1f
    UNUSED,              // OxydExtra floor 0x20
    UNUSED,              // OxydExtra floor 0x21
    UNUSED,              // OxydExtra floor 0x22
    UNUSED,              // OxydExtra floor 0x23
    UNUSED,              // OxydExtra floor 0x24
    "fl-gradient3",      // OxydExtra floor 0x25
    "fl-gradient4",      // OxydExtra floor 0x26
    UNUSED,              // OxydExtra floor 0x27
    UNUSED,              // OxydExtra floor 0x28
    UNUSED,              // OxydExtra floor 0x29
    UNUSED,              // OxydExtra floor 0x2a
    UNUSED,              // OxydExtra floor 0x2b
    UNUSED,              // OxydExtra floor 0x2c
    UNUSED,              // OxydExtra floor 0x2d
    UNUSED,              // OxydExtra floor 0x2e
    UNUSED,              // OxydExtra floor 0x2f
    UNUSED,              // OxydExtra floor 0x30
    "fl-gradient15",     // OxydExtra floor 0x31
    "fl-gradient16",     // OxydExtra floor 0x32
    UNUSED,              // OxydExtra floor 0x33
    "fl-rough",          // OxydExtra floor 0x34
    UNUSED,              // OxydExtra floor 0x35
    UNUSED,              // OxydExtra floor 0x36
    UNUSED,              // OxydExtra floor 0x37
    "fl-mortar",         // OxydExtra floor 0x38
    UNUSED,              // OxydExtra floor 0x39
    UNUSED,              // OxydExtra floor 0x3a
    UNUSED,              // OxydExtra floor 0x3b
    "fl-samba",          // OxydExtra floor 0x3c
    UNUSED,              // OxydExtra floor 0x3d
    UNUSED,              // OxydExtra floor 0x3e
    UNUSED,              // OxydExtra floor 0x3f
    UNUSED,              // OxydExtra floor 0x40
    "fl-samba",          // OxydExtra floor 0x41
    UNUSED,              // OxydExtra floor 0x42
    "fl-bumps",          // OxydExtra floor 0x43
    UNUSED,              // OxydExtra floor 0x44
    UNUSED,              // OxydExtra floor 0x45
    UNUSED,              // OxydExtra floor 0x46
    "fl-bumps",          // OxydExtra floor 0x47
    "fl-rock",           // OxydExtra floor 0x48
    UNUSED,              // OxydExtra floor 0x49
    UNUSED,              // OxydExtra floor 0x4a
    UNUSED,              // OxydExtra floor 0x4b
    "fl-bridge",         // OxydExtra floor 0x4c
    UNUSED,              // OxydExtra floor 0x4d
    UNUSED,              // OxydExtra floor 0x4e
    UNUSED,              // OxydExtra floor 0x4f
    UNUSED,              // OxydExtra floor 0x50
    UNUSED,              // OxydExtra floor 0x51
    UNUSED,              // OxydExtra floor 0x52
    UNUSED,              // OxydExtra floor 0x53
    "fl-gray",           // OxydExtra floor 0x54
    "fl-plank",          // OxydExtra floor 0x55
    UNUSED,              // OxydExtra floor 0x56
    UNUSED,              // OxydExtra floor 0x57
    UNUSED,              // OxydExtra floor 0x58
    "fl-trigger",        // OxydExtra floor 0x59
    UNUSED,              // OxydExtra floor 0x5a
    "fl-gravel",         // OxydExtra floor 0x5b
    // codes >= 0x5c are unused
};

const char *oxyd::oxydextra_stone_map[256] = {
    0,                   // no stone
    0, 0, 0, 0, 0, 0, 0, 0,     // oxyd stones
    0, 0, 0, 0, 0, 0, 0, 0,     // oxyd stones
    "st-fakeoxyd",       // OxydExtra stone 0x11
    "st-plain",          // OxydExtra stone 0x12
    "st-rock4",          // OxydExtra stone 0x13
    UNUSED,              // OxydExtra stone 0x14
    "st-rock5",          // OxydExtra stone 0x15
    "st-rock7",          // OxydExtra stone 0x16
    UNUSED,              // OxydExtra stone 0x17
    "st-grate1",         // OxydExtra stone 0x18
    UNUSED,              // OxydExtra stone 0x19
    UNUSED,              // OxydExtra stone 0x1a
    UNUSED,              // OxydExtra stone 0x1b
    "st-marble",         // OxydExtra stone 0x1c
    UNUSED,              // OxydExtra stone 0x1d
    UNUSED,              // OxydExtra stone 0x1e
    "st-rock5",          // OxydExtra stone 0x1f
    "st-stone2",         // OxydExtra stone 0x20
    "st-rock7",          // OxydExtra stone 0x21
    "st-glass1",         // OxydExtra stone 0x22
    UNUSED,              // OxydExtra stone 0x23
    "st-plain_break",    // OxydExtra stone 0x24
    "st-plain_hole",     // OxydExtra stone 0x25
    "st-plain_move",     // OxydExtra stone 0x26
    "st-wood",           // OxydExtra stone 0x27
    "st-switch",         // OxydExtra stone 0x28
    UNUSED,              // OxydExtra stone 0x29
    UNUSED,              // OxydExtra stone 0x2a
    UNUSED,              // OxydExtra stone 0x2b
    UNUSED,              // OxydExtra stone 0x2c
    "st-floppy",         // OxydExtra stone 0x2d
    UNUSED,              // OxydExtra stone 0x2e
    "st-death",          // OxydExtra stone 0x2f
    "st-death_invisible", // OxydExtra stone 0x30
    "st-oneway_black-w", // OxydExtra stone 0x31
    UNUSED,              // OxydExtra stone 0x32
    "st-oneway_black-n", // OxydExtra stone 0x33
    "st-oneway_black-s", // OxydExtra stone 0x34
    UNUSED,              // OxydExtra stone 0x35
    UNUSED,              // OxydExtra stone 0x36
    UNUSED,              // OxydExtra stone 0x37
    UNUSED,              // OxydExtra stone 0x38
    UNUSED,              // OxydExtra stone 0x39
    UNUSED,              // OxydExtra stone 0x3a
    UNUSED,              // OxydExtra stone 0x3b
    "st-actorimpulse",   // OxydExtra stone 0x3c
    UNUSED,              // OxydExtra stone 0x3d
    "st-laser-1",        // OxydExtra stone 0x3e  The laser-names are fake names!
    "st-laser-2",        // OxydExtra stone 0x3f  Direction and state are generated by Enigma.
    "st-laser-3",        // OxydExtra stone 0x40
    UNUSED,              // OxydExtra stone 0x41
    UNUSED,              // OxydExtra stone 0x42
    "st-mirror-p-",      // OxydExtra stone 0x43
    "st-mirror-p\\",     // OxydExtra stone 0x44
    UNUSED,              // OxydExtra stone 0x45
    "st-mirror-p/",      // OxydExtra stone 0x46
    UNUSED,              // OxydExtra stone 0x47
    UNUSED,              // OxydExtra stone 0x48
    UNUSED,              // OxydExtra stone 0x49
    UNUSED,              // OxydExtra stone 0x4a
    "st-mirror-p-t",     // OxydExtra stone 0x4b
    UNUSED,              // OxydExtra stone 0x4c
    "st-mirror-3>",      // OxydExtra stone 0x4d
    "st-mirror-3^",      // OxydExtra stone 0x4e
    "st-mirror-3<",      // OxydExtra stone 0x4f
    "st-mirror-3v",      // OxydExtra stone 0x50
    UNUSED,              // OxydExtra stone 0x51
    UNUSED,              // OxydExtra stone 0x52
    UNUSED,              // OxydExtra stone 0x53
    UNUSED,              // OxydExtra stone 0x54
    UNUSED,              // OxydExtra stone 0x55
    UNUSED,              // OxydExtra stone 0x56
    UNUSED,              // OxydExtra stone 0x57
    UNUSED,              // OxydExtra stone 0x58
    UNUSED,              // OxydExtra stone 0x59
    UNUSED,              // OxydExtra stone 0x5a
    UNUSED,              // OxydExtra stone 0x5b
    UNUSED,              // OxydExtra stone 0x5c
    "st-stone_break",    // OxydExtra stone 0x5d
    UNUSED,              // OxydExtra stone 0x5e
    UNUSED,              // OxydExtra stone 0x5f
    UNUSED,              // OxydExtra stone 0x60
    UNUSED,              // OxydExtra stone 0x61
    UNUSED,              // OxydExtra stone 0x62
    UNUSED,              // OxydExtra stone 0x63
    "st-coinslot",       // OxydExtra stone 0x64
    "st-thief",          // OxydExtra stone 0x65
    "st-shogun-s",       // OxydExtra stone 0x66
    UNUSED,              // OxydExtra stone 0x67
    UNUSED,              // OxydExtra stone 0x68
    UNUSED,              // OxydExtra stone 0x69
    UNUSED,              // OxydExtra stone 0x6a
    UNUSED,              // OxydExtra stone 0x6b
    UNUSED,              // OxydExtra stone 0x6c
    "st-stoneimpulse",   // OxydExtra stone 0x6d
    UNUSED,              // OxydExtra stone 0x6e
    UNUSED,              // OxydExtra stone 0x6f
    UNUSED,              // OxydExtra stone 0x70
    UNUSED,              // OxydExtra stone 0x71
    UNUSED,              // OxydExtra stone 0x72
    "st-door-h",         // OxydExtra stone 0x73
    "st-door-v",         // OxydExtra stone 0x74
    UNUSED,              // OxydExtra stone 0x75
    "st-invisible",      // OxydExtra stone 0x76
    UNUSED,              // OxydExtra stone 0x77
    UNUSED,              // OxydExtra stone 0x78
    UNUSED,              // OxydExtra stone 0x79
    UNUSED,              // OxydExtra stone 0x7a
    UNUSED,              // OxydExtra stone 0x7b
    UNUSED,              // OxydExtra stone 0x7c
    UNUSED,              // OxydExtra stone 0x7d
    UNUSED,              // OxydExtra stone 0x7e
    UNUSED,              // OxydExtra stone 0x7f
    UNUSED,              // OxydExtra stone 0x80
    UNUSED,              // OxydExtra stone 0x81
    UNUSED,              // OxydExtra stone 0x82
    UNUSED,              // OxydExtra stone 0x83
    UNUSED,              // OxydExtra stone 0x84
    UNUSED,              // OxydExtra stone 0x85
    UNUSED,              // OxydExtra stone 0x86
    UNUSED,              // OxydExtra stone 0x87
    UNUSED,              // OxydExtra stone 0x88
    UNUSED,              // OxydExtra stone 0x89
    "st-blue-sand",      // OxydExtra stone 0x8a
    "st-bluegray",       // OxydExtra stone 0x8b
    UNUSED,              // OxydExtra stone 0x8c
    UNUSED,              // OxydExtra stone 0x8d
    UNUSED,              // OxydExtra stone 0x8e
    UNUSED,              // OxydExtra stone 0x8f
    UNUSED,              // OxydExtra stone 0x90
    UNUSED,              // OxydExtra stone 0x91
    UNUSED,              // OxydExtra stone 0x92
    UNUSED,              // OxydExtra stone 0x93
    UNUSED,              // OxydExtra stone 0x94
    UNUSED,              // OxydExtra stone 0x95
    UNUSED,              // OxydExtra stone 0x96
    UNUSED,              // OxydExtra stone 0x97
    UNUSED,              // OxydExtra stone 0x98
    UNUSED,              // OxydExtra stone 0x99
    UNUSED,              // OxydExtra stone 0x9a
    UNUSED,              // OxydExtra stone 0x9b
    UNUSED,              // OxydExtra stone 0x9c
    UNUSED,              // OxydExtra stone 0x9d
    "st-turnstile",      // OxydExtra stone 0x9e
    "st-turnstile-n",    // OxydExtra stone 0x9f
    "st-turnstile-s",    // OxydExtra stone 0xa0
    "st-turnstile-w",    // OxydExtra stone 0xa1
    "st-turnstile-e",    // OxydExtra stone 0xa2
    UNUSED,              // OxydExtra stone 0xa3
    "st-fakeoxyd",       // OxydExtra stone 0xa4
    UNUSED,              // OxydExtra stone 0xa5
    UNUSED,              // OxydExtra stone 0xa6
    UNUSED,              // OxydExtra stone 0xa7
    UNUSED,              // OxydExtra stone 0xa8
    UNUSED,              // OxydExtra stone 0xa9
    UNUSED,              // OxydExtra stone 0xaa
    UNUSED,              // OxydExtra stone 0xab
    UNUSED,              // OxydExtra stone 0xac
    UNUSED,              // OxydExtra stone 0xad
    UNUSED,              // OxydExtra stone 0xae
    UNUSED,              // OxydExtra stone 0xaf
    UNUSED,              // OxydExtra stone 0xb0
    UNUSED,              // OxydExtra stone 0xb1
    UNUSED,              // OxydExtra stone 0xb2
    UNUSED,              // OxydExtra stone 0xb3
    UNUSED,              // OxydExtra stone 0xb4
    UNUSED,              // OxydExtra stone 0xb5
    UNUSED,              // OxydExtra stone 0xb6
    UNUSED,              // OxydExtra stone 0xb7
    UNUSED,              // OxydExtra stone 0xb8
    UNUSED,              // OxydExtra stone 0xb9
    UNUSED,              // OxydExtra stone 0xba
    "st-stoneimpulse-hollow", // OxydExtra stone 0xbb
    // codes >= 0xbc are unused
};

ITEMMAPTYPE oxyd::oxydextra_item_map[256] = {
    ITEMSPEC(it_none),                    // OxydExtra item 0x00
    ITEMSPEC(it_extralife),               // OxydExtra item 0x01
    ITEMSPEC(it_document),                // OxydExtra item 0x02
    ITEMSPEC(it_document),                // OxydExtra item 0x03
    ITEMSPEC(it_hammer),                  // OxydExtra item 0x04
    ITEMSPEC(it_coffee),                  // OxydExtra item 0x05
    ITEMSPEC(it_cherry),                  // OxydExtra item 0x06
    ITEMSPEC(it_umbrella),                // OxydExtra item 0x07
    ITEMSPEC(it_MISSING),                 // OxydExtra item 0x08
    ITEMSPEC(it_UNUSED),                  // OxydExtra item 0x09
    ITEMSPEC(it_dynamite),                // OxydExtra item 0x0a
    ITEMSPEC(it_UNUSED),                  // OxydExtra item 0x0b
    ITEMSPEC(it_UNUSED),                  // OxydExtra item 0x0c
    ITEMSPEC(it_crack0),                  // OxydExtra item 0x0d
    ITEMSPEC(it_crack1),                  // OxydExtra item 0x0e
    ITEMSPEC(it_crack2),                  // OxydExtra item 0x0f
    ITEMSPEC(it_crack3),                  // OxydExtra item 0x10
    ITEMSPEC(it_coin1),                   // OxydExtra item 0x11
    ITEMSPEC(it_coin2),                   // OxydExtra item 0x12
    ITEMSPEC(it_coin4),                   // OxydExtra item 0x13
    ITEMSPEC(it_MISSING),                 // OxydExtra item 0x14
    ITEMSPEC(it_MISSING),                 // OxydExtra item 0x15
    ITEMSPEC(it_key_a),                   // OxydExtra item 0x16
    ITEMSPEC(it_floppy),                  // OxydExtra item 0x17
    ITEMSPEC(it_UNUSED),                  // OxydExtra item 0x18
    ITEMSPEC(it_UNUSED),                  // OxydExtra item 0x19
    ITEMSPEC(it_UNUSED),                  // OxydExtra item 0x1a
    ITEMSPEC(it_UNUSED),                  // OxydExtra item 0x1b
    ITEMSPEC(it_UNUSED),                  // OxydExtra item 0x1c
    ITEMSPEC(it_UNUSED),                  // OxydExtra item 0x1d
    ITEMSPEC(it_UNUSED),                  // OxydExtra item 0x1e
    ITEMSPEC(it_UNUSED),                  // OxydExtra item 0x1f
    ITEMSPEC(it_UNUSED),                  // OxydExtra item 0x20
    ITEMSPEC(it_spade),                   // OxydExtra item 0x21
    ITEMSPEC(it_surprise),                // OxydExtra item 0x22
    ITEMSPEC(it_pin),                     // OxydExtra item 0x23
    ITEMSPEC(it_UNUSED),                  // OxydExtra item 0x24
    ITEMSPEC(it_UNUSED),                  // OxydExtra item 0x25
    ITEMSPEC(it_UNUSED),                  // OxydExtra item 0x26
    ITEMSPEC(it_bag),                     // OxydExtra item 0x27
    ITEMSPEC(it_UNUSED),                  // OxydExtra item 0x28
    ITEMSPEC(it_UNUSED),                  // OxydExtra item 0x29
    ITEMSPEC(it_sensor),                  // OxydExtra item 0x2a
    ITEMSPEC(it_shogun_s),                // OxydExtra item 0x2b
    ITEMSPEC(it_UNUSED),                  // OxydExtra item 0x2c
    ITEMSPEC(it_vortex_open),             // OxydExtra item 0x2d
    ITEMSPEC(it_UNUSED),                  // OxydExtra item 0x2e
    ITEMSPEC(it_wormhole_on),             // OxydExtra item 0x2f
    ITEMSPEC(it_hill),                    // OxydExtra item 0x30
    ITEMSPEC(it_tinyhill),                // OxydExtra item 0x31
    ITEMSPEC(it_hollow),                  // OxydExtra item 0x32
    ITEMSPEC(it_tinyhollow),              // OxydExtra item 0x33
    ITEMSPEC(it_UNUSED),                  // OxydExtra item 0x34
    ITEMSPEC(it_UNUSED),                  // OxydExtra item 0x35
    ITEMSPEC(it_UNUSED),                  // OxydExtra item 0x36
    ITEMSPEC(it_bridge_oxyd),             // OxydExtra item 0x37
    ITEMSPEC(it_UNUSED),                  // OxydExtra item 0x38
    ITEMSPEC(it_UNUSED),                  // OxydExtra item 0x39
    ITEMSPEC(it_UNUSED),                  // OxydExtra item 0x3a
    ITEMSPEC(it_UNUSED),                  // OxydExtra item 0x3b
    ITEMSPEC(it_MISSING),                 // OxydExtra item 0x3c
    ITEMSPEC(it_UNUSED),                  // OxydExtra item 0x3d
    ITEMSPEC(it_UNUSED),                  // OxydExtra item 0x3e
    ITEMSPEC(it_UNUSED),                  // OxydExtra item 0x3f
    ITEMSPEC(it_trigger),                 // OxydExtra item 0x40
    ITEMSPEC(it_brush),                   // OxydExtra item 0x41
    ITEMSPEC(it_MISSING),                 // OxydExtra item 0x42
    // codes >= 0x43 are unused
};
