//======================================================================
// Copyright (C) 2002 Daniel Heck
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//  
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//  
// You should have received a copy of the GNU General Public License along
// with this program; if not, write to the Free Software Foundation, Inc.,
// 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
//======================================================================
#ifndef ECL_BUFFER_HH
#define ECL_BUFFER_HH

#include "SDL_types.h"
#include <iosfwd>
#include <cstddef>
#include <string>

namespace ecl
{    
    class Buffer {
      public:
        typedef std::size_t size_t;

        explicit Buffer(size_t startlen=128);
        ~Buffer()           { delete[] buf; }
        void clear()        { sz=0; rpos=wpos=buf; }

        void assign (char *data, size_t size);

        int read();			// return the next byte or -1 on EOF
        Buffer& write(char c);	// write one byte at the current position
        Buffer& read(void* dest, size_t maxlen);
        Buffer& write(const void* src, size_t len);

        // Change the read/write position.  Returns <0 on error.
        enum SeekMode {SET, CUR, END};
        int seekr(long pos, SeekMode whence);
        int seekw(long pos, SeekMode whence);

        ptrdiff_t get_rpos() const           { return rpos-buf; }   
        ptrdiff_t get_wpos() const           { return wpos-buf; }
        
        size_t size() const             { return sz; }
        const char* data() const        { return buf; };

        char* get_wspace(size_t len); // get space to write into
        char* get_rspace(size_t len); // get space to read from

        enum State { GOODBIT=0, FAILBIT=1, EOFBIT=2 };
        operator void*() const;	// == 0  iff  fail()
        bool operator!() const;	// true iff fail()
    
        bool good() const               { return iostate==0; }
        bool fail() const               { return iostate & FAILBIT; }
        bool eof() const                { return (iostate & EOFBIT) != 0; }
        void clear_state(State st=GOODBIT) { iostate = st; }
        State state() const             { return iostate; }
      private:
        char        *buf, *rpos, *wpos;
        size_t      sz, capacity;
        State       iostate;

        Buffer(const Buffer&);
        const Buffer& operator=(const Buffer&);
    };

    Buffer& read(Buffer& buf, Uint8& byte);
    Buffer& write(Buffer& buf, Uint8 byte);
    Buffer& read(Buffer& buf, Uint16& word);
    Buffer& write(Buffer& buf, Uint16 word);
    Buffer& read(Buffer& buf, Uint32& dword);
    Buffer& write(Buffer& buf, Uint32 dword);
    Buffer& read(Buffer& buf, Uint64& lvar);
    Buffer& write(Buffer& buf, Uint64 lvar);
    Buffer& read(Buffer& buf, float& dvar);
    Buffer& write(Buffer& buf, float dvar);
    Buffer& read(Buffer& buf, double& dvar);
    Buffer& write(Buffer& buf, double dvar);
    Buffer& read(Buffer& buf, std::string& str);
    Buffer& write(Buffer& buf, const std::string& str);
    Buffer& write(Buffer& buf, const Buffer& dest);
    Buffer& read(Buffer& buf, Buffer& dest, int len);
    
    template <typename T>
    inline Buffer& operator>>(Buffer& buf, T& val)
    {
        read(buf, val);
        return buf;
    }
    
    template <typename T>
    inline Buffer& operator<<(Buffer& buf, const T& val)
    {
        write(buf, val);
        return buf;
    }

    std::ostream& operator <<(std::ostream& os, const Buffer& buf);
//    std::istream& operator >>(std::istream& is, Buffer& buf);
}
#endif
