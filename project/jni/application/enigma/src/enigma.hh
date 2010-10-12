/*
 * Copyright (C) 2002,2004 Daniel Heck
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
#ifndef ENIGMA_HH
#define ENIGMA_HH

/*
 * This file contains declarations for facilities used by several
 * different parts of the program, like common types and constants,
 * and routines for resource management.
 */

#include "fwd.hh"
#include "file.hh"
#include "ecl_fwd.hh"
#include "ecl_math.hh"
#include "ecl_util.hh"
#include "ecl_cache.hh"

#define NUMENTRIES(array) (sizeof(array)/sizeof(*array))

namespace enigma
{

/* -------------------- Various types -------------------- */

    enum Difficulty {
        DIFFICULTY_EASY = 1,
        DIFFICULTY_HARD = 2,
        DIFFICULTY_ANY  = 3
    };


    /*! Enigma can run its own levels but also emulate various
      versions of Oxyd.  All these games behave similarly, but there
      are a few differences in object behaviour, visual appearance,
      etc.  */
    enum GameType {
        GAMET_FIRST,

        GAMET_ENIGMA = GAMET_FIRST,
        GAMET_OXYD1,
        GAMET_PEROXYD,
        GAMET_OXYDEXTRA,
        GAMET_OXYDMAGNUM,

        GAMET_LAST = GAMET_OXYDMAGNUM,
        GAMET_COUNT,
        GAMET_UNKNOWN
    };

    GameType GetGameType(std::string name);
    std::string GetGameTypeName(GameType type);

/* -------------------- Resource Management -------------------- */

    class ImageCache : public ecl::PtrCache<ecl::Surface> {
    public:
        using ecl::PtrCache<ecl::Surface>::store;

        ecl::Surface *acquire(const std::string &name);
    };


/* The `Get...' functions return a pointer to a cached copy of the
   font or image.  The `Load...' functions load a new copy which
   you must deallocate yourself.  */
   

    void DefineFont (const char *name, 
                     const char *ttf_name, 
                     int ttf_size,
                     const char *bmf_name,
                     int r, int g, int b);
    ecl::Font *GetFont (const char *name);
    void ClearFontCache();

    ecl::Surface *LoadImage (const char *name);
    ecl::Surface *GetImage (const char *name, const char *ext = ".png");
    ecl::Surface *RegisterImage (const char *name, ecl::Surface *s);
    void ClearImageCache();

/* -------------------- Direction, DirectionBits -------------------- */

    enum Direction {
        NODIR = -1,
        WEST  = 0,
        SOUTH = 1,
        EAST  = 2,
        NORTH = 3,
    };

    Direction reverse (Direction d);
    Direction rotate_cw (Direction d);
    Direction rotate_ccw (Direction d);

    std::string to_suffix(Direction d);

    enum DirectionBits {
        NODIRBIT       = 0,
        WESTBIT        = 1 << WEST,
        SOUTHBIT       = 1 << SOUTH,
        EASTBIT        = 1 << EAST,
        NORTHBIT       = 1 << NORTH,
        ALL_DIRECTIONS = 15
    };

    DirectionBits rotate (DirectionBits d, bool clockwise);
    DirectionBits to_bits (Direction d);
    bool has_dir (DirectionBits db, Direction dir);


/* -------------------- Value -------------------- */

    class Value {
    public:
        enum Type { NIL, DOUBLE, STRING };

        Value() : type(NIL) {}
        Value(double d) : type(DOUBLE) { val.dval = d; }
        Value(const char* str);
        Value(const std::string& str);
        ~Value();

        Value(const Value& v);
        Value& operator=(const Value& v);

        void assign(double d);
        void assign(const char* s);

        Type    get_type() const { return type; }
        double  get_double() const throw();
        const char* get_string() const throw();
    private:
        void clear();

        // Variables
        Type type;
        union {
            double dval;
            char* str;
        } val;
    };

    ecl::Buffer& operator<<(ecl::Buffer& buf, const Value& val);
//     ecl::Buffer& operator>>(ecl::Buffer& buf, Value& val);

    std::ostream& operator<<(std::ostream& os, const Value& val);

    bool        to_bool(const Value &v);
    int         to_int(const Value &v);
    double      to_double(const Value &v);
    const char *to_string(const Value &v);
    Direction   to_direction (const Value &v);

/* -------------------- Timers -------------------- */

    /* Interface for time event handlers. */
    class TimeHandler {
    public:
        virtual ~TimeHandler() {}
        virtual void tick (double /*dtime*/) {}
        virtual void alarm() {}
    };

/* -------------------- GridPos -------------------- */

    struct GridPos {
        // Variables
        int x, y;

        // Methods
        explicit GridPos(int xx=0, int yy=0);
        explicit GridPos(const ecl::V2& p);
        void move(Direction dir);
        ecl::V2 center() const;
    };

    GridPos move(GridPos p, Direction dir);
    GridPos move(GridPos p, Direction dir, Direction dir2);
    bool operator== (GridPos a, GridPos b);
    bool operator != (GridPos a, GridPos b);
    bool operator< (GridPos a, GridPos b);

    /* 516
       203
       748 */
    GridPos get_neighbour (GridPos p, int i);

    // source and target have to be adjacent
    Direction direction_fromto(GridPos source, GridPos target); 

    std::ostream& operator<<(std::ostream& os, const GridPos& val);


    // ---------- GridLayer ----------

    enum GridLayer {
        GRID_FLOOR, 
        GRID_ITEMS, 
        GRID_STONES, 
        GRID_COUNT
    };

    enum GridLayerBits {
        GRID_FLOOR_BIT  = 1,
        GRID_ITEMS_BIT  = 2,
        GRID_STONES_BIT = 4
    };

    // ---------- GridLoc ----------

    struct GridLoc {
        // Variables
        GridPos pos;
        GridLayer layer;

        // Constructor
        GridLoc(GridLayer l = GRID_FLOOR, GridPos p = GridPos());
    };

    /*! Converts strings like "it(10 10)", "st(5 2)" to GridLoc
      structures. */
    bool to_gridloc (const char *str, GridLoc &loc);


/* -------------------- Random Numbers -------------------- */

    void   Randomize ();
    void   Randomize (unsigned seed);
    int    IntegerRand (int min, int max);
    double DoubleRand (double min, double max);

/* -------------------- Time & Date -------------------- */

    const char *date(const char *format); // format see 'man strftime'


    /* ==================== Inline definitions ==================== */

    inline DirectionBits to_bits(Direction d) {
        if (d==NODIR)
            return NODIRBIT;
        return DirectionBits(1 << d);
    }

    inline bool has_dir(DirectionBits db, Direction dir) {
        return (db & to_bits(dir)) != 0;
    }


    // ---------- GridPos ----------

    inline GridPos::GridPos(int xx, int yy) 
    : x(xx), y(yy) 
    {}


    inline void GridPos::move(Direction dir) {
        switch(dir) {
        case NORTH: y--; break; 
        case SOUTH: y++; break;
        case EAST: x++; break; 
        case WEST: x--; break;
        case NODIR: break;
        }
    }

    inline ecl::V2 GridPos::center() const { 
        return ecl::V2(x+.5, y+.5); 
    }


    inline GridPos move(GridPos p, Direction dir) {
        GridPos tmp = p;
        tmp.move(dir);
        return tmp;
    }

    inline GridPos move(GridPos p, Direction dir, Direction dir2) {
        GridPos tmp = p;
        tmp.move(dir);
        tmp.move(dir2);
        return tmp;
    }

    inline bool operator == (GridPos a, GridPos b) { 
        return (a.x==b.x && a.y==b.y); 
    }
  
    inline bool operator != (GridPos a, GridPos b) { 
        return (a.x!=b.x || a.y!=b.y); 
    }

    inline bool operator< (GridPos a, GridPos b) {
        return ((a.y<<16) + a.x) < ((b.y<<16) + b.x);
    }


    // ---------- GridLoc ---------- 

    inline GridLoc::GridLoc(GridLayer l, GridPos p) 
    : pos(p), layer(l) 
    {}

}

#endif
