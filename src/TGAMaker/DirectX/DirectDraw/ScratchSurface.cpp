// ScratchSurface.cpp : implementation file
//

#include "stdafx.h"
#include "ScratchSurface.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CScratchSurface

IMPLEMENT_DYNAMIC(CScratchSurface, CDDrawSurface)

CScratchSurface::CScratchSurface()
{
  // Some default sizes
  // Zeores are ilegal so the smallest one
  m_dwWidth = 1; m_dwHeight = 1;
  m_pPF = NULL;
}

CScratchSurface::~CScratchSurface()
{
}


/////////////////////////////////////////////////////////////////////////////
// CScratchSurface message handlers

// Sets parametrs for a sratch surface of given sizes
// set the sizes with SetWidth and SetHeight
BOOL CScratchSurface::PreCreate(DDSURFACEDESC * pddsd)
{
  pddsd->dwFlags = DDSD_CAPS | DDSD_WIDTH | DDSD_HEIGHT | DDSD_PIXELFORMAT;
  pddsd->ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN;
  pddsd->dwWidth = m_dwWidth; pddsd->dwHeight = m_dwHeight;
  
  if(m_pPF == NULL){
    pddsd->ddpfPixelFormat.dwSize = sizeof(pddsd->ddpfPixelFormat);
    pddsd->ddpfPixelFormat.dwFlags = DDPF_RGB;
    if(g_pDirectDraw->Is32BitMode())
      pddsd->ddpfPixelFormat.dwRGBBitCount = 32;
    else
      pddsd->ddpfPixelFormat.dwRGBBitCount = 24;
    pddsd->ddpfPixelFormat.dwRBitMask = 0x00FF0000;
    pddsd->ddpfPixelFormat.dwGBitMask = 0x0000FF00;
    pddsd->ddpfPixelFormat.dwBBitMask = 0x000000FF;
  }
  else{
    memcpy(&pddsd->ddpfPixelFormat, m_pPF, sizeof(DDPIXELFORMAT));
    m_bUserPixelFormat = TRUE;
  }

  return TRUE;
}

void CScratchSurface::SetWidth(DWORD dwWidth)
{
  // if the surface is not still created you can change the sizes
  if(m_lpDDSurface == NULL){
    m_dwWidth = dwWidth;
  }
}

void CScratchSurface::SetHeight(DWORD dwHeight)
{
  // if the surface is not still created you can change the sizes
  if(m_lpDDSurface == NULL){
    m_dwHeight = dwHeight;
  }
}

/////////////////////////////////////////////////////////////////////////////
// Debug methods

#ifdef _DEBUG

void CScratchSurface::AssertValid() const
{
  CDDrawSurface::AssertValid();
}

void CScratchSurface::Dump(CDumpContext & dc) const
{
  CDDrawSurface::Dump(dc);
}

#endif

void CScratchSurface::SetAlphaChannel(BOOL bHasAlphaChannel)
{
  if(m_lpDDSurface == NULL){
    m_bHasAlphaChannel = bHasAlphaChannel;
  }
}

void CScratchSurface::SetPixelFormat(DDPIXELFORMAT *pPF)
{
  if(m_lpDDSurface == NULL){
    m_pPF = pPF;
  }
}
