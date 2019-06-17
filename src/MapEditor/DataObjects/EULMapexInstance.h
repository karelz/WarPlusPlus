// EULMapexInstance.h: interface for the CEULMapexInstance class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_EULMAPEXINSTANCE_H__23152137_6E73_11D3_A06B_C85E707FE131__INCLUDED_)
#define AFX_EULMAPEXINSTANCE_H__23152137_6E73_11D3_A06B_C85E707FE131__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "EMapex.h"

class CEULMapexInstance
{
public:
	void SetZPos(DWORD dwZPos);
	BOOL PtInMapex(DWORD dwX, DWORD dwY);
	void SetPosition(DWORD dwXPos, DWORD dwYPos);
	void DrawLandTypesSelected(CPoint ptTopLeft, CRect *pRectDest, CDDrawSurface *pSurface, CDDrawSurface *pBuffer, CDDrawSurface *pSelectionLayer);
	void DrawLandTypes(CPoint ptTopLeft, CRect *pRectDest, CDDrawSurface *pSurface);
	void DrawSelected(CPoint ptTopLeft, CRect *pRectDest, CDDrawSurface *pSurface, CDDrawSurface *pBuffer, CDDrawSurface *pSelectionLayer);
	void Draw(CPoint ptTopLeft, CRect *pRectDest, CDDrawSurface *pSurface);
  // returns the position
  DWORD GetZPosition(){ return m_dwZ; }
  DWORD GetYPosition(){ return m_dwY; }
  DWORD GetXPosition(){ return m_dwX; }
  // returns the mapex object
  CEMapex *GetMapex(){ return m_pMapex; }
  // creates the UL mapex from file
	BOOL Create(CArchiveFile file);
  // saves the UL mapex to the file
	void SaveToFile(CArchiveFile file);
  // creates new UL mapex instance
	BOOL Create(CEMapex *pMapex, DWORD dwX, DWORD dwY, DWORD dwZ);

  // constructor & destructor
	CEULMapexInstance();
	~CEULMapexInstance();

private:
  DWORD m_dwX, m_dwY, m_dwZ; // coords on the map (in mapcells)
  CEMapex *m_pMapex; // pointer to the mapex

  CEULMapexInstance *m_pNext; // next one in the list

  friend class CEMapSquare;
};

#endif // !defined(AFX_EULMAPEXINSTANCE_H__23152137_6E73_11D3_A06B_C85E707FE131__INCLUDED_)
