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

#ifndef DATFILE_H
#define DATFILE_H

#include "VecUtils.h"
#include "OxydVersion.h"
#include <string>
#include <set>
#include <map>

namespace OxydLib {

class DatFile
{
public:
  DatFile();
  ~DatFile();

  void clear();

  bool getInit() const { return m_bInit; }
  void setInit(bool bInit) { m_bInit = bInit; }

  OxydVersion getVersion() const { return m_ver; }
  void setVersion(OxydVersion ver) { m_ver = ver; }

  const ByteVec &getCredits() const { return m_credits; }
  ByteVec *getCreditsForWrite() { return &m_credits; }

  const ByteVec &getCodes() const { return m_codes; }
  ByteVec *getCodesForWrite() { return &m_codes; }

  const ByteVec &getLevel(int nLevel) const { return m_levels[nLevel]; }
  ByteVec *getLevelForWrite(int nLevel) { return &m_levels[nLevel]; }

  void getChunkNames(std::set<std::string> *pChunkNames) const;
  void addChunk(const std::string &chunkName);
  void removeChunk(const std::string &chunkName);

  const ByteVec *getChunk(const std::string &chunkName) const;
  ByteVec *getChunkForWrite(const std::string &chunkName);

private:
  bool m_bInit;

  OxydVersion m_ver;

  ByteVec m_credits;
  ByteVec m_codes;

  std::vector<ByteVec> m_levels;

  typedef std::map<std::string, ByteVec> ChunkMap;
  ChunkMap m_chunks;
};

bool parseDatFile(const ByteVec &in,
                  OxydVersion ver,
                  DatFile *pDatFile,
                  std::string *pMsg = 0);
bool unparseDatFile(const DatFile &datFile,
                    ByteVec *pOut,
                    std::string *pMsg = 0);

}

#endif
