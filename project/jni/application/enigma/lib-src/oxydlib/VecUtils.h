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

#ifndef VECUTILS_H
#define VECUTILS_H

#include <vector>

namespace OxydLib {

typedef std::vector<unsigned char> ByteVec;

unsigned int getInt2(ByteVec::const_iterator it);
unsigned int getInt4(ByteVec::const_iterator it);

void putInt2(ByteVec::iterator it, unsigned int ii);
void putInt4(ByteVec::iterator it, unsigned int ii);

unsigned int getInt2Reversed(ByteVec::const_iterator it);
unsigned int getInt4Reversed(ByteVec::const_iterator it);

void putInt2Reversed(ByteVec::iterator it, unsigned int ii);
void putInt4Reversed(ByteVec::iterator it, unsigned int ii);

}

#endif
