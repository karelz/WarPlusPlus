#ifndef CMAPEXINSTANCE_H_
#define CMAPEXINSTANCE_H_

#include "CMapex.h"

class CCMap;

// Class representing one mapex instance (BL mapex)
class CCMapexInstance // no base class - need small object
{
public:
  // constructor & destructor
  CCMapexInstance();
  ~CCMapexInstance();

// Creation
  // Creates the object from the file
  // need the map object to find the mapex by ID
  void Create(CArchiveFile MapFile, CCMap *pMap);

  // Deletes the object
  void Delete();

// Get/Set methods
  // returns the mapex (type) object
  CCMapex *GetMapex(){ return m_pMapex; }

  // returns the position (in mapcells)
  DWORD GetXPosition(){ return m_dwXPosition; }
  DWORD GetYPosition(){ return m_dwYPosition; }

// Drawing functions
  // Draws the mapex into the given surface
  // must know the coords (top-left) of the surface in mapcells
  void Draw(DWORD dwXPos, DWORD dwYPos, CDDrawSurface *pSurface);

private:
  // pointer to the mapex (type) object
  CCMapex *m_pMapex;

  // position
  DWORD m_dwXPosition;
  DWORD m_dwYPosition;

  // next mapex instance in the list of them at map square
  CCMapexInstance *m_pNext;

  // map square must be friend to be able to manipulate with the list of us
  friend class CCMapSquare;
};

#endif