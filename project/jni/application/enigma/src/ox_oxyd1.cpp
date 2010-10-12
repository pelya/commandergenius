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
#define ITEMMAPTYPE ItemID

#include "objects.hh"
#include "server.hh"
#include "oxyd_internal.hh"

using namespace enigma;
using namespace world;
using namespace oxyd;

/* -------------------- Oxyd 1 level pack -------------------- */

LP_Oxyd1::LP_Oxyd1 (DatFile *dat, bool twoplayers)
: LevelPack_Oxyd (OxydVersion_Oxyd1, dat,
                  twoplayers ? 100 : 0,
                  twoplayers ? 199 : 99,
                  twoplayers)
{
}

void LP_Oxyd1::load (const OxydLib::Level &level)
{
    LoaderConfig c (needs_twoplayers(),
                    get_gamemode(),
                    oxyd1_floor_map,
                    oxyd1_item_map,
                    oxyd1_stone_map);
    c.id_timer  = 0x33;
    c.id_laser1 = 0x44;

    OxydLoader (level, c).load();

    if (server::GetDifficulty() == DIFFICULTY_EASY)
        server::WaterSinkSpeed = 100.0;
    else
        server::WaterSinkSpeed = 200.0;
}

#endif // PLAIN_SPEC_ONLY
// only tables following!

const char *oxyd::oxyd1_floor_map[256] = {
    "fl-abyss",                 // Oxyd1 floor 0x00
    "fl-gray",                  // Oxyd1 floor 0x01
    "fl-metal",                 // Oxyd1 floor 0x02
    "fl-samba",                 // Oxyd1 floor 0x03 was: checkered floor
    UNUSED,                     // Oxyd1 floor 0x04
    UNUSED,                     // Oxyd1 floor 0x05
    UNUSED,                     // Oxyd1 floor 0x06
    "fl-normal",                // Oxyd1 floor 0x07
    "fl-himalaya",              // Oxyd1 floor 0x08
    "fl-himalaya",              // Oxyd1 floor 0x09
    "fl-himalaya",              // Oxyd1 floor 0x0a
    "fl-himalaya",              // Oxyd1 floor 0x0b
    "fl-inverse2",              // Oxyd1 floor 0x0c
    "fl-acblack",               // Oxyd1 floor 0x0d
    "fl-acwhite",               // Oxyd1 floor 0x0e
    "fl-swamp",                 // Oxyd1 floor 0x0f
    UNUSED,                     // Oxyd1 floor 0x10
    UNUSED,                     // Oxyd1 floor 0x11
    UNUSED,                     // Oxyd1 floor 0x12
    "fl-water",                 // Oxyd1 floor 0x13
    UNUSED,                     // Oxyd1 floor 0x14
    UNUSED,                     // Oxyd1 floor 0x15
    UNUSED,                     // Oxyd1 floor 0x16
    "fl-ice",                   // Oxyd1 floor 0x17
    UNUSED,                     // Oxyd1 floor 0x18
    UNUSED,                     // Oxyd1 floor 0x19
    UNUSED,                     // Oxyd1 floor 0x1a
    UNUSED,                     // Oxyd1 floor 0x1b
    "fl-space",                 // Oxyd1 floor 0x1c
    "fl-space",                 // Oxyd1 floor 0x1d
    UNUSED,                     // Oxyd1 floor 0x1e
    UNUSED,                     // Oxyd1 floor 0x1f
    "fl-space",                 // Oxyd1 floor 0x20
    "fl-space-force",           // Oxyd1 floor 0x21
    "fl-space",                 // Oxyd1 floor 0x22
    "fl-gradient1",             // Oxyd1 floor 0x23
    "fl-gradient2",             // Oxyd1 floor 0x24
    "fl-gradient3",             // Oxyd1 floor 0x25
    "fl-gradient4",             // Oxyd1 floor 0x26
    "fl-gradient5",             // Oxyd1 floor 0x27
    "fl-gradient6",             // Oxyd1 floor 0x28
    "fl-gradient7",             // Oxyd1 floor 0x29
    "fl-gradient8",             // Oxyd1 floor 0x2a
    "fl-gradient9",             // Oxyd1 floor 0x2b
    "fl-gradient10",            // Oxyd1 floor 0x2c
    "fl-gradient11",            // Oxyd1 floor 0x2d
    "fl-gradient12",            // Oxyd1 floor 0x2e
    "fl-gradient13",            // Oxyd1 floor 0x2f
    "fl-gradient14",            // Oxyd1 floor 0x30
    "fl-gradient15",            // Oxyd1 floor 0x31
    "fl-gradient16",            // Oxyd1 floor 0x32
    "fl-brick",                 // Oxyd1 floor 0x33
    "fl-brick",                 // Oxyd1 floor 0x34 (common was 'fl-rough')
    UNUSED,                     // Oxyd1 floor 0x35
    UNUSED,                     // Oxyd1 floor 0x36
    "fl-brick",                 // Oxyd1 floor 0x37
    "fl-brick",                 // Oxyd1 floor 0x38
    UNUSED,                     // Oxyd1 floor 0x39
    UNUSED,                     // Oxyd1 floor 0x3a
    UNUSED,                     // Oxyd1 floor 0x3b
    "fl-rock",                  // Oxyd1 floor 0x3c
    UNUSED,                     // Oxyd1 floor 0x3d
    UNUSED,                     // Oxyd1 floor 0x3e
    UNUSED,                     // Oxyd1 floor 0x3f
    "fl-stwood1",               // Oxyd1 floor 0x40
    "fl-wood",                  // Oxyd1 floor 0x41
    "fl-samba1",                // Oxyd1 floor 0x42
    UNUSED,                     // Oxyd1 floor 0x43
    UNUSED,                     // Oxyd1 floor 0x44
    UNUSED,                     // Oxyd1 floor 0x45
    "fl-gravel",                // Oxyd1 floor 0x46
    "fl-gravel",                // Oxyd1 floor 0x47
    UNUSED,                     // Oxyd1 floor 0x48
    UNUSED,                     // Oxyd1 floor 0x49
    UNUSED,                     // Oxyd1 floor 0x4a
    UNUSED,                     // Oxyd1 floor 0x4b
    UNUSED,                     // Oxyd1 floor 0x4c
    "fl-tigris",                // Oxyd1 floor 0x4d
    UNUSED,                     // Oxyd1 floor 0x4e
    UNUSED,                     // Oxyd1 floor 0x4f
    UNUSED,                     // Oxyd1 floor 0x50
    UNUSED,                     // Oxyd1 floor 0x51
    "fl-bridge",                // Oxyd1 floor 0x52
    "fl-springboard",           // Oxyd1 floor 0x53
    UNUSED,                     // Oxyd1 floor 0x54
    "fl-gray",                  // Oxyd1 floor 0x55
    "fl-light",                 // Oxyd1 floor 0x56
    "fl-lightgray",             // Oxyd1 floor 0x57
    "fl-darkgray",              // Oxyd1 floor 0x58
    "fl-dunes",                 // Oxyd1 floor 0x59
    "fl-normal",                // Oxyd1 floor 0x5a
    "fl-plank",                 // Oxyd1 floor 0x5b
    // codes >= 0x5c are unused
};

const char *oxyd::oxyd1_stone_map[256] = {
    0,                          // Oxyd1 stone 0x00
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    "st-fakeoxyd",              // Oxyd1 stone 0x11
    "st-plain_break",           // Oxyd1 stone 0x12
    "st-bigbrick-es",           // Oxyd1 stone 0x13
    "st-bigbrick-sw",           // Oxyd1 stone 0x14
    "st-bigbrick-ne",           // Oxyd1 stone 0x15
    "st-bigbrick-nw",           // Oxyd1 stone 0x16
    "st-plain_hole",            // Oxyd1 stone 0x17
    "st-oxyd-0x18",             // Oxyd1 stone 0x18
    "st-glass1_hole",           // Oxyd1 stone 0x19
    "st-grate1",                // Oxyd1 stone 0x1a
    "st-grate2",                // Oxyd1 stone 0x1b
    "st-bug",                   // Oxyd1 stone 0x1c
    "st-surprise",              // Oxyd1 stone 0x1d
    "st-chameleon",             // Oxyd1 stone 0x1e
    "st-brick",                 // Oxyd1 stone 0x1f
    "st-rock1",                 // Oxyd1 stone 0x20
    "st-rock1",                 // Oxyd1 stone 0x21
    UNUSED,                     // Oxyd1 stone 0x22
    "st-woven",                 // Oxyd1 stone 0x23
    "st-marble",                // Oxyd1 stone 0x24
    "st-glass1",                // Oxyd1 stone 0x25
    "st-glass3",                // Oxyd1 stone 0x26
    "st-glass2",                // Oxyd1 stone 0x27
    "st-plain_break",           // Oxyd1 stone 0x28 [see Level#12]
    "st-plain_hole",            // Oxyd1 stone 0x29
    "st-plain_move",            // Oxyd1 stone 0x2a
    "st-wood",                  // Oxyd1 stone 0x2b
    "st-switch",                // Oxyd1 stone 0x2c
    "st-switch_black",          // Oxyd1 stone 0x2d
    "st-switch_white",          // Oxyd1 stone 0x2e
    "st-fourswitch",            // Oxyd1 stone 0x2f
    "st-key",                   // Oxyd1 stone 0x30
    "st-floppy",                // Oxyd1 stone 0x31
    "st-knight",                // Oxyd1 stone 0x32
    0,                          // Oxyd1 stone 0x33 oscillator (properties set in oxyd.cc)
    "st-death",                 // Oxyd1 stone 0x34
    "st-death_invisible",       // Oxyd1 stone 0x35
    "st-oneway_black-w",        // Oxyd1 stone 0x36
    "st-oneway_black-e",        // Oxyd1 stone 0x37
    "st-oneway_black-n",        // Oxyd1 stone 0x38
    "st-oneway_black-s",        // Oxyd1 stone 0x39
    "st-oneway_white-w",        // Oxyd1 stone 0x3a
    "st-oneway_white-e",        // Oxyd1 stone 0x3b
    "st-oneway_white-n",        // Oxyd1 stone 0x3c
    "st-oneway_white-s",        // Oxyd1 stone 0x3d
    "st-window",                // Oxyd1 stone 0x3e
    "",                         // Oxyd1 stone 0x3f magic stone
    "",                         // Oxyd1 stone 0x40 magic stone
    "",                         // Oxyd1 stone 0x41 magic stone
    "st-actorimpulse",          // Oxyd1 stone 0x42
    "st-actorimpulse_invisible", // Oxyd1 stone 0x43
    "st-laser-1",               // Oxyd1 stone 0x44  The laser-names are fake names!
    "st-laser-2",               // Oxyd1 stone 0x45  Direction and state are generated by Enigma.
    "st-laser-3",               // Oxyd1 stone 0x46
    "st-mirror-p|",             // Oxyd1 stone 0x47
    "st-mirror-p/",             // Oxyd1 stone 0x48
    "st-mirror-p-",             // Oxyd1 stone 0x49
    "st-mirror-p\\",            // Oxyd1 stone 0x4a
    "st-mirror-p|m",            // Oxyd1 stone 0x4b
    "st-mirror-p/m",            // Oxyd1 stone 0x4c
    "st-mirror-p-m",            // Oxyd1 stone 0x4d
    "st-mirror-p\\m",           // Oxyd1 stone 0x4e
    "st-mirror-p|t",            // Oxyd1 stone 0x4f
    "st-mirror-p/t",            // Oxyd1 stone 0x50
    "st-mirror-p-t",            // Oxyd1 stone 0x51
    "st-mirror-p\\t",           // Oxyd1 stone 0x52
    "st-mirror-3>",             // Oxyd1 stone 0x53
    "st-mirror-3^",             // Oxyd1 stone 0x54
    "st-mirror-3<",             // Oxyd1 stone 0x55
    "st-mirror-3v",             // Oxyd1 stone 0x56
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
    "st-metal_hole",            // Oxyd1 stone 0x62
    "st-laserbreak",            // Oxyd1 stone 0x63
    "st-fakeoxyda",             // Oxyd1 stone 0x64
    UNUSED,                     // Oxyd1 stone 0x65
    "st-disco-medium",          // Oxyd1 stone 0x66
    "st-disco-light",           // Oxyd1 stone 0x67
    "st-bombs",                 // Oxyd1 stone 0x68
    "st-flash",                 // Oxyd1 stone 0x69
    "st-coinslot",              // Oxyd1 stone 0x6a
    "st-thief",                 // Oxyd1 stone 0x6b
    "st-shogun-s",              // Oxyd1 stone 0x6c
    "st-stoneimpulse",          // Oxyd1 stone 0x6d
    "st-lasertimeswitch",       // Oxyd1 stone 0x6e
    "st-mail-n",                // Oxyd1 stone 0x6f
    "st-mail-w",                // Oxyd1 stone 0x70
    "st-mail-e",                // Oxyd1 stone 0x71
    "st-mail-s",                // Oxyd1 stone 0x72
    "st-door-h",                // Oxyd1 stone 0x73
    "st-door-v",                // Oxyd1 stone 0x74
    "st-metal",                 // Oxyd1 stone 0x75
    "st-invisible",             // Oxyd1 stone 0x76
    "st-door-h-open",           // Oxyd1 stone 0x77
    "st-door-v-open",           // Oxyd1 stone 0x78
    UNUSED,                     // Oxyd1 stone 0x79
    "st-timeswitch",            // Oxyd1 stone 0x7a (only used in level #23)
    UNUSED,                     // Oxyd1 stone 0x7b
    UNUSED,                     // Oxyd1 stone 0x7c
    UNUSED,                     // Oxyd1 stone 0x7d
    UNUSED,                     // Oxyd1 stone 0x7e
    UNUSED,                     // Oxyd1 stone 0x7f
    UNUSED,                     // Oxyd1 stone 0x80
    "",                         // Oxyd1 stone 0x81
    UNUSED,                     // Oxyd1 stone 0x82
    UNUSED,                     // Oxyd1 stone 0x83
    UNUSED,                     // Oxyd1 stone 0x84
    UNUSED,                     // Oxyd1 stone 0x85
    "st-metal_hole",            // Oxyd1 stone 0x86
    "st-rock2",                 // Oxyd1 stone 0x87
    UNUSED,                     // Oxyd1 stone 0x88
    UNUSED,                     // Oxyd1 stone 0x89
    "st-stone1",                // Oxyd1 stone 0x8a
    "st-blue-sand",             // Oxyd1 stone 0x8b
    "st-white1",                // Oxyd1 stone 0x8c
    "st-black1",                // Oxyd1 stone 0x8d
    "st-yinyang2",              // Oxyd1 stone 0x8e
    UNUSED,                     // Oxyd1 stone 0x8f
    "st-whiteballs",            // Oxyd1 stone 0x90
    "st-blackballs",            // Oxyd1 stone 0x91
    "st-fourswitch",            // Oxyd1 stone 0x92
    "st-yinyang1",              // Oxyd1 stone 0x93
    "st-break_acwhite",         // Oxyd1 stone 0x94
    "st-break_acblack",         // Oxyd1 stone 0x95
    "st-chargeplus",            // Oxyd1 stone 0x96
    "st-chargeminus",           // Oxyd1 stone 0x97
    "st-chargezero",            // Oxyd1 stone 0x98
    "st-glass1",                // Oxyd1 stone 0x99
    // codes >= 0x9a are unused
};

ITEMMAPTYPE oxyd::oxyd1_item_map[256] = {
    ITEMSPEC(it_none),                    // 0x00
    ITEMSPEC(it_extralife),               // 0x01
    ITEMSPEC(it_EXTERNAL),                // 0x02 document 1
    ITEMSPEC(it_EXTERNAL),                // 0x03 document 2
    ITEMSPEC(it_hammer),                  // 0x04
    ITEMSPEC(it_coffee),                  // 0x05
    ITEMSPEC(it_cherry),                  // 0x06
    ITEMSPEC(it_umbrella),                // 0x07
    ITEMSPEC(it_glasses),                 // 0x08
    ITEMSPEC(it_glasses_broken),          // 0x09
    ITEMSPEC(it_dynamite),                // 0x0a
    ITEMSPEC(it_blackbomb),               // 0x0b
    ITEMSPEC(it_whitebomb),               // 0x0c
    ITEMSPEC(it_crack0),                  // 0x0d
    ITEMSPEC(it_crack1),                  // 0x0e
    ITEMSPEC(it_crack2),                  // 0x0f
    ITEMSPEC(it_crack3),                  // 0x10
    ITEMSPEC(it_coin1),                   // 0x11
    ITEMSPEC(it_coin2),                   // 0x12
    ITEMSPEC(it_coin4),                   // 0x13
    ITEMSPEC(it_key_a),                   // 0x14
    ITEMSPEC(it_key_b),                   // 0x15
    ITEMSPEC(it_key_c),                   // 0x16
    ITEMSPEC(it_floppy),                  // 0x17
    ITEMSPEC(it_sword),                   // 0x18
    ITEMSPEC(it_flagwhite),               // 0x19
    ITEMSPEC(it_flagblack),               // 0x1a
    ITEMSPEC(it_ring),                    // 0x1b
    ITEMSPEC(it_pipe_wn),                 // 0x1c
    ITEMSPEC(it_pipe_sw),                 // 0x1d
    ITEMSPEC(it_pipe_ne),                 // 0x1e
    ITEMSPEC(it_pipe_es),                 // 0x1f
    ITEMSPEC(it_pipe_v),                  // 0x20
    ITEMSPEC(it_pipe_h),                  // 0x21
    ITEMSPEC(it_spade),                   // 0x22
    ITEMSPEC(it_surprise),                // 0x23
    ITEMSPEC(it_pin),                     // 0x24
    ITEMSPEC(it_seed),                    // 0x25
    ITEMSPEC(it_spring2),                 // 0x26
    ITEMSPEC(it_spring1),                 // 0x27
    ITEMSPEC(it_bag),                     // 0x28
    ITEMSPEC(it_magnet_off),              // 0x29
    ITEMSPEC(it_inversesensor),           // 0x2a
    ITEMSPEC(it_sensor),                  // 0x2b
    ITEMSPEC(it_shogun_s),                // 0x2c
    ITEMSPEC(it_vortex_open),             // 0x2d
    ITEMSPEC(it_vortex_closed),           // 0x2e
    ITEMSPEC(it_wormhole_on),             // 0x2f
    ITEMSPEC(it_hill),                    // 0x30
    ITEMSPEC(it_tinyhill),                // 0x31
    ITEMSPEC(it_hollow),                  // 0x32
    ITEMSPEC(it_tinyhollow),              // 0x33
    ITEMSPEC(it_vstrip),                  // 0x34
    ITEMSPEC(it_hstrip),                  // 0x35
    ITEMSPEC(it_springboard),             // 0x36
    ITEMSPEC(it_bridge_oxyd),             // 0x37 bridge active
    ITEMSPEC(it_bridge_oxyd),             // 0x38 bridge inactive
    ITEMSPEC(it_bridge_oxyd_active),      // 0x39 walkable bridge (?)
    ITEMSPEC(it_UNUSED),                  // 0x3a
    ITEMSPEC(it_UNUSED),                  // 0x3b
    ITEMSPEC(it_UNUSED),                  // 0x3c
    ITEMSPEC(it_UNUSED),                  // 0x3d
    ITEMSPEC(it_UNUSED),                  // 0x3e
    ITEMSPEC(it_UNUSED),                  // 0x3f
    ITEMSPEC(it_UNUSED),                  // 0x40
    ITEMSPEC(it_UNUSED),                  // 0x41
    ITEMSPEC(it_UNUSED),                  // 0x42
    ITEMSPEC(it_UNUSED),                  // 0x43
    ITEMSPEC(it_UNUSED),                  // 0x44
    ITEMSPEC(it_UNUSED),                  // 0x45
    ITEMSPEC(it_UNUSED),                  // 0x46
    ITEMSPEC(it_UNUSED),                  // 0x47
    ITEMSPEC(it_UNUSED),                  // 0x48
    ITEMSPEC(it_UNUSED),                  // 0x49
    ITEMSPEC(it_UNUSED),                  // 0x4a
    ITEMSPEC(it_UNUSED),                  // 0x4b
    ITEMSPEC(it_UNUSED),                  // 0x4c
    ITEMSPEC(it_UNUSED),                  // 0x4d
    ITEMSPEC(it_UNUSED),                  // 0x4e
    ITEMSPEC(it_UNUSED),                  // 0x4f
    ITEMSPEC(it_UNUSED),                  // 0x50
    ITEMSPEC(it_UNUSED),                  // 0x51
    ITEMSPEC(it_UNUSED),                  // 0x52
    ITEMSPEC(it_UNUSED),                  // 0x53
    ITEMSPEC(it_UNUSED),                  // 0x54
    ITEMSPEC(it_UNUSED),                  // 0x55
    ITEMSPEC(it_UNUSED),                  // 0x56
    ITEMSPEC(it_UNUSED),                  // 0x57
    ITEMSPEC(it_UNUSED),                  // 0x58
    ITEMSPEC(it_UNUSED),                  // 0x59
    ITEMSPEC(it_UNUSED),                  // 0x5a
    ITEMSPEC(it_UNUSED),                  // 0x5b
    ITEMSPEC(it_UNUSED),                  // 0x5c
    ITEMSPEC(it_UNUSED),                  // 0x5d
    ITEMSPEC(it_UNUSED),                  // 0x5e
    ITEMSPEC(it_oxyd5f),                  // 0x5f
    ITEMSPEC(it_drop),                  // 0x60    drop (turns actor into rotor)
    // codes >= 0x61 are unused
};
