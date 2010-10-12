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

#include "Bitmap.h"
#include <stdio.h>
#include <set>
#include <map>
#include <algorithm>

namespace OxydLib {

using namespace std;

Palette::Palette()
{
}

Palette::~Palette()
{
}

int Palette::getNumColors() const
{
  return m_colors.size() / 3;
}

void Palette::setNumColors(int numColors)
{
  m_colors.resize(numColors * 3);
}

void Palette::getColor(int nColor,
                       unsigned int *pRed,
                       unsigned int *pGreen,
                       unsigned int *pBlue) const
{
  unsigned int red   = m_colors[3 * nColor + 0];
  unsigned int green = m_colors[3 * nColor + 1];
  unsigned int blue  = m_colors[3 * nColor + 2];

  if (pRed)   { *pRed   = red;   }
  if (pGreen) { *pGreen = green; }
  if (pBlue)  { *pBlue  = blue;  }
}

void Palette::setColor(int nColor,
                       unsigned int red,
                       unsigned int green,
                       unsigned int blue)
{
  m_colors[3 * nColor + 0] = red;
  m_colors[3 * nColor + 1] = green;
  m_colors[3 * nColor + 2] = blue;
}

Bitmap::Bitmap()
{
  m_pixels.resize(640*400);
  clear();
}

Bitmap::~Bitmap()
{
}

void Bitmap::clear()
{
  setBitDepth(0);

  for (int y = 0; y < 400; y++) {
    for (int x = 0; x < 640; x++) {
      setPixel(x, y, 0);
    }
  }
}

unsigned char Bitmap::getPixel(int x, int y) const
{
  return m_pixels[(y * 640) + x];
}

void Bitmap::setPixel(int x, int y, unsigned char pixelVal)
{
  m_pixels[(y * 640) + x] = pixelVal;
}

int Bitmap::getBit(int x, int y, int bitNum) const
{
  return (getPixel(x, y) & (1 << bitNum)) >> bitNum;
}

void Bitmap::setBit(int x, int y, int bitNum, int bitVal)
{
  setPixel(x, y,
           (getPixel(x, y) & ~(1 << bitNum)) |
           ((bitVal ? 1 : 0) << bitNum));
}

namespace
{
  const unsigned char bitPlaneTypeName[8] =
    {'I', 'B', 'M', '1', '6', 'C', 'O', 'L'};

  int getBitStreamBit(const ByteVec &in,
                      int bitStreamStart,
                      int offset)
  {
    return (in[bitStreamStart + (offset >> 3)] & (1 << (7 - (offset & 0x7))))
           >> (7 - (offset & 0x7));
  }
}


bool parseBitmap(const ByteVec &in,
                 Bitmap *pBitmap,
                 Palette *pPalette,
                 string *pMsg)
{
  string msg;
  if (!pMsg) {
    pMsg = &msg;
  }

  pBitmap->clear();

  int sz = in.size();
  int cur = 0;

  if (cur + 2 > sz) {
    pMsg->assign("Not enough data reading bit depth.");
    return false;
  }

  int bitDepth = getInt2Reversed(in.begin() + cur);
  cur += 2;

  if (bitDepth != 1 &&
      bitDepth != 4) {
    pMsg->assign("Expected a bit depth of 1 or 4.");
    return false;
  }

  pBitmap->setBitDepth(bitDepth);

  if (cur + (2 * bitDepth) > sz) {
    pMsg->assign("Not enough data reading bit plane sizes.");
    return false;
  }

  vector<int> bitPlaneSizes;
  bitPlaneSizes.resize(bitDepth);
  int bitPlaneSizeTotal = 0;
  int nBitPlane;
  for (nBitPlane = 0; nBitPlane < bitDepth; nBitPlane++) {
    bitPlaneSizes[nBitPlane] = getInt2Reversed(in.begin() + cur);
    cur += 2;

    bitPlaneSizeTotal += bitPlaneSizes[nBitPlane];
  }

  if (cur + 2 > sz) {
    pMsg->assign("Not enough data reading block size.");
    return false;
  }

  int blockSize = getInt2Reversed(in.begin() + cur);
  cur += 2;

  if (blockSize != 16) {
    pMsg->assign("Expected a block size of 16.");
    return false;
  }

  if (cur + 96 > sz) {
    pMsg->assign("Not enough data reading palette.");
    return false;
  }

  pPalette->setNumColors(16);
  for (int nColor = 0; nColor < 16; nColor++) {
    unsigned int red = getInt2Reversed(in.begin() + cur);
    cur += 2;

    unsigned int green = getInt2Reversed(in.begin() + cur);
    cur += 2;

    unsigned int blue = getInt2Reversed(in.begin() + cur);
    cur += 2;

    pPalette->setColor(nColor, red, green, blue);
  }

  if (cur + bitPlaneSizeTotal > sz) {
    pMsg->assign("Not enough data in chunk based on bit plane sizes.");
    return false;
  }

  for (nBitPlane = 0; nBitPlane < bitDepth; nBitPlane++) {
    if (bitPlaneSizes[nBitPlane] < 40) {
      pMsg->assign("Not enough data in bit plane for header.");
      return false;
    }

    int bitPlaneEnd = cur + bitPlaneSizes[nBitPlane];

    for (int i = 0; i < 8; i++) {
      unsigned char ch = in[cur];
      cur++;

      if (ch != bitPlaneTypeName[i]) {
        pMsg->assign("Unexpected character in bit plane type name.");
        return false;
      }
    }

    int bitPlaneDataSize = getInt4(in.begin() + cur);
    cur += 4;

    if (bitPlaneDataSize + 8 > bitPlaneSizes[nBitPlane]) {
      pMsg->assign("Bit plane data size is too large for bit plane size.");
      return false;
    }

    int bitPlaneDataEnd = cur - 4 + bitPlaneDataSize;

    int pixelWidth = getInt2(in.begin() + cur);
    cur += 2;

    if (pixelWidth != 640) {
      pMsg->assign("Pixel width of 640 expected.");
      return false;
    }

    int pixelHeight = getInt2(in.begin() + cur);
    cur += 2;

    if (pixelHeight != 400) {
      pMsg->assign("Pixel height of 400 expected.");
      return false;
    }

    int blockWidth = getInt2(in.begin() + cur);
    cur += 2;

    if (blockWidth != 40) {
      pMsg->assign("Block width of 40 expected.");
      return false;
    }

    int blockHeight = getInt2(in.begin() + cur);
    cur += 2;

    if (blockHeight != 25) {
      pMsg->assign("Block height of 25 expected.");
      return false;
    }

    int bitStreamSize = getInt4(in.begin() + cur);
    cur += 4;

    int byteStreamSize = getInt4(in.begin() + cur);
    cur += 4;

    if (bitStreamSize + byteStreamSize + 32 != bitPlaneDataSize) {
      pMsg->assign("Bit stream and byte stream sizes don't add up.");
      return false;
    }

    bool invertFlags[2];
    for (int nInvertFlag = 0; nInvertFlag < 2; nInvertFlag++) {
      unsigned char invertVal = in[cur];
      cur++;

      if (invertVal != (unsigned char)0x00 &&
          invertVal != (unsigned char)0xff) {
        pMsg->assign("Invalid invert value.");
        return false;
      }

      invertFlags[nInvertFlag] = (invertVal == (unsigned char)0xff);
    }

    int blockCopyFlagVal = getInt2(in.begin() + cur);
    cur += 2;

    bool bHasBlockCopies = (blockCopyFlagVal == 0xdead);

    // Unused bytes
    cur += 8;

    int bitStreamStart = cur;
    int bitStreamEnd = bitStreamStart + bitStreamSize;

    int byteStreamStart = bitStreamEnd;
    int byteStreamEnd = byteStreamStart + byteStreamSize;

    int curBit = 0;
    int numBits = bitStreamSize * 8;

    int curByte = byteStreamStart;

    int st = bitStreamStart;

    int curBlockY;
    int curBlockX;

    for (curBlockY = 0; curBlockY < blockHeight; curBlockY++) {
      if (curBit + 1 > numBits) {
        pMsg->assign("Not enough bits reading row skip bit.");
        return false;
      }

      int rowSkip = getBitStreamBit(in, st, curBit);
      curBit++;

      if (rowSkip == 0) {
        // Row is empty.
      } else {
        for (curBlockX = 0; curBlockX < blockWidth; curBlockX++) {
          if (curBit + 1 > numBits) {
            pMsg->assign("Not enough bits reading first bit of block info.");
            return false;
          }

          int bit1 = getBitStreamBit(in, st, curBit);
          curBit++;

          if (bit1 == 0) {
            // Block is empty.
          } else {
            if (curBit + 2 > numBits) {
              pMsg->assign("Not enough bits reading block compression type.");
              return false;
            }

            int compressionType = (getBitStreamBit(in, st, curBit) << 1) +
                                   getBitStreamBit(in, st, curBit + 1);
            curBit += 2;

            if (compressionType == 3) {
              if (curByte + 32 > byteStreamEnd) {
                pMsg->assign("Not enough bytes reading uncompressed block.");
                return false;
              }

              int nByte;
              for (nByte = 0; nByte < 32; nByte++) {
                int byteVal = in[curByte];
                curByte++;

                int byteBit;
                for (byteBit = 7; byteBit >= 0; byteBit--) {
                  int bitVal = (byteVal & (1 << byteBit)) >> byteBit;

                  pBitmap->setBit((curBlockX * 16) + ((nByte % 2) * 8) +
                                    (7 - byteBit),
                                  (curBlockY * 16) + (nByte >> 1),
                                  nBitPlane,
                                  bitVal);
                }
              }
            } else {
              if (curBit + 4 > numBits) {
                pMsg->assign("Not enough bits reading quadrant skip bits.");
                return false;
              }

              int quadrantSkips[4];
              int nQuadrant;
              for (nQuadrant = 0; nQuadrant < 4; nQuadrant++) {
                quadrantSkips[nQuadrant] = getBitStreamBit(in, st, curBit);
                curBit++;
              }

              for (nQuadrant = 0; nQuadrant < 4; nQuadrant++) {
                unsigned char quadrantHeader = 0;
                if (quadrantSkips[nQuadrant]) {
                  if (curByte + 1 > byteStreamEnd) {
                    pMsg->assign("Not enough bytes reading quadrant header.");
                    return false;
                  }

                  quadrantHeader = in[curByte];
                  curByte++;
                }

                int nLine;
                for (nLine = 0; nLine < 8; nLine++) {
                  unsigned char lineByte = 0;

                  if (quadrantHeader & (1 << (7 - nLine))) {
                    if (curByte + 1 > byteStreamEnd) {
                      pMsg->assign("Not enough bytes reading line byte.");
                      return false;
                    }

                    lineByte = in[curByte];
                    curByte++;
                  }

                  int pixXstart = (curBlockX * 16) + (nQuadrant & 0x1) * 8;
                  int pixY = (curBlockY * 16) + (nQuadrant & 0x2) * 4 + nLine;

                  int bXor = (compressionType > 0);
                  int xorDiff = compressionType;

                  int nBit;
                  for (nBit = 0; nBit < 8; nBit++) {
                    int lineBit = (lineByte & (1 << (7 - nBit))) >> (7 - nBit);

                    if (bXor) {
                      if (((nQuadrant & 0x2) * 4 + nLine) >= xorDiff) {
                        lineBit ^= pBitmap->getBit(pixXstart + nBit,
                                                   pixY - xorDiff,
                                                   nBitPlane);
                      }
                    }

                    pBitmap->setBit(pixXstart + nBit,
                                    pixY,
                                    nBitPlane,
                                    lineBit);
                  }
                }
              }
            }
          }
        }
      }
    }

    if (curByte < byteStreamEnd) {
      pMsg->assign("Extra bytes in byte stream.");
      return false;
    }

    if (numBits - curBit >= 8) {
      pMsg->assign("Too many spare bits in bit stream.");
      return false;
    }

    while (curBit < numBits) {
      //int bb = getBitStreamBit(in, st, curBit);
      curBit++;

      // Spare bits are usually 0, but not always.
      /*
      if (bb != 0) {
        pMsg->assign("Spare bit is not 0.");
        return false;
      }
      */
    }

    cur = bitPlaneDataEnd;

    // Process copied blocks.
    if (bHasBlockCopies) {
      bool foundLastSource = false;
      do {
        if (cur + 2 > bitPlaneEnd) {
          pMsg->assign("Ran out of data reading block copy source.");
          return false;
        }

        unsigned char sourceBlockX = in[cur];
        cur++;
        unsigned char sourceBlockY = in[cur];
        cur++;

        if (sourceBlockX == 0xff && sourceBlockY == 0xff) {
          foundLastSource = true;
        } else {
          if (sourceBlockX >= blockWidth) {
            pMsg->assign("Block copy source X coordinate too large.");
            return false;
          }
          if (sourceBlockY >= blockHeight) {
            pMsg->assign("Block copy source Y coordinate too large.");
            return false;
          }

          vector<unsigned char> destBlockXVec;
          vector<unsigned char> destBlockYVec;
          bool foundLastDest = false;
          do {
            if (cur + 2 > bitPlaneEnd) {
              pMsg->assign("Ran out of data reading block copy destination.");
              return false;
            }

            unsigned char destBlockX = in[cur];
            cur++;
            unsigned char destBlockY = in[cur];
            cur++;

            if (destBlockX == 0xff && destBlockY == 0xff) {
              foundLastDest = true;
            } else {
              if (destBlockX >= blockWidth) {
                pMsg->assign("Block copy destination X coordinate too large.");
                return false;
              }
              if (destBlockY >= blockHeight) {
                pMsg->assign("Block copy destination Y coordinate too large.");
                return false;
              }

              destBlockXVec.push_back(destBlockX);
              destBlockYVec.push_back(destBlockY);
            }
          } while (!foundLastDest);

          if (destBlockXVec.size() == 0) {
            pMsg->assign("Block copy source has no destinations.");
            return false;
          }

          for (int nDest = 0; nDest < (int)destBlockXVec.size(); nDest++) {
            unsigned char destBlockX = destBlockXVec[nDest];
            unsigned char destBlockY = destBlockYVec[nDest];

            for (int pixelDeltaY = 0; pixelDeltaY < 16; pixelDeltaY++) {
              int sourcePixelY = sourceBlockY * 16 + pixelDeltaY;
              int destPixelY = destBlockY * 16 + pixelDeltaY;
              for (int pixelDeltaX = 0; pixelDeltaX < 16; pixelDeltaX++) {
                int sourcePixelX = sourceBlockX * 16 + pixelDeltaX;
                int destPixelX = destBlockX * 16 + pixelDeltaX;

                int bit = pBitmap->getBit(sourcePixelX,
                                          sourcePixelY,
                                          nBitPlane);
                pBitmap->setBit(destPixelX,
                                destPixelY,
                                nBitPlane,
                                bit);
              }
            }
          }
        }
      } while (!foundLastSource);
    }

    if (cur < bitPlaneEnd) {
      pMsg->assign("Unexpected data at the end of bit plane.");
      return false;
    }

    // Invert the bit plane
    if (invertFlags[0] || invertFlags[1]) {
      for (int y = 0; y < pixelHeight; y++) {
        if (invertFlags[y & 0x1]) {
          for (int x = 0; x < pixelWidth; x++) {
            int bit = pBitmap->getBit(x, y, nBitPlane);
            pBitmap->setBit(x, y, nBitPlane, 1 - bit);
          }
        }
      }
    }
  }

  if (cur < sz) {
    pMsg->assign("Unexpected data at the end of bitmap.");
    return false;
  }

  return true;
}

namespace
{
  void addBit(ByteVec *pBits, int *pBitCount, int bit)
  {
    int nByte = (*pBitCount) >> 3;
    int nBit = 7 - ((*pBitCount) & 0x7);
    if (nByte >= (int)pBits->size()) {
      pBits->resize(nByte + 1);
    }
    unsigned char ch = (*pBits)[nByte];
    ch = ch & ~(1<<nBit);
    ch = ch | (bit<<nBit);
    (*pBits)[nByte] = ch;
    (*pBitCount)++;
  }

  void addBits(ByteVec *pBits, int *pBitCount,
               const ByteVec &addedBits, int addedBitCount)
  {
    for (int i = 0; i < addedBitCount; i++) {
      int bit = getBitStreamBit(addedBits, 0, i);
      addBit(pBits, pBitCount, bit);
    }
  }

  void addBytes(ByteVec *pBytes, const ByteVec &addedBytes)
  {
    int sz = pBytes->size();
    pBytes->resize(sz + addedBytes.size());
    copy(addedBytes.begin(), addedBytes.end(),
         pBytes->begin() + sz);
  }

  int getBitInverted(const Bitmap &bitmap,
                     int x,
                     int y,
                     int bitNum,
                     bool bInvert)
  {
    int bit = bitmap.getBit(x, y, bitNum);
    if (bInvert) {
      bit = 1 - bit;
    }
    return bit;
  }

  bool blockIsEmpty(const Bitmap &bitmap,
                    int nBitPlane,
                    int blockX,
                    int blockY,
                    bool bInvert)
  {
    for (int deltaY = 0; deltaY < 16; deltaY++) {
      for (int deltaX = 0; deltaX < 16; deltaX++) {
        int bit = getBitInverted(bitmap,
                                 blockX * 16 + deltaX,
                                 blockY * 16 + deltaY,
                                 nBitPlane,
                                 bInvert);
        if (bit == 1) {
          return false;
        }
      }
    }

    return true;
  }

  void compressBlock(const Bitmap &bitmap,
                     int nBitPlane,
                     int blockX,
                     int blockY,
                     bool bInvert,
                     int compressionType,
                     ByteVec *pBits,
                     int *pBitCount,
                     ByteVec *pBytes)
  {
    addBit(pBits, pBitCount, 1);
    addBit(pBits, pBitCount, (compressionType & 0x2) >> 1);
    addBit(pBits, pBitCount, (compressionType & 0x1));

    if (compressionType == 3) {
      for (int i = 0; i < 32; i++) {
        int byteVal = 0x00;
        for (int b = 0; b < 8; b++) {
          int pixelY = (blockY * 16) + ((i & 0x1e) >> 1);
          int pixelX = (blockX * 16) + ((i & 0x01) * 8) + b;
          int bit = getBitInverted(bitmap,
                                   pixelX,
                                   pixelY,
                                   nBitPlane,
                                   bInvert);
          byteVal |= (bit << (7 - b));
        }

        pBytes->push_back(byteVal);
      }

      return;
    }

    int bXor = (compressionType > 0);
    int xorDiff = compressionType;

    for (int nQuadrant = 0; nQuadrant < 4; nQuadrant++) {
      unsigned char lineBytes[8];

      unsigned char quadrantHeader = 0x00;

      int nLine;
      for (nLine = 0; nLine < 8; nLine++) {
        unsigned char lineByte = 0;

        int pixXstart = (blockX * 16) + (nQuadrant & 0x1) * 8;
        int pixY = (blockY * 16) + (nQuadrant & 0x2) * 4 + nLine;

        int nBit;
        for (nBit = 0; nBit < 8; nBit++) {
          int lineBit = getBitInverted(bitmap,
                                       pixXstart + nBit,
                                       pixY,
                                       nBitPlane,
                                       bInvert);

          if (bXor) {
            if (((nQuadrant & 0x2) * 4 + nLine) >= xorDiff) {
              lineBit ^= getBitInverted(bitmap,
                                        pixXstart + nBit,
                                        pixY - xorDiff,
                                        nBitPlane,
                                        bInvert);
            }
          }

          lineByte |= lineBit << (7 - nBit);
        }

        lineBytes[nLine] = lineByte;

        if (lineByte != 0x00) {
          quadrantHeader |= 1 << (7 - nLine);
        }
      }

      if (quadrantHeader == 0x00) {
        addBit(pBits, pBitCount, 0);
      } else {
        addBit(pBits, pBitCount, 1);
        pBytes->push_back(quadrantHeader);
        for (nLine = 0; nLine < 8; nLine++) {
          unsigned char lineByte = lineBytes[nLine];
          if (lineByte != 0x00) {
            pBytes->push_back(lineByte);
          }
        }
      }
    }
  }

  void compressBlock(const Bitmap &bitmap,
                     int nBitPlane,
                     int blockX,
                     int blockY,
                     bool bInvert,
                     ByteVec *pBits,
                     int *pBitCount,
                     ByteVec *pBytes)
  {
    ByteVec bits;
    int bitCount = 0;
    ByteVec bytes;

    for (int compressionType = 0; compressionType <= 3; compressionType++) {
      bool first = (compressionType == 0);
      ByteVec curBits;
      int curBitCount = 0;
      ByteVec curBytes;

      compressBlock(bitmap,
                    nBitPlane,
                    blockX,
                    blockY,
                    bInvert,
                    compressionType,
                    &curBits,
                    &curBitCount,
                    &curBytes);

      if (first ||
          (curBytes.size() * 8 + curBitCount <
           bytes.size() * 8 + bitCount)) {
        bits.swap(curBits);
        bitCount = curBitCount;
        bytes.swap(curBytes);
      }
    }

    addBits(pBits, pBitCount, bits, bitCount);
    addBytes(pBytes, bytes);
  }

  class BlockCoord
  {
  public:
    BlockCoord(int x, int y);

    bool operator < (const BlockCoord &other) const;

    int m_x;
    int m_y;
  };

  BlockCoord::BlockCoord(int x, int y)
  : m_x(x)
  , m_y(y)
  {
  }

  bool BlockCoord::operator < (const BlockCoord &other) const
  {
    if (m_y != other.m_y) {
      return m_y < other.m_y;
    }

    if (m_x != other.m_x) {
      return m_x < other.m_x;
    }

    return false;
  }

  int blockCompare(const Bitmap &bitmap,
                   int nBitPlane,
                   const BlockCoord &coord1,
                   const BlockCoord &coord2)
  {
    for (int deltaY = 0; deltaY < 16; deltaY++) {
      for (int deltaX = 0; deltaX < 16; deltaX++) {
        int bit1 = bitmap.getBit(coord1.m_x * 16 + deltaX,
                                 coord1.m_y * 16 + deltaY,
                                 nBitPlane);
        int bit2 = bitmap.getBit(coord2.m_x * 16 + deltaX,
                                 coord2.m_y * 16 + deltaY,
                                 nBitPlane);
        if (bit1 < bit2) {
          return -1;
        }
        if (bit1 > bit2) {
          return 1;
        }
      }
    }

    return 0;
  }

  class BlockComp
  {
  public:
    BlockComp(const Bitmap &bitmap, int nBitPlane);

    bool operator () (const BlockCoord &coord1, const BlockCoord &coord2);

  private:
    const Bitmap &m_bitmap;
    int m_nBitPlane;
  };

  BlockComp::BlockComp(const Bitmap &bitmap, int nBitPlane)
  : m_bitmap(bitmap)
  , m_nBitPlane(nBitPlane)
  {
  }

  bool BlockComp::operator () (const BlockCoord &coord1,
                               const BlockCoord &coord2)
  {
    int blockCompResult = blockCompare(m_bitmap, m_nBitPlane, coord1, coord2);
    if (blockCompResult == -1) {
      return true;
    }
    if (blockCompResult == 1) {
      return false;
    }

    return coord1 < coord2;
  }

  void computeBlockCopies(const Bitmap &bitmap,
                          int nBitPlane,
                          bool bInvert,
                          map<BlockCoord, set<BlockCoord> > *pBlockCopyMap,
                          set<BlockCoord> *pBlockCopyDests)
  {
    vector<BlockCoord> sortVec;
    {
      int x, y;
      for (y = 0; y < 25; y++) {
        for (x = 0; x < 40; x++) {
          sortVec.push_back(BlockCoord(x, y));
        }
      }
    }

    sort(sortVec.begin(), sortVec.end(),
         BlockComp(bitmap, nBitPlane));

    int bitsSaved = 0;

    int low = 0;
    int high = 0;
    int sz = sortVec.size();
    while (low < sz) {
      while (high + 1 < sz &&
             blockCompare(bitmap, nBitPlane,
                          sortVec[low], sortVec[high + 1]) == 0) {
        high++;
      }

      if (low < high) {
        int destCount = high - low;
        int x = sortVec[low].m_x;
        int y = sortVec[low].m_y;
        if (!blockIsEmpty(bitmap,
                          nBitPlane,
                          sortVec[low].m_x,
                          sortVec[low].m_y,
                          bInvert)) {
          ByteVec bits;
          int bitCount = 0;
          ByteVec bytes;
          compressBlock(bitmap,
                        nBitPlane,
                        x,
                        y,
                        bInvert,
                        &bits,
                        &bitCount,
                        &bytes);

          int sizeWithoutCopy = (bytes.size() * 8 + bitCount) * destCount;
          int sizeWithCopy = (2 + 2 * destCount + 2) * 8 + destCount;
          if (sizeWithCopy < sizeWithoutCopy) {
            bitsSaved += (sizeWithoutCopy - sizeWithCopy);
            set<BlockCoord> &mapSet = (*pBlockCopyMap)[sortVec[low]];
            for (int nDest = low + 1; nDest <= high; nDest++) {
              mapSet.insert(sortVec[nDest]);
              pBlockCopyDests->insert(sortVec[nDest]);
            }
          }
        }
      }

      low = high + 1;
      high = low;
    }

    if (bitsSaved <= 16) {
      pBlockCopyMap->clear();
      pBlockCopyDests->clear();
    }
  }
}

bool unparseBitmap(const Bitmap &bitmap,
                   const Palette &palette,
                   ByteVec *pOut,
                   string *pMsg)
{
  string msg;
  if (!pMsg) {
    pMsg = &msg;
  }

  int bitDepth = bitmap.getBitDepth();

  vector<ByteVec> unparsedBitPlanes;
  unparsedBitPlanes.resize(bitDepth);

  int nBitPlane;
  for (nBitPlane = 0; nBitPlane < bitDepth; nBitPlane++) {
    bool bInvert = false;
    ByteVec bitStream;
    ByteVec byteStream;
    ByteVec blockCopies;

    for (int invertInt = 0; invertInt <= 1; invertInt++) {
      bool first = (invertInt == 0);
      bool bCurInvert = (invertInt == 1);
      ByteVec curBitStream;
      ByteVec curByteStream;
      ByteVec curBlockCopies;

      int bitCount = 0;

      map<BlockCoord, set<BlockCoord> > blockCopyMap;
      set<BlockCoord> blockCopyDests;

      // Don't use block copies for now because it isn't clear
      // how to do it properly for Bolo.
      if (false) {
        computeBlockCopies(bitmap, nBitPlane, bCurInvert,
                           &blockCopyMap, &blockCopyDests);
      }

      int blockX, blockY;
      for (blockY = 0; blockY < 25; blockY++) {
        bool emptyRow = true;
        for (blockX = 0; blockX < 40; blockX++) {
          if (!blockIsEmpty(bitmap, nBitPlane, blockX, blockY, bCurInvert)) {
            emptyRow = false;
            break;
          }
        }

        if (emptyRow) {
          addBit(&curBitStream, &bitCount, 0);
        } else {
          addBit(&curBitStream, &bitCount, 1);
          for (blockX = 0; blockX < 40; blockX++) {
            if ((blockCopyDests.find(BlockCoord(blockX, blockY)) !=
                blockCopyDests.end()) ||
                (blockIsEmpty(bitmap,
                              nBitPlane,
                              blockX,
                              blockY,
                              bCurInvert))) {
              addBit(&curBitStream, &bitCount, 0);
            } else {
              compressBlock(bitmap, nBitPlane, blockX, blockY, bCurInvert,
                            &curBitStream, &bitCount, &curByteStream);
            }
          }
        }
      }

      if (!blockCopyMap.empty()) {
        map<BlockCoord, set<BlockCoord> >::const_iterator sourceIter =
          blockCopyMap.begin();
        map<BlockCoord, set<BlockCoord> >::const_iterator sourceEnd =
          blockCopyMap.end();
        for (; sourceIter != sourceEnd; ++sourceIter) {
          const BlockCoord &source = sourceIter->first;
          const set<BlockCoord> &dests = sourceIter->second;

          curBlockCopies.push_back((unsigned char)source.m_x);
          curBlockCopies.push_back((unsigned char)source.m_y);

          set<BlockCoord>::const_iterator destIter = dests.begin();
          set<BlockCoord>::const_iterator destEnd = dests.end();
          for (; destIter != destEnd; ++destIter) {
            const BlockCoord &dest = *destIter;

            curBlockCopies.push_back((unsigned char)dest.m_x);
            curBlockCopies.push_back((unsigned char)dest.m_y);
          }

          curBlockCopies.push_back(0xff);
          curBlockCopies.push_back(0xff);
        }

        curBlockCopies.push_back(0xff);
        curBlockCopies.push_back(0xff);
      }

      if (first ||
          (curBitStream.size() + curByteStream.size() + curBlockCopies.size() <
           bitStream.size() + byteStream.size() + blockCopies.size())) {
        bInvert = bCurInvert;
        bitStream.swap(curBitStream);
        byteStream.swap(curByteStream);
        blockCopies.swap(curBlockCopies);
      }
    }

    int bitPlaneSize = 8 + 4 + 2 + 2 + 2 + 2 + 4 + 4 + 2 + 2 + 8 +
      bitStream.size() + byteStream.size() + blockCopies.size();

    ByteVec &bitPlane = unparsedBitPlanes[nBitPlane];
    bitPlane.resize(bitPlaneSize);

    int bitPlaneCur = 0;

    for (int i = 0; i < 8; i++) {
      bitPlane[bitPlaneCur] = bitPlaneTypeName[i];
      bitPlaneCur++;
    }

    putInt4(bitPlane.begin() + bitPlaneCur,
            bitPlaneSize - 8 - blockCopies.size());
    bitPlaneCur += 4;

    putInt2(bitPlane.begin() + bitPlaneCur, 640);
    bitPlaneCur += 2;

    putInt2(bitPlane.begin() + bitPlaneCur, 400);
    bitPlaneCur += 2;

    putInt2(bitPlane.begin() + bitPlaneCur, 40);
    bitPlaneCur += 2;

    putInt2(bitPlane.begin() + bitPlaneCur, 25);
    bitPlaneCur += 2;

    putInt4(bitPlane.begin() + bitPlaneCur, bitStream.size());
    bitPlaneCur += 4;

    putInt4(bitPlane.begin() + bitPlaneCur, byteStream.size());
    bitPlaneCur += 4;

    putInt2(bitPlane.begin() + bitPlaneCur, bInvert ? 0xffff : 0x0000);
    bitPlaneCur += 2;

    bool bHasBlockCopies = !blockCopies.empty();
    putInt2(bitPlane.begin() + bitPlaneCur, bHasBlockCopies ? 0xdead : 0x0000);
    bitPlaneCur += 2;

    for (int iUnusedByte = 0; iUnusedByte < 8; iUnusedByte++) {
      bitPlane[bitPlaneCur] = 0x00;
      bitPlaneCur++;
    }

    copy(bitStream.begin(), bitStream.end(),
         bitPlane.begin() + bitPlaneCur);
    bitPlaneCur += bitStream.size();

    copy(byteStream.begin(), byteStream.end(),
         bitPlane.begin() + bitPlaneCur);
    bitPlaneCur += byteStream.size();

    copy(blockCopies.begin(), blockCopies.end(),
         bitPlane.begin() + bitPlaneCur);
    bitPlaneCur += blockCopies.size();

    if (bitPlaneCur != (int)bitPlane.size()) {
      pMsg->assign("Error computing size of unparsed bit plane.");
      return false;
    }
  }

  int totalSize = 2 + (2 * bitDepth) + 2 + 96;
  for (nBitPlane = 0; nBitPlane < bitDepth; nBitPlane++) {
    totalSize += unparsedBitPlanes[nBitPlane].size();
  }

  ByteVec out;
  out.resize(totalSize);

  int cur = 0;

  putInt2Reversed(out.begin() + cur, bitDepth);
  cur += 2;

  for (nBitPlane = 0; nBitPlane < bitDepth; nBitPlane++) {
    putInt2Reversed(out.begin() + cur, unparsedBitPlanes[nBitPlane].size());
    cur += 2;
  }

  putInt2Reversed(out.begin() + cur, 0x0010);
  cur += 2;

  if (palette.getNumColors() != 16) {
    pMsg->assign("Palette must have 16 colors.");
    return false;
  }

  for (int nColor = 0; nColor < 16; nColor++) {
    unsigned int red, green, blue;
    palette.getColor(nColor, &red, &green, &blue);

    putInt2Reversed(out.begin() + cur, red);
    cur += 2;

    putInt2Reversed(out.begin() + cur, green);
    cur += 2;

    putInt2Reversed(out.begin() + cur, blue);
    cur += 2;
  }

  for (nBitPlane = 0; nBitPlane < bitDepth; nBitPlane++) {
    copy(unparsedBitPlanes[nBitPlane].begin(),
         unparsedBitPlanes[nBitPlane].end(),
         out.begin() + cur);
    cur += unparsedBitPlanes[nBitPlane].size();
  }

  if (cur != (int)out.size()) {
    pMsg->assign("Error computing size of unparsed bitmap.");
    return false;
  }

  pOut->clear();
  pOut->resize(out.size());
  copy(out.begin(), out.end(), pOut->begin());

  return true;
}

namespace
{
  void toPPM(const Bitmap &bitmap,
             const Palette &palette,
             int nBitPlane,
             ByteVec *pOut)
  {
    pOut->clear();

    pOut->resize(15 + (640 * 400 * 3));

    ByteVec &out = *pOut;

    int cur = 0;
    out[cur++] = 'P';
    out[cur++] = '6';
    out[cur++] = '\n';
    out[cur++] = '6';
    out[cur++] = '4';
    out[cur++] = '0';
    out[cur++] = ' ';
    out[cur++] = '4';
    out[cur++] = '0';
    out[cur++] = '0';
    out[cur++] = '\n';
    out[cur++] = '2';
    out[cur++] = '5';
    out[cur++] = '5';
    out[cur++] = '\n';

    int x, y;
    for (y = 0; y < 400; y++) {
      for (x = 0; x < 640; x++) {
        int nColor;

        if (nBitPlane == -1) {
          nColor = bitmap.getPixel(x, y);
        } else {
          nColor = bitmap.getBit(x, y, nBitPlane);
        }

        unsigned int red, green, blue;
        palette.getColor(nColor, &red, &green, &blue);

        out[cur++] = (red   & 0xff00) >> 8;
        out[cur++] = (green & 0xff00) >> 8;
        out[cur++] = (blue  & 0xff00) >> 8;
      }
    }
  }
}

void bitmapToPPM(const Bitmap &bitmap,
                 const Palette &palette,
                 ByteVec *pOut)
{
  toPPM(bitmap, palette, -1, pOut);
}

void bitPlaneToPPM(const Bitmap &bitmap,
                   int nBitPlane,
                   ByteVec *pOut)
{
  Palette palette;
  palette.setNumColors(2);
  palette.setColor(0, 0x0000, 0x0000, 0x0000);
  palette.setColor(1, 0xffff, 0xffff, 0xffff);

  toPPM(bitmap, palette, nBitPlane, pOut);
}

}
