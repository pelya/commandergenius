/*
 * Copyright (C) 2002,2003,2004,2005 Daniel Heck
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
#include "player.hh"
#include "sound.hh"
#include "server.hh"
#include "world.hh"

#include <iostream>
#include <set>

using namespace std;
using namespace enigma;
using namespace world;
using ecl::V2;

#include "actors_internal.hh"



/* -------------------- Helper functions -------------------- */


#define DECL_TRAITS \
        static ActorTraits traits; \
        const ActorTraits &get_traits() const { return traits; } \



/* -------------------- Actor -------------------- */

Actor::Actor (const ActorTraits &tr)
: Object(tr.name),
  m_actorinfo(),
  m_sprite(),
  startingpos(), 
  respawnpos(), use_respawnpos(false),
  spikes(false), controllers (0)
{
    set_attrib("mouseforce", 0.0);

    // copy default properties to dynamic properties
    m_actorinfo.mass = tr.default_mass;
    m_actorinfo.radius = tr.radius;

    ASSERT(m_actorinfo.radius <= get_max_radius(), XLevelRuntime, "Actor: radius of actor too large");
}

void Actor::on_collision (Actor *) 
{
}

ActorInfo *Actor::get_actorinfo() { 
    return &m_actorinfo; 
}

const ActorInfo &Actor::get_actorinfo() const {
    return m_actorinfo; 
}

const ecl::V2 &Actor::get_pos() const
{
    return m_actorinfo.pos;
}


double Actor::get_max_radius() {
    return 24/64.0;
}

void Actor::think(double /*dtime*/) {
    const Field *f = GetField (get_gridpos());
    if (f) {
        Floor *fl = f->floor;
        Item *it = f->item;
        bool item_covers_floor = (it && it->covers_floor(m_actorinfo.pos));
        if (!item_covers_floor && fl && this->is_on_floor())
            fl->actor_contact(this);
    }
}

void Actor::set_respawnpos(const V2& p)
{
    respawnpos     = p;
    use_respawnpos = true;
}

void Actor::remove_respawnpos() {
    use_respawnpos = false;
}

void Actor::find_respawnpos() {
    V2& what_pos = use_respawnpos ? respawnpos : startingpos;
    FreeRespawnLocationFinder unblocked(what_pos, *this);
    what_pos = unblocked.get_position();
}

const V2& Actor::get_respawnpos() const {
    return use_respawnpos ? respawnpos : startingpos;
}

const V2 &Actor::get_startpos() const {
    return startingpos;
}

void Actor::respawn() {
    V2 p =(use_respawnpos) ? respawnpos : startingpos;
    warp (p);
    on_respawn(p);
}

void Actor::add_force (const ecl::V2 &f) {
    m_actorinfo.forceacc += f;
}

void Actor::init() {
    m_sprite = display::AddSprite(get_pos());
}

void Actor::on_creation(const ecl::V2 &p) 
{
    set_model(get_kind());
    m_sprite.move (p);
    move ();
}

void Actor::on_respawn (const ecl::V2 &/*pos*/) {
}

void Actor::warp(const ecl::V2 &newpos) {
    m_actorinfo.pos = newpos;
    m_actorinfo.vel = V2();
    m_sprite.move (newpos);
    move ();
}

void Actor::move () 
{
    GridPos ofield = gridpos;
    gridpos = GridPos (m_actorinfo.pos);

    if (const Field *f = GetField (gridpos)) {
        if (gridpos != ofield) {
            // Actor entered a new field -> notify floor and item objects
            if (Floor *fl = f->floor)
                fl->actor_enter (this);
            if (Item *it = f->item)
                it->actor_enter (this);

            if (const Field *of = GetField (ofield)) {
                if (Floor *fl = of->floor)
                    fl->actor_leave (this);
                if (Item *it = of->item)
                    it->actor_leave (this);
            }
        }

        Item *it = f->item;
        if (it && it->actor_hit(this))
            player::PickupItem (this, gridpos);

        if (Stone *st = f->stone)
            st->actor_inside (this);
    }
    // move the actor and save the position
    m_actorinfo.oldpos = m_actorinfo.pos;
}

void Actor::move_screen () {
    m_sprite.move (m_actorinfo.pos);
}

void Actor::set_model(const string &name) {
    m_sprite.replace_model (display::MakeModel(name));
}

void Actor::animcb () {
}


void Actor::hide() {
    m_sprite.hide();
}

void Actor::show() {
    m_sprite.show();
}

void Actor::set_anim (const string &modelname) {
    set_model(modelname.c_str());
    get_sprite().set_callback (this);
}

bool Actor::can_move() const { 
    if (Stone *st = GetStone (get_gridpos())) {
        if (!server::NoCollisions)
            return !st->is_sticky(this);
    }
    return true;
}

Value Actor::message(const string &m, const Value &) {
    if (m == "init") {
        startingpos = get_pos();
    }
    return Value();
}

bool Actor::sound_event (const char *name, double vol) {
    return sound::EmitSoundEvent (name, get_pos(), getVolume(name, this, vol));
}

void Actor::set_attrib(const string& key, const Value &val)
{
    if (key == "controllers")
        controllers = to_int (val);
    else if (key == "mouseforce") 
        mouseforce = to_double (val);
    Object::set_attrib (key, val);
}




/* -------------------- RotorBase -------------------- */

namespace
{
    /*! The base class for rotors and spinning tops. */
    class RotorBase : public Actor {
    protected:
        RotorBase(const ActorTraits &tr);
    private:
        // Actor interface.
        void think (double dtime);
        bool is_dead() const { return false; }
//         bool is_flying() const { return true; }
        bool is_flying() const { return false; }
//        bool is_on_floor() const { return false; } we need friction!

	void on_collision (Actor *a) {
	    SendMessage(a, "shatter");
	}

        bool attackCurrentOnly;
        double timeKeepAttackStrategy;
    };
}

RotorBase::RotorBase(const ActorTraits &tr)
: Actor(tr), attackCurrentOnly(false), timeKeepAttackStrategy(0)
{
    set_attrib ("range", 5.0);
    set_attrib ("force", 10.0);
    set_attrib ("gohome", 1.0);
    set_attrib ("attacknearest", 1.0);
    set_attrib ("prefercurrent", 0.0);
}

void RotorBase::think (double dtime)
{
    double range = 0, force = 0;
    double_attrib("range", &range);
    double_attrib("force", &force);

    force /= 6;

    Actor *target = 0;
    V2     target_vec;
    bool attack_nearest = (int_attrib ("attacknearest") != 0);

    double preferCurrent;
    double_attrib("prefercurrent", &preferCurrent);
    timeKeepAttackStrategy  -= dtime;
    if (timeKeepAttackStrategy < 0) {
        timeKeepAttackStrategy = enigma::DoubleRand(0.8, 1.6);
        attackCurrentOnly = (enigma::DoubleRand(0.0, 1.0) < preferCurrent);
    }

    vector<Actor *> actors;
    GetActorsInRange (get_pos(), range, actors);
    for (size_t i=0; i<actors.size(); ++i) {
        Actor *a = actors[i];
        ActorID id = get_id (a);
        if ((id == ac_whiteball || id == ac_blackball || id == ac_meditation) 
            && a->is_movable() && !a->is_invisible())
        {
            V2 v = a->get_pos() - get_pos();
            if (attack_nearest && !attackCurrentOnly ||
                    attackCurrentOnly && a == player::GetMainActor(
                    player::CurrentPlayer())) {
                if (!target || (length(v) < length(target_vec))) {
                    target = a;
                    target_vec = v;
                }
            } else if (!attackCurrentOnly) {            
                target = a;
                target_vec += normalize(v);
            }
        }
    }

    if (!target && int_attrib("gohome")) { 
        // no actors focussed -> return to start position
        target_vec = get_respawnpos()-get_pos();
    }

    double target_dist = length(target_vec);

    if (target_dist > 0.2)
        target_vec.normalize();
    add_force (target_vec * force);

    Actor::think(dtime);
}


/* -------------------- Rotor -------------------- */
namespace
{
    class Rotor : public RotorBase {
        CLONEACTOR(Rotor);
        DECL_TRAITS;
   public:
        Rotor() : RotorBase (traits)
        {
        }
    };
}

ActorTraits Rotor::traits = {
    "ac-rotor",                 // name
    ac_rotor,                   // id
    22.0f/64,                   // radius
    0.8f                        // mass
};


/* -------------------- Spinning top -------------------- */
namespace
{
    class Top : public RotorBase {
        CLONEACTOR(Top);
        DECL_TRAITS;
    public:
        Top() : RotorBase (traits)
        {
        }
    };
}

ActorTraits Top::traits = {
    "ac-top",                   // name
    ac_top,                     // id
    16.0f/64,                   // radius
    0.8f                        // mass
};


/* -------------------- Bug -------------------- */

namespace
{
    class Bug : public Actor {
        CLONEACTOR(Bug);
        DECL_TRAITS;
    public:
        Bug() : Actor(traits) {}
        bool is_flying() const { return true; }
        bool is_dead() const { return false; }
        int get_id() const { return ac_bug; }
    };
}

ActorTraits Bug::traits = {
    "ac-bug",                   // name
    ac_bug,                     // id
    12.0f/64,                   // radius
    0.7f                        // mass
};



/* -------------------- Horse -------------------- */

namespace
{
    class Horse : public Actor {
        CLONEACTOR(Horse);
        DECL_TRAITS;
 
        int get_id() const { return ac_horse; }
        bool is_flying() const { return true; }
        bool is_dead() const { return false; }

        void think (double dtime);

        // Private methods
        void update_target ();
        bool try_target (int idx);

        // Variables
        int m_targetidx;
        V2 m_target;
   public:
        Horse();
    };
}

ActorTraits Horse::traits = {
    "ac-horse",                 // name
    ac_horse,                   // id
    24.0f/64,                   // radius
    1.2f                        // mass
};

Horse::Horse() 
: Actor(traits),
  m_targetidx(-1),
  m_target()
{
    set_attrib("force", Value(10.0));
    set_attrib("target1", Value());
    set_attrib("target2", Value());
    set_attrib("target3", Value());
    set_attrib("target4", Value());
}
 
void Horse::think (double /* dtime */) 
{
    double force = 0;
    double_attrib("force", &force);
    update_target ();
    if (m_targetidx != -1) {
        add_force (normalize(m_target - get_pos()) * force);
    }
}

bool Horse::try_target (int idx) {
    if (idx <= 0 || idx > 4)
        return false;

    const char *attrs[] = {
        "target1", "target2", "target3", "target4",
    };
    string targetstr;
    GridLoc loc;

    if (!string_attrib (attrs[idx-1], &targetstr))
        return false;
    
    to_gridloc(targetstr.c_str(), loc);
    m_target = loc.pos.center();
    m_targetidx = idx;
    return true;
}

void Horse::update_target () {

    if (m_targetidx == -1) {
        // no target defined so far
        try_target(1);
    } 
    else if (length(m_target - get_pos()) < 0.2) {
        // target reached or? try next one
        if (!try_target (m_targetidx + 1))
            m_targetidx = -1;     // failed -> start anew
    } 
}




/* -------------------- CannonBall -------------------- */

namespace
{
    class CannonBall : public Actor {
        CLONEACTOR(CannonBall);
        DECL_TRAITS;
    public:
        CannonBall();
        bool is_flying() const { return true; }
        bool is_dead() const { return false; }
        bool is_on_floor() const { return false; }
        void on_creation(const ecl::V2 &p);
        bool can_move() const { return true; }

        void animcb();
    };
}

ActorTraits CannonBall::traits = {
    "ac-cannonball",            // name
    ac_cannonball,              // id
    24.0/64,                    // radius
    1.0                         // mass
};

CannonBall::CannonBall()
: Actor(traits)
{
    get_actorinfo()->ignore_contacts = true;
}

void CannonBall::animcb() 
{
    const GridPos &p = get_gridpos();

    if (Stone *st = GetStone (p)) {
        SendMessage (st, "spitter");
    }
    else if (Item *it = GetItem(p)) {
        if (!has_flags(it, itf_indestructible))
            SetItem (p, it_explosion3);
    }
    else if (Floor *fl = GetFloor(p)) {
        if (fl->is_destructible())
            SetItem (p, it_explosion3);
    }
    KillActor (this);
}

void CannonBall::on_creation(const ecl::V2 &p) 
{
    Actor::on_creation(p);

    display::SpriteHandle &sprite = get_sprite();
    sprite.kill();
    sprite = display::AddEffect(p, "ac-cannonball");
    sprite.set_callback(this);
}



/* -------------------- BasicBall -------------------- */
namespace
{
    /*! The base class for all marbles. */
    class BasicBall : public Actor {
    protected:
        BasicBall(const ActorTraits &tr);

        enum State {
            NO_STATE,
            NORMAL,
            SHATTERING,
            DROWNING,
            BUBBLING,
            FALLING,            // falling into abyss
            JUMPING,
            DEAD,               // marble is dead
            RESURRECTED,        // has been resurrected; about to respawn
            APPEARING,          // appearing when level starts/after respawn
            DISAPPEARING,       // disappearing when level finished
            FALLING_VORTEX,     // falling into vortex
            RISING_VORTEX,      // appear in vortex
            JUMP_VORTEX,        // jump out of vortex (here player controls actor)
        };

        enum HaloState {
            NOHALO, HALOBLINK, HALONORMAL
        };

        void sink (double dtime);
	void disable_shield();
        void change_state_noshield (State newstate);
        void change_state(State newstate);

        // Model management
        void update_model();
        void set_sink_model(const string &m);
        void set_shine_model (bool shinep);
        void update_halo();
        virtual void hide();

        /* ---------- Actor interface ---------- */
        
        virtual void think (double dtime);
        virtual void move_screen ();

        void on_creation(const ecl::V2 &p);
        void on_respawn (const ecl::V2 &/*pos*/)
        {
            change_state(APPEARING);
        }

        bool is_dead() const;
	bool is_movable() const;
        bool is_flying() const { return state == JUMPING; }
        bool is_on_floor() const;
        bool is_drunken() const { return m_drunk_rest_time>0; }
        bool is_invisible() const { return m_invisible_rest_time>0; }

        bool can_drop_items() const;
        bool can_pickup_items() const;
        bool has_shield() const;

        bool can_be_warped() const { return state==NORMAL; }

        // Object interface.
        virtual Value message(const string &m, const Value &);

        // ModelCallback interface.
        void animcb();

        /* ---------- Variables ---------- */

        State state;            // The marble's current state

        static const int minSinkDepth = 0; // normal level
        int maxSinkDepth;       // actor dies at this depth
        double sinkDepth;       // how deep actor has sunk
        int    sinkModel;       // current model
        bool   lastshinep;

        double vortex_normal_time; // while jumping out of vortex: time at normal level

        display::SpriteHandle m_halosprite;
        double                m_shield_rest_time;
        static const double   SHIELD_TIME;
        HaloState             m_halostate;

        double m_drunk_rest_time;
        double m_invisible_rest_time;
    };

    const double BasicBall::SHIELD_TIME = 10.0;
}

BasicBall::BasicBall(const ActorTraits &tr)
: Actor                 (tr),
  state                 (NO_STATE),
  maxSinkDepth          (7),
  sinkDepth             (minSinkDepth),
  sinkModel             (-1),
  lastshinep            (false),
  vortex_normal_time    (0),
  m_halosprite          (),
  m_shield_rest_time    (0),
  m_halostate           (NOHALO),
  m_drunk_rest_time     (0),
  m_invisible_rest_time (0)
{
}

void BasicBall::on_creation(const ecl::V2 &p) 
{
    Actor::on_creation(p);
    if (server::CreatingPreview)
        change_state(NORMAL);
    else
        change_state(APPEARING);
}


void BasicBall::move_screen ()
{
    update_model();
    update_halo();
    Actor::move_screen();
}

bool BasicBall::is_movable() const 
{
    return (state!=DEAD && state!=RESURRECTED && state!=APPEARING && state!=DISAPPEARING); 
}

bool BasicBall::is_dead() const {
    return state == DEAD;
}

bool BasicBall::is_on_floor() const {
    return state == NORMAL || state == JUMP_VORTEX || state==APPEARING;
}

bool BasicBall::can_drop_items() const {
    return state == NORMAL || state == JUMP_VORTEX || state==JUMPING;
}

bool BasicBall::can_pickup_items() const {
    return state == NORMAL || state == JUMP_VORTEX;
}

void BasicBall::change_state_noshield (State newstate) 
{
    if (!has_shield())
        change_state(newstate);
}

Value BasicBall::message(const string &m, const Value &v) 
{
    switch (state) {
    case NORMAL:
        if (m == "shatter")         change_state_noshield(SHATTERING);
        else if (m == "suicide")    change_state(SHATTERING);
        else if (m == "laserhit")   change_state_noshield(SHATTERING);
        else if (m == "drown")      change_state_noshield(DROWNING);
        else if (m == "fall")       change_state_noshield(FALLING);
        else if (m == "fallvortex") change_state(FALLING_VORTEX);
        else if (m == "jump")       change_state(JUMPING);
        else if (m == "appear")     change_state(APPEARING);
        else if (m == "disappear")  change_state(DISAPPEARING);
        break;
    case JUMPING:
        if (m == "shatter")         change_state_noshield(SHATTERING);
        else if (m == "disappear")  change_state(DISAPPEARING);
        break;
    case DEAD:
        if (m == "resurrect")       change_state(RESURRECTED);
        break;
    case FALLING_VORTEX:
        if (m == "rise")            change_state(RISING_VORTEX); // vortex->vortex teleportation
        else if (m == "appear")     change_state(APPEARING); // vortex->non-vortex teleportation
        break;
    case JUMP_VORTEX:
        if (m == "laserhit")        change_state(SHATTERING);
        break;
    case APPEARING:
        // ugly hack
        if (m == "init")
            Actor::message (m, v);
        else if (m == "shatter")    change_state (SHATTERING);
	break;
    default:
        break;
    }


    // Shield, booze and invisibility can be activated in all states except DEAD

    if (state != DEAD) {
        if (m == "shield") {
            m_shield_rest_time += SHIELD_TIME;
            update_halo();
        }
        else if (m == "invisibility") {
            m_invisible_rest_time += 8.0;
        }
        else if (m == "booze") {
            m_drunk_rest_time += 5.0; // Drunken for 5 more seconds
        }
    }
    return Value();
}

void BasicBall::set_sink_model(const string &m)
{
    int modelnum = ecl::round_down<int>(sinkDepth);

    if (!has_shield() && modelnum != sinkModel) {
        ASSERT(modelnum >= minSinkDepth && modelnum < maxSinkDepth, XLevelRuntime,
            "BasicBall: set_sink_model called though modelnum incorrect");

        string img = m+"-sink";
        img.append(1, static_cast<char>('0'+modelnum));
        set_model(img);

        sinkModel = modelnum;
    }
}

void BasicBall::set_shine_model (bool shinep)
{
    if (shinep != lastshinep) {
        string modelname = get_kind();
        if (shinep)
            set_model (modelname + "-shine");
        else
            set_model (modelname);
        lastshinep = shinep;
    }
}

void BasicBall::update_model()
{
    if (m_invisible_rest_time > 0)
        get_sprite().hide();
    else 
        get_sprite().show();

    switch (state) {
    case NORMAL:
        if (sinkDepth > minSinkDepth && sinkDepth < maxSinkDepth) {
            set_sink_model(get_kind());
        }
        else {
            ActorInfo *ai = get_actorinfo();
            int xpos = ecl::round_nearest<int> (ai->pos[0] * 32.0);
            int ypos = ecl::round_nearest<int> (ai->pos[1] * 32.0);

            bool shinep = ((xpos + ypos) % 2) != 0;
            set_shine_model (shinep);
        }
        break;
    default:
        break;
    }
}

void BasicBall::sink (double dtime)
{
    double sink_speed  = 0.0;
    double raise_speed = 0.0;   // at this velocity don't sink; above: raise

    Item *it = GetItem(get_gridpos());
    Floor *fl = GetFloor (get_gridpos());
    if (!(it != NULL && it->covers_floor(get_pos())) && fl != NULL)
        fl->get_sink_speed (sink_speed, raise_speed);
    
    if (sink_speed == 0.0 || has_shield()) {
        sinkDepth = minSinkDepth;
        sinkModel = -1;
    }
    else {
        ActorInfo *ai = get_actorinfo();
        double sinkSpeed = sink_speed * (1 - length(ai->vel) / raise_speed);
        sinkDepth += sinkSpeed*dtime;

        if (sinkDepth >= maxSinkDepth) {
            set_model(string(get_kind())+"-sunk");
            ai->vel = V2();     // stop!
            sound_event ("swamp");
            change_state(BUBBLING);
        }
        else {
            if (sinkDepth < minSinkDepth) 
                sinkDepth = minSinkDepth;
        }
    }
}

void BasicBall::think (double dtime) 
{
    if (m_invisible_rest_time > 0)
        m_invisible_rest_time -= dtime;

    // Update protection shield
    if (m_shield_rest_time > 0) 
        m_shield_rest_time -= dtime;

    switch (state) {
    case NORMAL: 
        if (m_drunk_rest_time > 0) 
            m_drunk_rest_time -= dtime;
        sink (dtime);
        break;
    case JUMP_VORTEX:
        vortex_normal_time += dtime;
        if (vortex_normal_time > 0.025) // same time as appear animation
            if (vortex_normal_time > dtime) // ensure min. one tick in state JUMP_VORTEX!
                change_state(JUMPING); // end of short control over actor
        break;
    default:

        break;
    }

    Actor::think(dtime);
}

void BasicBall::animcb()
{
    string kind=get_kind();

    switch (state) {
    case SHATTERING:
        set_model(kind+"-shattered");
        change_state(DEAD);
        break;
    case DROWNING:
    case BUBBLING:
        set_model("invisible");
        change_state(DEAD);
        break;
    case FALLING:
        set_model(kind+"-fallen"); // invisible
        if (get_id (this) == ac_meditation)
            sound_event ("shattersmall");
        else
            sound_event ("shatter");
        change_state(DEAD);
        break;
    case JUMPING:
        set_model(kind);
        change_state(NORMAL);
        break;
    case APPEARING:
        set_model(kind);
        change_state(NORMAL);
        break;
    case DISAPPEARING:
        set_model("ring-anim");
        break;
    case FALLING_VORTEX: {
        set_model(kind+"-fallen"); // invisible
        break;
    }
    case RISING_VORTEX: {
        set_model(kind);
        if (Item *it = GetItem(get_gridpos())) {
            world::ItemID id = get_id(it);
            if (id == world::it_vortex_open || id == world::it_vortex_closed) 
                SendMessage(it, "arrival"); // closes some vortex
        }
        change_state(JUMP_VORTEX);
        break;
    }
    default:
        break;
    }
}

void BasicBall::change_state(State newstate) {
    if (newstate == state)
        return;

    string kind     = get_kind();
    State  oldstate = state;

    state = newstate;
    switch (newstate) {
    case NORMAL:
        if (oldstate == APPEARING) {
            ActorInfo *ai = get_actorinfo();
            ai->forceacc = V2();
        }
        world::ReleaseActor(this);
        break;

    case SHATTERING:
        if (get_id (this) == ac_meditation)
            sound_event ("shattersmall");
        else
            sound_event ("shatter");
        world::GrabActor(this);
        set_anim (kind+"-shatter");
        break;

    case DROWNING:
        // @@@ FIXME: use same animation as SINKING ?
        world::GrabActor(this);
//         sound::PlaySound("drown");
        sound_event("drown");
//         set_anim ("ring-anim");
        set_anim ("ac-drowned");
        break;
    case BUBBLING:
        world::GrabActor(this);
//         sound::PlaySound("drown");
        set_anim ("ac-drowned");
        break;
    case FALLING:
    case FALLING_VORTEX:
        world::GrabActor(this);
        set_anim(kind+"-fall");
        break;
    case DEAD: 
        disable_shield();
        m_drunk_rest_time = 0;
        m_invisible_rest_time = 0;
        break;
    case JUMPING:
        sound_event ("jump");
        set_anim(kind+"-jump");
        break;
    case APPEARING:
    case RISING_VORTEX:
        set_anim(kind+"-appear");
        world::GrabActor(this);
        break;
    case JUMP_VORTEX:
        ASSERT(oldstate == RISING_VORTEX, XLevelRuntime,
            "BasicBall: change to state JUMP_VORTEX but not RISING_VORTEX");
        vortex_normal_time = 0;
        set_model(kind);
        world::ReleaseActor(this);
        break;
    case DISAPPEARING:
        world::GrabActor(this);
	disable_shield();
        set_anim(kind+"-disappear");
        break;
    case RESURRECTED:
	disable_shield();
        sinkDepth = minSinkDepth;
	break;
    default:
        break;
    }
}

void BasicBall::disable_shield() {
    if (has_shield()) {
	m_shield_rest_time = 0;
	update_halo();
    }
}

bool BasicBall::has_shield() const {
    return m_shield_rest_time > 0;
}

void BasicBall::update_halo() {
    HaloState newstate = m_halostate;

    double radius = get_actorinfo()->radius;
    string halokind;

    // Determine which halomodel has to be used:
    if (radius == 19.0/64) { // Halo for normal balls
        halokind="halo";
    }else if (radius == 13.0f/64) { // Halo for small balls
        halokind="halo-small";
    }

    if (m_shield_rest_time <= 0)
        newstate = NOHALO;
    else if (m_shield_rest_time <= 3.0)
        newstate = HALOBLINK;
    else
        newstate = HALONORMAL;

    if (newstate != m_halostate) {
	if (m_halostate == NOHALO){
	   m_halosprite = display::AddSprite (get_pos(), halokind.c_str());
    }
        switch (newstate) {
        case NOHALO:
            // remove halo
            m_halosprite.kill();
            m_halosprite = display::SpriteHandle();
            break;
        case HALOBLINK:
            // blink for the last 3 seconds
            m_halosprite.replace_model (display::MakeModel (halokind+"-blink"));
            break;
        case HALONORMAL:
            m_halosprite.replace_model (display::MakeModel (halokind));
            break;
        }
        m_halostate = newstate;
    }
    else if (m_halostate != NOHALO) {
        m_halosprite.move (get_pos());
    }
}

void BasicBall::hide() {
    Actor::hide();
    disable_shield();
}

//----------------------------------------
// Balls of different sorts
//----------------------------------------

namespace
{
    class BlackBall : public BasicBall {
        CLONEACTOR(BlackBall);
        DECL_TRAITS;
    public:
        BlackBall() : BasicBall(traits)
        {
            set_attrib("mouseforce", Value(1.0));
            set_attrib("color", Value(0.0));
            set_attrib("blackball", Value(true));
            set_attrib("player", Value(0.0));
            set_attrib("controllers", Value(1.0));
        }
    };

    class WhiteBall : public BasicBall {
        CLONEACTOR(WhiteBall);
        DECL_TRAITS;
    public:
        WhiteBall() : BasicBall(traits)
        {
            set_attrib("mouseforce", Value(1.0));
            set_attrib("color", Value(1.0));
            set_attrib("whiteball", Value(true));
            set_attrib("player", Value(1.0));
            set_attrib("controllers", Value(2.0));
        }
    };

    class WhiteBall_Small : public BasicBall {
        CLONEACTOR(WhiteBall_Small);
        DECL_TRAITS;
    public:
        WhiteBall_Small() : BasicBall(traits)
        {
            set_attrib("mouseforce", Value(1.0));
            set_attrib("color", Value(1.0));
            set_attrib("whiteball", Value(true));
            set_attrib("controllers", Value(3.0));
            maxSinkDepth = 4;
        }
    };

    class Killerball : public Actor {
        CLONEACTOR(Killerball);
        DECL_TRAITS;
    public:

        Killerball() : Actor (traits)
        {
            set_attrib("mouseforce", Value(2.0));
            set_attrib("color", Value(1.0));
            set_attrib("whiteball", Value(true));
            set_attrib("controllers", Value(3.0));
        }
        bool is_dead() const { return false; }

	void on_collision(Actor *a) {
	    SendMessage(a, "shatter");
	}
    };
}

ActorTraits BlackBall::traits = {
    "ac-blackball",             // name
    ac_blackball,               // id
    19.0/64,                    // radius
    1.0                         // mass
};

ActorTraits WhiteBall::traits = {
    "ac-whiteball",             // name
    ac_whiteball,               // id
    19.0/64,                    // radius
    1.0                         // mass
};

ActorTraits WhiteBall_Small::traits = {
    "ac-whiteball-small",       // name
    ac_meditation,              // id
    13.0f/64,                   // radius
    0.7f                        // mass
};

ActorTraits Killerball::traits = {
    "ac-killerball",            // name
    ac_killerball,              // id
    13.0f/64,                   // radius
    0.7f                        // mass
};


/* -------------------- Functions -------------------- */

void world::InitActors () 
{
    RegisterActor (new Bug);
    RegisterActor (new Horse);
    RegisterActor (new Rotor);
    RegisterActor (new Top);
    RegisterActor (new BlackBall);
    RegisterActor (new WhiteBall);
    RegisterActor (new WhiteBall_Small);
    RegisterActor (new Killerball);
    RegisterActor (new CannonBall);
}
