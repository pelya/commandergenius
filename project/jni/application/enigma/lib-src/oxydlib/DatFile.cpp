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

#include "DatFile.h"
#include "VecUtils.h"
#include <stdio.h>
#include <algorithm>

namespace OxydLib {

using namespace std;

namespace
{
  unsigned char levelShifts[OxydVersion_Count][16] =
  {{0x45, 0x23, 0xEA, 0xB9, 0x11, 0xF1, 0x9F, 0x5A,
    0x33, 0x3E, 0x0F, 0xB4, 0x41, 0x56, 0xAF, 0xAA},

   {0x45, 0x23, 0xEA, 0xB9, 0x11, 0xF1, 0x9F, 0x5A,
    0x33, 0x3E, 0x0F, 0xB4, 0x41, 0x56, 0xAF, 0xAA},

   {0x45, 0x23, 0xEA, 0xB9, 0x11, 0xF1, 0x9F, 0x5A,
    0x33, 0x3E, 0x0F, 0xB4, 0x41, 0x56, 0xAF, 0xAA},

   {0xAA, 0xE1, 0x82, 0x34, 0xAF, 0x24, 0x72, 0x42,
    0x37, 0x3E, 0x83, 0x98, 0x22, 0xFF, 0x2F, 0x31},

   {0x45, 0x23, 0xEA, 0xB9, 0x11, 0xF1, 0x9F, 0x5A,
    0x33, 0x3E, 0x0F, 0xB4, 0x41, 0x56, 0xAF, 0xAA}};

  void unshiftBytes(ByteVec *pVec, OxydVersion ver)
  {
    for (int nByte = 0; nByte < (int)pVec->size(); nByte++) {
      (*pVec)[nByte] =
        (int((*pVec)[nByte]) + 256 - levelShifts[ver][nByte % 16]) % 256;
    }
  }

  void shiftBytes(ByteVec *pVec, OxydVersion ver)
  {
    for (int nByte = 0; nByte < (int)pVec->size(); nByte++) {
      (*pVec)[nByte] =
        (int((*pVec)[nByte]) + levelShifts[ver][nByte % 16]) % 256;
    }
  }

  void unscrambleChunkTableData(ByteVec::iterator it)
  {
    unsigned int val = getInt4(it);
    unsigned int valPrime = 0xffffffff &
      ~(((val & 0x1fffffff) << 3) +
        ((val & 0xe0000000) >> 29));
    putInt4(it, valPrime);
  }

  void scrambleChunkTableData(ByteVec::iterator it)
  {
    unsigned int val = getInt4(it);
    unsigned int valPrime = 0xffffffff &
      ~(((val & 0xfffffff8) >> 3) +
        ((val & 0x00000007) << 29));
    putInt4(it, valPrime);
  }
}

DatFile::DatFile()
{
  clear();
}

DatFile::~DatFile()
{
}

void DatFile::clear()
{
  m_bInit = false;

  m_ver = OxydVersion_Oxyd1;
  m_credits.clear();
  m_codes.clear();
  m_levels.clear();
  m_levels.resize(200);
  m_chunks.clear();
}

void DatFile::getChunkNames(set<string> *pChunkNames) const
{
  pChunkNames->clear();

  ChunkMap::const_iterator iter = m_chunks.begin();
  ChunkMap::const_iterator end = m_chunks.end();
  for (; iter != end; ++iter) {
    pChunkNames->insert(iter->first);
  }
}

void DatFile::addChunk(const string &chunkName)
{
  m_chunks[chunkName];
}

void DatFile::removeChunk(const string &chunkName)
{
  m_chunks.erase(chunkName);
}

const ByteVec *DatFile::getChunk(const string &chunkName) const
{
  ChunkMap::const_iterator iter = m_chunks.find(chunkName);
  if (iter == m_chunks.end()) {
    return 0;
  } else {
    return &iter->second;
  }
}

ByteVec *DatFile::getChunkForWrite(const string &chunkName)
{
  ChunkMap::iterator iter = m_chunks.find(chunkName);
  if (iter == m_chunks.end()) {
    return 0;
  } else {
    return &iter->second;
  }
}

namespace
{
  bool uncompressChunk(const ByteVec &in,
                       ByteVec *pOut,
                       string *pMsg)
  {
    string msg;
    if (!pMsg) {
      pMsg = &msg;
    }

    ByteVec &out = *pOut;

    int compressedSize = in.size();
    int uncompressedSize = out.size();

    if (compressedSize == uncompressedSize) {
      copy(in.begin(), in.end(), out.begin());
    } else {
      unsigned char keyByte = 0;
      int curBit = 0;
      int curIn = 0;
      int curOut = 0;
      int szIn = in.size();
      int szOut = out.size();

      while (curOut < szOut) {
        if (curBit == 0) {
          if (curIn + 1 > szIn) {
            pMsg->assign("Not enough compressed data reading key byte.");
            return false;
          }

          keyByte = in[curIn];
          curIn++;
        }

        if (keyByte & (1 << curBit)) {
          if (curIn + 1 > szIn) {
            pMsg->assign("Not enough compressed data reading literal byte.");
            return false;
          }

          unsigned char literalByte = in[curIn];
          curIn++;

          out[curOut] = literalByte;
          curOut++;
        } else {
          if (curIn + 2 > szIn) {
            pMsg->assign("Not enough compressed data reading compressed byte.");
            return false;
          }

          unsigned char b1 = in[curIn];
          curIn++;
          unsigned char b2 = in[curIn];
          curIn++;

          int outBytes = ((int)(b2 & 0x0f)) + 3;

          if (curOut + outBytes > szOut) {
            pMsg->assign("Too much uncompressed data.");
            return false;
          }

          int offset = ((int)b1) + ((((int)b2) & 0xf0) << 4);

          offset = ((offset + 0x12) & 0x0fff);

          offset |= (curOut & ~0x0fff);

          if (offset >= curOut) {
            offset -= 0x1000;
          }

          if (offset + outBytes < 0) {
            pMsg->assign("Referencing data unnecessarily far before beginning of output.");
            return false;
          }

          for (int i = 0; i < outBytes; i++) {
            if (offset < 0) {
              out[curOut] = 0x20;
            } else {
              out[curOut] = out[offset];
            }
            curOut++;
            offset++;
          }
        }

        curBit = ((curBit + 1) & 0x7);
      }

      if (curIn < szIn) {
        pMsg->assign("Too much compressed data while uncompressing chunk.");
        return false;
      }

      while (curBit != 0) {
        if (keyByte & (1 << curBit)) {
          pMsg->assign("Extra bit in compressed chunk key byte is not 0.");
          return false;
        }

        curBit = ((curBit + 1) & 0x7);
      }
    }

    return true;
  }
}

bool parseDatFile(const ByteVec &in,
                  OxydVersion ver,
                  DatFile *pDatFile,
                  string *pMsg)
{
  string msg;
  if (!pMsg) {
    pMsg = &msg;
  }

  pDatFile->clear();

  pDatFile->setVersion(ver);

  int sz = in.size();
  int cur = 0;

  if (cur + 600 + 4 + 4 + 400 > sz) {
    pMsg->assign("File too small for header.");
    return false;
  }

  ByteVec *pCredits = pDatFile->getCreditsForWrite();
  pCredits->resize(600);
  copy(in.begin() + cur, in.begin() + cur + 600, pCredits->begin());
  cur += 600;

  int levelSkip = getInt4(in.begin() + cur);
  cur += 4;
  if (levelSkip < (4 + 4 + 400)) {
    pMsg->assign("Level skip value is too small.");
    return false;
  }

  ByteVec *pCodes = pDatFile->getCodesForWrite();
  pCodes->resize(4);
  copy(in.begin() + cur, in.begin() + cur + 4, pCodes->begin());
  cur += 4;

  vector<int> levelSizes;
  levelSizes.resize(200);
  int levelSizeTotal = 0;

  int nLevel;
  for (nLevel = 0; nLevel < 200; nLevel++) {
    int levelSize = getInt2(in.begin() + cur);
    cur += 2;
    levelSizes[nLevel] = levelSize;
    levelSizeTotal += levelSize;
  }

  if (levelSkip != 4 + 4 + 400 + levelSizeTotal) {
    pMsg->assign("Level skip value does not correspond to sum of level sizes.");
    return false;
  }

  if (cur + levelSizeTotal > sz) {
    pMsg->assign("File too small for levels.");
    return false;
  }

  for (nLevel = 0; nLevel < 200; nLevel++) {
    ByteVec *pLevelData = pDatFile->getLevelForWrite(nLevel);
    pLevelData->resize(levelSizes[nLevel]);
    copy(in.begin() + cur,
         in.begin() + cur + levelSizes[nLevel],
         pLevelData->begin());
    unshiftBytes(pLevelData, ver);
    cur += levelSizes[nLevel];
  }

  // Chunks
  if (cur + 2 > sz) {
    pMsg->assign("Not enough data reading number of chunks.");
    return false;
  }

  int numChunks = getInt2(in.begin() + cur);
  cur += 2;

  if (cur + (24 * numChunks) > sz) {
    pMsg->assign("Not enough data reading chunk table.");
    return false;
  }

  vector<int> chunkCompressedSizes, chunkUncompressedSizes, chunkOffsets;
  vector<string> chunkNames;
  chunkCompressedSizes.resize(numChunks);
  chunkUncompressedSizes.resize(numChunks);
  chunkOffsets.resize(numChunks);
  chunkNames.resize(numChunks);

  int nChunk;
  for (nChunk = 0; nChunk < numChunks; nChunk++) {
    ByteVec chunkTableEntry;
    chunkTableEntry.resize(24);
    copy(in.begin() + cur, in.begin() + cur + 24,
         chunkTableEntry.begin());
    cur += 24;

    for (int i = 0; i < 24; i += 4) {
      unscrambleChunkTableData(chunkTableEntry.begin() + i);
    }

    chunkCompressedSizes[nChunk] = getInt4(chunkTableEntry.begin() + 0);
    chunkUncompressedSizes[nChunk] = getInt4(chunkTableEntry.begin() + 4);
    chunkOffsets[nChunk] = getInt4(chunkTableEntry.begin() + 8);

    string &chunkName = chunkNames[nChunk];
    for (int nChar = 0; nChar < 12; nChar++) {
      char ch = chunkTableEntry[12 + nChar];
      if ((int)chunkName.size() == nChar) {
        if (ch != 0x00) {
          chunkName.append(1, ch);
        }
      } else {
        if (ch != 0x00) {
          pMsg->assign("Chunk name has non-null byte after null byte.");
          return false;
        }
      }
    }

    if (pDatFile->getChunk(chunkName)) {
      pMsg->assign("Duplicate chunk name.");
      return false;
    }

    pDatFile->addChunk(chunkName);
  }

  int chunkOffset = 2 + (24 * numChunks);

  for (nChunk = 0; nChunk < numChunks; nChunk++) {
    if (chunkOffset != chunkOffsets[nChunk]) {
      pMsg->assign("Unexpected chunk starting point.");
      return false;
    }

    int compressedSize = chunkCompressedSizes[nChunk];
    if (cur + compressedSize > sz) {
      pMsg->assign("Not enough data reading chunk.");
      return false;
    }

    ByteVec compressedChunk;
    compressedChunk.resize(compressedSize);
    copy(in.begin() + cur, in.begin() + cur + compressedSize,
         compressedChunk.begin());
    cur += compressedSize;

    chunkOffset += compressedSize;

    const string &chunkName = chunkNames[nChunk];
    int uncompressedSize = chunkUncompressedSizes[nChunk];
    ByteVec &uncompressedChunk = *pDatFile->getChunkForWrite(chunkName);
    uncompressedChunk.resize(uncompressedSize);

    if (!uncompressChunk(compressedChunk, &uncompressedChunk, pMsg)) {
      return false;
    }
  }

  if (cur < sz) {
    pMsg->assign("Extra data after reading chunks.");
    return false;
  }

  pDatFile->setInit(true);
  return true;
}

namespace
{
  bool compressChunk(const ByteVec &in,
                     ByteVec *pOut,
                     string *pMsg)
  {
    string msg;
    if (!pMsg) {
      pMsg = &msg;
    }

    ByteVec out;

    // We could do some real compression here instead.
    out.resize(in.size());
    copy(in.begin(), in.end(), out.begin());

    pOut->resize(out.size());
    copy(out.begin(), out.end(), pOut->begin());

    return true;
  }
}

bool unparseDatFile(const DatFile &datFile,
                    ByteVec *pOut,
                    string *pMsg)
{
  string msg;
  if (!pMsg) {
    pMsg = &msg;
  }

  if (!datFile.getInit()) {
    pMsg->assign("DatFile not initialized.");
    return false;
  }

  ByteVec out;

  out.clear();

  int cur = 0;

  out.resize(out.size() + 600 + 4 + 4 + 400);

  const ByteVec &credits = datFile.getCredits();
  if (credits.size() != 600) {
    pMsg->assign("Credits must be 600 bytes.");
    return false;
  }
  copy(credits.begin(), credits.end(), out.begin() + cur);
  cur += 600;

  int levelSizeTotal = 0;
  int nLevel;
  for (nLevel = 0; nLevel < 200; nLevel++) {
    levelSizeTotal += datFile.getLevel(nLevel).size();
  }

  putInt4(out.begin() + cur, 4 + 4 + 400 + levelSizeTotal);
  cur += 4;

  const ByteVec &codes = datFile.getCodes();
  if (codes.size() != 4) {
    pMsg->assign("Codes must be 4 bytes.");
    return false;
  }
  copy(codes.begin(), codes.end(), out.begin() + cur);
  cur += 4;

  for (nLevel = 0; nLevel < 200; nLevel++) {
    putInt2(out.begin() + cur, datFile.getLevel(nLevel).size());
    cur += 2;
  }

  out.resize(out.size() + levelSizeTotal);

  for (nLevel = 0; nLevel < 200; nLevel++) {
    const ByteVec &level = datFile.getLevel(nLevel);
    ByteVec encodedLevel;
    encodedLevel.resize(level.size());
    copy(level.begin(), level.end(),
         encodedLevel.begin());
    shiftBytes(&encodedLevel, datFile.getVersion());
    copy(encodedLevel.begin(), encodedLevel.end(),
         out.begin() + cur);
    cur += level.size();
  }

  // Chunks
  set<string> chunkNameSet;
  datFile.getChunkNames(&chunkNameSet);

  int numChunks = chunkNameSet.size();

  vector<string> chunkNames;
  vector<ByteVec> compressedChunks;
  chunkNames.resize(numChunks);
  compressedChunks.resize(numChunks);

  int nChunk = 0;
  set<string>::const_iterator chunkIter = chunkNameSet.begin();
  set<string>::const_iterator chunkEnd = chunkNameSet.end();
  for (; chunkIter != chunkEnd; ++chunkIter) {
    const string &chunkName = *chunkIter;
    chunkNames[nChunk] = chunkName;

    if (!compressChunk(*datFile.getChunk(chunkName),
                       &compressedChunks[nChunk],
                       pMsg)) {
      return false;
    }

    nChunk++;
  }

  out.resize(out.size() + 2 + (24 * numChunks));

  putInt2(out.begin() + cur, numChunks);
  cur += 2;

  int chunkOffset = 2 + (24 * numChunks);

  for (nChunk = 0; nChunk < numChunks; nChunk++) {
    ByteVec chunkTableEntry;
    chunkTableEntry.resize(24);

    const string &chunkName = chunkNames[nChunk];

    int uncompressedSize = datFile.getChunk(chunkName)->size();
    int compressedSize = compressedChunks[nChunk].size();

    putInt4(chunkTableEntry.begin(), compressedSize);
    putInt4(chunkTableEntry.begin() + 4, uncompressedSize);
    putInt4(chunkTableEntry.begin() + 8, chunkOffset);

    if (chunkName.size() > 12) {
      pMsg->assign("Chunk name too long.");
      return false;
    }
    for (int nChar = 0; nChar < 12; nChar++) {
      char ch = 0x00;
      if (nChar < (int)chunkName.size()) {
        ch = chunkName[nChar];
      }
      chunkTableEntry[12 + nChar] = ch;
    }

    for (int i = 0; i < 24; i += 4) {
      scrambleChunkTableData(chunkTableEntry.begin() + i);
    }

    copy(chunkTableEntry.begin(), chunkTableEntry.end(), out.begin() + cur);
    cur += 24;

    chunkOffset += compressedSize;
  }

  for (nChunk = 0; nChunk < numChunks; nChunk++) {
    const ByteVec &compressedChunk = compressedChunks[nChunk];
    int compressedSize = compressedChunk.size();

    out.resize(out.size() + compressedSize);

    copy(compressedChunk.begin(),
         compressedChunk.end(),
         out.begin() + cur);
    cur += compressedSize;
  }

  if (cur != (int)out.size()) {
    pMsg->assign("Error computing size of unparsed data file.");
    return false;
  }

  pOut->clear();
  pOut->resize(out.size());
  copy(out.begin(), out.end(), pOut->begin());

  return true;
}

}
