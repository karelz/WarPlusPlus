#if !defined(AFX_DIRECTDRAW_H__580097B2_47CC_11D2_8EB5_CC7104C10000__INCLUDED_)
#define AFX_DIRECTDRAW_H__580097B2_47CC_11D2_8EB5_CC7104C10000__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// DirectDraw.h : header file
//

#include <ddraw.h>
#include "DirectDrawException.h"
#include "..\..\Events\Events.h"

#define RGB32(r, g, b) (DWORD)((((BYTE)r)<<16) | (((BYTE)g)<<8) | (((BYTE)b)))
#define R32(color) (BYTE)((color & 0x0FF0000) >> 16)
#define G32(color) (BYTE)((color & 0x000FF00) >> 8)
#define B32(color) (BYTE)((color & 0x00000FF))

/////////////////////////////////////////////////////////////////////////////
// CDirectDraw command target

class CDDrawSurface;
class CDDrawClipper;

class CDirectDraw : public CObserver
{
  DECLARE_DYNAMIC(CDirectDraw)
  DECLARE_OBSERVER_MAP(CObserver)

public:
  CDirectDraw();
  virtual ~CDirectDraw();

// Attributes
public:
  typedef enum DisplayModes{
    DM_NoChange,
    DM_320x240x16,
    DM_320x240x24,
    DM_640x480x8,
    DM_640x480x16,
    DM_640x480x24,
    DM_800x600x8,
    DM_800x600x16,
    DM_800x600x24,
    DM_1024x768x8,
    DM_1024x768x16,
    DM_1024x768x24
  }EDisplayModes;

// Operations
public:

// Implementation
public:
	// returns TRUE if the application is in the fullscreen exclusive mode
  BOOL ExclusiveMode();
  // returns TRUE if the object is valid (it means is created)
	virtual BOOL IsValid();
  // deletes given clipper object
	void DeleteClipper(CDDrawClipper *pDDClipper);
  // creates clipper object
	BOOL CreateClipper(CDDrawClipper *pDDClipper);
  // destryes the g_pDirectDraw object
	static void Close();
  // inits the DirectDraw -> creates the g_pDirectDraw
	static BOOL Init(CWnd *pWnd, EDisplayModes eDisplayMode = DM_NoChange, DWORD dwFlags = DDSCL_NORMAL);
  // returns TRUE if current video mode is 32bit (else it's 24bit)
  BOOL Is32BitMode(){ return m_b32BitMode; }
  // deletes the given surface
  void DeleteSurface(CDDrawSurface *pDDSurface);
  // returns pointer to the IDirectDraw OLE interface
	LPDIRECTDRAW GetLP();
  // creates the surface
	BOOL CreateSurface(CDDrawSurface *pDDSurface, DDSURFACEDESC *pddsd = NULL);
  // deletes this object
	void Delete();
  // creates the DirectDraw object
	BOOL Create(CWnd *wnd, EDisplayModes eDisplayMode = DM_NoChange, DWORD dwFlags = DDSCL_NORMAL);

#ifdef _DEBUG
  virtual void Dump(CDumpContext &dc) const;
	virtual void AssertValid() const;
#endif

private:
	// TRUE - the application has exclusive fullscreen mode
  // FALSE - normal windowed mode
  BOOL m_bExclusiveMode;
  // list of clippers created on this DirectDraw object
	CDDrawClipper * m_pClippers;
  // TRUE - the current video mode is 32-bit
  // FALSE - some other bit count (most usually 24-bit)
  // used for inner functions to determine which mode to use
	BOOL m_b32BitMode;
	// A list of created surfaces
  CDDrawSurface *m_pSurfaces;

	// helper for results from DirectX
  HRESULT m_hResult;
  // pointer to DirectDraw object
	LPDIRECTDRAW m_lpDD;
protected:
	void OnAbort(DWORD dwExitCode);
};

extern CDirectDraw *g_pDirectDraw;

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DIRECTDRAW_H__580097B2_47CC_11D2_8EB5_CC7104C10000__INCLUDED_)
