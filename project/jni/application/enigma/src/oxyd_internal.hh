/*
 * Copyright (C) 2003,2004 Daniel Heck
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
#ifndef OXYD_INTERNAL_HH
#define OXYD_INTERNAL_HH

#include "world.hh"

#include "oxydlib/DatFile.h"
#include "oxydlib/FileUtils.h"
#include "oxydlib/Level.h"
#include "lev/Proxy.hh"
#include "lev/Index.hh"

#include <cassert>

#define it_UNUSED it_INVALID
#define it_EXTERNAL it_INVALID
#define it_MISSING it_INVALID

#define UNUSED ""
#define SPECIAL ""

namespace oxyd
{
    using namespace OxydLib;
    using namespace enigma;

    using OxydLib::Level;

/* -------------------- OxydLoader -------------------- */

    /** This data structure is used to encode differences between different
        Oxyd versions and other .*/
    struct LoaderConfig {
        // Variables
        const char    *oxyd_flavor;
        double         timer_factor;
        bool           twoplayers;
        GameMode       gamemode;
        const char   **floortable;
        world::ItemID *itemtable;
        const char   **stonetable;

        int id_timer;
        int id_laser1;

        // Constructor
        LoaderConfig (bool twoplayers_,
                      GameMode gamemode_,
                      const char **floortable_,
                      world::ItemID *itemtable_,
                      const char **stonetable_,
                      const char *oxyd_flavor_ = "a"
                      )
        : oxyd_flavor(oxyd_flavor_),
          timer_factor (0.678),
          twoplayers (twoplayers_),
          gamemode (gamemode_),
          floortable (floortable_),
          itemtable (itemtable_),
          stonetable (stonetable_),
          id_timer (-1),
          id_laser1 (-1)
        {}
    };

    class OxydLoader {
    public:
        OxydLoader (const Level &level_, 
                    const LoaderConfig config_);

        virtual ~OxydLoader()
        {}

        void load();

        Stone *make_laser (int type);
        Stone *make_timer (int x, int y);

        /* ---------- OxydLoader interface ---------- */

        virtual Stone *make_stone (int type, int x, int y);
        virtual Item  *make_item (int type);


        /* ---------- Variables ---------- */
        const Level &level;
        LoaderConfig config;

    private:
        /* ---------- Private methods ---------- */
        void load_floor ();
        void load_items ();
        void load_stones ();
        void load_actors ();
        void parse_specials ();
        void scramble_puzzles ();

        void connect_rubberbands ();
        void connect_signals ();
        world::Actor *get_actor (int idx);


        /* ---------- Private variables ---------- */
        std::vector<world::Actor *> m_actors;
        bool harmless_medi;
    };

    class Oxyd1Loader : public OxydLoader {
    public:
        Oxyd1Loader (const OxydLib::Level &level_,
                     const LoaderConfig &config_)
        : OxydLoader (level_, config_)
        {}
    
        virtual Stone *make_stone (int type, int x, int y) { return 0; }
    };

    class PerOxydLoader : public OxydLoader {
    public:
        PerOxydLoader (const OxydLib::Level &level_, 
                       const LoaderConfig &config_)
        : OxydLoader (level_, config_)
        {}
        
        virtual Stone *make_stone (int type, int x, int y);
    };


/* -------------------- LevelPack_Oxyd -------------------- */

    class LevelPack_Oxyd : public lev::Index {
    public:
        LevelPack_Oxyd (OxydVersion ver, DatFile *dat,
                        int idx_start, int idx_end, bool twoplayers);

        void load_oxyd_level (size_t index);

        /* ---------- LevelPack interface ---------- */
        string get_name() const;
        int size() const { return nlevels; }
        const char* get_default_SoundSet() const;
        bool needs_twoplayers() const;
    protected:
        virtual bool has_easymode(size_t /*index*/) const;
        GameMode get_gamemode() const;

    private:
        /* ---------- Private interface ---------- */

        virtual void load (const Level &level) = 0;

        /* ---------- Private methods ---------- */

        GameType get_gametype() const;
        OxydVersion get_version () const {
            return m_datfile->getVersion();
        }
        int get_default_location() const;

        /* ---------- Variables ---------- */
        DatFile     *m_datfile; // just a reference (owned by GameInfo)
        bool         m_twoplayers; // true -> twoplayer game

        int m_index_start; // first index of this level pack
        int level_index[120];
        int nlevels;
    };

    class LP_Oxyd1 : public LevelPack_Oxyd {
        // LevelPack_Oxyd interface
        void load (const Level &);
    public:
        LP_Oxyd1 (DatFile *dat, bool twoplayers);
    };

    class LP_OxydExtra : public LevelPack_Oxyd {
        // LevelPack_Oxyd interface
        void load (const Level &);
    public:
        LP_OxydExtra (DatFile *dat);
    };

    class LP_PerOxyd : public LevelPack_Oxyd {
        // LevelPack_Oxyd interface
        void load (const Level &);
    public:
        LP_PerOxyd (DatFile *dat, bool twoplayers);
        static bool hasEasymode(size_t index);
    };

    class LP_OxydMagnum : public LevelPack_Oxyd {
        // LevelPack_Oxyd interface
        void load (const Level &);
    public:
        LP_OxydMagnum(OxydVersion, DatFile *dat);
    };

    /* -------------------- CommandString -------------------- */

    /** A class used to decode "command strings" used for 
        settings in Oxyd levels. */
    class CommandString {
    public:
        CommandString(const string &cmd);

        int get_int (int min, int max, int dflt);
        int get_char ();
        bool get_bool (bool dflt);
    private:
        // Variables
        string           m_cmd;
        string::iterator m_iter;
    };


    /* -------------------- GameInfo -------------------- */

    class GameInfo {
    public:
        GameInfo();
        GameInfo (OxydVersion ver_, const string &game_, const string &datfile_name_, const bool searchDAT);
        ~GameInfo();

        bool is_present() const { return m_present; }
        DatFile *getDatfile() { return datfile; }

    private:
        // Variables.
        OxydVersion  ver;
        string       game;
        DatFile     *datfile;
        string       datfile_path;
        bool         m_present;

        // Private methods.
        void       openDatFile();
        lev::Index *makeLevelIndex(bool twoplayer);
    };


    enum MarbleInfoIndices {
        // valid for all actor types:
        MI_FORCE = 0,

        // marbles only:

        // Jack/Rotor only:
        MI_RANGE = 1,
        MI_GOHOME = 2,

        // Horse:
        MI_HORSETARGET1 = 1,
        MI_HORSETARGET2 = 2,
        MI_HORSETARGET3 = 3,
        MI_HORSETARGET4 = 4,
    };

    class MarbleInfo {
        enum { MAX_MARBLE_INFO_FIELDS = 11 };
        enum { DEFAULT_VALUE = -1 };

        int  value[MAX_MARBLE_INFO_FIELDS];
        bool interpreted[MAX_MARBLE_INFO_FIELDS];

    public:
        MarbleInfo(const Marble& marble);
        ~MarbleInfo();

        bool is_default(int idx) {
            assert(idx >= 0 && idx<MAX_MARBLE_INFO_FIELDS);
            return value[idx] == DEFAULT_VALUE;
        }

        int get_value(int idx) {
            assert(!is_default(idx)); // you cannot ask for default value (not stored here)
            interpreted[idx] = true;
            return value[idx];
        }
        int get_value (int idx, int default_) {
            if (!is_default(idx))
                return get_value (idx);
            else
                return default_;
        }
    };

/* -------------------- Global Variables -------------------- */

    extern world::ItemID oxyd1_item_map[];
    extern const char *oxyd1_floor_map[];
    extern const char *oxyd1_stone_map[];

    extern world::ItemID peroxyd_item_map[];
    extern const char *peroxyd_floor_map[];
    extern const char *peroxyd_stone_map[];

    extern world::ItemID oxydmag_item_map[];
    extern const char *oxydmag_floor_map[];
    extern const char *oxydmag_stone_map[];

    extern world::ItemID oxydextra_item_map[];
    extern const char *oxydextra_floor_map[];
    extern const char *oxydextra_stone_map[];

}

#endif
