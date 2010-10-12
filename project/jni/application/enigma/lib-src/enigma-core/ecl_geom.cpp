/*
 * Copyright (C) 2002,2005 Daniel Heck
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
 */
#include "ecl_geom.hh"
#include <iostream>
#include <list>

using namespace std;
using namespace ecl;

/* -------------------- Rect -------------------- */

std::ostream& ecl::operator<<(std::ostream& os, const Rect& r) {
    return os << "Rect(" << r.x << " " << r.y << " " << r.w << " " << r.h << ")";
}

/* -------------------- RectList -------------------- */

RectList::RectList(const RectList& rl)
: m_rectangles(rl.m_rectangles)
{}

void RectList::swap(RectList &rl) {
    std::swap(m_rectangles, rl.m_rectangles);
}

RectList& RectList::operator=(const RectList& rl) {
    m_rectangles = rl.m_rectangles;
    return *this;
}

void RectList::push_back(const Rect& r) {
    if (r.w > 0 && r.h > 0)
        m_rectangles.push_back(r);
}

void RectList::pop_back() {
    return m_rectangles.pop_back();
}

void RectList::merge(const RectList& rl) {
    for (const_iterator i = rl.begin(); i != rl.end(); ++i)
        add(*i);
}

void RectList::intersect(const Rect& rect) {
    RectList rl;
    for (iterator i=begin(); i != end(); ++i) {
        int a = max(i->x, rect.x);
        int b = min(i->x + i->w, rect.x + rect.w);
        int c = max(i->y, rect.y);
        int d = min(i->y + i->h, rect.y + rect.h);
        rl.push_back(Rect(a, c, b-a, d-c));
    }
    swap(rl);
}

void RectList::add(const Rect& r) {
    RectList rl;
    rl.push_back(r);
    for (iterator i=begin(); i != end(); ++i)
        rl.sub(*i);
    copy(rl.begin(), rl.end(), back_inserter(*this));
}

/* Cut a region out of the rectangle list, i.e., remove all points
   that are _not_ covered by `rect'. */
void RectList::sub(const Rect& rect) {
    RectList rl;
    for (iterator i=begin(); i != end(); ++i) {
        int lft = max(i->x, rect.x);
        int top = max(i->y, rect.y);
        int rgt = min(i->x + i->w, rect.x + rect.w);
        int bot = min(i->y + i->h, rect.y + rect.h);

        if (rgt > lft && bot > top) {
            // the coordinates of the intersection rectangle
            rl.push_back(Rect(i->x, i->y, i->w,  top - i->y));
            rl.push_back(Rect(i->x, top, lft - i->x, bot - top));
            rl.push_back(Rect(rgt, top, i->x + i->w - rgt, bot - top));
            rl.push_back(Rect(i->x, bot, i->w, i->y + i->h - bot));
        } 
        else
            rl.push_back(*i);
    }
    swap(rl);
}
