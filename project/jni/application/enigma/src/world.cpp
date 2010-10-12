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
#include "laser.hh"
#include "player.hh"
#include "sound.hh"
#include "options.hh"
#include "server.hh"
#include "lua.hh"
#include "client.hh"
#include "main.hh"
#include "stones_internal.hh"

#include <iostream>
#include <algorithm>
#include <functional>
#include <map>
#include <numeric>



// remove comment from define below to switch on verbose messaging
// note: VERBOSE_MESSAGES is defined in multiple source files!
// #define VERBOSE_MESSAGES

using namespace std;
using namespace world;
using namespace ecl;

#include "world_internal.hh"

// Size of one time step -- do not change!
const double ActorTimeStep = 0.0025;


/* -------------------- Auxiliary functions -------------------- */

namespace {

    /*! Enigma's stones have rounded corners; this leads to realistic
      behaviour when a marble rebounds from the corner of a single
      stone. But it's annoying when there are two adjacent stones and
      a marble rebounds from any one (or even both) corners, because
      it changes the direction of the marble quite unpredictably.

      This function modifies the contact information for two adjacent
      stones in such a way that the two collisions are treated as a
      single collision with a flat surface. */
    void maybe_join_contacts (StoneContact &a, StoneContact &b)
    {
//         double maxd = 4.0/32;   // Max distance between joinable collisions

        if (a.is_contact && b.is_contact
            && a.is_collision && b.is_collision 
            && a.response==STONE_REBOUND && b.response==STONE_REBOUND)
            // && length(a.contact_point - b.contact_point) <= maxd)
        {
            b.ignore = true; // Don't rebound from `b'

            DirectionBits fa = contact_faces(a);
            DirectionBits fb = contact_faces(b);

            switch (fa & fb) {
            case NORTHBIT: a.normal = V2(0,-1); break;
            case EASTBIT:  a.normal = V2(1,0); break;
            case SOUTHBIT: a.normal = V2(0,1); break;
            case WESTBIT:  a.normal = V2(-1,0); break;
            case NODIRBIT:
                //fprintf(stderr, "Strange: contacts have no direction in common\n");
                break;
            default:
                //fprintf(stderr, "Strange: contacts have multiple directions in common\n");
                break;
            }
        }
    }

/*! Find an already existing contact point in the ContactList that is
  similar to the second argument. */
    bool has_nearby_contact (const ContactList &cl, const Contact &c)
    {
        double posdelta = 0.2;
        double normaldelta = 0.1;
        for (size_t i=0; i<cl.size(); ++i) {
            if (length (cl[i].pos - c.pos) < posdelta
                && length (cl[i].normal - c.normal) < normaldelta)
                return true;
        }
        return false;
    }
}


/* -------------------- ActorInfo -------------------- */

ActorInfo::ActorInfo()
: pos(), vel(), forceacc(),
  charge(0), mass(1), radius(1),
  grabbed(false), ignore_contacts (false),
  last_pos(), oldpos(), force(),
  contacts(), new_contacts()
{}



/* -------------------- Messages -------------------- */

Message::Message ()
{
}
 
Message::Message (const std::string &message_,
                  const enigma::Value &value_,
                  GridPos from_)
: message (message_),
  value (value_),
  gridpos (from_)
{
}


/* -------------------- Signals -------------------- */

namespace
{

    void emit_signal (const Signal *s, int value)
    {
        Object *src = s->source;
        Object *dst = GetObject(s->destloc);

#if defined(VERBOSE_MESSAGES)
        src->warning("emit_from: msg='%s'", // dest=%i/%i obj=%p",
                     s->message.c_str()
//                      destloc.pos.x, destloc.pos.y,
//                      dst);
                     );
#endif
        if (GridObject *go = dynamic_cast<GridObject*>(src))
            SendMessage (dst, Message (s->message, value, go->get_pos()));
        else
            SendMessage (dst, Message (s->message, value));
    }

    void emit_from (const SignalList &sl, Object *source, int value)
    {
        size_t size = sl.size();
        for (unsigned i=0; i<size; ++i) {
            if (sl[i].source == source)
                emit_signal (&sl[i], value);
        }
//         // signals may have side effects. To minimize them
//         //   1. collect all targets and then
//         //   2. emit signals to targets

//         vector<Object*> targets;

//         for (unsigned i=0; i<size; ++i) {
//             if (m_signals[i].get_source() == source)
//                 target.push_back (m_signals[i]);

//             for (unsigned i=0; i<size; ++i)
//                 if (GetObject(m_signals[i].get_target_loc()) == targets[i])
//                     m_signals[i].emit_from (source, value);
//         }
    }

    bool emit_by_index (const SignalList &sl, Object *source, int signalidx, int value) 
    {
        size_t size      = sl.size();
        int    signalcnt = 0;
        for (unsigned i=0; i<size; ++i) {
            if (sl[i].source == source) {
                if (signalcnt == signalidx) {
                    emit_signal (&sl[i], value);
                    return true;
                }
                signalcnt += 1;
            }
        }
        return false;
    }

    Object *find_single_destination (const SignalList &sl, Object *src)
    {
        Object *found = 0;
        size_t  size  = sl.size();

        for (unsigned i = 0; i<size; ++i) {
            if (sl[i].source == src) {
                if (Object *candidate = GetObject(sl[i].destloc)) {
                    if (!found)
                        found = candidate;
                    else if (candidate != found)
                        return 0;   // multiple targets
                }
            }
        }
        return found;
    }
}


/* -------------------- RubberBandData -------------------- */

RubberBandData::RubberBandData () {
    strength = 1;
    length = 2;
    minlength = 0;
}

RubberBandData::RubberBandData (const RubberBandData &x) {
    strength  = x.strength;
    length    = x.length;
    minlength = x.minlength;
}

/* -------------------- RubberBand -------------------- */

RubberBand::RubberBand (Actor *a1, Actor *a2, const RubberBandData &d)
: actor(a1), actor2(a2), stone(0),
  model(display::AddRubber(get_p1(),get_p2())),
  data (d)
{
    ASSERT(actor, XLevelRuntime, "RubberBand: no actor defined");
    ASSERT(d.length >= 0, XLevelRuntime, "RubberBand: length negative");
}

RubberBand::RubberBand (Actor *a1, Stone *st, const RubberBandData &d)
: actor(a1), actor2(0), stone(st), model(0),
  data (d)
{
    ASSERT(actor, XLevelRuntime, "RubberBand: no actor defined");
    ASSERT(d.length >= 0, XLevelRuntime, "RubberBand: length negative");
    model = display::AddRubber(get_p1(), get_p2());
}

RubberBand::~RubberBand() {
    model.kill();
}

void RubberBand::apply_forces ()
{
    V2 v = get_p2()-get_p1();
    double vv = ecl::length(v);

    if (vv > data.length) {
        V2 force = v * data.strength*(vv-data.length)/vv;
        force /= 6;
        actor->add_force(force);
        if (actor2)
            actor2->add_force(-force);
    } 
    else if (vv < data.minlength) {
        V2 force = v * data.strength * (vv-data.minlength) / vv;
        force /= 6;
        actor->add_force(force);
        if (actor2)
            actor2->add_force(-force);
    }

}

V2 RubberBand::get_p1() const
{
    return V2(actor->get_pos()[0], actor->get_pos()[1]);
}

V2 RubberBand::get_p2() const
{
    if (!stone)
        return V2(actor2->get_pos()[0], actor2->get_pos()[1]);
    else
        return stone->get_pos().center();
}

void RubberBand::tick (double /* dtime */) 
{
    model.update_first (get_p1());
    model.update_second (get_p2());
}


/* -------------------- Field -------------------- */

Field::Field()
{
    floor=0;
    item=0;
    stone=0;
}

Field::~Field()
{
    DisposeObject(floor);
    DisposeObject(item);
    DisposeObject(stone);
}


/* -------------------- StoneContact -------------------- */

StoneContact::StoneContact(Actor *a, GridPos p,
                           const V2 &cp, const V2 &n)
: actor(a), stonepos(p),
  response(STONE_PASS),
  contact_point(cp),
  normal(n),
  is_collision(false),
  ignore (false),
  new_collision(false),
  is_contact(true)
{}

StoneContact::StoneContact()
: is_collision(false),
  ignore (false),
  new_collision(false),
  is_contact(false)
{}

DirectionBits
world::contact_faces(const StoneContact &sc)
{
    using namespace enigma;

    int dirs = NODIRBIT;

    if (sc.normal[0] < 0)
        dirs |= WESTBIT;
    else if (sc.normal[0] > 0)
        dirs |= EASTBIT;
    if (sc.normal[1] < 0)
        dirs |= NORTHBIT;
    else if (sc.normal[1] > 0)
        dirs |= SOUTHBIT;

    return DirectionBits(dirs);
}

Direction
world::contact_face(const StoneContact &sc)
{
    using namespace enigma;
    if (sc.normal == V2(-1,0))
        return WEST;
    else if (sc.normal == V2(1,0))
        return EAST;
    else if (sc.normal == V2(0,-1))
        return NORTH;
    else if (sc.normal == V2(0,1))
        return SOUTH;
    else
        return NODIR;
}


/* -------------------- Global variables -------------------- */

namespace
{
    auto_ptr<World> level;
}

enigma::Timer  world::GameTimer;
bool           world::TrackMessages;
Actor         *world::CurrentCollisionActor = 0;



/* -------------------- Layer implementation -------------------- */

template <class T>
T *Layer<T>::get(GridPos p) {
    if (Field *f=level->get_field(p))
        return raw_get(*f);
    else
        return defaultval;
}

template <class T>
T *Layer<T>::yield(GridPos p) {
    if (Field *f=level->get_field(p)) {
        T *x = raw_get(*f);
        if (x) {
            raw_set(*f, 0);
            x->removal(p);
        }
        return x;
    } else
        return defaultval;
}

template <class T>
void Layer<T>::set(GridPos p, T *x) {
    if (x) {
        if (Field *f=level->get_field(p)) {
            if (T *old = raw_get (*f)) {
                old->removal(p);
                dispose (old);
            }
            raw_set(*f, x);
            x->creation(p);
        }
        else
            dispose(x);
    }
}




/* -------------------- World -------------------- */

World::World(int ww, int hh) 
: fields(ww,hh),
  preparing_level(true)
{
    w = ww;
    h = hh;

    scrambleIntensity = server::GetDifficulty() == DIFFICULTY_EASY ? 3 : 10;
}

World::~World()
{
    fields = FieldArray(0,0);
    for_each(actorlist.begin(), actorlist.end(), mem_fun(&Actor::dispose));
    delete_sequence (m_rubberbands.begin(), m_rubberbands.end());
}

bool World::is_border (GridPos p) {
    return(p.x==0 || p.y==0 || p.x==w-1 || p.y==h-1);
}

void World::remove (ForceField *ff)
{
    ForceList::iterator i=find (forces.begin(), forces.end(), ff);
    if (i != forces.end())
        forces.erase(i);
}

Object *World::get_named (const string &name)
{
    ecl::Dict<Object*>::iterator found = m_objnames.find(name);
    if (found != m_objnames.end()) 
        return found->second;
    Log << "Did not find named object: " << name << '\n';
    return 0;
}

void World::name_object (Object *obj, const std::string &name)
{
    m_objnames.insert(name, obj); // [name] = obj;
    obj->set_attrib("name", name);
}

void World::unname (Object *obj)
{
    ASSERT(obj, XLevelRuntime, "unname: no object given");
    string name;
    if (obj->string_attrib("name", &name)) {
        m_objnames.remove(name);
        obj->set_attrib("name", "");
    }
}

void World::add_actor (Actor *a)
{
    add_actor (a, a->get_pos());
}

void World::add_actor (Actor *a, const V2 &pos)
{
    actorlist.push_back(a);
    a->get_actorinfo()->pos = pos;
    if (!preparing_level) {
        // if game is already running, call on_creation() from here
        a->on_creation(pos);
    }
}

void World::tick (double dtime)
{
    // dtime is always 0.01 (cf. server.cc)

    move_actors (dtime);
    handle_delayed_impulses (dtime);
    tick_sound_dampings();

    // Tell floors and items about new stones.
    for (unsigned i=0; i<changed_stones.size(); ++i)
        stone_change(changed_stones[i]);
    changed_stones.clear();

    m_mouseforce.tick (dtime);
    for_each (forces.begin(), forces.end(),
              bind2nd(mem_fun(&ForceField::tick), dtime));

    GameTimer.tick(dtime);

    lasers::RecalcLightNow();   // recalculate laser beams if necessary
}

/* ---------- Puzzle scrambling -------------------- */

void World::add_scramble(GridPos p, Direction dir)
{
    scrambles.push_back(Scramble(p, dir, scrambleIntensity));
}

void World::scramble_puzzles()
{
    while (!scrambles.empty()) {
        list<Scramble>::iterator i = scrambles.begin();
        list<Scramble>::iterator e = scrambles.end();

        for (; i != e; ++i) {
            Stone *puzz = GetStone(i->pos);
            if (puzz && i->intensity) {
                SendMessage(puzz, "scramble", Value(double(i->dir)));
                --i->intensity;
            }
            else {
                fprintf(stderr, "no stone found for scramble at %i/%i\n", i->pos.x, i->pos.y);
                i->intensity = 0;
            }
        }

        scrambles.remove_if(mem_fun_ref(&Scramble::expired));
    }
}

//----------------------------------------------------------------------
// Physics simulation
//----------------------------------------------------------------------


/* -------------------- Force calculation -------------------- */

#ifndef M_PI
#define M_PI 3.1415926535
#endif

void World::add_mouseforce (Actor *a, Floor *floor, V2 &mforce)
{
    if (a->get_controllers() != 0) {
        V2 f = floor->process_mouseforce(a, m_mouseforce.get_force(a));
        if (a->is_drunken()) {
            // rotate mouse force by random angle
            double maxangle = M_PI * 0.7;
            double angle = DoubleRand (-maxangle, maxangle);
            f = V2(f[0]*cos(angle) - f[1]*sin(angle),
                   f[0]*sin(angle) + f[1]*cos(angle));
        }
        mforce += f;
    }
}

/*! Calculate the total force on an actor A at time TIME.  The
  actor's current position X and velocity V are also passed.  [Note
  that the position and velocity entries in ActorInfo will be updated
  only after a *successful* time step, so they cannot be used
  here.] */
V2 World::get_local_force (Actor *a)
{
    V2 f;

    if (a->is_on_floor()) {
        const Field *field = GetField (a->get_gridpos());
        if (Floor *floor = field->floor) {
            // Constant force
            m_flatforce.add_force(a, f);

            // Mouse force
            add_mouseforce (a, floor, f);

            // Friction
            double friction = floor->friction();
            if (a->has_spikes())
                friction += 7.0;

            V2 v = a->get_vel();
            double vv=length(v);
            if (vv > 0) {
                V2 frictionf = v * (server::FrictionFactor*friction);
                frictionf /= vv;
                frictionf *= pow(vv, 0.8);
                f -= frictionf;
            }

            floor->add_force(a, f);
        }

        if (Item *item = field->item) 
            item->add_force(a, f);
    }

    return f;
}

/* Global forces are calculated less often than local ones, namely
   only once every four time steps, cf. move_actors().  They are used
   for forces that are more time consuming to calculate, i.e.,
   actor-actor interactions and external force fields. */
V2 World::get_global_force (Actor *a)
{
    V2 f;
    // Electrostatic forces between actors.
    if (double q = get_charge(a)) {
        for (ActorList::iterator i=actorlist.begin();
             i != actorlist.end(); ++i)
        {
            Actor *a2 = *i;
            if (a2 == a) continue;
            if (double q2 = get_charge(a2)) {
                V2 distv = a->get_pos() - a2->get_pos();
                if (double dist = distv.normalize())
                    f += server::ElectricForce * q * q2 / (dist) * distv;
            }
        }
    }

    // All other force fields.
    for (ForceList::iterator i=forces.begin(); i != forces.end(); ++i) 
        (*i)->add_force(a, f);

    return f;
}

/* -------------------- Collision handling -------------------- */

struct Ball {
    ecl::V2 c;                   // center
    double r;                   // radius
};

struct Oblong {
    ecl::V2 c;                   // center
    double w;                   // width
    double h;                   // height
    double erad;                // edge radius
};

// static void contact_ball_oblong ()
// {
// }

/* Determine whether an actor is in contact with a stone at position
   `p'.  The result is returned in `c'.  Three situations can occur:

   1) There is no stone at `p'. In this case, `is_contact' is set to
      false and nothing else is done.

   2) The stone and the actor are in contact. In this case, `c' is
      filled with the contact information and `is_contact'p is set to
      true.

   3) The stone and the actor are _not_ in contact.  In this case, `c'
      is filled is filled with information about the closest feature
      on the stone and `is_contact' is set to false. 
*/
void World::find_contact_with_stone (Actor *a, GridPos p, StoneContact &c) 
{
    c.is_contact = false;

    const ActorInfo &ai = *a->get_actorinfo();
    double r = ai.radius;
//     Ball b (a->get_pos(), ai.radius);
//     Oblong o (p.center(), 1, 1, 2.0 / 32);

    V2 centerdist = a->get_pos() - p.center();
    if  (square (centerdist) > 1.0)
        return;

    Stone *stone = world::GetStone(p);
    if (!stone)
        return;

    int x = p.x, y = p.y;

    double ax = ai.pos[0];
    double ay = ai.pos[1];
    const double contact_e = 0.02;
    const double erad = 2.0/32; // edge radius

    // Closest feature == north or south face of the stone?
    if (ax>x+erad && ax<x+1-erad) {
        double dist = r+5;

        // south
        if (ay>y+1) {
            c.contact_point = V2(ax, y+1);
            c.normal        = V2(0,+1);
            dist            = ay-(y+1);
        }
        // north
        else if (ay<y) {
            c.contact_point = V2(ax, y);
            c.normal        = V2(0,-1);
            dist            = y-ay;
        }
        c.is_contact = (dist-r < contact_e);
    }
    // Closest feature == west or east face of the stone?
    else if (ay>y+erad && ay<y+1-erad) {
        double dist=r+5;
        if (ax>x+1) { // east
            c.contact_point = V2(x+1,ay);
            c.normal        = V2(+1,0);
            dist            = ax-(x+1);
        }
        else if (ax<x) { // west
            c.contact_point = V2(x,ay);
            c.normal        = V2(-1,0);
            dist            = x-ax;
        }
	c.is_contact = (dist-r < contact_e);
    }
    // Closest feature == any of the four corners
    else {
        int xcorner=(ax >= x+1-erad);
        int ycorner=(ay >= y+1-erad);
        double cx[2] = {erad, -erad};

        V2 corner(x+xcorner+cx[xcorner], y+ycorner+cx[ycorner]);
        V2 b=V2(ax,ay) - corner;

        c.is_contact    = (length(b)-r-erad < contact_e);
        c.normal        = normalize(b);
        c.contact_point = corner + c.normal*erad;
    }

    if (c.is_contact) {
        // treat this as a collision only if actor not inside the stone
        // and velocity towards stone
        if (ax >= x && ax < x+1 && ay >= y && ay < y+1)
            c.is_collision = false;
        else
            c.is_collision  = c.normal*ai.vel < 0;

        c.ignore   = false;
        c.actor    = a;
        c.stonepos = p;
	c.stoneid  = stone->get_traits().id;
        c.response = stone->collision_response(c);
        c.sound    = stone->collision_sound();
    }
}

void World::find_stone_contacts (Actor *a, StoneContactList &cl)
{
    ActorInfo &ai = *a->get_actorinfo();
    ecl::Rect r(round_down<int>(ai.pos[0]-0.5), round_down<int>(ai.pos[1]-0.5), 1, 1);

    static StoneContact contacts[2][2];

    // Test for collisions with the nearby stones
    int ncontacts = 0;
    for (int i=r.w; i>=0; --i) {
        for (int j=r.h; j>=0; --j) {
            GridPos p (r.x + i, r.y + j);
            find_contact_with_stone(a, p, contacts[i][j]);
            ncontacts += contacts[i][j].is_contact;
        }
    }
    if (ncontacts > 0) {
        maybe_join_contacts (contacts[0][0], contacts[1][0]);
        maybe_join_contacts (contacts[0][0], contacts[0][1]);
        maybe_join_contacts (contacts[1][0], contacts[1][1]);
        maybe_join_contacts (contacts[0][1], contacts[1][1]);

        for (int i=0; i<=r.w; i++)
            for (int j=0; j<=r.h; j++)
                if (contacts[i][j].is_contact)
                    cl.push_back(contacts[i][j]);
    }
}

/*! This function is called for every contact between an actor and a
  stone.  It handles: (1) informing stones about contacts, (2)
  collision response, (3) updating the actor's contact list.  Note
  that a stone may kill itself during a call to actor_hit() or
  actor_contact() so we must not refer to it after having called these
  functions.  (Incidentally, this is why StoneContact only contains a
  'stonepos' and no 'stone' entry.) */
void World::handle_stone_contact (StoneContact &sc) 
{
    Actor     *a           = sc.actor;
    ActorInfo &ai          = *a->get_actorinfo();
    double     restitution = 1.0; //0.85;

    if (server::NoCollisions  && (sc.stoneid != st_borderstone))
        return;

    Contact contact (sc.contact_point, sc.normal);
    
    if (sc.is_contact && sc.response == STONE_REBOUND) {
        ai.new_contacts.push_back (contact);
    }
    
    if (sc.is_collision) {
        if (!sc.ignore && sc.response == STONE_REBOUND) {
            bool slow_collision = length (ai.vel) < 0.3;
            if (!has_nearby_contact(ai.contacts, contact)) {
                if (Stone *stone = world::GetStone(sc.stonepos)) {
                    CurrentCollisionActor = a;
                    if (slow_collision) stone->actor_touch(sc);
                    else stone->actor_hit(sc);
                    CurrentCollisionActor = 0;

                    if (!slow_collision) {
                        client::Msg_Sparkle (sc.contact_point);
                        double volume = std::max (0.25, length(ai.vel)/8);
                        volume = std::min (1.0, volume);
                        volume = getVolume(sc.sound.c_str(), a, volume);
                        sound::EmitSoundEvent (sc.sound.c_str(), sc.contact_point, volume);
                    }
                }
            }

            double dt = ActorTimeStep;
            ai.collforce -= (1 + restitution)*(ai.vel*sc.normal)*sc.normal / dt * ai.mass;
        }
    }
    else if (sc.is_contact) {
        if (Stone *stone = world::GetStone(sc.stonepos))
            stone->actor_contact(sc.actor);
    }
}

namespace {
    struct ActorEntry {
        double pos;
        size_t idx;

        ActorEntry () { pos = 0; idx = 0; }
        ActorEntry (double pos_, size_t idx_) { pos = pos_; idx = idx_; }

        bool operator < (const ActorEntry &x) const {
            return pos < x.pos;
        }
    };
};

void World::handle_actor_contacts () {
    size_t nactors = actorlist.size();
    vector<ActorEntry> xlist (nactors);

    for (size_t i=0; i<nactors; ++i) {
        ActorInfo *ai = actorlist[i]->get_actorinfo();
        xlist[i] = ActorEntry (ai->pos[0], i);
    }
    sort (xlist.begin(), xlist.end());
    for (size_t i=0; i<nactors; ++i) {
        size_t a1 = xlist[i].idx;
        ActorInfo &ai1 = *actorlist[a1]->get_actorinfo();
        double r1 = ai1.radius;
        double x1 = ai1.pos[0];
        for (size_t j=i+1; j<nactors; ++j) {
            size_t a2 = xlist[j].idx;
            if (xlist[j].pos - x1 < r1 + get_radius(actorlist[a2])) 
                handle_actor_contact (a1, a2);
            else
                break;
        }
    }
}

void World::handle_actor_contact (size_t i, size_t j)
{
    Actor *actor1 = actorlist[i];
    ActorInfo &a1 = *actor1->get_actorinfo();
    Actor *actor2 = actorlist[j];
    ActorInfo &a2 = *actor2->get_actorinfo();

    if (a1.ignore_contacts || a2.ignore_contacts)
        return;

    V2 n = a1.pos - a2.pos; // normal to contact surface
    double dist = n.normalize();
    double overlap = a1.radius + a2.radius - dist;
    if (overlap > 0 && !a2.grabbed) {
        double relspeed = (a2.vel-a1.vel)*n;

        if (relspeed < 0)   // not moving towards each other
            return;

        actor1->on_collision (actor2);
        actor2->on_collision (actor1);

        bool reboundp = (actor1->is_movable() && actor2->is_movable() &&
                         (actor1->is_on_floor() == actor2->is_on_floor()));

        if (reboundp) {
            Contact contact (a2.pos + n*a2.radius, -n);
            a2.new_contacts.push_back (contact);
            contact.normal = n;
            a1.new_contacts.push_back (contact);

            double restitution = 1.0; //0.95;
            double mu = a1.mass*a2.mass / (a1.mass + a2.mass); // reduced mass

            V2 force = (restitution * 2 * mu * relspeed / ActorTimeStep) * n;
            a1.collforce += force;
            a2.collforce -= force;

            if (!has_nearby_contact (a1.contacts, contact)) {
                double volume = length (force) * ActorTimeStep;
                volume = std::min(1.0, volume);
                if (volume > 0.4) {
                    volume = getVolume("ballcollision", NULL, volume);
                    sound::EmitSoundEvent ("ballcollision", contact.pos, volume);
                }
            }
        }
    }
}

void World::handle_contacts (unsigned actoridx) 
{
    Actor *actor1 = actorlist[actoridx];
    ActorInfo &a1 = *actor1->get_actorinfo();

    if (a1.ignore_contacts)     // used by the cannonball for example
        return;

    // Handle contacts with stones
    StoneContactList cl;
    find_stone_contacts(actor1, cl);
    for (StoneContactList::iterator i=cl.begin(); i != cl.end(); ++i)
        handle_stone_contact (*i);
}

/* -------------------- Actor Motion -------------------- */

void World::move_actors (double dtime) 
{
    const double dt = ActorTimeStep;

    static double rest_time = 0;
    rest_time += dtime;

    size_t nactors = actorlist.size();
    vector<V2> global_forces (nactors);
    for (unsigned i=0; i<nactors; ++i) {
        Actor *a = actorlist[i];
        global_forces[i] = get_global_force (a);
    }

    while (rest_time > 0) {
        for (unsigned i=0; i<nactors; ++i) {
            Actor *a = actorlist[i];
            ActorInfo &ai = *a->get_actorinfo();

            // the "6" is a historical accident, don't change it!
            ai.force     = ai.forceacc * 6;
            ai.force += global_forces[i]; 
            ai.force += get_local_force (a);

            ai.forceacc  = V2();
            ai.collforce = V2();
            ai.last_pos  = ai.pos;
            ai.new_contacts.clear();
        }
        
        handle_actor_contacts();
        for (unsigned i=0; i<nactors; ++i) 
            handle_contacts (i);

        for (unsigned i=0; i<nactors; ++i) {
            Actor     *a  = actorlist[i];
            ActorInfo &ai = * a->get_actorinfo();

            swap (ai.contacts, ai.new_contacts);

            if (!a->can_move()) {
                if (length(ai.force) > 30)
                    client::Msg_Sparkle (ai.pos);
                ai.vel = V2();
            }
            else if (!a->is_dead() && a->is_movable() && !ai.grabbed) {
                advance_actor(a, dt);
            }
            a->move ();         // 'move' nevertheless, to pick up items etc
            a->think (dt); 
        }
        for_each (m_rubberbands.begin(), m_rubberbands.end(), 
                  mem_fun(&RubberBand::apply_forces));

        rest_time -= dt;
    }
}

/* This function performs one step in the numerical integration of an
   actor's equation of motion.  TIME ist the current absolute time and
   H the size of the integration step. */
void World::advance_actor (Actor *a, double dtime)
{
    const double MAXVEL = 70;

    ActorInfo &ai = *a->get_actorinfo();
    V2 force = ai.force;

    // If the actor is currently in contact with other objects, remove
    // the force components in the direction of these objects.
    for (unsigned i=0; i<ai.contacts.size(); ++i)
    {
        const V2 &normal = ai.contacts[i].normal;
        double normal_component = normal * force;
        if (normal_component < 0) {
            force -= normal_component * normal;
        }
    }
    force += ai.collforce;

    ai.vel += dtime * force / ai.mass;
    ai.pos += dtime * ai.vel;

    // Limit maximum velocity
    double q = length(ai.vel) / MAXVEL;
    if (q > 1)
        ai.vel /= q;
}



void World::handle_delayed_impulses (double dtime)
{
    // Handle delayed impulses
    ImpulseList::iterator i = delayed_impulses.begin(),
        end = delayed_impulses.end();
    while (i != end) {
        // shall the impulse take effect now ?
        if (i->tick(dtime)) {
            i->mark_referenced(true);
            if (Stone *st = GetStone(i->destination()))
                i->send_impulse(st);  // may delete stones and revoke delayed impuleses!
            i = delayed_impulses.erase(i);
        }
        else
            ++i;
    }
}

void World::revoke_delayed_impulses(const Stone *target) {
    // Revokes delayed impulses to and from target
    ImpulseList::iterator i = delayed_impulses.begin(),
        end = delayed_impulses.end();
    while (i != end) {
        if (i->is_receiver(target) || i->is_sender(target)) {
            if (i->is_referenced()) {
                i->mark_obsolete();
                ++i;
            } else {
                i = delayed_impulses.erase(i);
            }
        } else {
            ++i;
        }
    }
}

void World::tick_sound_dampings ()
{
    // See sound.hh and sound.cc for details.
    static int counter = 0;
    ++counter;

    if (counter > 9) {
        counter = 0;
        SoundDampingList::iterator i = level->sound_dampings.begin(),
            end = level->sound_dampings.end();
        int count = 0;
        while (i != end) {
            if(i->tick()) // return true if damping entry should be deleted
                i = level->sound_dampings.erase(i);
            else
                ++i;
        }
    }
}

void World::stone_change(GridPos p) 
{
    if (const Field *f = GetField (p)) {
        Stone *st = f->stone;
        if (st)
            st->on_floor_change();

        if (Item *it = f->item)
            it->stone_change(st);

        if (Floor *fl = f->floor)
            fl->stone_change(st);

        lasers::MaybeRecalcLight(p);
    }
}




/* -------------------- Functions -------------------- */

void world::Resize (int w, int h)
{
    level.reset (new World(w,h));
    display::NewWorld(w, h);
}

void world::PrepareLevel ()
{
    GameTimer.clear();
    CurrentCollisionActor = 0;
    Resize (20, 13);
}

bool world::InitWorld()
{
    level->scramble_puzzles();

    lasers::RecalcLight();
    lasers::RecalcLightNow();    // recalculate laser beams if necessary

    bool seen_player0 = false;

    for (ActorList::iterator i=level->actorlist.begin();
         i != level->actorlist.end(); ++i)
    {
        Actor *a = *i;
        a->on_creation(a->get_actorinfo()->pos);
        a->message ("init", Value());

        int iplayer;
        if (a->int_attrib("player", &iplayer)) {
            player::AddActor(iplayer,a);
            if (iplayer == 0) seen_player0 = true;
        } else {
            player::AddUnassignedActor(a);
        }
    }

    level->changed_stones.clear();

    if (!seen_player0) 
        throw XLevelLoading("Error: No player 0 defined!");

    world::BroadcastMessage("init", Value(),
        GridLayerBits(GRID_ITEMS_BIT | GRID_STONES_BIT | GRID_FLOOR_BIT));

    server::InitMoveCounter();
    STATUSBAR->show_move_counter (server::ShowMoves);

    display::FocusReferencePoint();

    level->preparing_level = false;

    return true;
}

void world::SetMouseForce(V2 f)
{
    level->m_mouseforce.add_force(f);
}

void world::NameObject(Object *obj, const std::string &name)
{
    string old_name;
    if (obj->string_attrib("name", &old_name)) {
        obj->warning("name '%s' overwritten by '%s'",
                     old_name.c_str(), name.c_str());
        UnnameObject(obj);
    }
    level->name_object (obj, name);
}

void world::UnnameObject(Object *obj)
{
    level->unname(obj);
}

void world::TransferObjectName (Object *source, Object *target)
{
    string name;
    if (source->string_attrib("name", &name)) {
        UnnameObject(source);
        string targetName;
        if (target->string_attrib("name", &targetName)) {
            target->warning("name '%s' overwritten by '%s'",
                            targetName.c_str(), name.c_str());
            UnnameObject(target);
        }
        NameObject(target, name);
    }
}

Object * world::GetNamedObject (const std::string &name)
{
    return level->get_named (name);
}

bool world::IsLevelBorder(GridPos p)
{
    return level->is_border(p);
}

bool world::IsInsideLevel(GridPos p)
{
    return level->contains(p);
}


/* -------------------- Force fields -------------------- */

void world::AddForceField(ForceField *ff)
{
    level->forces.push_back(ff);
}

void world::RemoveForceField(ForceField *ff) {
    level->remove (ff);
}

void world::SetConstantForce (V2 force) {
    level->m_flatforce.set_force(force);
}



/* -------------------- Rubber bands -------------------- */

void world::AddRubberBand (Actor *a, Stone *st, const RubberBandData &d)
{
    level->m_rubberbands.push_back(new RubberBand (a, st, d));
}

void world::AddRubberBand (Actor *a, Actor *a2, const RubberBandData &d)
{
    RubberBandData rbd (d);
    rbd.length = ecl::Max (d.length, get_radius(a) + get_radius(a2));
    level->m_rubberbands.push_back(new RubberBand (a, a2, rbd));
}

bool world::KillRubberBands (Actor *a)
{
    bool didKill = false;
    for (unsigned i=0; i<level->m_rubberbands.size(); ) {
        RubberBand &r = *level->m_rubberbands[i];
        if (r.get_actor() == a || r.get_actor2() == a) {
            delete &r;
            level->m_rubberbands.erase(level->m_rubberbands.begin()+i);
            didKill = true;
            continue;       // don't increment i
        }
        ++i;
    }
    return didKill;
}


void world::KillRubberBand (Actor *a, Stone *st)
{
    ASSERT(a, XLevelRuntime, "KillRubberBand: no actor attached");
    for (unsigned i=0; i<level->m_rubberbands.size(); ) {
        RubberBand &r = *level->m_rubberbands[i];
        if (r.get_actor() == a && r.get_stone() != 0)
            if (r.get_stone()==st || st==0) {
                delete &r;
                level->m_rubberbands.erase(level->m_rubberbands.begin()+i);
                continue;       // don't increment i
            }
        ++i;
    }
}

void world::KillRubberBand (Actor *a, Actor *a2)
{
    ASSERT(a, XLevelRuntime, "KillRubberBand: no actor attached");
    for (unsigned i=0; i<level->m_rubberbands.size(); ) {
        RubberBand &r = *level->m_rubberbands[i];
        if (r.get_actor() == a && r.get_actor2() != 0)
            if (r.get_actor2()==a2 || a2==0) {
                delete &r;
                level->m_rubberbands.erase(level->m_rubberbands.begin()+i);
                continue;       // don't increment i
            }
        ++i;
    }
}

void world::KillRubberBands (Stone *st)
{
   for (unsigned i=0; i<level->m_rubberbands.size(); ) {
        RubberBand &r = *level->m_rubberbands[i];
        if (r.get_stone() != 0 && r.get_stone()==st) {
            delete &r;
            level->m_rubberbands.erase(level->m_rubberbands.begin()+i);
            continue;       // don't increment i
        }
        ++i;
    }
}

void world::GiveRubberBands (Stone *st, vector<Rubber_Band_Info> &rubs) {
   for (unsigned i=0; i<level->m_rubberbands.size(); ) {
        RubberBand &r = *level->m_rubberbands[i];
        if (r.get_stone() == st) {
            Rubber_Band_Info rbi;
            rbi.act = r.get_actor();
            rbi.data = r.get_data();
            rubs.push_back(rbi);
        }
        ++i;
    }
}

bool world::HasRubberBand (Actor *a, Stone *st)
{
    for (unsigned i=0; i<level->m_rubberbands.size(); ++i) {
        RubberBand &r = *level->m_rubberbands[i];
        if (r.get_actor() == a && r.get_stone() == st)
            return true;
    }
    return false;
}

 
/* -------------------- Signals -------------------- */

void world::AddSignal (const GridLoc &srcloc, 
                       const GridLoc &dstloc, 
                       const string &msg)
{
#if defined(VERBOSE_MESSAGES)
    fprintf(stderr, "AddSignal src=%i/%i dest=%i/%i msg='%s'\n",
            srcloc.pos.x, srcloc.pos.y, dstloc.pos.x, dstloc.pos.y, msg.c_str());
#endif // VERBOSE_MESSAGES

    if (Object *src = GetObject(srcloc)) {
        src->set_attrib("action", "signal");
        level->m_signals.push_back (Signal (src, dstloc, msg));
    }
    else {
        Log << "AddSignal: Invalid signal source\n";
    }
}

bool world::HaveSignals (Object *src) 
{
    SignalList::const_iterator i=level->m_signals.begin(),
        end = level->m_signals.end();
    for (; i != end; ++i) 
        if (i->source == src) 
            return true;
    return false;
}


bool world::EmitSignalByIndex (Object *src, int signalidx, int value) 
{
    return emit_by_index (level->m_signals, src, signalidx, value);
}

bool world::GetSignalTargetPos (Object *src, GridPos &pos, int signalidx) 
{
    SignalList::const_iterator i = level->m_signals.begin(),
        end = level->m_signals.end();
    int idx = 0;
    for (; i != end; ++i) {
        if (i->source == src) {
            if (idx == signalidx) {
                pos = i->destloc.pos;
                return true;
            }
            idx += 1;
        }
    }
    return false;
}


Value world::SendMessage(Object *o, const std::string &msg) 
{
    return SendMessage (o, Message (msg, Value()));
}

Value world::SendMessage(Object *o, const std::string &msg, const Value& value)
{
    return SendMessage (o, Message (msg, value));
}

Value world::SendMessage (Object *o, const Message &m)
{
    if (o) {
        if (TrackMessages)
            o->warning("will be sent message '%s' (with Value)", m.message.c_str());
        return o->on_message(m);
    }
    else if (TrackMessages) {
        fprintf(stderr, "Sending message '%s' to NULL-object\n", m.message.c_str());
        return Value();
    }
    return Value();
}


void world::BroadcastMessage (const std::string& msg, 
                              const Value& value, 
                              GridLayerBits grids)
{
    int  width     = level->w;
    int  height    = level->h;
    bool to_floors = (grids & GRID_FLOOR_BIT) != 0;
    bool to_items  = (grids & GRID_ITEMS_BIT) != 0;
    bool to_stones = (grids & GRID_STONES_BIT) != 0;

    for (int y = 0; y<height; ++y) {
        for (int x = 0; x<width; ++x) {
            GridPos p(x, y);
            Field *f = level->get_field(p);
            if (to_floors && f->floor) SendMessage (f->floor, msg, value);
            if (to_items && f->item)  SendMessage (f->item,  msg, value);
            if (to_stones && f->stone) SendMessage (f->stone, msg, value);
        }
    }
}


void world::PerformAction (Object *o, bool onoff) 
{
    string action = "idle";
    string target;

    o->string_attrib("action", &action);
    o->string_attrib("target", &target);

#if defined(VERBOSE_MESSAGES)
    o->warning("PerformAction action=%s target=%s", action.c_str(), target.c_str());
#endif // VERBOSE_MESSAGES

    if (action == "callback") {
        if (lua::CallFunc(lua::LevelState(), target.c_str(), Value(onoff), o) != 0) {
            throw XLevelRuntime(string("callback '")+target+"' failed:\n"+lua::LastError(lua::LevelState()));
        }
    }
    else if (action == "signal") {
        emit_from (level->m_signals, o, onoff);
    }
    else if (Object *t = GetNamedObject(target)) {
        if (GridObject *go = dynamic_cast<GridObject*>(o))
            SendMessage (t, Message (action, Value(onoff), go->get_pos()));
        else
            SendMessage (t, Message (action, Value(onoff)));
    }
    else if (action != "idle") {
        fprintf (stderr, "Unknown target '%s' for action '%s'\n",
                 target.c_str(), action.c_str());
    }
}


namespace
{
    void explosion (GridPos p, ItemID explosion_item)
    {
        if (Stone *stone = GetStone(p))
            SendMessage(stone, "expl");
        if (Item  *item  = GetItem(p)) {
            if (has_flags(item, itf_indestructible))
                SendMessage(item, "expl");
            else
                SetItem(p, explosion_item);
        }
        else
            SetItem(p, explosion_item);
        if (Floor *floor = GetFloor(p))
            SendMessage(floor, "expl");
    }
}

void world::SendExplosionEffect(GridPos center, ExplosionType type) 
{
    const int AFFECTED_FIELDS       = 8;

    for (int a = 0; a<AFFECTED_FIELDS; ++a) {
        GridPos  dest            = get_neighbour (center, a+1);
        Item    *item            = GetItem (dest);
        Stone   *stone           = GetStone (dest);
        Floor   *floor           = GetFloor (dest);
        bool     direct_neighbor = a<4;

        switch (type) {
        case EXPLOSION_DYNAMITE:
            if (stone) SendMessage(stone, "ignite");
            if (item) SendMessage(item, "ignite");
            if (floor) SendMessage(floor, "ignite");
            break;

        case EXPLOSION_BLACKBOMB:
            if (direct_neighbor) {
                explosion (dest, it_explosion1);
            } else {
                // Note: should not ignite in non-enigma-mode!
                if (stone) SendMessage(stone, "ignite");
                if (item) SendMessage(item, "ignite");
                if (floor) SendMessage(floor, "ignite");
            }
            break;

        case EXPLOSION_WHITEBOMB:
            // Note: at least in oxyd1 only direct neighbors
            // explode, and the others not even ignite
            explosion (dest, it_explosion3);            
            break;

        case EXPLOSION_BOMBSTONE:
            if (direct_neighbor) {
                if (stone) SendMessage(stone, "bombstone");
                if (item) SendMessage(item, "bombstone");
                if (floor) SendMessage(floor, "bombstone");
            }
            break;

        case EXPLOSION_SPITTER:
            // TODO: spitter explosions
            break;
        }
    }
}

Object *world::GetObject (const GridLoc &l)
{
    switch (l.layer) {
    case GRID_FLOOR:  return GetFloor(l.pos);
    case GRID_ITEMS:  return GetItem(l.pos);
    case GRID_STONES: return GetStone(l.pos);
    default: return 0;
    }
}

const Field *world::GetField (GridPos p)
{
    return level->get_field(p);
}


/* -------------------- Floor manipulation -------------------- */

void world::KillFloor(GridPos p) 
{
    level->fl_layer.kill(p);
}

Floor *world::GetFloor(GridPos p) 
{
    return level->fl_layer.get(p);
}

void world::SetFloor(GridPos p, Floor* fl) 
{
    level->fl_layer.set(p,fl);
    if (!level->preparing_level)
        if (Stone *st = GetStone(p))
            st->on_floor_change();
}


/* -------------------- Stone manipulation -------------------- */

Stone * world::GetStone(GridPos p) {
    if (Field *f = level->get_field (p))
        return f->stone;
    else
        return level->st_layer.get(p);
}

void world::KillStone(GridPos p) {
    level->st_layer.kill(p);
    level->changed_stones.push_back(p);
}

Stone * world::YieldStone(GridPos p) {
    Stone *st = level->st_layer.yield(p);
    level->changed_stones.push_back(p);
    return st;
}

void world::SetStone(GridPos p, Stone* st) {
    level->st_layer.set(p,st);
    level->changed_stones.push_back(p);
}

void world::ReplaceStone (GridPos p, Stone* st) {
    Stone *old = level->st_layer.get(p);
    if (old) {
        TransferObjectName(old, st);
        level->st_layer.kill(p);
    }
    SetStone(p, st);
}

void world::MoveStone (GridPos oldPos, GridPos newPos) {
    SetStone(newPos, YieldStone(oldPos));
}

void world::SetScrambleIntensity (int intensity) {
    level->scrambleIntensity = intensity;
}

void world::AddScramble(GridPos p, Direction d) {
    level->add_scramble(p, d);
}


/* -------------------- Item manipulation -------------------- */

void world::KillItem(GridPos p) 
{
    lasers::MaybeRecalcLight(p);
    level->it_layer.kill(p);
}

Item *world::GetItem(GridPos p) {
    return level->it_layer.get(p);
}

Item *world::YieldItem(GridPos p) {
    lasers::MaybeRecalcLight(p);
    return level->it_layer.yield(p);
}

void world::SetItem (GridPos p, Item* it) 
{
    lasers::MaybeRecalcLight(p);
    level->it_layer.set(p,it);
}

void world::SetItem (GridPos p, ItemID id) 
{
    SetItem (p, MakeItem (id));
}


/* -------------------- Actor manipulation -------------------- */

void world::AddActor(double x, double y, Actor* a) 
{
    level->add_actor (a, V2(x, y));
}

void world::AddActor (Actor *a)
{
    level->add_actor (a);
}

Actor * world::YieldActor(Actor *a) 
{
    ActorList::iterator i=find(level->actorlist.begin(), level->actorlist.end(), a);
    if (i != level->actorlist.end()) {
        level->actorlist.erase(i);
        GrabActor(a);
        return a;
    }
    return 0;
}

void world::KillActor (Actor *a) {
    delete YieldActor (a);
}

void world::WarpActor(Actor *a, double newx, double newy, bool keep_velocity)
{
    V2 newpos = V2(newx, newy);
    ASSERT(IsInsideLevel(GridPos(newpos)), XLevelRuntime,
        "WarpActor: Tried to warp actor out of level grid. (And hyperspace travel is not implemented yet, sorry.)");
    if (!keep_velocity)
        a->get_actorinfo()->vel = V2();
    a->warp(newpos);
}

void world::FastRespawnActor(Actor *a, bool keep_velocity) {
    a->find_respawnpos();
    const V2& p = a->get_respawnpos();
    WarpActor(a, p[0], p[1], keep_velocity);
}


void world::RespawnActor(Actor *a) {
    a->find_respawnpos();
    a->respawn();
}

Actor *FindActorByID (ActorID id)
{
    ActorList::iterator i = level->actorlist.begin(),
        end = level->actorlist.end();
    for (; i != end; ++i) {
        Actor *a = *i;
        if (get_id(a) == id)
            return a;
    }
    return 0;
}

unsigned world::CountActorsOfKind (ActorID id) 
{
    unsigned count = 0;
    ActorList::iterator i = level->actorlist.begin(),
        end = level->actorlist.end();
    for (; i != end; ++i) {
        Actor *a = *i;
        if (get_id(a) == id)
            ++count;
    }
    return count;
}

Actor *world::FindOtherMarble(Actor *thisMarble) 
{
    if (!thisMarble) 
        return 0;

    switch (get_id(thisMarble)) {
    case ac_blackball: return FindActorByID (ac_whiteball);
    case ac_whiteball: return FindActorByID (ac_blackball);
    default:
        return 0;
    }
}

bool world::ExchangeMarbles(Actor *marble1) {
    Actor *marble2 = FindOtherMarble(marble1);
    if (marble2) {
        ActorInfo *info1 = marble1->get_actorinfo();
        ActorInfo *info2 = marble2->get_actorinfo();

        swap(info1->pos, info2->pos);
        swap(info1->oldpos, info2->oldpos);

        return true;
    }
    return false;
}


void world::GrabActor(Actor *a)
{
    a->get_actorinfo()->grabbed = true;
}

void world::ReleaseActor(Actor *a)
{
    a->get_actorinfo()->grabbed = false;
}

bool world::GetActorsInRange (ecl::V2 center, double range,
                              vector<Actor*>& actors)
{
    ActorList &al = level->actorlist;
    for (ActorList::iterator i=al.begin(); i!=al.end(); ++i) {
        Actor *a = *i;
        if (length(a->get_pos() - center) < range)
            actors.push_back(a);
    }
    return !actors.empty();
}

bool world::GetActorsInsideField (const GridPos& pos, vector<Actor*>& actors)
{
    ActorList &al = level->actorlist;
    for (ActorList::iterator i=al.begin(); i!=al.end(); ++i) {
        Actor *a = *i;
        if (a->get_gridpos() == pos)
            actors.push_back(a);
    }
    return !actors.empty();
}

void world::ShatterActorsInsideField (const GridPos &p)
{
    vector<Actor *> actors;
    GetActorsInsideField (p, actors);
    vector<Actor *>::iterator i=actors.begin(),
        end = actors.end();
    for (; i != end; ++i) 
        SendMessage(*i, "shatter");
}


/* -------------------- Functions -------------------- */

void world::addDelayedImpulse (const Impulse& impulse, double delay, 
                               const Stone *estimated_receiver) 
{
    // @@@ FIXME: is a special handling necessary if several impulses hit same destination ?

    level->delayed_impulses.push_back(DelayedImpulse(impulse, delay, estimated_receiver));
}

void world::revokeDelayedImpulses(const Stone *target) {
    // Any stone may call this function on deletion.
    // When the repository shuts down no world is existing thus check
    // world first.
    if (level.get() != NULL) 
        level->revoke_delayed_impulses(target);
}

float world::getVolume(const char *name, Object *obj, float def_volume)
{
    // See sound.hh and sound.cc for details.
    SoundDampingList::iterator i = level->sound_dampings.begin(),
        end = level->sound_dampings.end();
    while (i != end) {
        if (i->is_equal(name, obj))
            return i->get_volume(def_volume);
        ++i;
    }
    // No entry found for this object. Create a new one.
    level->sound_dampings.push_back(sound::SoundDamping(name, obj));
    return def_volume;
}

void world::Tick(double dtime) {
    level->tick (dtime);
}

void world::TickFinished () {
    for (unsigned i=0; i<level->actorlist.size(); ++i) {
        level->actorlist[i]->move_screen();
    }

    // 
    for (unsigned i=0; i<level->m_rubberbands.size();++i) 
        level->m_rubberbands[i]->tick (0.0);
}

void world::Init()
{
    InitActors();
    lasers::Init();
    InitItems();
    stones::Init();
    InitFloors();
}

void world::Shutdown()
{
    level.reset();
    Repos_Shutdown();
}


/* -------------------- Object repository -------------------- */
namespace
{
    class ObjectRepos : public ecl::Nocopy {
    public:
        ObjectRepos();
        ~ObjectRepos();
        void add_templ(Object *o);
        void add_templ (const string &name, Object *o);
        bool has_templ(const string &name);
        Object *make(const string &name);
        Object *get_template(const string &name);

        void dump_info();
    private:
        typedef std::map<string, Object*> ObjectMap;
        ObjectMap objmap;           // repository of object templates
        int stonecount, floorcount, itemcount;
    };
}

ObjectRepos::ObjectRepos() {
    stonecount = floorcount = itemcount = 0;
}

ObjectRepos::~ObjectRepos()
{
    ecl::delete_map(objmap.begin(), objmap.end());
}


void ObjectRepos::add_templ (const string &kind, Object *o)
{
    if (has_templ(kind))
        enigma::Log << "add_templ: redefinition of object `" <<kind<< "'.\n";
    else
        objmap[kind] = o;
}

void ObjectRepos::add_templ (Object *o) {
    string kind = o->get_kind();
    if (has_templ(kind))
        enigma::Log << "add_templ: redefinition of object `" <<kind<< "'.\n";
    else
        objmap[kind] = o;
}

bool ObjectRepos::has_templ(const string &name) {
    return objmap.find(name) != objmap.end();
}

Object * ObjectRepos::make(const string &name) {
    ObjectMap::iterator i=objmap.find(name);
    if (i==objmap.end())
        return 0;
    else
        return i->second->clone();
}

Object * ObjectRepos::get_template(const string &name) {
    if (objmap.find (name) != objmap.end())
        return objmap[name];
    else
        return 0;
}

/* Generate a list of all available objects and their attributes. */
void ObjectRepos::dump_info() 
{
    ObjectMap::iterator iter = objmap.begin();
    for (; iter != objmap.end(); ++iter) {
        cout << iter->first << "( ";
        Object *obj = iter->second;
        const Object::AttribMap &a = obj->get_attribs();
        for (Object::AttribMap::const_iterator j=a.begin(); j!=a.end(); ++j)
        {
            if (j->first != "kind")
                cout << j->first << " ";
        }
        cout << ")\n";
    }
}


namespace
{
    ObjectRepos *repos;
    vector<Actor *> actor_repos(ac_COUNT);
    vector<Stone *> stone_repos(st_COUNT);
    vector<Item *> item_repos(it_COUNT);
}

void world::Register (const string &kind, Object *obj) {
    if (!repos)
        repos = new ObjectRepos;
    if (kind.empty())
        repos->add_templ(obj->get_kind(), obj);
    else
        repos->add_templ(kind, obj);
}


void world::Register (Object *obj) {
    Register (obj->get_kind(), obj);
}

void world::Register (const string &kind, Floor *obj)
{
    Object *o = obj;
    Register(kind, o);
}

void world::Register (const string &kind, Stone *obj) {
    Object *o = obj;
    Register(kind, o);
}

void world::RegisterStone (Stone *stone) 
{
    Register(static_cast<Object*>(stone));
    StoneID id = get_id(stone);
    ASSERT (id != st_INVALID, XLevelRuntime,
        "RegisterStone: trying to register with invalid ID");
    stone_repos[id] = stone;
}

void world::RegisterActor (Actor *actor) 
{
    Register(static_cast<Object*>(actor));
    ActorID id = get_id(actor);
    ASSERT (id != ac_INVALID, XLevelRuntime,
        "RegisterActor: trying to register with invalid ID");
    actor_repos[id] = actor;
}

void world::Repos_Shutdown() {
    delete repos;
}

Object * world::MakeObject(const char *kind) {
    static Object *last_templ = 0;
    static string last_kind;

    if (last_kind != kind) {
        last_kind = kind;
        last_templ = repos->get_template(kind);
    }

    Object *o = 0;
    if (last_templ)
        o=last_templ->clone();
    if (!o)
        fprintf(stderr, "MakeObject: unknown object name `%s'\n",kind);
    return o;
}

Object * world::GetObjectTemplate(const std::string &kind) {
    if (!repos->has_templ(kind)) {
        cerr << "GetObjectTemplate: unknown object name `" <<kind<< "'.\n";
        return 0;
    } else
        return repos->get_template(kind);
}

Floor* world::MakeFloor(const char *kind) {
    return dynamic_cast<Floor*>(MakeObject(kind));
}

Stone * world::MakeStone (const char *kind) {
    return dynamic_cast<Stone*>(MakeObject(kind));
}

Actor * world::MakeActor (const char *kind) {
    return dynamic_cast<Actor*>(MakeObject(kind));
}

Actor *world::MakeActor (ActorID id) 
{
    if (Actor *ac = actor_repos[id])
        return ac->clone();
    else
        ASSERT(0, XLevelRuntime, "MakeActor: no actor for ID defined");
    return 0;
}

Stone *world::MakeStone (StoneID id) 
{
    if (Stone *st = stone_repos[id])
        return st->clone();
    else
        ASSERT(0, XLevelRuntime, "MakeStone: no stone for ID defined");
    return 0;
}


void world::DisposeObject(Object *o) {
    if (o != 0) {
        UnnameObject(o);
        o->dispose();
    }
}

void world::DefineSimpleFloor(const std::string &kind, double friction,
                              double mousefactor, bool burnable,
                              const std::string &firetransform)
{
    Register(new Floor(kind.c_str(), friction, mousefactor,
             flf_default, burnable ? flft_burnable : flft_default,
             firetransform.c_str(), ""));
}

void world::DumpObjectInfo() {
    repos->dump_info();
}

/* ------------------- Item repository ------------------- */

void world::Register (const string &kind, Item *obj) 
{
    Object *o = obj;
    world::Register(kind, o);
}

void world::RegisterItem (Item *item) 
{
    Register(static_cast<Object*>(item));
    ItemID id = get_id(item);
    ASSERT(id != it_INVALID, XLevelRuntime,
        "RegisterItem: trying to register with invalid ID");
    item_repos[id] = item;
}

Item *world::MakeItem (ItemID id) 
{
    if (Item *it = item_repos[id])
        return it->clone();
    else
        ASSERT(0, XLevelRuntime, "MakeItem: no item for ID defined");
    return 0;
}

Item * world::MakeItem(const char *kind) {
    return dynamic_cast<Item*>(MakeObject(kind));
}

