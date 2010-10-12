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

#include "Inventory.hh"
#include "errors.hh"
#include "player.hh"
#include "server.hh"
#include "world.hh"
#include "main.hh"

#include <iostream>

using namespace std;
using namespace enigma;
using namespace world;

Floor::Floor(const char *kind, double friction_, double mfactor,
             FloorFlags flags, FloorFireType flft,
             const char *firetransform_, const char *heattransform_)
: GridObject (kind),
  traits (kind, friction_, mfactor, flags, flft, firetransform_, heattransform_),
  heating_animation(false),
  fire_countdown(1)
{}

Floor::Floor (const FloorTraits &tr)
: GridObject (tr.name.c_str()),
  traits (tr),
  heating_animation(false),
  fire_countdown(1)
{}

Floor *Floor::clone() {
    return new Floor(*this);
}

void Floor::dispose() {
    delete this;
}

Value Floor::message(const string &msg, const Value &val) {
    // "init"     : Start burning, if "initfire" is set.
    // "heat"     : Heat the item, heat-transform floor
    //                 or maybe set fire to it (if burnable).
    // "setfire"  : Just try to make fire (if burnable).
    // "forcefire": Force fire, even on unburnable floor.
    // "stopfire" : Stop fire, put ash but don't transform floor.
    if(msg == "init" && has_firetype(flft_initfire))
        return force_fire();
    if(msg == "heat")
        return try_heating(NODIR, flhf_message);
    if((msg == "ignite" || msg == "expl") && has_firetype(flft_ignitable))
        return try_ignite(NODIR, flhf_message);
    if(msg == "setfire")
        return try_ignite(NODIR, flhf_message);
    if(msg == "forcefire")
        return force_fire();
    if(msg == "stopfire")
        return stop_fire(true);
    return Object::message(msg, val);
}

ecl::V2 Floor::process_mouseforce (Actor *a, ecl::V2 force) {
    if (a->controlled_by(player::CurrentPlayer()))
        return mousefactor() * force;
    else
        return ecl::V2();
}

void Floor::get_sink_speed (double &sinkspeed, double &raisespeed) const {
//     sinkspeed = raisespeed = 0.0;
}

double Floor::friction() const 
{ 
    if (const Value *v = this->get_attrib("friction")) 
        return to_double(*v);
    return traits.friction; 
}

double Floor::mousefactor() const 
{ 
    if (const Value *v = this->get_attrib("mousefactor")) 
        return to_double(*v);
    return traits.mousefactor; 
}

bool Floor::is_destructible() const 
{
    return true;
}

void Floor::set_model (const std::string &mname) 
{
    display::SetModel (GridLoc(GRID_FLOOR, get_pos()), mname);
}

display::Model *Floor::get_model () 
{
    return display::GetModel (GridLoc(GRID_FLOOR, get_pos()));
}

void Floor::kill_model (GridPos p) 
{
    display::KillModel (GridLoc (GRID_FLOOR, p));
}

void Floor::add_force (Actor *, V2 &f)
{
    // Note that actor == 0 is possible from lightpassenger-calculation.
    if(const Value *x = this->get_attrib("force_x")) {
        f[0] += to_double(*x);
    }
    if(const Value *y = this->get_attrib("force_y")) {
        f[1] += to_double(*y);
    }
}

/* -------------- Fire Handling -------------- */

/*  The fire system is a confusing tangle of bools and flags.
 *
 *  A short overview:
 *  A floor is allowed to catch fire when it is:
 *    - burnable by default
 *    - the "burnable"-attribute is set and in Enigma-comp.mode
 *    - it-burnable or it-burnable_oil is on it
 *  Fire on a burnable floor can be ignited by:
 *    - "setfire"-message
 *    - "expl" or "ignite" message if the "ignitable"-attribute is set
 *        - thereby by it-dynamite or it-*bomb + "ignitable"
 *    - fire in the neighborhood (see below)
 *    - on initialisation ("init"-message) when "initfire"-attribute is set
 *  On the floor itself it does
 *    - inform stones on it -> e.g. kill st-wood, st-hay
 *    - keep on burning by chance or if eternal-attribute set
 *    - when stop burning
 *        - replace floor by floor given in floor-transform,
 *            or defaults in non-Enigma-compatibility mode.
 *        - put ash, if noash-attribute of *new* floor is not set
 *  When a floor tile is burning, it does the following to its neighbor:
 *    - ignite items by chance or if secure-attribute is set and
 *        source fire stops next time
 *    - start heat-transformation on first fire call but not yet
 *        transforming and only in Enigma-mode
 *    - inform neighboring stones about fire
 *    - In Enigma-mode (resp. non-Enigma-mode), spread to other burnable
 *        floors (resp. floors of the same kind) by chance or message
 *        or last+secure, if none of the first two methods worked, but
 *        delayed by firecountdown to simulate the old system, but not
 *        if message or last+secure or fastfire-attribute set: Then spread
 *        without countdown. Don't do any of these when there is a movable
 *        stone on it.
 *    - And don't do any of the former three if there is a stone on it,
 *        which is not floating (in non-Enigma-mode: neither movable).
 *  Have I forgotten something?
 */

string Floor::get_firetransform() {
    if(server::GameCompatibility == GAMET_ENIGMA)
        return traits.firetransform;
    // In non-Enigma-compatibility-modes, only fl-wood and fl-stwood
    // transform. Note: This might have been a bug, as fl-stwood1/2
    // and fl-wood1/2 were not included (at least fl-wood1/2 were of
    // a newer date). However, we include them here.
    string model = this->get_kind();
    if (   model == "fl-wood"   || model == "fl-wood1"   || model == "fl-wood2"
        || model == "fl-stwood" || model == "fl-stwood1" || model == "fl-stwood2")
        return "fl-abyss";
    else
        return "";
}
string Floor::get_heattransform(bool override_mode) {
    // The bool is needed to correctly exit the heating-animation
    // in case the level switched to non-Enigma-mode in between.
    if(server::GameCompatibility == GAMET_ENIGMA || override_mode)
        return traits.heattransform;
    // In non-Enigma-compatibility-modes, there is no heat-transformation.
    return "";
}
int Floor::get_fire_countdown() {
    if(Item *it = GetItem(get_pos()))
        if(get_id(it) == it_burnable || get_id(it) == it_burnable_oil)
            return 0;
    return fire_countdown;
}

Value Floor::force_fire() {
    SetItem(get_pos(), it_burnable_ignited);
    fire_countdown = 0;
    return Value(1.0);
}

Value Floor::try_ignite(Direction sourcedir, FloorHeatFlags flhf) {
    GridPos p = get_pos();

    // Don't disturb heating-transformation
    if(heating_animation)
        return Value();

    // No or floating stone -> Burn items and replicate.
    // Movable stone && enigma-mode -> Burn items and replicate.
    // Movable stone && non-enigma-mode -> Only burn items.
    // Else -> Don't do anything.
    // Special case: "st-flrock": No fire at all!
    bool no_closing_stone = true;
    if (Stone *st = GetStone(p)) {
        string model = st->get_kind();
        if (st->is_movable())
            no_closing_stone = false;
        else if(!st->is_floating())
            return Value();
    }

    if(server::GameCompatibility == GAMET_ENIGMA) {
        if(has_firetype(flft_burnable)) {
            // has_firetype also checks whether floor is already burning or ignited
            // (via it-burnables), but not which stone is above it.
            if(Item *it = GetItem(p)) {
                // The item didn't respond to the "heat"-message, so we
                // could assume it's burnable. However, as there might also
                // be a "setfire"-message from the user or a lazy "message"-
                // implementation, we still have to check the burnable-flag
                // of this item:
                if(!has_flags(it, itf_fireproof))
                    return force_fire();
            } else {
                // Just spread. Use the fire-countdown to delay fire without
                // it-burnable, but not in case of a message or a secure+last
                // call or a fastfire-floor.
                if(    (get_fire_countdown() == 0) || (flhf == flhf_message)
                    || (has_firetype(flft_fastfire))
                    || (((bool) (flhf & flhf_last)) && has_firetype(flft_secure)))
                    return force_fire();            
                fire_countdown = max(fire_countdown - 1, 0);
            }        
        }
    } else {  // non-Enigma-mode
        if(has_firetype(flft_burnable) || flhf == flhf_message) {
            // We only get here when there is a burnable item or the
            // burnable attribute is set (e.g. as part of replication).
            // A fireproof item doesn't allow to get here. (Note: We
            // don't check a second time for no_closing_stone.)
            return force_fire();
        } else {
            // This floor doesn't burn by default or by item. But, in
            // non-Enigma-compatibility-mode it should, when its neighbor
            // is of the same kind and burns also.
            if(Floor *fl = GetFloor(move(p, sourcedir))) {
                string sourcekind = fl->get_kind();
                string mykind = this->get_kind();
                if(no_closing_stone && sourcekind == mykind)
                    if(has_firetype(flft_fastfire))
                        return force_fire();
                    else
                        this->set_attrib("burnable", Value(1.0));
            }
        }
    }
    return Value();  // meaning: no fire
}

Value Floor::try_heating(Direction sourcedir, FloorHeatFlags flhf) {
    // First of all: How are we allowed to react at all?
    // There are four branches of heating:
    //
    //  1) item-transformation (e.g. igniting it-dynamite, *not* burning!)
    //  2) floor-heat-transformation (e.g. melting ice)
    //  3) stone-heat-transformation (e.g. fireblocker st-flrock, itemfreeze)
    //  4) fire
    //
    //  a) Always do (1),(2),(3),(4) if try_heating is called from
    //     a "heat"-message by the user/level.
    //  b) Always do (2) if this is the first call of a regular fire
    //     (checked in on_heattransform).
    //  c) Always do (1), (3) and (4) if this is the last call of a regular
    //     fire and the secure-attribute is set (so ignition is safe).
    //  d) Else, do (1), (3) and (4) with probability 0.7, and always both.
    //  e) However, if (1), (2) or (3) led to success, then never do (4),
    //     this way we prevent fire to disturb any transformations.
    //
    bool secure = ((bool) (flhf & flhf_last)) && has_firetype(flft_secure);
    bool doItem = (flhf == flhf_message) || secure || DoubleRand(0, 1) > 0.3;
    bool doStone = doItem;
    bool doIgnite = doItem;
    bool reaction_happened = false;
    // Heat item -> destroy cracks, ignite bombs...
    if(doItem)
        if(Item *it = GetItem(get_pos()))
            if(to_int(SendMessage(it, "heat", Value(sourcedir))) != 0.0)
                reaction_happened = true;        
    // Maybe also transform floor?
    reaction_happened = on_heattransform(sourcedir, flhf) || reaction_happened;
    // Maybe transform stone, or stone blocks fire?
    if(doStone)
        if(Stone *st = GetStone(get_pos()))
            if(to_int(SendMessage(st, "heat", Value(sourcedir))) != 0.0)
                reaction_happened = true;
    // Not item nor floor nor stone reacted? Then try to ignite the floor!
    // (Note: try_ignite also tests for the heating animation:
    //        No fire during transformation allowed!)
    if(doIgnite && !reaction_happened)
        return this->try_ignite(sourcedir, flhf);
    // Else: return reaction_happened from item or heat-transform
    return reaction_happened ? Value(1.0) : Value();
}

bool Floor::on_heattransform(Direction sourcedir, FloorHeatFlags flhf) {
    // return true to forbid fire (message caught)
    bool doHeatTransform = (flhf == flhf_message) || ((bool) (flhf & flhf_first));
    if(!doHeatTransform || get_heattransform(false) == "")
        return false;
    if(doHeatTransform && get_heattransform(false) != "" && !heating_animation) {
        set_anim(((string) this->get_kind()) + "-heating");
        heating_animation = true;
    }
    return true;
}

void Floor::heat_neighbor(Direction dir, FloorHeatFlags flhf) {
    if(Floor *fl = GetFloor(move(get_pos(), dir))) {
        fl->try_heating(reverse(dir), flhf);
    }
}

Value Floor::stop_fire(bool is_message) {
    // stop burning
    //   -> kill burnable-item
    //   -> transform floor?
    //   -> put ash?  (depends on the new floor!)
    //   -> reset fire-countdown to 1
    GridPos p = get_pos();

    // is_message indicates use of the stopfire-message,
    // so we have to check if there is fire at all.
    if(is_message)
        if(Item *it = GetItem(p)) {
            ItemID id = get_id(it);
            if(id != it_burnable_burning && id != it_burnable_ignited)
                return Value();  // no fire
        } else
            return Value(); // no item == no fire

    KillItem(p);
    fire_countdown = 1;
    if(!is_message && get_firetransform() != "")
        SetFloor(p, MakeFloor(get_firetransform().c_str()));
    // Remember, at this point "this" may be destroyed.
    if(!GetFloor(p)->has_firetype(flft_noash))
        SetItem(p, it_burnable_ash);
    return Value(1.0); // fire extinguished  
}

void Floor::on_burnable_animcb(bool justIgnited) {
    GridPos p = get_pos();
    // Analyse and maybe kill stone: May the fire spread?
    bool spread = true;
    if( Stone *st = GetStone(p)) {
        // Return true on fire-message to allow fire to spread.
        // Floating stones also allow spreading. Don't use an
        // OR-statement as the message might kill the stone.
        spread = st->is_floating();
        if(to_int(SendMessage(st, "fire")) != 0.0)
            spread = true;
    }
    // Will we stop this time with burning?
    bool cont_fire = justIgnited || has_firetype(flft_eternal) || DoubleRand(0,1) < 0.7;
    FloorHeatFlags flhf = (FloorHeatFlags) (flhf_fire
        | (justIgnited ? flhf_first : flhf_fire) | (cont_fire ? flhf_fire : flhf_last));
    if(spread) {
        heat_neighbor(NORTH, flhf);
        heat_neighbor(EAST,  flhf);
        heat_neighbor(SOUTH, flhf);
        heat_neighbor(WEST,  flhf);
    }
    if(cont_fire)
        // continue burning
        //   -> put animation
        SetItem(p, it_burnable_burning);
    else
        stop_fire(false);
}

bool Floor::has_firetype(FloorFireType selector) {
    if(Item *it = GetItem(get_pos())) {
        ItemID id = get_id(it);
        if(selector == flft_burnable || selector == flft_ignitable) {
            if(   id == it_burnable || id == it_burnable_oil )
                return true;
            if(   id == it_burnable_ash     || id == it_burnable_fireproof
               || id == it_burnable_ignited || id == it_burnable_burning )
                return false;
            // In non-Enigma-compatibility-modes, the item decides about
            // burnability and only it_burnable[_oil] is ignitable:
            if(server::GameCompatibility != GAMET_ENIGMA)
                return (selector == flft_burnable) && !has_flags(it, itf_fireproof);
        }
    }
    if(selector == flft_burnable)
        if(const Value *v = get_attrib("burnable"))  return to_int(*v) == 1;
    if(selector == flft_ignitable)
        if(const Value *v = get_attrib("ignitable")) return to_int(*v) == 1;
    if(selector == flft_secure)
        if(const Value *v = get_attrib("secure"))    return to_int(*v) == 1;
    if(selector == flft_eternal)
        if(const Value *v = get_attrib("eternal"))   return to_int(*v) == 1;
    if(selector == flft_noash)
        if(const Value *v = get_attrib("noash"))     return to_int(*v) == 1;
    if(selector == flft_fastfire)
        if(const Value *v = get_attrib("fastfire"))  return to_int(*v) == 1;
    if(selector == flft_initfire)
        if(const Value *v = get_attrib("initfire"))  return to_int(*v) == 1;
    if(server::GameCompatibility == GAMET_ENIGMA)
        return traits.firetype & selector;
    // In non-Enigma-modes, without items on them, all floors behave the same:
    return false;
}

void Floor::animcb() {
    // Probably the heating-animation ended.
    if(heating_animation) {
        if(this->get_heattransform(true) != "")
            SetFloor(get_pos(), MakeFloor(get_heattransform(true).c_str()));
        heating_animation = false;
    }
}

/* =============== Special Floors ================ */

namespace
{

/* -------------------- Abyss -------------------- */

    class Abyss : public Floor {
        CLONEOBJ(Abyss);
    public:
        Abyss() : Floor("fl-abyss", 2.0, 1, flf_indestructible, flft_noash) {}
    private:
//         void actor_enter(Actor* a) {SendMessage(a, "fall");}
        void actor_contact (Actor* a) {SendMessage(a, "fall");}
	bool is_destructible() const {return false;}
    };

/* -------------------- Ice -------------------- */

    class Ice : public Floor {
        CLONEOBJ (Ice);
    public:
        Ice() : Floor ("fl-ice", 0.1, 0.1, flf_default, flft_default, "",
            "fl-water") { }

        virtual double friction() const {
            return 0.1 * server::IceFriction;
        }
    };

/* -------------------- Water -------------------- */

    class Water : public Floor {
        CLONEOBJ(Water);
    public:
        Water() : Floor("fl-water", 5, 1, flf_indestructible, flft_default, "",
            "fl-swamp") {}
    private:

        bool is_destructible() const {return false;}

        void get_sink_speed (double &sink_speed, double &raise_speed) const { 
            sink_speed = server::WaterSinkSpeed;
            raise_speed = 1000; // always sink in water
        }

    };

/* -------------------- Swamp -------------------- */

    class Swamp : public Floor {
        CLONEOBJ(Swamp);
    public:
        Swamp() : Floor("fl-swamp", 13, 1.0, flf_indestructible, flft_default, "",
            "fl-dunes") {}
    private:
        bool is_destructible() const {return false;}
        
        void get_sink_speed (double &sink_speed, double &raise_speed) const { 
            sink_speed = server::SwampSinkSpeed;
            raise_speed = 6.0;
        }
    };

/* -------------------- Space with force -------------------- */

    class SpaceForce : public Floor {
        CLONEOBJ (SpaceForce);

        // Floor interface
        void add_force (Actor *, V2 &f) {
            f[1] -= server::FlatForce;
        }

    public:
        SpaceForce() : Floor("fl-space-force", 0, 0) {
        }
    };

/* -------------------- FallenBox -------------------- */

    class FallenBox : public Floor {
        CLONEOBJ(FallenBox);
    public:
        FallenBox(const char *kind)
        :  Floor(kind, 6.4, 2.0, flf_default, flft_burnable, "fl-abyss")
        // uses same traits as fl-wood
        {}

    private:
        const char *modify_kind(const char *kind) {
            if (0 == strcmp(kind, "fl-stwood")) {
                return enigma::IntegerRand(0, 1) ? "fl-stwood1" :  "fl-stwood2";
            }
            return kind;
        }
    };

/* -------------------- DummyFloor -------------------- */

    class DummyFloor : public Floor {
        CLONEOBJ(DummyFloor);
    public:
        DummyFloor() : Floor("fl-dummy", 4.0, 2.5) {}
    private:
        void actor_enter(Actor *) {
            static int lastCode = -1;
            int        code     = int_attrib("code");
            if (lastCode != code) {
                fprintf(stderr, "Entering floor 0x%x\n", code);
                lastCode = code;
            }
        }
    };
}


/* -------------------- Gradient -------------------- */

/** \page fl-gradient Gradient Floor

This is a sloped floor that accelerates in a particular direction.

\subsection gradienta Attributes

- \b type: number between 0 and 15 (see below)

\subsection gradiente Examples

*/
namespace
{
    class Gradient : public Floor {
        CLONEOBJ(Gradient);
        int type;
        bool use_forcefac;
        double forcefac;
    public:
        Gradient(int type=MINTYPE);
    private:
        int get_type() const;
        enum { MINTYPE=1, MAXTYPE=24 };

        // Object interface
        virtual void set_attrib (const string& key, const Value &val);

        // GridObject interface.
        virtual void init_model();

        // Floor interface
        virtual void add_force(Actor *a, V2 &f);
    };
}

Gradient::Gradient(int type_)
: Floor("fl-gradient", 4, 2),
  type (type_),
  use_forcefac (false),
  forcefac (1.0)
{
    set_attrib ("type", Value(type_));
//    set_attrib ("force", Value(1.0));
}

void Gradient::set_attrib (const string& key, const Value &val)
{
    if (key == "type") {
        int t=to_int (val);
        if (t < MINTYPE || t>MAXTYPE) {
            enigma::Log << "Gradient: Illegal type="<< int(t) << std::endl;
            t=MINTYPE;
        }
        type = t;
    }
    else if (key == "force") {
        use_forcefac = true;
        forcefac = to_double (val);
    }
    Object::set_attrib (key, val);
}


int Gradient::get_type() const
{
    return type;
}

void Gradient::init_model()
{
    set_model(ecl::strf("fl-gradient%d", get_type()));
}

void Gradient::add_force(Actor *a, V2 &f)
{
    ecl::V2 force;
    int t = get_type();

    static int xforce[MAXTYPE-MINTYPE+1] = {
         0,  0,  1, -1,
         1, -1,  1, -1,
         1,  1, -1, -1,
        -1, -1,  1,  1,
         1, -1,  1, -1,
         0,  0,  1, -1
    };
    static int yforce[MAXTYPE-MINTYPE+1] = {
         1, -1,  0,  0,
         1,  1, -1, -1,
         1, -1,  1, -1,
         1, -1,  1, -1,
        -1, -1,  1,  1,
         1, -1,  0,  0
    };
    force = ecl::V2(xforce[t-MINTYPE], yforce[t-MINTYPE]);
    force.normalize();

    // use enigma::SlopeForce if no "force" attribute is set
    double factor = server::SlopeForce;
    if (use_forcefac)
        factor = forcefac;
    force *= factor;
    f += force;
    Floor::add_force(a, f);
}


/* -------------------- Bridge -------------------- */

/** \page fl-bridge Bridge Floor

This Floor can be opened and closed much like a bridge.
The actor can move over this floor if the bridge is closed,
and will fall into abyss when the bridge is open.

\subsection bridgea Attributes

- \b type	The type of the bridge, currently only 'a' is possible

\subsection bridgem Messages
- \b open	open the bridge so actors cannot pass it
- \b close	close the bridge so actors can pass it
- \b openclose	toggle the state of the bridge
- \b signal     same as \b openclose

*/
namespace
{
    class Bridge : public Floor {
        CLONEOBJ(Bridge);
    public:
        Bridge(bool open=true);
        virtual Value message(const string &m, const Value &);
    private:
        enum State {
            OPEN, CLOSED, OPENING, CLOSING, // normal states
            CLOSING_BYSTONE, CLOSED_BYSTONE // when stones are moved onto the bridge
        } state;
        // the BYSTONE-states look like closed, but act like open

        char get_type() const {
            string type = "a";
            string_attrib("type", &type);
            return type[0];
        }

//         void actor_enter(Actor *);
        void actor_contact (Actor* a) {if (state!=CLOSED) SendMessage(a, "fall");}
        void init_model();
        void stone_change(Stone *st);

        void change_state( State newstate);
        void animcb();
    };
}

Bridge::Bridge(bool open) : Floor("fl-bridge", 5, 1)
{
    set_attrib("type", "a");
    state=open ? OPEN : CLOSED;
}

void Bridge::stone_change(Stone *st) {
    if (st && !st->is_floating()) {
        if (state == OPEN || state == OPENING) {
            change_state(CLOSING_BYSTONE);
        }
    }
    else {
        if (state == CLOSED_BYSTONE || state == CLOSING_BYSTONE) {
            change_state(OPENING);
        }
    }
}

Value Bridge::message(const string &m, const Value &)
{
    if (m == "open" && (state==CLOSED || state==CLOSING))
        change_state(OPENING);
    else if (m=="close")
        switch (state) {
        case OPEN:
        case OPENING:
        case CLOSING_BYSTONE:
            change_state(CLOSING);
            break;
        case CLOSED_BYSTONE:
            change_state(CLOSED);
            break;
        case CLOSED:
        case CLOSING:
            break; // already closed

        }
    else if (m=="openclose" || m=="signal")
        switch (state) {
        case OPEN:
        case OPENING:
        case CLOSING_BYSTONE:
            change_state(CLOSING);
            break;
        case CLOSED_BYSTONE:
            change_state(CLOSED);
            break;
        case CLOSED:
        case CLOSING:
            change_state(OPENING);
            break;
        }
    return Value();
}

void Bridge::init_model()
{
    set_model(ecl::strf("fl-bridge%c-%s", get_type(),
                       (state==OPEN) ? "open" : "closed"));
}

void Bridge::change_state( State newstate)
{
    if (state != newstate) {
        string mname = string("fl-bridge")+get_type();

        switch( newstate) {
        case OPENING: {
            Stone *st = GetStone(get_pos());
            if (st && !st->is_floating()) {
                if (state == CLOSED || state == CLOSED_BYSTONE)
                    newstate = CLOSED_BYSTONE;
                else if (state == CLOSING || state == CLOSING_BYSTONE)
                    newstate = CLOSING_BYSTONE;
                // here the model is already correct!
            }
            else { // no stone or floating stone :
                if( state == CLOSING || state == CLOSING_BYSTONE)
                    get_model()->reverse();
                else
                    set_anim(mname+"-opening");
            }
            break;
        }
        case CLOSING:
        case CLOSING_BYSTONE:
            if( state == OPENING)
                get_model()->reverse();
            else if (state != CLOSING && state != CLOSING_BYSTONE)
                set_anim(mname+"-closing");
            break;
        case OPEN:
        case CLOSED:
        case CLOSED_BYSTONE:
            state = newstate;
            init_model();
            break;
        }
        state = newstate;
    }
}

void Bridge::animcb()
{
    switch (state) {
        case OPENING: change_state(OPEN); break;
        case CLOSING: change_state(CLOSED); break;
        case CLOSING_BYSTONE: change_state(CLOSED_BYSTONE); break;
        default :
            ASSERT(0, XLevelRuntime, "Bridge: animcb called after unknown animation");
            break;
    }
}

/* -------------------- Thief Floor -------------------- */
namespace{
    class Thief : public Floor {
        CLONEOBJ(Thief);
    public:
        Thief();
    private:
        string modelname;
        enum State { IDLE, EMERGING, RETREATING, CAPTURED } state;
        Actor *m_affected_actor;
        int affected_player;
        Item *bag;
        string get_modelname();
        void init_model();
        void actor_enter(Actor* a);
        void animcb();
        void steal();
        virtual Value message(const string &msg, const Value &v);        
    };
}

Thief::Thief() 
: Floor("fl-thief", 2.0, 1),
  state(IDLE), m_affected_actor (0), affected_player (-1), modelname(""), bag(NULL) { }

string Thief::get_modelname() {
    if(modelname == "") {
        // initialize on first call
        int r = IntegerRand(1,4);
        modelname = string("fl-thief")
            + ((string) (r==1?"1":r==2?"2":r==3?"3":"4"));
    }
    return modelname;
}

void Thief::init_model() {
    set_model(get_modelname());
}

void Thief::actor_enter(Actor *a) {
    ActorID id = get_id(a);
    if (state == IDLE) {
        set_anim(get_modelname() + string("-emerge"));
        state = EMERGING;
        m_affected_actor = a;
        affected_player = -1;
        m_affected_actor->int_attrib("player", &affected_player);
    }
}

void Thief::animcb() {
    switch (state) {
    case EMERGING:
        steal();
        state = RETREATING;
        set_anim(get_modelname() + string("-retreat"));
        break;
    case RETREATING:
        state = IDLE;
        init_model();
        break;
    case CAPTURED:
        // Floor is not killed or replaced - it just keeps inactive.
        init_model();
        break;
    default:
        ASSERT(0, XLevelRuntime, "Thief (floor): animcb called with inconsistent state");
    }
}

void Thief::steal() 
{
    bool didSteal = false;
    // steal from player -- the actor that hit the thief may no longer exist!
    if (m_affected_actor && affected_player >= 0 &&
            player::HasActor(affected_player, m_affected_actor) && 
            !m_affected_actor->has_shield()) {
        enigma::Inventory *inv = player::GetInventory(m_affected_actor);
        if (inv && inv->size() > 0) {
            if (bag == NULL)
                bag = world::MakeItem(it_bag);
            int i = IntegerRand (0, int (inv->size()-1));
            dynamic_cast<ItemHolder *>(bag)->add_item(inv->yield_item(i));
            player::RedrawInventory (inv);
        }
    }
    // steal from grid
    if(Item *it = GetItem(get_pos())) {
        if (!(it->get_traits().flags & itf_static)) {
            if (bag == NULL)
                bag = world::MakeItem(it_bag);
            dynamic_cast<ItemHolder *>(bag)->add_item(world::YieldItem(get_pos())); 
            didSteal = true;
        }
    }
    if (didSteal)
        sound_event("thief");
}

Value Thief::message(const string &msg, const Value &v) {
    if(msg == "capture" && state == IDLE) {
        state = CAPTURED;
        Item * it =  world::GetItem(get_pos());
        
        // add items on grid pos that can be picked up to our bag
        if (it != NULL && !(it->get_traits().flags & itf_static) && bag != NULL) {
            dynamic_cast<ItemHolder *>(bag)->add_item(world::YieldItem(get_pos()));
        }
        // drop bag if pos is not occupied by a static item
        if (world::GetItem(get_pos()) == NULL)
            world::SetItem(get_pos(), bag);
        bag = NULL;
        set_anim(get_modelname() + string("-captured"));
        return Value(1);
    } else
        return Floor::message(msg, v);
}

//----------------------------------------
// Black and white tiles
//----------------------------------------
namespace
{
    class BlackTile : public Floor {
        CLONEOBJ(BlackTile);
    public:
        BlackTile() : Floor ("fl-acblack", 5.2, 2.0) {}

        ecl::V2 process_mouseforce (Actor *, ecl::V2 force) {
            if (player::CurrentPlayer() == 0)
                return mousefactor() * force;
            else
                return ecl::V2();
        }
    };

    class WhiteTile : public Floor {
        CLONEOBJ(WhiteTile);
    public:
        WhiteTile() : Floor ("fl-acwhite", 5.2, 2.0) {}

        ecl::V2 process_mouseforce (Actor *, ecl::V2 force) {
            if (player::CurrentPlayer() == 1)
                return mousefactor() * force;
            else
                return ecl::V2();
        }
    };
}


void world::InitFloors()
{
    // Floors (most floors are defined in init.lua)
    Register(new Abyss);
    Register(new Ice);
    Register(new Water);
    Register(new Swamp);
    Register(new DummyFloor);
    Register(new FallenBox("fl-stwood"));
    Register(new FallenBox("fl-stwood1"));
    Register(new FallenBox("fl-stwood2"));
    Register(new Bridge);
    Register("fl-bridge-open", new Bridge(true));
    Register("fl-bridge-closed", new Bridge(false));
    Register(new Thief);
    Register(new WhiteTile);
    Register(new BlackTile);
    Register(new SpaceForce);

    Register(new Gradient);
    Register("fl-gradient1", new Gradient(1));
    Register("fl-gradient2", new Gradient(2));
    Register("fl-gradient3", new Gradient(3));
    Register("fl-gradient4", new Gradient(4));
    Register("fl-gradient5", new Gradient(5));
    Register("fl-gradient6", new Gradient(6));
    Register("fl-gradient7", new Gradient(7));
    Register("fl-gradient8", new Gradient(8));
    Register("fl-gradient9", new Gradient(9));
    Register("fl-gradient10", new Gradient(10));
    Register("fl-gradient11", new Gradient(11));
    Register("fl-gradient12", new Gradient(12));
    Register("fl-gradient13", new Gradient(22));
    Register("fl-gradient14", new Gradient(21));
    Register("fl-gradient15", new Gradient(24));
    Register("fl-gradient16", new Gradient(23));
}
