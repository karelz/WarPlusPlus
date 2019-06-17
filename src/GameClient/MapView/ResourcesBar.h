// ResourcesBar.h: interface for the CResourcesBar class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_RESOURCESBAR_H__B40B79C5_5A33_11D4_B0BB_004F49068BD6__INCLUDED_)
#define AFX_RESOURCESBAR_H__B40B79C5_5A33_11D4_B0BB_004F49068BD6__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "Common\Map\Map.h"

class CResourcesBar : public CWindow  
{
  DECLARE_DYNAMIC(CResourcesBar);
  DECLARE_OBSERVER_MAP(CResourcesBar);

public:
  // Constructor & Destructor
	CResourcesBar();
	virtual ~CResourcesBar();

  // Debug functions
#ifdef _DEBUG
  virtual void AssertValid() const;
  virtual void Dump(CDumpContext &dc) const;
#endif

  // Creation
  // Creates the window at the top of the given parent window
  void Create(CWindow *pParent, CFontObject *pFont);
  // Deletes the object
  virtual void Delete();

  // Sets new resource count
  void SetResource(DWORD dwIndex, int nNewValue);

  virtual void Draw(CDDrawSurface *pSurface, CRect *pRect);

protected:
  // Returns the window from givven screen point
  // works only on subtree of windows
	virtual CWindow * WindowFromPoint(CPoint &pt);

private:
  // array of resources
  int m_aResources[RESOURCE_COUNT];

  // array of points where to draw the resources
  CPoint m_aPositions[RESOURCE_COUNT];

  // Font to draw the numbers
  CFontObject *m_pFont;
};

#endif // !defined(AFX_RESOURCESBAR_H__B40B79C5_5A33_11D4_B0BB_004F49068BD6__INCLUDED_)
