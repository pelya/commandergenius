// Copyright 2003 Jeremy Sawicki
//
// This file is part of OxydLib.
//
// OxydLib is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// OxydLib is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with OxydLib; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

#include "VecUtils.h"

namespace OxydLib {

unsigned int getInt2(ByteVec::const_iterator it)
{
  return (((unsigned int)(*it)) << 8) |
    (((unsigned int)(*(it + 1))));
}

unsigned int getInt4(ByteVec::const_iterator it)
{
  return (((unsigned int)(*it)) << 24) |
    (((unsigned int)(*(it + 1))) << 16) |
    (((unsigned int)(*(it + 2))) << 8) |
    (((unsigned int)(*(it + 3))));
}

void putInt2(ByteVec::iterator it, unsigned int ii)
{
  *it       = (unsigned char)((ii & 0xff00) >> 8);
  *(it + 1) = (unsigned char)((ii & 0x00ff));
}

void putInt4(ByteVec::iterator it, unsigned int ii)
{
  *it       = (unsigned char)((ii & 0xff000000) >> 24);
  *(it + 1) = (unsigned char)((ii & 0x00ff0000) >> 16);
  *(it + 2) = (unsigned char)((ii & 0x0000ff00) >> 8);
  *(it + 3) = (unsigned char)((ii & 0x000000ff));
}

unsigned int getInt2Reversed(ByteVec::const_iterator it)
{
  return (((unsigned int)(*it))) |
    (((unsigned int)(*(it + 1))) << 8);
}

unsigned int getInt4Reversed(ByteVec::const_iterator it)
{
  return (((unsigned int)(*it))) |
    (((unsigned int)(*(it + 1))) << 8) |
    (((unsigned int)(*(it + 2))) << 16) |
    (((unsigned int)(*(it + 3))) << 24);
}

void putInt2Reversed(ByteVec::iterator it, unsigned int ii)
{
  *it       = (unsigned char)((ii & 0x00ff));
  *(it + 1) = (unsigned char)((ii & 0xff00) >> 8);
}

void putInt4Reversed(ByteVec::iterator it, unsigned int ii)
{
  *it       = (unsigned char)((ii & 0x000000ff));
  *(it + 1) = (unsigned char)((ii & 0x0000ff00) >> 8);
  *(it + 2) = (unsigned char)((ii & 0x00ff0000) >> 16);
  *(it + 3) = (unsigned char)((ii & 0xff000000) >> 24);
}

}
