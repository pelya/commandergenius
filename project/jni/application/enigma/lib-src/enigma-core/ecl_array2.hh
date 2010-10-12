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
#ifndef ECL_ARRAY2_HH
#define ECL_ARRAY2_HH

/*
** This file implements templated two-dimensional arrays.
*/

#include <memory>

namespace ecl
{
    template <class T, class A = std::allocator<T> >
    struct Array2Base {
        A alloc;                // allocator
        T *first, *last;        // start/end of allocated space

        Array2Base(const A &a, typename A::size_type n)
        : alloc(a), first(alloc.allocate(n)), last(first+n)
        {}

        ~Array2Base() {
            // allocate(0) returns 0 on GCC 2.95 -- standard?
            if (first)
                alloc.deallocate(first, last-first);
        }
        void resize (typename A::size_type n)
        {
            if (first)
                alloc.deallocate(first, last-first);
            first = alloc.allocate(n);
            last = first+n;
        }
    };

    template <class T, class A=std::allocator<T> >
    class Array2 : private Array2Base<T,A> {
        A alloc;
    public:
        typedef T           value_type;
        typedef T*          iterator;
        typedef const T *   const_iterator;
        typedef T &         reference;
        typedef const T &   const_reference;
        typedef typename A::size_type size_type;

        // Construction / Copying
        explicit Array2(int ww=0, int hh=0, const T& val=T(), const A& a=A());
        Array2(const Array2<T,A> &a);
        Array2<T,A> &operator=(Array2<T,A> a2); // call by value!

        // Destructor
        ~Array2() { destroy_elements(); }

        iterator begin() { return this->first; }
        iterator end() { return this->last; }
        const_iterator begin() const { return this->first; }
        const_iterator end() const { return this->last; }
        iterator row_begin(size_type y) { return this->first + y*w; }
        iterator row_end(size_type y) { return this->first + y*w + w; }
        const_iterator row_begin(size_type y) const { return this->first + y*w; }
        const_iterator row_end(size_type y) const { return this->first + y*w + w; }


        void swap(Array2<T,A> &a2);

        size_type width() const { return w; }
        size_type height()const { return h; }


        T&	 get(size_type x, size_type y) { return this->first[y*w+x]; }
        const T& get(size_type x, size_type y) const { return this->first[y*w+x]; }
        T& 	 operator()(size_type x, size_type y) { return get(x,y); }
        const T& operator()(size_type x, size_type y) const { return get(x,y); }

        void 	 set(size_type x, size_type y, const T& val) {
            this->first[y*w+x] = val;
        }

        /*! Fill the array with some value or the default value. */
        void fill (const T& val=T());

        /*! Resize the array in place, but discard any old array
          entries */
        void resize (int w, int h, const T& val=T());

    private:
        void destroy_elements();

        size_type w, h;
    };

    template <class T, class A>
    Array2<T,A>::Array2(int ww, int hh, const T& val, const A& a)
    : Array2Base<T,A>(a, ww*hh), w(ww), h(hh)
    {
        std::uninitialized_fill(this->first, this->last, val);
    }

    template <class T, class A>
    Array2<T,A>::Array2(const Array2<T,A> &a)
    : Array2Base<T,A>(a.alloc, a.last-a.first)
    {
        std::uninitialized_copy(a.begin(), a.end(), this->first);
    }

    template <class T, class A>
    void Array2<T,A>::destroy_elements() {
        for (T* p=this->first; p!=this->last; ++p)
            p->~T();
    }

    template <class T, class A>
    void Array2<T,A>::fill (const T& val)
    {
        destroy_elements();
        std::uninitialized_fill(this->first, this->last, val);
    }

    /*! Resize the array in place, but discard any old array
      entries */
    template <class T, class A>
    void Array2<T,A>::resize (int w_, int h_, const T& val)
    {
        destroy_elements();
        Array2Base<T,A>::resize(w_*h_);
        std::uninitialized_fill(this->first, this->last, val);
        w = w_;
        h = h_;
    }

    template <class T, class A>
    void Array2<T,A>::swap(Array2<T,A> &a2)
    {
        std::swap(this->first, a2.first);
        std::swap(this->last, a2.last);
        std::swap(w, a2.w);
        std::swap(h, a2.h);
    }

    template <class T, class A>
    void swap (Array2<T,A> &a, Array2<T,A> &b)
    {
        a.swap(b);
    }

    template <class T, class A> Array2<T,A>&
    Array2<T,A>::operator= (Array2<T,A> a2)
    {
        ecl::swap(*this, a2);
        return *this;
    }
}

#endif
