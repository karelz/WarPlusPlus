// EUnit.h: interface for the CEUnit class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_EUNIT_H__79EF3FE3_EC61_11D3_8454_004F4E0004AA__INCLUDED_)
#define AFX_EUNIT_H__79EF3FE3_EC61_11D3_8454_004F4E0004AA__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "EUnitType.h"
#include "ECivilization.h"
#include "EUnitAnimation.h"

class CEUnit
{
public:
  // constructor
	CEUnit();
  // destructor
	~CEUnit();

  // creates the unit instance (sets position to zeroes)
  void Create(CEUnitType *pUnitType, CECivilization *pCivilization);
  // creates the unit instance from file
  void Create(CArchiveFile file);
  // deletes the object
  void Delete();

  // save the object to file
  void Save(CArchiveFile file);

  // Checks if the unit is valid
  BOOL CheckValid();

  // draw normal
  void Draw(CPoint ptTopLeft, CRect *pRectDest, CDDrawSurface *pSurface);
  // draw selected
  void DrawSelected(CPoint ptTopLeft, CRect *pRectDest, CDDrawSurface *pSurface, CDDrawSurface *pBuffer, CDDrawSurface *pSelectionLayer);

  // returns the unit type
  CEUnitType *GetUnitType(){ return m_pUnitType; }
  // returns the civilization
  CECivilization *GetCivilization(){ return m_pCivilization; }

  // returns the position coords
  DWORD GetXPos(){ return m_dwX; }
  DWORD GetYPos(){ return m_dwY; }
  DWORD GetZPos(){ return m_dwZ; }

  BYTE GetDirection(){ return m_nDirection; }
  void SetDirection(BYTE nDirection = 8){
    ASSERT((nDirection >= 0) && (nDirection <= 8));
    m_nDirection = nDirection;
  }

  // sets new position
  void SetPos(DWORD dwX, DWORD dwY, DWORD dwZ){
    m_dwX = dwX; m_dwY = dwY; m_dwZ = dwZ;
  }

  // returns if given point (in mapcells) lies inside the unit
  BOOL PtInUnit(DWORD dwX, DWORD dwY);

  // computes the x,y translation from the top left point of the unit rect
  int GetXTransition();
  int GetYTransition();

  // returns the default graphics size
  CSize GetDefaultGraphicsSize();

private:
  // helper function
  // returns first usable direction (default appearance)
  CEUnitAnimation *GetDefaultDirection();

  // unit type
  CEUnitType *m_pUnitType;
  // civilization of the unit
  CECivilization *m_pCivilization;

  // map position
  DWORD m_dwX;
  DWORD m_dwY;
  DWORD m_dwZ;

  // direction of the unit
  BYTE m_nDirection;

  // next unit in mapsquare
  CEUnit *m_pNext;

  friend class CEMapSquare;
};

#endif // !defined(AFX_EUNIT_H__79EF3FE3_EC61_11D3_8454_004F4E0004AA__INCLUDED_)
