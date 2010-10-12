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

#ifndef LEVEL_H
#define LEVEL_H

#include "VecUtils.h"
#include <string>
#include <set>
#include <map>

namespace OxydLib {

// using std::string;
// using std::set;
// using std::map;

enum Language
{
  Language_Invalid = -1,
  Language_First = 0,

  Language_German = Language_First,
  Language_English,
  Language_French,
  Language_International,

  Language_Count,
  Language_Last = Language_Count - 1
};

enum GameMode
{
  GameMode_Invalid = -1,
  GameMode_First = 0,

  GameMode_Hard = GameMode_First,
  GameMode_Easy,

  GameMode_Count,
  GameMode_Last = GameMode_Count - 1
};

enum GridType
{
  GridType_Invalid = -1,
  GridType_First = 0,

  GridType_Surfaces = GridType_First,
  GridType_Pieces,
  GridType_Objects,

  GridType_Count,
  GridType_Last = GridType_Count - 1
};

enum MarbleType
{
  MarbleType_Invalid = -1,
  MarbleType_First = 0,

  MarbleType_Black = MarbleType_First,
  MarbleType_White,
  MarbleType_Meditation,
  MarbleType_Horse,
  MarbleType_Jack,
  MarbleType_LifeSpitter,
  MarbleType_DynamiteHolder,
  MarbleType_Rotor,             /* Oxyd 1 */
  MarbleType_Bug,               /* Oxyd 1 */

  MarbleType_Count,
  MarbleType_Last = MarbleType_Count - 1
};

class Marble
{
public:
  Marble();
  ~Marble();

  MarbleType getMarbleType() const { return m_marbleType; }
  void setMarbleType(MarbleType marbleType) { m_marbleType = marbleType; }

  unsigned int getX() const { return m_x; }
  void setX(unsigned int x) { m_x = x; }

  unsigned int getY() const { return m_y; }
  void setY(unsigned int y) { m_y = y; }

  const std::string &getData(GameMode gameMode) const { return m_data[gameMode]; }
  std::string *getDataForWrite(GameMode gameMode) { return &m_data[gameMode]; }

private:
  MarbleType m_marbleType;
  unsigned int m_x;
  unsigned int m_y;
  std::string m_data[GameMode_Count];
};

class RubberBand
{
public:
  RubberBand();
  ~RubberBand();

  int getNaturalLength() const { return m_naturalLength; }
  void setNaturalLength(int naturalLength) { m_naturalLength = naturalLength; }

  int getForce() const { return m_force; }
  void setForce(int force) { m_force = force; }

  int getFirstEndMarble() const { return m_firstEndMarble; }
  void setFirstEndMarble(int nMarble) { m_firstEndMarble = nMarble; }

  bool isSecondEndMarble() const { return m_bSecondEndIsMarble; }
  int getSecondEndMarble() const { return m_secondEndMarble; }
  void setSecondEndMarble(int nMarble);
  unsigned int getSecondEndPieceX() const { return m_secondEndPieceX; }
  unsigned int getSecondEndPieceY() const { return m_secondEndPieceY; }
  void setSecondEndPiece(unsigned int x, unsigned int y);

  bool operator == (const RubberBand &other) const;
  bool operator != (const RubberBand &other) const;

private:
  int m_naturalLength;
  int m_force;
  int m_firstEndMarble;
  bool m_bSecondEndIsMarble;
  int m_secondEndMarble;
  unsigned int m_secondEndPieceX;
  unsigned int m_secondEndPieceY;
};

class Block
{
public:
  Block();
  Block(unsigned int x, unsigned int y);
  ~Block();

  unsigned int getX() const { return m_x; }
  void setX(unsigned int x) { m_x = x; }

  unsigned int getY() const { return m_y; }
  void setY(unsigned int y) { m_y = y; }

  bool operator < (const Block &other) const;
  bool operator == (const Block &other) const;
  bool operator != (const Block &other) const;

private:
  unsigned int m_x;
  unsigned int m_y;
};

enum Direction
{
  Direction_Invalid = -1,
  Direction_First = 0,

  Direction_Up = Direction_First,
  Direction_Down,
  Direction_Left,
  Direction_Right,

  Direction_Count,
  Direction_Last = Direction_Count - 1
};

class ScrambleItem
{
public:
  ScrambleItem();
  ScrambleItem(unsigned int x, unsigned int y, Direction dir);
  ~ScrambleItem();

  unsigned int getX() const { return m_x; }
  void setX(unsigned int x) { m_x = x; }

  unsigned int getY() const { return m_y; }
  void setY(unsigned int y) { m_y = y; }

  Direction getDir() const { return m_dir; }
  void setDir(Direction dir) { m_dir = dir; }

private:
  unsigned int m_x;
  unsigned int m_y;
  Direction m_dir;
};

class Laser
{
public:
  Laser();
  Laser(Direction dir, bool bOn);
  ~Laser();

  Direction getDir() const { return m_dir; }
  void setDir(Direction dir) { m_dir = dir; }

  bool getOn() const { return m_bOn; }
  void setOn(bool bOn) { m_bOn = bOn; }

  bool operator == (const Laser &other) const;
  bool operator != (const Laser &other) const;

private:
  Direction m_dir;
  bool m_bOn;
};

class Oscillator
{
public:
  Oscillator();
  Oscillator(unsigned int period, bool bOn);
  ~Oscillator();

  unsigned int getPeriod() const { return m_period; }
  void setPeriod(unsigned int period) { m_period = period; }

  bool getOn() const { return m_bOn; }
  void setOn(bool bOn) { m_bOn = bOn; }

  bool operator == (const Oscillator &other) const;
  bool operator != (const Oscillator &other) const;

private:
  unsigned int m_period;
  bool m_bOn;
};

typedef std::map<Block, Oscillator> OscillatorMap;

class SignalLocation
{
public:
  SignalLocation();
  SignalLocation(unsigned int x, unsigned int y, GridType gridType);
  ~SignalLocation();

  unsigned int getX() const { return m_x; }
  void setX(unsigned int x) { m_x = x; }

  unsigned int getY() const { return m_y; }
  void setY(unsigned int y) { m_y = y; }

  GridType getGridType() const { return m_gridType; }
  void setGridType(GridType gridType) { m_gridType = gridType; }

  bool operator < (const SignalLocation &other) const;
  bool operator == (const SignalLocation &other) const;
  bool operator != (const SignalLocation &other) const
  { return !(*this == other); }

private:
  unsigned int m_x;
  unsigned int m_y;
  GridType m_gridType;
};

class Grid
{
public:
  Grid();
  Grid(int width, int height);
  ~Grid();

  unsigned int getWidth() const { return m_width; }
  unsigned int getHeight() const { return m_height; }

private:
  friend class Level;
  void resize(unsigned int width, unsigned int height);

public:
  unsigned char get(unsigned int x, unsigned int y) const;
  void set(unsigned int x, unsigned int y, unsigned char val);

private:
  unsigned int m_width;
  unsigned int m_height;
  ByteVec m_data;
};

class Level
{
public:
  Level();
  ~Level();

  void clear();

  bool getInit() const { return m_bInit; }
  void setInit(bool bInit) { m_bInit = bInit; }

  bool isEmpty() const { return m_bEmpty; }
  void setEmpty(bool bEmpty) { m_bEmpty = bEmpty; }

  unsigned int getWidth() const { return m_width; }
  unsigned int getHeight() const { return m_height; }
  void resize(unsigned int width, unsigned int height);

  bool getMeditation() const { return m_bMeditation; }
  void setMeditation(bool bMeditation) { m_bMeditation = bMeditation; }

  bool getHarmlessMeditationMarbles() const
  { return m_bHarmlessMeditationMarbles; }
  void setHarmlessMeditationMarbles(bool bHarmlessMeditationMarbles)
  { m_bHarmlessMeditationMarbles = bHarmlessMeditationMarbles; }

  size_t getNumMarbles() const { return m_marbles.size(); }
  void setNumMarbles(int num) { m_marbles.resize(num); }
  const Marble &getMarble(int nMarble) const { return m_marbles[nMarble]; }
  Marble *getMarbleForWrite(int nMarble) { return &m_marbles[nMarble]; }

  int getNumRubberBands(GameMode gameMode) const;
  const RubberBand &getRubberBand(GameMode gameMode, int nRubberBand) const;
  RubberBand *getRubberBandForWrite(GameMode gameMode, int nRubberBand);
  void addRubberBand(GameMode gameMode,
                     const RubberBand &rubberBand,
                     int nRubberBand = -1);
  void removeRubberBand(GameMode gameMode, int nRubberBand);

  int getBlackRubberBandPieceNaturalLength(GameMode gameMode) const;
  void setBlackRubberBandPieceNaturalLength(GameMode gameMode,
                                            int naturalLength);

  int getBlackRubberBandPieceForce(GameMode gameMode) const;
  void setBlackRubberBandPieceForce(GameMode gameMode, int force);

  int getWhiteRubberBandPieceNaturalLength(GameMode gameMode) const;
  void setWhiteRubberBandPieceNaturalLength(GameMode gameMode,
                                            int naturalLength);

  int getWhiteRubberBandPieceForce(GameMode gameMode) const;
  void setWhiteRubberBandPieceForce(GameMode gameMode, int force);

  int getRubberBandObjectNaturalLength(GameMode gameMode) const;
  void setRubberBandObjectNaturalLength(GameMode gameMode, int naturalLength);

  int getRubberBandObjectForce(GameMode gameMode) const;
  void setRubberBandObjectForce(GameMode gameMode, int force);

  int getBlackRubberBandObjectMarble() const;
  void setBlackRubberBandObjectMarble(int nMarble);

  int getWhiteRubberBandObjectMarble() const;
  void setWhiteRubberBandObjectMarble(int nMarble);

  bool getRequireMagicPiece() const
  { return m_bRequireMagicPiece; }
  void setRequireMagicPiece(bool bRequireMagicPiece)
  { m_bRequireMagicPiece = bRequireMagicPiece; }

  bool getScrolling() const { return m_bScrolling; }
  void setScrolling(bool bScrolling) { m_bScrolling = bScrolling; }

  bool getReset(GameMode gameMode) const
  { return m_bReset[gameMode]; }
  void setReset(GameMode gameMode, bool bReset)
  { m_bReset[gameMode] = bReset; }

  bool getWalkThroughPuzzle() const
  { return m_bWalkThroughPuzzle; }
  void setWalkThroughPuzzle(bool bWalkThroughPuzzle)
  { m_bWalkThroughPuzzle = bWalkThroughPuzzle; }

  int getFlatForce(GameMode gameMode) const
  { return m_flatForce[gameMode]; }
  void setFlatForce(GameMode gameMode, int flatForce)
  { m_flatForce[gameMode] = flatForce; }

  int getSlopeForce(GameMode gameMode) const
  { return m_slopeForce[gameMode]; }
  void setSlopeForce(GameMode gameMode, int slopeForce)
  { m_slopeForce[gameMode] = slopeForce; }

  int getFriction(GameMode gameMode) const
  { return m_friction[gameMode]; }
  void setFriction(GameMode gameMode, int friction)
  { m_friction[gameMode] = friction; }

  int getNumScrambleItems() const;
  const ScrambleItem &getScrambleItem(int nScrambleItem) const;
  ScrambleItem *getScrambleItemForWrite(int nScrambleItem);
  void addScrambleItem(const ScrambleItem &scrambleItem,
                       int nScrambleItem = -1);
  void removeScrambleItem(int nScrambleItem);

  const Laser &getLaser(int nLaser) const;
  void setLaser(int nLaser, const Laser &laser);

  const OscillatorMap &getOscillators(GameMode gameMode) const;
  OscillatorMap *getOscillatorsForWrite(GameMode gameMode);

  const std::string &getNoteText(int nNote, Language lang) const;
  void setNoteText(int nNote, Language lang, const std::string &noteText);

  const Grid &getGrid(GridType gridType) const { return m_grids[gridType]; }
  Grid *getGridForWrite(GridType gridType) { return &m_grids[gridType]; }

  const std::vector<std::string> &getSpecialItems() const { return m_specialItems; }
  std::vector<std::string> *getSpecialItemsForWrite() { return &m_specialItems; }

  void getSenders(std::set<SignalLocation> *pSenders) const;
  int getNumRecipients(const SignalLocation &sender) const;
  const SignalLocation &getRecipient(const SignalLocation &sender,
                                     int nRecipient) const;
  void setRecipient(const SignalLocation &sender,
                    int nRecipient,
                    const SignalLocation &recipient);
  void addRecipient(const SignalLocation &sender,
                    const SignalLocation &recipient,
                    int nRecipient = -1);
  void removeRecipient(const SignalLocation &sender,
                       int nRecipient);

private:
  bool m_bInit;
  bool m_bEmpty;

  unsigned int m_width;
  unsigned int m_height;

  bool m_bMeditation;
  bool m_bHarmlessMeditationMarbles;
  std::vector<Marble> m_marbles;
  std::vector<RubberBand> m_rubberBands[GameMode_Count];
  int m_blackRubberBandPieceNaturalLength[GameMode_Count];
  int m_blackRubberBandPieceForce[GameMode_Count];
  int m_whiteRubberBandPieceNaturalLength[GameMode_Count];
  int m_whiteRubberBandPieceForce[GameMode_Count];
  int m_rubberBandObjectNaturalLength[GameMode_Count];
  int m_rubberBandObjectForce[GameMode_Count];
  int m_blackRubberBandObjectMarble;
  int m_whiteRubberBandObjectMarble;
  bool m_bRequireMagicPiece;
  bool m_bScrolling;
  bool m_bReset[GameMode_Count];
  bool m_bWalkThroughPuzzle;
  int m_flatForce[GameMode_Count];
  int m_slopeForce[GameMode_Count];
  int m_friction[GameMode_Count];
  std::vector<ScrambleItem> m_scrambleItems;
  Laser m_lasers[3];
  OscillatorMap m_oscillators[GameMode_Count];
  std::string m_noteText[2][Language_Count];

  Grid m_grids[GridType_Count];

  std::vector<std::string> m_specialItems;

  typedef std::map<SignalLocation, std::vector<SignalLocation> > SignalMap;
  SignalMap m_signals;
};

bool parseNumberList(const std::string &str, std::vector<int> *pNumberList);
bool parseLevel(const ByteVec &in, Level *pLevel, std::string *pMsg = 0);
bool unparseLevel(const Level &level, ByteVec *pOut, std::string *pMsg = 0);

}

#endif
