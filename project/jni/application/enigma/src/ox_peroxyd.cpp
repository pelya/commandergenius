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

#include "objects.hh"
#include "server.hh"
#include "oxyd_internal.hh"

using namespace world;
using namespace oxyd;

Stone *PerOxydLoader::make_stone (int type, int x, int y)
{
    Stone *st = 0;
    switch (type) {
    case 0x39: case 0x3a: case 0x3b:
        // Create magic stones only if they are absolutely necessary
        if (level.getRequireMagicPiece())
            st = MakeStone ("st-magic");
        break;
    default:
        st = OxydLoader::make_stone (type, x, y);
        break;
    }
    return st;
}



/* -------------------- Per.Oxyd level pack -------------------- */

LP_PerOxyd::LP_PerOxyd (DatFile *dat, bool twoplayers)
: LevelPack_Oxyd (OxydVersion_PerOxyd, dat,
                  twoplayers ? 100 : 0,
                  twoplayers ? 199 : 99,
                  twoplayers)
{
}

void LP_PerOxyd::load (const OxydLib::Level &level)
{
    LoaderConfig c (needs_twoplayers(),
                    get_gamemode(),
                    peroxyd_floor_map,
                    peroxyd_item_map,
                    peroxyd_stone_map);
    c.id_timer  = 0x2e;
    c.id_laser1 = 0x3e;
    PerOxydLoader (level, c).load();
}

bool LP_PerOxyd::hasEasymode(size_t index) {
    static const bool easymode_map[200] = { // 1-100 single, 101 - 200 netplay
        false, false, true,  false, false, false, false, true,  true,  true,  //   1 -  10
        true,  false, false, true,  false, false, true,  false, false, false, //  11 -  20
        false, false, false, true,  false, true,  true,  false, true,  true,  //  21 -  30
        true,  false, true,  true,  false, false, true,  true,  false, true,  //  31 -  40
        true,  true,  true,  false, true,  false, true,  true,  false, true,  //  41 -  50
        true,  true,  false, true,  true,  true,  false, true,  false, true,  //  51 -  60
        true,  false, false, false, true,  false, true,  true,  false, true,  //  61 -  70
        false, true,  true,  true,  true,  true,  true,  true,  true,  true,  //  71 -  80
        true,  true,  true,  true,  false, true,  false, true,  true,  true,  //  81 -  90
        false, true,  false, true,  true,  true,  true,  true,  true,  true,  //  91 - 100
        false, false, true,  true,  true,  false, false, true,  false, true,  // 101 - 110
        false, true,  false, true,  true,  true,  false, false, false, false, // 111 - 120
        true,  false, false, true,  false, false, false, false, true,  false, // 121 - 130
        false, false, true,  true,  false, true,  false, true,  true,  true,  // 131 - 140
        false, false, true,  false, false, true,  true,  true,  true,  false, // 141 - 150
        true,  true,  true,  true,  false, true,  true,  true,  false, false, // 151 - 160
        true,  true,  true,  true,  true,  true,  false, true,  true,  true,  // 161 - 170
        false, true,  true,  true,  true,  true,  true,  true,  true,  true,  // 171 - 180
        false, true,  false, true,  true,  true,  false, true,  false, true,  // 181 - 190
        false, true,  false, false, true,  true,  true,  true,  true,  true   // 191 - 200
    };
    return easymode_map[index];
} 

#endif // PLAIN_SPEC_ONLY
// only tables following!

const char *oxyd::peroxyd_floor_map[256] = {
    "fl-abyss",                 // PerOxyd floor 0x00
    "fl-gray",                  // PerOxyd floor 0x01
    "fl-metal",                 // PerOxyd floor 0x02
    "fl-metal",                 // PerOxyd floor 0x03
    "fl-metal",                 // PerOxyd floor 0x04
    "fl-metal",                 // PerOxyd floor 0x05
    "fl-metal",                 // PerOxyd floor 0x06
    "fl-metal",                 // PerOxyd floor 0x07
    "fl-rough-red",             // PerOxyd floor 0x08
    "fl-rough-blue",            // PerOxyd floor 0x09
    "fl-rough-blue",            // PerOxyd floor 0x0a
    "fl-rough-red",             // PerOxyd floor 0x0b
    "fl-inverse",               // PerOxyd floor 0x0c
    "fl-acblack",               // PerOxyd floor 0x0d
    "fl-acwhite",               // PerOxyd floor 0x0e
    "fl-swamp",                 // PerOxyd floor 0x0f
    UNUSED,                     // PerOxyd floor 0x10
    UNUSED,                     // PerOxyd floor 0x11
    UNUSED,                     // PerOxyd floor 0x12
    "fl-water",                 // PerOxyd floor 0x13
    "fl-water",                 // PerOxyd floor 0x14
    UNUSED,                     // PerOxyd floor 0x15
    UNUSED,                     // PerOxyd floor 0x16
    "fl-ice",                   // PerOxyd floor 0x17
    UNUSED,                     // PerOxyd floor 0x18
    UNUSED,                     // PerOxyd floor 0x19
    UNUSED,                     // PerOxyd floor 0x1a
    UNUSED,                     // PerOxyd floor 0x1b
    "fl-space",                 // PerOxyd floor 0x1c
    UNUSED,                     // PerOxyd floor 0x1d
    UNUSED,                     // PerOxyd floor 0x1e
    UNUSED,                     // PerOxyd floor 0x1f
    UNUSED,                     // PerOxyd floor 0x20
    "fl-space-force",           // PerOxyd floor 0x21 (flat force)
    UNUSED,                     // PerOxyd floor 0x22
    "fl-gradient1",             // PerOxyd floor 0x23
    "fl-gradient2",             // PerOxyd floor 0x24
    "fl-gradient3",             // PerOxyd floor 0x25
    "fl-gradient4",             // PerOxyd floor 0x26
    "fl-gradient5",             // PerOxyd floor 0x27
    "fl-gradient6",             // PerOxyd floor 0x28
    "fl-gradient7",             // PerOxyd floor 0x29
    "fl-gradient8",             // PerOxyd floor 0x2a
    "fl-gradient9",             // PerOxyd floor 0x2b
    "fl-gradient10",            // PerOxyd floor 0x2c
    "fl-gradient11",            // PerOxyd floor 0x2d
    "fl-gradient12",            // PerOxyd floor 0x2e
    "fl-gradient13",            // PerOxyd floor 0x2f (flat force)
    "fl-gradient14",            // PerOxyd floor 0x30 (flat force)
    "fl-gradient15",            // PerOxyd floor 0x31 (flat force)
    "fl-gradient16",            // PerOxyd floor 0x32 (flat force)
    "fl-rough",                 // PerOxyd floor 0x33 (common was 'fl-rough', should be a 'fl-paper')(only level 75)
    "fl-rough",                 // PerOxyd floor 0x34
    UNUSED,                     // PerOxyd floor 0x35
    UNUSED,                     // PerOxyd floor 0x36
    UNUSED,                     // PerOxyd floor 0x37
    "fl-concrete",              // PerOxyd floor 0x38 (Tan/green horizontal Grain)
    "fl-concrete",              // PerOxyd floor 0x39 (Tan/green horizontal Grain, same as 0x38)(only level 118)
    UNUSED,                     // PerOxyd floor 0x3a
    UNUSED,                     // PerOxyd floor 0x3b
    "fl-stwood1",               // PerOxyd floor 0x3c
    "fl-samba",                 // PerOxyd floor 0x3d (common was 'fl-wood', horizontal or vertical slats)(many)
    "fl-samba1",                // PerOxyd floor 0x3e (common was 'fl-wood', horizontal slats)(only level 32)
    "fl-samba2",                // PerOxyd floor 0x3f (common was 'fl-wood', vertical slats)(levels: 35, 152)
    UNUSED,                     // PerOxyd floor 0x40
    UNUSED,                     // PerOxyd floor 0x41
    "fl-bumps",                 // PerOxyd floor 0x42
    "fl-bumps",                 // PerOxyd floor 0x43
    UNUSED,                     // PerOxyd floor 0x44
    UNUSED,                     // PerOxyd floor 0x45
    UNUSED,                     // PerOxyd floor 0x46
    UNUSED,                     // PerOxyd floor 0x47
    "fl-concrete",              // PerOxyd floor 0x48
    UNUSED,                     // PerOxyd floor 0x49
    UNUSED,                     // PerOxyd floor 0x4a
    UNUSED,                     // PerOxyd floor 0x4b
    "fl-bridge-open",           // PerOxyd floor 0x4c
    UNUSED,                     // PerOxyd floor 0x4d
    "fl-gray",                  // PerOxyd floor 0x4e
    "fl-gradient13",            // PerOxyd floor 0x4f (flat force up, beige)        (maybe gradient22?)
    "fl-black",                 // PerOxyd floor 0x50 (flat force down, dark olive) (maybe gradient21?)
    "fl-white",                 // PerOxyd floor 0x51 (flat force left, beige)      (maybe gradient24?)
    "fl-black",                 // PerOxyd floor 0x52 (flat force right, dark olive)(maybe gradient23?)
    "fl-floor_001",             // PerOxyd floor 0x53
    UNUSED,                     // PerOxyd floor 0x54
    "fl-plank",                 // PerOxyd floor 0x55
    UNUSED,                     // PerOxyd floor 0x56
    "fl-woven_orange",          // PerOxyd floor 0x57 (only used in Link-Level 91, friction (2.5) questionable!)
    UNUSED,                     // PerOxyd floor 0x58
    UNUSED,                     // PerOxyd floor 0x59
    UNUSED,                     // PerOxyd floor 0x5a
    "fl-gravel",                // PerOxyd floor 0x5b
    UNUSED,                     // PerOxyd floor 0x5c
    UNUSED,                     // PerOxyd floor 0x5d
    "fl-gravel",                // PerOxyd floor 0x5e (same as 0x5b)(only level 192)
    // codes >= 0x5f are unused
};

const char *oxyd::peroxyd_stone_map[256] = {
    0,                          // PerOxyd stone 0x00 no stone
    0,0,0,0,0,0,0,0,            // PerOxyd stone 0x01 - 0x08   oxyd stones
    0,0,0,0,0,0,0,0,            // PerOxyd stone 0x08 - 0x10   oxyd stones
    "st-likeoxydc",             // PerOxyd stone 0x11
    "st-plain_break",           // PerOxyd stone 0x12   no, should be destructible with hammer
    "st-rock4",                 // PerOxyd stone 0x13
    "st-beads",                 // PerOxyd stone 0x14
    "st-rock5",                 // PerOxyd stone 0x15
    "st-rock7",                 // PerOxyd stone 0x16
    "st-white1",                // PerOxyd stone 0x17 (Looks a little bit different, but function should be the same.)
    "st-grate1",                // PerOxyd stone 0x18
    UNUSED,                     // PerOxyd stone 0x19
    "st-surprise",              // PerOxyd stone 0x1a
    "st-chameleon",             // PerOxyd stone 0x1b (levels: 23 142 178 183)
    "st-brick_magic",           // PerOxyd stone 0x1c
    "st-camouflage",            // PerOxyd stone 0x1d (green camouflage piece)(if st-camouflage is implemented, put it in here)
    "st-camouflage",            // PerOxyd stone 0x1e (green camouflage piece)
    UNUSED,                     // PerOxyd stone 0x1f
    "st-stone2",                // PerOxyd stone 0x20
    "st-greenbrown",            // PerOxyd stone 0x21
    "st-glass",                 // PerOxyd stone 0x22 (semi opaque piece, not really glass)
    "st-glass",                 // PerOxyd stone 0x23 (must be st-glass, see link level 92)
    "st-plain_break",           // PerOxyd stone 0x24
    "st-bluegray_hole",         // PerOxyd stone 0x25
    "st-plain_move",            // PerOxyd stone 0x26
    "st-wood",                  // PerOxyd stone 0x27
    "st-switch",                // PerOxyd stone 0x28
    "st-switch_black",          // PerOxyd stone 0x29
    "st-switch_white",          // PerOxyd stone 0x2a
    "st-fourswitch",            // PerOxyd stone 0x2b
    "st-key",                   // PerOxyd stone 0x2c
    "st-floppy",                // PerOxyd stone 0x2d
    "",                         // PerOxyd stone 0x2e (?)
    "st-death",                 // PerOxyd stone 0x2f
    "st-death_invisible",       // PerOxyd stone 0x30
    "st-oneway_black-w",        // PerOxyd stone 0x31
    "st-oneway_black-e",        // PerOxyd stone 0x32
    "st-oneway_black-n",        // PerOxyd stone 0x33
    "st-oneway_black-s",        // PerOxyd stone 0x34
    "st-oneway_white-w",        // PerOxyd stone 0x35
    "st-oneway_white-e",        // PerOxyd stone 0x36
    "st-oneway_white-n",        // PerOxyd stone 0x37
    "st-oneway_white-s",        // PerOxyd stone 0x38
    "st-magic",                 // PerOxyd stone 0x39 ('st-magic')
    "st-magic",                 // PerOxyd stone 0x3a ('st-magic')
    "st-magic",                 // PerOxyd stone 0x3b ('st-magic')
    "st-actorimpulse",          // PerOxyd stone 0x3c
    "st-actorimpulse_invisible", // PerOxyd stone 0x3d
    "st-laser-1",               // PerOxyd stone 0x3e  The laser-names are fake names!
    "st-laser-2",               // PerOxyd stone 0x3f  Direction and state are generated by Enigma.
    "st-laser-3",               // PerOxyd stone 0x40
    "st-mirror-p|",             // PerOxyd stone 0x41
    "st-mirror-p/",             // PerOxyd stone 0x42
    "st-mirror-p-",             // PerOxyd stone 0x43
    "st-mirror-p\\",            // PerOxyd stone 0x44
    UNUSED,                     // PerOxyd stone 0x45
    "st-mirror-p/m",            // PerOxyd stone 0x46
    "st-mirror-p-m",            // PerOxyd stone 0x47
    "st-mirror-p\\m",           // PerOxyd stone 0x48
    "st-mirror-p|t",            // PerOxyd stone 0x49
    "st-mirror-p/t",            // PerOxyd stone 0x4a
    "st-mirror-p-t",            // PerOxyd stone 0x4b
    "st-mirror-p\\t",           // PerOxyd stone 0x4c
    "st-mirror-3>",             // PerOxyd stone 0x4d
    "st-mirror-3^",             // PerOxyd stone 0x4e
    "st-mirror-3<",             // PerOxyd stone 0x4f
    "st-mirror-3v",             // PerOxyd stone 0x50
    "st-puzzle-es",             // PerOxyd stone 0x51
    "st-puzzle-sw",             // PerOxyd stone 0x52
    "st-puzzle-nw",             // PerOxyd stone 0x53
    "st-puzzle-ne",             // PerOxyd stone 0x54
    "st-puzzle-ew",             // PerOxyd stone 0x55
    "st-puzzle-ns",             // PerOxyd stone 0x56
    "st-puzzle-n",              // PerOxyd stone 0x57
    "st-puzzle-s",              // PerOxyd stone 0x58
    "st-puzzle-w",              // PerOxyd stone 0x59
    "st-puzzle-e",              // PerOxyd stone 0x5a
    "st-puzzle-nesw",           // PerOxyd stone 0x5b
    "st-puzzle-hollow",         // PerOxyd stone 0x5c
    "st-laserbreak",            // PerOxyd stone 0x5d
    "st-coffee",                // PerOxyd stone 0x5e
    "st-shogun",                // PerOxyd stone 0x5f (oxyd with a hole, movable ... strange stone! st-shogun as workaround, only link level 74)
    "st-disco-dark",            // PerOxyd stone 0x60
    "st-disco-medium",          // PerOxyd stone 0x61
    "st-bombs",                 // PerOxyd stone 0x62
    "st-flash",                 // PerOxyd stone 0x63
    "st-coinslot",              // PerOxyd stone 0x64
    "st-thief",                 // PerOxyd stone 0x65
    "st-shogun-s",              // PerOxyd stone 0x66
    "st-shogun-m",              // PerOxyd stone 0x67
    "st-shogun-l",              // PerOxyd stone 0x68
    "st-shogun-sml",            // PerOxyd stone 0x69
    "st-shogun-ml",             // PerOxyd stone 0x6a
    "st-shogun-sl",             // PerOxyd stone 0x6b
    "st-shogun-sm",             // PerOxyd stone 0x6c
    "st-stoneimpulse",          // PerOxyd stone 0x6d
    "st-lasertimeswitch",       // PerOxyd stone 0x6e
    "st-mail-n",                // PerOxyd stone 0x6f
    "st-mail-w",                // PerOxyd stone 0x70
    "st-mail-e",                // PerOxyd stone 0x71
    "st-mail-s",                // PerOxyd stone 0x72
    "st-door-h",                // PerOxyd stone 0x73
    "st-door-v",                // PerOxyd stone 0x74
    "st-metal",                 // PerOxyd stone 0x75
    "st-stonebrush",            // PerOxyd stone 0x76
    "st-door-h-open",           // PerOxyd stone 0x77
    "st-door-v-open",           // PerOxyd stone 0x78
    "st-white1",                // PerOxyd stone 0x79 (Can be either st-white1 or st-white4)
    "st-black1",                // PerOxyd stone 0x7a (Can be either st-black1 or st-black4)
    "st-metal_hole",            // PerOxyd stone 0x7b (because it looks similar ...; only link level 79)
    0,                          // PerOxyd stone 0x7c (probably complex stone, behavior unknown)(only levels: 23 142, these levels are identical!)
    "st-yinyang3",              // PerOxyd stone 0x7d
    "st-break_acwhite",         // PerOxyd stone 0x7e
    "st-break_acblack",         // PerOxyd stone 0x7f
    "st-chargeplus",            // PerOxyd stone 0x80
    "st-chargeminus",           // PerOxyd stone 0x81
    "st-chargezero",            // PerOxyd stone 0x82
    "st-bolder-n",              // PerOxyd stone 0x83
    "st-bolder-s",              // PerOxyd stone 0x84
    "st-bolder-w",              // PerOxyd stone 0x85
    "st-bolder-e",              // PerOxyd stone 0x86
    "st-rock5",                 // PerOxyd stone 0x87
    UNUSED,                     // PerOxyd stone 0x88
    UNUSED,                     // PerOxyd stone 0x89
    "st-rock6",                 // PerOxyd stone 0x8a
    "st-blue-sand",             // PerOxyd stone 0x8b
    "st-rock1",                 // PerOxyd stone 0x8c
    "st-rock2",                 // PerOxyd stone 0x8d (simple border stone, not exactly st-rock2 ...; only link level 92)
    "st-rock2",                 // PerOxyd stone 0x8e
    UNUSED,                     // PerOxyd stone 0x8f
    "st-rock8",                 // PerOxyd stone 0x90
    "st-rotator_move-left",     // PerOxyd stone 0x91
    "st-rotator_move-right",    // PerOxyd stone 0x92
    "st-swap",                  // PerOxyd stone 0x93
    "st-spitter",               // PerOxyd stone 0x94
    0,                          // PerOxyd stone 0x95 (dynamite holder, will implememnt it)(levels: 41 184 200)
    "st-rubberband",            // PerOxyd stone 0x96
    "st-scissors",              // PerOxyd stone 0x97
    "st-grate3",                // PerOxyd stone 0x98
    "st-blocker",               // PerOxyd stone 0x99
    "st-grate1",                // PerOxyd stone 0x9a
    "st-metal_hole",            // PerOxyd stone 0x9b
    "st-stone1",                // PerOxyd stone 0x9c
    "st-fart",                  // PerOxyd stone 0x9d
    "st-turnstile",             // PerOxyd stone 0x9e
    "st-turnstile-n",           // PerOxyd stone 0x9f
    "st-turnstile-s",           // PerOxyd stone 0xa0
    "st-turnstile-w",           // PerOxyd stone 0xa1
    "st-turnstile-e",           // PerOxyd stone 0xa2
    UNUSED,                     // PerOxyd stone 0xa3
    "st-likeoxydc",             // PerOxyd stone 0xa4 laserswitch on
    UNUSED,                     // PerOxyd stone 0xa5
    UNUSED,                     // PerOxyd stone 0xa6
    UNUSED,                     // PerOxyd stone 0xa7
    UNUSED,                     // PerOxyd stone 0xa8
    UNUSED,                     // PerOxyd stone 0xa9
    UNUSED,                     // PerOxyd stone 0xaa
    UNUSED,                     // PerOxyd stone 0xab
    UNUSED,                     // PerOxyd stone 0xac
    UNUSED,                     // PerOxyd stone 0xad
    UNUSED,                     // PerOxyd stone 0xae
    UNUSED,                     // PerOxyd stone 0xaf
    UNUSED,                     // PerOxyd stone 0xb0
    UNUSED,                     // PerOxyd stone 0xb1
    UNUSED,                     // PerOxyd stone 0xb2
    UNUSED,                     // PerOxyd stone 0xb3
    UNUSED,                     // PerOxyd stone 0xb4
    UNUSED,                     // PerOxyd stone 0xb5
    UNUSED,                     // PerOxyd stone 0xb6
    "st-easymode",              // PerOxyd stone 0xb7
    "st-peroxyd-0xb8",          // PerOxyd stone 0xb8
    "st-peroxyd-0xb9",          // PerOxyd stone 0xb9
    // codes >= 0xba are unused
};


ITEMMAPTYPE oxyd::peroxyd_item_map[256] = {
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
    ITEMSPEC(it_flagwhite),               // 0x18
    ITEMSPEC(it_flagblack),               // 0x19
    ITEMSPEC(it_ring),                    // 0x1a
    ITEMSPEC(it_pipe_wn),                 // 0x1b
    ITEMSPEC(it_pipe_sw),                 // 0x1c
    ITEMSPEC(it_pipe_ne),                 // 0x1d
    ITEMSPEC(it_pipe_es),                 // 0x1e
    ITEMSPEC(it_pipe_v),                  // 0x1f
    ITEMSPEC(it_pipe_h),                  // 0x20
    ITEMSPEC(it_spade),                   // 0x21
    ITEMSPEC(it_surprise),                // 0x22
    ITEMSPEC(it_pin),                     // 0x23
    ITEMSPEC(it_seed),                    // 0x24
    ITEMSPEC(it_spring2),                 // 0x25
    ITEMSPEC(it_spring1),                 // 0x26
    ITEMSPEC(it_bag),                     // 0x27
    ITEMSPEC(it_magnet_off),              // 0x28
    ITEMSPEC(it_signalfilter0),           // 0x29
    ITEMSPEC(it_signalfilter1),           // 0x2a
    ITEMSPEC(it_shogun_s),                // 0x2b
    ITEMSPEC(it_shogun_l),                // 0x2c
    ITEMSPEC(it_vortex_open),             // 0x2d
    ITEMSPEC(it_vortex_closed),           // 0x2e
    ITEMSPEC(it_wormhole_on),             // 0x2f XXX
    ITEMSPEC(it_hill),                    // 0x30
    ITEMSPEC(it_tinyhill),                // 0x31
    ITEMSPEC(it_hollow),                  // 0x32
    ITEMSPEC(it_tinyhollow),              // 0x33
    ITEMSPEC(it_vstrip),                  // 0x34
    ITEMSPEC(it_hstrip),                  // 0x35
    ITEMSPEC(it_springboard),             // 0x36
    ITEMSPEC(it_MISSING),                 // 0x37
    ITEMSPEC(it_bridge_oxyd),             // 0x38
    ITEMSPEC(it_UNUSED),                  // 0x39
    ITEMSPEC(it_UNUSED),                  // 0x3a
    ITEMSPEC(it_UNUSED),                  // 0x3b
    ITEMSPEC(it_cross),                   // 0x3c
    ITEMSPEC(it_spoon),                   // 0x3d
    ITEMSPEC(it_MISSING),                 // 0x3e rubber band
    ITEMSPEC(it_changefloor),             // 0x3f
    ITEMSPEC(it_trigger),                 // 0x40
    ITEMSPEC(it_brush),                   // 0x41
    ITEMSPEC(it_banana),                  // 0x42
    ITEMSPEC(it_pencil),                  // 0x43
    ITEMSPEC(it_brake),                   // 0x44
    ITEMSPEC(it_squashed),                // 0x45
    ITEMSPEC(it_blocker),                 // 0x46
    ITEMSPEC(it_magicwand),               // 0x47
    ITEMSPEC(it_wrench),                  // 0x48
    ITEMSPEC(it_UNUSED),                  // 0x49
    ITEMSPEC(it_odometer),                // 0x4a
    ITEMSPEC(it_puller_n),                // 0x4b
    ITEMSPEC(it_puller_s),                // 0x4c
    ITEMSPEC(it_puller_w),                // 0x4d
    ITEMSPEC(it_puller_e),                // 0x4e
    ITEMSPEC(it_UNUSED),                  // 0x4f
    ITEMSPEC(it_UNUSED),                  // 0x50
    ITEMSPEC(it_MISSING),                 // 0x51 puller left, active
    ITEMSPEC(it_UNUSED),                  // 0x52
    ITEMSPEC(it_MISSING),                 // 0x53 oxyd on floor (?)
    ITEMSPEC(it_UNUSED),                  // 0x54
    ITEMSPEC(it_UNUSED),                  // 0x55
    ITEMSPEC(it_UNUSED),                  // 0x56
    ITEMSPEC(it_UNUSED),                  // 0x57
    ITEMSPEC(it_MISSING),                 // 0x58 oxyd on floor
    ITEMSPEC(it_MISSING),                 // 0x59 oxyd on floor
    ITEMSPEC(it_UNUSED),                  // 0x5a
    ITEMSPEC(it_UNUSED),                  // 0x5b
    ITEMSPEC(it_UNUSED),                  // 0x5c
    ITEMSPEC(it_UNUSED),                  // 0x5d
    ITEMSPEC(it_UNUSED),                  // 0x5e
    ITEMSPEC(it_UNUSED),                  // 0x5f
    ITEMSPEC(it_UNUSED),                  // 0x60
    ITEMSPEC(it_UNUSED),                  // 0x61
    ITEMSPEC(it_UNUSED),                  // 0x62
    ITEMSPEC(it_UNUSED),                  // 0x63
    ITEMSPEC(it_UNUSED),                  // 0x64
    ITEMSPEC(it_UNUSED),                  // 0x65
    ITEMSPEC(it_UNUSED),                  // 0x66
    ITEMSPEC(it_UNUSED),                  // 0x67
    ITEMSPEC(it_UNUSED),                  // 0x68
    ITEMSPEC(it_UNUSED),                  // 0x69
    ITEMSPEC(it_blackbomb_burning),       // 0x6a
    ITEMSPEC(it_UNUSED),                  // 0x6b
    ITEMSPEC(it_UNUSED),                  // 0x6c
    ITEMSPEC(it_UNUSED),                  // 0x6d
    ITEMSPEC(it_UNUSED),                  // 0x6e
    ITEMSPEC(it_UNUSED),                  // 0x6f
    ITEMSPEC(it_UNUSED),                  // 0x70
    ITEMSPEC(it_UNUSED),                  // 0x71
    ITEMSPEC(it_UNUSED),                  // 0x72
    ITEMSPEC(it_UNUSED),                  // 0x73
    ITEMSPEC(it_UNUSED),                  // 0x74
    ITEMSPEC(it_UNUSED),                  // 0x75
    ITEMSPEC(it_easykillstone),           // 0x76
    ITEMSPEC(it_easykeepstone),           // 0x77
    ITEMSPEC(it_2pkillstone),             // 0x78
    ITEMSPEC(it_1pkillstone),             // 0x79
    // codes >= 0x7a are unused
};

