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

#ifndef BITMAP_H
#define BITMAP_H

#include "VecUtils.h"
#include <string>

namespace OxydLib {

using std::string;

class Palette
{
public:
  Palette();
  ~Palette();

  int getNumColors() const;
  void setNumColors(int numColors);

  void getColor(int nColor,
                unsigned int *pRed,
                unsigned int *pGreen,
                unsigned int *pBlue) const;
  void setColor(int nColor,
                unsigned int red,
                unsigned int green,
                unsigned int blue);

private:
  std::vector<unsigned int> m_colors;
};

class Bitmap
{
public:
  Bitmap();
  ~Bitmap();

  void clear();

  int getBitDepth() const { return m_bitDepth; }
  void setBitDepth(int bitDepth) { m_bitDepth = bitDepth; }

  unsigned char getPixel(int x, int y) const;
  void setPixel(int x, int y, unsigned char pixelVal);

  int getBit(int x, int y, int bitNum) const;
  void setBit(int x, int y, int bitNum, int bitVal);

private:
  int m_bitDepth;
  ByteVec m_pixels;
};

// Both parseBitmap and unparseBitmap work with 16-color palettes,
// regardless of the bit depth of the bitmap.

bool parseBitmap(const ByteVec &in,
                 Bitmap *pBitmap,
                 Palette *pPalette,
                 string *pMsg = 0);
bool unparseBitmap(const Bitmap &bitmap,
                   const Palette &palette,
                   ByteVec *pOut,
                   string *pMsg = 0);

// The palette passed to bitmapToPPM should have 2^(bit depth of bitmap)
// colors.

void bitmapToPPM(const Bitmap &bitmap,
                 const Palette &palette,
                 ByteVec *pOut);
void bitPlaneToPPM(const Bitmap &bitmap,
                   int nBitPlane,
                   ByteVec *pOut);

}

#endif
