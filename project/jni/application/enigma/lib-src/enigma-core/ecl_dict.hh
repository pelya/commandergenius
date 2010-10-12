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
#ifndef ECL_DICT_HH
#define ECL_DICT_HH

#include "ecl_error.hh"
#include <utility>

namespace ecl
{
    extern unsigned hash(const std::string &key);

    class XInvalidKey : XGeneric {
	XInvalidKey () : XGeneric("invalid dictionary key")
        {}
    };


    template <class T>
    class Dict {
    public:
	typedef std::string            key_type;
    	typedef std::pair<key_type, T> value_type;
	typedef size_t                 size_type;

    private:
    	struct Entry {
	    Entry(const key_type &k, const T &v) : pair(k,v) {}
            value_type  pair;
            Entry      *next;
	};

        // ---------- Iterator ----------
        template <class U>
        class Iter {
        public:
            typedef U                          value_type;
            typedef ptrdiff_t                  difference_type;
            typedef std::forward_iterator_tag  iterator_category;
            typedef value_type                &reference;
            typedef value_type                *pointer;

            Iter() : dict(0), idx(0), cur(0) {}
            Iter(const Dict<T> *d, size_type i, Entry *c)
            : dict(d),idx(i),cur(c) 
            {}
            Iter(const Dict<T> *d) :dict(d)
            {
                for (idx=0; idx<dict->nbuckets; ++idx)
                    if ((cur=dict->hashtab[idx]) != 0)
                        return;
                dict = 0;       // End
                idx=0;
            }
            bool operator==(const Iter &i) {
                return dict==i.dict && idx==i.idx && cur==i.cur;
            }
            bool operator!=(const Iter &i) {
                return !this->operator==(i);
            }

            reference operator*() {return cur->pair;}
            pointer operator->() {return &cur->pair;}
            Iter &operator++() {
                if ((cur=cur->next) == 0) {
                    for (++idx; idx<dict->nbuckets; ++idx)
                        if ((cur=dict->hashtab[idx]) != 0)
                            return *this;
                    dict = 0;
                    cur=0;
                    idx=0;
                }
                return *this;
            }
            Iter &operator++(int)
            {
                Iter tmp=*this;
                ++*this;
                return tmp;
            }
        private:
            friend class Dict<T>;

            const Dict<T> *dict;
            size_type      idx;
            Entry         *cur;
        };

    public:
	typedef Iter<value_type>                iterator;
  	typedef Iter<const value_type>          const_iterator;
        friend class Iter<value_type>;
        friend class Iter<const value_type>;        

	Dict(size_type table_size = 257);
        ~Dict();

        size_type size() const { return nentries; }

	iterator begin() { return iterator(this); }
	iterator end() { return iterator(); }
 	const_iterator begin() const { return const_iterator(this); }
 	const_iterator end() const { return const_iterator(); }

        iterator find (const key_type &key);
        const_iterator find (const key_type &key) const;

	T &lookup(const key_type &key) {
    	    Entry *e = find_entry(key);
//     	    if (!e) throw XInvalidKey();
    	    return e->pair.second;
    	}
	T &operator[] (const key_type &key) { return lookup(key); }
	
	const T& lookup(const key_type &key) const {
	    Entry *e = find_entry(key);
	    if (!e) throw XInvalidKey();
	    return e->pair.second;
	}

	const T& operator[] (const key_type &key) const
	{ return lookup(key); }

	bool has_key(const key_type &key) const;

        /*! Insert a new element into the table. */
	void insert(const key_type &key, const T &val);

        /*! Remove all entries from the hash table. */
	void clear();

        /*! Remove the entry with key \var key from the table. */
	void remove(const key_type &key);

        /*! Remove the element pointed to by an iterator. */
        void remove (iterator i);

    private:
    	Entry *find_entry(const key_type &key) const;

        // ----------  Variables ----------
        size_type nentries;     // Number of entries
	size_type nbuckets;     // Number of buckets in `hashtab'
	Entry **hashtab;
    private:
        // Copying not implemented yet
        Dict (const Dict<T> &d)
        : nentries(d.nentries), 
          nbuckets(d.nbuckets), 
          hashtab(new Entry*[nbuckets])
	{
	    for (size_type i=0; i<nbuckets; ++i) {
	    	Entry *e = d.hashtab[i];
		hashtab[i] = 0;     	    	// XXX Fix
	    }
	}
        Dict &operator=(const Dict<T> &d);
    };

/* -------------------- Dict implementation -------------------- */

    template <class T>
    Dict<T>::Dict(size_type table_size)
    : nentries(0), nbuckets (table_size), hashtab (new Entry*[nbuckets])
    {
        for (size_type i=0; i<nbuckets; ++i) 
            hashtab[i] = 0;
    }

    template <class T>
    Dict<T>::~Dict() {
        clear();
        delete [] hashtab;
    }

    template <class T>
    typename Dict<T>::iterator 
    Dict<T>::find (const key_type &key) 
    {
        unsigned h = hash(key) % nbuckets;
        for (Entry *e = hashtab[h]; e != 0; e=e->next)
            if (e->pair.first == key)
                return iterator(this, h, e);
        return end();
    }

    template <class T>
    typename Dict<T>::const_iterator 
    Dict<T>::find (const key_type &key) const 
    {
        unsigned h = hash(key) % nbuckets;
        for (Entry *e = hashtab[h]; e != 0; e=e->next)
            if (e->pair.first == key)
                return const_iterator(this, h, e);
        return end();
    }

    template <class T>
    void Dict<T>::clear() 
    {
        for (size_type i=0; i<nbuckets; ++i) {
            Entry *cur, *next;
            for (cur = hashtab[i]; cur != 0; cur=next) {
                next = cur->next;
                delete cur;
            }
            hashtab[i] = 0;
        }
        nentries = 0;
    }

    template <class T>
    void Dict<T>::insert (const key_type &key, const T &val) 
    {
        unsigned h = hash(key) % nbuckets;
        Entry *e = new Entry(key, val);
        e->next = hashtab[h];
        hashtab[h] = e;
        nentries += 1;
    }

    template <class T>
    void Dict<T>::remove (const key_type &key) 
    {
        unsigned h = hash(key) % nbuckets;
        Entry *e = hashtab[h];
        Entry **eptr = &hashtab[h];

        while (e != 0) {
            if (e->pair.first == key) {
                *eptr = e->next;	    	// Modify pointer referring to e
                delete e;
                nentries -= 1;
                break;
            }
            eptr = &e->next;
            e = e->next;
        }
    }

    template <class T>
    bool Dict<T>::has_key (const key_type &key) const 
    {
        return find_entry(key) != 0;
    }

    template <class T>
    typename Dict<T>::Entry *
    Dict<T>::find_entry (const key_type &key) const 
    {
        unsigned h = hash(key) % nbuckets;
        for (Entry *e = hashtab[h]; e != 0; e=e->next)
            if (e->pair.first == key)
                return e;
        return 0;
    }
}
#endif
