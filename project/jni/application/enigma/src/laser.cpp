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
#include "laser.hh"
#include "sound.hh"
#include "stones_internal.hh"
#include "server.hh"
#include <algorithm>
#include <cassert>
#include <map>

using namespace std;
using namespace world;
using namespace lasers;

using stones::maybe_push_stone;
using ecl::V2;

namespace
{

/* -------------------- LaserBeam -------------------- */

    class LaserBeam : public Item, public LaserEmitter {
    public:
        static void emit_from(GridPos p, Direction d);
        static void kill_all();
        static void all_emitted();

	// LaserEmitter interface
        DirectionBits emission_directions() const { return directions; }
        static ItemTraits traits;

        const ItemTraits &get_traits() const {
            return traits;
        }
    private:
        LaserBeam(Direction dir) {
            directions = to_bits(dir);
        }

        // Item interface.
        void on_laserhit(Direction dir);
        void on_creation (GridPos p);
        void init_model();
        bool actor_hit(Actor *actor);

        Item *clone() {
            // new LaserBeams may only created inside `emit_from'.
            assert(0);
            return 0;
        }
        void dispose();

        // Variables
        DirectionBits directions;

        static vector<LaserBeam*> instances;
        static map<GridPos, int>  old_laser_positions;
    };
    ItemTraits LaserBeam::traits = {"it-laserbeam", it_laserbeam, 
                                    itf_static | itf_indestructible, 0.0 };


/* -------------------- Laser Stones -------------------- */

/** \page st-laser Laser Stone

These stones emit a laser beam in a specified direction when
activated.  They are the only objects in the game that can act as
primary light sources (mirrors can also emit light but they require
an incoming beam).

\subsection lasera Attributes

- \b on:      1 if laser in active, 0 if not
- \b dir:     the Direction in which light is emitted
            (NORTH, EAST, SOUTH, WEST)

\subsection laserm Messages

- \b on, \b off, \b onoff: as usual

\subsection lasersa See also

\ref st-pmirror, \ref st-3mirror

*/
    class LaserStone : public LaserEmitter, public stones::OnOffStone {
    public:
        LaserStone (Direction dir=EAST);
        static void reemit_all();

    private:

//        INSTANCELISTOBJ(LaserStone);

// We can't use this macro here: g++ can't handle multiple inheritance
// and covariant return types at the same time ("sorry, not
// implemented: ..." first time I ever saw this error message :-)

        typedef std::vector<LaserStone*> InstanceList;
        static InstanceList instances;
        Stone *clone() {
            LaserStone *o = new LaserStone(*this);
            instances.push_back(o);
            return o;
        }
        void dispose() {
            instances.erase(find(instances.begin(), instances.end(), this));
            delete this;
        }

        // LaserEmitter interface
        DirectionBits emission_directions() const;

        // OnOffStone interface.
        void notify_onoff(bool on);

        // Private methods.
        void emit_light();
        Direction get_dir() const {return Direction(int_attrib("dir"));}

        // Stone interface.
        void on_creation (GridPos p);
        void init_model();
    };
}


/* -------------------- PhotoCell -------------------- */

vector<void*> PhotoCell::instances;

PhotoCell::~PhotoCell() 
{
    photo_deactivate();
}


/**
 * This function notifies all instances of PhotoCell that a
 * recalculation of the laser beams is about to begin by calling
 * on_recalc_start() for each instance.
 */
void PhotoCell::notify_start()
{
    for(unsigned i=0; i<instances.size(); ++i)
    {
        PhotoCell *pc = (PhotoCell*) instances[i];
        pc->on_recalc_start();
    }
}

/**
 * This function notifies all instances of PhotoCell that the engine
 * has finished recalculating the laser beams by calling
 * on_recalc_finish() for each instance.
 */
void PhotoCell::notify_finish()
{
    for(unsigned i=0; i<instances.size(); ++i)
    {
        PhotoCell *pc = (PhotoCell*) instances[i];
        pc->on_recalc_finish();
    }
}

void PhotoCell::photo_activate()
{
    vector<void*>::iterator i = std::find(instances.begin(), instances.end(), this);
    if (i != instances.end())
        assert (0 || "Photocell activated twice\n");
    else
        instances.push_back (this);
}

void PhotoCell::photo_deactivate()
{
    vector<void*>::iterator i;
    i = std::find (instances.begin(), instances.end(), this);
    if (i != instances.end())
        instances.erase(i);
}


/* -------------------- PhotoStone -------------------- */

PhotoStone::PhotoStone(const char *kind) : Stone(kind) 
{
    illuminated = false;
}

void PhotoStone::on_recalc_start() 
{}

void PhotoStone::on_recalc_finish() 
{ 
    GridPos p = get_pos();
    bool illu = (LightFrom(p, NORTH) || LightFrom(p, EAST)
                 || LightFrom(p, WEST) || LightFrom(p, SOUTH));

    if (illu != illuminated) {
        if (illu) notify_laseron();
        else      notify_laseroff();
        illuminated = illu;
    }
}


/* -------------------- LaserBeam -------------------- */

// The implementation of laser beams is a little tricky because, in
// spite of being implemented as Items, lasers aren't localized and
// changes to any part of the beam can affect the beam elsewhere.  A
// `change' may be anything from moving a stone in or out of the beam,
// rotating or moving one of the mirrors, to making a stone in the
// beam transparent.
//
// Here are a couple of facts about laser beams in Enigma:
//
// - Laser beams are static. Once calculated they do not change until
//   they are completely recalculated
//
// - LaserBeam::emit_from() is the only way to emit laser beams. A new
//   beam will propagate automatically and stops only if it comes
//   across an item or a stone that returns `false' from
//   Stone::is_transparent().
//
// - `on_laserhit()' is called for objects in the beam *whenever*
//   the beam is recalculated.  For objects that need to be notified
//   when the laser goes on or off, use the `PhotoStone'
//   mixin.

vector<LaserBeam*> LaserBeam::instances;
map<GridPos, int>  LaserBeam::old_laser_positions;

void LaserBeam::init_model()
{
    if (directions & (EASTBIT | WESTBIT)) {
        if (directions & (NORTHBIT | SOUTHBIT))
            set_model("it-laserhv");
        else
            set_model("it-laserh");
    }
    else if (directions & (NORTHBIT | SOUTHBIT))
        set_model("it-laserv");
}

void LaserBeam::on_laserhit(Direction dir)
{
    DirectionBits dirbit = to_bits(dir);
    if (!(directions & dirbit)) {
        // `dir' not in `directions' ?
        directions = DirectionBits(directions | dirbit);
        emit_from(get_pos(), dir);
        init_model();
    }
}

void LaserBeam::on_creation (GridPos p)
{
    if (directions & EASTBIT) emit_from(p, EAST);
    if (directions & WESTBIT) emit_from(p, WEST);
    if (directions &NORTHBIT) emit_from(p, NORTH);
    if (directions &SOUTHBIT) emit_from(p, SOUTH);
    init_model();
}

void LaserBeam::emit_from(GridPos p, Direction dir)
{
    bool may_pass = true;

    p.move(dir);
    if (Stone *st = GetStone(p)) {
        may_pass = st->is_transparent (dir);
        st->on_laserhit (dir);
    }

    if (may_pass) {
        if (Item *it = GetItem(p))
            it->on_laserhit (dir);
        else {
            LaserBeam *lb = new LaserBeam (dir);
            SetItem(p, lb);
            instances.push_back(lb);
        }
    }
}

bool LaserBeam::actor_hit(Actor *actor)
{
    double r = get_radius(actor);
    V2 p = actor->get_pos();
    GridPos gp = get_pos();

    // distance of actor from center of the grid
    double dx = fabs(p[0] - gp.x - 0.5) - r;
    double dy = fabs(p[1] - gp.y - 0.5) - r;

    if ((directions & (EASTBIT | WESTBIT) && dy<-0.1) ||
        (directions & (NORTHBIT | SOUTHBIT)) && dx<-0.1)
    {
        SendMessage(actor, "laserhit");
    }

    return false; // laser beams can't be picked up
}

void LaserBeam::kill_all()
{
    assert(old_laser_positions.empty());

    while (!instances.empty())
    {
        LaserBeam *lb  = instances[0];
        GridPos    pos = lb->get_pos();

        old_laser_positions[pos] = static_cast<int>(lb->directions);
        world::KillItem(pos);
    }
}

void LaserBeam::all_emitted()
{
    vector<LaserBeam*>::const_iterator end  = instances.end();
    map<GridPos, int>::iterator        none = old_laser_positions.end();

    double x     = 0, y = 0;
    int    count = 0;

    for (vector<LaserBeam*>::const_iterator i = instances.begin(); i != end; ++i) {
        LaserBeam                   *lb    = *i;
        GridPos                      pos   = lb->get_pos();
        map<GridPos, int>::iterator  found = old_laser_positions.find(pos);

        if (found != none) {
            // a beam was at the current position (during last kill_all())
            DirectionBits old_dir = static_cast<DirectionBits>(found->second);

            if ((old_dir&lb->directions) != lb->directions) {
                // a beam has been added here
                x += pos.x;
                y += pos.y;
                ++count;
            }
        }
        else {
            // store newly created LaserBeams
            x += pos.x;
            y += pos.y;
            ++count;
        }
    }

    if (count) {
        sound::EmitSoundEvent ("laseron", ecl::V2(x/count+.5, y/count+.5),
                               getVolume("laseron", NULL));
    }

    old_laser_positions.clear();
}

void LaserBeam::dispose()
{
    instances.erase(std::find(instances.begin(), instances.end(), this));
    delete this;
}


//----------------------------------------
// Laser stone
//----------------------------------------
LaserStone::InstanceList LaserStone::instances;

LaserStone::LaserStone (Direction dir)
: OnOffStone("st-laser")
{
    set_attrib("dir", Value(dir));
}

DirectionBits
LaserStone::emission_directions() const
{
    if (is_on()) {
        return to_bits(get_dir());
    }
    return NODIRBIT;
}


void LaserStone::reemit_all()
{
    for (unsigned i=0; i<instances.size(); ++i)
    {
        LaserStone *ls = (LaserStone*) instances[i];
        ls->emit_light();
    }
}

void LaserStone::notify_onoff(bool /*on*/)
{
    RecalcLight();
}

void LaserStone::emit_light()
{
    if (is_on())
        LaserBeam::emit_from(get_pos(), get_dir());
}

void LaserStone::on_creation (GridPos p)
{
    if (is_on())
        RecalcLight();
    Stone::on_creation(p);
}

void LaserStone::init_model()
{
    string mname = is_on() ? "st-laseron" : "st-laser";
    mname += to_suffix(get_dir());
    set_model(mname);
}


/* -------------------- MirrorStone -------------------- */
namespace
{
    class MirrorStone
        : public Stone, public LaserEmitter, public PhotoCell
    {
    protected:
        MirrorStone(const char *name, bool movable=false, bool transparent=false);

        bool is_transparent() const { return int_attrib("transparent") != 0; }
        bool is_movable() const { return int_attrib("movable") != 0; }

        void set_orientation(int o) { set_attrib("orientation", o); }
        int get_orientation() { return int_attrib("orientation"); }

        void emit_light(Direction dir) {
            if (!has_dir(outdirs, dir))
            {
                outdirs = DirectionBits(outdirs | to_bits(dir));
                LaserBeam::emit_from(get_pos(), dir);
            }
        }

        void init_model();
    private:
	// Object interface.
        virtual Value message(const string &m, const Value &);

        // LaserEmitter interface
        DirectionBits emission_directions() const {
            return outdirs;
        }

        // PhotoCell interface
        void on_recalc_start() { outdirs = NODIRBIT; }
        void on_recalc_finish() {}

        // Stone interface
        void actor_hit(const world::StoneContact &sc);
        void on_creation (GridPos p);
        void on_removal (GridPos p);
        bool is_transparent(Direction) const { return is_transparent(); }

        // Private methods
        void rotate_right();

        // Variables
        DirectionBits outdirs;
    };
}

MirrorStone::MirrorStone(const char *name, bool movable, bool transparent)
: Stone(name), outdirs(NODIRBIT)
{
    set_attrib("transparent", transparent);
    set_attrib("movable", movable);
    set_attrib("orientation", Value(1));
}

void MirrorStone::init_model() {
    string mname = get_kind();
    mname += is_movable() ? "-m" : "-s";
    mname += is_transparent() ? "t" : "o";
    mname += char('0' + get_orientation());
    set_model(mname);
}

Value MirrorStone::message(const string &m, const Value &val) {
    if (m == "trigger" || m=="turn") {
        rotate_right();
    }
    else if (m == "signal") {
        if (to_double(val) != 0) {
            rotate_right();
        }
    }
    else if (m == "mirror-north") {
        set_orientation(3);
        init_model();
        MaybeRecalcLight(get_pos());
    }
    else if (m == "mirror-east") {
        set_orientation(4);
        init_model();
        MaybeRecalcLight(get_pos());
    }
    else if (m == "mirror-south") {
        set_orientation(1);
        init_model();
        MaybeRecalcLight(get_pos());
    }
    else if (m == "mirror-west") {
        set_orientation(2);
        init_model();
        MaybeRecalcLight(get_pos());
    }
    return Value();
}

void MirrorStone::actor_hit(const world::StoneContact &sc)
{
    if (is_movable())
        maybe_push_stone(sc);
    rotate_right();
}

void MirrorStone::on_creation (GridPos p)
{
    photo_activate();
    Stone::on_creation(p);
}

void MirrorStone::on_removal(GridPos p)
{
    photo_deactivate();
    Stone::on_removal(p);
}

void MirrorStone::rotate_right()
{
    set_orientation(1+(get_orientation() % 4));
    init_model();
    MaybeRecalcLight(get_pos());
    sound_event ("mirrorturn");
}



/* -------------------- Plane Mirror -------------------- */
namespace
{
    class PlaneMirror : public MirrorStone {
        CLONEOBJ(PlaneMirror);
    public:
        PlaneMirror(char orientation='/', bool movable=false, bool transparent=false)
        : MirrorStone("st-pmirror", movable, transparent)
        {
            SetOrientation(orientation);
        }
    private:
        void SetOrientation(char o) {
            const char *a = " -\\|/";
            MirrorStone::set_orientation(int (strchr(a,o)-a));
        }
        char GetOrientation() {
            const char *a = " -\\|/";
            return a[MirrorStone::get_orientation()];
        }
        void on_laserhit(Direction dir);
    };
}

void PlaneMirror::on_laserhit(Direction dir) 
{
    char orientation = GetOrientation();
    bool transparent = is_transparent();

    switch (orientation) {
    case '|':
        if (dir==EAST || dir==WEST) {
            emit_light(reverse(dir));
            if (transparent)
                emit_light(dir);
        }
        else if ((dir == NORTH || dir == SOUTH) && transparent &&
                 server::GameCompatibility == GAMET_OXYD1) {
            emit_light(dir);
        }
        break;
    case '-':
        if (dir==NORTH || dir==SOUTH) {
            emit_light(reverse(dir));
            if (transparent)
                emit_light(dir);
        }
        else if ((dir == EAST || dir == WEST) && transparent &&
                 server::GameCompatibility == GAMET_OXYD1) {
            emit_light(dir);
        }
        break;
    case '/':
        switch(dir) {
        case EAST: emit_light(NORTH); break;
        case SOUTH: emit_light(WEST); break;
        case NORTH: emit_light(EAST); break;
        case WEST: emit_light(SOUTH); break;
        case NODIR: break;
        }
        if (transparent)
            emit_light(dir);
        break;
    case '\\':
        switch(dir) {
        case EAST: emit_light(SOUTH); break;
        case SOUTH: emit_light(EAST); break;
        case NORTH: emit_light(WEST); break;
        case WEST: emit_light(NORTH); break;
        case NODIR: break;
        }
        if (transparent)
            emit_light(dir);
        break;
    }
}


/* -------------------- TriangleMirror -------------------- */

namespace
{
    // The orientations of the TriangleMirror have an unusual definition,
    // but we cannot change them w/o changing many levels
    //
    //    Flat side of the triangle
    //    points to :                            Orientation :
    //
    //    WEST                                   4
    //    SOUTH                                  3
    //    EAST                                   2
    //    NORTH                                  1

    class TriangleMirror : public MirrorStone {
        CLONEOBJ(TriangleMirror);
    public:
        TriangleMirror(char orientation='v', bool movable=false, bool transparent=false)
        : MirrorStone("st-3mirror", movable, transparent)
        {
            SetOrientation (orientation);
        }
    private:

        void SetOrientation(char o) {
            const char *a = " v<^>";
            MirrorStone::set_orientation( int (strchr(a,o)-a));
        }

        Direction GetOrientation() // orientation of the flat side of the mirror
        {
            const Direction a[] = {NODIR, NORTH, EAST, SOUTH, WEST};
            return a[MirrorStone::get_orientation()];
        }
        void on_laserhit (Direction dir);
    };
}

void TriangleMirror::on_laserhit(Direction beam_dir)
    // note: 'beam_dir' is the direction where laserbeam goes to
{
    // direction where flat side of triangle points to
    Direction flat_dir = GetOrientation();
    Direction tip_dir  = reverse(flat_dir);

    if (beam_dir == tip_dir) // beam hits the flat side
        emit_light(flat_dir);
    else if (beam_dir == flat_dir) {
        // this is the "complicated" case where the light falls
        // on the tip of the triangle
        switch (beam_dir) {
            case SOUTH: case NORTH:
                emit_light(EAST); emit_light(WEST); break;
            case WEST: case EAST:
                emit_light(SOUTH); emit_light(NORTH); break;
            case NODIR: break;
        }
    } else
        emit_light(tip_dir);

    if (is_transparent())
        emit_light(beam_dir);
}

//----------------------------------------------------------------------
// FUNCTIONS
//----------------------------------------------------------------------
namespace
{
    /* This flag is true iff all lasers should be recalculated at the
       end of the next tick. */
    bool light_recalc_scheduled = false;
}

void lasers::Init() {
    Register (new LaserStone);
    Register ("st-laser-n", new LaserStone(NORTH));
    Register ("st-laser-e", new LaserStone(EAST));
    Register ("st-laser-s", new LaserStone(SOUTH));
    Register ("st-laser-w", new LaserStone(WEST));

    Register (new TriangleMirror);
    Register ("st-mirror-3v", new TriangleMirror('v'));
    Register ("st-mirror-3<", new TriangleMirror('<'));
    Register ("st-mirror-3^", new TriangleMirror('^'));
    Register ("st-mirror-3>", new TriangleMirror('>'));
    Register ("st-mirror-3vm", new TriangleMirror('v', true));
    Register ("st-mirror-3<m", new TriangleMirror('<', true));
    Register ("st-mirror-3^m", new TriangleMirror('^', true));
    Register ("st-mirror-3>m", new TriangleMirror('>', true));
    Register ("st-mirror-3vt", new TriangleMirror('v', false, true));
    Register ("st-mirror-3<t", new TriangleMirror('<', false, true));
    Register ("st-mirror-3^t", new TriangleMirror('^', false, true));
    Register ("st-mirror-3>t", new TriangleMirror('>', false, true));
    Register ("st-mirror-3vtm", new TriangleMirror('v', true, true));
    Register ("st-mirror-3<tm", new TriangleMirror('<', true, true));
    Register ("st-mirror-3^tm", new TriangleMirror('^', true, true));
    Register ("st-mirror-3>tm", new TriangleMirror('>', true, true));

    Register (new PlaneMirror);
    Register ("st-mirror-p|", new PlaneMirror('|'));
    Register ("st-mirror-p/", new PlaneMirror('/'));
    Register ("st-mirror-p-", new PlaneMirror('-'));
    Register ("st-mirror-p\\", new PlaneMirror('\\'));
    Register ("st-mirror-p|m", new PlaneMirror('|', true));
    Register ("st-mirror-p/m", new PlaneMirror('/', true));
    Register ("st-mirror-p-m", new PlaneMirror('-', true));
    Register ("st-mirror-p\\m", new PlaneMirror('\\', true));
    Register ("st-mirror-p|t", new PlaneMirror('|', false, true));
    Register ("st-mirror-p/t", new PlaneMirror('/', false, true));
    Register ("st-mirror-p-t", new PlaneMirror('-', false, true));
    Register ("st-mirror-p\\t", new PlaneMirror('\\', false, true));
    Register ("st-mirror-p|tm", new PlaneMirror('|', true, true));
    Register ("st-mirror-p/tm", new PlaneMirror('/', true, true));
    Register ("st-mirror-p-tm", new PlaneMirror('-', true, true));
    Register ("st-mirror-p\\tm", new PlaneMirror('\\', true, true));
}


void lasers::MaybeRecalcLight(GridPos p) {
    light_recalc_scheduled |=
        (LightFrom(p, NORTH) || LightFrom(p, SOUTH) ||
         LightFrom(p, WEST) || LightFrom(p, EAST));
}

void lasers::RecalcLight() {
    light_recalc_scheduled = true;
}

bool lasers::LightFrom (GridPos p, Direction dir) {
    p.move(dir);
    if (LaserEmitter *le = dynamic_cast<LaserEmitter*>(GetStone(p)))
        if (has_dir(le->emission_directions(), reverse(dir)))
            return true;
    if (LaserEmitter *le = dynamic_cast<LaserEmitter*>(GetItem(p)))
        return (has_dir(le->emission_directions(), reverse(dir)));
    return false;
}

void lasers::RecalcLightNow() {
    if (light_recalc_scheduled) {
        PhotoCell::notify_start();
        LaserBeam::kill_all();
        LaserStone::reemit_all();
        PhotoCell::notify_finish();
        LaserBeam::all_emitted();
        light_recalc_scheduled = false;
    }
}
