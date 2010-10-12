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
#ifndef ECL_UTIL_HH_INCLUDED
#define ECL_UTIL_HH_INCLUDED

#include <string>
#include <algorithm>

/* hide GNU extensions for non-gnu compilers: */
#ifndef __GNU__
# ifndef __attribute__
// #  define __attribute__(x)
// # error wuah
# endif
#endif

namespace ecl
{

/*
** Base class for classes that may not be copied, neither via copy
** constructor or assignment operator.
*/
    class Nocopy {
        Nocopy(const Nocopy&);
        Nocopy& operator=(const Nocopy&);
    public:
        Nocopy() {}
    };

/*
** Some kind of smart pointer.  Almost verbatim from tc++pl3
*/
    template <class X> class Handle {
        X *rep;
        int *cnt;
    public:
        X *operator->() { return rep; }
        X *get() const { return rep; }
        X &operator *() { return *rep; }

        explicit Handle(X *p=0) : rep(p), cnt(new int(1)) {}
        Handle(const Handle<X>& a) : rep(a.rep), cnt(a.cnt) {
            ++(*cnt);
        }
        Handle<X> &operator=(const Handle<X> &a) {
            if (cnt == a.cnt) return *this;
            if (--(*cnt) == 0) {
                delete rep;
                delete cnt;
            }
            rep = a.rep;
            cnt = a.cnt;
            (*cnt)++;
            return *this;
        }
        ~Handle() {
            if (--(*cnt) == 0) {
                delete rep;
                delete cnt;
            }
        }
    };

/*
** More C++ magic.  This class wraps around arbitrary linear
** containers (vector,list), but deletes all stored entries on
** destruction.
**
** Example:
**
**     {   AutoSequence <vector<Image *> > images;
**         images->push_back(new Image("hello.bmp"));
**         // ...
**     }
**
** All images allocated and stored in `images' have by now been freed.
*/
    template <class T>
    class AutoSequence : public ecl::Nocopy {
    public:
        ~AutoSequence() { delete_sequence(seq.begin(), seq.end()); }
        T *operator->() { return &seq; }
        T &operator*() { return seq; }
    private:
        T seq;
    };



    template <class Iter> inline
    Iter next(const Iter i) {
        Iter j=i;
        return ++j;
    }
    template <class Iter> inline
    Iter prev(const Iter i) {
        Iter j=i;
        return --j;
    }

    template <class For> inline
    void
    delete_sequence(For begin, For end)
    {
        while (begin != end)
            delete *begin++;
    }

    template <class For> inline void
    delete_map(For begin, For end)
    {
        while (begin != end)
        {
            delete begin->second;
            ++begin;
        }
    }

    template <class T> inline const T &
    Max(const T &a, const T &b)
    {
        return (a > b) ? a : b;
    }

    template <class T> inline const T &
    Min(const T &a, const T &b)
    {
        return (a < b) ? a : b;
    }

    template <class T> inline const T &
    Clamp (const T& a, const T& min, const T& max)
    {
        if (a < min)
            return min;
        else if (a > max)
            return max;
        else
            return a;
    }

/*
** Split a string at every occurrence of 'sep' and store the
** resulting strings using the iterator 'out'.
**
** Example:
**
**     vector<string> vs;
**     split_copy("/usr/bin/enigma", '/', back_inserter(vs));
*/
    template <class Ch, class OutIt>
    void split_copy(const std::basic_string<Ch> &str, Ch sep, OutIt out)
    {
        typename std::basic_string<Ch>::const_iterator i,j;
        i=str.begin();
        while (true) {
            j = std::find(i, str.end(), sep);
            *out = std::basic_string<Ch>(i,j);
            ++out;
            if (j==str.end())
                break;
            i=j+1;
        }
    }

    template <class Ch, class OutIt>
    size_t split_copy_n (const std::basic_string<Ch> &str, Ch sep, OutIt out, size_t n)
    {
        typename std::basic_string<Ch>::const_iterator i,j;
        i=str.begin();
        size_t cnt=0;
        for (; cnt < n; ++cnt) {
            j = std::find(i, str.end(), sep);
            *out = std::basic_string<Ch>(i,j);
            ++out;
            if (j==str.end())
                break;
            i=j+1;
        }
        return cnt;
    }

/*
** Remove white space at the beginning and end of a string.
*/
    template <class Str>
    Str trim(const Str& str)
    {
        if (str.size() == 0)
            return str;
        typename Str::size_type b = str.find_first_not_of(" \t\n\r");
        typename Str::size_type e = str.find_last_not_of(" \t\n\r");
        if (b == Str::npos)
            return "";
        return Str(str, b, e-b+1);
    }


/*
** Some helper functions for dealing with bit flags.
*/

    template <class E, class I> inline void
    set_flags (E &val, I flags)
    {
        val = static_cast<E> (val | flags);
    }

    template <class E, class I> inline void
    clear_flags (E &val, I flags)
    {
        val = static_cast<E> (val & ~flags);
    }

    template <class E, class I> inline void
    toggle_flags (E &val, I flags)
    {
        val = static_cast<E> (val ^ flags);
    }

    template <class E, class I> inline bool
    has_flags (E val, I flags)
    {
        return (static_cast<int>(val & flags) == static_cast<int>(flags));
    }

    /*
    ** Some helper functions for strings and filenames
    */

    std::string strf(const char *format, ...) __attribute__((format(printf, 1, 2)));

    std::string concat_paths(const std::string& path, const std::string& filename);
    
    /**
     * Split a given path into the leading directory part and the last filename
     * part. The path may use unix slash separators as well as windows backlashes.
     * Even mixed usage is allowed. Trailing path separators are skipped.
     * @param path           the path to be split
     * @param dir_part       ptr to a return string where the leading directory 
     *                       part without trailing separators will be stored. 
     *                       The string will not be set if the result is false.
     * @param filename_part  ptr to a return string where the trailing filename 
     *                       part will be stored. 
     *                       The string will not be set if the result is false.
     * @return               success of splitting the path into 2 parts. If no
     *                       path separator was found just false will be returned
     *                       without modification of the return strings.
     */
    bool        split_path(const std::string& path, std::string* dir_part, std::string* filename_part);

}

#endif
