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

#include "FileUtils.h"
#include <stdio.h>

namespace OxydLib {

using namespace std;

bool readFile(const string &fileName, ByteVec *pBuf)
{
  pBuf->clear();

  FILE *inFile = fopen(fileName.c_str(), "rb");
  if (!inFile) {
    return false;
  }

  while (!feof(inFile)) {
    unsigned char tmpBuf[65536];

    int count = fread(tmpBuf,
                      sizeof(unsigned char),
                      65536,
                      inFile);

    if (count == 0) {
      fclose(inFile);
      return false;
    }

    int ii;
    for (ii = 0; ii < count; ii++) {
      pBuf->push_back(tmpBuf[ii]);
    }
  }

  fclose(inFile);

  return true;
}

bool writeFile(const string &fileName, const ByteVec &data)
{
  FILE *outFile = fopen(fileName.c_str(), "wb");
  if (!outFile) {
    return false;
  }

  int count = fwrite(&data[0], //.begin(),
                     sizeof(unsigned char),
                     data.size(),
                     outFile);

  if (count != (int)data.size()) {
    fclose(outFile);
    return false;
  }

  fclose(outFile);

  return true;
}

}
