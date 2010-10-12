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
#ifndef ECL_CACHE_HH
#define ECL_CACHE_HH

/* -------------------- Cache -------------------- */

/*
  A generic class for caching external data.  Stored data is owned by
  the cache and is automatically `release'd on destruction.  Missing
  values are automatically retrieved using the `acquire' method.
*/

#include "ecl_dict.hh"

namespace ecl
{
    template <class T>
    class DeleteDisposer {
    public:
        static void dispose (T p) 
        { delete p; }
    };

    template <class T, class Disposer>
    class Cache {
    public:
        Cache();
        virtual ~Cache() {}

        // ---------- Methods ----------
        void     clear();
        T        get (const std::string &key);
        void     remove (const std::string &key);
        unsigned size() const;
        bool     has_key(const std::string &key) const;
    protected:
        T        store (const std::string &key, T value);

    private:
        Cache (const Cache &other);
        Cache &operator= (const Cache &other);

        void release (T value) {
            Disposer::dispose (value);
        }

        // ---------- Interface ----------
        virtual T    acquire (const std::string &name) = 0;

        // ---------- Variables ----------
        typedef ecl::Dict<T> Map;
        typedef typename Map::iterator iterator;

        Map cache;
    };

    template <class T, class D>
    Cache<T,D>::Cache() : cache(1223) {
    }

    template <class T, class D>
    void Cache<T, D>::clear() {
        for (iterator i=cache.begin(); i!=cache.end(); ++i) 
            release(i->second);
        cache.clear();
    }

    template <class T, class D>
    void Cache<T, D>::remove (const std::string &key)
    {
        cache.remove (key);
    }


    template <class T, class D>
    T Cache<T,D>::store (const std::string &key, T value) {
        cache.insert(key, value);
        return value;
    }

    template <class T, class D>
    T Cache<T,D>::get(const std::string &key) {
        iterator i=cache.find(key);
        if (i!=cache.end())
            return i->second;
        else
            return store (key, acquire(key));
    }

    template <class T, class D>
    unsigned Cache<T,D>::size() const { 
        return cache.size(); 
    }
    
    template <class T, class D>
    bool Cache<T, D>::has_key(const std::string &key) const {
        return cache.has_key(key);
    }

/* -------------------- PtrCache -------------------- */

    template <class T>
    class PtrCache : public Cache<T*, DeleteDisposer<T*> > {
    public:
        ~PtrCache() { this->clear(); }

//        void release (T *value) { delete value; }
    };
}

#endif
