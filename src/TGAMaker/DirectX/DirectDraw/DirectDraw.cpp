// DirectDraw.cpp : implementation file
//

#include "stdafx.h"
#include "DirectDraw.h"
#include "DDrawSurface.h"
#include "DDrawClipper.h"
#include "..\..\Events\Events.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDirectDraw

CDirectDraw *g_pDirectDraw = NULL;

IMPLEMENT_DYNAMIC(CDirectDraw, CObserver);

BEGIN_OBSERVER_MAP(CDirectDraw, CObserver)
  BEGIN_ABORT()
    ON_ABORT()
  END_ABORT()
END_OBSERVER_MAP(CDirectDraw, CObserver)

CDirectDraw::CDirectDraw()
{
  m_lpDD = NULL;
  m_pSurfaces = NULL;
  m_pClippers = NULL;
  m_b32BitMode = FALSE;
  m_bExclusiveMode = FALSE;
}

CDirectDraw::~CDirectDraw()
{
  Delete();
}


/////////////////////////////////////////////////////////////////////////////
// CDirectDraw message handlers

// Initialize the object
// Creates the DirectDraw object and sets coop level and display mode
BOOL CDirectDraw::Create(CWnd * wnd, EDisplayModes eDisplayMode, DWORD dwFlags)
{
  DWORD dwWidth, dwHeight, dwBPP;

#ifdef _DEBUG
  if(wnd != NULL){
    ASSERT_VALID(wnd);
    ASSERT_KINDOF(CWnd, wnd);
  }
#endif
  
  // Create the DirectDraw OLE object by calling the API function
  m_hResult = DirectDrawCreate(NULL, &m_lpDD, NULL);
  DIRECTDRAW_ERROR(m_hResult);

  // Set cooperative level for givven window
  m_hResult = m_lpDD->SetCooperativeLevel(wnd->GetSafeHwnd(), dwFlags);
  DIRECTDRAW_ERROR(m_hResult);

  // does we have the exclusive mode
  if(dwFlags & DDSCL_FULLSCREEN){
    m_bExclusiveMode = TRUE;
  }
  else{
    m_bExclusiveMode = FALSE;
  }

  // Get sizes from predefined modes
  switch(eDisplayMode){
  case DM_320x240x16:  dwWidth = 320;  dwHeight = 240; dwBPP = 16; break;
  case DM_320x240x24:  dwWidth = 320;  dwHeight = 240; dwBPP = 24; break;
  case DM_640x480x8:   dwWidth = 640;  dwHeight = 480; dwBPP = 8;  break;
  case DM_640x480x16:  dwWidth = 640;  dwHeight = 480; dwBPP = 16; break;
  case DM_640x480x24:  dwWidth = 640;  dwHeight = 480; dwBPP = 24; break;
  case DM_800x600x8:   dwWidth = 800;  dwHeight = 600; dwBPP = 8;  break;
  case DM_800x600x16:  dwWidth = 800;  dwHeight = 600; dwBPP = 16; break;
  case DM_800x600x24:  dwWidth = 800;  dwHeight = 600; dwBPP = 24; break;
  case DM_1024x768x8:  dwWidth = 1024; dwHeight = 768; dwBPP = 8;  break;
  case DM_1024x768x16: dwWidth = 1024; dwHeight = 768; dwBPP = 16; break;
  case DM_1024x768x24: dwWidth = 1024; dwHeight = 768; dwBPP = 24; break;
  default: dwWidth = 640; dwHeight = 480; dwBPP = 24; break;
  }
  
  if(dwBPP == 32) m_b32BitMode = TRUE;
//  if(dwBPP == 32) dwBPP = 24;
  // if DM_NoChange set, left the display mode as it is
  if(eDisplayMode != DM_NoChange){
    // Set wanted display mode
    m_hResult = m_lpDD->SetDisplayMode(dwWidth, dwHeight, dwBPP);
    if(m_hResult != DD_OK){
      // if it was a 32bit mode try to set the 24bit mode
      // some cards do not support the 32bit mode
      if(dwBPP == 24){
        dwBPP = 32;
        m_b32BitMode = TRUE;
        m_hResult = m_lpDD->SetDisplayMode(dwWidth, dwHeight, dwBPP);
      }
      DIRECTDRAW_ERROR(m_hResult);
    }
  }

  if(eDisplayMode == DM_NoChange){
    DDSURFACEDESC ddsd;

    ddsd.dwSize = sizeof(ddsd);
    m_hResult = m_lpDD->GetDisplayMode(&ddsd);
    DIRECTDRAW_ERROR(m_hResult);

    dwBPP = ddsd.ddpfPixelFormat.dwRGBBitCount;
    if(dwBPP == 32) m_b32BitMode = TRUE;
    else m_b32BitMode = FALSE;
    dwWidth = ddsd.dwWidth;
    dwHeight = ddsd.dwHeight;
  }
  else{
    TRACE("New display mode set. %dx%dx%d\n", dwWidth, dwHeight, dwBPP);
  }

  // Because there can be only one object DirectDraw ( more doesn't make sense)
  // we can set a global pointer to it
  ASSERT(g_pDirectDraw == NULL);  // if it's not null it's an error
  g_pDirectDraw = this;

  // connect us to the abort notification
  // on abort we will destroy this object -> close DirectDraw
  g_AbortNotifier.Connect(this);

  return TRUE;
}

void CDirectDraw::Delete()
{
  // we are destroying ourself -> no future abort needed
  g_AbortNotifier.Disconnect(this);
  
  // as in Create we can assume only one instance of CDirectDraw
  // so we can null a global pointer
  g_pDirectDraw = NULL;
  
  while(m_pSurfaces != NULL){  // Delete all surfaces
    m_pSurfaces->m_bRemoveFromList = FALSE;
    m_pSurfaces->Delete();
    m_pSurfaces = m_pSurfaces->m_pNextSurface;
  }
  m_pSurfaces = NULL;

  while(m_pClippers != NULL){
    m_pClippers->m_bRemoveFromList = FALSE;
    m_pClippers->Delete();
    m_pClippers = m_pClippers->m_pNextClipper;
  }
  m_pClippers = NULL;

  if(m_lpDD){
    ASSERT_VALID(this);
    // return cooperative level to normal
    // for this mode we don't need to know the window
    m_lpDD->SetCooperativeLevel(NULL, DDSCL_NORMAL);
    m_lpDD->Release();
    m_lpDD = 0;
  }
  m_b32BitMode = FALSE;
}

// Creates DirectDraw surface
// the specification of the surface is givven by virtual
// function PreCreate
// if the second parametr is givven the PreCreate is ignored
// !!!! be carful some Surfaces require the call of PreCreate
BOOL CDirectDraw::CreateSurface(CDDrawSurface * pDDSurface, DDSURFACEDESC * pddsd)
{
  LPDIRECTDRAWSURFACE lpDDSurface;
  DDSURFACEDESC lddsd;

  ASSERT(pDDSurface != NULL);

  lddsd.dwSize = sizeof(lddsd);

  // Call the Precreate to determine the DDSurface description
  if(pddsd == NULL){
    if(!pDDSurface->PreCreate(&lddsd)) return FALSE;
    pddsd = &lddsd;
  }

  // Create the DirectDraw Surface object
  m_hResult = m_lpDD->CreateSurface(pddsd, &lpDDSurface, NULL);
  DIRECTDRAW_ERROR(m_hResult);

  // Call the virtual post create
  if(!pDDSurface->PostCreate(lpDDSurface)) return FALSE;

  // Add the surface to the list of all surfaces
  pDDSurface->m_pDirectDraw = this;
  pDDSurface->m_pNextSurface = m_pSurfaces;
  m_pSurfaces = pDDSurface;

  pDDSurface->m_bRemoveFromList = TRUE;

  return TRUE;
}

// Returns save pointer to DirectDraw interface
// You must call Release() on it when you finish using it
LPDIRECTDRAW CDirectDraw::GetLP()
{
  m_lpDD->AddRef();
  return m_lpDD;
}

/////////////////////////////////////////////////////////////////////////////
// Debug methods

#ifdef _DEBUG

void CDirectDraw::AssertValid() const
{
  CObserver::AssertValid();
  ASSERT_POINTER(m_lpDD, 1);
}

void CDirectDraw::Dump(CDumpContext & dc) const
{
  CObserver::Dump(dc);
  dc << "LPDIRECTDRAW : " << (void *)m_lpDD << "\n";
}

#endif

// removes surface from list of surfaces
void CDirectDraw::DeleteSurface(CDDrawSurface * pDDSurface)
{
  CDDrawSurface *pCurrent;

  if(m_pSurfaces == pDDSurface){
    m_pSurfaces = pDDSurface->m_pNextSurface;
    return;
  }

  pCurrent = m_pSurfaces;
  while(pCurrent->m_pNextSurface != pDDSurface){
    if(pCurrent->m_pNextSurface == NULL){
      ASSERT(FALSE);  // Given surface was not created by us
      return; // We failed to find it
    }
    pCurrent = pCurrent->m_pNextSurface;
  }
  // remove it from list
  pCurrent->m_pNextSurface = pDDSurface->m_pNextSurface;
}

BOOL CDirectDraw::Init(CWnd *pWnd, EDisplayModes eDisplayMode, DWORD dwFlags)
{
  CDirectDraw *pDirectDraw;

  ASSERT(g_pDirectDraw == NULL);

  if(g_pDirectDraw != NULL) return FALSE;

  pDirectDraw = new CDirectDraw();
  if(!pDirectDraw->Create(pWnd, eDisplayMode, dwFlags)) return FALSE;

  return TRUE;
}

void CDirectDraw::Close()
{
  CDirectDraw *pDirectDraw;

  if(g_pDirectDraw == NULL) return;

  pDirectDraw = g_pDirectDraw;
  pDirectDraw->Delete();
  delete pDirectDraw;
}

BOOL CDirectDraw::CreateClipper(CDDrawClipper *pDDClipper)
{
  LPDIRECTDRAWCLIPPER lpDDClipper;

  ASSERT(pDDClipper != NULL);

  // Create the DirectDraw Clipper object
  m_hResult = m_lpDD->CreateClipper(0, &lpDDClipper, NULL);
  DIRECTDRAW_ERROR(m_hResult);

  pDDClipper->m_lpDDClipper = lpDDClipper;
  
  // Add the clipper to the list of all clippers
  pDDClipper->m_pDirectDraw = this;
  pDDClipper->m_pNextClipper = m_pClippers;
  m_pClippers = pDDClipper;

  pDDClipper->m_bRemoveFromList = TRUE;

  return TRUE;
}

void CDirectDraw::DeleteClipper(CDDrawClipper *pDDClipper)
{
  CDDrawClipper *pCurrent;

  if(m_pClippers == pDDClipper){
    m_pClippers = pDDClipper->m_pNextClipper;
    return;
  }

  pCurrent = m_pClippers;
  while(pCurrent->m_pNextClipper != pDDClipper){
    if(pCurrent->m_pNextClipper == NULL){
      ASSERT(FALSE);  // Given clipper was not created by us
      return; // We failed to find it
    }
    pCurrent = pCurrent->m_pNextClipper;
  }
  // remove it from list
  pCurrent->m_pNextClipper = pDDClipper->m_pNextClipper;
}

BOOL CDirectDraw::IsValid()
{
  if(m_lpDD == NULL) return FALSE;
  return TRUE;
}


// On abort we will destroy our object
void CDirectDraw::OnAbort(DWORD dwExitCode)
{
  Delete();
}

BOOL CDirectDraw::ExclusiveMode()
{
  return m_bExclusiveMode;
}
