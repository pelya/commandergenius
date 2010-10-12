/*
 * Copyright (C) 2002,2003,2004 Daniel Heck
 *
 * This program is free software; you can redistribute it and/ or
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
#include "laser.hh"
#include "sound.hh"
#include "server.hh"
#include "player.hh"
#include "Inventory.hh"
#include "stones_internal.hh"
#include "actors.hh"

#include "ecl_util.hh"

#include <algorithm>
#include <iostream>

using namespace std;
using namespace world;
using namespace stones;


/* -------------------- RotatorStone -------------------- */
namespace
{
    class RotatorStone : public PhotoStone {
    public:
        RotatorStone(bool clockwise_, bool movable_)
        : PhotoStone("st-rotator"), clockwise(clockwise_), movable(movable_)
        {}

    private:
        static const double RATE;
        static const double IMPULSE_DELAY;

        bool clockwise;
        bool movable;

        Stone *clone() { return new RotatorStone(clockwise, movable); }
        void dispose() { delete this; }
        
        // Stone interface
        void on_creation (GridPos p) {
            Stone::on_creation(p);
            photo_activate();
        }
        void on_removal (GridPos p){
            photo_deactivate();
            Stone::on_removal(p);
        }

        void send_impulses() {
            GridPos p = get_pos();

            if (clockwise) {
                send_impulse (move(p, NORTH), EAST, IMPULSE_DELAY);
                send_impulse (move(p, EAST), SOUTH, IMPULSE_DELAY);
                send_impulse (move(p, SOUTH), WEST, IMPULSE_DELAY);
                send_impulse (move(p, WEST), NORTH, IMPULSE_DELAY);
            } else {
                send_impulse (move(p, NORTH), WEST, IMPULSE_DELAY);
                send_impulse (move(p, EAST), NORTH, IMPULSE_DELAY);
                send_impulse (move(p, SOUTH), EAST, IMPULSE_DELAY);
                send_impulse (move(p, WEST), SOUTH, IMPULSE_DELAY);
            }
        }


        void init_model() {
            set_anim(clockwise ? "st-rotator-right" : "st-rotator-left");
        }
        void animcb() {
            init_model();
            send_impulses();
        }

        bool is_movable () const { return movable; }

        void actor_hit (const StoneContact &sc) {
            if (player::WieldedItemIs (sc.actor, "it-wrench")) {
                clockwise = !clockwise;
                init_model();
            }

            if (movable)
                maybe_push_stone(sc);
        }

        void on_impulse(const Impulse& impulse) {
            if (movable)
                move_stone(impulse.dir);
        }

        // PhotoStone interface
        void notify_laseron() {
            clockwise = !clockwise;
            init_model();
        }
        void notify_laseroff() {}
    };

    const double RotatorStone::RATE          = 1.0;
    const double RotatorStone::IMPULSE_DELAY = 0.1;
}


/* -------------------- PullStone -------------------- */

// When pushed this stone acts like pulled.
// When pushed by an actor it exchanges its position with the actor.

namespace
{
    struct PulledActor {
        // Variables
        Actor *actor;
        V2     speed;

        // Constructor
        PulledActor(Actor *actor_, const V2& speed_)
        : actor(actor_), speed(speed_)
        {
        }

    };

    class PullInfo {
        list<PulledActor>  actors;
        YieldedGridStone  *ystone;

    public:
        PullInfo(Stone *st)
        : ystone(st ? new YieldedGridStone(st) : 0)
        {}
        ~PullInfo() { delete ystone; }

        void add_actor(Actor *actor, const V2& speed) {
            actors.push_back(PulledActor(actor, speed));
        }

        list<PulledActor>& get_actors() { return actors; }

        void set_stone(GridPos pos) { if (ystone) ystone->set_stone(pos); }
        void dispose() { if (ystone) ystone->dispose(); }
    };


    class PullStone : public Stone, public TimeHandler {
        // Variables.
        enum State { IDLE, MOVING, VANISHED } state;
        Direction  m_movedir;
        PullInfo  *pull_info;   // information about moved objects (only valid during pull)

    public:
        PullStone();
        ~PullStone();

    private:
        // Object interface.
        PullStone *clone();
        void       dispose();

        // Stone interface.
        bool is_movable () const {
            return state == IDLE;
        }
        void actor_hit(const StoneContact &sc) {
            if (state == IDLE)
                maybe_push_stone(sc);
        }
        void on_impulse(const Impulse& impulse);
        bool is_removable() const {
            return state == IDLE;
        }

        // TimeHandler interface.
        void alarm();

        // Functions.
        void change_state(State new_state);
        void set_move_state(bool appearing, Direction move_dir);

    };
}

PullStone::PullStone()
: Stone("st-pull"), state(IDLE), m_movedir(NODIR) , pull_info(0)
{}

PullStone::~PullStone() {
    GameTimer.remove_alarm(this);
    delete pull_info;
}

PullStone *PullStone::clone() {
    PullStone *other = new PullStone(*this);
    other->pull_info = 0;
    return other;
}

void PullStone::dispose() {
    if (state == MOVING && pull_info != 0)
        pull_info->dispose();
    delete this;
}

void PullStone::set_move_state (bool appearing, Direction move_dir) {
    if (appearing) {
        m_movedir = move_dir;
        // only the half-stone on the new field gets an alarm
        // wherein it completes the move
        GameTimer.set_alarm(this, 0.09, false);
    }
    else
        m_movedir = reverse(move_dir);
    change_state(MOVING);
}

void PullStone::change_state (State new_state) {
    switch (new_state) {
    case IDLE: set_model("st-pull"); break;
    case MOVING: {
            string mname = string("st-pull") + to_suffix(m_movedir);
            set_model(mname);
            break;
        }
    case VANISHED: break;
    }
    state = new_state;
}

void PullStone::alarm() {
    ASSERT(state == MOVING, XLevelRuntime, "PullStone: alarm called with inconsistent state");
    GridPos oldpos = move (get_pos(), reverse(m_movedir));

    // remove the disappearing half of the PullStone :
    PullStone *oldStone = dynamic_cast<PullStone*>(GetStone(oldpos));
    ASSERT(oldStone, XLevelRuntime, "PullStone: oldStone non-existent in alarm");
    oldStone->change_state(VANISHED);
    KillStone(oldpos);

    if (pull_info) {            // have other objects been moved ?
        pull_info->set_stone(oldpos); // re-sets any pulled stone

        // set pulled actor(s):
        list<PulledActor>::iterator e = pull_info->get_actors().end();
        for (list<PulledActor>::iterator i = pull_info->get_actors().begin(); i != e; ++i) {
            PulledActor& pulled = *i;
            pulled.actor->get_actorinfo()->vel = pulled.speed;
        }
        delete pull_info;
        pull_info = 0;
    }

    change_state(IDLE);
}

void PullStone::on_impulse(const Impulse& impulse) 
{
    if (state != IDLE)
        return;

    Direction       move_dir    = reverse(impulse.dir);
    const GridPos&  oldPos      = get_pos();
    GridPos         newPos      = move(oldPos, move_dir);
    Stone          *other_stone = GetStone(newPos);

    if (other_stone &&
        (!other_stone->is_removable() || IsLevelBorder(newPos))) {
        return;                 // avoid unremoveable and border stones
    }

    PullStone *newStone = this->clone();

    if (other_stone) {
        // yield other_stone:
        newStone->pull_info = new PullInfo(other_stone);
    }
    else {
        newStone->pull_info = new PullInfo(0);
    }

    // search for affected actors
    vector<Actor*> found_actors;
    const double   range_one_field = 1.415; // approx. 1 field [ > sqrt(1+1) ]
    GetActorsInRange(newPos.center(), range_one_field, found_actors);
    vector<Actor*>::iterator e = found_actors.end();
    for (vector<Actor*>::iterator i = found_actors.begin(); i != e; ++i) {
        Actor     *actor     = *i;
        ActorInfo *ai        = actor->get_actorinfo();
        GridPos    actor_pos(ai->pos);

        if (actor_pos == newPos) { // if the actor is in the dest field
            V2 vel = ai->vel;
            V2 mid_dest = ai->pos;

            mid_dest[0] = ecl::Clamp<double> (mid_dest[0], oldPos.x+0.01, oldPos.x+0.99);
            mid_dest[1] = ecl::Clamp<double> (mid_dest[1], oldPos.y+0.01, oldPos.y+0.99);
            WarpActor(actor, mid_dest[0], mid_dest[1], false);

            newStone->pull_info->add_actor(actor, vel);
        }
    }

    SetStone(newPos, newStone);
    newStone->set_move_state(true, move_dir);
    set_move_state(false, move_dir);

    sound_event("moveslow");
}


/* -------------------- Oneway stones -------------------- */

// These stone can only be passed in one direction.

namespace
{
    class OneWayBase : public Stone {
    protected:
        OneWayBase(const char *kind, Direction dir);

        void init_model();
        virtual Value message(const string& msg, const Value &val);

        void actor_hit (const StoneContact&);
        StoneResponse collision_response(const StoneContact &sc);
        bool is_floating() const { return true; }

        Direction get_orientation() const {
            return Direction(int_attrib("orientation"));
        }
        void set_orientation(Direction dir) {
            set_attrib("orientation", Value(dir));
        }

        virtual bool actor_may_pass (Actor *a) = 0;
    };

    class OneWayStone : public OneWayBase {
    public:
        OneWayStone(Direction dir=SOUTH) : OneWayBase("st-oneway", dir) {}
    private:
        CLONEOBJ(OneWayStone);
        virtual bool actor_may_pass (Actor * /*a*/) { return true; }
    };


    class OneWayStone_black : public OneWayBase {
    public:
        OneWayStone_black(Direction dir=SOUTH)
            : OneWayBase("st-oneway_black",dir) {}
    private:
        CLONEOBJ(OneWayStone_black);
        virtual bool actor_may_pass (Actor *a) {
            return a->get_attrib("blackball") != 0;
        }
        void actor_hit (const StoneContact&) {
            // do nothing if hit by actor
        }
    };

    class OneWayStone_white : public OneWayBase {
    public:
        OneWayStone_white(Direction dir=SOUTH)
            : OneWayBase("st-oneway_white", dir) {}
    private:
        CLONEOBJ(OneWayStone_white);
        virtual bool actor_may_pass (Actor *a) {
            return a->get_attrib("whiteball") != 0;
        }
        void actor_hit (const StoneContact&) {
            // do nothing if hit by actor
        }
    };
}

OneWayBase::OneWayBase(const char *kind, Direction dir) 
: Stone(kind)
{
    set_orientation(dir);
}

void OneWayBase::init_model()
{
    string mname = get_kind();
    mname += to_suffix(get_orientation());
    set_model (mname);
}

Value OneWayBase::message(const string& msg, const Value &val) {
    if (msg == "direction" && val.get_type() == Value::DOUBLE) {
        set_orientation(to_direction(val));
        init_model();
    }
    else if (msg == "signal" || msg == "flip") {
        Direction dir = get_orientation();
        set_orientation(reverse(dir));
        init_model();
    }
    return Value();
}

void OneWayBase::actor_hit(const StoneContact &sc) {
    Direction o=get_orientation();

    if (has_dir(contact_faces(sc), o)) {
        if (player::WieldedItemIs (sc.actor, "it-magicwand")) {
            set_orientation(reverse(o));
            init_model();
        }
    }
}

StoneResponse OneWayBase::collision_response(const StoneContact &sc) {
    DirectionBits dirs=contact_faces(sc);
    Direction o=get_orientation();

    if (!sc.actor->is_flying() && actor_may_pass(sc.actor))
        return has_dir(dirs,o) ? STONE_REBOUND : STONE_PASS;
    else
        return STONE_REBOUND;
}


/* -------------------- BolderStone -------------------- */

/** \page st-bolder Bolder Stone

The bolder stone will move in one direction until another stone will
block.  When hit with a magic wand, the bolder stone reverse its
direction. When hitting a blocking stone it can activate switches or
oxyd stones.

\subsection boldera Attributes

- \c direction  \n NORTH, EAST, SOUTH, WEST

*/
namespace
{
    class BolderStone : public Stone {
        CLONEOBJ(BolderStone);
    public:
        BolderStone(Direction dir=NORTH)
        : Stone("st-bolder"), state(IDLE)
        {
            set_attrib("direction", dir);
            // do not use set_dir, because this will set the state to ACTIVE
        }

    private:
        enum State {
            ACTIVE,             // may send trigger into direction
            IDLE,               // already sent trigger w/o success
            FALLING             // falling into abyss
        } state;


        Direction get_dir() const {
            return static_cast<Direction>(int_attrib("direction"));
        }
        void set_dir(Direction d) {
            if (d != get_dir())
                state = ACTIVE; // if turned by it-magicwand -> allow triggering
            set_attrib("direction", d);
        }

        void on_floor_change() {
            Floor *fl = GetFloor(get_pos());
            if (fl->is_kind("fl-abyss")) {
                state = FALLING;
                init_model();
            }
        }

        bool have_obstacle (Direction dir) {
            return GetStone(move(get_pos(), dir)) != 0;
        }

        void trigger_obstacle (Direction dir) {
            if (Stone *st = GetStone(move(get_pos(), dir))) {
                SendMessage(st, "trigger", Value(dir));
            }
        }

        void on_move() {
            state = ACTIVE;
            trigger_obstacle(get_dir());
            Stone::on_move();
        }

        // Stone interface.
        void init_model() {
            string mname  = "st-bolder" + to_suffix(get_dir());
            if (state == FALLING)
                mname += "-fall-anim";
            set_anim(mname);
        }

        void animcb() {
            display::Model *m = get_model();
            Direction dir = get_dir();
            switch (state) {
            case FALLING:
                KillStone(get_pos());
//                init_model();
                break;

            case IDLE:
                if (!have_obstacle(dir)) {
                    state = ACTIVE;
                    trigger_obstacle(dir);
                }
//                 if (Model *m = get_model())
                m->restart();
//                init_model();
                break;

            case ACTIVE: {
                trigger_obstacle(dir);
                if (!move_stone(dir)) {
//                    if (state == MOVING) // state may be FALLING
                        state = IDLE;
                }
                init_model();
                break;
            }
            }
        }

        bool is_movable() const { return state != FALLING; }

        void actor_hit(const StoneContact &sc) {
            if (player::WieldedItemIs (sc.actor, "it-magicwand")) {
                set_dir(reverse(get_dir()));
                init_model();
            }
        }

        void on_laserhit(Direction) {
            set_dir(reverse(get_dir()));
            init_model();

            // @@@ FIXME:  the direction should only be inverted on NEW laserbeam
            // not on every light-recalc. Need to use PhotoCell!
        }

        void on_impulse (const Impulse& impulse) {
            if (state == FALLING)
                return;

            if (impulse.sender && impulse.sender->is_kind("st-rotator")) {
                set_dir(impulse.dir);
            }
            init_model();
            move_stone(impulse.dir);
        }

        virtual Value message(const string& msg, const Value &val) {
            if (msg == "direction" && state != FALLING) {
                set_dir (to_direction(val));
                init_model();
            }
            return Value();
        }
    };
}


/* -------------------- BlockerStone -------------------- */

/** \page st-blocker Blocker Stone

The BlockerStone acts like a normal stone until it is hit by a
BolderStone. Then it shrinks and morphs into a 'Blocker' item.

*/

namespace
{
    class BlockerStone : public Stone {
        CLONEOBJ(BlockerStone);
    public:
        BlockerStone(bool solid)
        : Stone(solid ? "st-blocker" : "st-blocker-growing"), 
          state(solid ? SOLID : GROWING)
        {}

    private:
        enum State { SOLID, SHRINKING, GROWING } state;

        void init_model() {
            switch (state) {
            case SOLID:
                set_model("st-blocker");
                break;

            case SHRINKING:
                set_anim("st-blocker-shrinking");
                break;

            case GROWING:
                set_anim("st-blocker-growing");
                break;
            }
        }

        void change_state(State newState) {
            if (state != newState) {
                if (state == GROWING && newState == SHRINKING) {
                    state = SHRINKING;
                    get_model()->reverse();
                }
                else if (state == SHRINKING && newState == GROWING) {
                    state = GROWING;
                    get_model()->reverse();
                }
                else {
                    state = newState;
                    init_model();
                    if (newState == SOLID) {
                        set_attrib("kind", "st-blocker");
                    }
                }
            }
        }

        void animcb() {
            switch (state) {
            case SHRINKING: {
                Item *it = world::MakeItem("it-blocker-new");
                world::SetItem(get_pos(), it);
                TransferObjectName(this, it);
                world::KillStone(get_pos());
                break;
            }
            case GROWING:
                change_state(SOLID);
                break;
            default :
                ASSERT(0, XLevelRuntime,
                    "BlockerStone: animcb called with inconsistent state");
                break;
            }
        }

        virtual Value message(const string &msg, const Value &val) {
            if (msg == "trigger" || msg == "openclose") {
                if (state == SHRINKING) {
                    change_state(GROWING);
                }
                else {
                    change_state(SHRINKING);
                }
            }
            else if (msg == "signal") {
                int value = to_int(val);
//                 warning("received signal (value=%i)", value);
                if (value) {    // value == 1 -> shrink
                    if (state != SHRINKING)
                        change_state(SHRINKING);
                }
                else {          // value == 0 -> grow
                    if (state == SHRINKING)
                        change_state(GROWING);
                }
            }
            else if (msg == "open") { // aka "shrink"
                if (state != SHRINKING)
                    change_state(SHRINKING);
            }
            else if (msg == "close") { // aka "grow"
                if (state == SHRINKING)
                    change_state(GROWING);
            }
            return Value();
        }

        void actor_contact(Actor *a) {
            if (state == GROWING) {
                SendMessage(a, "shatter");
            }
        }
        void actor_inside(Actor *a) {
            if (state == GROWING) {
                SendMessage(a, "shatter");
            }
        }
    };
}


/* -------------------- Volcano -------------------- */
namespace
{
    class VolcanoStone : public Stone {
        CLONEOBJ(VolcanoStone);
    public:
        enum State {INACTIVE, ACTIVE, FINISHED, BREAKING};
        VolcanoStone( State initstate=INACTIVE) : Stone("st-volcano"), state( initstate) {}
    private:
        enum State state;

        void init_model() {
            switch( state) {
            case FINISHED:
            case INACTIVE: set_model( "st-plain"); break;
            case ACTIVE: set_anim( "st-farting"); break;
            case BREAKING: set_anim("st-stone_break-anim"); break;
            }
        }

        void animcb() {
            if (state == ACTIVE) {
                // Spread
                GridPos p = get_pos();
                if (DoubleRand(0, 1) > 0.7) spread (move(p, NORTH));
                if (DoubleRand(0, 1) > 0.7) spread (move(p, EAST));
                if (DoubleRand(0, 1) > 0.7) spread (move(p, SOUTH));
                if (DoubleRand(0, 1) > 0.7) spread (move(p, WEST));

                // Be finished at random time
                if (DoubleRand(0, 1) > 0.95)
                    state = FINISHED;
                init_model();
            } else if( state == BREAKING) {
                KillStone( get_pos());
            }
        }

        virtual Value message(const string &msg, const Value &) {
            if (msg == "trigger") {
                if (state == INACTIVE) {
                    state = ACTIVE;
                    init_model();
                }
            }
            return Value();
        }

        void spread( GridPos p) {
            Stone *st = GetStone(p);
            if( !st) {
                Item *it = MakeItem("it-seed_volcano");
                SetItem( p, it);
                SendMessage( it, "grow");
            }
        }

        void actor_hit(const StoneContact &sc) {
            Actor *a = sc.actor;

            if( state == ACTIVE && player::WieldedItemIs (a, "it-hammer")) {
                state = BREAKING;
                init_model();
            }
        }
    };
}


/* -------------------- ConnectiveStone -------------------- */

// base class for PuzzleStone and BigBrick

namespace {
    class ConnectiveStone : public Stone {
    public:
        ConnectiveStone(const char *kind, int connections)
        : Stone(kind)
        {
            set_attrib("connections", connections);
        }

        DirectionBits get_connections() const;
    protected:
        void init_model();
    private:
        virtual int get_modelno() const;
    };
}

DirectionBits
ConnectiveStone::get_connections() const
{
    int conn=int_attrib("connections") - 1;
    if (conn >=0 && conn <16)
        return DirectionBits(conn);
    else
        return NODIRBIT;
}

void ConnectiveStone::init_model() {
    set_model(get_kind()+ecl::strf("%d", get_modelno()));
}

int ConnectiveStone::get_modelno() const {
    return int_attrib("connections");
}


/* -------------------- BigBrick -------------------- */

// BigBricks allow to build stones of any size

namespace 
{
    class BigBrick : public ConnectiveStone {
        CLONEOBJ(BigBrick);
    public:
        BigBrick(int connections)
        : ConnectiveStone("st-bigbrick", connections)
        {}
        bool is_removable() const { return false; }
   };
}

/* -------------------- BigBlueSand -------------------- */

// Same as BigBrick but with st-blue-sand

namespace
{
    class BigBlueSand : public ConnectiveStone {
        CLONEOBJ(BigBlueSand);
    public:
        BigBlueSand(int connections)
        : ConnectiveStone("st-bigbluesand", connections)
        {}
        bool is_removable() const { return false; }
    };
}


/* -------------------- Puzzle stones -------------------- */ 

/** \page st-puzzle Puzzle Stone

Puzzle stones can be connected to other stones of the same type.  Any
of the four faces of the stone can have ``socket''.  If the adjoining
faces of two neighboring stones both have a socket, the two stones
link up and henceforth move as group.

A cluster of puzzle stones may for example look like this:

\verbatim
+---+---+---+---+
|   |   |   |   |
| --+-+-+---+-+ |
|   | | |   | | |
+---+-+-+---+-+-+
    | | |   | | |
    | | |   | | |
    |   |   |   |
    +---+   +---+
\endverbatim

This example actually presents the special case of a ``complete''
cluster.  A cluster is complete if none of its stones has an
unconnected socket.

When touched with a magic wand the puzzle stones rotate
row- or columnwise.  

\subsection puzzlea Attributes

- \b connections
   number between 1 an 16.  Each bit in (connections-1) corresponds to
   a socket on one of the four faces.  You will normally simply use
   one of the Lua constants \c PUZ_0000 to \c PUZ_1111.

- \b oxyd
   If 1 then the puzzle stones act oxyd-compatible: Complete clusters
   explode, when they get touched. All other puzzle stones rotate row-
   or columnwise. Groups of oxyd-compatible puzzle stones are shuffled
   randomly at level startup.

\subsection puzzlee Example
<table>
<tr>
<td> \image html st-puzzletempl_0001.png "PUZ_0000"
<td> \image html st-puzzletempl_0002.png "PUZ_0001"
<td> \image html st-puzzletempl_0003.png "PUZ_0010"
<td> \image html st-puzzletempl_0004.png "PUZ_0011"
<tr>
<td> \image html st-puzzletempl_0005.png "PUZ_0100"
<td> \image html st-puzzletempl_0006.png "PUZ_0101"
<td> \image html st-puzzletempl_0007.png "PUZ_0110"
<td> \image html st-puzzletempl_0008.png "PUZ_0111"
<tr>
<td> \image html st-puzzletempl_0009.png "PUZ_1000"
<td> \image html st-puzzletempl_0010.png "PUZ_1001"
<td> \image html st-puzzletempl_0011.png "PUZ_1010"
<td> \image html st-puzzletempl_0012.png "PUZ_1011"
<tr>
<td> \image html st-puzzletempl_0013.png "PUZ_1100"
<td> \image html st-puzzletempl_0014.png "PUZ_1101"
<td> \image html st-puzzletempl_0015.png "PUZ_1110"
<td> \image html st-puzzletempl_0016.png "PUZ_1111"
</tr>
</table>
*/
namespace
{
    class PuzzleStone : public ConnectiveStone, public TimeHandler, public world::PhotoCell {
        INSTANCELISTOBJ(PuzzleStone);
    public:
        PuzzleStone(int connections, bool oxyd1_compatible_);  
    protected:
        virtual ~PuzzleStone() {
            GameTimer.remove_alarm (this);
        }     
    private:
        typedef vector<GridPos> Cluster;

        /* ---------- Private methods ---------- */

        bool oxyd1_compatible() const { return int_attrib("oxyd") != 0; }

        static bool visit_dir(vector<GridPos> &stack, GridPos curpos,
                              Direction dir, int wanted_oxyd_attrib);
        static void visit_adjacent(vector<GridPos>& stack, GridPos curpos,
                                   Direction dir, int wanted_oxyd_attrib);

        bool find_cluster(Cluster &);
        void find_adjacents(Cluster &);
        void find_row_or_column_cluster(Cluster &c, GridPos startpos,
                                        Direction  dir, int wanted_oxyd_attrib);

        bool cluster_complete();
        bool can_move_cluster (Cluster &c, Direction dir);
        void maybe_move_cluster(Cluster &c, bool is_complete, bool actor_with_wand, 
                                Direction dir);
        void rotate_cluster(const Cluster &c);
        void maybe_rotate_cluster(Direction dir);

        int get_modelno() const;

        void        trigger_explosion(double delay);
        static void trigger_explosion_at(GridPos p, double delay, int wanted_oxyd_attrib);
        void        explode();
        bool        explode_complete_cluster();

        /* ---------- TimeHandler interface ---------- */

        void alarm();

        /* ---------- PhotoCell interface ---------- */

        void on_recalc_start();
        void on_recalc_finish();

        /* ---------- Stone interface ---------- */

        virtual Value message(const string& msg, const Value &val);

        void on_creation (GridPos p);
        void on_removal (GridPos p);
        void on_impulse (const Impulse& impulse);
        void on_laserhit (Direction dir);

        bool is_floating() const;

        StoneResponse collision_response(const StoneContact &sc);
        void actor_hit (const StoneContact &sc);
        void actor_contact (Actor *a);

        /* ---------- Variables ---------- */
        bool visited;           // flag for DFS
        enum { IDLE, EXPLODING } state;
        DirectionBits illumination; // last state of surrounding laser beams
    };
}

PuzzleStone::InstanceList PuzzleStone::instances;

PuzzleStone::PuzzleStone(int connections, bool oxyd1_compatible_)
: ConnectiveStone("st-puzzle", connections), 
  state (IDLE), 
  illumination (NODIRBIT)
{
    set_attrib("oxyd", int(oxyd1_compatible_));
}


bool PuzzleStone::visit_dir(vector<GridPos> &stack, GridPos curpos, 
        Direction dir, int wanted_oxyd_attrib)
{
    GridPos newpos = move(curpos, dir);
    PuzzleStone *pz = dynamic_cast<PuzzleStone*>(GetStone(newpos));

    if ((!pz) || (wanted_oxyd_attrib != pz->int_attrib("oxyd")))
        return false;

    DirectionBits cfaces = pz->get_connections();

    if (cfaces==NODIRBIT || has_dir(cfaces, reverse(dir))) {
        // Puzzle stone at newpos is connected to stone at curpos
        if (!pz->visited) {
            pz->visited = true;
            stack.push_back(newpos);
        }
        return true;
    } else {
        // The two stones are adjacent but not connected
        return false;
    }
}

/* Use a depth first search to determine the group of all stones that
   are connected to the current stone.  Returns true if the cluster is
   ``complete'' in the sense defined above. */
bool PuzzleStone::find_cluster(Cluster &cluster) {
    for (unsigned i=0; i<instances.size(); ++i)
        instances[i]->visited=false;

    vector<GridPos> pos_stack;
    bool is_complete = true;
    pos_stack.push_back(get_pos());
    this->visited = true;
    int wanted_oxyd_attrib = int_attrib("oxyd");

    while (!pos_stack.empty())
    {
        GridPos curpos = pos_stack.back();
        pos_stack.pop_back();

        PuzzleStone *pz = dynamic_cast<PuzzleStone*>(GetStone(curpos));
        ASSERT(pz, XLevelRuntime, "PuzzleStone: missing stone in find_cluster");

        cluster.push_back(curpos);
        DirectionBits cfaces = pz->get_connections();

        if (cfaces==NODIRBIT)
            cfaces = DirectionBits(NORTHBIT | SOUTHBIT | EASTBIT | WESTBIT);

        if (has_dir(cfaces, NORTH))
            is_complete &= visit_dir(pos_stack, curpos, NORTH, wanted_oxyd_attrib);
        if (has_dir(cfaces, EAST))
            is_complete &= visit_dir(pos_stack, curpos, EAST, wanted_oxyd_attrib);
        if (has_dir(cfaces, SOUTH))
            is_complete &= visit_dir(pos_stack, curpos, SOUTH, wanted_oxyd_attrib);
        if (has_dir(cfaces, WEST))
            is_complete &= visit_dir(pos_stack, curpos, WEST, wanted_oxyd_attrib);
    }
    return is_complete;
}

void PuzzleStone::visit_adjacent (vector<GridPos>& stack, GridPos curpos, 
                                  Direction dir, int wanted_oxyd_attrib)
{
    GridPos newpos = move(curpos, dir);
    if (PuzzleStone *pz = dynamic_cast<PuzzleStone*>(GetStone(newpos))) {
        if (!pz->visited) {
            if (wanted_oxyd_attrib == pz->int_attrib("oxyd")) {
                pz->visited = true;
                stack.push_back(newpos);
            }
        }
    }
}

/* Use a depth first search to determine the group of all puzzle stones
   with the same "oxyd" attrib that are adjacent to the current stone
   (or to any other member of the group).
*/
void PuzzleStone::find_adjacents(Cluster &cluster) {
    for (unsigned i=0; i<instances.size(); ++i)
        instances[i]->visited=false;

    vector<GridPos> pos_stack;
    pos_stack.push_back(get_pos());
    this->visited = true;

    int wanted_oxyd_attrib = int_attrib("oxyd");

    while (!pos_stack.empty()) {
        GridPos curpos = pos_stack.back();
        pos_stack.pop_back();

        cluster.push_back(curpos);
        visit_adjacent(pos_stack, curpos, NORTH, wanted_oxyd_attrib);
        visit_adjacent(pos_stack, curpos, SOUTH, wanted_oxyd_attrib);
        visit_adjacent(pos_stack, curpos, EAST, wanted_oxyd_attrib);
        visit_adjacent(pos_stack, curpos, WEST, wanted_oxyd_attrib);
    }
}

/* searches from 'startpos' into 'dir' for puzzle-stones.
   wanted_oxyd_attrib == -1 -> take any puzzle stone
                      else  -> take only puzzle stones of same type
*/

void PuzzleStone::find_row_or_column_cluster(Cluster &c, GridPos startpos, 
                                             Direction dir, int wanted_oxyd_attrib)
{
    ASSERT(dir != NODIR, XLevelRuntime,
        "PuzzleStone: no direction in find_row_or_column_cluster");

    GridPos p = startpos;
    while (Stone *puzz = dynamic_cast<PuzzleStone*>(GetStone(p))) {
        if (wanted_oxyd_attrib != -1 && wanted_oxyd_attrib != puzz->int_attrib("oxyd"))
            break; // stop when an unrequested puzzle stone type is readed
        c.push_back(p);
        p.move(dir);
    }
}

bool PuzzleStone::can_move_cluster (Cluster &c, Direction dir)
{
    sort(c.begin(), c.end());
    Cluster mc(c);              // Moved cluster
    Cluster diff;               // Difference between mc and c

    for (unsigned i=0; i<mc.size(); ++i)
        mc[i].move(dir);

    set_difference(mc.begin(), mc.end(), c.begin(), c.end(),
                   back_inserter(diff));

    // Now check whether all stones can be placed at their new
    // position
    bool move_ok = true;
    for (unsigned i=0; i<diff.size(); ++i)
        if (GetStone(diff[i]) != 0)
            move_ok = false;

    return move_ok;    
}

void PuzzleStone::maybe_move_cluster(Cluster &c, bool is_complete, 
                                     bool actor_with_wand, Direction dir)
{
    sort(c.begin(), c.end());
    Cluster mc(c);              // Moved cluster
    Cluster diff;               // Difference between mc and c

    for (unsigned i=0; i<mc.size(); ++i)
        mc[i].move(dir);

    set_difference(mc.begin(), mc.end(), c.begin(), c.end(),
                   back_inserter(diff));

    // Now check whether all stones can be placed at their new
    // position
    bool move_ok = true;
    for (unsigned i=0; i<diff.size(); ++i)
        if (GetStone(diff[i]) != 0)
            move_ok = false;

    if (!move_ok)
        return;

    // If the floor at a complete cluster's new position consists
    // exclusively of abyss or water, create a bridge instead of
    // moving the cluster.
    //
    // For partial clusters build bridges only on water and if the
    // wielded item is NOT the magic wand.

    bool create_bridge = true;

    for (unsigned i=0; create_bridge && i<mc.size(); ++i) {
        if (Floor *fl = GetFloor(mc[i])) {
            if (fl->is_kind("fl-abyss")) {
                if (!is_complete)
                    create_bridge = false;
            }
            else if (fl->is_kind("fl-water")) {
                if (!is_complete && actor_with_wand) 
                    create_bridge = false;
            }
            else
                create_bridge = false;
        }
    }

    // Finally, either move the whole cluster or create a bridge
    sound_event("movebig");
    if (create_bridge) {
        for (unsigned i=0; i<c.size(); ++i) {
            KillStone(c[i]);
            SetFloor(mc[i], MakeFloor("fl-gray"));
        }
    } 
    else {
        vector<Stone*> clusterstones;
        for (unsigned i=0; i<c.size(); ++i)
            clusterstones.push_back(YieldStone(c[i]));

        for (unsigned i=0; i<c.size(); ++i) {
            SetStone(mc[i], clusterstones[i]);
            clusterstones[i]->on_move();
        }
    }

    server::IncMoveCounter (static_cast<int> (c.size()));
}

bool PuzzleStone::cluster_complete() {
    Cluster c;
    return find_cluster(c);
}

int PuzzleStone::get_modelno() const {
    int modelno = int_attrib("connections");
    if (oxyd1_compatible()) modelno += 16;
    return modelno;
}

void PuzzleStone::rotate_cluster(const Cluster &c) {
    size_t size = c.size();
    if (size > 1) {
        int cn = GetStone(c[size-1])->int_attrib("connections");
        for (size_t i=size-1; i>0; --i) {
            PuzzleStone *st = dynamic_cast<PuzzleStone*> (GetStone (c[i]));
            st->set_attrib ("connections", GetStone(c[i-1])->int_attrib ("connections"));
            st->init_model();
        }
        GetStone(c[0])->set_attrib ("connections", cn);
        dynamic_cast<PuzzleStone*> (GetStone(c[0]))->init_model();
    }
}

StoneResponse PuzzleStone::collision_response(const StoneContact &/*sc*/) {
    if (get_connections() == NODIRBIT)
        return STONE_PASS;
    return STONE_REBOUND;
}

void PuzzleStone::trigger_explosion(double delay) {
    if (state == IDLE) {
        state = EXPLODING;
        GameTimer.set_alarm(this, delay, false);
    }
}

void PuzzleStone::trigger_explosion_at (GridPos p, double delay,
                                        int wanted_oxyd_attrib)
{
    PuzzleStone *puzz = dynamic_cast<PuzzleStone*>(GetStone(p));
    if (puzz && wanted_oxyd_attrib == puzz->int_attrib("oxyd")) {
        // explode adjacent puzzle stones of same type
        puzz->trigger_explosion(delay);
    }
}

void PuzzleStone::explode() {
    GridPos p       = get_pos();
    int     ox_attr = int_attrib("oxyd");

    // exchange puzzle stone with explosion
    sound_event("stonedestroy");
    SetStone(p, MakeStone("st-explosion"));

    // trigger all adjacent puzzle stones :
    const double DEFAULT_DELAY = 0.2;
    trigger_explosion_at(move(p, NORTH), DEFAULT_DELAY, ox_attr);
    trigger_explosion_at(move(p, SOUTH), DEFAULT_DELAY, ox_attr);
    trigger_explosion_at(move(p, EAST),  DEFAULT_DELAY, ox_attr);
    trigger_explosion_at(move(p, WEST),  DEFAULT_DELAY, ox_attr);

    // @@@ FIXME: At the moment it's possible to push partial puzzle stones
    // next to an already exploding cluster. Then the part will explode as well.
    // Possible fix : mark whole cluster as "EXPLODING_SOON" when explosion is initiated

    // ignite adjacent fields
//     SendExplosionEffect(p, DYNAMITE);
}

void PuzzleStone::alarm() {
    explode();
}

Value PuzzleStone::message(const string& msg, const Value &val) {
    if (msg == "scramble") {
        // oxyd levels contain explicit information on how to
        // scramble puzzle stones. According to that information
        // a "scramble" message is send to specific puzzle stones
        // together with information about the direction.
        //
        // enigma levels may create scramble messages using
        // AddScramble() and SetScrambleIntensity()

        Direction dir = to_direction(val);
        Cluster   c;
        find_row_or_column_cluster(c, get_pos(), dir, oxyd1_compatible());

        size_t size = c.size();

        // warning("received 'scramble'. dir=%s size=%i", to_suffix(dir).c_str(), size);

        if (size >= 2) {
            int count = IntegerRand(0, static_cast<int> (size-1));
            while (count--)
                rotate_cluster(c);
        }
        else {
            warning("useless scramble (cluster size=%i)", size);
        }
    }
    return Value();
}

void PuzzleStone::on_impulse(const Impulse& impulse) 
{
//    if (!oxyd1_compatible() && state == IDLE) {
    if (state == IDLE) {
        Cluster c;
        bool    is_complete     = find_cluster(c);
        bool    actor_with_wand = false;

        if (Actor *ac = dynamic_cast<Actor*>(impulse.sender)) 
            actor_with_wand = player::WieldedItemIs (ac, "it-magicwand");

        maybe_move_cluster(c, is_complete, actor_with_wand, impulse.dir);
    }
}

bool PuzzleStone::explode_complete_cluster() 
{
    // @@@ FIXME: explode_complete_cluster should mark the whole cluster
    // as "EXPLODING_SOON" (otherwise it may be changed before it explodes completely)

    ASSERT(state == IDLE, XLevelRuntime,
        "PuzzleStone: explode_complete_cluster called with inconsistent state");
    bool exploded = false;

    Cluster complete;
    if (find_cluster(complete)) {
        Cluster all;
        find_adjacents(all);

        // If all adjacent stones build one complete cluster explode it
        if (all.size() == complete.size()) {
            explode();          // explode complete cluster
            exploded = true;
        }
        else {
            ASSERT(all.size() > complete.size(), XLevelRuntime,
                "PuzzleStone: sizes don't match in explode_complete_cluster");
            if (!oxyd1_compatible()) {
                // check if 'all' is made up of complete clusters :

                sort(all.begin(), all.end());

                while (1) {
                    sort(complete.begin(), complete.end());

                    // remove one complete cluster from 'all'
                    {
                        Cluster rest;
                        set_symmetric_difference(all.begin(), all.end(),
                                                 complete.begin(), complete.end(),
                                                 back_inserter(rest));
                        // now rest contains 'all' minus 'complete'
                        swap(all, rest);
                    }

                    if (all.empty()) { // none left -> all were complete
                        exploded = true;
                        break;
                    }

                    // look for next complete cluster :
                    complete.clear();
                    {
                        PuzzleStone *pz = dynamic_cast<PuzzleStone*>(GetStone(all[0]));
                        ASSERT(pz, XLevelRuntime,
                            "PuzzleStone: missing stone in explode_complete_cluster");
                        if (!pz->find_cluster(complete)) {
                            break; // incomplete cluster found -> don't explode
                        }
                    }
                }

                if (exploded) {
//                     warning("exploding complete cluster");
                    explode();
                }
            }
        }
    }

    return exploded;
}

bool PuzzleStone::is_floating() const {
    return get_connections() == 0;
}

void PuzzleStone::maybe_rotate_cluster(Direction dir) 
{
    if (dir != NODIR) {
        Cluster c;
        find_row_or_column_cluster(c, get_pos(), dir, int_attrib ("oxyd"));
        if (c.size() >= 2) {
//             warning("ok -> rotate");
            rotate_cluster(c);
        }
    }
}

void PuzzleStone::on_creation (GridPos p) {
    photo_activate();
    ConnectiveStone::on_creation (p);
    illumination = NODIRBIT;
}

void PuzzleStone::on_removal(GridPos p) {
    photo_deactivate();
    ConnectiveStone::on_removal(p);
}

void PuzzleStone::on_laserhit (Direction dir) {
    ecl::set_flags (illumination, to_bits(reverse(dir)));
}

void PuzzleStone::on_recalc_start() {
    illumination = NODIRBIT;
}

void PuzzleStone::on_recalc_finish() {
    if (illumination != (ALL_DIRECTIONS+1) && 
        illumination != NODIRBIT && 
        state == IDLE) 
    {
        if (!explode_complete_cluster() && oxyd1_compatible()) {
            if (illumination & NORTHBIT) maybe_rotate_cluster(SOUTH);
            if (illumination & SOUTHBIT) maybe_rotate_cluster(NORTH);
            if (illumination & EASTBIT)  maybe_rotate_cluster(WEST);
            if (illumination & WESTBIT)  maybe_rotate_cluster(EAST);
        }
    }
}

void PuzzleStone::actor_hit(const StoneContact &sc) 
{
    if (get_connections() == NODIRBIT)
        return;                 // Puzzle stone is hollow

    if (state == EXPLODING)
        return;

    Cluster c;
    find_cluster (c);

    Direction rotate_dir = reverse (contact_face (sc));
    Direction move_dir = get_push_direction(sc);

    if (oxyd1_compatible()) {
        // Oxyd 1

        if (explode_complete_cluster())
            return;

        // 1) If unconnected puzzle stones -> try to move it
        if (c.size() == 1 && move_dir != NODIR) {
            // if cluster contains single stone
            // -> move it if dest pos is free
            GridPos dest = move(c[0], move_dir);
            if (GetStone(dest) == 0) {
                Stone *puzz = YieldStone(c[0]);
                SetStone(dest, puzz);
                puzz->on_move();
                sound_event ("movesmall");
            } else
                maybe_rotate_cluster (rotate_dir);
        } 
        // 2) If more than one stone, 
        else 
            maybe_rotate_cluster (rotate_dir);
    }
    else {
        // Not Oxyd 1

        bool has_magic_wand = player::WieldedItemIs (sc.actor, "it-magicwand");

        // 1) Try to start explosion of complete cluster
        if (has_magic_wand && explode_complete_cluster())
            return;

        // 2) Failed? Try to move the cluster
        if (move_dir != NODIR && can_move_cluster (c, move_dir)) {
            sc.actor->send_impulse(get_pos(), move_dir);
            return;
        }

        // 3) Last chance: try to rotate the row or column 
        if (has_magic_wand)
            maybe_rotate_cluster (rotate_dir);
    }
}

void PuzzleStone::actor_contact (Actor *a)
{
    if (state == EXPLODING)
        SendMessage (a, "shatter");
}



/* -------------------- DoorBase -------------------- */

// Base class for everything that behaves like a door, i.e., it has
// four states OPEN, CLOSED, OPENING, CLOSING.
namespace
{
    class DoorBase : public Stone {
    protected:
        enum State { OPEN, CLOSED, OPENING, CLOSING } state;

        DoorBase(const char *name, State initstate=CLOSED)
        : Stone(name), state(initstate)
        {}

        State get_state() const { return state; }
        void set_state(State st) { state=st; }

    private:
        // DoorBase interface
        virtual string model_basename() { return get_kind(); }
        virtual void init_model();
        virtual string opening_sound() const { return ""; }
        virtual string closing_sound() const { return ""; }

        // Private methods
        void change_state(State newstate) ;
        virtual Value message(const string &m, const Value &);

        StoneResponse collision_response(const StoneContact &sc);

        void animcb();

        // Stone interface
        virtual bool is_transparent (Direction) const
        { return state==OPEN; }

        virtual bool is_sticky (const Actor *) const
        { return false; }
    };
}

Value DoorBase::message(const string &m, const Value &val) {
    State newstate = state;
    int ival = to_int (val);

    if (m == "open")
        newstate = OPENING;
    else if (m == "close")
        newstate = CLOSING;
    else if (m == "openclose")
        newstate = (state==OPEN || state==OPENING) ? CLOSING : OPENING;
    else if (m == "signal")
        newstate = ival > 0 ? OPENING : CLOSING;

    if (newstate==OPENING && (state==CLOSED || state==CLOSING))
        change_state(OPENING);
    else if (newstate==CLOSING && (state==OPEN || state==OPENING))
        change_state(CLOSING);
    return Value();
}

void DoorBase::init_model() {
    string mname = model_basename();
    if (state == CLOSED)
        mname += "-closed";
    else if (state==OPEN)
        mname += "-open";
    set_model(mname);
}

void DoorBase::animcb() {
    if (state == OPENING)
        change_state(OPEN);
    else if (state == CLOSING)
        change_state(CLOSED);
}

StoneResponse
DoorBase::collision_response(const StoneContact &/*sc*/)
{
    return (state == OPEN) ? STONE_PASS:STONE_REBOUND;
}

void DoorBase::change_state(State newstate) 
{
    string basename = model_basename();

    switch (newstate) {
    case OPEN:
        set_model(basename+"-open");
        lasers::MaybeRecalcLight(get_pos());
        break;
    case CLOSED:
        set_model(basename+"-closed");
        world::ShatterActorsInsideField (get_pos());
        lasers::MaybeRecalcLight(get_pos()); // maybe superfluous
        break;
    case OPENING:
        sound_event (opening_sound().c_str());
        if (state == CLOSING)
            get_model()->reverse();
        else
            set_anim(basename+"-opening");
        break;
    case CLOSING:
        sound_event (closing_sound().c_str());
        if (state == OPENING)
            get_model()->reverse();
        else
            set_anim(basename+"-closing");
        world::ShatterActorsInsideField (get_pos());
        lasers::MaybeRecalcLight(get_pos());
        break;
    }
    set_state(newstate);
}


/* -------------------- Door -------------------- */

// Attributes:
//
// :type        h or v for a door that opens horizontally or vertically
namespace
{
    class Door : public DoorBase {
        CLONEOBJ(Door);
    public:
        Door(const char *type="h", bool open=false)
        : DoorBase("st-door", open ? OPEN : CLOSED)
        {
            set_attrib("type", type);
        }
    private:
        virtual string opening_sound() const { return "dooropen"; }
        virtual string closing_sound() const { return "doorclose"; }
        virtual const char *collision_sound() { return "electric"; }
        string get_type() const {
            string type="h";
            string_attrib("type", &type);
            return type;
        }

        bool is_transparent (Direction) const;
        bool is_floating () const {
            return true;        // don't let door press buttons
        }

        void actor_hit(const StoneContact &)
        {
            if (Item *it = GetItem (get_pos()))
                PerformAction (it, true);
        }

        string model_basename() { return string("st-door")+get_type(); }
        StoneResponse collision_response(const StoneContact &sc);
    };

    class Door_a : public DoorBase {
        CLONEOBJ(Door_a);
    public:
        Door_a() : DoorBase("st-door_a") {}
    };

    class Door_b : public DoorBase {
        CLONEOBJ(Door_b);
    public:
        Door_b() : DoorBase("st-door_b") {}
    };

    class Door_c : public DoorBase {
        CLONEOBJ(Door_c);
    public:
        Door_c() : DoorBase("st-door_c") {}
    };
}

bool Door::is_transparent (Direction dir) const {
    if (get_type() == "h")
        return state==OPEN || dir==EAST || dir==WEST;
    else
        return state==OPEN || dir==NORTH || dir==SOUTH;
}

StoneResponse
Door::collision_response(const StoneContact &sc)
{
    Direction cf = contact_face(sc);
    if (state == OPEN)
        return STONE_PASS;
    else if (state == CLOSING)
        return STONE_REBOUND;
    else {
        string t = get_type();
        return ((t == "v" && (cf==WEST || cf==EAST)) ||
                (t == "h" && (cf==SOUTH || cf==NORTH)))
            ? STONE_REBOUND
            : STONE_PASS;
    }
}


/* -------------------- ShogunStone -------------------- */

// Attributes:
//
// :holes            1..7
namespace
{
    class ShogunStone : public Stone {
        CLONEOBJ(ShogunStone);

        enum Holes { SMALL = 1, MEDIUM = 2, LARGE = 4};
        static Holes smallest_hole(Holes s);
        void set_holes(Holes h) { set_attrib("holes", h); }

    public:
        ShogunStone(int holes=SMALL) : Stone("st-shogun") {
            set_holes(static_cast<Holes>(holes));
        }
    private:
        Holes get_holes() const;
        void notify_item();

        virtual Value message(const string &m, const Value &) {
            if (m == "init") { // request from ShogunDot (if set _after_ ShogunStone)
                notify_item();
            }
            return Value();
        }

        void add_hole(Holes h) {
            set_attrib("holes", get_holes() | h);
            notify_item();
            init_model();
        }

        void on_creation (GridPos p) {
            init_model();
            notify_item();
        }

        void on_impulse(const Impulse& impulse);

        void init_model() {
            set_model(ecl::strf("st-shogun%d", int(get_holes())));
        }

        bool is_movable() const { return false; }

        void actor_hit (const StoneContact &sc) {
            maybe_push_stone (sc);
        }
    };
}

ShogunStone::Holes ShogunStone::get_holes() const {
    int h=int_attrib("holes");
    if (h>=1 && h<=7)
        return Holes(h);
    else {
        warning("Wrong 'holes' attribute (%i)", h);
        return SMALL;
    }
}

ShogunStone::Holes ShogunStone::smallest_hole(Holes s) {
    if (s & SMALL) return SMALL;
    if (s & MEDIUM) return MEDIUM;
    if (s & LARGE) return LARGE;
    throw XLevelRuntime ("ShogunStone: internal error");
}

void ShogunStone::notify_item ()
{
    if (Item *it = GetItem(get_pos())) {
        switch (get_holes()) {
        case SMALL:                    SendMessage(it, "shogun1"); break;
        case (MEDIUM | SMALL):         SendMessage(it, "shogun2"); break;
        case (LARGE | MEDIUM | SMALL): SendMessage(it, "shogun3"); break;
        default:                       SendMessage(it, "noshogun"); break;
        }
    }
}

void ShogunStone::on_impulse(const Impulse& impulse) {
    GridPos destpos     = move(get_pos(), impulse.dir);
    Holes holes         = get_holes();
    Holes smallest      = smallest_hole(holes);
    ShogunStone *target = 0;

    if (Stone *st = GetStone(destpos)) {
        target = dynamic_cast<ShogunStone*>(st);

        /* If the stone at `p' is not a shogun stone or if smallest hole
           does not fit into target, do not transfer the smallest hole. */
        if (!target || smallest >= smallest_hole(target->get_holes()))
            return;
    }

    /* It's important to remove the old stone before setting the new
       one: otherwise it is possible to activate two triggers with one
       shogun stone when shifting from one shogun target to a second
       adjacent shogun target. */

    GridPos my_pos = get_pos();
    string  old_name;

    // Remove/modify source stone:
    if (Holes newholes = Holes(holes & ~smallest)) {
        set_holes(newholes);
        notify_item();
        init_model();
    }
    else {
        string_attrib("name", &old_name); // store name of disappearing stone
        SendMessage(GetItem(my_pos), "noshogun");
        KillStone(my_pos);
    }

    // Modify/create target stone:
    if (target) {
        target->add_hole(smallest);
//         target->sound_event("st-magic");
//         sound::PlaySound("st-magic", my_pos.center()); // object already disappeared
    }
    else {                       // create new
        target = new ShogunStone(smallest);
        SetStone(destpos, target);
        target->on_move();
    }

    if (!old_name.empty())
        NameObject(target, old_name);

    server::IncMoveCounter();
    sound::EmitSoundEvent ("movesmall", my_pos.center());
}



/* -------------------- Stone impulse stones -------------------- */

// Messages:
//
// :trigger
namespace
{
    class StoneImpulse_Base : public Stone {
    protected:
        StoneImpulse_Base(const char *kind) : Stone(kind), state(IDLE), incoming(NODIR)
        {}

        enum State { IDLE, PULSING, CLOSING };
        State     state;
        Direction incoming; // direction of incoming impulse (may be NODIR)

        void change_state(State st);

        virtual void on_impulse(const Impulse& impulse) {
            incoming = impulse.dir;
            change_state(PULSING);
        }

    private:

        virtual void notify_state(State st) = 0;

        virtual Value message(const string &m, const Value &value) {
            if (m=="trigger") {
                incoming = (value.get_type() == Value::DOUBLE)
                    ? Direction( static_cast<int> (value.get_double()+0.1))
                    : NODIR;

                change_state(PULSING);
            }
            else if (m == "signal" && to_double (value) != 0) {
                incoming = NODIR;
                change_state (PULSING);
            }
            return Value();
        }

        void animcb() {
            if (state == PULSING)
                change_state (CLOSING);
            else if (state == CLOSING)
                change_state (IDLE);
        }

        void on_laserhit(Direction dir) {
            incoming = dir;
            change_state(PULSING);
        }
    };

}

void StoneImpulse_Base::change_state(State new_state) {
    if (new_state == state) return;

    GridPos p = get_pos();
    switch (new_state) {
        case IDLE: {
            state = new_state;
            notify_state(state);
            break;
        }
        case PULSING:
            if (state != IDLE) {
                return;         // do not set new state
            }
            state = new_state;
            notify_state(state);
            sound_event("impulse");
            break;
        case CLOSING: {
            GridPos targetpos[4];
            bool    haveStone[4];

            // set CLOSING model _before_ sending impulses !!!
            // (any impulse might have side effects that move this stone)

            state = new_state;
            notify_state(state);

            for (int d = 0; d < 4; ++d) {
                targetpos[d] = move(p, Direction(d));
                haveStone[d] = GetStone(targetpos[d]) != 0;
            }

            for (int d = int(incoming)+1; d <= int(incoming)+4; ++d) {
                int D = d%4;
                if (haveStone[D]) {
                    send_impulse(targetpos[D], Direction(D));
                }
            }

            incoming = NODIR;   // forget impulse direction
            break;
        }
    }
}


namespace
{
    class StoneImpulseStone : public StoneImpulse_Base {
        CLONEOBJ(StoneImpulseStone);
    public:
        StoneImpulseStone() : StoneImpulse_Base("st-stoneimpulse")
        {}

    private:
        void notify_state(State st) {
            switch (st) {
            case IDLE:
                init_model();
                break;
            case PULSING:
                set_anim("st-stoneimpulse-anim1");
                break;
            case CLOSING:
                set_anim("st-stoneimpulse-anim2");
                break;
            }
        }

        void actor_hit(const StoneContact &/*sc*/) {
            change_state(PULSING);
        }

    };


    class HollowStoneImpulseStone : public StoneImpulse_Base {
        CLONEOBJ(HollowStoneImpulseStone);
    public:
        HollowStoneImpulseStone()
        : StoneImpulse_Base("st-stoneimpulse-hollow") {}
    private:
        void notify_state(State st) {
            switch (st) {
            case IDLE:
                init_model();
                lasers::MaybeRecalcLight(get_pos());
                break;
            case PULSING:
                lasers::MaybeRecalcLight(get_pos());
                set_anim("st-stoneimpulse-hollow-anim1");
                break;
            case CLOSING:
                set_anim("st-stoneimpulse-hollow-anim2");
                break;
            }
        }

        StoneResponse collision_response(const StoneContact &/*sc*/) {
            return (state == IDLE) ? STONE_PASS : STONE_REBOUND;
        }
        void actor_inside(Actor *a) {
            if (state == PULSING || state == CLOSING)
                SendMessage(a, "shatter");
        }

        bool is_floating () const {
            return true;
        }

        void on_laserhit (Direction) {
            // hollow StoneImpulseStones cannot be activated using lasers
        }
    };


    class MovableImpulseStone : public StoneImpulse_Base {
        CLONEOBJ(MovableImpulseStone);
    public:
        MovableImpulseStone()
        : StoneImpulse_Base("st-stoneimpulse_movable"), 
          repulse(false)
        {
        }

    private:

        void notify_state(State st) {
            switch (st) {
            case IDLE:
                if (repulse) {
                    repulse = false;
                    change_state(PULSING);
                }
                else
                    init_model();
                break;
            case PULSING:
                set_anim("st-stoneimpulse-anim1");
                break;
            case CLOSING:
                set_anim("st-stoneimpulse-anim2");
                break;
            }
        }

        void init_model() {
            set_model("st-stoneimpulse");
        }

        // Stone interface:

        void actor_hit(const StoneContact &sc) {
            if (!maybe_push_stone (sc)) {
                incoming = NODIR; // bad, but no real problem!
                if (state == IDLE)
                    change_state(PULSING);
            }
        }

        void on_impulse(const Impulse& impulse) {
            State oldstate = state;

            if (move_stone(impulse.dir)) {
                notify_state(oldstate); // restart anim if it was animated before move

                Actor *hitman = dynamic_cast<Actor*>(impulse.sender);
                if (hitman && player::WieldedItemIs (hitman, "it-magicwand")) {
                    return;     // do not change state to PULSING
                }
            }

            if (state == IDLE)
                change_state(PULSING);
        }

        void on_move() {
            if (state != PULSING)
                repulse = true; // pulse again
            Stone::on_move();
        }

        bool is_movable() const {
            // moving the stone is handled explicitly in actor_hit()
            return false; //true;
        }

        // Variables.
        bool repulse;
    };
}


/* -------------------- Oxyd stone -------------------- */

/** \page st-oxyd Oxyd Stone

Oxyd stones are characterized by two attributes: Their flavor and
their color.  The flavor only affects the visual representation of
the stone; it can be either 'a' (opening like a flower) or 'b'
(displaying a fade-in animation).  The color attribute determines
the shape on the oxyd stone.

\b Note: You should usually not to create Oxyd stones manually
with \c set_stone(). Use the predefined \c oxyd() function instead.

\subsection oxyda Attributes

- \b flavor      "a", "b", "c", or "d"
- \b color       number between 0 and 7

\subsection oxydm Messages

- \b closeall    close all oxyd stones
- \b shuffle     interchange the colors of the oxyd stones in the current landscape
- \b trigger     open the stone

<table><tr>
<td>\image html st-oxyda.png "flavor A"
<td>\image html st-oxydb.png "flavor B"
<td>\image html st-oxydc.png "flavor C"
<td>\image html st-oxydd.png "flavor D"
</table>
*/

namespace
{
    class OxydStone : public PhotoStone {
        INSTANCELISTOBJ(OxydStone);
    public:
        OxydStone();

        static void shuffle_colors();
        virtual bool is_removable() const;
    private:
        enum State { CLOSED, OPEN, OPENING, CLOSING, BLINKING };
        State state;

        // Stone interface
        void actor_hit(const StoneContact &sc);
        void on_creation (GridPos p);
        void on_removal (GridPos p);
        const char *collision_sound() { return "stone"; }
        virtual Value message(const string &m, const Value &);


        // PhotoStone interface
        void notify_laseron() { maybe_open_stone(); }
        void notify_laseroff() {}

        // Animation callback
        void animcb();

        // Private methods
        void maybe_open_stone();
        void change_state(State newstate);


        static bool blinking(OxydStone *a) {
            return (a->state==BLINKING);
        }
        static bool blinking_or_opening(OxydStone *a) {
            return (a->state==BLINKING || a->state == OPENING);
        }
        static bool not_open(OxydStone *a) {
            return !(a->state==OPEN || a->state==OPENING);
        }

    };
}

OxydStone::InstanceList OxydStone::instances;

OxydStone::OxydStone()
: PhotoStone("st-oxyd"),
  state(CLOSED)
{
    set_attrib("flavor", "b");
    set_attrib("color", "0");
}

Value OxydStone::message(const string &m, const Value &val) 
{
    if (m=="closeall") {
        for (unsigned i=0; i<instances.size(); ++i)
            instances[i]->change_state(CLOSING);
    }
    else if (m=="shuffle") {
        shuffle_colors();
    }
    else if (m=="trigger" || m=="spitter") {
	maybe_open_stone();
    }
    else if (m=="signal" && to_int(val) != 0) {
	maybe_open_stone();
    }
    else if (m=="init") {
        // odd number of oxyd stones in the level? no problem, turn a
        // random one into a fake oxyd

        if (instances.size() % 2) {
            // "odd number of oxyd stones";
            // TODO
        }
    }
    return Value();
}

void OxydStone::shuffle_colors() 
{
    vector<size_t> closed_oxyds;
    size_t         isize = instances.size();
    for (size_t i=0; i<isize; ++i) {
        if (instances[i]->state == CLOSED) {
            closed_oxyds.push_back(i);
        }
    }

    unsigned size = closed_oxyds.size();
    if (size>1) {
        for (unsigned i = 0; i<size; ++i) {
            unsigned a = IntegerRand(0, static_cast<int> (size-2));
            if (a >= i) ++a;        // make a always different from j

            OxydStone *o1 = instances[closed_oxyds[i]];
            OxydStone *o2 = instances[closed_oxyds[a]];

            string icolor, acolor;
            o1->string_attrib("color", &icolor);
            o2->string_attrib("color", &acolor);

            o1->set_attrib("color", acolor.c_str());
            o2->set_attrib("color", icolor.c_str());
        }
    }
}

void OxydStone::change_state(State newstate) 
{
    string flavor = "a";
    string color = "1";
    string_attrib("flavor", &flavor);
    string_attrib("color", &color);

    string modelname = string("st-oxyd") + flavor + color;

    State oldstate = state;
    state = newstate;

    switch (newstate) {
    case CLOSED:
        set_model(string("st-oxyd")+flavor);
        break;

    case BLINKING:
        set_model(modelname + "-blink");
        break;

    case OPEN:
	if (oldstate == CLOSED) {
            sound_event("oxydopen");
            sound_event("oxydopened");
            set_anim(modelname+"-opening");
        } else {
            set_model(modelname + "-open");
        }
        /* If this was the last closed oxyd stone, finish the
           level */
        if (find_if(instances.begin(),instances.end(),not_open)
            ==instances.end())
        {
            server::FinishLevel();
        }
        break;

    case OPENING:
        sound_event("oxydopen");
	if (oldstate == CLOSED)
            set_anim(modelname + "-opening");
	else if (oldstate == CLOSING)
            get_model()->reverse();

	break;

    case CLOSING:
        if (oldstate == CLOSED || oldstate==CLOSING) {
            state = oldstate;
            return;
        }

        sound_event("oxydclose");
        if (oldstate == OPENING)
            get_model()->reverse();
	else if (oldstate == BLINKING || oldstate == OPEN) {
            set_anim(modelname + "-closing");
        }
        break;
    }
}

void OxydStone::animcb() {
    if (state == CLOSING)
        change_state(CLOSED);
    else if (state == OPENING)
        change_state(BLINKING);
    else if (state == OPEN)
        change_state(OPEN); // set the right model
}

void OxydStone::maybe_open_stone() {
    if (state == CLOSED || state == CLOSING) {
        int mycolor = int_attrib("color");

        // Is another oxyd stone currently blinking?
        InstanceList::iterator i;
        i=find_if(instances.begin(), instances.end(), blinking_or_opening);

        if (i != instances.end()) {

            bool can_open;

            if (server::GameCompatibility != GAMET_ENIGMA) {
                // If colors match and stone (*i) is already blinking,
                // open both stones. Close one of them otherwise.
                // (This is the Oxyd behaviour; it doesn't work with
                // some Enigma levels.)
                can_open = (mycolor == (*i)->int_attrib("color") && (*i)->state==BLINKING);
            }
            else 
                can_open = (mycolor == (*i)->int_attrib("color"));

            if (can_open) {
                change_state(OPEN);
                (*i)->change_state(OPEN);
            } else {
                (*i)->change_state(CLOSING);
                change_state(OPENING);
            }
        }
        else {
            // no blinking stone? -> make this one blink
            change_state(OPENING);
        }
    }
}

void OxydStone::actor_hit(const StoneContact &/*sc*/) {
    maybe_open_stone();
}

void OxydStone::on_creation (GridPos) 
{
    string flavor = "a";
    string_attrib("flavor", &flavor);
    set_model(string("st-oxyd") + flavor);
    photo_activate();
}

bool OxydStone::is_removable() const {
    const Value* isStatic = get_attrib("static");
    if (isStatic != NULL)
        return !to_bool(*isStatic);
    else
        return true;
}

void OxydStone::on_removal(GridPos p) 
{
    photo_deactivate();
    kill_model (p);
}


/* -------------------- Turnstiles -------------------- */
namespace
{
    class Turnstile_Arm;

    /*
    ** The stone at the center of a turnstile
    */
    class Turnstile_Pivot_Base : public Stone {
    public:
        Turnstile_Pivot_Base(const char *kind);

    protected:
        bool rotate(bool clockwise, Object *impulse_sender);

        friend class Turnstile_Arm; // uses rotate

    private:
        // Object interface
        virtual Value on_message (const Message &m);
        virtual void animcb();

        // Private methods
        DirectionBits arms_present() const;
        bool          no_stone (int xoff, int yoff) const;
        void set_arm (Direction dir, RBI_vector &rubs);
        void remove_arms (DirectionBits arms);
	void rotate_arms (DirectionBits arms, bool clockwise);
        void handleActorsAndItems(bool clockwise, Object *impulse_sender);

        // Turnstile_Pivot_Base interface
        virtual const char *model() const    = 0;
        virtual const char *anim() const     = 0;
        virtual bool oxyd_compatible() const = 0;

        // Variables
        bool active;
    };

    class Turnstile_Pivot : public Turnstile_Pivot_Base {
        CLONEOBJ(Turnstile_Pivot);
    public:
        Turnstile_Pivot() : Turnstile_Pivot_Base(model()) {}

        const char *model() const { return "st-turnstile"; }
        const char *anim() const  { return "st-turnstile-anim"; }
        bool oxyd_compatible() const { return true; }
    };

    class Turnstile_Pivot_Green : public Turnstile_Pivot_Base {
        CLONEOBJ(Turnstile_Pivot_Green);
    public:
        Turnstile_Pivot_Green() : Turnstile_Pivot_Base(model()) {}

        const char *model() const { return "st-turnstile-green"; }
        const char *anim() const  { return "st-turnstile-green-anim"; }
        bool oxyd_compatible() const { return false; }
    };

    /*
    ** The base class for any of the four arms of the turnstile
    */
    class Turnstile_Arm : public Stone {
        virtual Direction get_dir() const = 0;

        void actor_hit(const StoneContact &sc);
        void on_impulse(const Impulse& impulse);

        Turnstile_Pivot_Base *get_pivot() {
            Stone *st = GetStone (move (get_pos(), reverse(get_dir())));
            return dynamic_cast<Turnstile_Pivot_Base*>(st);
        }

        bool is_movable () const { return true; }
    protected:
        Turnstile_Arm (const char *kind) : Stone(kind)
        {}
    };

    class Turnstile_N : public Turnstile_Arm {
        CLONEOBJ(Turnstile_N);
    public:
        Turnstile_N(): Turnstile_Arm("st-turnstile-n") {}
        Direction get_dir () const { return NORTH; }
    };

    class Turnstile_S : public Turnstile_Arm {
        CLONEOBJ(Turnstile_S);
        Direction get_dir () const { return SOUTH; }
    public:
        Turnstile_S(): Turnstile_Arm("st-turnstile-s") {}
    };

    class Turnstile_E : public Turnstile_Arm {
        CLONEOBJ(Turnstile_E);
        Direction get_dir () const { return EAST; }
    public:
        Turnstile_E(): Turnstile_Arm("st-turnstile-e") {}
    };

    class Turnstile_W : public Turnstile_Arm {
        CLONEOBJ(Turnstile_W);
        Direction get_dir () const { return WEST; }
    public:
        Turnstile_W(): Turnstile_Arm("st-turnstile-w") {}
    };
}


/* -------------------- Turnstile_Arm -------------------- */

void Turnstile_Arm::on_impulse(const Impulse& impulse) {
    enum Action { ROTL, ROTR, stay };
    static Action actions[4][4] = {
        { stay, ROTL, stay, ROTR }, // west arm
        { ROTR, stay, ROTL, stay }, // south arm
        { stay, ROTR, stay, ROTL }, // east arm
        { ROTL, stay, ROTR, stay }  // north arm
    };

    Turnstile_Pivot_Base *pivot = get_pivot();

    if (pivot) {
        Action a = actions[get_dir()][impulse.dir];
        if (a != stay) {
            pivot->rotate(a == ROTR, impulse.sender); // ROTR is clockwise
        }
    }
    else {
        // Move arms not attached to a pivot individually
        move_stone(impulse.dir);
    }
}

void Turnstile_Arm::actor_hit(const StoneContact &sc) 
{
    maybe_push_stone(sc);
}

// --------------------------------------------
//      Turnstile_Pivot_Base implementation
// --------------------------------------------

Turnstile_Pivot_Base::Turnstile_Pivot_Base(const char *kind) 
: Stone (kind),
  active (false)
{}

void Turnstile_Pivot_Base::animcb() 
{ 
    set_model(model()); 
    active = false;
}

Value Turnstile_Pivot_Base::on_message (const Message &m)
{
    if (m.message == "signal") {
        int val = to_int (m.value);
        if (val == 1)
            rotate(false, 0);
        else
            rotate(true, 0);
    }
    return Value();
}


DirectionBits
Turnstile_Pivot_Base::arms_present() const
{
    DirectionBits arms = NODIRBIT;
    GridPos p = get_pos();
    if (dynamic_cast<Turnstile_N*>(GetStone(move(p, NORTH))))
        ecl::set_flags (arms, NORTHBIT);
    if (dynamic_cast<Turnstile_S*>(GetStone(move(p, SOUTH))))
        ecl::set_flags (arms, SOUTHBIT);
    if (dynamic_cast<Turnstile_E*>(GetStone(move(p, EAST))))
        ecl::set_flags (arms, EASTBIT);
    if (dynamic_cast<Turnstile_W*>(GetStone(move(p, WEST))))
        ecl::set_flags (arms, WESTBIT);
    return arms;
}

bool Turnstile_Pivot_Base::no_stone (int xoff, int yoff) const {
    GridPos p = get_pos();
    p.x += xoff;
    p.y += yoff;
    return (0 == GetStone(p));
}

void Turnstile_Pivot_Base::remove_arms (DirectionBits arms) {
    GridPos p = get_pos();
    if (arms & NORTHBIT) KillStone (move (p, NORTH));
    if (arms & EASTBIT) KillStone (move (p, EAST));
    if (arms & SOUTHBIT) KillStone (move (p, SOUTH));
    if (arms & WESTBIT) KillStone (move (p, WEST));
}

void Turnstile_Pivot_Base::rotate_arms (DirectionBits arms, bool clockwise) {
    GridPos p = get_pos();

    RBI_vector Nrubs;
    RBI_vector Erubs;
    RBI_vector Srubs;
    RBI_vector Wrubs;

    if (arms & NORTHBIT) GiveRubberBands(GetStone(move (p, NORTH)), Nrubs);
    if (arms & EASTBIT) GiveRubberBands(GetStone(move (p, EAST)), Erubs);
    if (arms & SOUTHBIT) GiveRubberBands(GetStone(move (p, SOUTH)), Srubs);
    if (arms & WESTBIT) GiveRubberBands(GetStone(move (p, WEST)), Wrubs);

    remove_arms(arms);

    if (clockwise) {
	if (arms & NORTHBIT) set_arm(EAST, Nrubs);
	if (arms & EASTBIT)  set_arm(SOUTH, Erubs);
	if (arms & SOUTHBIT) set_arm(WEST, Srubs);
	if (arms & WESTBIT)  set_arm(NORTH, Wrubs);
    }
    else {
	if (arms & NORTHBIT) set_arm(WEST, Nrubs);
	if (arms & EASTBIT)  set_arm(NORTH, Erubs);
	if (arms & SOUTHBIT) set_arm(EAST, Srubs);
	if (arms & WESTBIT)  set_arm(SOUTH, Wrubs);
    }
}

void Turnstile_Pivot_Base::set_arm (Direction dir, RBI_vector &rubs) {
    const char *names[4] = { "st-turnstile-w", "st-turnstile-s",
                             "st-turnstile-e", "st-turnstile-n" };
    Stone   *st   = MakeStone(names[dir]);
    GridPos  newp = move(get_pos(), dir);
    SetStone (newp, st);

    if (Item *it = GetItem(newp))
        it->on_stonehit(st);

    if (!rubs.empty())
	for (RBI_vector::iterator i = rubs.begin(); i != rubs.end(); ++i)
	    AddRubberBand (i->act, st, i->data);
}

bool Turnstile_Pivot_Base::rotate(bool clockwise, Object *impulse_sender) {
    if (active)
        return false;

    DirectionBits arms       = arms_present();
    bool          can_rotate = true;

    if (clockwise)  {
        if (arms & NORTHBIT) {
            can_rotate &= no_stone(+1,-1);
            if (! (arms & EASTBIT)) can_rotate &= no_stone(+1,0);
        }
        if (arms & WESTBIT) {
            can_rotate &= no_stone(-1,-1);
            if (! (arms & NORTHBIT)) can_rotate &= no_stone(0,-1);
        }
        if (arms & SOUTHBIT) {
            can_rotate &= no_stone(-1,+1);
            if (! (arms & WESTBIT)) can_rotate &= no_stone(-1,0);
        }
        if (arms & EASTBIT) {
            can_rotate &= no_stone(+1,+1);
            if (! (arms & SOUTHBIT)) can_rotate &= no_stone(0,+1);
        }
    }
    else {
        if (arms & NORTHBIT) {
            can_rotate &= no_stone(-1,-1);
            if (! (arms & WESTBIT)) can_rotate &= no_stone(-1,0);
        }
        if (arms & WESTBIT) {
            can_rotate &= no_stone(-1,+1);
            if (! (arms & SOUTHBIT)) can_rotate &= no_stone(0,+1);
        }
        if (arms & SOUTHBIT) {
            can_rotate &= no_stone(+1,+1);
            if (! (arms & EASTBIT)) can_rotate &= no_stone(+1,0);
        }
        if (arms & EASTBIT) {
            can_rotate &= no_stone(+1,-1);
            if (! (arms & NORTHBIT)) can_rotate &= no_stone(0,-1);
        }
    }

    if (can_rotate) {
        sound_event (clockwise ? "turnstileright" : "turnstileleft");
        sound_event("movesmall");

        active = true;
        set_anim(anim());
	    rotate_arms(arms, clockwise);
        handleActorsAndItems(clockwise, impulse_sender);

        PerformAction (this, clockwise == 0);
        server::IncMoveCounter();
    }
    return can_rotate;
}

namespace {
    bool calc_arm_seen (bool cw, DirectionBits arms, int field) {
        // for each field calculate whether an arm has passed by, first
        // counterclockwise and then clockwise:
        const DirectionBits neededArm[2][8] = {
            {WESTBIT, NORTHBIT, NORTHBIT, EASTBIT, EASTBIT, SOUTHBIT, SOUTHBIT, WESTBIT},
            {NORTHBIT, NORTHBIT, EASTBIT, EASTBIT, SOUTHBIT, SOUTHBIT, WESTBIT, WESTBIT}
        };
        return (arms & neededArm[cw][field]) != 0;
    }
}

void Turnstile_Pivot_Base::handleActorsAndItems(bool clockwise, Object *impulse_sender) {
    const int to_index[3][3] = { // (read this transposed)
        { 0, 7, 6 }, // x == 0
        { 1,-1, 5 }, // x == 1
        { 2, 3, 4 }  // x == 2
    };
    const int to_x[8] = { -1, 0, 1, 1, 1, 0, -1, -1 };
    const int to_y[8] = { -1, -1, -1, 0, 1, 1, 1, 0 };

    bool arm_seen[8];
    DirectionBits arms = arms_present(); // Note: already the rotated state
    for (int i = 0; i<8; ++i)
        arm_seen[i] = calc_arm_seen (clockwise, arms, i);

    // ---------- Handle items in range ----------
    GridPos pv_pos = get_pos();
    for (int i = 0; i<8; ++i) 
        if (arm_seen[i]) {
            GridPos item_pos(pv_pos.x+to_x[i], pv_pos.y+to_y[i]);
            if (Item *it = GetItem(item_pos)) 
                it->on_stonehit(this); // hit with pivot (shouldn't matter)
        }

    // ---------- Handle actors in range ----------
    vector<Actor*> actorsInRange;

    // tested range is sqrt(sqr(1.5)+sqr(1.5)) 
    if (!GetActorsInRange(pv_pos.center(), 2.124, actorsInRange))
        return;

    vector<Actor*>::iterator iter = actorsInRange.begin(), end = actorsInRange.end();
    for (; iter != end; ++iter) {
        Actor *ac = *iter;
        const V2 &ac_center = ac->get_pos();
        GridPos   ac_pos(ac_center);
        int dx  = ac_pos.x-pv_pos.x;
        int dy  = ac_pos.y-pv_pos.y;

        // ignore if actor is not inside the turnstile square or is not
        // in distance of the the rotating arms
        if ((dx<-1 || dx>1 || dy<-1 || dy>1) || 
                (length(ac->get_pos() - pv_pos.center()) > 1.58114 + ac->get_actorinfo()->radius))
            continue;

        int idx_source = to_index[dx+1][dy+1];
        if (idx_source == -1) 
            continue;       // actor inside pivot -- should not happen

        const int rot_index[4][8] = {
            // The warp-destinations for actors. Why different destinations
            // for oxyd/non-oxyd-type turnstiles? Imagine the actor on position
            // 1 (North of pivot), the turnstile rotates anticlockwise. Then
            // a green turnstile-arm, if at all, would push the actor one field
            // to the left (position 0). Now assume it's a red turnstile. If the
            // actor is to be warped, it has to be the one that activated the
            // turnstile. Yet it is on position 1, in principle not able to
            // hit an arm. But it can, if it hits fast enough on the edge of
            // pivot and left arm. In this case, the actor should be handled
            // as if on position 1, thus warping to 6.
            { 6,  0, 0,  2, 2,  4, 4,  6 }, // anticlockwise
            { 2,  2, 4,  4, 6,  6, 0,  0 }, // clockwise
            { 6,  6, 0,  0, 2,  2, 4,  4 }, // anticlockwise (oxyd-compatible)
            { 2,  4, 4,  6, 6,  0, 0,  2 }, // clockwise (oxyd-compatible)
        };

        bool compatible = oxyd_compatible();
        int  idx_target = rot_index[clockwise+2*compatible][idx_source]; // destination index
        bool do_warp = false; // move the actor along with the turnstile?

        if (compatible) {
            // Move only the actor that hit the turnstile in Oxyd mode
            do_warp = (ac == dynamic_cast<Actor*>(impulse_sender));
            if (!do_warp && arm_seen[idx_source])
                SendMessage(ac, "shatter"); // hit by an arm
        } else { // green turnstile
            // move all actors only if pushed by an arm
            do_warp = arm_seen[idx_source];
        }

        if (!do_warp) 
            continue;

        // Pushing an actor out of the level results in a shatter (no warp) instead
        GridPos ac_target_pos(pv_pos.x+to_x[idx_target], pv_pos.y+to_y[idx_target]);

        if(!IsInsideLevel(ac_target_pos)) {
            SendMessage(ac, "shatter");
            continue;
        }

        world::WarpActor(ac, ac_target_pos.x+.5, ac_target_pos.y+.5, false);

        if (Stone *st = GetStone(ac_target_pos)) {

            // destination is blocked

            Turnstile_Arm *arm = dynamic_cast<Turnstile_Arm*>(st);
            if (arm && !compatible) { // if blocking stone is turnstile arm -> hit it!
                const int impulse_dir[2][8] = {
                    // anticlockwise
                    { SOUTHBIT|WESTBIT, WESTBIT, NORTHBIT|WESTBIT, NORTHBIT,
                      NORTHBIT|EASTBIT, EASTBIT, SOUTHBIT|EASTBIT, SOUTHBIT },
                    // clockwise
                    { NORTHBIT|EASTBIT, EASTBIT, SOUTHBIT|EASTBIT, SOUTHBIT,
                      SOUTHBIT|WESTBIT, WESTBIT, NORTHBIT|WESTBIT, NORTHBIT }
                };

                DirectionBits possible_impulses =
                    static_cast<DirectionBits>(impulse_dir[clockwise][idx_target]);

                for (int d = 0; d<4; ++d) 
                    if (has_dir(possible_impulses, Direction(d))) 
                        ac->send_impulse(ac_target_pos, Direction(d));

//                 if (GetStone(ac_target_pos) == 0)  // arm disappeared
//                     break;
            }
        }
    }

// @@@ FIXME: it's possible that two actors are moved to the same
// destination field.  In that case the second actor is put on top of
// the first actor (happens only in non-oxyd-compat-mode with three
// balls or pullers/impulsestones)
//
// Note: With black and whiteball it's normally no problem, because
// when one of the actors was moving, it's looking natural.  Problems
// occur when small balls come into play.

}


/* -------------------- Mail stone -------------------- */

namespace
{
    class MailStone : public Stone {
        CLONEOBJ(MailStone);
        Direction m_dir;


        MailStone (const char *kind, Direction dir);
        void actor_hit (const StoneContact &sc);

        GridPos find_pipe_endpoint();
    public:
        static void setup();
    };
}

void MailStone::setup()
{
    Register (new MailStone ("st-mail-n", NORTH));
    Register (new MailStone ("st-mail-e", EAST));
    Register (new MailStone ("st-mail-s", SOUTH));
    Register (new MailStone ("st-mail-w", WEST));
}

MailStone::MailStone (const char *kind, Direction dir)
: Stone(kind), m_dir(dir)
{}


void MailStone::actor_hit (const StoneContact &sc) 
{
    if (enigma::Inventory *inv = player::GetInventory(sc.actor)) {
        if (Item *it = inv->get_item(0)) {
            GridPos p = find_pipe_endpoint();
            if (world::IsInsideLevel(p) && it->can_drop_at (p)) {
                it = inv->yield_first();
                player::RedrawInventory (inv);
                it->drop(sc.actor, p);
            }
        }
    }
}

/** About recursion detection while finding the end of a mailpipe
 *  
 *  Since there are no possibilities for forking a mailpipe, there is only one 
 *  cause that may lead to a closed, circular mailpipe. This is if a pipeitem is
 *  placed exactly under the mailstone. But not every pipepiece is dangerous,
 *  there are some that can be placed under the mailstone without problems.
 * 
 *  The mailpipe is only closed to a circular one if the pipepiece under the
 *  mailstone has the same 'output'-direction as the stone.
 */
GridPos MailStone::find_pipe_endpoint() 
{
    GridPos p = get_pos();
    Direction move_dir = m_dir;
    GridPos q = p; // Store the stonepos for recursion detection.

    while (move_dir != NODIR) {
        p.move (move_dir);
        if (Item *it = world::GetItem(p)) {
            switch (get_id(it)) {
            case it_pipe_h:
                if (!(move_dir == EAST || move_dir == WEST))
                    move_dir = NODIR;
                break;
            case it_pipe_v:
                if (!(move_dir == SOUTH || move_dir == NORTH))
                    move_dir = NODIR;
                break;
            case it_pipe_ne:
                if (move_dir == SOUTH)     move_dir = EAST;
                else if (move_dir == WEST) move_dir = NORTH;
                else                       move_dir = NODIR;
                break;
            case it_pipe_es:
                if (move_dir == NORTH)     move_dir = EAST;
                else if (move_dir == WEST) move_dir = SOUTH;
                else                       move_dir = NODIR;
                break;
            case it_pipe_sw:
                if (move_dir == NORTH)     move_dir = WEST;
                else if (move_dir == EAST) move_dir = SOUTH;
                else                       move_dir = NODIR;
                break;
            case it_pipe_wn:
                if (move_dir == SOUTH)     move_dir = WEST;
                else if (move_dir == EAST) move_dir = NORTH;
                else                       move_dir = NODIR;
                break;
            default:
                move_dir = NODIR;; // end of pipe reached
            }
        } else
            move_dir = NODIR;

        if (p == q)
            ASSERT(move_dir != m_dir, XLevelRuntime, "Mailpipe is circular! Recursion detected!");   
    }
    return p;
}


/* -------------------- Chess stone -------------------- */

namespace
{
    class ChessStone : public Stone, public TimeHandler {
        CLONEOBJ(ChessStone);
    public:
        ChessStone (int color) : Stone("st-chess") {
            newcolor = color;
            Stone::set_attrib("color", color);
            destination = GridPos(0,0);
            capture_retry = 0;
            rememberFalling = false;
            rememberSwamp = false;
            state = IDLE;
        }
        virtual ~ChessStone() {
            GameTimer.remove_alarm (this);
        }
        void init_model();
        void animcb();
        void set_attrib(const string& key, const Value &val);
        virtual Value message(const string &msg, const Value &v);
        void actor_hit(const StoneContact &sc);
        void alarm();
    private:
        // Variables and Constants
        enum State {IDLE, APPEARING, DISAPPEARING, CAPTURING,
                    CAPTURED, FALLING, SWAMP} state;
        GridPos destination;
        int newcolor;  // Buffers a color-changing message while not IDLE.
        int capture_retry;
        static const int max_capture_retry = 20;
        static double capture_interval;
        static double hit_threshold;
        bool rememberFalling;
        bool rememberSwamp;

        // Methods
        string get_model_name();
        Value maybe_move_to(Direction dir1, Direction dir2);
        virtual Value message_move(Direction dir1, Direction dir2);
        bool try_state(State newstate);
        void set_color(int color);
        void on_floor_change();

        // Interface for st-swap and st-pull
        bool is_removable() const { return state == IDLE; }
    };
    double ChessStone::capture_interval = 0.1;
    double ChessStone::hit_threshold = 3.0;

    string ChessStone::get_model_name() {
        string mname = get_kind();
        mname += int_attrib("color") == 0.0 ? "_black" : "_white";
        return mname;
    }

    void ChessStone::init_model() { set_model(get_model_name()); }

    void ChessStone::animcb() {
        Stone *st;
        switch (state) {
        case APPEARING:
            if(try_state(IDLE))
            // Maybe falling in the meantime? Otherwise:
                init_model();
            break;
        case DISAPPEARING:
            // Maybe the floor has changed into swamp or abyss?
            if(!rememberFalling && !rememberSwamp) {
                if(try_state(APPEARING)) {
                    st = GetStone(destination);
                    if(st) {
                        // Something went wrong while killing the old
                        // stone, or maybe a third one intervened.
                        // Don't move, just reappear at old position.                
                    } else {
                        move_stone(destination, "movesmall");
                        SendMessage(GetFloor(destination), "capture");
                    }
                    // maybe a floor-change has happened, but during
                    // state APPEARING this doesn't mean anything:
                    set_anim(get_model_name()+"-appearing");
                }
                break;
            }
            // Else: If floor is swamp or abyss, kill the stone. Do this
            // just by continuing to the next case:
        case CAPTURED:
        case FALLING:
        case SWAMP:
            KillStone(get_pos());
            break;
        default:
           ASSERT(0, XLevelRuntime, "ChessStone: inconsistent state in animcb()");
        }
    }

    void ChessStone::actor_hit(const StoneContact &sc) {
         if (player::WieldedItemIs (sc.actor, "it-magicwand")) {
             sound_event ("stonepaint");
             set_color(1 - int_attrib("color"));
             // If not IDLE, color will be set next time IDLE is set.
         } else if ((sc.actor->get_attrib("blackball") && int_attrib("color") == 0)
                    || (sc.actor->get_attrib("whiteball") && int_attrib("color") == 1)) {
            V2 v = sc.actor->get_vel();
            Direction dir1 = get_push_direction(sc);
            if(dir1 == NODIR)  return;
            Direction dir2 = NODIR;
            if (dir1 == SOUTH || dir1 == NORTH) {
                dir2 = v[0] >  hit_threshold ? EAST :
                       v[0] < -hit_threshold ? WEST : NODIR;
            } else {
                dir2 = v[1] >  hit_threshold ? SOUTH :
                       v[1] < -hit_threshold ? NORTH : NODIR;
            }
            if(dir2 == NODIR)  return;
            // maybe_move_to tests for state == IDLE by itself.
            maybe_move_to(dir1, dir2);
        }
    }

    Value ChessStone::maybe_move_to(Direction dir1, Direction dir2) {
        if(state == IDLE) {
            // check for fire, step by step
            destination = move(get_pos(), dir1);
            if(Item *it = GetItem(destination))
                if(get_id(it) == it_burnable_burning)
                    return Value();
            destination = move(destination, dir2);
            if(Item *it = GetItem(destination))
                if(get_id(it) == it_burnable_burning)
                    return Value();
            destination = move(destination, dir1);
            if(Item *it = GetItem(destination))
                if(get_id(it) == it_burnable_burning)
                    return Value();
            // check for boundary
            if(!IsInsideLevel(destination))  return Value();
            // check for stone
            if(!GetStone(destination)) {
                // Simple case: Just move.
                if(try_state(DISAPPEARING)) {
                    set_anim(get_model_name() + "-disappearing");
                    return Value(1);
                } else
                    return Value();
            } else {
                // Test stone. Is it opposite chess stone or totally another one?
                Stone *st = GetStone(destination);
                const Value *col = get_attrib("color");
                if(to_int(SendMessage(st, "capture", Value(get_model_name()))) ) {
                    // Give it some time for animation, then replace it.
                    ASSERT(try_state(CAPTURING), XLevelRuntime,
                        "ChessStone: strange things happening in maybe_move_to");
                    // must work, because state is IDLE
                    GameTimer.set_alarm(this, capture_interval, false);
                    capture_retry = 0;
                    return Value(1);
                }
                return Value();
            }
        }
        return Value();
    }

    void ChessStone::alarm() {
        switch(state) {
        case CAPTURING:
            if(!GetStone(destination)) {
                if(try_state(DISAPPEARING))
                    set_anim(get_model_name() + "-disappearing");
                break;
            } else if(capture_retry < max_capture_retry) {
                ++capture_retry;
                GameTimer.set_alarm(this, capture_interval, false);
            } else {
                // Cancel efforts to capture foreign stone.
                capture_retry = 0;
                try_state(IDLE);
            }
            break;
        default:
           ASSERT(0, XLevelRuntime, "ChessStone: inconsistent state in alarm()");
        }
    }

    Value ChessStone::message(const string &msg, const Value &v) {
        if(msg == "capture") {
            if(state == IDLE && to_string(v) != get_model_name())
                if(try_state(CAPTURED)) {
                    set_anim(get_model_name() + "-captured");
                    return Value(1);
                }
            return Value();
        } else if(msg == "move_nne") { return message_move(NORTH, EAST); }
        else   if(msg == "move_een") { return message_move(EAST, NORTH); }
        else   if(msg == "move_ees") { return message_move(EAST, SOUTH); }
        else   if(msg == "move_sse") { return message_move(SOUTH, EAST); }
        else   if(msg == "move_ssw") { return message_move(SOUTH, WEST); }
        else   if(msg == "move_wws") { return message_move(WEST, SOUTH); }
        else   if(msg == "move_wwn") { return message_move(WEST, NORTH); }
        else   if(msg == "move_nnw") { return message_move(NORTH, WEST); }
        else   if(msg == "move") {
            Direction dir1 = (Direction) int_attrib("direction1");
            Direction dir2 = (Direction) int_attrib("direction2");
            return message_move(dir1, dir2);
        } else if(msg == "signal") { set_color(to_int(v)); }
        else   if(msg == "flip") { set_color(1 - int_attrib("color")); }
        else
            return Stone::message(msg, v);
        return Value();
    }

    Value ChessStone::message_move(Direction dir1, Direction dir2) {
        // Restrict message-moves to chess-knight-moves
        // state == IDLE is tested by maybe_move_to.
        if((((dir1==NORTH || dir1==SOUTH) && (dir2==EAST || dir2==WEST))
           || ((dir1==EAST || dir1==WEST) && (dir2==SOUTH || dir2==NORTH))))
            return maybe_move_to(dir1, dir2);
        return Value();
    }

    bool ChessStone::try_state(State newstate) {
        if (state != FALLING && state != SWAMP) {
            // Switch to FALLING or SWAMP only when IDLE,
            // but remember them!
            if (newstate == FALLING)
                rememberFalling = true;
            else if (newstate == SWAMP)
                rememberSwamp = true;
            else
                state = newstate;        
            if(state == IDLE && newcolor != int_attrib("color"))
                set_color(newcolor);
            if(state == IDLE && rememberFalling) {
                state = FALLING;
                set_anim(get_model_name() + "-disappearing");
            }
            if(state == IDLE && rememberSwamp) {
                state = SWAMP;
                set_anim(get_model_name() + "-swamp");
            }
        }
        return state == newstate;
    }

    void ChessStone::set_attrib(const string& key, const Value &val) {
        if(key == "color") {
            set_color(to_int(val));
        } else
            Stone::set_attrib(key, val);
    }

    void ChessStone::set_color(int color) {
        if(color != 0 && color != 1) {
            ASSERT(0, XLevelRuntime, "ChessStone: argument to color not 0 or 1");
        }
        if(state == IDLE) {
            Stone::set_attrib("color", color);
            newcolor = color;
            init_model();
        } else {
            // Remember this color and set it the next time IDLE is set.
            newcolor = color;
        }
    }

    void ChessStone::on_floor_change() {
        Floor *fl = GetFloor(get_pos());
        if (fl != NULL) {
            if (fl->is_kind("fl-abyss"))
                try_state(FALLING);
            if (fl->is_kind("fl-swamp"))
                try_state(SWAMP);
        }
    }
}


/* -------------------- Light Passenger Stone -------------------- */
namespace
{
    class LightPassengerStone : public PhotoStone, public TimeHandler {
        CLONEOBJ(LightPassengerStone);
        DECL_TRAITS;
    public:
        LightPassengerStone(bool active_) : PhotoStone("st-lightpassenger"),
            skateDir (NODIR), state(active_ ? ACTIVE : INACTIVE),
            isLighted (false) { }

    virtual ~LightPassengerStone() {
        GameTimer.remove_alarm (this);
    }

    private:
        Direction skateDir;
        //bool isActive;
        bool isLighted;
        enum State {INACTIVE, ACTIVE, BLINK, BREAK} state;
        //bool isBlinking;

        void on_creation(GridPos p) {
            PhotoStone::on_creation(p);
            photo_activate();
        }
        void on_removal(GridPos p) {
            photo_deactivate();
            PhotoStone::on_removal(p);
        }
        void init_model() {
            switch(state) {
            case INACTIVE:
                set_anim("st-lightpassenger_off"); break;
            case ACTIVE:
                set_anim("st-lightpassenger"); break;
            case BLINK:
                set_anim("st-lightpassenger-blink"); break;
            case BREAK:
                GridPos p = get_pos();
                bool NorthSouth = lasers::LightFrom(p,NORTH) &&
		    lasers::LightFrom(p,SOUTH);
                bool EastWest = lasers::LightFrom(p,EAST) &&
		    lasers::LightFrom(p,WEST);
                sound_event ("stonedestroy");
                if(NorthSouth && !EastWest)
                    set_anim("st-lightpassenger-break-v");
                else if(!NorthSouth && EastWest)
                    set_anim("st-lightpassenger-break-h");
                else // and this even in case the laser recently disappeared
                    set_anim("st-lightpassenger-break-hv");
                break;
            }
        }
        void animcb() {
            ASSERT(state == BREAK, XLevelRuntime,
                "LightPassengerStone: animcb called with inconsistent state");
            KillStone(get_pos());
        }
        void notify_laseroff() {
            if (isLighted) GameTimer.remove_alarm(this);
            isLighted = false;
            if(state == BLINK) {                
                state = ACTIVE;
                init_model();
            }
        }
        void notify_laseron() {
            if (!isLighted) GameTimer.set_alarm(this, get_interval(), false);
            isLighted = true;
        }        

        double get_interval() {
            /*  Interval is calculated from
                        1 + friction_factor * friction
            interval = -------------------------------- * baseinterval
                        1 + gradient_factor * gradient
                and min-maxed to sensible values. "gradient" is just
                the force resulting from floor->add_force. "baseinterval"
                is 50 ms or the interval given in "interval".
            */
            double base = 0.05;
            if (const Value *v = this->get_attrib("interval")) 
                base = to_double(*v);
            if (const Value *f = this->get_attrib("friction_factor"))
                base *= 1.0 + to_double(*f) * GetFloor(get_pos())->friction();
            if (const Value *g = this->get_attrib("gradient_factor"))
                if (skateDir != NODIR) {
                    V2 vec = V2(0.0,0.0);
                    double quot = 0;
                    GetFloor(get_pos())->add_force(0, vec);
                    quot = skateDir == NORTH ? -vec[1] : skateDir == SOUTH ? vec[1] :
                        skateDir == EAST ? vec[0] : skateDir == WEST ? -vec[0] : 0;
                    base /= max(1.0 + to_double(*g) * quot, 0.01);                    
                }
            return max(base, 0.02);
        }

        void set_on(bool newon){
            if(state == INACTIVE && newon) {
                state = ACTIVE;
                init_model();
                if(isLighted)
                    GameTimer.set_alarm(this, get_interval(), false);
            } else if((state == ACTIVE || state == BLINK) && !newon) {
                state = INACTIVE;
                init_model();
                skateDir = NODIR;
            }
        }

        virtual Value message (const string &msg, const Value &v) {
            if      (msg == "onoff")   set_on(state == INACTIVE);
            else if (msg == "signal")  set_on(to_int(v) != 0);
            else if (msg == "on")      set_on(true);
            else if (msg == "off")     set_on(false);
            else if (msg == "trigger") set_on(state == INACTIVE);
            return Value();
        }

        void alarm() {
            if(isLighted && (state == ACTIVE || state == BLINK)) {
                GridPos p = get_pos();
                int toSouth =    (lasers::LightFrom(p,NORTH)?1:0)
                               + (lasers::LightFrom(p,SOUTH)?-1:0);
                int toWest =   (lasers::LightFrom(p,EAST)?1:0)
                             + (lasers::LightFrom(p,WEST)?-1:0);
                if(toSouth * toWest != 0) {
                    // Light is coming from two directions. Choose the one you are
                    // *not* coming from (thus changing beams), in doubt: random.
                    if(skateDir == NORTH || skateDir == SOUTH)
                        toSouth = 0;
                    if(skateDir == EAST || skateDir == WEST)
                        toWest = 0;
                    if(skateDir == NODIR) {
                        toSouth = IntegerRand(0,1) ? 0 : toSouth;
                        toWest = toSouth ? 0 : toWest;
                    }
                }
                skateDir = (toSouth == 1) ? SOUTH : (toSouth == -1) ? NORTH :
                    (toWest == 1) ? WEST : (toWest == -1) ? EAST : NODIR;
                if(skateDir == NODIR && state != BLINK) {
                    // No direction but lighted? Seems to be lasers from
                    // two opposite directions. Be sure and then start blinking.
                    if(lasers::LightFrom(p,EAST) || lasers::LightFrom(p,WEST) ||
                       lasers::LightFrom(p,NORTH) || lasers::LightFrom(p,SOUTH)) {
                        state = BLINK;
                        init_model();
                    }
                } else if(skateDir != NODIR) {
                    if(state == BLINK) {
                        state == ACTIVE;
                        init_model();
                    }
                    if(GetStone(move(p, skateDir))) {
                        // Skipping each second turn makes the passenger stone seem
                        // slower when pushing another stone. This looks more
                        // natural. That's why impulse is delayed:
                        send_impulse(move(p, skateDir), skateDir, get_interval());
                    }
                    move_stone(skateDir);
                }
                GameTimer.set_alarm(this, get_interval(), false);
            }
        }
        
        void on_impulse(const Impulse &impulse) {
            Actor *a = dynamic_cast<Actor*>(impulse.sender);
            if(!a && (!isLighted || state == INACTIVE))
                move_stone(impulse.dir);
        }
        void actor_hit(const StoneContact &sc) {
            Actor *a = sc.actor;
            if(a && state == BLINK && player::WieldedItemIs(a, "it-hammer")) {
                state = BREAK;
                init_model();
            }
        }
        bool is_movable () const { return false; }
    };
    
    DEF_TRAITS(LightPassengerStone,"st-lightpassenger", st_lightpassenger);
}

// --------------------------------------------------------------------------------

void stones::Init_complex()
{
    Register(new BolderStone);
    Register("st-bolder-n", new BolderStone(NORTH));
    Register("st-bolder-e", new BolderStone(EAST));
    Register("st-bolder-s", new BolderStone(SOUTH));
    Register("st-bolder-w", new BolderStone(WEST));

    Register(new BlockerStone(true));
    Register(new BlockerStone(false));

    Register(new Door);
    Register("st-door-h", new Door("h"));
    Register("st-door-v", new Door("v"));
    Register("st-door-h-open", new Door("h", true));
    Register("st-door-v-open", new Door("v", true));
    Register(new Door_a);
    Register(new Door_b);
    Register(new Door_c);

    Register(new HollowStoneImpulseStone);

    MailStone::setup();

    Register(new MovableImpulseStone);

    Register(new OneWayStone);
    Register("st-oneway-n", new OneWayStone(NORTH));
    Register("st-oneway-e", new OneWayStone(EAST));
    Register("st-oneway-s", new OneWayStone(SOUTH));
    Register("st-oneway-w", new OneWayStone(WEST));
    Register(new OneWayStone_black);
    Register("st-oneway_black-n", new OneWayStone_black(NORTH));
    Register("st-oneway_black-e", new OneWayStone_black(EAST));
    Register("st-oneway_black-s", new OneWayStone_black(SOUTH));
    Register("st-oneway_black-w", new OneWayStone_black(WEST));
    Register(new OneWayStone_white);
    Register("st-oneway_white-n", new OneWayStone_white(NORTH));
    Register("st-oneway_white-e", new OneWayStone_white(EAST));
    Register("st-oneway_white-s", new OneWayStone_white(SOUTH));
    Register("st-oneway_white-w", new OneWayStone_white(WEST));

    Register(new OxydStone);

    Register (new PullStone);

    Register( new VolcanoStone);
    Register("st-volcano_inactive", new VolcanoStone(VolcanoStone::INACTIVE));
    Register("st-volcano_active", new VolcanoStone(VolcanoStone::ACTIVE));

    // PerOxyd/Enigma compatible puzzle stones:
    Register(new PuzzleStone(0, false));
    Register("st-puzzle-hollow", new PuzzleStone(1, false));
    Register("st-puzzle-n", new PuzzleStone(9, false));
    Register("st-puzzle-e", new PuzzleStone(5, false));
    Register("st-puzzle-s", new PuzzleStone(3, false));
    Register("st-puzzle-w", new PuzzleStone(2, false));
    Register("st-puzzle-ne", new PuzzleStone(13, false));
    Register("st-puzzle-nw", new PuzzleStone(10, false));
    Register("st-puzzle-es", new PuzzleStone(7, false));
    Register("st-puzzle-sw", new PuzzleStone(4, false));
    Register("st-puzzle-ns", new PuzzleStone(11, false));
    Register("st-puzzle-ew", new PuzzleStone(6, false));
    Register("st-puzzle-nes", new PuzzleStone(15, false));
    Register("st-puzzle-new", new PuzzleStone(14, false));
    Register("st-puzzle-nsw", new PuzzleStone(12, false));
    Register("st-puzzle-esw", new PuzzleStone(8, false));
    Register("st-puzzle-nesw", new PuzzleStone(16, false));

    // Oxyd1 compatible puzzle stones:
    Register("st-puzzle2-hollow", new PuzzleStone(1, true));
    Register("st-puzzle2-n", new PuzzleStone(9, true));
    Register("st-puzzle2-e", new PuzzleStone(5, true));
    Register("st-puzzle2-s", new PuzzleStone(3, true));
    Register("st-puzzle2-w", new PuzzleStone(2, true));
    Register("st-puzzle2-ne", new PuzzleStone(13, true));
    Register("st-puzzle2-nw", new PuzzleStone(10, true));
    Register("st-puzzle2-es", new PuzzleStone(7, true));
    Register("st-puzzle2-sw", new PuzzleStone(4, true));
    Register("st-puzzle2-ns", new PuzzleStone(11, true));
    Register("st-puzzle2-ew", new PuzzleStone(6, true));
    Register("st-puzzle2-nes", new PuzzleStone(15, true));
    Register("st-puzzle2-new", new PuzzleStone(14, true));
    Register("st-puzzle2-nsw", new PuzzleStone(12, true));
    Register("st-puzzle2-esw", new PuzzleStone(8, true));
    Register("st-puzzle2-nesw", new PuzzleStone(16, true));

    //Register("st-bigbrick", new BigBrick(1));  // use st-brick instead
    Register("st-bigbrick-n", new BigBrick(9));
    Register("st-bigbrick-e", new BigBrick(5));
    Register("st-bigbrick-s", new BigBrick(3));
    Register("st-bigbrick-w", new BigBrick(2));
    Register("st-bigbrick-ne", new BigBrick(13));
    Register("st-bigbrick-nw", new BigBrick(10));
    Register("st-bigbrick-es", new BigBrick(7));
    Register("st-bigbrick-sw", new BigBrick(4));
    Register("st-bigbrick-ns", new BigBrick(11));
    Register("st-bigbrick-ew", new BigBrick(6));
    Register("st-bigbrick-nes", new BigBrick(15));
    Register("st-bigbrick-new", new BigBrick(14));
    Register("st-bigbrick-nsw", new BigBrick(12));
    Register("st-bigbrick-esw", new BigBrick(8));
    Register("st-bigbrick-nesw", new BigBrick(16));
    
    //Register("st-bigbluesand", new BigBlueSand(1));  // use st-blue-sand instead
    Register("st-bigbluesand-n", new BigBlueSand(9));
    Register("st-bigbluesand-e", new BigBlueSand(5));
    Register("st-bigbluesand-s", new BigBlueSand(3));
    Register("st-bigbluesand-w", new BigBlueSand(2));
    Register("st-bigbluesand-ne", new BigBlueSand(13));
    Register("st-bigbluesand-nw", new BigBlueSand(10));
    Register("st-bigbluesand-es", new BigBlueSand(7));
    Register("st-bigbluesand-sw", new BigBlueSand(4));
    Register("st-bigbluesand-ns", new BigBlueSand(11));
    Register("st-bigbluesand-ew", new BigBlueSand(6));
    Register("st-bigbluesand-nes", new BigBlueSand(15));
    Register("st-bigbluesand-new", new BigBlueSand(14));
    Register("st-bigbluesand-nsw", new BigBlueSand(12));
    Register("st-bigbluesand-esw", new BigBlueSand(8));
    Register("st-bigbluesand-nesw", new BigBlueSand(16));

    Register ("st-rotator-right", new RotatorStone(true, false));
    Register ("st-rotator-left", new RotatorStone(false, false));
    Register ("st-rotator_move-right", new RotatorStone(true, true));
    Register ("st-rotator_move-left", new RotatorStone(false, true));

    Register(new ShogunStone);
    Register("st-shogun-s", new ShogunStone(1));
    Register("st-shogun-m", new ShogunStone(2));
    Register("st-shogun-sm", new ShogunStone(3));
    Register("st-shogun-l", new ShogunStone(4));
    Register("st-shogun-sl", new ShogunStone(5));
    Register("st-shogun-ml", new ShogunStone(6));
    Register("st-shogun-sml", new ShogunStone(7));

    Register(new StoneImpulseStone);

    Register (new Turnstile_Pivot); // oxyd-comaptible
    Register (new Turnstile_Pivot_Green); // not oxyd-comaptible
    Register (new Turnstile_N);
    Register (new Turnstile_S);
    Register (new Turnstile_E);
    Register (new Turnstile_W);

    Register("st-chess_black", new ChessStone(0));
    Register("st-chess_white", new ChessStone(1));

    Register("st-lightpassenger", new LightPassengerStone(true));
    Register("st-lightpassenger_off", new LightPassengerStone(false));
}
