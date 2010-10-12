/*
 * Copyright (C) 2002,2003,2004 Daniel Heck
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

/*
 * This files contains the interface between Enigma and the original
 * Oxyd games.  It is responsible for converting level descriptions or
 * sound effects.
 */

#include "errors.hh"
#include "oxyd.hh"
#include "sound.hh"
#include "lua.hh"
#include "server.hh"
#include "player.hh"
#include "main.hh"

#include <cstdio>
#include <cstdlib>
#include <iostream>

#include "oxyd_internal.hh"

#define SOUND sound::GetEngine()

using namespace std;
using namespace enigma;
using world::Stone;
using world::MakeStone;
using world::Item;
using world::MakeItem;

using namespace oxyd;
using OxydLib::Level;


/* -------------------- Helper functions -------------------- */

namespace 
{
    enigma::Direction
    direction_oxyd2enigma (OxydLib::Direction odir)
    {
        switch (odir) {
        case Direction_Up:    return NORTH; break;
        case Direction_Down:  return SOUTH; break;
        case Direction_Left:  return WEST; break;
        case Direction_Right: return EAST; break;
        default :
            fprintf(stderr, "Unknown OxydLib-direction %i!\n", int(odir));
            return NODIR;
        }
    }

    GameType to_gametype (OxydLib::OxydVersion ver) 
    {
        GameType typ = GAMET_UNKNOWN;
        switch (ver) {
        case OxydVersion_Oxyd1: typ = GAMET_OXYD1; break;
        case OxydVersion_OxydMagnum:
        case OxydVersion_OxydMagnumGold: typ = GAMET_OXYDMAGNUM; break;
        case OxydVersion_OxydExtra: typ = GAMET_OXYDEXTRA; break;
        case OxydVersion_PerOxyd: typ = GAMET_PEROXYD; break;
        default :
            assert(0);
            break;
        }
        return typ;
    }


    void dumpUnknownObjects (const OxydLib::Level& level) 
    {
        set<int> stones, items, floors;

        const Grid &sgrid = level.getGrid (GridType_Pieces);
        for (unsigned y=0; y<sgrid.getHeight(); ++y)
            for (unsigned x=0; x<sgrid.getWidth(); ++x)
                if (Stone *st = world::GetStone(GridPos(x, y)))
                    if (int code = st->int_attrib("code"))
                        stones.insert(code);

        const Grid &igrid = level.getGrid (GridType_Objects);
        for (unsigned y=0; y<igrid.getHeight(); ++y)
            for (unsigned x=0; x<igrid.getWidth(); ++x)
                if (Item *it = world::GetItem(GridPos(x, y)))
                    if (int code = it->int_attrib("code"))
                        items.insert(code);

        const Grid &fgrid = level.getGrid (GridType_Objects);
        for (unsigned y=0; y<fgrid.getHeight(); ++y)
            for (unsigned x=0; x<fgrid.getWidth(); ++x)
                if (world::Floor *fl = world::GetFloor(GridPos(x, y)))
                    if (int code = fl->int_attrib("code"))
                        floors.insert(code);

        if (!stones.empty()) {
            enigma::Log << "Unknown stones:";
            for (set<int>::iterator i = stones.begin(); i != stones.end(); ++i)
                enigma::Log << ' ' << *i;
            enigma::Log << endl;
        }
        if (!items.empty()) {
            enigma::Log << "Unknown items:";
            for (set<int>::iterator i = items.begin(); i != items.end(); ++i)
                enigma::Log << ' ' << *i;
            enigma::Log << endl;
        }
        if (!floors.empty()) {
            enigma::Log << "Unknown floors:";
            for (set<int>::iterator i = floors.begin(); i != floors.end(); ++i)
                enigma::Log << ' ' << *i;
            enigma::Log << endl;
        }
    }

    GridLoc to_gridloc (const SignalLocation &a)
    {
        assert (a.getGridType() >= GridType_First &&
                a.getGridType() <= GridType_Last);
        static GridLayer tab[3] = { GRID_FLOOR, GRID_STONES, GRID_ITEMS };
        return GridLoc (tab[a.getGridType()], GridPos(a.getX(), a.getY()));
    }

    string patchfile_name (enigma::GameType t, size_t index, bool twoplayers)
    {
        string patchfile = "levels/patches/";
        switch (t) {
        case GAMET_OXYD1:      patchfile += "ox1_"; break;
        case GAMET_PEROXYD:    patchfile += "pox_"; break;
        case GAMET_OXYDMAGNUM: patchfile += "oxm_"; break;
        case GAMET_OXYDEXTRA:  patchfile += "oxe_"; break;
        default:
            assert (0);
        }
        if (twoplayers) 
            patchfile += ecl::strf ("%03d.lua", index+101);
        else
            patchfile += ecl::strf ("%03d.lua", index+1);
    
        return patchfile;
    }

    bool parse_gridpos (CommandString &cs, int levelw, int levelh, GridPos &p)
    {
        int pos = cs.get_int (0, levelw * levelh - 1, -1);
        if (pos != -1) {
            p.x = pos % levelw;
            p.y = pos / levelw;
            return true;
        }
        return false;
    }

}


/* -------------------- CommandString implementation -------------------- */

CommandString::CommandString(const string &cmd) 
: m_cmd(cmd),
  m_iter (m_cmd.begin())
{}

int CommandString::get_char () {
    if (m_iter == m_cmd.end())
        return 0;
    return *m_iter++;
}

bool CommandString::get_bool (bool dflt) {
    int c = get_char();
    if (c == 0xf8 || c == 0xf9)
        return  (c == 0xf9);
    return dflt;
}


int CommandString::get_int (int min, int max, int dflt)
{
    int val = 0;
    bool error = false;
    int c = get_char();
    if (c != '(') 
        error = true;
    else {
        bool positive = true;
        c = get_char(); 
        if (c == '+') {
            c = get_char();
        }
        else if (c == '-') {
            positive = false;
            c = get_char();
        }
        if (isdigit(c)) {
            val = 0;
            do {
                val = 10*val + (c - '0');
                c = get_char();
            } while (isdigit(c));
            if (!positive)
                val = -val;
        }
        else
            error = true;
    }
    return (error || val > max || val < min) ? dflt : val;
}


/* -------------------- OxydLoader -------------------- */

OxydLoader::OxydLoader (const OxydLib::Level &level_, 
                        const LoaderConfig config_)
: level (level_),
  config (config_)
{
    harmless_medi = false;
}

void OxydLoader::load ()
{
    // Prepare Enigma game engine
    world::Resize (level.getWidth(), level.getHeight());
    if (config.twoplayers) 
        server::TwoPlayerGame = true;
    display::ResizeGameArea (20, 11);
    if (level.getScrolling())
        SetFollowMode (display::FOLLOW_SCROLLING);
    else
        SetFollowMode (display::FOLLOW_SCREEN);

    // Populate Enigma game 
    load_floor ();
    load_items ();
    load_stones ();
    scramble_puzzles ();

    load_actors ();
    connect_signals ();
    connect_rubberbands ();

    parse_specials ();

    // Debugging output
    dumpUnknownObjects(level);
}

Stone * OxydLoader::make_laser (int type)
{
    ecl::Assert<XLevelLoading> 
        (type >= 0 && type <= 2,
         "Oxyd supports only three different lasers per level");

    const Laser& laser = level.getLaser(type);

    enigma::Direction dir = direction_oxyd2enigma(laser.getDir());
    bool              on  = laser.getOn();

    Stone *st = 0;
    if (dir != NODIR) {
        string lasername("st-laser");
        lasername += to_suffix(dir);
        st         = MakeStone(lasername.c_str());
        st->set_attrib("on", Value(on)); // OnOffStone attribute
    }
    return st;
}

Stone *OxydLoader::make_timer (int x, int y)
{
    const OscillatorMap &oscillators = level.getOscillators(config.gamemode);
    Stone *st = world::MakeStone ("st-timer");
    st->set_attrib("interval", Value(0.2));

    Block block(x, y);
    OscillatorMap::const_iterator i = oscillators.find(block);
    if (i != oscillators.end()) {
        const Oscillator &o = i->second;
        st->set_attrib("on", Value(o.getOn()));
        double interval = (1 + o.getPeriod()) * config.timer_factor;
        st->set_attrib("interval", Value(interval));
    }
    return st;
}

Stone *OxydLoader::make_stone (int type, int x, int y)
{
    world::Stone *st = 0;

    if (type == 0) {
        // ignore
    }
    else if (type >= 0x01 && type <= 0x10) {
        // Oxyd stones of different colors
        char color[2] = "0";
        color[0] += (type-1) / 2; 

        st = world::MakeStone("st-oxyd");
        st->set_attrib("color", color);
        st->set_attrib("flavor", config.oxyd_flavor);
    }
    else if (type == config.id_timer) {
        st = make_timer (x, y);
    }
    else if (type >= config.id_laser1 && type < config.id_laser1 + 3) {
        st = make_laser (type - config.id_laser1);
    }
    else {
        // No special case -> get Stone from map
        const char *name = config.stonetable[type];
        if (name == 0) {
            Log << ecl::strf("Unknown stone %X\n", type);
            st = world::MakeStone ("st-dummy");
            st->set_attrib("code", type);
        }
        else if (name[0] != '\0') { // ignore if name==""
            st = world::MakeStone (name);
        }
    }
    return st;
}

static std::string convert_encoding (const std::string &t)
{
    std::string tt = t;
    for (size_t i = 0; i<tt.length(); ++i) {
	unsigned char c = tt[i];
        switch (c) {
        case 129: tt.replace (i, 1, "\303\274"), i++; break; // �
        case 130: tt.replace (i, 1, "\303\251"), i++; break; // �
        case 132: tt.replace (i, 1, "\303\244"), i++; break; // �
        case 133: tt.replace (i, 1, "\303\240"), i++; break; // � 
        case 136: tt.replace (i, 1, "\303\252"), i++; break; // �
        case 138: tt.replace (i, 1, "\303\250"), i++; break; // �
        case 142: tt.replace (i, 1, "\303\204"), i++; break; // �
        case 145: tt.replace (i, 1, "\302\251"), i++; break; // �
        case 148: tt.replace (i, 1, "\303\266"), i++; break; // �
        case 150: tt.replace (i, 1, "\303\273"), i++; break; // �
        case 151: tt.replace (i, 1, "\303\271"), i++; break; // �
        case 154: tt.replace (i, 1, "\303\234"), i++; break; // �
        case 158: tt.replace (i, 1, "\303\237"), i++; break; // �
        default:
            if (c > 128)
                Log << "Unknown character in Oxyd text: " << int(c) << ' ';
            break;
        }
    }
    Log << ">> '" << t << "'\n";
    Log << ">> '" << tt << "'\n";
    return tt;
}

Item  *OxydLoader::make_item (int type)
{
    using namespace world;

    Item *it = 0;

    OxydLib::Language lang = Language_English;
    std::string localelang = ecl::GetLanguageCode (ecl::SysMessageLocaleName());
    if (localelang == "de")
	lang = Language_German;
    else if (localelang == "fr")
	lang = Language_French;
    
    switch (type) {
    case 0x00: break;           // ignore
    case 0x02:                  // note 1
	{
	    it = MakeItem (world::it_document);
	    string text = convert_encoding(level.getNoteText(0, lang));
	    it->set_attrib ("text", text.c_str());
	}
        break;
    case 0x03:                  // note 2
	{
	    it = MakeItem (world::it_document);
	    string text = convert_encoding(level.getNoteText(1, lang));
	    it->set_attrib ("text", text.c_str());
	}
	break;
    default:
        {
            ItemID id = config.itemtable[type];
            if (id == it_INVALID) {
                Log << ecl::strf ("Unknown item %X\n",type);
                it = MakeItem (world::it_dummy);
                it->set_attrib("code", type);
            }
            else
                it = MakeItem (id);
        }
    }
    return it;
}


void OxydLoader::load_floor () 
{
    using namespace world;

    const Grid &grid = level.getGrid (GridType_Surfaces);
    for (unsigned y=0; y<grid.getHeight(); ++y) {
        for (unsigned x=0; x<grid.getWidth(); ++x) {
            int         code = grid.get(x,y);
            const char *name = config.floortable[code];
            Floor      *fl;

            if( name == 0) {
                Log << ecl::strf ("Unknown floor %X\n",code);
                fl = MakeFloor("fl-dummy");
                fl->set_attrib("code", code);
            }
            else {
                fl = MakeFloor(name);
            }

            SetFloor (GridPos(x, y), fl);
        }
    }
}

void OxydLoader::load_items () 
{
    const Grid &grid = level.getGrid (GridType_Objects);
    for (unsigned y=0; y<grid.getHeight(); ++y)
        for (unsigned x=0; x<grid.getWidth(); ++x)
            if (Item *it = make_item (grid.get(x,y)))
                world::SetItem (GridPos(x, y), it);
}

void OxydLoader::load_stones()
{
    using namespace world;

    const Grid &grid = level.getGrid (GridType_Pieces);
    for (unsigned y=0; y<grid.getHeight(); ++y) {
        for (unsigned x=0; x<grid.getWidth(); ++x) {
            if (Stone *st = make_stone(grid.get(x,y), x, y))
                SetStone (GridPos(x, y), st);
        }
    }

    SendMessage(GetObjectTemplate("st-oxyd"), "shuffle");
}

void OxydLoader::scramble_puzzles ()
{
    int count = level.getNumScrambleItems();

    for (int i = 0; i<count; ++i) {
        const ScrambleItem& si = level.getScrambleItem(i);
        world::AddScramble (GridPos(si.getX(), si.getY()),
                            direction_oxyd2enigma(si.getDir()));
    }
}


void OxydLoader::load_actors () 
{
    using world::MakeActor;

    int     nmeditationmarbles = 0;
    size_t  nmarbles           = level.getNumMarbles();
    bool    have_black_marble  = false;

    for (size_t i=0; i<nmarbles; ++i)
        if (level.getMarble(i).getMarbleType() == MarbleType_Black)
            have_black_marble                   = true;


    for (size_t i=0; i<nmarbles; ++i) {
        const Marble &marble = level.getMarble(i);
        double        x      = marble.getX()/32.0;
        double        y      = marble.getY()/32.0;
        Actor        *ac     = 0;

        MarbleInfo minfo(marble);

        switch (marble.getMarbleType()) {
        case MarbleType_Black:
            ac = MakeActor (world::ac_blackball);
            ac->set_attrib ("player", Value(0.0));
            break;
        case MarbleType_White:
            ac = MakeActor (world::ac_whiteball);
            ac->set_attrib ("player", Value(1.0));
            break;
        case MarbleType_Meditation:
            if (have_black_marble && !level.getHarmlessMeditationMarbles()) {
                // # example: Oxyd Extra #28
                ac = MakeActor (world::ac_killerball);
//                ac->set_attrib ("player", Value(0.0));
                ac->set_attrib ("mouseforce", Value (1.0));
                ac->set_attrib ("controllers", Value (3.0));
            }
            else {
                ac = MakeActor (world::ac_meditation);
                nmeditationmarbles += 1;

                if (config.twoplayers && (nmeditationmarbles % 2) == 0)
                    ac->set_attrib("player", Value(1.0));
                else
                    ac->set_attrib ("player", Value(0.0));
            }

            if (minfo.is_default(MI_FORCE)) {
                ac->set_attrib("mouseforce", Value(1.0));
            }
            else {
                ac->set_attrib("mouseforce", Value(minfo.get_value(MI_FORCE) / 32.0)); // just a guess
            }
            break;
        case MarbleType_Jack:
            ac = MakeActor (world::ac_top);
            if (!minfo.is_default(MI_FORCE)) {
                double force = minfo.get_value(MI_FORCE) / 4; // just a guess
                ac->set_attrib("force", Value(force) );
                enigma::Log << "Set jack force to " << force << endl;
            }
            if (!minfo.is_default(MI_RANGE)) {
                double range = minfo.get_value(MI_RANGE) / 32.0; // value seems to contain distance in pixels
                ac->set_attrib("range", Value(range) );
                enigma::Log << "Set jack range to " << range << endl;
            }
            break;

        case MarbleType_Rotor: {
            ac = MakeActor (world::ac_rotor);

            double force = minfo.get_value (MI_FORCE, 30) * 0.3;
            double range = minfo.get_value (MI_RANGE, 100) / 32.0;
            int gohome = minfo.get_value (MI_GOHOME, 1);

            ac->set_attrib ("force", force);
            ac->set_attrib ("range", range);
            ac->set_attrib ("gohome", Value (gohome));

            enigma::Log << "rotor force " << force << endl;
            enigma::Log << "rotor range " << range << endl;
            
            break;
        }

        case MarbleType_Horse: {
            ac = MakeActor (world::ac_horse);
            int levelw = level.getWidth();
            if (!minfo.is_default(MI_HORSETARGET1)) {
                int targetpos = minfo.get_value(MI_HORSETARGET1);
                ac->set_attrib("target1", ecl::strf("%d %d", 
                                                   targetpos % levelw, 
                                                   int(targetpos / levelw)));
            }
            if (!minfo.is_default(MI_HORSETARGET2)) {
                int targetpos = minfo.get_value(MI_HORSETARGET2);
                ac->set_attrib("target2", ecl::strf("%d %d", 
                                                   targetpos % levelw, 
                                                   int(targetpos / levelw)));
            }
            if (!minfo.is_default(MI_HORSETARGET3)) {
                int targetpos = minfo.get_value(MI_HORSETARGET3);
                ac->set_attrib("target3", ecl::strf("%d %d", 
                                                   targetpos % levelw, 
                                                   int(targetpos / levelw)));
            }
            if (!minfo.is_default(MI_HORSETARGET4)) {
                int targetpos = minfo.get_value(MI_HORSETARGET4);
                ac->set_attrib("target4", ecl::strf("%d %d", 
                                                   targetpos % levelw, 
                                                   int(targetpos / levelw)));
            }
            break;
        }
        case MarbleType_Bug:
            ac = MakeActor (world::ac_bug);
            break;
        default:
            enigma::Log << "Unhandled actor type " << int(marble.getMarbleType()) << endl;
            break;
//         case MarbleType_LifeSpitter:
//         case MarbleType_DynamiteHolder:
//             break;
        }

        if (ac) 
            world::AddActor (x, y, ac);

        m_actors.push_back (ac);
    }
}

void OxydLoader::connect_rubberbands () 
{
    GameMode game_mode = config.gamemode;
    int num_rubberbands = level.getNumRubberBands(game_mode);
    for (int i=0; i<num_rubberbands; ++i) {
        const RubberBand &rb = level.getRubberBand(game_mode, i);

        world::Actor *actor = get_actor (rb.getFirstEndMarble());
        world::RubberBandData rbd;
        rbd.length = rb.getNaturalLength() / 32.0;
        rbd.strength = rb.getForce() / 60.0;
        if (rb.isSecondEndMarble()) {
            world::Actor *actor2 = get_actor (rb.getSecondEndMarble());
            world::AddRubberBand (actor, actor2, rbd);
        }
        else {
            GridPos p(rb.getSecondEndPieceX(), rb.getSecondEndPieceY());
            if (world::GetStone(p) != NULL) // Fix for MagnumGold Level #108
                world::AddRubberBand (actor, world::GetStone(p), rbd); 
        }
    }
}

void OxydLoader::connect_signals () 
{
    using namespace world;

    set<SignalLocation> senders;
    level.getSenders(&senders);
    set<SignalLocation>::const_iterator senderIter = senders.begin();
    set<SignalLocation>::const_iterator senderEnd = senders.end();
    for (; senderIter != senderEnd; ++senderIter) {
        const SignalLocation &sender = *senderIter;

        int nrec = level.getNumRecipients(sender);
        for (int irec=0; irec<nrec; ++irec) {
            SignalLocation recipient = level.getRecipient(sender, irec);
            GridLoc src = to_gridloc(sender);
            GridLoc dst = to_gridloc(recipient);
            world::AddSignal (src, dst, "signal");
        }
    }
}

void OxydLoader::parse_specials ()
{
    using world::SendMessage;

    server::FlatForce = 5.0 * level.getFlatForce (config.gamemode);

    GridPos p;
    const std::vector<std::string> &special = level.getSpecialItems();
    vector<int> numberlist;
    int levelw = level.getWidth();
    int levelh = level.getHeight();
    for (size_t i=0; i<special.size(); ++i) {
        CommandString cmd(special[i]);
        string args=special[i].substr(1);
        switch (cmd.get_char()) {
        case 'y':
            ; // use black and white stones with hole
            break;

        case 'M':
            // seems to be present in all levels with movable 'st-plain's.
            break;

        case 'm':               // magnet force
            server::MagnetForce = cmd.get_int (0, 32000, 3000) / 1000.0;
            break;

        case 't':           // wormhole force and range
            server::WormholeForce = cmd.get_int(0,32000,3000) / 5000.0;
            server::WormholeRange = cmd.get_int(0,32000,100) / 32.0;
            break;

        case 'R':           // brittleness
            server::Brittleness = 1 - cmd.get_int (0, 32000, 2)/32000.0;
            break;

        case 'i':           // turn item on
            if (parse_gridpos (cmd, levelw, levelh, p))
                SendMessage(world::GetItem(p), "signal", Value(1.0));
            break;

        case 'o':           // turn item off
            if (parse_gridpos (cmd, levelw, levelh, p))
                SendMessage(world::GetItem(p), "signal", Value(0.0));
            break;

        case 'I':           // turn stone on
            if (parse_gridpos (cmd, levelw, levelh, p))
                SendMessage(world::GetStone(p), "signal", Value(1.0));
            break;

        case 'O':           // turn stone off
            if (parse_gridpos (cmd, levelw, levelh, p))
                SendMessage(world::GetStone(p), "signal", Value(0.0));
            break;

        case 'Q':           // default charge
            server::ElectricForce = cmd.get_int(0,30, 6) * 2.5;
            break;

        case 'T':
            server::HoleForce = cmd.get_int (0, 4000, 40) / 40.0;
            break;

        default:
            Log << "special " << i << ": '" << special[i] << "'\n";
            break;
        }
    }
}

world::Actor *OxydLoader::get_actor (int idx) 
{
    assert (0 <= idx && unsigned(idx) <= m_actors.size());
    return m_actors[idx];
}




/* -------------------- LevelPack_Oxyd -------------------- */

LevelPack_Oxyd::LevelPack_Oxyd (OxydVersion ver, DatFile *dat, 
        int idx_start, int idx_end, bool twoplayers) : 
        m_datfile (dat), m_twoplayers (twoplayers), m_index_start (idx_start) {
    nlevels = 0;
    for (int i = idx_start; i <= idx_end; i++) {
        if (!m_datfile->getLevel(i).empty()) {
            level_index[nlevels] = i;
            
            char txt[5];
            snprintf(txt, sizeof(txt), "%d", nlevels);
            lev::Proxy * aProxy = lev::Proxy::registerLevel(
                     "#" + get_name() + "#" + txt,  "#" + get_name(),
                    ecl::strf ("Import %s %d", get_name().c_str(), nlevels), 
                    ecl::strf ("%s #%d", get_name().c_str(), nlevels+1), 
                    "Dongleware", 1, 1, has_easymode(i), get_gametype());
            proxies.push_back(aProxy);
            nlevels++;
        }
    }
    // post initialization of superclass Index based on dat file values
    indexName = get_name();
    indexGroup = defaultGroup = "Dongleware";
    indexDefaultLocation = get_default_location();
    indexLocation = indexDefaultLocation;
    Log << "Levelpack '" << get_name() << "' has " << nlevels << " levels." << endl;
}

const char* LevelPack_Oxyd::get_default_SoundSet() const 
{ 
    return sound::GetOxydSoundSet(m_datfile->getVersion()).c_str();
}

bool LevelPack_Oxyd::needs_twoplayers() const 
{
    return m_twoplayers;
}

bool LevelPack_Oxyd::has_easymode(size_t index) const {
     if (get_version() != OxydVersion_PerOxyd)
         return false;
     else 
        return LP_PerOxyd::hasEasymode(index);
}

GameType LevelPack_Oxyd::get_gametype() const 
{
    return to_gametype(m_datfile->getVersion());
}

GameMode LevelPack_Oxyd::get_gamemode() const 
{
    return (server::GetDifficulty() == DIFFICULTY_EASY 
            ? GameMode_Easy : GameMode_Hard);
}

string LevelPack_Oxyd::get_name() const 
{
    static const char *names1p[] = {
        "Oxyd 1", "Oxyd magnum", "Magnum Gold", "Per.Oxyd", "Oxyd extra"
    };
    static const char *names2p[] = {
        "Oxyd 1 (2p)", "", "", "Per.Oxyd (2p)", "Oxyd extra (2p)"
    };
    OxydVersion v = get_version();
    return level_index[0]>99 ? names2p[v] : names1p[v];
}

int LevelPack_Oxyd::get_default_location() const 
{
    static const int location1p[] = {
        90100, 90700, 90800, 90300, 90500
    };
    static const int location2p[] = {
        90200, 0, 0, 90400, 90600
    };
    OxydVersion v = get_version();
    return level_index[0]>99 ? location2p[v] : location1p[v];
}

void LevelPack_Oxyd::load_oxyd_level (size_t index) 
{
    ecl::Assert <XLevelLoading> (index < size(), "Invalid level index");

    // Prepare level data
    string msg;
    Level  level;
    if (!parseLevel (m_datfile->getLevel(level_index[index]), &level, &msg)) {
        throw XLevelLoading(msg);
    }

    // Load level
    load (level);

    // Apply patch file
    string patchname = patchfile_name (get_gametype(), index, m_twoplayers);
    string patchfile;
    if (app.resourceFS->findFile (patchname, patchfile)) {
        if (lua::Dofile (lua::LevelState(), patchname) != 0) {
            string err = string("While executing '")+patchname+"':\n"+lua::LastError(lua::LevelState());
            throw XLevelLoading(err);
        }
    }
}


/* -------------------- Oxyd extra level pack -------------------- */

LP_OxydExtra::LP_OxydExtra (DatFile *dat)
: LevelPack_Oxyd (OxydVersion_OxydExtra, dat, 0, 99, false)
{
}


void LP_OxydExtra::load (const OxydLib::Level &level)
{
    LoaderConfig c (needs_twoplayers(), 
                    get_gamemode(), 
                    oxydextra_floor_map, 
                    oxydextra_item_map,
                    oxydextra_stone_map);
//     c.id_timer  = 0x2e;
    c.id_laser1 = 0x3e;
    PerOxydLoader (level, c).load();
}



/* -------------------- Oxyd magnum level pack -------------------- */

LP_OxydMagnum::LP_OxydMagnum(OxydVersion version, DatFile *dat)
: LevelPack_Oxyd (version, dat, 0, 
                  (version==OxydVersion_OxydMagnumGold) ? 120 : 99, 
                  false)
{
}

void LP_OxydMagnum::load (const OxydLib::Level &level)
{
    LoaderConfig c (needs_twoplayers(), 
                    get_gamemode(), 
                    oxydmag_floor_map, 
                    oxydmag_item_map,
                    oxydmag_stone_map);
    c.id_timer  = 0x33;
    c.id_laser1 = 0x44;
    PerOxydLoader (level, c).load();

    // Add a yinyang item if a white marble is present
    if (world::CountActorsOfKind (world::ac_whiteball) > 0)
        player::AddYinYang();
}


/* -------------------- MarbleInfo -------------------- */

MarbleInfo::MarbleInfo (const Marble& marble) 
{
    const string& data = marble.getData(server::GetDifficulty() == DIFFICULTY_EASY 
                                        ? GameMode_Easy : GameMode_Hard);
    size_t        from = 0;
    int           idx  = 0;

    while (from != string::npos) {
        size_t par_open = data.find('(', from);
        from            = string::npos;

        if (par_open != string::npos) {
            size_t par_close = data.find(')', par_open);
            if (par_close != string::npos) {
                from = par_close;
                if (par_close == par_open+1) {
                    value[idx++] = DEFAULT_VALUE;
                }
                else {
                    value[idx++] = atoi(data.substr(par_open+1, par_close-par_open-1).c_str());
                }
            }
            else {
                Log << "Error in MarbleInfo: missing closing parenthesis" << endl;
            }
        }
    }
    for (; idx<MAX_MARBLE_INFO_FIELDS; ++idx)
        value[idx] = DEFAULT_VALUE;

    for (idx = 0; idx<MAX_MARBLE_INFO_FIELDS; ++idx)
        interpreted[idx] = false;
}

MarbleInfo::~MarbleInfo() {
    for (int idx = 0; idx<MAX_MARBLE_INFO_FIELDS; ++idx) {
        if (!interpreted[idx] && !is_default(idx)) {
            enigma::Log << "MarbleInfo[" << idx << "]=" 
                        << get_value(idx) << " is not used yet." << endl;
        }
    }
}



/* -------------------- GameInfo -------------------- */

GameInfo::GameInfo()
: ver(OxydVersion_Invalid), datfile(0), m_present(false)
{}

GameInfo::~GameInfo() {
	delete datfile;
}


GameInfo::GameInfo (OxydVersion ver_, const string &game_, const string &datfile_name_, const bool searchDAT)
: ver(ver_), game(game_), datfile(0), /*datfile_name(datfile_name_), */m_present(false)
{
    string alt_datfile_name = "levels/legacy_dat/" + datfile_name_;
    string fname;
    if (searchDAT && app.resourceFS->findFile (datfile_name_, datfile_path) ||
            app.resourceFS->findFile (alt_datfile_name, datfile_path)) {
        enigma::Log << "Found " << game << " data file\n";
        m_present = true;
        openDatFile();

        if (m_present) {
            lev::Index::registerIndex(makeLevelIndex(false));
            lev::Index::registerIndex(makeLevelIndex(true));
        }
    }
}

void GameInfo::openDatFile()
{
    assert(m_present);

    OxydLib::ByteVec data;
    readFile (datfile_path, &data);

    datfile  = new DatFile;

    string errmsg;
    if (!parseDatFile (data, ver, datfile, &errmsg)) {
        enigma::Log << "Error loading " << datfile_path << ": " << errmsg << endl;
        delete datfile;
        datfile    = 0;
        m_present = false;
    } else {
        enigma::Log << "Loaded "<< datfile_path << endl;
    }
}

lev::Index *GameInfo::makeLevelIndex(bool twoplayers)
{
    if (datfile == 0)
        return 0;

    if (twoplayers && (ver == OxydVersion_OxydExtra ||
                       ver == OxydVersion_OxydMagnum ||
                       ver == OxydVersion_OxydMagnumGold))
    {
        return 0;           // no twoplayer levels available
    }

    switch (ver) {
    case OxydVersion_Oxyd1:
        return new LP_Oxyd1 (datfile, twoplayers);
    case OxydVersion_OxydExtra:
        return new LP_OxydExtra(datfile);
    case OxydVersion_PerOxyd:
        return new LP_PerOxyd (datfile, twoplayers);
    case OxydVersion_OxydMagnum:
    case OxydVersion_OxydMagnumGold:
        return new LP_OxydMagnum (ver, datfile);

    default:
        return 0;
    }
}


/* -------------------- Local variables -------------------- */

namespace
{
    vector<GameInfo*> games;
}



/* -------------------- Functions -------------------- */

void oxyd::Init(bool searchDAT) 
{
    games.clear();
    games.resize(OxydVersion_Count);

    games[OxydVersion_Oxyd1] 
        = new GameInfo (OxydVersion_Oxyd1,          "Oxyd 1",           "oxyd1ibm.dat", searchDAT);
    games[OxydVersion_OxydMagnum]     
        = new GameInfo (OxydVersion_OxydMagnum,     "Oxyd magnum",      "oxydmibm.dat", searchDAT);
    games[OxydVersion_OxydMagnumGold] 
        = new GameInfo(OxydVersion_OxydMagnumGold, "Oxyd magnum gold", "oxydmgg.dat", searchDAT);
    games[OxydVersion_OxydExtra]      
        = new GameInfo(OxydVersion_OxydExtra,      "Oxyd extra",       "oxydex.dat", searchDAT);
    games[OxydVersion_PerOxyd]        
        = new GameInfo(OxydVersion_PerOxyd,        "Per.Oxyd",         "peroxyd.dat", searchDAT);
}

void oxyd::Shutdown()
{
    ecl::delete_sequence(games.begin(), games.end());
}

bool oxyd::FoundOxyd (OxydVersion ver) {
    return games[ver]->is_present();
}

bool oxyd::InitOxydSoundSet(OxydVersion ver)
{
    GameInfo&   gi  = *(games[ver]);

    if (!gi.is_present())
        return false;                 // not installed -> use enigma soundset

    static const char *oxydsounds[] = {
        "OXBLOOP.SDD", "OXBOING.SDD", "OXBOLD.SDD", "OXCRACK.SDD",
        "OXCRASH1.SDD", "OXCRASH2.SDD", "OXCUT.SDD", "OXDROP.SDD",
        "OXEXIT.SDD", "OXFINITO.SDD", "OXINTRO.SDD", "OXINVENT.SDD", 
        "OXINVROT.SDD", "OXJUMP.SDD", "OXKLICK1.SDD", "OXKLICK2.SDD", 
        "OXKLICK3.SDD", "OXKLICK4.SDD", "OXKLICK5.SDD", "OXKLICK6.SDD",
        "OXKLIRR.SDD", "OXLASER.SDD", "OXMAGIC.SDD", "OXMAGIC2.SDD",
        "OXMAGIC3.SDD", "OXMAGIC4.SDD", "OXMATSCH.SDD", "OXMEMCL.SDD",
        "OXMEMOK.SDD", "OXMEMOP.SDD", "OXMONEY.SDD", "OXMOTOR.SDD", 
        "OXMOVE.SDD", "OXPULLER.SDD", "OXSWOFF.SDD", "OXSWON.SDD", 
        "OXTHIEF.SDD", "OXTRANS.SDD", "OXTURN.SDD", "OXUNTITL.SDD", 
        "OXWOUOU.SDD", 0
    };

    OxydLib::DatFile *datfile = gi.getDatfile();
    for (int i=0; oxydsounds[i]; ++i) {
        string chunkname = oxydsounds[i];
        const OxydLib::ByteVec *snddata = datfile->getChunk(chunkname);

        if (snddata) {
            //enigma::Log << "Loaded sound file " << chunkname<< "\n";

            sound::SoundData snd;
            snd.buf.assign (snddata->begin(), snddata->end() - 4);
//            snd.buf        = *snddata;
            snd.freq       = 6274;
            snd.signedp    = true;
            snd.samplesize = 1;
            snd.nchannels  = 1;
            sound::DefineSound (oxydsounds[i], snd);
        }
    }

    return true;
}
