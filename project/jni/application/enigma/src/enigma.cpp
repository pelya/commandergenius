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

#include "errors.hh"
#include "enigma.hh"
#include "ecl.hh"
#include "main.hh"

#include <iostream>
#include <ctime>
#include <set>

using namespace std;
using namespace ecl;
using namespace enigma;


/* -------------------- Game Type -------------------- */

static const char *versionName[GAMET_COUNT+1] = {
    "enigma", // same indices as enum GameType
    "oxyd1",
    "per.oxyd",
    "oxyd.extra",
    "oxyd.magnum",
    0
};

GameType enigma::GetGameType(std::string name) {
    GameType type = GAMET_UNKNOWN;
    for (int v = 0; v<GAMET_COUNT; ++v) {
        if (0 == strcmp(name.c_str(), versionName[v])) {
            type = GameType(v);
            break;
        }
    }
    return type;
}

std::string enigma::GetGameTypeName(GameType type) {
    if (type >= GAMET_FIRST && type <= GAMET_LAST)
        return versionName[type];
    else
        return "unknown";
}

/* -------------------- Direction -------------------- */

Direction enigma::reverse(Direction d) {
    static Direction rdir[] = { NODIR, EAST, NORTH, WEST, SOUTH };
    return rdir[d+1];
}

Direction enigma::rotate_cw (Direction d)
{
    static Direction rdir[] = { NODIR, NORTH, WEST, SOUTH, EAST };
    return rdir[d+1];
}

Direction enigma::rotate_ccw (Direction d)
{
    static Direction rdir[] = { NODIR, SOUTH, EAST, NORTH, WEST };
    return rdir[d+1];
}

Direction
direction_fromto(GridPos source, GridPos target)
{
    // source and target have to be adjacent

    int       dx = target.x-source.x;
    int       dy = target.y-source.y;
    Direction d  = NODIR;

    if (dx == 0) {
        if      (dy == -1) d = NORTH;
        else if (dy ==  1) d = SOUTH;
    }
    else if (dy == 0) {
        if      (dx == -1) d = WEST;
        else if (dx ==  1) d = EAST;
    }

    ASSERT(d != NODIR, XLevelRuntime,
        "direction_fromto: source and target not adjacent");
    return d;
}

string enigma::to_suffix(Direction d) {
    static const char *sfx[] = { "", "-w", "-s", "-e", "-n" };
    return sfx[d+1];
}


/* -------------------- DirectionBits -------------------- */

DirectionBits
enigma::rotate(DirectionBits d, bool clockwise)
{
    if (clockwise) {
        d = DirectionBits(((d>>1) | (d<<3)) & ALL_DIRECTIONS);
    } else {
        d = DirectionBits(((d>>3) | (d<<1)) & ALL_DIRECTIONS);
    }
    return d;
}


/* -------------------- Value implementation -------------------- */

Value::Value(const char* str)
: type(STRING)
{
    val.str = new char[strlen(str)+1];
    strcpy(val.str, str);
}

Value::~Value() 
{ 
    clear(); 
}


Value::Value(const string& str)
: type(STRING)
{
    val.str = new char[str.length()+1];
    strcpy(val.str, str.c_str());
}

Value::Value (const Value& other) : type(NIL) {
    this->operator=(other);
}

Value& Value::operator= (const Value& other) {
    if (this != &other) {
        if (other.type == STRING) {
            assign(other.val.str);
        } else {
	    clear();
            type = other.type;
            val = other.val;
        }
    }
    return *this;
}


void Value::assign(const char* s) {
    clear();
    type = STRING;
    val.str = new char[strlen(s)+1];
    strcpy(val.str, s);
}

void Value::assign(double d) 
{ 
    clear(); type=DOUBLE; val.dval=d; 
}


void Value::clear() {
    if (type == STRING)
	delete[] val.str;
    type = NIL;
}

double Value::get_double() const throw()
{
    ASSERT(type == DOUBLE, XLevelRuntime, "get_double: type not double");
    return val.dval;
}

const char* Value::get_string() const throw()
{
    ASSERT(type == STRING, XLevelRuntime, "get_string: type not string");
    return val.str;
}

Buffer& enigma::operator<<(Buffer& buf, const Value& val)
{
    buf << Uint8(val.get_type());

    switch (val.get_type()) {
    case Value::NIL:
        break;
    case Value::DOUBLE:
        buf << val.get_double();
        break;
    case Value::STRING:
        {
            const char* str = val.get_string();
            buf << (Uint16)strlen(str);
            buf.write(str, strlen(str));
        } break;
    }
    return buf;
}

// Buffer& enigma::operator>>(Buffer& buf, Value& val)
// {
//     Uint8 type = Value::NIL;
//     buf >> type;

//     switch (type) {
//     case Value::NIL:
//         // ## fixme
//         break;
//     case Value::DOUBLE:
//         {
//             double tmp;
//             if (buf >> tmp)
//                 val = Value(tmp);
//         } break;
//     case Value::STRING:
//         {
//             Uint16 len;
//             if (buf >> len) {
//                 char* tmp = new char[len+1];
//                 tmp[len] = 0;
//                 if (buf.read(tmp, len))
//                     val.assign(tmp);
//                 delete[] tmp;
//             }
//         } break;
//     }
//     return buf;
// }

int enigma::to_int(const Value &v) {
    switch (v.get_type()) {
    case Value::DOUBLE: return round_nearest<int>(v.get_double());
    case Value::STRING: return atoi(v.get_string());
    default: return 0;
    }
}

bool enigma::to_bool(const Value &v) {
    return (v.get_type() != Value::NIL);
}

double enigma::to_double(const Value &v) {
    switch (v.get_type()) {
    case Value::DOUBLE: return v.get_double();
    case Value::STRING: return atof(v.get_string());
    default: return 0;
    }
}

#ifdef _MSC_VER
#define snprintf _snprintf
#endif

const char * enigma::to_string(const Value &v) {
    static char buf[30];
    switch (v.get_type()) {
    case Value::NIL: return "";
    case Value::DOUBLE:
        snprintf(buf, sizeof(buf), "%f", v.get_double());
        return buf;
    case Value::STRING: return v.get_string();
    default: return 0;
    }
}

Direction enigma::to_direction (const Value &v) {
    int val = Clamp(to_int(v), 0, 3);
    return static_cast<Direction>(val);
}

ostream& enigma::operator<<(ostream& os, const Value& val)
{
    switch (val.get_type()) {
    case Value::NIL:   os << "nil"; break;
    case Value::DOUBLE: os << val.get_double(); break;
    case Value::STRING: os << val.get_string(); break;
    }
    return os;
}

/* -------------------- GridPos -------------------- */

GridPos::GridPos(const ecl::V2& p) 
: x (round_down<int>(p[0])), 
  y (round_down<int>(p[1])) 
{}


std::ostream& enigma::operator<<(std::ostream& os, const GridPos& val)
{
    return os << '(' << val.x << ',' << val.y << ')';
}

/* 
   516
   203
   748
*/
GridPos enigma::get_neighbour (GridPos p, int i)
{
    ASSERT (i >= 0 && i <= 9, XLevelRuntime, "get_neighbour: index out of bounds");
    static int xoff[9] = { 0,0,-1,1,0,-1,1,-1,1 };
    static int yoff[9] = { 0,-1,0,0,1,-1,-1,1,1 };
    return GridPos(p.x + xoff[i], p.y + yoff[i]);
}

/* -------------------- GridLoc -------------------- */

bool enigma::to_gridloc (const char *str, GridLoc &l) {
    GridLoc loc;
    const char *numstr = str + 3;

    if (strncmp (str, "fl(", 3) == 0)
        loc.layer = GRID_FLOOR;
    else if (strncmp (str, "it(", 3) == 0)
        loc.layer = GRID_ITEMS;
    else if (strncmp (str, "st(", 3) == 0)
        loc.layer = GRID_STONES;
    else
        numstr = str;
    
    if (2 != sscanf (numstr, "%d %d", &loc.pos.x, &loc.pos.y))
        return false;
    l = loc;
    return true;
}




/* -------------------- Random numbers -------------------- */

void  enigma::Randomize ()
{
    srand (time(NULL));
}

void   enigma::Randomize (unsigned seed)
{
    srand (seed);
}

int    enigma::IntegerRand (int min, int max)
{
    int r = round_down<int>((max-min+1) * (rand()/(RAND_MAX+1.0)));
    return r+min;
}

double enigma::DoubleRand (double min, double max)
{
    return min + double(rand())/RAND_MAX * (max-min);
}


/* -------------------- Time & Date -------------------- */

#define MAX_DATE_LENGTH 256
const char *enigma::date(const char *format) { // format see 'man strftime'
    static char *result = 0;
    char         buffer[MAX_DATE_LENGTH];

    time_t t;
    time(&t);

    struct tm *tm  = localtime(&t);
    strftime(buffer, MAX_DATE_LENGTH, format, tm);

    if (result) free(result);
    result = strdup(buffer);

    return result;
}


/* -------------------- Resource management -------------------- */

namespace
{
    struct FontDescr {
        // Variables
        string name;
        string ttf_name;
        int    ttf_size;
        string bitmap_name;
        int r, g, b;

        // Constructor
        FontDescr (const string &name_,
                   const string &ttf_name_,
                   int ttf_size_,
                   const string &bitmap_name_, 
                   int r_, int g_, int b_)
        : name (name_),
          ttf_name (ttf_name_),
          ttf_size (ttf_size_),
          bitmap_name (bitmap_name_), 
          r (r_), g(g_), b(b_)
        {}
    };

    class FontCache : public PtrCache<Font> {
    public:
        Font *acquire (const std::string &name) {
            Font *f = 0;
            if (m_fonts.has_key (name)) {
                const FontDescr &fd = m_fonts[name];
                f = load_ttf (fd.ttf_name, fd.ttf_size, fd.r, fd.g, fd.b);
                if (f == 0) {
                    std::cerr << "Could not load .ttf file " << fd.ttf_name << "\n";
                    f = load_bmf (fd.bitmap_name);
                }
            }
            else {
                f = load_bmf (name);
            }
            return f;
        }

        void define_font (const FontDescr &descr) {
            remove (descr.name); // remove entry in cache (if any)
            if (m_fonts.has_key (descr.name))
                m_fonts[descr.name]= descr;
            else
                m_fonts.insert (descr.name, descr);
        }

    private:

        Font *load_bmf (const string &name) {
            string png, bmf;
            if (app.resourceFS->findFile(string("fonts/")+name+".png", png) &&
                app.resourceFS->findFile(string("fonts/")+name+".bmf", bmf))
            {
                return ecl::LoadBitmapFont(png.c_str(), bmf.c_str());
            }
            return 0;
        }

        Font *load_ttf (const string &name, int ptsize, int r, int g, int b) {
            string ttf;
            if (app.resourceFS->findFile(string("fonts/") + name, ttf))
                return ecl::LoadTTF (ttf.c_str(), ptsize, r, g, b);
            return 0;
        }

        // Variables
        ecl::Dict <FontDescr> m_fonts;
    };

    // ---------- Variables ----------

    FontCache  font_cache;
    ImageCache image_cache;
}

ecl::Surface *ImageCache::acquire (const std::string &name) 
{
    return ecl::LoadImage(name.c_str());
}

void enigma::DefineFont (const char *name, 
                         const char *ttf_name, 
                         int ttf_size,
                         const char *bmf_name,
                         int r, int g, int b)
{
    font_cache.define_font (FontDescr (name, ttf_name, ttf_size, bmf_name, r, g, b));
}

ecl::Font *enigma::GetFont (const char *name) 
{
    return font_cache.get(name);
}

void enigma::ClearFontCache() {
    font_cache.clear();
}

ecl::Surface *enigma::LoadImage(const char *name) 
{
    string filename;
    if (app.resourceFS->findImageFile (string(name) + ".png", filename)) 
        return ecl::LoadImage(filename.c_str());
    return 0;
}

ecl::Surface *enigma::GetImage(const char *name, const char *ext) 
{
    string filename;
    if (app.resourceFS->findImageFile (string(name) + ext, filename)) 
        return image_cache.get(filename);
    return 0;
}

ecl::Surface *enigma::RegisterImage (const char *name, ecl::Surface *s) 
{
    image_cache.store(name, s);
    return s;
}

void enigma::ClearImageCache() {
    image_cache.clear();
}
