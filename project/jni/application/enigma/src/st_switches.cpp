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
#include "laser.hh"
#include "player.hh"
#include "server.hh"
#include "Inventory.hh"

#include "stones_internal.hh"

using namespace std;
using namespace world;
using namespace stones;


/* -------------------- Switch -------------------- */

namespace
{
    class SwitchStone : public OnOffStone {
        CLONEOBJ(SwitchStone);
    public:
        SwitchStone() : OnOffStone("st-switch"), state(IDLE) {}
    private:
        enum State { IDLE, TOGGLING } state;

        void init_model() {
            set_model(is_on() ? "st-switch-on" : "st-switch-off");
        }

        void actor_hit(const StoneContact &/*sc*/) {
            set_on (!is_on());
        }

        virtual void set_on(bool newon) {
            if (state == IDLE && newon != is_on()) {
//                set_attrib("on", enigma::Value(newon));
                sound_event ("switchon");
                state = TOGGLING;
                if (newon)
                    set_anim("st-switch-turnon");
                else
                    set_anim("st-switch-turnoff");
                PerformAction(this, newon);
            }
        }

        void animcb() {
            state = IDLE;
            set_attrib("on", enigma::Value(!is_on()));
            init_model();
//            set_on(!is_on());
        }
        const char *collision_sound() { return "metal"; }
    };
}

/* -------------------- Switch_black -------------------- */

namespace
{
    class Switch_black : public OnOffStone {
        CLONEOBJ(Switch_black);
    public:
        Switch_black() : OnOffStone("st-switch_black"), 
                         state(IDLE) 
        {}
    private:
        enum State { IDLE, TOGGLING } state;
        void init_model() {
            set_model(is_on() ? "st-switch_black-on" : "st-switch_black-off");
        }

        void actor_hit(const StoneContact &sc) {
            if (get_id (sc.actor) == world::ac_blackball)
                set_on (!is_on());
        }

        virtual void set_on(bool newon) {
            if (state == IDLE && newon != is_on()) {
//                set_attrib("on", enigma::Value(newon));
                sound_event ("switchon");
                state = TOGGLING;
                if (newon)
                    set_anim("st-switch_black-turnon");
                else
                    set_anim("st-switch_black-turnoff");
                PerformAction(this, newon);
            }
        }
        void animcb() {
            state = IDLE;
            set_attrib("on", enigma::Value(!is_on()));
            init_model();
//            set_on(!is_on());
        }

        const char *collision_sound() { return "metal"; }
    };
}

/* -------------------- Switch_white -------------------- */

namespace
{
    class Switch_white : public OnOffStone {
        CLONEOBJ(Switch_white);
    public:
        Switch_white() : OnOffStone("st-switch_white"), 
                         state(IDLE) 
        {}
    private:
        enum State { IDLE, TOGGLING } state;
        void init_model() {
            set_model(is_on() ? "st-switch_white-on" : "st-switch_white-off");
        }

        void actor_hit(const StoneContact &sc) {
            if (get_id (sc.actor) == world::ac_whiteball)
                set_on (!is_on());
        }

        virtual void set_on(bool newon) {
            if (state == IDLE && newon != is_on()) {
//                set_attrib("on", enigma::Value(newon));
                sound_event ("switchon");
                state = TOGGLING;
                if (newon)
                    set_anim("st-switch_white-turnon");
                else
                    set_anim("st-switch_white-turnoff");
                PerformAction(this, newon);
            }
        }
        void animcb() {
            state = IDLE;
            set_attrib("on", enigma::Value(!is_on()));
            init_model();
        }

        const char *collision_sound() { return "metal"; }
    };
}



/* -------------------- Coin slot -------------------- */

namespace
{
    class CoinSlot : public OnOffStone, public TimeHandler {
        CLONEOBJ(CoinSlot);
    public:
        CoinSlot();
        ~CoinSlot();

    private:
        // Variables.
        enum State { ACTIVE, INACTIVE } state;
        double remaining_time;

        // GridObject interface
        void init_model();
        void animcb();

        // TimeHandler interface
        void tick(double dtime);

        // Stone interface
        void actor_hit(const StoneContact &sc);
        const char *collision_sound() { return "metal"; }

        // Private methods.
        void change_state(State newstate);
    };
}

CoinSlot::CoinSlot()
: OnOffStone("st-coinslot"), state(INACTIVE), remaining_time(0)
{
}

CoinSlot::~CoinSlot() {
    GameTimer.remove_alarm (this);
}


void CoinSlot::init_model() {
    set_model(state==ACTIVE ? "st-coinslot-active" : "st-coinslot");
}


void CoinSlot::change_state(State newstate) {
    if (state == newstate) return;

    switch (newstate) {
    case ACTIVE:
        PerformAction(this, true);
        GameTimer.activate(this);
        break;
    case INACTIVE:
        PerformAction(this, false);
        GameTimer.deactivate(this);
        sound_event ("coinslotoff");
        break;
    }
    state = newstate;
    init_model();
}

void CoinSlot::animcb() {
    change_state(ACTIVE);
    init_model();
}

void CoinSlot::tick(double dtime)
{
    ASSERT(remaining_time > 0, XLevelRuntime, "CoinSlot: tick called, but no remaining time");

    remaining_time -= dtime;
    if (remaining_time <= 0)
        change_state(INACTIVE);
}

void CoinSlot::actor_hit(const StoneContact &sc)
{
    if (enigma::Inventory *inv = player::GetInventory(sc.actor)) {
        if (Item *it = inv->get_item (0)) {
            ItemID id = get_id(it);
            if (id == it_coin1 || id == it_coin2 || id == it_coin4) {
                sound_event ("coinsloton");
                set_anim("st-coin2slot");

                double coin_value = 0;
                it->double_attrib("value", &coin_value);
                remaining_time += coin_value;

                inv->yield_first();
                player::RedrawInventory (inv);
                delete it;
            }
        }
    }
}


/* -------------------- Key switches -------------------- */

// Attributes:
//
// :keycode         a numerical code; only keys with the same code
//                  can activate this switch
// :on              1 or 0
// :target,action   as usual

/** \page st-key Key Switch Stone

This stone acts as a lock and can only be activated by
using a key item. You can use keycodes to let keys only
open specific key stones.

\subsection keye Example
\verbatim
set_stone( "st-key_b", 14,77, {action="openclose", target="door2"})
set_item("it-key_b", 12,7)
\endverbatim

\image html st-key0.png
*/
namespace
{
    class KeyStone : public OnOffStone {
        CLONEOBJ(KeyStone);

        void init_model() {set_model(is_on() ? "st-key1" : "st-key0");}
        void actor_hit(const StoneContact &sc);
        const char *collision_sound() { return "metal"; }
        bool check_matching_key (enigma::Inventory *inv);
    public:
        KeyStone(const char *kind="st-key", int keycode=0) 
        : OnOffStone(kind) 
        {
            set_attrib("keycode", keycode);
        }
    };

    class KeyStone_a : public KeyStone {
        CLONEOBJ(KeyStone_a);
    public:
        KeyStone_a() : KeyStone("st-key_a", 1) {}
    };
    class KeyStone_b : public KeyStone {
        CLONEOBJ(KeyStone_b);
    public:
        KeyStone_b() : KeyStone("st-key_b", 2) {}
    };
    class KeyStone_c : public KeyStone {
        CLONEOBJ(KeyStone_c);
    public:
        KeyStone_c() : KeyStone("st-key_c", 3) {}
    };
}

bool KeyStone::check_matching_key (enigma::Inventory *inv)
{
    Item *it = inv->get_item(0);
    int keycode, my_keycode = int_attrib ("keycode");
    return (it
            && it->is_kind("it-key*")
            && it->int_attrib("keycode", &keycode)
            && my_keycode == keycode);
}

void KeyStone::actor_hit(const StoneContact &sc)
{
    enigma::Inventory *inv = player::GetInventory(sc.actor);
    if (!inv)
        return;

    bool toggle = false;

    if (server::GameCompatibility == enigma::GAMET_ENIGMA) {
        if (is_on()) {
            if (!inv->is_full()) {
                Item *key = MakeItem("it-key");
                key->set_attrib ("keycode", int_attrib ("keycode"));
                inv->add_item(key);
                toggle = true;
            }
        }
        else if (check_matching_key (inv)) {
            DisposeObject (inv->yield_first());
            toggle = true;
        }
        player::RedrawInventory (inv);
    }
    else {
        if (check_matching_key (inv))
            toggle = true;
    }

    if (toggle) {
        set_on (!is_on());
        PerformAction (this, is_on());
    }
}


/* -------------------- FourSwitch -------------------- */

// Attributes:
//
// :on              1 or 0
// :target,action   as usual

namespace
{
    class FourSwitch : public OnOffStone {
        CLONEOBJ(FourSwitch);
    public:
        FourSwitch() : OnOffStone("st-fourswitch"), 
                       m_direction(NORTH), 
                       m_inactive_so_far (true)
        {}
    private:
        // Variables
        Direction m_direction;
        bool m_inactive_so_far;

        // Private methods
        void turn() 
        {
            static int direction2idx[] = { 
                3, // WEST
                2, // SOUTH
                1, // EAST
                0  // NORTH
            };

            if (!m_inactive_so_far) {
                world::EmitSignalByIndex(this, direction2idx[m_direction], 0);
            } else
                m_inactive_so_far = false;

            m_direction = rotate_cw (m_direction);
            init_model();
            set_on(!is_on());
            sound_event ("fourswitch");

            if (world::HaveSignals (this)) {
                world::EmitSignalByIndex(this, direction2idx[m_direction], 1);
            } else {
                // no signal handler defined
                PerformAction(this, is_on());
            }
        }

        void init_model() {
            switch (m_direction) {
            case NORTH: set_model("st-fourswitch-n"); break;
            case EAST:  set_model("st-fourswitch-e"); break;
            case SOUTH: set_model("st-fourswitch-s"); break;
            case WEST:  set_model("st-fourswitch-w"); break;
            case NODIR: ASSERT(0, XLevelRuntime,
                "FourSwitch: no direction defined (found in init_model)");
            }
        }

        void actor_hit(const StoneContact &/*sc*/) {
            turn();
        }

        virtual Value on_message (const Message &m)
        {
            if (m.message == "signal" || m.message == "trigger")
                turn();
            return Value();
        }

        const char *collision_sound() { return "metal"; }
    };
}


/* -------------------- Laser / Time switches -------------------- */

namespace
{
    class LaserTimeSwitchBase : public PhotoStone, public TimeHandler {
    public:
        LaserTimeSwitchBase(const char *kind);
        virtual ~LaserTimeSwitchBase();

    private:
        // LaserTimeSwitchBase interface
        virtual const char *get_active_model() const   = 0;
        virtual const char *get_inactive_model() const = 0;
        virtual double timer_delay() const;

        bool inverse() { return int_attrib("inverse") == 1; }

        // Stone interface
        void on_creation (GridPos p);
        void on_removal (GridPos p);
        const char *collision_sound() { return "metal"; }

        // PhotoStone interface.
        void notify_laseron();
        void notify_laseroff();

        // TimeHandler interface
        void alarm();

    protected:
        // variables :
        enum State { IDLE, LIGHTED, TOUCHED };
        State state;

        void change_state(State newstate);
    };


    class LaserSwitch : public LaserTimeSwitchBase {
        CLONEOBJ(LaserSwitch);
    public:
        LaserSwitch();
    private:
        // LaserTimeSwitchBase interface
        const char *get_active_model() const;
        const char *get_inactive_model() const;
    };


    class LaserTimeSwitch : public LaserTimeSwitchBase {
        CLONEOBJ(LaserTimeSwitch);
    public:
        LaserTimeSwitch(const char *kind = "st-lasertimeswitch");
    private:
        // LaserTimeSwitchBase interface
        const char *get_active_model() const;
        const char *get_inactive_model() const;
        double timer_delay() const;

        // Stone interface
        void actor_hit(const StoneContact &sc);
    };

    class TimeSwitch : public LaserTimeSwitch {
        CLONEOBJ(TimeSwitch);
    public:
        TimeSwitch();
    private:
        // ignore laser:
        void notify_laseron();
        void notify_laseroff();
    };
}


LaserTimeSwitchBase::LaserTimeSwitchBase(const char *kind)
: PhotoStone(kind) , state(IDLE) 
{}

LaserTimeSwitchBase::~LaserTimeSwitchBase() {
    GameTimer.remove_alarm (this);
}

void LaserTimeSwitchBase::change_state(State newstate) {
    if (state == newstate)
        return;

    if (state == IDLE) {
//         sound_event ("st-switch");
        set_model(get_active_model());
        PerformAction(this, !inverse());
        if (newstate == TOUCHED) {
            double delay = timer_delay();
            ASSERT(delay>0.0, XLevelRuntime, "LaserTimeSwitchBase: delay non-positive");
            GameTimer.set_alarm(this, delay, false);
        }
    }
    else if (newstate == IDLE) {
//         sound_event ("st-switch");
        set_model(get_inactive_model());
        PerformAction(this, inverse());
    }
    else {
        // it's not allowed to switch from LIGHTED to TOUCHED
       ASSERT(!(state == LIGHTED && newstate == TOUCHED), XLevelRuntime,
           "LaserTimeSwitchBase: trying to switch from lighted to touched");
    }
    state = newstate;
}

void LaserTimeSwitchBase::on_creation(GridPos) 
{
    set_model(get_inactive_model());
    photo_activate();
}

void LaserTimeSwitchBase::on_removal(GridPos)
{
    photo_deactivate();
}

void LaserTimeSwitchBase::notify_laseron()  {
    if (state != LIGHTED)
        change_state(LIGHTED);
}

void LaserTimeSwitchBase::notify_laseroff() {
    if (state == LIGHTED)
        change_state(IDLE);
}

void LaserTimeSwitchBase::alarm() {
    if (state == TOUCHED)
        change_state(IDLE);
}

double LaserTimeSwitchBase::timer_delay() const {
    return -1; // we have no timer delay
}

/* ---------- LaserSwitch ---------- */

LaserSwitch::LaserSwitch()
: LaserTimeSwitchBase("st-laserswitch")
{}

const char *LaserSwitch::get_active_model() const { 
    return "st-laserswitch1"; 
}

const char *LaserSwitch::get_inactive_model() const { 
    return "st-laserswitch0"; 
}

/* ---------- LaserTimeSwitch ---------- */

LaserTimeSwitch::LaserTimeSwitch(const char *kind)
    : LaserTimeSwitchBase(kind)
{
    set_attrib("delay", 1.8);
}

const char *LaserTimeSwitch::get_active_model() const { 
    return "st-time1switch"; 
}

const char *LaserTimeSwitch::get_inactive_model() const { 
    return "st-timeswitch"; 
}

double LaserTimeSwitch::timer_delay() const {
    double delay;
    if (!double_attrib("delay", &delay))
        ASSERT(0, XLevelRuntime, "LaserTimeSwitch: delay not properly defined");
    return delay;
}

void LaserTimeSwitch::actor_hit(const StoneContact &sc) {
    if (sc.actor && state == IDLE)
        change_state(TOUCHED);
}

/* ---------- TimeSwitch ---------- */

// Attributes:
//
// :on              1 or 0
// :target,action   as usual

TimeSwitch::TimeSwitch()
: LaserTimeSwitch("st-timeswitch")
{
}

void TimeSwitch::notify_laseron()  {} // ignore laser
void TimeSwitch::notify_laseroff() {}


/* -------------------- Floppy switch -------------------- */

// Attributes:
//
// :on              1 or 0
// :target,action   as usual
namespace
{
    class FloppyStone : public OnOffStone {
        CLONEOBJ(FloppyStone);
    public:
        FloppyStone() : OnOffStone("st-floppy") {}
    private:
        // Stone interface
        void init_model();
        void actor_hit(const StoneContact &sc);
        const char *collision_sound() { return "metal"; }
    };
}

void FloppyStone::init_model() 
{
    set_model(is_on() ? "st-floppy1" : "st-floppy0");
}

void FloppyStone::actor_hit (const StoneContact &sc)
{
    if (enigma::Inventory *inv = player::GetInventory(sc.actor)) {
        if (is_on()) {
            if (!inv->is_full()) {
                inv->add_item (MakeItem("it-floppy"));
                set_on(false);
                PerformAction(this, is_on());
            }
        }
        else if (player::WieldedItemIs (sc.actor, "it-floppy")) {
            DisposeObject (inv->yield_first());
            set_on(true);
            PerformAction(this, is_on());
        }
        player::RedrawInventory (inv);
    }
}


/* -------------------- Functions -------------------- */

void InitSwitches()
{
    Register (new CoinSlot);
    Register (new FloppyStone);
    Register (new FourSwitch);
    Register (new KeyStone);
    Register (new KeyStone_a);
    Register (new KeyStone_b);
    Register (new KeyStone_c);
    Register (new LaserSwitch);
    Register (new LaserTimeSwitch);
    Register (new SwitchStone);
    Register (new Switch_black);
    Register (new Switch_white);
    Register (new TimeSwitch);
}
