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
#include "game.hh"
#include "sound.hh"
#include "world.hh"

#include "ecl_util.hh"
#include "ecl_dict.hh"

#include <algorithm>
#include <string>
#include <cstdlib>
#include <cstdarg>
#include <iostream>
#include <iomanip>

using namespace std;
using namespace enigma;
using namespace world;

// remove comment from define below to switch on verbose messaging
// note: VERBOSE_MESSAGES is defined in multiple source files!
// #define VERBOSE_MESSAGES


/* -------------------- Helper routines -------------------- */

namespace
{
    // string_match accepts simple wildcards
    // '?' means 'any character'
    // '*' means '0 or more characters'
    bool string_match(const char *str, const char *templ) {
        while (true) {
            char t = *templ++;
            char s = *str++;

            if (t == s) {
                if (!t) return true;
                continue;
            }
            else { // mismatch
                if (t == '?') continue;
                if (t != '*') break;

                t = *templ++;
                if (!t) return true; // '*' at EOS

                while (1) {
                    if (!s) break;
                    if (s == t) {
                        if (string_match(str, templ))
                            return true;
                    }
                    s = *str++;
                }
            }
        }
        return false;
    }
}


/* -------------------- Object implementation -------------------- */

Object::Object(const char *kind) {
    set_attrib("kind", Value(kind));
}


Value Object::on_message (const world::Message &m)
{
    return message (m.message, m.value);
}

Value Object::message(const string& /*msg*/, const Value &/*val*/)
{
    return Value();
}

void Object::on_levelinit() {
}


const char *
Object::get_kind() const
{
    const Value *v = get_attrib("kind");
    ASSERT(v && v->get_type()==Value::STRING, XLevelRuntime,
        "Object: attribute kind is not of type string (found in get_kind)");
    return v->get_string();
}

// check kind of object
// kind_templ may contain wildcards ( ? and * )
bool Object::is_kind(const char *kind_templ) const {
    return string_match(get_kind(), kind_templ);
}

bool Object::is_kind(const string& kind_templ) const {
    return string_match(get_kind(), kind_templ.c_str());
}

void Object::set_attrib(const string& key, const Value& val) {
    attribs[key] = val;//.insert (key, val);
}

const Value* Object::get_attrib(const string& key) const {
    AttribMap::const_iterator i = attribs.find(key);
    if (i == attribs.end())
        return 0;
    else
        return &i->second;
}

bool Object::string_attrib(const string &name, string *val) const {
    if (const Value *v = get_attrib(name)) {
        if (v->get_type() != Value::NIL) {
            const char *s = to_string(*v);
            if (s != 0) {
                *val = s;
                return true;
            }
        }
    }
    return false;
}

int Object::int_attrib(const string &name) const {
    if (const Value *v = get_attrib(name))
        return to_int(*v);
    return 0;
}

bool Object::int_attrib(const string &name, int *val) const {
    if (const Value *v = get_attrib(name)) {
        *val = to_int(*v);
        return true;
    }
    return false;
}


bool Object::double_attrib(const string &name, double *val) const {
    if (const Value *v = get_attrib(name)) {
        if (v->get_type() != Value::NIL) {
            *val = to_double(*v);
            return true;
        }
    }
    return false;
}

/* Send an impulse to position 'dest' into direction dir.  If 'dest'
   contains a stone, on_impulse() is called for that stone */
void Object::send_impulse(const GridPos& dest, Direction dir) 
{
    if (Stone *st = GetStone(dest)) {
        Impulse impulse(this, dest, dir);
        st->on_impulse(impulse);
    }
}

/* Like variant above, but the _result_ of the impulse is delayed.
 */

void
Object::send_impulse(const GridPos& dest, Direction dir, double delay)
{
    if (Stone *st = GetStone(dest)) {
        addDelayedImpulse(Impulse(this, dest, dir), delay, st);
    }
}

void Object::warning(const char *format, ...) const {
    va_list arg_ptr;

    va_start(arg_ptr, format);

    fprintf(stderr, "%p non-grid-\"%s\": ", this, get_kind());
    vfprintf(stderr, format, arg_ptr);
    fputc('\n', stderr);

    va_end(arg_ptr);
}


/* -------------------- GridObject implementation -------------------- */

display::Model *GridObject::set_anim (const std::string &mname) 
{
    set_model (mname);
    display::Model *m = get_model();
    m->set_callback(this);
    return m;
}

bool GridObject::sound_event (const char *name, double vol)
{
    return sound::EmitSoundEvent (name, get_pos().center(), getVolume(name, this, vol));
}

void GridObject::warning(const char *format, ...) const 
{
    va_list        arg_ptr;
    const GridPos& position = get_pos();

    va_start(arg_ptr, format);

    fprintf(stderr, "%p \"%s\" at %i/%i: ", this, get_kind(), position.x, position.y);
    vfprintf(stderr, format, arg_ptr);
    fputc('\n', stderr);

    va_end(arg_ptr);
}

