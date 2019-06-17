#if !defined(AFX_DDRAWSURFACE_H__580097B3_47CC_11D2_8EB5_CC7104C10000__INCLUDED_)
#define AFX_DDRAWSURFACE_H__580097B3_47CC_11D2_8EB5_CC7104C10000__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// DDrawSurface.h : header file
//

#include "DirectDraw.h"

/////////////////////////////////////////////////////////////////////////////
// CDDrawSurface command target
// The direct draw surface object

class CDDrawSurface : public CObject
{
  DECLARE_DYNAMIC(CDDrawSurface);

public:	
  // constructs the object (do nothing)
  CDDrawSurface();           // protected constructor used by dynamic creation

  // the constant for no key color
  static const DWORD m_dwNoKeyColor;

// Attributes
public:

// Operations
public:

// Implementation
public:
	virtual BOOL IsValid();
	void SetClipper(CDDrawClipper *pDDClipper);
  // Creates this surface
  // it need the DirectDraw to be initialized
  // it means the CDirectDraw object exists and was created
  // it will use the global pointer to the DirectDraw object
  // to create this surface
  // it just call g_pDirectDraw->CreateSurface
  BOOL Create();
  // returns TRUE if the surface has the alpha channel
  // or if it has the keycolor set ->
  // there could be transparent pixels in it
  BOOL IsTransparent();
	// Sets the clipping rectangle for the surface
  virtual void SetClipRect(CRect *pRectNew, CRect *rcOld);
  // Returns the clipping rectangle for the surface
	virtual void GetClipRect(CRect *rc);
  // Returns pointer to a memory, where the alpha channel
  // is stored
  // !!! be carefull with using it, no ranges are guarded
	virtual BYTE * GetAlphaChannel();
  // Returns the transformation for the surface
	virtual CPoint GetTransformation();
  // Sets the transformation for the surface
  // and returns the old one
	virtual CPoint SetTransformation(CPoint pointNew);
  // Pastes the given rectangle from source surface to this surface
	virtual void Paste(LONG lX, LONG lY, CDDrawSurface *pSrcSurface, CRect *pRectSource);
  // Pastes the given rectangle from source surface to this surface
  void Paste(CPoint &pt, CDDrawSurface *pSrcSurface, CRect *pRectSource){ Paste(pt.x, pt.y, pSrcSurface, pRectSource); }
  // Pastes the given rectangle from source surface to this surface
  void Paste(LONG lX, LONG lY, CDDrawSurface *pSrcSurface){ Paste(lX, lY, pSrcSurface, NULL); }
  // Pastes the given rectangle from source surface to this surface
  void Paste(CPoint &pt, CDDrawSurface *pSrcSurface){ Paste(pt.x, pt.y, pSrcSurface, NULL); }

  // Clips the rectangles for blitting
  // don't call it - it's used internaly
  virtual BOOL ClipBltFastRect(CRect *pRectDest, CRect *pRectSource, CDDrawSurface *pSrcSurface, LONG lX, LONG lY);
	// returns the key color for this surface
  DWORD GetColorKey();
  // returns the rectangle for the whole surface
  // allways has top left at (0, 0)
  CRect * GetAllRect(){ ASSERT_VALID(this); return &m_rectAll; }
  // returns height of the surface
	DWORD GetHeight() const;
  // returns width of the surface
	DWORD GetWidth() const;
  // Unlocks the surface
  // you have to specify the pointer to the surface memory
	void Unlock(LPVOID lpSurface);
  // Locks the surface (rectangle on it)
  // allways use the returned pitch
	LPVOID Lock(CRect *pLockRect, DWORD Flags, DWORD &dwPitch);
  // Sets the key color for the surface
	void SetColorKey(DWORD dwKeyColor);
  // Returns the description of the surface
	void GetSurfaceDesc(LPDDSURFACEDESC lpddsd);
  // Fills the rectangle of the surface with given color
	void Fill(DWORD dwFillColor, CRect *DestRect = NULL);
  // Blt fast
	void BltFast(LONG lX, LONG lY, CDDrawSurface *pSrcSurface, CRect *pSrcRect = NULL, DWORD dwTrans = 0);
  // Blit
	void Blt(CRect *pDestRect, CDDrawSurface *pSrcSurface, CRect *pSrcRect, DWORD dwFlags = 0, LPDDBLTFX lpDDBltFx = NULL);
  // Next surface
  // !!!!! Don't touch it !!!!!
	CDDrawSurface * m_pNextSurface;
  // !!!!! Never call it !!!!!
  // Sets the parameters of the surface for the CDirectDraw object
  // and it will create this one from the parameters
  // Fill the DDSURFACEDESC structure
  // Returns FALSE if something failed
	virtual BOOL PreCreate(LPDDSURFACEDESC ddsd);
  // Releases the DC returned by GetDC
  void ReleaseDC(CDC *dc);
  // Gets the DC for this surface
	CDC *GetDC();
  // Restores the surface memory
	virtual void Restore();
  // Returns the LP for the OLE interface
	LPDIRECTDRAWSURFACE GetLP();
	BOOL GetAttachedSurface(CDDrawSurface &AttachedSurface, DDSCAPS &ddscaps);
	// Use CDirectDraw::CreateSurface instead
  virtual BOOL PostCreate(LPDIRECTDRAWSURFACE lpDDSurface, BOOL bCallRelease = TRUE);
	// Deletes the surface
  virtual void Delete();
  // Destroys the surface
  virtual ~CDDrawSurface();

#ifdef _DEBUG
  virtual void Dump(CDumpContext &dc) const;
	virtual void AssertValid() const;
#endif

private:
	// The clipping rectangle
  CRect m_rectClip;
  // Blits with alpha channel (transparency is computed)
	virtual void BltAlpha(LONG lX, LONG lY, CDDrawSurface * pSrcSurface, CRect * pSrcRect);
	// Pointer to DirectDraw object - on delete I will use it
  CDirectDraw * m_pDirectDraw;
  friend class CDirectDraw;
  // Use or not the key color
	BOOL m_bUseKeyColor;
  // Do we have to call Release on the interface
  // if this object represents for example the back buffer
  // it can't call Release
	BOOL m_bCallRelease;
  // Rect covering the whole surface
  CRect m_rectAll;
protected:
	// Paste called by all the Paste functions
  // It's here because of the mouse
  // it has to have an intermediate level of calles there
  virtual void DoPaste(LONG lX, LONG lY, CDDrawSurface *pSrcSurface, CRect *pRectSource);
	// The transformation fo the surface (can be negative)
  CPoint m_pointTransformation;
  // The pixel format of this surface is givven by user
  // some wierd format is supported this way
  // (for example primary surface should have this flag set, because we didn't create it)
	BOOL m_bUserPixelFormat;
  // Do we have the alpha or not
	BOOL m_bHasAlphaChannel;
  // Pointer to the alpha channel memory
	BYTE * m_pAlphaChannel;
	// Do we have to remove us from the list of surfaces
  BOOL m_bRemoveFromList;
  // Can we use the BltFast
  // if the surface is in the system memory we can't
	BOOL m_bUseBltFast;
  // sizes of the surface
  // height of the surface
  DWORD m_dwHeight;
  // width of the surface
	DWORD m_dwWidth;

	// variable to store the results of calls to OLE interface
  HRESULT m_hResult;
  // Pointer to the OLE interface
	LPDIRECTDRAWSURFACE m_lpDDSurface;

  friend CDirectDraw;
};

/////////////////////////////////////////////////////////////////////////////

#endif // !defined(AFX_DDRAWSURFACE_H__580097B3_47CC_11D2_8EB5_CC7104C10000__INCLUDED_)
