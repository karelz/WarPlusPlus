#if !defined(AFX_SCRATCHSURFACE_H__3A383C84_53D1_11D2_8EB5_947204C10000__INCLUDED_)
#define AFX_SCRATCHSURFACE_H__3A383C84_53D1_11D2_8EB5_947204C10000__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// ScratchSurface.h : header file
//

#include "DDrawSurface.h"

/////////////////////////////////////////////////////////////////////////////
// CScratchSurface command target

class CScratchSurface : public CDDrawSurface
{
	DECLARE_DYNAMIC(CScratchSurface)

public:
	CScratchSurface();           // protected constructor used by dynamic creation

// Attributes
public:

// Operations
public:

// Implementation
public:
	virtual void Restore();
	void SetPixelFormat(DDPIXELFORMAT *pPF);
	void SetAlphaChannel(BOOL bHasAlphaChannel);
	void SetHeight(DWORD dwHeight);
	void SetWidth(DWORD dwWidth);
  // Sets the video memory flag
  // TRUE -> the surface should be created in video memory
  //         if there is not enough video memory the surface will be created in system memory
  // FALSE -> the surface must be created in system memory
  void SetVideoMemory(BOOL bVideoMemory = TRUE){ m_bVideoMemory = bVideoMemory; }
	virtual BOOL PreCreate(DDSURFACEDESC *ddsd);
  virtual ~CScratchSurface();

#ifdef _DEBUG
  virtual void Dump(CDumpContext &dc) const;
	virtual void AssertValid() const;
#endif

private:
	DDPIXELFORMAT * m_pPF;
  // TRUE - if the surface is about to be created in the video memory
  BOOL m_bVideoMemory;
};

/////////////////////////////////////////////////////////////////////////////

#endif // !defined(AFX_SCRATCHSURFACE_H__3A383C84_53D1_11D2_8EB5_947204C10000__INCLUDED_)
