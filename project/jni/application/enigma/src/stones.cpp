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
#include "stones_internal.hh"
#include "server.hh"
#include "client.hh"
#include "player.hh"
#include "Inventory.hh"

using namespace std;
using namespace world;
using namespace stones;


/* -------------------- Helper routines -------------------- */

/*! Determine whether the actor hitting the stone can move stone
  and return either the direction the stone should move or NODIR. */
Direction stones::get_push_direction (const StoneContact &sc) 
{
    ActorInfo *ai  = sc.actor->get_actorinfo();
    Direction  dir = contact_face(sc);

    // Make sure the speed component towards the face of the stone is
    // large enough and pointing towards the stone.
    if (dir!=enigma::NODIR && ai->vel * sc.normal < -4)
        return reverse(dir);
    return NODIR;
}

/* Move a stone (by sending an impulse) Called when an actor hits a
   stone. */
bool stones::maybe_push_stone (const StoneContact &sc) 
{
    Direction dir = get_push_direction(sc);
    if (dir != enigma::NODIR) {
        sc.actor->send_impulse(sc.stonepos, dir);
        return GetStone(sc.stonepos) == 0; // return true only if stone vanished
    }
    return false;
}


//======================================================================
// STONES
//======================================================================

/** \page lstones  Available Stones

Oxyd Stones:
- \ref st-oxyd
- \ref st-fakeoxyd
- \ref st-fart

Movable stones:
- \ref st-brownie
- \ref st-wood
- \ref st-block

Stones that can trigger actions:
- \ref st-switch
- \ref st-fourswitch
- \ref st-laserswitch
- \ref st-key
- \ref st-coinslot
- \ref st-timer

Stones that can change their behaviour:
- \ref st-brick_magic
- \ref st-stonebrush
- \ref st-invisible_magic
- \ref st-break_invisible

Lasers and Mirrors:
- \ref st-laser
- \ref st-pmirror
- \ref st-3mirror

Other stones:
- \ref st-death
- \ref st-swap
- \ref st-bolder
- \ref st-puzzle
- \ref st-stone_break
- \ref st-window
- \ref st-break_acwhite
- \ref st-break_acblack
- \ref st-oneway
- \ref st-oneway_black
- \ref st-oneway_white
- \ref st-chameleon
*/

Stone::Stone()
{}

Stone::Stone(const char * kind)
: GridObject (kind)
{}

Stone::~Stone() { revokeDelayedImpulses(this); }

const StoneTraits &Stone::get_traits() const
{
    static StoneTraits default_traits = {
        "INVALID", st_INVALID, stf_none, material_stone, 1.0
    };
    return default_traits;
}

const char *Stone::get_kind() const
{
    const StoneTraits &tr = get_traits();
    if (tr.id != st_INVALID)
        return tr.name;
    else
        return Object::get_kind();
}

StoneResponse Stone::collision_response(const StoneContact &) {
    return STONE_REBOUND;
}


void Stone::actor_hit(const StoneContact &sc) 
{
    if (is_movable())
        stones::maybe_push_stone (sc);
}

void Stone::actor_touch(const StoneContact &sc) {
}

void Stone::on_impulse(const Impulse& impulse) {
    if (is_movable())
        move_stone(impulse.dir);
}

const char * Stone::collision_sound() {
    return "stone";
}

/* Move a stone (regardless whether it is_movable() or not) if
   the destination field is free.
   Returns: true if stone has been moved.

   Note: This should be used by on_impulse() to perform a move.
*/
bool Stone::move_stone(GridPos newPos, const char *soundevent) {
    GridPos p      = get_pos();

    if (!GetStone(newPos)) {
        sound_event (soundevent);

        MoveStone(p, newPos);
        server::IncMoveCounter();

        on_move();
        if (Item *it = GetItem(newPos)) it->on_stonehit(this);

        return true;
    }
    return false;
}
bool Stone::move_stone(Direction dir) {
    return move_stone(move(get_pos(), dir), "movesmall");
}

void Stone::on_move() {
    if (!is_floating()) 
        ShatterActorsInsideField (get_pos());
}

/* Multiplies velocity with the attribute-matrix
   hit_distortion_[xx,xy,yx,yy] and factor hit_factor
   If components are not set, use ((1,0),(0,1)) as
   default matrix, resp. defaultfactor as hit_factor. */
ecl::V2 Stone::distortedVelocity (ecl::V2 vel, double defaultfactor = 1.0) {
    ecl::V2 newvel(0,0);    
    if(const Value *xx = this->get_attrib("hit_distortion_xx"))
        newvel[0] += to_double(*xx) * vel[0];
    else
        newvel[0] += vel[0];
    if(const Value *xy = this->get_attrib("hit_distortion_xy"))
        newvel[0] += to_double(*xy) * vel[1];
    if(const Value *yx = this->get_attrib("hit_distortion_yx"))
        newvel[1] += to_double(*yx) * vel[0];
    if(const Value *yy = this->get_attrib("hit_distortion_yy"))
        newvel[1] += to_double(*yy) * vel[1];
    else
        newvel[1] += vel[1];
    if (const Value *factor = this->get_attrib("hit_factor"))
        newvel *= to_double(*factor);
    else
        newvel *= defaultfactor;    
    return newvel;
}



// *******************************************************************************
//  Stones under development :



/* -------------------- Explosion stone -------------------- */
namespace
{
    class ExplosionStone : public Stone {
        CLONEOBJ(ExplosionStone);

        StoneResponse collision_response(const StoneContact &) {
            return STONE_PASS;
        }

        void actor_contact (Actor *a) {
            SendMessage(a, "shatter");
        }
        void init_model() {
            set_anim("st-explosion");
        }

        void animcb() {
            KillStone(get_pos());
        }
    public:
        ExplosionStone(): Stone("st-explosion")
        {}
    };
}


/* -------------------- Charge stone -------------------- */

// Attributes:
//
// :charge  + - 0
namespace
{
    class ChargeStone : public Stone {
        CLONEOBJ(ChargeStone);
    public:
        ChargeStone(const char *kind, double charge)
        : Stone(kind)
        {
            set_attrib("charge", charge);
        }
    private:
        double get_charge() {
            double q = 0;
            double_attrib("charge", &q);
            return max(-1.0, min(1.0, q));
        }
        void animcb() { init_model(); }
        void actor_hit (const StoneContact &sc) {
            ActorInfo *ai = sc.actor->get_actorinfo();
            ai->charge = get_charge();
            set_anim(string(get_kind())+"-anim");
        }
    };
}


/* -------------------- SpitterStone -------------------- */

namespace
{
    class SpitterStone : public Stone {
        CLONEOBJ(SpitterStone);

        enum State { IDLE, LOADING, SPITTING };

        // Variables
        State state;
        V2    ball_velocity;

        // Functions.
        void animcb();
        void actor_hit (const StoneContact &sc);

    public:
        SpitterStone () : Stone("st-spitter"), state (IDLE) {
        }
    };
}

void SpitterStone::animcb() {
    switch (state) {
    case IDLE:
        ASSERT(0, XLevelRuntime, "SpitterStone: animcb called with inconsistent state"); 
    case LOADING: {
        Actor     *ball = MakeActor (ac_cannonball);
        ActorInfo *ai   = ball->get_actorinfo();
        V2         center  = get_pos().center();

        state = SPITTING;
        ai->vel = ball_velocity;
        world::AddActor (center[0], center[1], ball);
        set_anim ("st-spitter-spitting");
        break;
    }
    case SPITTING:
        init_model();
        state = IDLE;
        break;
    }
}

void SpitterStone::actor_hit (const StoneContact &sc) 
{
    if (state != IDLE)
        return;

    if (enigma::Inventory *inv = player::GetInventory(sc.actor)) {
        int lifepos = inv->find("it-extralife");
        if (lifepos != -1) {
            delete inv->yield_item(lifepos);
            player::RedrawInventory (inv);
            ball_velocity = distortedVelocity(sc.actor->get_vel(), 1.0);
            state = LOADING;
            set_anim ("st-spitter-loading");
        }
    }
}


/* -------------------- YieldedGridStone -------------------- */

YieldedGridStone::YieldedGridStone(Stone *st)
: stone(st)
{
    GridPos pos = stone->get_pos();
    model       = display::YieldModel(GridLoc(GRID_STONES, pos));
    YieldStone(pos);
}

YieldedGridStone::~YieldedGridStone() 
{
    ASSERT(!stone, XLevelRuntime,
        "YieldedGridStone: destructor called though stone still exists");
    ASSERT(!model, XLevelRuntime,
        "YieldedGridStone: destructor called though model still exists");
}

void YieldedGridStone::set_stone(GridPos pos) 
{
    SetStone(pos, stone);
    display::SetModel(GridLoc(GRID_STONES, stone->get_pos()), model);
    stone->on_move();
    stone = 0;
    model = 0;
}

void YieldedGridStone::dispose() 
{
    stone->dispose();
    delete model;
    stone = 0;
    model = 0;
}


/* -------------------- Oxyd compatibility stones -------------------- */

namespace
{
    /* I have no idea what these stones are _really_ supposed to do;
       they seemingly do not appear in the landscape and they create
       normal floor tiles on creation.  Other than that... who
       knows... */
    class Peroxyd_0xb8 : public Stone {
        CLONEOBJ(Peroxyd_0xb8);
    public:
        Peroxyd_0xb8() : Stone("st-peroxyd-0xb8")
        {}

        void on_creation (GridPos p) {
            SetFloor (p, MakeFloor ("fl-normal"));
            KillStone(p);
        }
    };

    class Peroxyd_0xb9 : public Stone {
        CLONEOBJ(Peroxyd_0xb9);
    public:
        Peroxyd_0xb9() : Stone("st-peroxyd-0xb9")
        {}

        void on_creation (GridPos p) {
            SetFloor (p, MakeFloor ("fl-normal"));
            KillStone(p);
        }
    };

    class Oxyd_0x18 : public Stone {
        CLONEOBJ(Oxyd_0x18);
    public:
        Oxyd_0x18() : Stone("st-oxyd-0x18") {
        }
        void on_creation (GridPos p) {
            KillStone (p);
        }
    };
}


/* -------------------- Flash stone -------------------- */
namespace
{
    class FlashStone : public Stone {
        CLONEOBJ(FlashStone);

        void actor_hit (const StoneContact &sc) {
            if (Actor *other = FindOtherMarble(sc.actor)) {
                other->add_force (distortedVelocity(sc.actor->get_vel(), 20));
            }
        }

    public:
        FlashStone() : Stone ("st-flash")
        {}
    };
}


/* -------------------- Surprise stone -------------------- */
namespace 
{
    class SurpriseStone : public Stone {
        CLONEOBJ (SurpriseStone);
    public:
        SurpriseStone() : Stone("st-surprise")
        {}

        void actor_hit (const StoneContact &) {
            static const char *stonename[] = {
                "st-grate1",
                "st-death",
                "st-surprise",
                "st-glass1_hole",
                "st-magic",
                "st-knight",
                "st-thief",
                "st-plain_break",
                "st-plain_breaking"
            };
            int idx = enigma::IntegerRand (1, 9) - 1;
            sound_event ("stonetransform");
            ReplaceStone (get_pos(), MakeStone (stonename[idx]));
        }
    };
}


/* -------------------- Coffee stone -------------------- */
namespace
{
    class CoffeeStone : public Stone {
        CLONEOBJ(CoffeeStone);
    public:
        CoffeeStone() : Stone ("st-coffee") {
        }

        void actor_hit (const StoneContact &) {
            sound_event ("stonetransform");
            ReplaceStone(get_pos(), MakeStone("st-glass_move"));
        }
    };
}


/* -------------------- Breaking stone -------------------- */
namespace
{
    class BreakingStone : public Stone {
        CLONEOBJ(BreakingStone);

        void init_model() {
            sound_event("stonedestroy");
            set_anim("st-breaking");
        }

        void animcb() {
            KillStone(get_pos());
        }
    public:
        BreakingStone() : Stone("st-breaking") {
        }
    };
}


/* -------------------- Bug stone -------------------- */
namespace
{
    class BugStone : public Stone {
        CLONEOBJ(BugStone);
    public:
        BugStone() : Stone("st-bug") {
        }

        void actor_hit (const StoneContact &sc) {
            if (get_id(sc.actor) == ac_bug) {
                ReplaceStone(get_pos(), MakeStone("st-breaking"));
            }
        }
    };
}


/* -------------------- Plain stones -------------------- */

/* These stones mimic the behaviour of the plain-looking stones in
   Oxyd. */
namespace
{
    class PlainStone : public Stone {
        CLONEOBJ(PlainStone);


        void on_laserhit (Direction) {
            ReplaceStone (get_pos(), MakeStone("st-plain_cracked"));
        }

        const char *collision_sound() {return "stone";}

        virtual Value message (const string &msg, const Value &) {
            if (msg == "trigger" || msg == "signal") {
                ReplaceStone(get_pos(), MakeStone("st-plain_hole"));
            }
            return Value();
        }
        void actor_hit (const StoneContact &sc) {
            if (player::WieldedItemIs (sc.actor, "it-pencil")) {
                enigma::Inventory *inv = player::GetInventory(sc.actor);
                if (inv && inv->size() > 0) {
                    delete inv->yield_item(0);
                    player::RedrawInventory(inv);
                    sound_event("stonepaint");
                    ReplaceStone(get_pos(), MakeStone("st-firebreak"));
                }
            } else
                Stone::actor_hit(sc);
        }
    public:
        PlainStone() : Stone("st-plain") {}
    };

    class PlainStone_Hollow : public Stone {
        CLONEOBJ(PlainStone_Hollow);

        virtual Value message (const string &msg, const Value &) {
            if (msg == "trigger" || msg == "signal") {
                ReplaceStone(get_pos(), MakeStone("st-plain"));
            }
            return Value();
        }

        StoneResponse collision_response(const StoneContact &) 
        { return STONE_PASS; }

        bool is_floating() const { return true; }
    public:
        PlainStone_Hollow() : Stone("st-plain_hole") {
        }
    };

    class PlainStone_Breaking : public Stone {
        CLONEOBJ(PlainStone_Breaking);

        void init_model() {
            set_anim("st-plain_breaking");
        }
        void animcb() {
            KillStone(get_pos());
        }
        const char *collision_sound() {return "metal";}
    public:
        PlainStone_Breaking() : Stone ("st-plain_breaking") {
        }
    };

    class PlainStone_Breakable : public Stone {
        CLONEOBJ(PlainStone_Breakable);

        const char *collision_sound() {return "metal";}

        void break_me() {
            sound_event ("stonedestroy");
            ReplaceStone(get_pos(), MakeStone ("st-plain_breaking"));
        }
        void on_laserhit (Direction) {
            break_me();
        }
        virtual Value message (const string &msg, const Value &) {
            if (msg =="ignite" || msg == "expl" || msg == "bombstone")
                break_me();
            return Value();
        }
        void actor_hit (const StoneContact &sc) {
            if (player::WieldedItemIs (sc.actor, "it-hammer")) {
                break_me();
            }
        }

        void on_floor_change() {
            if (Floor *fl = GetFloor (get_pos()))
                if (fl->is_kind("fl-abyss"))
                    ReplaceStone (get_pos(), MakeStone("st-plain_falling"));
        }

    public:
        PlainStone_Breakable() : Stone("st-plain_break") {
        }
    };

    class PlainStone_Cracked : public Stone {
        CLONEOBJ(PlainStone_Cracked);

        void break_me() {
            sound_event ("stonedestroy");
            ReplaceStone(get_pos(), MakeStone("st-plain_breaking"));
        }

        void actor_hit (const StoneContact &sc) {
            if (player::WieldedItemIs (sc.actor, "it-hammer")) {
                break_me();
            }
        }

        virtual Value message (const string &msg, const Value &) {
            if (msg =="ignite" || msg == "expl" || msg == "bombstone")
                break_me();
            return Value();
        }
        const char *collision_sound() {return "metal";}
    public:
        PlainStone_Cracked() : Stone("st-plain_cracked") {
        }
    };

    class PlainStone_Falling : public Stone {
        CLONEOBJ(PlainStone_Falling);

        void init_model() {
            set_anim("st-plain_falling");
        }

        void animcb() {
            sound_event ("stonedestroy");
            KillStone(get_pos());
        }
    public:
        PlainStone_Falling() : Stone("st-plain_falling") {
        }
    };

    class PlainStone_Movable : public Stone {
        CLONEOBJ(PlainStone_Movable);

        void break_me() {
            sound_event ("stonedestroy");
            ReplaceStone(get_pos(), MakeStone ("st-plain_breaking"));
        }
        virtual Value message (const string &msg, const Value &) {
            if (msg =="ignite" || msg == "expl" || msg == "bombstone")
                break_me();
            return Value();
        }
        void on_move() {
            Stone::on_move();
            GridPos p = get_pos();
            if (Floor *fl = GetFloor (p)) {
                if (fl->is_kind("fl-abyss")) {
                    ReplaceStone (p, MakeStone("st-plain_falling"));
                }
                else if (fl->is_kind("fl-swamp") || fl->is_kind("fl-water")) {
                    sound_event ("drown");
                    client::Msg_Sparkle (p.center());
                    KillStone (p);
                }
            }
        }

        bool is_movable () const { return true; }

        void actor_hit (const StoneContact &sc) {
            if (player::WieldedItemIs (sc.actor, "it-pencil")) {
                enigma::Inventory *inv = player::GetInventory(sc.actor);
                if (inv && inv->size() > 0) {
                    delete inv->yield_item(0);
                    player::RedrawInventory(inv);
                    sound_event("stonepaint");
                    ReplaceStone(get_pos(), MakeStone("st-firebreak_move"));
                }
            } else
                Stone::actor_hit(sc);
        }

    public:
        PlainStone_Movable() : Stone("st-plain_move") {
        }
    };
}


/* -------------------- Black- and Whiteballs Stones -------------------- */

namespace
{
    class BlackBallsStone : public Stone {
        CLONEOBJ(BlackBallsStone);

        virtual Value on_message (const Message &m)
        {
            GridPos p = get_pos();
            Actor *a = world::CurrentCollisionActor;
            if (a && get_id(a) == ac_blackball) {
                if (p.y == m.gridpos.y) {
                    SendMessage (GetStone (move (p, EAST)),  "signal", 1.0);
                    SendMessage (GetStone (move (p, WEST)),  "signal", 1.0);
                    SendMessage (GetStone (move (p, NORTH)), "signal", 0.0);
                    SendMessage (GetStone (move (p, SOUTH)), "signal", 0.0);
                }
                else {
                    SendMessage (GetStone (move (p, EAST)),  "signal", 0.0);
                    SendMessage (GetStone (move (p, WEST)),  "signal", 0.0);
                    SendMessage (GetStone (move (p, NORTH)), "signal", 1.0);
                    SendMessage (GetStone (move (p, SOUTH)), "signal", 1.0);
                }
            }
            return Value();
        }
    public:
        BlackBallsStone() : Stone ("st-blackballs") {
        }
    };

    class WhiteBallsStone : public Stone {
        CLONEOBJ(WhiteBallsStone);

        virtual Value on_message (const Message &m)
        {
            GridPos p = get_pos();
            Actor *a = world::CurrentCollisionActor;
            if (a && get_id(a) == ac_whiteball) {
                if (p.y == m.gridpos.y) {
                    SendMessage (GetStone (move (p, EAST)),  "signal", 1.0);
                    SendMessage (GetStone (move (p, WEST)),  "signal", 1.0);
                    SendMessage (GetStone (move (p, NORTH)), "signal", 0.0);
                    SendMessage (GetStone (move (p, SOUTH)), "signal", 0.0);
                }
                else {
                    SendMessage (GetStone (move (p, EAST)),  "signal", 0.0);
                    SendMessage (GetStone (move (p, WEST)),  "signal", 0.0);
                    SendMessage (GetStone (move (p, NORTH)), "signal", 1.0);
                    SendMessage (GetStone (move (p, SOUTH)), "signal", 1.0);
                }
            }
            return Value();
        }

    public:
        WhiteBallsStone() : Stone ("st-whiteballs") {
        }

    };
}

/* -------------------- Unimplemented stones -------------------- */

namespace
{
    class FakeOxydA : public Stone {
        CLONEOBJ(FakeOxydA);
    public:

        FakeOxydA() : Stone("st-fakeoxyda") {
        }

        void actor_hit (const StoneContact &) {
            sound_event ("stonetransform");
            ReplaceStone(get_pos(), MakeStone("st-glass1_move"));
        }
    };
}


// --------------------------------------------------------------------------------

extern void InitSwitches();

void stones::Init()
{

    // Register(new ...);

    Register (new ExplosionStone);
    Register (new ChargeStone ("st-chargeplus", +1.0));
    Register (new ChargeStone ("st-chargeminus", -1.0));
    Register (new ChargeStone ("st-chargezero", 0.0));
    Register (new SpitterStone);
    Register (new Peroxyd_0xb8);
    Register (new Peroxyd_0xb9);
    Register (new Oxyd_0x18);
    Register (new FlashStone);
    Register (new SurpriseStone);
    Register (new CoffeeStone);
    Register (new BlackBallsStone);
    Register (new WhiteBallsStone);
    Register (new FakeOxydA);
    Register (new BreakingStone);
    Register (new BugStone);
    Register (new PlainStone);
    Register (new PlainStone_Hollow);
    Register (new PlainStone_Breakable);
    Register (new PlainStone_Breaking);
    Register (new PlainStone_Cracked);
    Register (new PlainStone_Movable);
    Register (new PlainStone_Falling);

    // Init stones from stones_simple.cc and stones_complex.cc:
    Init_simple();
    Init_complex();
    InitSwitches();
}
