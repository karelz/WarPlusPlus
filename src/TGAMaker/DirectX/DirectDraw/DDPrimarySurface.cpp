// DDPrimarySurface.cpp : implementation file
//

#include "stdafx.h"
#include "DDPrimarySurface.h"
#include "DirectDrawException.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDDPrimarySurface

// The only one primary surface
// use this variable to access the display
CDDPrimarySurface *g_pDDPrimarySurface = NULL;

IMPLEMENT_DYNAMIC(CDDPrimarySurface, CDDrawSurface)

CDDPrimarySurface::CDDPrimarySurface()
{
  m_dwBackBufferCount = 0;
  m_dwCaps = DDSCAPS_PRIMARYSURFACE;
  m_bHasClipper = FALSE;
  m_pClipWindow = NULL;
}

CDDPrimarySurface::~CDDPrimarySurface()
{
}

/////////////////////////////////////////////////////////////////////////////
// Debug methods

#ifdef _DEBUG

void CDDPrimarySurface::AssertValid() const
{
  CDDrawSurface::AssertValid();  
}

void CDDPrimarySurface::Dump(CDumpContext & dc) const
{
  CDDrawSurface::Dump(dc);
  dc << "Back buffer count : " << m_dwBackBufferCount << "\n";
  dc << "Has clipper : " << m_bHasClipper << "\n";
}

#endif

/////////////////////////////////////////////////////////////////////////////
// CDDPrimarySurface message handlers

// Returns the back buffer of this surface it has to be only one
// if you will have more than one back buffer
// don't use this function it will fail
BOOL CDDPrimarySurface::CreateBackBuffer(CDDrawSurface & BackBuffer)
{
  DDSCAPS ddscaps;

  ASSERT_POINTER(m_lpDDSurface, 1);

  ddscaps.dwCaps = DDSCAPS_BACKBUFFER;
  return GetAttachedSurface(BackBuffer, ddscaps);
}

// Flips the primary surface
// will wait for the filpper to finish its operation
BOOL CDDPrimarySurface::Flip()
{
  ASSERT_POINTER(m_lpDDSurface, 1);

  m_hResult = m_lpDDSurface->Flip(NULL, DDFLIP_WAIT);
  if(m_hResult == DDERR_SURFACELOST){
    // if the surface is lost -> restore it
    // no flip -> next frame will do it
    Restore();
    return FALSE;
  }
  DIRECTDRAW_ERROR(m_hResult);

  return TRUE;
}

// Sets the caps for primary surface and flipping surface
// to use it we have to have EXCLUSIVE mode set
BOOL CDDPrimarySurface::PreCreate(LPDDSURFACEDESC pddsd)
{
  ASSERT(pddsd);

  if(m_dwBackBufferCount == 0){
    pddsd->dwFlags = DDSD_CAPS;
    pddsd->ddsCaps.dwCaps = m_dwCaps;
  }
  else{
    pddsd->dwFlags = DDSD_CAPS | DDSD_BACKBUFFERCOUNT;
    pddsd->ddsCaps.dwCaps = m_dwCaps;
    pddsd->dwBackBufferCount = m_dwBackBufferCount;
  }

  // Set that this surface has user pixel format, because we don't care about the
  // pixel format of the primary surface (we can't change it directly)
  m_bUserPixelFormat = TRUE;

  return TRUE;
}

BOOL CDDPrimarySurface::PostCreate(LPDIRECTDRAWSURFACE lpDDSurface, BOOL bCallRelease)
{
  if(!CDDrawSurface::PostCreate(lpDDSurface, bCallRelease)) return FALSE;

  if(m_dwCaps & DDSCAPS_FLIP) m_bUseBltFast = TRUE;
  else m_bUseBltFast = FALSE;

  if(m_dwBackBufferCount == 1){
    if(!CreateBackBuffer(m_BackBuffer)) return FALSE;
	}

  // if some window to clip for do it
  if(m_pClipWindow != NULL){
    // we does have a clipper
    m_bHasClipper = TRUE;
    // create it and set it
    if(!m_Clipper.Create(m_pClipWindow)) return FALSE;
    SetClipper(&m_Clipper);
  }
  else{
    m_bHasClipper = FALSE;
  }

  // If there was another primary surface -> error
  ASSERT(g_pDDPrimarySurface == NULL);
  // This is the only primary surface
  g_pDDPrimarySurface = this;

  return TRUE;
}

void CDDPrimarySurface::Delete()
{
  CDDrawSurface::Delete();
  m_BackBuffer.Delete();
  m_Clipper.Delete();

  // assuming we was the only on eprimary surface
  // if not there must be some assertion fail (see Create)
  g_pDDPrimarySurface = NULL;
}

void CDDPrimarySurface::SetBackBufferCount(DWORD dwBackBufferCount)
{
  // if the surface is still not created we can change the value
  ASSERT(m_lpDDSurface == NULL);
  if(m_lpDDSurface == NULL){
    m_dwBackBufferCount = dwBackBufferCount;
  }
}

void CDDPrimarySurface::SetCaps(DWORD dwCaps)
{
  ASSERT(m_lpDDSurface == NULL);
  if(m_lpDDSurface == NULL){
    m_dwCaps = DDSCAPS_PRIMARYSURFACE | dwCaps;
  }
}

DWORD CDDPrimarySurface::GetBackBufferCount()
{
  return m_dwBackBufferCount;
}

CDDrawSurface * CDDPrimarySurface::GetBackBuffer()
{
  ASSERT(m_dwBackBufferCount == 1);

  return &m_BackBuffer;
}

void CDDPrimarySurface::SetClipWindow(CWnd *pWnd)
{
  m_pClipWindow = pWnd;
}

void CDDPrimarySurface::Restore()
{
  CDDrawSurface::Restore();

  if(m_bHasClipper){
    SetClipper(&m_Clipper);
  }
}

void CDDPrimarySurface::DoPaste(LONG lX, LONG lY, CDDrawSurface * pSrcSurface, CRect * pRectSource)
{
  if(!m_bHasClipper){
    CDDrawSurface::DoPaste(lX, lY, pSrcSurface, pRectSource);
  }
  else{
    // we just have to move the coords of the paste to origin of the window
    CRect rcWindow;

    m_pClipWindow->GetClientRect(&rcWindow);
    m_pClipWindow->ClientToScreen(&rcWindow);
    
    BltFast(lX + rcWindow.left, lY + rcWindow.top, pSrcSurface, pRectSource);
  }
}