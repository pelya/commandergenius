/*
 * Copyright (C) 2002,2003 Daniel Heck
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
#ifndef ECL_GEOM_HH
#define ECL_GEOM_HH

#include "ecl_util.hh"

#include <iosfwd>
#include <vector>

namespace ecl
{

/* -------------------- Generic rectangles -------------------- */

    template <class T>
    class TRect {
    public:
        T x, y, w, h;

        TRect() { x=y=w=h=0; }
        TRect (T xx, T yy, T ww, T hh) { x=xx; y=yy; w=ww; h=hh; }
        void move(T xx, T yy) { x = xx; y = yy; }

        void resize (T ww, T hh) { w = ww; h = hh; }

        void assign (T xx, T yy, T ww, T hh) {
            x = xx; y = yy;
            w = ww; h = hh;
        }

        void intersect(const TRect<T> & r)
        {
            T x1 = Max(x, r.x);
            T y1 = Max(y, r.y);
            T x2 = Min(x+w, r.x+r.w);
            T y2 = Min(y+h, r.y+r.h);
            assign(x1, y1, Max(x2-x1, 0), Max(y2-y1,0));
        }

        bool contains(T xx, T yy) const {
            return (xx >= x && xx < x+w && yy >= y && yy<y+h);
        }

        bool overlaps(const TRect<T>& r) const {
            T x1 = Max(x, r.x);
            T y1 = Max(y, r.y);
            T x2 = Min(x+w, r.x+r.w);
            T y2 = Min(y+h, r.y+r.h);

            return !(x2 <= x1 || y2 <= y1);
        }

        bool empty() const { return !(w > 0 && h > 0); }

        bool operator==(const TRect<T> &r) {
            return (x==r.x && y==r.y && w==r.w && h==r.h);
        }
    };

/* -------------------- Helper routines -------------------- */

    /*! Center one rectangle inside another. */
    template <class T>
    TRect<T> center(const TRect<T> &outer, const TRect<T> &inner)
    {
        return TRect<T>(outer.x + (outer.w-inner.w)/2,
                        outer.y + (outer.h-inner.h)/2,
                        inner.w, inner.h);
    }

    /*! Construct the bounding box for two rectangles */
    template <class T>
    TRect<T> boundingbox(const TRect<T> & r1, const TRect<T>& r2)
    {
        T x1 = Min(r1.x, r2.x);
        T y1 = Min(r1.y, r2.y);
        T x2 = Max(r1.x+r1.w, r2.x+r2.w);
        T y2 = Max(r1.y+r1.h, r2.y+r2.h);
        return TRect<T> (x1,y1, x2-x1, y2-y1);
    }

    /*! Intersect two rectangles */
    template <class T>
    TRect<T> intersect(const TRect<T> & r1, const TRect<T>& r2)
    {
        TRect<T> retval(r1);
        retval.intersect(r2);
        return retval;
    }

    template <class T>
    TRect<T> larger(const TRect<T> &r, T amount) {
        return TRect<T>(r.x-amount, r.y-amount, r.w+2*amount, r.h+2*amount);
    }

    template <class T>
    TRect<T> smaller(const TRect<T> &r, T amount) {
        return larger(r, -amount);
    }

/* -------------------- Integer rectangles -------------------- */

    typedef TRect<int> Rect;

    std::ostream& operator<<(std::ostream& os, const Rect& r);

/* -------------------- Rectangle list -------------------- */

    class RectList {
    public:

        typedef Rect                               value_type;
        typedef value_type                        &reference;
        typedef const value_type                  &const_reference;
        typedef std::vector<Rect>::iterator        iterator;
        typedef std::vector<Rect>::const_iterator  const_iterator;

        RectList() {}
        RectList(const RectList&);
        RectList& operator= (const RectList&);

        iterator       begin()      { return m_rectangles.begin(); }
        const_iterator begin() const { return m_rectangles.begin(); }
        iterator       end()        { return m_rectangles.end(); }
        const_iterator end() const  { return m_rectangles.end(); }

        void clear()                { m_rectangles.clear(); }
        size_t size() const           { return m_rectangles.size(); }
        bool empty() const          { return m_rectangles.empty(); }
        void push_back(const Rect& r);
        void pop_back();
        void intersect(const Rect& r2);
        void add(const Rect& r);
        void sub(const Rect& r2);
        void merge(const RectList& rl);

        void swap(RectList &rl);
    private:
        std::vector<Rect> m_rectangles;
    };
}

#endif
