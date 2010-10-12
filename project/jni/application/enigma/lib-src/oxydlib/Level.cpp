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

#include "Level.h"
#include <stdio.h>

namespace OxydLib {

using namespace std;

Marble::Marble()
: m_marbleType(MarbleType_Black)
, m_x(0)
, m_y(0)
, m_data()
{
}

Marble::~Marble()
{
}

RubberBand::RubberBand()
: m_naturalLength(0)
, m_force(0)
, m_firstEndMarble(0)
, m_bSecondEndIsMarble(false)
, m_secondEndMarble(0)
, m_secondEndPieceX(0)
, m_secondEndPieceY(0)
{
}

RubberBand::~RubberBand()
{
}

void RubberBand::setSecondEndMarble(int nMarble)
{
  m_bSecondEndIsMarble = true;
  m_secondEndMarble = nMarble;
  m_secondEndPieceX = 0;
  m_secondEndPieceY = 0;
}

void RubberBand::setSecondEndPiece(unsigned int x, unsigned int y)
{
  m_bSecondEndIsMarble = false;
  m_secondEndMarble = 0;
  m_secondEndPieceX = x;
  m_secondEndPieceY = y;
}

bool RubberBand::operator == (const RubberBand &other) const
{
  if (getNaturalLength() != other.getNaturalLength()) {
    return false;
  }

  if (getForce() != other.getForce()) {
    return false;
  }

  if (getFirstEndMarble() != other.getFirstEndMarble()) {
    return false;
  }

  if (isSecondEndMarble() != other.isSecondEndMarble()) {
    return false;
  }

  if (isSecondEndMarble()) {
    if (getSecondEndMarble() != other.getSecondEndMarble()) {
      return false;
    }
  } else {
    if (getSecondEndPieceX() != other.getSecondEndPieceX() ||
        getSecondEndPieceY() != other.getSecondEndPieceY()) {
      return false;
    }
  }

  return true;
}

bool RubberBand::operator != (const RubberBand &other) const
{
  return !(*this == other);
}

Block::Block()
: m_x(0)
, m_y(0)
{
}

Block::Block(unsigned int x, unsigned int y)
: m_x(x)
, m_y(y)
{
}

Block::~Block()
{
}

bool Block::operator < (const Block &other) const
{
  if (getY() != other.getY()) {
    return getY() < other.getY();
  }

  if (getX() != other.getX()) {
    return getX() < other.getX();
  }

  return false;
}

bool Block::operator == (const Block &other) const
{
  if (getX() != other.getX()) {
    return false;
  }

  if (getY() != other.getY()) {
    return false;
  }

  return true;
}

bool Block::operator != (const Block &other) const
{
  return !(*this == other);
}

ScrambleItem::ScrambleItem()
: m_x(0)
, m_y(0)
, m_dir(Direction_Up)
{
}

ScrambleItem::ScrambleItem(unsigned int x, unsigned int y, Direction dir)
: m_x(x)
, m_y(y)
, m_dir(dir)
{
}

ScrambleItem::~ScrambleItem()
{
}

Laser::Laser()
: m_dir(Direction_Up)
, m_bOn(false)
{
}

Laser::Laser(Direction dir, bool bOn)
: m_dir(dir)
, m_bOn(bOn)
{
}

Laser::~Laser()
{
}

bool Laser::operator == (const Laser &other) const
{
  if (getDir() != other.getDir()) {
    return false;
  }

  if (getOn() != other.getOn()) {
    return false;
  }

  return true;
}

bool Laser::operator != (const Laser &other) const
{
  return !(*this == other);
}

Oscillator::Oscillator()
: m_period(0)
, m_bOn(true)
{
}

Oscillator::Oscillator(unsigned int period, bool bOn)
: m_period(period)
, m_bOn(bOn)
{
}

Oscillator::~Oscillator()
{
}

bool Oscillator::operator == (const Oscillator &other) const
{
  if (getPeriod() != other.getPeriod()) {
    return false;
  }

  if (getOn() != other.getOn()) {
    return false;
  }

  return true;
}

bool Oscillator::operator != (const Oscillator &other) const
{
  return !(*this == other);
}

SignalLocation::SignalLocation()
: m_x(0)
, m_y(0)
, m_gridType(GridType_Pieces)
{
}

SignalLocation::SignalLocation(unsigned int x, unsigned int y,
                               GridType gridType)
: m_x(x)
, m_y(y)
, m_gridType(gridType)
{
}

SignalLocation::~SignalLocation()
{
}

bool SignalLocation::operator < (const SignalLocation &other) const
{
  if (getY() != other.getY()) {
    return getY() < other.getY();
  }

  if (getX() != other.getX()) {
    return getX() < other.getX();
  }

  if (getGridType() != other.getGridType()) {
    return getGridType() < other.getGridType();
  }

  return false;
}

bool SignalLocation::operator == (const SignalLocation &other) const
{
  return
    getX() == other.getX() &&
    getY() == other.getY() &&
    getGridType() == other.getGridType();
}

Grid::Grid()
: m_width(0)
, m_height(0)
, m_data()
{
}

Grid::Grid(int width, int height)
: m_width(0)
, m_height(0)
, m_data()
{
  resize(width, height);
}

Grid::~Grid()
{
}

void Grid::resize(unsigned int width, unsigned int height)
{
  unsigned int oldWidth = m_width;
  unsigned int oldHeight = m_height;
  ByteVec oldData;
  oldData.swap(m_data);

  m_width = width;
  m_height = height;
  m_data.resize(width * height);

  for (unsigned int y = 0; y < getHeight(); y++) {
    for (unsigned int x = 0; x < getWidth(); x++) {
      if (x < oldWidth && y < oldHeight) {
        set(x, y, oldData[y * oldWidth + x]);
      } else {
        set(x, y, 0x00);
      }
    }
  }
}

unsigned char Grid::get(unsigned int x, unsigned int y) const
{
  return m_data[y * getWidth() + x];
}

void Grid::set(unsigned int x, unsigned int y, unsigned char val)
{
  m_data[y * getWidth() + x] = val;
}

namespace
{
  enum
  {
    INFOSURFACES = 0x0000,
    INFOGENERAL  = 0x0001,
    INFOSIGNAL   = 0x0002,
    INFOEND      = 0x0003,
    INFOPIECES   = 0x0004,
    INFOSPECIAL  = 0x0005,
    INFOOBJECTS  = 0x0006,

    NUMINFOTYPES
  };

  bool unpackSignalLocation(const Level &level,
                            unsigned int packedSignalLocation,
                            SignalLocation *pSignalLocation,
                            bool *pWithinGrid,
                            string *pMsg)
  {
    if (packedSignalLocation & 0xc000) {
      if (pMsg) {
        pMsg->assign("High bits set in signal location.");
      }
      return false;
    }

    GridType gridType =
      ((packedSignalLocation & 0x2000) == 0) ?
      GridType_Pieces : GridType_Objects;

    unsigned int blockNum = packedSignalLocation & 0x1fff;
    unsigned int gridSize = level.getWidth() * level.getHeight();
    if (blockNum >= gridSize) {
      // This can happen.
      /*
      if (pMsg) {
        pMsg->assign("Signal block number too high.");
      }
      return false;
      */

      *pWithinGrid = false;
      return true;
    }

    unsigned int x = blockNum % level.getWidth();
    unsigned int y = blockNum / level.getWidth();

    pSignalLocation->setX(x);
    pSignalLocation->setY(y);
    pSignalLocation->setGridType(gridType);
    *pWithinGrid = true;
    return true;
  }

  bool packSignalLocation(const Level &level,
                          const SignalLocation &signalLocation,
                          unsigned int *pPackedSignalLocation,
                          string *pMsg)
  {
    unsigned int blockNum =
      signalLocation.getY() * level.getWidth() + signalLocation.getX();

    if (blockNum > 0x1fff) {
      if (pMsg) {
        pMsg->assign("Signal block number too high.");
      }
      return false;
    }

    unsigned int gridTypeBit = 0;
    switch (signalLocation.getGridType()) {
    case GridType_Pieces: gridTypeBit = 0; break;
    case GridType_Objects: gridTypeBit = 1; break;
    default:
      if (pMsg) {
        pMsg->assign("Invalid grid type for signal location.");
      }
      return false;
    }

    *pPackedSignalLocation =
      (gridTypeBit ? 0x2000 : 0x0000) |
      blockNum;

    return true;
  }

  Laser s_defaultLasers[3] = {
    Laser(Direction_Up, false),
    Laser(Direction_Down, false),
    Laser(Direction_Right, false)
  };
}

Level::Level()
{
  clear();
}

Level::~Level()
{
}

void Level::clear()
{
  m_bInit = false;
  m_bEmpty = true;

  m_width = 0;
  m_height = 0;

  m_bMeditation = false;
  m_bHarmlessMeditationMarbles = false;

  m_marbles.clear();

  m_blackRubberBandObjectMarble = 1;
  m_whiteRubberBandObjectMarble = 0;
  m_bRequireMagicPiece = false;
  m_bScrolling = false;
  m_bWalkThroughPuzzle = false;
  m_scrambleItems.clear();

  for (int nLaser = 0; nLaser < 3; nLaser++) {
    m_lasers[nLaser] = s_defaultLasers[nLaser];
  }

  for (int nNote = 0; nNote < 2; nNote++) {
    for (int nLang = Language_First;
         nLang <= Language_Last;
         nLang++) {
      m_noteText[nNote][nLang].assign("");
    }
  }

  for (int gameModeInt = GameMode_First;
       gameModeInt <= GameMode_Last;
       gameModeInt++) {
    GameMode gameMode = GameMode(gameModeInt);
    m_rubberBands[gameMode].clear();
    m_blackRubberBandPieceNaturalLength[gameMode] = -1;
    m_blackRubberBandPieceForce[gameMode] = -1;
    m_whiteRubberBandPieceNaturalLength[gameMode] = -1;
    m_whiteRubberBandPieceForce[gameMode] = -1;
    m_rubberBandObjectNaturalLength[gameMode] = -1;
    m_rubberBandObjectForce[gameMode] = -1;
    m_bReset[gameMode] = false;
    m_flatForce[gameMode] = -1;
    m_slopeForce[gameMode] = -1;
    m_friction[gameMode] = -1;
    m_oscillators[gameMode].clear();
  }

  for (int gridTypeInt = GridType_First;
       gridTypeInt <= GridType_Last;
       gridTypeInt++) {
    GridType gridType = GridType(gridTypeInt);
    m_grids[gridType].resize(0, 0);
  }

  m_specialItems.clear();

  m_signals.clear();
}

void Level::resize(unsigned int width, unsigned int height)
{
  m_width = width;
  m_height = height;

  for (int gridTypeInt = GridType_First;
       gridTypeInt <= GridType_Last;
       gridTypeInt++) {
    GridType gridType = GridType(gridTypeInt);
    m_grids[gridType].resize(width, height);
  }
}

int Level::getNumRubberBands(GameMode gameMode) const
{
  return m_rubberBands[gameMode].size();
}

const RubberBand &Level::getRubberBand(GameMode gameMode,
                                       int nRubberBand) const
{
  return m_rubberBands[gameMode][nRubberBand];
}

RubberBand *Level::getRubberBandForWrite(GameMode gameMode,
                                         int nRubberBand)
{
  return &m_rubberBands[gameMode][nRubberBand];
}

void Level::addRubberBand(GameMode gameMode,
                          const RubberBand &rubberBand,
                          int nRubberBand)
{
  vector<RubberBand> &vec = m_rubberBands[gameMode];

  if (nRubberBand < 0) {
    nRubberBand = vec.size();
  }

  vec.insert(vec.begin() + nRubberBand, rubberBand);
}

void Level::removeRubberBand(GameMode gameMode, int nRubberBand)
{
  vector<RubberBand> &vec = m_rubberBands[gameMode];
  vec.erase(vec.begin() + nRubberBand);
}

int Level::getBlackRubberBandPieceNaturalLength(GameMode gameMode) const
{
  return m_blackRubberBandPieceNaturalLength[gameMode];
}

void Level::setBlackRubberBandPieceNaturalLength(GameMode gameMode,
                                                 int naturalLength)
{
  m_blackRubberBandPieceNaturalLength[gameMode] = naturalLength;
}

int Level::getBlackRubberBandPieceForce(GameMode gameMode) const
{
  return m_blackRubberBandPieceForce[gameMode];
}

void Level::setBlackRubberBandPieceForce(GameMode gameMode, int force)
{
  m_blackRubberBandPieceForce[gameMode] = force;
}

int Level::getWhiteRubberBandPieceNaturalLength(GameMode gameMode) const
{
  return m_whiteRubberBandPieceNaturalLength[gameMode];
}

void Level::setWhiteRubberBandPieceNaturalLength(GameMode gameMode,
                                                 int naturalLength)
{
  m_whiteRubberBandPieceNaturalLength[gameMode] = naturalLength;
}

int Level::getWhiteRubberBandPieceForce(GameMode gameMode) const
{
  return m_whiteRubberBandPieceForce[gameMode];
}

void Level::setWhiteRubberBandPieceForce(GameMode gameMode, int force)
{
  m_whiteRubberBandPieceForce[gameMode] = force;
}

int Level::getRubberBandObjectNaturalLength(GameMode gameMode) const
{
  return m_rubberBandObjectNaturalLength[gameMode];
}

void Level::setRubberBandObjectNaturalLength(GameMode gameMode,
                                             int naturalLength)
{
  m_rubberBandObjectNaturalLength[gameMode] = naturalLength;
}

int Level::getRubberBandObjectForce(GameMode gameMode) const
{
  return m_rubberBandObjectForce[gameMode];
}

void Level::setRubberBandObjectForce(GameMode gameMode, int force)
{
  m_rubberBandObjectForce[gameMode] = force;
}

int Level::getBlackRubberBandObjectMarble() const
{
  return m_blackRubberBandObjectMarble;
}

void Level::setBlackRubberBandObjectMarble(int nMarble)
{
  m_blackRubberBandObjectMarble = nMarble;
}

int Level::getWhiteRubberBandObjectMarble() const
{
  return m_whiteRubberBandObjectMarble;
}

void Level::setWhiteRubberBandObjectMarble(int nMarble)
{
  m_whiteRubberBandObjectMarble = nMarble;
}

int Level::getNumScrambleItems() const
{
  return m_scrambleItems.size();
}

const ScrambleItem &Level::getScrambleItem(int nScrambleItem) const
{
  return m_scrambleItems[nScrambleItem];
}

ScrambleItem *Level::getScrambleItemForWrite(int nScrambleItem)
{
  return &m_scrambleItems[nScrambleItem];
}

void Level::addScrambleItem(const ScrambleItem &scrambleItem,
                            int nScrambleItem)
{
  vector<ScrambleItem> &vec = m_scrambleItems;

  if (nScrambleItem < 0) {
    nScrambleItem = vec.size();
  }

  vec.insert(vec.begin() + nScrambleItem, scrambleItem);
}

void Level::removeScrambleItem(int nScrambleItem)
{
  vector<ScrambleItem> &vec = m_scrambleItems;
  vec.erase(vec.begin() + nScrambleItem);
}

const std::string &Level::getNoteText(int nNote, Language lang) const
{ 
  const std::string &note = m_noteText[nNote][lang];
  if (note == "" && lang != Language_International)
    return m_noteText[nNote][Language_International];
  return note;
}

void Level::setNoteText(int nNote, Language lang, const std::string &noteText)
{ 
  m_noteText[nNote][lang].assign(noteText); 
}


const Laser &Level::getLaser(int nLaser) const
{
  return m_lasers[nLaser];
}

void Level::setLaser(int nLaser, const Laser &laser)
{
  m_lasers[nLaser] = laser;
}

const OscillatorMap &Level::getOscillators(GameMode gameMode) const
{
  return m_oscillators[gameMode];
}

OscillatorMap *Level::getOscillatorsForWrite(GameMode gameMode)
{
  return &m_oscillators[gameMode];
}

void Level::getSenders(set<SignalLocation> *pSenders) const
{
  pSenders->clear();
  SignalMap::const_iterator iter = m_signals.begin();
  SignalMap::const_iterator end = m_signals.end();
  for (; iter != end; ++iter) {
    pSenders->insert(iter->first);
  }
}

int Level::getNumRecipients(const SignalLocation &sender) const
{
  SignalMap::const_iterator findIter = m_signals.find(sender);
  if (findIter == m_signals.end()) {
    return 0;
  } else {
    return findIter->second.size();
  }
}

const SignalLocation &Level::getRecipient(const SignalLocation &sender,
                                          int nRecipient) const
{
  SignalMap::const_iterator findIter = m_signals.find(sender);
  if (findIter == m_signals.end()) {
    return *(SignalLocation *)0;
  } else {
    return findIter->second[nRecipient];
  }
}

void Level::setRecipient(const SignalLocation &sender,
                         int nRecipient,
                         const SignalLocation &recipient)
{
  SignalMap::iterator findIter = m_signals.find(sender);
  SignalLocation *pRecipient = 0;
  if (findIter != m_signals.end()) {
    pRecipient = &findIter->second[nRecipient];
  }
  *pRecipient = recipient;
}

void Level::addRecipient(const SignalLocation &sender,
                         const SignalLocation &recipient,
                         int nRecipient)
{
  vector<SignalLocation> &vec = m_signals[sender];

  if (nRecipient < 0) {
    nRecipient = vec.size();
  }

  vec.insert(vec.begin() + nRecipient, recipient);
}

void Level::removeRecipient(const SignalLocation &sender,
                            int nRecipient)
{
  vector<SignalLocation> &vec = m_signals[sender];
  vec.erase(vec.begin() + nRecipient);
  if (vec.empty()) {
    m_signals.erase(sender);
  }
}

namespace
{
  bool parseGrid(const ByteVec &compressedGrid,
                 Grid *pGrid,
                 string *pMsg)
  {
    string msg;
    if (!pMsg) {
      pMsg = &msg;
    }

    int gridSize = pGrid->getHeight() * pGrid->getWidth();

    ByteVec data;
    data.resize(gridSize);

    int curIn = compressedGrid.size() - 1;
    int curOut = gridSize - 1;

    while (curIn >= 0) {
      unsigned char curByte = compressedGrid[curIn];
      curIn--;

      switch (curByte) {
      case 0xFF: // long repeat
        {
          if (curIn - 3 < -1) {
            pMsg->assign("Unexpected end of compressed grid data.");
            return false;
          }

          unsigned char repeatByte = compressedGrid[curIn];
          curIn--;

          int repeatCount = compressedGrid[curIn];
          curIn--;
          repeatCount <<= 8;
          repeatCount += compressedGrid[curIn];
          repeatCount++;
          curIn--;

          if (curOut - repeatCount < -1) {
            pMsg->assign("Too much compresed grid data writing repeated bytes.");
            return false;
          }

          int ii;
          for (ii = 0; ii < repeatCount; ii++) {
            data[curOut] = repeatByte;
            curOut--;
          }
        }
        break;
      case 0xFD: // short repeat
        {
          if (curIn - 2 < -1) {
            pMsg->assign("Unexpected end of compressed grid data.");
            return false;
          }

          unsigned char repeatByte = compressedGrid[curIn];
          curIn--;

          int repeatCount = compressedGrid[curIn] + 1;
          curIn--;

          if (curOut - repeatCount < -1) {
            pMsg->assign("Too much compresed grid data writing repeated bytes.");
            return false;
          }

          int ii;
          for (ii = 0; ii < repeatCount; ii++) {
            data[curOut] = repeatByte;
            curOut--;
          }
        }
        break;
      case 0xFE: // pattern
        {
          if (curIn - 3 < -1) {
            pMsg->assign("Unexpected end of compressed grid data.");
            return false;
          }

          unsigned char patternBytes[2];

          patternBytes[0] = compressedGrid[curIn];
          curIn--;

          patternBytes[1] = compressedGrid[curIn];
          curIn--;

          int patternCount = compressedGrid[curIn] + 1;
          curIn--;

          int bitPatternSize = (patternCount + 7) / 8;
          if (curIn - bitPatternSize < -1) {
            pMsg->assign("Unexpected end of compressed grid data.");
            return false;
          }

          if (curOut - patternCount < -1) {
            pMsg->assign("Too much compresed grid data writing repeated bytes.");
            return false;
          }

          int ii;
          for (ii = patternCount - 1; ii >= 0; ii--) {
            unsigned char bitPatternByte = compressedGrid[curIn];
            int bitNum = ii % 8;
            int bit = (bitPatternByte & (1 << bitNum)) >> bitNum;
            data[curOut] = patternBytes[bit];
            curOut--;

            if (ii % 8 == 0) {
              curIn--;
            }
          }
        }
        break;
      default:
        {
          if (curOut - 1 < -1) {
            pMsg->assign("Too much compresed grid data writing literal byte.");
            return false;
          }

          data[curOut] = curByte;
          curOut--;
        }
      }
    }

    if (curOut >= 0) {
      pMsg->assign("Not enough compressed grid data.");
      return false;
    }

    for (unsigned int y = 0; y < pGrid->getHeight(); y++) {
      for (unsigned int x = 0; x < pGrid->getWidth(); x++) {
        pGrid->set(x, y, data[y * pGrid->getWidth() + x]);
      }
    }

    return true;
  }

  bool unparseGrid(const Grid &grid,
                   ByteVec *pCompressedGrid,
                   string *pMsg)
  {
    string msg;
    if (!pMsg) {
      pMsg = &msg;
    }

    // No compression for now
    pCompressedGrid->resize(grid.getWidth() * grid.getHeight());
    for (unsigned int y = 0; y < grid.getHeight(); y++) {
      for (unsigned int x = 0; x < grid.getWidth(); x++) {
        (*pCompressedGrid)[y * grid.getWidth() + x] = grid.get(x, y);
      }
    }

    return true;
  }

  void unparseNumberList(const vector<int> &numberList,
                         string *pStr)
  {
    for (int i = 0; i < (int)numberList.size(); i++) {
      int num = numberList[i];
      pStr->append(1, '(');
      if (num != -1) {
        char buf[1000];
        sprintf(buf, "%d", num);
        pStr->append(buf);
      }
      pStr->append(1, ')');
    }
  }
}


bool parseNumberList(const string &str,
                     vector<int> *pNumberList)
{
    pNumberList->clear();

    int cur = 0;
    int sz = str.size();
    while (cur < sz) {
        if (str[cur] != '(') {
            return false;
        }
        cur++;

        bool bEmpty = true;
        int num = 0;
        while (cur < sz && str[cur] >= '0' && str[cur] <= '9') {
            bEmpty = false;
            num *= 10;
            num += (str[cur] - '0');
            cur++;
        }

        if (cur < sz) {
            if (str[cur] == ')') {
                pNumberList->push_back(bEmpty ? -1 : num);
                cur++;
            } else {
                return false;
            }
        } else {
            return false;
        }
    }

    return true;
}

namespace
{
  class CommandParser {
    string            m_cmd;
    string::iterator  m_iter;

  public:
    CommandParser (const string &cmd)
      : m_cmd(cmd), m_iter(m_cmd.begin())
    {}

    char get_char () {
      char c = 0;
      if (m_iter != m_cmd.end()) {
        c = *m_iter;
        ++m_iter;
      }
      return c;
    }


    bool get_bool(bool default_) {
      unsigned char c = get_char();
      switch (c) {
      case 0xf8: return false;
      case 0xf9: return true;
      default: return default_;
      }
    }

    int get_int(int default_) {
        return default_;
    }
  };
}


bool parseLevel(const ByteVec &in, Level *pLevel, string *pMsg)
{
  string msg;
  if (!pMsg) {
    pMsg = &msg;
  }

  pLevel->clear();

  if (in.empty()) {
    pLevel->setInit(true);
    pLevel->setEmpty(true);
    return true;
  }

  pLevel->setEmpty(false);

  int sz = in.size();

  bool found[NUMINFOTYPES];
  int nFound;
  for (nFound = 0; nFound < NUMINFOTYPES; nFound++) {
    found[nFound] = false;
  }

  int cur1 = 0;

  ByteVec infoArr[NUMINFOTYPES];

  bool foundAll = false;
  do {
    if (cur1 + 2 > sz) {
      pMsg->assign("Unexpected end of level reading info type code.");
      return false;
    }

    unsigned int infoType = getInt2(in.begin() + cur1);
    cur1 += 2;

    if (infoType >= NUMINFOTYPES) {
      pMsg->assign("Unexpected info type code.");
      return false;
    }

    if (found[infoType]) {
      pMsg->assign("Duplicate info type found.");
      return false;
    }

    found[infoType] = true;

    int infoSize = 0;

    switch (infoType) {
    case INFOSURFACES:
    case INFOSIGNAL:
    case INFOPIECES:
    case INFOSPECIAL:
    case INFOOBJECTS:
      {
        if (cur1 + 2 > sz) {
          pMsg->assign("Unexpected end of level reading info length.");
          return false;
        }

        infoSize = getInt2(in.begin() + cur1);
        cur1 += 2;

      }
      break;
    case INFOGENERAL:
      {
        infoSize = 68;
        break;
      }
    case INFOEND:
      {
        infoSize = 0;
        break;
      }
    }

    if (cur1 + infoSize > sz) {
      pMsg->assign("Unexpected end of level reading info data.");
      return false;
    }

    infoArr[infoType].resize(infoSize);
    copy(in.begin() + cur1, in.begin() + cur1 + infoSize,
         infoArr[infoType].begin());
    cur1 += infoSize;

    foundAll = true;
    for (nFound = 0; nFound < NUMINFOTYPES; nFound++) {
      foundAll = foundAll && found[nFound];
    }

    if (infoType == INFOEND && !foundAll) {
      pMsg->assign("Info type info-end found too soon.");
      return false;
    }
  } while (!foundAll);

  if (cur1 != sz) {
    pMsg->assign("Unexpected data at the end of level.");
    return false;
  }

  ByteVec &infoGeneral = infoArr[INFOGENERAL];
  unsigned int width = getInt2(infoGeneral.begin() + 0x00);
  unsigned int height = getInt2(infoGeneral.begin() + 0x02);
  pLevel->resize(width, height);

  for (int nMarble = 0; nMarble < 8; nMarble++) {
    if (infoGeneral[0x04 + (nMarble * 8) + 0x02] != 0x00 ||
        infoGeneral[0x04 + (nMarble * 8) + 0x03] != 0x00 ||
        infoGeneral[0x04 + (nMarble * 8) + 0x06] != 0x00 ||
        infoGeneral[0x04 + (nMarble * 8) + 0x07] != 0x00) {
      pMsg->assign("Unexpected non-zero byte in info-general.");
      return false;
    }
  }

  {
    const ByteVec &infoSpecialVec = infoArr[INFOSPECIAL];

    if (infoSpecialVec.size() < 1) {
      pMsg->assign("Info special is too small.");
      return false;
    }

    {
      for (int i = 0; i < (int)infoSpecialVec.size() - 1; i++) {
        if (infoSpecialVec[i] == 0x00) {
          pMsg->assign("Info special contains a 0x00 byte before the end.");
          return false;
        }
      }
    }

    if (infoSpecialVec[infoSpecialVec.size() - 1] != 0x00) {
      pMsg->assign("Info special does not end with 0x00.");
      return false;
    }

    string infoSpecial;
    infoSpecial.resize(infoSpecialVec.size() - 1);
    copy(infoSpecialVec.begin(),
         infoSpecialVec.begin() + (infoSpecialVec.size() - 1),
         infoSpecial.begin());

    //printf("%s\n", infoSpecial.c_str());

    vector<string> specialItems;

    int start = 0;
    int infoSpecialSize = infoSpecial.size();
    while (start < infoSpecialSize) {
      while (start < infoSpecialSize &&
             infoSpecial[start] == 0x20) {
        start++;
      }

      if (start < infoSpecialSize) {
        int end = start + 1;
        bool inQuote = false;
        while (end < infoSpecialSize &&
               (infoSpecial[end] != 0x20 || inQuote)) {
          if (infoSpecial[end] == '"') {
            inQuote = !inQuote;
          }
          end++;
        }

        // This can happen
        /*
        if (inQuote) {
          pMsg->assign("Quote not terminated in info-special.");
          return false;
        }
        */

        string specialItem = infoSpecial.substr(start, end - start);
        specialItems.push_back(specialItem);

        start = end;
      }
    }

    /*
    {
      for (int nItem = 0; nItem < (int)specialItems.size(); nItem++) {
        printf("  %s\n", specialItems[nItem].c_str());
      }
    }
    */

    GameMode gameMode = GameMode_Invalid;

    int numMarblesArr[GameMode_Count];
    int numExtraRubberBandsArr[GameMode_Count];
    {
      for (int curGameModeInt = GameMode_First;
           curGameModeInt <= GameMode_Last;
           curGameModeInt++) {
        GameMode curGameMode = GameMode(curGameModeInt);
        numMarblesArr[curGameMode] = 0;
        numExtraRubberBandsArr[curGameMode] = 0;
      }
    }

    bool foundLaserArr[3];
    {
      for (int nLaser = 0; nLaser < 3; nLaser++) {
        foundLaserArr[nLaser] = false;
      }
    }

    for (int nItem = 0; nItem < (int)specialItems.size(); nItem++) {
      const string &item = specialItems[nItem];

      // Easy and hard game mode
      if (item.compare("[") == 0) {
        if (gameMode != GameMode_Invalid) {
          pMsg->assign("Hard game mode begin not expected.");
          return false;
        }
        gameMode = GameMode_Hard;
        continue;
      }

      if (item.compare("]") == 0) {
        if (gameMode != GameMode_Hard) {
          pMsg->assign("Hard game mode end not expected.");
          return false;
        }
        gameMode = GameMode_Invalid;
        continue;
      }

      if (item.compare("{") == 0) {
        if (gameMode != GameMode_Invalid) {
          pMsg->assign("Easy game mode begin not expected.");
          return false;
        }
        gameMode = GameMode_Easy;
        continue;
      }

      if (item.compare("}") == 0) {
        if (gameMode != GameMode_Easy) {
          pMsg->assign("Easy game mode end not expected.");
          return false;
        }
        gameMode = GameMode_Invalid;
        continue;
      }

      // Goal
      if (item[0] == 'G') {
        if (item.size() == 2 && item[1] == 'M') {
          if (gameMode != GameMode_Invalid) {
            pMsg->assign("Goal must be the same in all game modes.");
            return false;
          }
          pLevel->setMeditation(true);
          continue;
        }
        if (item.size() >= 2 && item[1] == 'N') {
          if (gameMode != GameMode_Invalid) {
            pMsg->assign("Goal must be the same in all game modes.");
            return false;
          }
          pLevel->setMeditation(false);
          continue;
        }
      }

      // Harmless meditation marbles
      if (item.compare("n") == 0) {
        if (gameMode != GameMode_Invalid) {
          pMsg->assign("Harmless meditation marbles setting must be the same in all game modes.");
          return false;
        }
        pLevel->setHarmlessMeditationMarbles(true);
        continue;
      }

      // Marbles
      if (item[0] == 'F' && item.size() >= 2) {
        MarbleType marbleType = MarbleType_Invalid;
        switch (item[1]) {
        case 'B': marbleType = MarbleType_Black;          break;
        case 'b': marbleType = MarbleType_White;          break;
        case 'M': marbleType = MarbleType_Meditation;     break;
        case 'G': marbleType = MarbleType_Horse;          break;
        case 'K': marbleType = MarbleType_Jack;           break;
        case 'C': marbleType = MarbleType_LifeSpitter;    break;
        case 'D': marbleType = MarbleType_DynamiteHolder; break;
        case 'R': marbleType = MarbleType_Rotor;          break;
        case 'H': marbleType = MarbleType_Bug;            break;
        default:
          printf ("(Oxydlib) unknown marble type: %c\n", item[1]);
          break;
        }
        if (marbleType != MarbleType_Invalid) {
          for (int curGameModeInt = GameMode_First;
               curGameModeInt <= GameMode_Last;
               curGameModeInt++) {
            GameMode curGameMode = GameMode(curGameModeInt);
            if (gameMode != GameMode_Invalid &&
                curGameMode != gameMode) {
              continue;
            }

            if (numMarblesArr[curGameMode] >= 8) {
              pMsg->assign("Too many marbles.");
              return false;
            }

            int nMarble = numMarblesArr[curGameMode];
            numMarblesArr[curGameMode]++;

            if (pLevel->getNumMarbles() < nMarble) {
              pMsg->assign("Something strange has happened parsing marbles.");
              return false;
            } else if (pLevel->getNumMarbles() == nMarble) {
              pLevel->setNumMarbles(nMarble + 1);
              Marble *pMarble = pLevel->getMarbleForWrite(nMarble);

              pMarble->setMarbleType(marbleType);
              pMarble->setX(getInt2(infoGeneral.begin() + 0x04 +
                                    (nMarble * 8)));
              pMarble->setY(getInt2(infoGeneral.begin() + 0x04 +
                                    (nMarble * 8) + 0x04));

              string *pData = pMarble->getDataForWrite(curGameMode);
              *pData = item.substr(2);
            } else { // if (pLevel->getNumMarbles() > nMarble)
              Marble *pMarble = pLevel->getMarbleForWrite(nMarble);

              if (pMarble->getMarbleType() != marbleType) {
                pMsg->assign("Marble types differ in easy and hard game mode.");
                return false;
              }

              string *pData = pMarble->getDataForWrite(curGameMode);
              *pData = item.substr(2);
            }
          }

          continue;
        }
      }

      // Rubber bands
      if (item[0] == 'B') {
        string temp1 = item.substr(1);

        string::size_type f8Pos = temp1.find(0xf8);
        string temp2;
        string temp3;
        if (f8Pos == string::npos) {
          temp2 = temp1;
        } else {
          temp2 = temp1.substr(0, f8Pos);
          temp3 = temp1.substr(f8Pos + 1);
        }

        if (!temp3.empty()) {
          pMsg->assign("Rubber band has extra data after the 0xF8 byte.");
          return false;
        }

        string::size_type f9Pos = temp2.find((char)0xf9);
        string strNumberList1;
        string strNumberList2;
        if (f9Pos == string::npos) {
          strNumberList1 = temp2;
        } else {
          strNumberList1 = temp2.substr(0, f9Pos);
          strNumberList2 = temp2.substr(f9Pos + 1);
        }

        vector<int> numberList1;
        if (!parseNumberList(strNumberList1, &numberList1)) {
          pMsg->assign("Cannot parse first number list in rubber band.");
          return false;
        }
        vector<int> numberList2;
        if (!parseNumberList(strNumberList2, &numberList2)) {
          pMsg->assign("Cannot parse second number list in rubber band.");
          return false;
        }

        if (numberList1.size() > 3) {
          pMsg->assign("First number list in rubber band is too long.");
          return false;
        }
        if (numberList2.size() > 2) {
          pMsg->assign("Second number list in rubber band is too long.");
          return false;
        }

        int naturalLength = -1;
        if (numberList1.size() > 0) {
          naturalLength = numberList1[0];
        }
        int force = -1;
        if (numberList1.size() > 1) {
          force = numberList1[1];
        }
        int blockNumber = -1;
        if (numberList1.size() > 2) {
          blockNumber = numberList1[2];
        }

        int marbleNumber1 = -1;
        if (numberList2.size() > 0) {
          marbleNumber1 = numberList2[0];
        }
        int marbleNumber2 = -1;
        if (numberList2.size() > 1) {
          marbleNumber2 = numberList2[1];
        }

        bool bInitialRubberBand = false;
        RubberBand rubberBand;

        if (blockNumber != -1 ||
            marbleNumber2 !=  -1) {
          if (blockNumber != -1 &&
              marbleNumber2 != -1) {
            pMsg->assign("A rubber band can't have a block number and two marble numbers.");
            return false;
          }

          if (marbleNumber1 == -1 &&
              marbleNumber2 != -1) {
            pMsg->assign("A rubber band can't have a second marble number without a first.");
            return false;
          }

          if (naturalLength == -1 ||
              force == -1) {
            pMsg->assign("An initial rubber band must have both natural length and force values.");
            return false;
          }

          bInitialRubberBand = true;

          rubberBand.setNaturalLength(naturalLength);
          rubberBand.setForce(force);
          rubberBand.setFirstEndMarble(marbleNumber1 == -1 ?
                                       0 : marbleNumber1);
          if (blockNumber != -1) {
            unsigned int gridSize = width * height;
            if (blockNumber >= (int)gridSize) {
              pMsg->assign("Rubber band block number is too large.");
              return false;
            }
            unsigned int x = blockNumber % width;
            unsigned int y = blockNumber / width;
            rubberBand.setSecondEndPiece(x, y);
          } else {
            rubberBand.setSecondEndMarble(marbleNumber2);
          }
        } else if (marbleNumber1 != -1) {
          pMsg->assign("A rubber band can't have a first marble number without a block number or a secend marble number.");
          return false;
        } else {
          // No checks on natural length or force for extra rubber bands
        }

        for (int curGameModeInt = GameMode_First;
             curGameModeInt <= GameMode_Last;
             curGameModeInt++) {
          GameMode curGameMode = GameMode(curGameModeInt);
          if (gameMode != GameMode_Invalid &&
              curGameMode != gameMode) {
            continue;
          }

          if (bInitialRubberBand) {
            pLevel->addRubberBand(curGameMode, rubberBand);
          } else {
            int nExtraRubberBand = numExtraRubberBandsArr[curGameMode];
            if (nExtraRubberBand >= 3) {
              pMsg->assign("Too many extra rubber bands.");
              return false;
            }
            numExtraRubberBandsArr[curGameMode]++;
            switch (nExtraRubberBand) {
            case 0:
              {
                pLevel->setBlackRubberBandPieceNaturalLength(curGameMode,
                                                             naturalLength);
                pLevel->setBlackRubberBandPieceForce(curGameMode,
                                                     force);
              }
              break;
            case 1:
              {
                pLevel->setWhiteRubberBandPieceNaturalLength(curGameMode,
                                                             naturalLength);
                pLevel->setWhiteRubberBandPieceForce(curGameMode,
                                                     force);
              }
              break;
            case 2:
              {
                pLevel->setRubberBandObjectNaturalLength(curGameMode,
                                                         naturalLength);
                pLevel->setRubberBandObjectForce(curGameMode, force);
              }
              break;
            }
          }
        }

        continue;
      }

      // Rubber band object marbles
      if (item[0] == 'N') {
        string strNumberList = item.substr(1);
        vector<int> numberList;
        if (parseNumberList(strNumberList, &numberList)) {
          if (numberList.size() <= 2) {
            if (gameMode != GameMode_Invalid) {
              pMsg->assign("Rubber band object marbles must be the same in all game modes.");
              return false;
            }

            int nMarbleBlack = 1;
            if (numberList.size() > 0 &&
                numberList[0] != -1) {
              nMarbleBlack = numberList[0];
            }

            int nMarbleWhite = 0;
            if (numberList.size() > 1 &&
                numberList[1] != -1) {
              nMarbleWhite = numberList[1];
            }

            pLevel->setBlackRubberBandObjectMarble(nMarbleBlack);
            pLevel->setWhiteRubberBandObjectMarble(nMarbleWhite);
          }
        }

        // Skip anything else that begins with 'N', since there is
        // a rubber band that mistakenly begins with 'N' instead
        // of 'B'.

        continue;
      }

      // Require magic piece
      if (item.compare("!") == 0) {
        if (gameMode != GameMode_Invalid) {
          pMsg->assign("Require magic piece setting must be the same in all game modes.");
          return false;
        }
        pLevel->setRequireMagicPiece(true);
        continue;
      }

      // Scrolling
      if (item.compare("f") == 0) {
        if (gameMode != GameMode_Invalid) {
          pMsg->assign("Scrolling setting must be the same in all game modes.");
          return false;
        }
        pLevel->setScrolling(true);
        continue;
      }

      // Reset
      if (item.compare("s") == 0) {
        for (int curGameModeInt = GameMode_First;
             curGameModeInt <= GameMode_Last;
             curGameModeInt++) {
          GameMode curGameMode = GameMode(curGameModeInt);
          if (gameMode != GameMode_Invalid &&
              curGameMode != gameMode) {
            continue;
          }

          pLevel->setReset(curGameMode, true);
        }
        continue;
      }

      // Walk-through puzzle
      if (item.compare("j") == 0) {
        if (gameMode != GameMode_Invalid) {
          pMsg->assign("Walk-through puzzle setting must be the same in all game modes.");
          return false;
        }
        pLevel->setWalkThroughPuzzle(true);
        continue;
      }

      // Force
      if (item[0] == 'g') {
        for (int curGameModeInt = GameMode_First;
             curGameModeInt <= GameMode_Last;
             curGameModeInt++) {
          GameMode curGameMode = GameMode(curGameModeInt);
          if (gameMode != GameMode_Invalid &&
              curGameMode != gameMode) {
            continue;
          }

          string strNumberList = item.substr(1);
          vector<int> numberList;
          if (parseNumberList(strNumberList, &numberList)) {
            if (numberList.size() >= 1) {
              pLevel->setFlatForce(curGameMode, numberList[0]);
            }
            if (numberList.size() >= 2) {
              pLevel->setFriction(curGameMode, numberList[1]);
            }
            if (numberList.size() >= 3) {
              pLevel->setSlopeForce(curGameMode, numberList[2]);
            }
          } else {
            pMsg->assign("Could not parse force special item.");
            return false;
          }
        }
        continue;
      }

      // Scramble items
      if (item[0] == 'P') {
        if (gameMode != GameMode_Invalid) {
          pMsg->assign("Scramble items must be the same in all game modes.");
          return false;
        }

        string strNumberList = item.substr(1);
        vector<int> numberList;
        if (!parseNumberList(strNumberList, &numberList)) {
          pMsg->assign("Could not parse scramble items.");
          return false;
        }

        if (numberList.size() % 2 == 1) {
          // This can happen
          /*
          pMsg->assign("Scramble item number list size should not be odd.");
          return false;
          */

          numberList.erase(numberList.begin() + (numberList.size() - 1));
        }

        for (int nScrambleItem = 0;
             nScrambleItem < (int)(numberList.size() / 2);
             nScrambleItem++) {
          int blockNumber = numberList[nScrambleItem * 2];
          int dirInt = numberList[nScrambleItem * 2 + 1];

          unsigned int gridSize = width * height;
          if (blockNumber >= (int)gridSize) {
            pMsg->assign("Scramble item block number is too large.");
            return false;
          }
          unsigned int x = blockNumber % width;
          unsigned int y = blockNumber / width;

          Direction dir = Direction_Invalid;
          switch (dirInt) {
          case 0: dir = Direction_Up;    break;
          case 1: dir = Direction_Down;  break;
          case 2: dir = Direction_Left;  break;
          case 3: dir = Direction_Right; break;
          }

          if (dir == Direction_Invalid) {
            pMsg->assign("Invalid scramble item direction number.");
            return false;
          }

          pLevel->addScrambleItem(ScrambleItem(x, y, dir));
        }

        continue;
      }

      // Lasers
      if (item[0] == 'L') {
        if (gameMode != GameMode_Invalid) {
          pMsg->assign("Lasers must be the same in all game modes.");
          return false;
        }

        string strLaserData = item.substr(1);

        string strNumberList;
        bool bOn = false;
        if (strLaserData.size() > 0) {
          char lastByte = strLaserData[strLaserData.size() - 1];
          if (lastByte == (char)0xf9 ||
              lastByte == (char)0xf8) {
            strNumberList = strLaserData.substr(0, strLaserData.size() - 1);
            if (lastByte == (char)0xf9) {
              bOn = true;
            }
          } else {
            strNumberList = strLaserData;
          }
        }

        vector<int> numberList;
        if (!parseNumberList(strNumberList, &numberList)) {
          pMsg->assign("Could not parse laser.");
          return false;
        }

        if (numberList.size() != 2) {
          pMsg->assign("Laser number list should have two numbers.");
          return false;
        }

        int laserNumber = numberList[0];
        int dirInt = numberList[1];

        if (laserNumber < 1 || laserNumber > 3) {
          pMsg->assign("Laser type must be between 1 and 3.");
          return false;
        }

        Direction dir = Direction_Invalid;
        switch (dirInt) {
        case 0: dir = Direction_Up;    break;
        case 1: dir = Direction_Down;  break;
        case 2: dir = Direction_Left;  break;
        case 3: dir = Direction_Right; break;
        }

        if (dir == Direction_Invalid) {
          pMsg->assign("Invalid laser direction number.");
          return false;
        }

        if (foundLaserArr[laserNumber - 1]) {
          pMsg->assign("Laser type occurs more than once.");
          return false;
        }
        foundLaserArr[laserNumber - 1] = true;

        pLevel->setLaser(laserNumber - 1, Laser(dir, bOn));

        continue;
      }

      // Oscillators
      if (item[0] == '~') {
        string strNumberList = item.substr(1);
        vector<int> numberList;
        if (!parseNumberList(strNumberList, &numberList)) {
          pMsg->assign("Could not parse oscillator.");
          return false;
        }

        if (numberList.size() != 2) {
          pMsg->assign("Oscillator number list should have two numbers.");
          return false;
        }

        int blockNumber = numberList[0];
        int period = numberList[1];

        if (blockNumber == -1) {
          pMsg->assign("Oscillator block number is empty.");
          return false;
        }

        if (period == -1) {
          pMsg->assign("Oscillator period is empty.");
          return false;
        }

        unsigned int gridSize = width * height;
        if (blockNumber >= (int)gridSize) {
          pMsg->assign("Oscillator block number is too large.");
          return false;
        }

        unsigned int x = blockNumber % width;
        unsigned int y = blockNumber / width;

        for (int curGameModeInt = GameMode_First;
             curGameModeInt <= GameMode_Last;
             curGameModeInt++) {
          GameMode curGameMode = GameMode(curGameModeInt);
          if (gameMode != GameMode_Invalid &&
              curGameMode != gameMode) {
            continue;
          }

          OscillatorMap *pOscillators =
            pLevel->getOscillatorsForWrite(curGameMode);
          (*pOscillators)[Block(x, y)].setPeriod(period);
        }

        continue;
      }

      // Still oscillators
      if (item[0] == 'O') {
        string strNumberList = item.substr(1);
        vector<int> numberList;
        if (!parseNumberList(strNumberList, &numberList)) {
          pMsg->assign("Could not parse still oscillator.");
          return false;
        }

        if (numberList.size() != 1) {
          pMsg->assign("Still oscillator number list should have one number.");
          return false;
        }

        int blockNumber = numberList[0];

        if (blockNumber == -1) {
          pMsg->assign("Still oscillator block number is empty.");
          return false;
        }

        unsigned int gridSize = width * height;
        if (blockNumber >= (int)gridSize) {
          pMsg->assign("Still oscillator block number is too large.");
          return false;
        }

        unsigned int x = blockNumber % width;
        unsigned int y = blockNumber / width;

        for (int curGameModeInt = GameMode_First;
             curGameModeInt <= GameMode_Last;
             curGameModeInt++) {
          GameMode curGameMode = GameMode(curGameModeInt);
          if (gameMode != GameMode_Invalid &&
              curGameMode != gameMode) {
            continue;
          }

          OscillatorMap *pOscillators =
            pLevel->getOscillatorsForWrite(curGameMode);
          (*pOscillators)[Block(x, y)].setOn(false);
        }

        continue;
      }

      // Notes
      if (item[0] == 'Z' || item[0] == 'z') {
        if (gameMode != GameMode_Invalid) {
          pMsg->assign("Note text must be the same in all game modes.");
          return false;
        }
        int nNote = (item[0] == 'Z' ? 0 : 1);

        if (item.size() >= 4) {
          Language lang = Language_Invalid;
          switch (item[1]) {
          case 'g': lang = Language_German;  break;
          case 'e': lang = Language_English; break;
          case 'f': lang = Language_French;  break;
          case 'i': lang = Language_International; break;
          }
          if (lang != Language_Invalid) {
            if (item[2] == '"') {
              int i = 3;
              while (i < (int)item.size() &&
                     item[i] != '"') {
                i++;
              }
              // Allow extra characters after the second quote
              if (i < (int)item.size()) {
                string noteText = item.substr(3, i-3);
                pLevel->setNoteText(nNote, lang, noteText);
                continue;
              }
            }
          }
        }
      }

      // Clean up some bad data in notes
      if (item[0] == '"') {
        continue;
      }

      /*
      if (item.compare(">(0)") == 0) {
        pLevel->getSpecialItemsForWrite()->push_back(">(1)");
        continue;
      }
      */

      /*
      if (item.compare("#") == 0) {
        continue;
      }
      */

      /*
      if (item.compare("r") == 0) {
        continue;
      }
      */

      if (true || gameMode != GameMode_Easy) {
        pLevel->getSpecialItemsForWrite()->push_back(item);
      }
    }

    if (gameMode != GameMode_Invalid) {
      pMsg->assign("Special items ended while in a game mode.");
      return false;
    }

    if (numMarblesArr[GameMode_Hard] != numMarblesArr[GameMode_Easy]) {
      pMsg->assign("Number of marbles is different in different game modes.");
      return false;
    }
  }

  if (!parseGrid(infoArr[INFOSURFACES],
                 pLevel->getGridForWrite(GridType_Surfaces), pMsg)) {
    return false;
  }
  if (!parseGrid(infoArr[INFOPIECES],
                 pLevel->getGridForWrite(GridType_Pieces), pMsg)) {
    return false;
  }
  if (!parseGrid(infoArr[INFOOBJECTS],
                 pLevel->getGridForWrite(GridType_Objects), pMsg)) {
    return false;
  }

  /*
  {
    printf("%d, %d\n", getWidth(), getHeight());
    const ByteVec &infoSignal = infoArr[INFOSIGNAL];
    int q = 0;
    int j = 9999;
    for (int i = 0; i < infoSignal.size(); i++) {
      printf("%02x ", infoSignal[i]);
      if (q == 3) {
        j = infoSignal[i] * 2;
      }
      if (j == 0) {
        printf("\n");
        q = 0;
        j = 9999;
      } else {
        q++;
        j--;
      }
    }
    printf("\n");
  }
  */

  {
    const ByteVec &infoSignal = infoArr[INFOSIGNAL];

    int cur = 0;
    while (true) {
      if (cur + 2 > (int)infoSignal.size()) {
        pMsg->assign("Unexpected end of data reading signal sender.");
        return false;
      }

      unsigned int packedSignalSender = getInt2(infoSignal.begin() + cur);
      cur += 2;

      if (packedSignalSender == 0xffff) {
        break;
      }

      SignalLocation signalSender;
      bool senderWithinGrid = true;
      if (!unpackSignalLocation(*pLevel,
                                packedSignalSender,
                                &signalSender,
                                &senderWithinGrid,
                                pMsg)) {
        return false;
      }

      if (senderWithinGrid) {
        if (pLevel->getNumRecipients(signalSender) > 0) {
          pMsg->assign("Duplicate signal sender.");
          return false;
        }
      }

      if (cur + 2 > (int)infoSignal.size()) {
        pMsg->assign("Unexpected end of data reading signal recipient count.");
        return false;
      }

      unsigned int signalRecipientCount = getInt2(infoSignal.begin() + cur);
      cur += 2;

      if (signalRecipientCount <= 0) {
        pMsg->assign("Signal sender has no recipients.");
        return false;
      }

      if (cur + (2 * signalRecipientCount) > infoSignal.size()) {
        pMsg->assign("Unexpected end of data reading signal recipients.");
        return false;
      }

      for (unsigned int nRecipient = 0;
           nRecipient < signalRecipientCount;
           nRecipient++) {
        unsigned int packedSignalRecipient = getInt2(infoSignal.begin() + cur);
        cur += 2;

        SignalLocation signalRecipient;
        bool recipientWithinGrid = true;
        if (!unpackSignalLocation(*pLevel,
                                  packedSignalRecipient,
                                  &signalRecipient,
                                  &recipientWithinGrid,
                                  pMsg)) {
          return false;
        }

        if (senderWithinGrid && recipientWithinGrid) {
          pLevel->addRecipient(signalSender, signalRecipient);
        }
      }
    }

    if (cur < (int)infoSignal.size()) {
      pMsg->assign("Extra data after signals.");
      return false;
    }
  }

  pLevel->setInit(true);
  return true;
}

bool unparseLevel(const Level &level, ByteVec *pOut, string *pMsg)
{
  string msg;
  if (!pMsg) {
    pMsg = &msg;
  }

  if (!level.getInit()) {
    pMsg->assign("Level not initialized.");
    return false;
  }

  if (level.isEmpty()) {
    pOut->clear();
    return true;
  }

  ByteVec out;

  int cur = 0;

  // General
  {
    out.resize(out.size() + 2 + 68);

    putInt2(out.begin() + cur, INFOGENERAL);
    cur += 2;

    putInt2(out.begin() + cur, level.getWidth());
    cur += 2;

    putInt2(out.begin() + cur, level.getHeight());
    cur += 2;

    if (level.getNumMarbles() > 8) {
      pMsg->assign("Too many marbles.");
      return false;
    }

    for (int nMarble = 0; nMarble < 8; nMarble++) {
      int x = 0;
      int y = 0;
      if (nMarble < level.getNumMarbles()) {
        const Marble &marble = level.getMarble(nMarble);
        x = marble.getX();
        y = marble.getY();
      }

      putInt2(out.begin() + cur, x);
      cur += 2;

      out[cur++] = 0x00;
      out[cur++] = 0x00;

      putInt2(out.begin() + cur, y);
      cur += 2;

      out[cur++] = 0x00;
      out[cur++] = 0x00;
    }
  }

  // Signal
  {
    set<SignalLocation> senders;
    level.getSenders(&senders);

    int numSignals = 0;
    {
      set<SignalLocation>::const_iterator iter = senders.begin();
      set<SignalLocation>::const_iterator end = senders.end();
      for (; iter != end; ++iter) {
        const SignalLocation &sender = *iter;
        numSignals += level.getNumRecipients(sender);
      }
    }

    int infoSignalSize = 4 * senders.size() + 2 * numSignals + 2;

    out.resize(out.size() + 2 + 2 + infoSignalSize);

    putInt2(out.begin() + cur, INFOSIGNAL);
    cur += 2;

    putInt2(out.begin() + cur, infoSignalSize);
    cur += 2;

    set<SignalLocation>::const_iterator senderIter = senders.begin();
    set<SignalLocation>::const_iterator senderEnd = senders.end();
    for (; senderIter != senderEnd; ++senderIter) {
      const SignalLocation &sender = *senderIter;

      unsigned int packedSignalSender = 0;
      if (!packSignalLocation(level,
                              sender,
                              &packedSignalSender,
                              pMsg)) {
        return false;
      }

      putInt2(out.begin() + cur, packedSignalSender);
      cur += 2;

      int numRecipients = level.getNumRecipients(sender);

      putInt2(out.begin() + cur, numRecipients);
      cur += 2;

      for (int nRecipient = 0; nRecipient < numRecipients; nRecipient++) {
        const SignalLocation &recipient =
          level.getRecipient(sender, nRecipient);

        unsigned int packedSignalRecipient = 0;
        if (!packSignalLocation(level,
                                recipient,
                                &packedSignalRecipient,
                                pMsg)) {
          return false;
        }

        putInt2(out.begin() + cur, packedSignalRecipient);
        cur += 2;
      }
    }

    putInt2(out.begin() + cur, 0xffff);
    cur += 2;
  }

  // Special
  vector<string> specialItems;

  bool needGameModeArr[GameMode_Count];
  {
    for (int curGameModeInt = GameMode_First;
         curGameModeInt <= GameMode_Last;
         curGameModeInt++) {
      GameMode curGameMode = GameMode(curGameModeInt);
      needGameModeArr[curGameMode] = false;
    }
  }

  bool bMarblesDifferent = false;
  {
    for (int nMarble = 0; nMarble < level.getNumMarbles(); nMarble++) {
      const Marble &marble = level.getMarble(nMarble);
      const string &dataHard = marble.getData(GameMode_Hard);
      const string &dataEasy = marble.getData(GameMode_Easy);
      if (dataHard.compare(dataEasy) != 0) {
        bMarblesDifferent = true;
        needGameModeArr[GameMode_Hard] = true;
        needGameModeArr[GameMode_Easy] = true;
        break;
      }
    }
  }

  bool bRubberBandsDifferent = false;
  {
    if (level.getNumRubberBands(GameMode_Hard) !=
        level.getNumRubberBands(GameMode_Easy)) {
      bRubberBandsDifferent = true;
    }
    for (int nRubberBand = 0;
         (nRubberBand < level.getNumRubberBands(GameMode_Hard) &&
          nRubberBand < level.getNumRubberBands(GameMode_Easy));
         nRubberBand++) {
      const RubberBand &rubberBandHard = level.getRubberBand(GameMode_Hard,
                                                             nRubberBand);
      const RubberBand &rubberBandEasy = level.getRubberBand(GameMode_Easy,
                                                             nRubberBand);
      if (rubberBandHard != rubberBandEasy) {
        bRubberBandsDifferent = true;
      }
    }
    if ((level.getBlackRubberBandPieceNaturalLength(GameMode_Hard) !=
         level.getBlackRubberBandPieceNaturalLength(GameMode_Easy)) ||
        (level.getBlackRubberBandPieceForce(GameMode_Hard) !=
         level.getBlackRubberBandPieceForce(GameMode_Easy)) ||
        (level.getWhiteRubberBandPieceNaturalLength(GameMode_Hard) !=
         level.getWhiteRubberBandPieceNaturalLength(GameMode_Easy)) ||
        (level.getWhiteRubberBandPieceForce(GameMode_Hard) !=
         level.getWhiteRubberBandPieceForce(GameMode_Easy)) ||
        (level.getRubberBandObjectNaturalLength(GameMode_Hard) !=
         level.getRubberBandObjectNaturalLength(GameMode_Easy)) ||
        (level.getRubberBandObjectForce(GameMode_Hard) !=
         level.getRubberBandObjectForce(GameMode_Easy))) {
      bRubberBandsDifferent = true;
    }

    if (bRubberBandsDifferent) {
      for (int curGameModeInt = GameMode_First;
           curGameModeInt <= GameMode_Last;
           curGameModeInt++) {
        GameMode curGameMode = GameMode(curGameModeInt);
        if (level.getNumRubberBands(curGameMode) > 0 ||
            level.getBlackRubberBandPieceNaturalLength(curGameMode) != -1 ||
            level.getBlackRubberBandPieceForce(curGameMode) != -1 ||
            level.getWhiteRubberBandPieceNaturalLength(curGameMode) != -1 ||
            level.getWhiteRubberBandPieceForce(curGameMode) != -1 ||
            level.getRubberBandObjectNaturalLength(curGameMode) != -1 ||
            level.getRubberBandObjectForce(curGameMode) != -1) {
          needGameModeArr[curGameMode] = true;
        }
      }
    }
  }

  bool bResetDifferent = false;
  {
    if (level.getReset(GameMode_Hard) != level.getReset(GameMode_Easy)) {
      bResetDifferent = true;

      for (int curGameModeInt = GameMode_First;
           curGameModeInt <= GameMode_Last;
           curGameModeInt++) {
        GameMode curGameMode = GameMode(curGameModeInt);
        if (level.getReset(curGameMode)) {
          needGameModeArr[curGameMode] = true;
        }
      }
    }
  }

  bool bForceDifferent = false;
  {
    if ((level.getFlatForce(GameMode_Hard) !=
         level.getFlatForce(GameMode_Easy)) ||
        (level.getSlopeForce(GameMode_Hard) !=
         level.getSlopeForce(GameMode_Easy)) ||
        (level.getFriction(GameMode_Hard) !=
         level.getFriction(GameMode_Easy))) {
      bForceDifferent = true;

      for (int curGameModeInt = GameMode_First;
           curGameModeInt <= GameMode_Last;
           curGameModeInt++) {
        GameMode curGameMode = GameMode(curGameModeInt);
        if (level.getFlatForce(curGameMode) != -1 ||
            level.getSlopeForce(curGameMode) != -1 ||
            level.getFriction(curGameMode) != -1) {
          needGameModeArr[curGameMode] = true;
        }
      }
    }
  }

  bool bOscillatorsDifferent = false;
  {
    const OscillatorMap &hardOscillators =
      level.getOscillators(GameMode_Hard);
    const OscillatorMap &easyOscillators =
      level.getOscillators(GameMode_Easy);

    if (hardOscillators.size() != easyOscillators.size()) {
      bOscillatorsDifferent = true;
    }
    OscillatorMap::const_iterator hardIter = hardOscillators.begin();
    OscillatorMap::const_iterator hardEnd = hardOscillators.end();
    OscillatorMap::const_iterator easyIter = easyOscillators.begin();
    OscillatorMap::const_iterator easyEnd = easyOscillators.end();
    for(;
        hardIter != hardEnd && easyIter != easyEnd;
        ++hardIter, ++easyIter) {
      const Block &hardBlock = hardIter->first;
      const Block &easyBlock = easyIter->first;
      const Oscillator &hardOscillator = hardIter->second;
      const Oscillator &easyOscillator = easyIter->second;
      if (hardBlock != easyBlock ||
          hardOscillator != easyOscillator) {
        bOscillatorsDifferent = true;
      }
    }

    if (bOscillatorsDifferent) {
      for (int curGameModeInt = GameMode_First;
           curGameModeInt <= GameMode_Last;
           curGameModeInt++) {
        GameMode curGameMode = GameMode(curGameModeInt);
        const OscillatorMap &oscillators = level.getOscillators(curGameMode);
        OscillatorMap::const_iterator iter = oscillators.begin();
        OscillatorMap::const_iterator end = oscillators.end();
        for (; iter != end; ++iter) {
          const Oscillator &oscillator = iter->second;
          if (oscillator != Oscillator()) {
            needGameModeArr[curGameMode] = true;
          }
        }
      }
    }
  }

  {
    // Goal
    if (level.getMeditation()) {
      specialItems.push_back("GM");
    } else {
      specialItems.push_back("GN");
    }
  }

  {
    // Harmless meditation marbles
    if (level.getHarmlessMeditationMarbles()) {
      specialItems.push_back("n");
    }
  }

  {
    GameMode gameMode = GameMode_Invalid;
    bool bFirstPass = true;
    bool bDone = false;
    while (!bDone) {
      if (gameMode == GameMode_Invalid ||
          needGameModeArr[gameMode]) {

        GameMode realGameMode =
          (gameMode == GameMode_Invalid) ? GameMode_Hard : gameMode;

        if (gameMode == GameMode_Hard) {
          specialItems.push_back("[");
        }
        if (gameMode == GameMode_Easy) {
          specialItems.push_back("{");
        }

        // Marbles
        if ((bMarblesDifferent &&
             gameMode != GameMode_Invalid) ||
            (!bMarblesDifferent &&
             gameMode == GameMode_Invalid &&
             bFirstPass)) {

          for (int nMarble = 0; nMarble < level.getNumMarbles(); nMarble++) {
            const Marble &marble = level.getMarble(nMarble);

            unsigned char marbleCode = 0x00;
            switch (marble.getMarbleType()) {
            case MarbleType_Black:          marbleCode = 'B'; break;
            case MarbleType_White:          marbleCode = 'b'; break;
            case MarbleType_Meditation:     marbleCode = 'M'; break;
            case MarbleType_Horse:          marbleCode = 'G'; break;
            case MarbleType_Jack:           marbleCode = 'K'; break;
            case MarbleType_LifeSpitter:    marbleCode = 'C'; break;
            case MarbleType_DynamiteHolder: marbleCode = 'D'; break;
            default:
              pMsg->assign("Invalid marble type.");
              return false;
            }

            string item;
            item.append(1, 'F');
            item.append(1, marbleCode);
            item.append(marble.getData(realGameMode));

            specialItems.push_back(item);
          }
        }

        // Rubber bands
        if ((bRubberBandsDifferent &&
             gameMode != GameMode_Invalid) ||
            (!bRubberBandsDifferent &&
             gameMode == GameMode_Invalid &&
             bFirstPass)) {

          for (int nRubberBand = 0;
               nRubberBand < level.getNumRubberBands(realGameMode);
               nRubberBand++) {
            const RubberBand &rubberBand =
              level.getRubberBand(realGameMode, nRubberBand);
            vector<int> numberList1;
            vector<int> numberList2;
            numberList1.push_back(rubberBand.getNaturalLength());
            numberList1.push_back(rubberBand.getForce());
            numberList2.push_back(rubberBand.getFirstEndMarble());
            if (rubberBand.isSecondEndMarble()) {
              numberList1.push_back(-1);
              numberList2.push_back(rubberBand.getSecondEndMarble());
            } else {
              unsigned int x = rubberBand.getSecondEndPieceX();
              unsigned int y = rubberBand.getSecondEndPieceY();
              if (x > level.getWidth() ||
                  y > level.getHeight()) {
                pMsg->assign("Rubber band block out of range.");
                return false;
              }
              unsigned int blockNumber =
                y * level.getWidth() + x;
              numberList1.push_back(blockNumber);
            }

            string strNumberList1;
            unparseNumberList(numberList1, &strNumberList1);
            string strNumberList2;
            unparseNumberList(numberList2, &strNumberList2);

            string str;
            str.append(1, 'B');
            str.append(strNumberList1);
            str.append(1, (char)0xf9);
            str.append(strNumberList2);
            specialItems.push_back(str);
          }

          {
            vector<int> numberList1;
            numberList1.push_back(level.getBlackRubberBandPieceNaturalLength(realGameMode));
            numberList1.push_back(level.getBlackRubberBandPieceForce(realGameMode));
            if (numberList1[0] == -1 && numberList1[1] == -1) {
              numberList1.clear();
            }

            vector<int> numberList2;
            numberList2.push_back(level.getWhiteRubberBandPieceNaturalLength(realGameMode));
            numberList2.push_back(level.getWhiteRubberBandPieceForce(realGameMode));
            if (numberList2[0] == -1 && numberList2[1] == -1) {
              numberList2.clear();
            }

            vector<int> numberList3;
            numberList3.push_back(level.getRubberBandObjectNaturalLength(realGameMode));
            numberList3.push_back(level.getRubberBandObjectForce(realGameMode));
            if (numberList3[0] == -1 && numberList3[1] == -1) {
              numberList3.clear();
            }

            if (!numberList1.empty() ||
                !numberList2.empty() ||
                !numberList3.empty()) {
              string strNumberList1;
              unparseNumberList(numberList1, &strNumberList1);

              string str;
              str.append(1, 'B');
              str.append(strNumberList1);
              specialItems.push_back(str);
            }

            if (!numberList2.empty() ||
                !numberList3.empty()) {
              string strNumberList2;
              unparseNumberList(numberList2, &strNumberList2);

              string str;
              str.append(1, 'B');
              str.append(strNumberList2);
              specialItems.push_back(str);
            }

            if (!numberList3.empty()) {
              string strNumberList3;
              unparseNumberList(numberList3, &strNumberList3);

              string str;
              str.append(1, 'B');
              str.append(strNumberList3);
              specialItems.push_back(str);
            }
          }
        }

        // Reset
        if ((bResetDifferent &&
             gameMode != GameMode_Invalid) ||
            (!bResetDifferent &&
             gameMode == GameMode_Invalid &&
             !bFirstPass)) {

          if (level.getReset(realGameMode)) {
            specialItems.push_back("s");
          }
        }

        // Force
        if ((bForceDifferent &&
             gameMode != GameMode_Invalid) ||
            (!bForceDifferent &&
             gameMode == GameMode_Invalid &&
             !bFirstPass)) {

          vector<int> numberList;
          numberList.resize(3);
          numberList[0] = level.getFlatForce(realGameMode);
          numberList[1] = level.getFriction(realGameMode);
          numberList[2] = level.getSlopeForce(realGameMode);

          int sz = 3;
          while (sz > 0 && numberList[sz - 1] == -1) {
            sz--;
          }
          numberList.resize(sz);

          if (sz > 0) {
            string strNumberList;
            unparseNumberList(numberList, &strNumberList);

            string str;
            str.append(1, 'g');
            str.append(strNumberList);
            specialItems.push_back(str);
          }
        }

        // Oscillators
        if ((bOscillatorsDifferent &&
             gameMode != GameMode_Invalid) ||
            (!bOscillatorsDifferent &&
             gameMode == GameMode_Invalid &&
             !bFirstPass)) {
          const OscillatorMap &oscillators =
            level.getOscillators(realGameMode);
          OscillatorMap::const_iterator iter = oscillators.begin();
          OscillatorMap::const_iterator end = oscillators.end();
          for (; iter != end; ++iter) {
            const Block &block = iter->first;
            const Oscillator &oscillator = iter->second;

            unsigned int x = block.getX();
            unsigned int y = block.getY();
            if (x > level.getWidth() || y > level.getHeight()) {
              pMsg->assign("Oscillator block out of range.");
              return false;
            }
            unsigned int blockNumber =
              y * level.getWidth() + x;

            if (oscillator.getPeriod() != 0) {
              vector<int> numberList;
              numberList.push_back(blockNumber);
              numberList.push_back(oscillator.getPeriod());

              string strNumberList;
              unparseNumberList(numberList, &strNumberList);

              string str;
              str.append(1, '~');
              str.append(strNumberList);
              specialItems.push_back(str);
            }
          }
        }

        // Still oscillators
        if ((bOscillatorsDifferent &&
             gameMode != GameMode_Invalid) ||
            (!bOscillatorsDifferent &&
             gameMode == GameMode_Invalid &&
             !bFirstPass)) {
          const OscillatorMap &oscillators =
            level.getOscillators(realGameMode);
          OscillatorMap::const_iterator iter = oscillators.begin();
          OscillatorMap::const_iterator end = oscillators.end();
          for (; iter != end; ++iter) {
            const Block &block = iter->first;
            const Oscillator &oscillator = iter->second;

            unsigned int x = block.getX();
            unsigned int y = block.getY();
            if (x > level.getWidth() || y > level.getHeight()) {
              pMsg->assign("Oscillator block out of range.");
              return false;
            }
            unsigned int blockNumber =
              y * level.getWidth() + x;

            if (!oscillator.getOn()) {
              vector<int> numberList;
              numberList.push_back(blockNumber);

              string strNumberList;
              unparseNumberList(numberList, &strNumberList);

              string str;
              str.append(1, 'O');
              str.append(strNumberList);
              specialItems.push_back(str);
            }
          }
        }

        if (gameMode == GameMode_Hard) {
          specialItems.push_back("]");
        }
        if (gameMode == GameMode_Easy) {
          specialItems.push_back("}");
        }

      }

      switch (gameMode) {
      case GameMode_Invalid:
        if (bFirstPass) {
          bFirstPass = false;
          gameMode = GameMode_Hard;
        } else {
          bDone = true;
        }
        break;
      case GameMode_Hard:
        gameMode = GameMode_Easy;
        break;
      case GameMode_Easy:
        gameMode = GameMode_Invalid;
        break;
      default:
        ;
      }
    }
  }

  {
    // Rubber band object marbles
    int nMarbleBlack = level.getBlackRubberBandObjectMarble();
    if (nMarbleBlack == -1) {
      nMarbleBlack = 1;
    }

    int nMarbleWhite = level.getWhiteRubberBandObjectMarble();
    if (nMarbleWhite == -1) {
      nMarbleWhite = 0;
    }

    if (nMarbleBlack != 1 || nMarbleWhite != 0) {
      vector<int> numberList;
      numberList.resize(2);
      numberList[0] = nMarbleBlack;
      numberList[1] = nMarbleWhite;

      string strNumberList;
      unparseNumberList(numberList, &strNumberList);

      string str;
      str.append(1, 'N');
      str.append(strNumberList);
      specialItems.push_back(str);
    }
  }

  {
    // Require magic piece
    if (level.getRequireMagicPiece()) {
      specialItems.push_back("!");
    }
  }

  {
    // Scrolling
    if (level.getScrolling()) {
      specialItems.push_back("f");
    }
  }

  {
    // Walk-through puzzle
    if (level.getWalkThroughPuzzle()) {
      specialItems.push_back("j");
    }
  }

  {
    // Scramble items
    if (level.getNumScrambleItems() > 0) {
      vector<int> numberList;
      for (int nScrambleItem = 0;
           nScrambleItem < level.getNumScrambleItems();
           nScrambleItem++) {
        const ScrambleItem &scrambleItem =
          level.getScrambleItem(nScrambleItem);

        unsigned int x = scrambleItem.getX();
        unsigned int y = scrambleItem.getY();
        if (x > level.getWidth() ||
            y > level.getHeight()) {
          pMsg->assign("Scramble item block out of range.");
          return false;
        }
        unsigned int blockNumber =
          y * level.getWidth() + x;

        Direction dir = scrambleItem.getDir();
        int dirInt = -1;
        switch (dir) {
        case Direction_Up:    dirInt = 0; break;
        case Direction_Down:  dirInt = 1; break;
        case Direction_Left:  dirInt = 2; break;
        case Direction_Right: dirInt = 3; break;
        default:
          pMsg->assign("Invalid scramble item direction.");
          return false;
        }

        numberList.push_back(blockNumber);
        numberList.push_back(dirInt);
      }

      string strNumberList;
      unparseNumberList(numberList, &strNumberList);

      string str;
      str.append(1, 'P');
      str.append(strNumberList);
      specialItems.push_back(str);
    }
  }

  {
    // Lasers
    for (int nLaser = 0; nLaser < 3; nLaser++) {
      const Laser &laser = level.getLaser(nLaser);
      if (laser != s_defaultLasers[nLaser]) {
        vector<int> numberList;

        Direction dir = laser.getDir();
        int dirInt = -1;
        switch (dir) {
        case Direction_Up:    dirInt = 0; break;
        case Direction_Down:  dirInt = 1; break;
        case Direction_Left:  dirInt = 2; break;
        case Direction_Right: dirInt = 3; break;
        default:
          pMsg->assign("Invalid laser direction.");
          return false;
        }

        numberList.push_back(nLaser + 1);
        numberList.push_back(dirInt);

        string strNumberList;
        unparseNumberList(numberList, &strNumberList);

        string str;
        str.append(1, 'L');
        str.append(strNumberList);
        if (laser.getOn()) {
          str.append(1, (char)0xf9);
        }

        specialItems.push_back(str);
      }
    }
  }

  {
    // Stuff we don't know about yet
    for (int i = 0; i < (int)level.getSpecialItems().size(); i++) {
      specialItems.push_back(level.getSpecialItems()[i]);
    }
  }

  {
    // Notes
    for (int nNote = 0; nNote < 2; nNote++) {
      for (int nLang = Language_First;
           nLang <= Language_Last;
           nLang++) {
        Language lang = (Language)nLang;
        const string &noteText = level.getNoteText(nNote, lang);
        if (!noteText.empty()) {
          unsigned char noteCode = 0x00;
          switch (nNote) {
          case 0: noteCode = 'Z'; break;
          case 1: noteCode = 'z'; break;
          }

          unsigned char langCode = 0x00;
          switch(lang) {
          case Language_German:  langCode = 'g'; break;
          case Language_English: langCode = 'e'; break;
          case Language_French:  langCode = 'f'; break;
          default:
            pMsg->assign("Invalid language.");
            return false;
          }

          string noteItem;

          noteItem.append(1, noteCode);
          noteItem.append(1, langCode);
          noteItem.append(1, '"');
          noteItem.append(noteText);
          noteItem.append(1, '"');

          specialItems.push_back(noteItem);
        }
      }
    }
  }

  /*
  printf("---\n");
  {
    for (int nItem = 0; nItem < specialItems.size(); nItem++) {
      printf("  %s\n", specialItems[nItem].c_str());
    }
  }
  */

  int infoSpecialSize = 0;
  int nItem;
  for (nItem = 0; nItem < (int)specialItems.size(); nItem++) {
    if (nItem != 0) {
      infoSpecialSize += 1;
    }
    infoSpecialSize += specialItems[nItem].size();
  }
  infoSpecialSize += 1;
  out.resize(out.size() + 2 + 2 + infoSpecialSize);

  putInt2(out.begin() + cur, INFOSPECIAL);
  cur += 2;

  putInt2(out.begin() + cur, infoSpecialSize);
  cur += 2;

  for (nItem = 0; nItem < (int)specialItems.size(); nItem++) {
    if (nItem != 0) {
      out[cur++] = 0x20;
    }
    copy(specialItems[nItem].begin(),
         specialItems[nItem].end(),
         out.begin() + cur);
    cur += specialItems[nItem].size();
  }
  out[cur++] = 0x00;

  // Surfaces
  ByteVec unparsedSurfaces;
  if (!unparseGrid(level.getGrid(GridType_Surfaces),
                   &unparsedSurfaces, pMsg)) {
    return false;
  }

  out.resize(out.size() + 2 + 2 + unparsedSurfaces.size());

  putInt2(out.begin() + cur, INFOSURFACES);
  cur += 2;

  putInt2(out.begin() + cur, unparsedSurfaces.size());
  cur += 2;

  copy(unparsedSurfaces.begin(), unparsedSurfaces.end(), out.begin() + cur);
  cur += unparsedSurfaces.size();

  // Pieces
  ByteVec unparsedPieces;
  if (!unparseGrid(level.getGrid(GridType_Pieces),
                   &unparsedPieces, pMsg)) {
    return false;
  }

  out.resize(out.size() + 2 + 2 + unparsedPieces.size());

  putInt2(out.begin() + cur, INFOPIECES);
  cur += 2;

  putInt2(out.begin() + cur, unparsedPieces.size());
  cur += 2;

  copy(unparsedPieces.begin(), unparsedPieces.end(), out.begin() + cur);
  cur += unparsedPieces.size();

  // Objects
  ByteVec unparsedObjects;
  if (!unparseGrid(level.getGrid(GridType_Objects),
                   &unparsedObjects, pMsg)) {
    return false;
  }

  out.resize(out.size() + 2 + 2 + unparsedObjects.size());

  putInt2(out.begin() + cur, INFOOBJECTS);
  cur += 2;

  putInt2(out.begin() + cur, unparsedObjects.size());
  cur += 2;

  copy(unparsedObjects.begin(), unparsedObjects.end(), out.begin() + cur);
  cur += unparsedObjects.size();

  // End
  out.resize(out.size() + 2);

  putInt2(out.begin() + cur, INFOEND);
  cur += 2;

  if (cur != (int)out.size()) {
    pMsg->assign("Error computing size of unparsed level.");
    return false;
  }

  pOut->clear();
  pOut->resize(out.size());
  copy(out.begin(), out.end(), pOut->begin());

  return true;
}

}
