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

#ifndef OXYDVERSION_H
#define OXYDVERSION_H

namespace OxydLib {

enum OxydVersion
{
  OxydVersion_Invalid = -1,
  OxydVersion_First = 0,

  OxydVersion_Oxyd1 = OxydVersion_First,
  OxydVersion_OxydMagnum,
  OxydVersion_OxydMagnumGold,
  OxydVersion_PerOxyd,
  OxydVersion_OxydExtra,

  OxydVersion_Count,
  OxydVersion_Last = OxydVersion_Count - 1
};

}

#endif
