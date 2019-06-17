// EMapexInstance.h: interface for the CEMapexInstance class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_EMAPEXINSTANCE_H__23152136_6E73_11D3_A06B_C85E707FE131__INCLUDED_)
#define AFX_EMAPEXINSTANCE_H__23152136_6E73_11D3_A06B_C85E707FE131__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "EMapex.h"

class CEMapexInstance  
{
public:
	void DrawLandTypesSelected(CPoint ptTopLeft, CRect *pRectDest, CDDrawSurface *pSurface, CDDrawSurface *pBuffer, CDDrawSurface *pSelectionLayer);
	void DrawLandTypes(CPoint ptTopLeft, CRect *pRectDest, CDDrawSurface *pSurface);
	void SetPosition(DWORD dwX, DWORD dwY);
	BOOL PtInMapex(DWORD dwX, DWORD dwY);
  // returns position
  DWORD GetXPosition() { return m_dwX; }
  DWORD GetYPosition() { return m_dwY; }
  // returns mapex
  CEMapex *GetMapex() { return m_pMapex; }

  // draws mapex instance
  // ptTopLeft position of the top left corner in pixels on the dest surface
  // pRectDest destination rectangle (clipping one)
  // pSurface destination surface
	void Draw(CPoint ptTopLeft, CRect *pRectDest, CDDrawSurface *pSurface);
  // same as the Draw
  // pBuffer , buffer for some graphical operations (contents will be destroyed)
  // pSelectionLayer, surface with selection overlay (must be bigger than the pBuffer)
	void DrawSelected(CPoint ptTopLeft, CRect *pRectDest, CDDrawSurface *pSurface, CDDrawSurface *pBuffer, CDDrawSurface *pSelectionLayer);

  // saves mapex instance to file
	void SaveToFile(CArchiveFile file);
  // creates mapex instance from file
	BOOL Create(CArchiveFile file);
  // creates the new mapex instance for the mapex (pMapex) on coords (dwX, dwY)
	BOOL Create(CEMapex *pMapex, DWORD dwX, DWORD dwY);

  // constructor & destructor
	CEMapexInstance();
	~CEMapexInstance();

private:
  DWORD m_dwX, m_dwY; // coords on the map (in mapcells)
  CEMapex *m_pMapex;  // pointer to the mapex

  CEMapexInstance *m_pNext; // next one in the list

  friend class CEMapSquare;
};

#endif // !defined(AFX_EMAPEXINSTANCE_H__23152136_6E73_11D3_A06B_C85E707FE131__INCLUDED_)
