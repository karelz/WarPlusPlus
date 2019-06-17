#if !defined(AFX_DDPRIMARYSURFACE_H__580097B4_47CC_11D2_8EB5_CC7104C10000__INCLUDED_)
#define AFX_DDPRIMARYSURFACE_H__580097B4_47CC_11D2_8EB5_CC7104C10000__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// DDPrimarySurface.h : header file
//

#include "DDrawSurface.h"
#include "DDrawClipper.h"

/////////////////////////////////////////////////////////////////////////////
// CDDPrimarySurface command target
// the primary surface for the application

class CDDPrimarySurface : public CDDrawSurface
{
	DECLARE_DYNAMIC(CDDPrimarySurface)

public:	
  // constructor of the object (do nothing)
  CDDPrimarySurface();           // protected constructor used by dynamic creation

// Attributes
public:

// Operations
public:

// Implementation
public:
	virtual void DoPaste(LONG lX, LONG lY, CDDrawSurface * pSrcSurface, CRect * pRectSource);
	// restore the surface (sets the permanent clipper)
  virtual void Restore();
	// sets the window for which is this surface to be clipped
  // set it before creation of the surface
  void SetClipWindow(CWnd *pWnd);
  // sets wanted capabilities of the surface
  // set it before you create the Primary surface
  // afetr create has no effect
	void SetCaps(DWORD dwCaps);
  // Returns the back buffer for this primary surface
  // !!!! works only if there's only one back buffer
  // for more back buffers you have to use another methods
	CDDrawSurface * GetBackBuffer();
  // Deletes the surface
	virtual void Delete();
  // Creates the surface
  // don't call it directly (instead call the CDirectDraw::CreateSurface
	virtual BOOL PostCreate(LPDIRECTDRAWSURFACE lpDDSurface, BOOL bCallRelease = TRUE);
  // Return the back buffer count
	DWORD GetBackBufferCount();
  // Sets the back buffer count
  // !!! Has to be called before creating the surface
	void SetBackBufferCount(DWORD dwBackBufferCount);
  // Don't call it
	virtual BOOL PreCreate(LPDDSURFACEDESC ddsd);
  // Flips the primary and back buffer
	virtual BOOL Flip();
  // destroyes the object
  virtual ~CDDPrimarySurface();

#ifdef _DEBUG
  virtual void Dump(CDumpContext &dc) const;
	virtual void AssertValid() const;
#endif

protected:
	// Creates the back buffer in the given object
  BOOL CreateBackBuffer(CDDrawSurface &BackBuffer);
  // The number of back buffers
	DWORD m_dwBackBufferCount;
  // The back buffer surface
	CDDrawSurface m_BackBuffer;
private:
	CWnd * m_pClipWindow;
  // a clipper object for this surface
  CDDrawClipper m_Clipper;
  // TRUE if we does have a clipper
  BOOL m_bHasClipper;
  // capabilities
	DWORD m_dwCaps;
};

// The one and only primary surface in the application
// If NULL no primary surface was created
extern CDDPrimarySurface *g_pDDPrimarySurface;

/////////////////////////////////////////////////////////////////////////////

#endif // !defined(AFX_DDPRIMARYSURFACE_H__580097B4_47CC_11D2_8EB5_CC7104C10000__INCLUDED_)
