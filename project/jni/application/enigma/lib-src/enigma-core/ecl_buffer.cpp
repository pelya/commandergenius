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
#include "ecl_buffer.hh"
#include <iostream>

using namespace ecl;
using namespace std;

Buffer::Buffer(size_t startlen)
    : buf(new char[startlen])
    , rpos(buf), wpos(buf)
    , sz(0)
    , capacity(startlen)
    , iostate(GOODBIT)
{}


void Buffer::assign (char *data, size_t size)
{
    clear();
    write (data, size);
}


char* Buffer::get_wspace(size_t len)
{
    size_t newcap = capacity;
    while (wpos + len > buf + newcap)
        newcap *= 2;
    if (newcap != capacity)
    {
	char* newbuf = new char[newcap];
	memcpy(newbuf, buf, sz);
        capacity = newcap;
	wpos = (wpos - buf) + newbuf;
	rpos = (rpos - buf) + newbuf;
	delete[] buf;
	buf = newbuf;
    }
    char* ret = wpos;
    wpos += len;
    sz += len;
    return ret;
}

char* Buffer::get_rspace(size_t len)
{
    if (!good() || rpos + len > buf + sz)
    {
        iostate = State(iostate | FAILBIT | EOFBIT);
        return 0;
    }
    char* ret = rpos;
    rpos += len;
    return ret;
}

int Buffer::read()
{
    if (good() && rpos < buf+sz)
	return *rpos++;
    else
	return -1;
}

Buffer& Buffer::write(char c)
{
    char* ptr = get_wspace(1);
    *ptr = c;
    return *this;
}

Buffer& Buffer::read(void* dest, size_t len)
{
    if (!good() || (rpos + len > buf + sz))
    {
        iostate = State(iostate | EOFBIT | FAILBIT);
    }
    else
    {
        memcpy(dest, rpos, len);
        rpos += len;
    }
    return *this;
}

Buffer& Buffer::write(const void* src, size_t len)
{
    if (char* dest = get_wspace(len)) {
        memcpy(dest, src, len);
    }
    return *this;
}

int Buffer::seekr(long pos, SeekMode whence)
{
    size_t offs = 0;

    switch (whence) {
    case SET: offs = pos; break;
    case CUR: offs = (rpos - buf) + pos; break;
    case END: offs = sz + pos; break;
    }
    if ( /* offs < 0 || */ /* always false */
        offs > sz)
        return -1;
    rpos = buf + offs;
    return 0;
}

int Buffer::seekw(long pos, SeekMode whence)
{
    size_t offs = 0;

    switch (whence) {
    case SET: offs = pos; break;
    case CUR: offs = (wpos - buf) + pos; break;
    case END: offs = sz + pos; break;
    }
    if (/* offs < 0 || */ /* always false */
        offs > sz)
        return -1;
    wpos = buf + offs;
    return 0;
}

Buffer::operator void*() const
{
    return (iostate & FAILBIT) ? 0 : reinterpret_cast<void*>(1);
}

bool Buffer::operator!() const
{
    return (iostate & FAILBIT) ? true : false;
}


//----------------------------------------
// Buffer helper routines.
//----------------------------------------

/*
** === Read and write bytes ===
*/
Buffer& ecl::read(Buffer& buf, Uint8& byte)
{
    int a = buf.read();
    if (a != -1)
        byte = a;
    return buf;
}

Buffer& ecl::write(Buffer& buf, Uint8 byte)
{
    buf.write(byte);
    return buf;
}

/*
** === Read and write words ===
*/

Buffer& ecl::read(Buffer& buf, Uint16& word)
{
    Uint8* ptr = (Uint8*) buf.get_rspace(2);
    if (ptr != 0)
        word = ptr[0] + (ptr[1] << 8);
    return buf;
}

Buffer& ecl::write(Buffer& buf, Uint16 word)
{
    Uint8* ptr = (Uint8*) buf.get_wspace(2);
    ptr[0] = word & 0xff;
    ptr[1] = (word >> 8) & 0xff;
    return buf;
}

/*
** === Read and write dwords ===
*/

Buffer& ecl::read(Buffer& buf, Uint32& dword)
{
    Uint8* ptr = (Uint8*) buf.get_rspace(4);
    if (ptr != 0)
    {
        dword = ptr[0];
        dword |= (ptr[1] << 8);
        dword |= (ptr[2] << 16);
        dword |= (ptr[3] << 24);
    }
    return buf;
}

Buffer& ecl::write(Buffer& buf, Uint32 dword)
{
    Uint8* ptr = (Uint8*) buf.get_wspace(4);
    ptr[0] = dword & 0xff;
    ptr[1] = (dword >> 8) & 0xff;
    ptr[2] = (dword >> 16) & 0xff;
    ptr[3] = (dword >> 24) & 0xff;
    return buf;
}

/*
 ** === Read and write long longs ===
 */

Buffer& ecl::read(Buffer& buf, Uint64& lvar)
{
    Uint8* ptr = (Uint8*) buf.get_rspace(8);
    if (ptr != 0)
    {
        lvar = ptr[0];
        lvar |= (Uint64(ptr[1]) << 8);
        lvar |= (Uint64(ptr[2]) << 16);
        lvar |= (Uint64(ptr[3]) << 24);
        lvar |= (Uint64(ptr[4]) << 32);
        lvar |= (Uint64(ptr[5]) << 40);
        lvar |= (Uint64(ptr[6]) << 48);
        lvar |= (Uint64(ptr[7]) << 56);
    }
    return buf;
}

Buffer& ecl::write(Buffer& buf, Uint64 lvar)
{
    Uint8* ptr = (Uint8*) buf.get_wspace(8);
    ptr[0] = Uint8(lvar & 0xff);
    ptr[1] = Uint8((lvar >> 8) & 0xff);
    ptr[2] = Uint8((lvar >> 16) & 0xff);
    ptr[3] = Uint8((lvar >> 24) & 0xff);
    ptr[4] = Uint8((lvar >> 32) & 0xff);
    ptr[5] = Uint8((lvar >> 40) & 0xff);
    ptr[6] = Uint8((lvar >> 48) & 0xff);
    ptr[7] = Uint8((lvar >> 56) & 0xff);
    return buf;
}

/*
** === Read and write floats ===
*/

/* NOTE: we reinterpret floats and doubles as Uint32/Uint64, 
** and byte-swap them like integers.
** This is how quake does it, too (except they only use floats).
** I couldn't believe that this worked, so I checked it out 
** (PPC <-> x86), and it does. The upside (over multiplying 
** with 2048 and converting to int, as it was before) is that
** we have the same rep on both machines, possibly leading to 
** less desynchronisation in network games.
*/

Buffer& ecl::read(Buffer& buf, float& dvar)
{
	union
{
	float f;
	Uint32 a;
} t;
    if (buf >> t.a)
        dvar = t.f;
    return buf;
}

Buffer& ecl::write(Buffer& buf, float dvar)
{
	union
{
	float f;
	Uint32 a;
} t;
    t.f = dvar;
    write(buf, t.a);
    return buf;
}

/*
 ** === Read and write doubles ===
 */

Buffer& ecl::read(Buffer& buf, double& dvar)
{
    union
{
	double d;
	Uint64 a;
} t;
    if (buf >> t.a)
        dvar = t.d;
    return buf;
}

Buffer& ecl::write(Buffer& buf, double dvar)
{
	union
{
	double d;
	Uint64 a;
} t;
    t.d=dvar;
    write(buf, t.a);
    return buf;
}

/*
** === Read and write another buffer ===
*/

Buffer& ecl::read(Buffer& srcbuf, Buffer& destbuf, int len)
{
    if (&srcbuf != &destbuf)
    {
        char* dest = destbuf.get_wspace(len);
        char* src = srcbuf.get_rspace(len);
        if (src != 0)
            memcpy(dest, src, len);
    }
    return srcbuf;
}

Buffer& ecl::write(Buffer& buf, const Buffer& databuf)
{
    if (&buf != &databuf)
    {
        buf.write(databuf.data(), databuf.size());
    }
    return buf;
}

/*
** === Read and write strings ===
*/

Buffer& ecl::read(Buffer& buf, string& str)
{
    Uint16 size;
    if (buf >> size)
    {
        char* ptr = buf.get_rspace(size);
        if (ptr != 0)
            str.assign(ptr, ptr+size);
    }
    return buf;
}

Buffer& ecl::write(Buffer& buf, const string& str)
{
    Uint16 size = str.size();
    buf << size;
    buf.write(str.c_str(), size);
    return buf;
}

/*
** Read and write from and to streams
*/

ostream& ecl::operator<<(ostream& os, const Buffer& buf)
{
    os.write(buf.data(), buf.size());
    return os;
}

// istream& ecl::operator>>(istream& is, Buffer& buf)
// {
//     buf.clear();
//     char tmp[2048];
//     while (is.read(tmp, sizeof tmp))
//     {
//         buf.write(tmp, sizeof tmp);
//     }
//     buf.write(tmp, is.gcount());
//     return is;
// }

