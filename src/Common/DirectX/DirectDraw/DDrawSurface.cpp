// DDrawSurface.cpp : implementation file
//

#include "stdafx.h"
#include "DDrawSurface.h"
#include "DDrawClipper.h"
#include "DirectDrawException.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define DIRECTDRAW_ERROR_UNLOCK(hResult, lock) \
  try{ \
    DIRECTDRAW_ERROR(hResult); \
  } \
  catch(CException *) \
  { \
    VERIFY(lock.Unlock()); \
    throw; \
  } \
  VERIFY(lock.Unlock());

/////////////////////////////////////////////////////////////////////////////
// CDDrawSurface

IMPLEMENT_DYNAMIC(CDDrawSurface, CObject)

// Initialize the constant for no key color with unusable color
const DWORD CDDrawSurface::m_dwNoKeyColor = 0xFFFFFFFF;

CDDrawSurface::CDDrawSurface()
{
  m_lpDDSurface = NULL;
  m_pNextSurface = NULL;
  m_bCallRelease = FALSE;
  m_bUseKeyColor = FALSE;
  m_pDirectDraw = NULL;
  m_bUseBltFast = FALSE;
  m_bRemoveFromList = FALSE;
  m_pointTransformation.x = 0;
  m_pointTransformation.y = 0;
  m_pAlphaChannel = NULL;
  m_bHasAlphaChannel = FALSE;
  m_bUserPixelFormat = FALSE;
#ifdef ALPHA_BLIT_QUICK
  m_pAlphaChannelBackup = NULL;
  m_bReadOnlyAlphaChannel = TRUE;
#endif
}

CDDrawSurface::~CDDrawSurface()
{
  Delete();
}

/////////////////////////////////////////////////////////////////////////////
// Debug methods

#ifdef _DEBUG

void CDDrawSurface::AssertValid() const
{
  CObject::AssertValid();
  ASSERT(m_lpDDSurface != NULL);
}

void CDDrawSurface::Dump(CDumpContext & dc) const
{
  CObject::Dump(dc);
  dc << "LPDDRAWSURFACE : " << (void *)m_lpDDSurface << "\n";
  dc << "Width : " << m_dwWidth << "\n";
  dc << "Height : " << m_dwHeight << "\n";
}

#endif

/////////////////////////////////////////////////////////////////////////////
// CDDrawSurface message handlers

BOOL CDDrawSurface::PostCreate(LPDIRECTDRAWSURFACE lpDDSurface, BOOL bCallRelease)
{
  ASSERT_POINTER(lpDDSurface, 1);
  
  m_lpDDSurface = lpDDSurface;
  m_bCallRelease = bCallRelease;

  // Get the size of this surface and store it
  DDSURFACEDESC ddsd;
  ddsd.dwSize = sizeof(ddsd);
  ddsd.dwFlags = DDSD_HEIGHT | DDSD_WIDTH;
  GetSurfaceDesc(&ddsd);
  m_dwWidth = ddsd.dwWidth;
  m_dwHeight = ddsd.dwHeight;
  m_rectAll.SetRect(0, 0, m_dwWidth, m_dwHeight);
  m_rectClip = m_rectAll;

  if(m_bHasAlphaChannel){
    m_pAlphaChannel = new BYTE[m_dwWidth * m_dwHeight];
    if(m_pAlphaChannel == NULL) return FALSE;
#ifdef ALPHA_BLIT_QUICK
    memset(m_pAlphaChannel, 0, m_dwWidth * m_dwHeight);
#else
    memset(m_pAlphaChannel, 255, m_dwWidth * m_dwHeight);
#endif
  }

  m_bRemoveFromList = FALSE;

  return TRUE;
}

void CDDrawSurface::Delete()
{
  if(m_lpDDSurface){
    ASSERT_POINTER(m_lpDDSurface, 1);

    if(m_bCallRelease) m_lpDDSurface->Release();
    m_bCallRelease = FALSE;
    m_lpDDSurface = NULL;

    if(m_pAlphaChannel != NULL){
      delete [] m_pAlphaChannel;
      m_pAlphaChannel = NULL;
    }
    m_bHasAlphaChannel = FALSE;

#ifdef ALPHA_BLIT_QUICK
    if(m_pAlphaChannelBackup != NULL){
      delete m_pAlphaChannelBackup;
      m_pAlphaChannelBackup = NULL;
    }
#endif
    
    if(m_bRemoveFromList){
      ASSERT(m_pDirectDraw);
      if(m_pDirectDraw) m_pDirectDraw->DeleteSurface(this);
    }
  }
}

// Returns attached surface
// !!!! Warning !!! the surface has to have only one attached surface
// if not, this method will fail
BOOL CDDrawSurface::GetAttachedSurface(CDDrawSurface & AttachedSurface,
                                       DDSCAPS & ddscaps)
{
  LPDIRECTDRAWSURFACE lpAttachedSurface;

  ASSERT_VALID(this);
  
  m_hResult = m_lpDDSurface->GetAttachedSurface(&ddscaps, &lpAttachedSurface);
  DIRECTDRAW_ERROR(m_hResult);

  // second param is FALSE because the attached surfaces are
  // destroyed automaticaly with their owners
  return AttachedSurface.PostCreate(lpAttachedSurface, FALSE);
}

// Returns safe pointer to DirectDrawSurface interface
// you must call Release() on it after finishing using it
LPDIRECTDRAWSURFACE CDDrawSurface::GetLP()
{
  ASSERT_VALID(this);

  m_lpDDSurface->AddRef();
  return m_lpDDSurface;
}

void CDDrawSurface::Restore()
{
  ASSERT_VALID(this);
  if(g_pDirectDraw->Disabled()) return;

  VERIFY(m_lockExclusive.Lock());

  // Just restore the surface
  // More complicated surfaces also has to reload the graphics
  // We'll loop here if the surface can't be restored, cause it can happen that
  // the mode is still not set (our mode), so wait for the change of it
  DWORD dwLoop = 20; // We'll wait for it for 2 seconds max
  while ( dwLoop > 0 )
  {
    m_hResult = m_lpDDSurface->Restore();
    if ( m_hResult == DDERR_WRONGMODE )
    {
      // OK sleep for a while
      Sleep ( 100 );
      dwLoop --;
      // And try it again
      continue;
    }
    // Some other error, or succes -> just break it
    break;
  }
  DIRECTDRAW_ERROR_UNLOCK(m_hResult, m_lockExclusive);
}

// returns the DC for this surface
// be carfule - don't use it for a long time
// this call locks the surface and a part of the system
// so you should release it as soon as possible
CDC *CDDrawSurface::GetDC()
{
  HDC hdc;
  CDC *dc;

  ASSERT_VALID(this);

  VERIFY(m_lockExclusive.Lock());
  m_hResult = m_lpDDSurface->GetDC(&hdc);
  if(m_hResult != DD_OK){
    VERIFY(m_lockExclusive.Unlock());
  }
  DIRECTDRAW_ERROR(m_hResult);

  dc = new CDC();
  dc->Attach(hdc);
  return dc;
}

// Releases the DC returned by the GetDC method
void CDDrawSurface::ReleaseDC(CDC *dc)
{
  HDC hdc;

  ASSERT_VALID(this);
  ASSERT_VALID(dc);
  ASSERT_KINDOF(CDC, dc);

  hdc = dc->Detach();
  m_hResult = m_lpDDSurface->ReleaseDC(hdc);
  DIRECTDRAW_ERROR_UNLOCK(m_hResult, m_lockExclusive);

  delete dc;
}

// Callback function
// This class don't have any information for the surface
// so you have to either inherit it or use the second parameter
// of method CreateSurface in CDirectDraw
BOOL CDDrawSurface::PreCreate(LPDDSURFACEDESC ddsd)
{
  TRACE("PreCreate Abstract\n");
  return FALSE;  // Call of this function is an error
}

// Blits the image form one surface to this one
void CDDrawSurface::Blt(CRect * pDestRect, CDDrawSurface * pSrcSurface,
                        CRect * pSrcRect, DWORD dwFlags, LPDDBLTFX lpDDBltFx)
{
  LPDIRECTDRAWSURFACE lpDDSrcSurface;
  DWORD flags;

  ASSERT_VALID(this);
#ifdef _DEBUG
  if(pSrcSurface != NULL){
    ASSERT_VALID(pSrcSurface);
    ASSERT_KINDOF(CDDrawSurface, pSrcSurface);
  }
#endif

  if(g_pDirectDraw->Disabled()) return;

  // all blits will wait until the blitter is free
  // we don't want any errors just for not enough time
  flags = dwFlags | DDBLT_WAIT;

  // if we have to use the key color
  // we also have to specify the DDBLT_KEYSRC flag
  if(pSrcSurface){
    if(pSrcSurface->m_bUseKeyColor) flags |= DDBLT_KEYSRC;
  }

  CRect rectDest(pDestRect);

  // Can't do any clipping -> don't know how to if streching is possible

  // if the surface is NULL use the NULL also for the interface
  lpDDSrcSurface = (pSrcSurface == NULL) ? NULL : pSrcSurface->m_lpDDSurface;

  VERIFY(m_lockExclusive.Lock());

  m_hResult = m_lpDDSurface->Blt(&rectDest, lpDDSrcSurface,
    pSrcRect, flags, lpDDBltFx);

  if(m_hResult == DDERR_SURFACELOST){  // if lost restore and try again
    // we have to restore both surfaces
    // don't know which was lost
    try{
      Restore();
      if(pSrcSurface)
        pSrcSurface->Restore();
    }
    catch(...){
      VERIFY(m_lockExclusive.Unlock());
      throw;
    }

    if(g_pDirectDraw->Disabled()){
      VERIFY(m_lockExclusive.Unlock());
      return;
    }

    m_hResult = m_lpDDSurface->Blt(&rectDest, lpDDSrcSurface,
      pSrcRect, flags, lpDDBltFx);
  }

  CorrectSurfaceAfterBlit(&rectDest);

  DIRECTDRAW_ERROR_UNLOCK(m_hResult, m_lockExclusive);
}

// Clips the regions for blitting
// Nothing could be blitted outside the destination rectangle
// returns FALSE if there is nothing to blit
BOOL CDDrawSurface::ClipBltFastRect(CRect * pRectDest, CRect * pRectSource, CDDrawSurface *pSrcSurface, LONG lX, LONG lY)
{
  // Clip the rects to blit only in the dest surface

  CRect rcSourceClip(pRectSource);
  pRectSource->IntersectRect(&rcSourceClip, pSrcSurface->GetAllRect());

  pRectDest->SetRect(lX, lY, lX + rcSourceClip.Width(), lY + rcSourceClip.Height());
  
  CRect rcDestClip(pRectDest);
  pRectDest->IntersectRect(&rcDestClip, &m_rectClip);

  pRectSource->OffsetRect(-rcSourceClip.left, -rcSourceClip.top);
  pRectDest->OffsetRect(-rcDestClip.left, -rcDestClip.top);

  pRectDest->IntersectRect(pRectSource, pRectDest);
  if(pRectDest->IsRectEmpty()) return FALSE;
  *pRectSource = pRectDest;

  pRectSource->OffsetRect(rcSourceClip.left, rcSourceClip.top);
  pRectDest->OffsetRect(rcDestClip.left, rcDestClip.top);

  // clip the source rectangle
/*  pRectSource->IntersectRect(pRectSource, pSrcSurface->GetAllRect());
  if(pRectSource->IsRectEmpty()) return FALSE; // if the source is empty -> return

  (*pRectDest) = (*pRectSource);
  pRectDest->OffsetRect(-pRectSource->left, -pRectSource->top);

  // clip the dest rect
  pRectDest->OffsetRect(lX, lY);  // rectDest contains the destination rect
  // determine the intersection with the clip region
  // if there's no cliping region this region is equal to m_rectAll
  pRectDest->IntersectRect(pRectDest, &m_rectClip); 
  if(pRectDest->IsRectEmpty()) return FALSE;  // if it's empty -> do nothing

  (*pRectSource) = (*pRectDest);
  pRectSource->OffsetRect(-lX, -lY); // move it back to source
  pRectSource->OffsetRect(lsourceX, lsourceY);
  // now pRectSource contains the new source rect to blit from
  // and pRectDest the new destination rect to blit to
  // it's clipped*/

  return TRUE;
}

extern DWORD dcount;

// Performs simplier blit than the blt so faster
void CDDrawSurface::BltFast(LONG lX, LONG lY,
                            CDDrawSurface * pSrcSurface,
                            CRect * pSrcRect, DWORD dwTrans)
{
  DWORD flags;
  CRect rectDest, rectSource;

  ASSERT_VALID(this);
  ASSERT_VALID(pSrcSurface);
  ASSERT_KINDOF(CDDrawSurface, pSrcSurface);

  if(g_pDirectDraw->Disabled()) return;

  // if the source rect is NULL we will get the whole surface
  if(pSrcRect == NULL) pSrcRect = pSrcSurface->GetAllRect();

  // Clip the rects to blit only in the dest surface
  rectSource = (*pSrcRect);
  if(!ClipBltFastRect(&rectDest, &rectSource, pSrcSurface,
    lX, lY)) return;

  if((pSrcSurface != NULL) && (!(dwTrans & 0x80000000))){
    if((pSrcSurface->m_bHasAlphaChannel) && (!pSrcSurface->m_bUserPixelFormat)){
      BltAlpha(rectDest.left, rectDest.top, pSrcSurface, &rectSource);
      return;
    }
  }
  dwTrans &= ~(0x80000000); // mask out our alpha blit flag

  if(!m_bUseBltFast){ // if we can't use the BltFast -> call normal Blt
    Blt(&rectDest, pSrcSurface, &rectSource, 0);
    return;
  }

  // we will wait for the blitter to finish its operations
  flags = dwTrans | DDBLTFAST_WAIT;

  // use the color keying when we have to
  if(pSrcSurface->m_bUseKeyColor) flags |= DDBLTFAST_SRCCOLORKEY;

  VERIFY(m_lockExclusive.Lock());

  m_hResult = m_lpDDSurface->BltFast(rectDest.left, rectDest.top, pSrcSurface->m_lpDDSurface,
    &rectSource, flags);

  if(m_hResult == DDERR_SURFACELOST){  // if lost restore and try again
    try{
      Restore();
      if(pSrcSurface)
        pSrcSurface->Restore();
    }
    catch(...){
      VERIFY(m_lockExclusive.Unlock());
      throw;
    }
    m_hResult = m_lpDDSurface->BltFast(rectDest.left, rectDest.top, pSrcSurface->m_lpDDSurface,
      &rectSource, dwTrans);
  }

  CorrectSurfaceAfterBlit(&rectDest);

  DIRECTDRAW_ERROR_UNLOCK(m_hResult, m_lockExclusive);
}

// fills given rectangle with given color
void CDDrawSurface::Fill(DWORD dwFillColor, CRect *pDestRect)
{
  DDBLTFX ddBltFx;

  ASSERT_VALID(this);
  if(g_pDirectDraw->Disabled()) return;
  
  // if the rect was NULL fill whole surface
  if(pDestRect == NULL) pDestRect = &m_rectAll;

  CRect rectDest(pDestRect);
  rectDest.OffsetRect(m_pointTransformation);

  rectDest.IntersectRect(&rectDest, &m_rectClip);
  if(rectDest.IsRectEmpty()) return;

  ddBltFx.dwSize = sizeof(ddBltFx);
  ddBltFx.dwFillColor = dwFillColor;
  // just call the blit for filling
  Blt(&rectDest, NULL, NULL, DDBLT_COLORFILL, &ddBltFx);
}

void CDDrawSurface::GetSurfaceDesc(LPDDSURFACEDESC lpddsd)
{
  ASSERT_VALID(this);
  ASSERT_POINTER(lpddsd, DDSURFACEDESC);
  
  VERIFY(m_lockExclusive.Lock());
  m_hResult = m_lpDDSurface->GetSurfaceDesc(lpddsd);
  DIRECTDRAW_ERROR_UNLOCK(m_hResult, m_lockExclusive);
}

void CDDrawSurface::SetColorKey(DWORD dwKeyColor)
{
  DDCOLORKEY ddColKey;

  ASSERT_VALID(this);

  // if it's the NoKeyColor forbid using of color keying
  if(dwKeyColor == m_dwNoKeyColor){
    m_bUseKeyColor = FALSE;
    return;
  }

  ddColKey.dwColorSpaceLowValue = dwKeyColor;
  ddColKey.dwColorSpaceHighValue = dwKeyColor;

  VERIFY(m_lockExclusive.Lock());
  m_hResult = m_lpDDSurface->SetColorKey(DDCKEY_SRCBLT, &ddColKey);
  DIRECTDRAW_ERROR_UNLOCK(m_hResult, m_lockExclusive);

  // enable the color keying
  m_bUseKeyColor = TRUE;
}

// Locks the surface and returns a pointer to its memory
// you have to use the Pitch returned by this function
// this value can change at any time the surface is not locked
// !!!! Beware !!!! locking of the surface locks some system resources
// so you should unlock it as soon as possible
LPVOID CDDrawSurface::Lock(CRect * pLockRect, DWORD dwFlags, DWORD & dwPitch)
{
  DDSURFACEDESC ddsd;

  ASSERT_VALID(this);
  ASSERT(pLockRect);

  VERIFY(m_lockExclusive.Lock());

Retry:;
  ddsd.dwSize = sizeof(ddsd);
  m_hResult = m_lpDDSurface->Lock(pLockRect, &ddsd, dwFlags | DDLOCK_WAIT,
    NULL);
  if(m_hResult == DDERR_SURFACELOST){
    try{
      Restore();
    }
    catch(...){
      VERIFY(m_lockExclusive.Unlock());
      throw;
    }
    goto Retry;
  }
  DIRECTDRAW_ERROR(m_hResult);
  dwPitch = ddsd.lPitch;

  return ddsd.lpSurface;
}

// Unlocks the surface
void CDDrawSurface::Unlock(void * lpSurface)
{
  ASSERT_VALID(this);
  ASSERT_POINTER(lpSurface, 1);
  
Retry:;
  m_hResult = m_lpDDSurface->Unlock(lpSurface);
  if(m_hResult == DDERR_SURFACELOST){
    try{
      Restore();
    }
    catch(...){
      VERIFY(m_lockExclusive.Unlock());
      throw;
    }
    goto Retry;
  }
  DIRECTDRAW_ERROR_UNLOCK(m_hResult, m_lockExclusive);
}

DWORD CDDrawSurface::GetWidth() const
{
  ASSERT_VALID(this);

  return m_dwWidth;
}

DWORD CDDrawSurface::GetHeight() const
{
  ASSERT_VALID(this);

  return m_dwHeight;
}

DWORD CDDrawSurface::GetColorKey()
{
  DDCOLORKEY ddColKey;

  ASSERT_VALID(this);

  VERIFY(m_lockExclusive.Lock());
  m_hResult = m_lpDDSurface->GetColorKey(DDCKEY_SRCBLT, &ddColKey);
  if(m_hResult == DDERR_SURFACELOST){
    try{
      Restore();
    }
    catch(...){
      VERIFY(m_lockExclusive.Unlock());
      throw;
    }
    m_hResult = m_lpDDSurface->GetColorKey(DDCKEY_SRCBLT, &ddColKey);
  }
  if(m_hResult == DDERR_NOCOLORKEY){
    VERIFY(m_lockExclusive.Unlock());
    return m_dwNoKeyColor;
  }
  DIRECTDRAW_ERROR_UNLOCK(m_hResult, m_lockExclusive);

  // if the color key was set by this object we can choose if
  // the low one or the high one
  // elseway just return the low one
  return ddColKey.dwColorSpaceLowValue;
}

// Pastes (blits) the givven surface (its rect) into this surface
// if the this is MousePrimarySurface it takes care about mouse flashing
void CDDrawSurface::Paste(LONG lX, LONG lY, CDDrawSurface * pSrcSurface, CRect * pRectSource)
{
  DoPaste(lX + m_pointTransformation.x,
    lY + m_pointTransformation.y, pSrcSurface, pRectSource);
  // just transform the dest coordinates
}

void CDDrawSurface::DoPaste(LONG lX, LONG lY, CDDrawSurface * pSrcSurface, CRect * pRectSource)
{
  BltFast(lX, lY, pSrcSurface, pRectSource);  
  // in this version just call the BltFast
}

CPoint CDDrawSurface::SetTransformation(CPoint pointNew)
{
  CPoint pointTemp;

//  if(!m_rectAll.PtInRect(pointNew)){
//    ASSERT(FALSE);
//    return m_pointTransformation;
//  }
  
  pointTemp = m_pointTransformation;
  m_pointTransformation = pointNew;
  return pointTemp;
}

CPoint CDDrawSurface::GetTransformation()
{
  return m_pointTransformation;
}

BYTE * CDDrawSurface::GetAlphaChannel(BOOL bReadOnly)
{
  ASSERT_VALID(this);
  ASSERT(m_bHasAlphaChannel);

  VERIFY(m_lockExclusive.Lock());

#ifdef ALPHA_BLIT_QUICK
  m_bReadOnlyAlphaChannel = bReadOnly;
  if(!m_bReadOnlyAlphaChannel){
    // must copy the alpha channel
    if(m_pAlphaChannelBackup == NULL){
      m_pAlphaChannelBackup = new BYTE[m_dwWidth * m_dwHeight];
    }
    memcpy(m_pAlphaChannelBackup, m_pAlphaChannel, m_dwWidth * m_dwHeight);
  }
  // must inverse the alpha channel
  DWORD dwOff, dwLen = m_dwWidth * m_dwHeight;
  for(dwOff = 0; dwOff < dwLen; dwOff++){
    m_pAlphaChannel[dwOff] = 255 - m_pAlphaChannel[dwOff];
  }
#endif

  return m_pAlphaChannel;
}

void CDDrawSurface::ReleaseAlphaChannel(BOOL bReleaseBuffers)
{
  ASSERT_VALID(this);

#ifdef ALPHA_BLIT_QUICK
  // must inverse the alpha channel
  DWORD dwOff, dwLen = m_dwWidth * m_dwHeight;
  for(dwOff = 0; dwOff < dwLen; dwOff++){
    m_pAlphaChannel[dwOff] = 255 - m_pAlphaChannel[dwOff];
  }
  if((!m_bReadOnlyAlphaChannel) && m_bHasAlphaChannel){
    ASSERT(m_pAlphaChannelBackup != NULL);

    // here we must recompute the image
    // So lock the surface
    LPBYTE pData;
    DWORD dwPitch;
    try{
      pData = (LPBYTE)Lock(GetAllRect(), 0, dwPitch);
    }
    catch(...){
      VERIFY(m_lockExclusive.Unlock());
      throw;
    }

    if(g_pDirectDraw->Is32BitMode()){
      // go through all pixels
      DWORD x, y;
      DWORD dwImageOff, dwAlphaOff;
      DWORD dwHlp1, dwHlp2;
      for(y = 0; y < m_dwHeight; y++){
        dwImageOff = y * dwPitch;
        dwAlphaOff = y * m_dwWidth;
        for(x = 0; x < m_dwWidth; x++, dwImageOff += 4, dwAlphaOff++){
          dwHlp1 = 255 - m_pAlphaChannel[dwAlphaOff];
          dwHlp2 = 255 - m_pAlphaChannelBackup[dwAlphaOff];
          if(dwHlp2 == 0) continue;
          
          pData[dwImageOff] = (BYTE)(((DWORD)pData[dwImageOff]) * dwHlp1 / dwHlp2);
          pData[dwImageOff + 1] = (BYTE)(((DWORD)pData[dwImageOff + 1]) * dwHlp1 / dwHlp2);
          pData[dwImageOff + 2] = (BYTE)(((DWORD)pData[dwImageOff + 2]) * dwHlp1 / dwHlp2);
        }
      }
    }
    else{
      // go through all pixels
      DWORD x, y;
      DWORD dwImageOff, dwAlphaOff;
      DWORD dwHlp1, dwHlp2;
      for(y = 0; y < m_dwHeight; y++){
        dwImageOff = y * dwPitch;
        dwAlphaOff = y * m_dwWidth;
        for(x = 0; x < m_dwWidth; x++, dwImageOff += 3, dwAlphaOff++){
          dwHlp1 = 255 - m_pAlphaChannel[dwAlphaOff];
          dwHlp2 = 255 - m_pAlphaChannelBackup[dwAlphaOff];
          if(dwHlp2 == 0) continue;
          
          pData[dwImageOff] = (BYTE)(((DWORD)pData[dwImageOff]) * dwHlp1 / dwHlp2);
          pData[dwImageOff + 1] = (BYTE)(((DWORD)pData[dwImageOff + 1]) * dwHlp1 / dwHlp2);
          pData[dwImageOff + 2] = (BYTE)(((DWORD)pData[dwImageOff + 2]) * dwHlp1 / dwHlp2);
        }
      }
    }
    
    try{
      Unlock(pData);
    }
    catch(...){
      VERIFY(m_lockExclusive.Unlock());
      throw;
    }

    // if we are to release buffers -> do it
    if(bReleaseBuffers){
      delete m_pAlphaChannelBackup;
      m_pAlphaChannelBackup = NULL;
    }
  }

#endif

  VERIFY(m_lockExclusive.Unlock());

  // elseway do nothing
}

void CDDrawSurface::ClearAlphaChannel()
{
  VERIFY(m_lockExclusive.Lock());

  // just set the alpha channel
#ifdef ALPHA_BLIT_QUICK
  memset(m_pAlphaChannel, 0, m_dwWidth * m_dwHeight);
#else
  memset(m_pAlphaChannel, 255, m_dwWidth * m_dwHeight);
#endif

  VERIFY(m_lockExclusive.Unlock());
}


void CDDrawSurface::BltAlpha(LONG lX, LONG lY, CDDrawSurface * pSrcSurface, CRect * pSrcRect)
{
  ASSERT_VALID(pSrcSurface);
  ASSERT(pSrcRect != NULL);
  ASSERT(pSrcSurface->m_bHasAlphaChannel);

  DWORD width, height;
  DWORD i;
  DWORD dwSPitch, dwDPitch, dwAPitch;
  DWORD dwSOff, dwDOff, dwAOff, dwSDif, dwDDif, dwADif;
  BYTE *pAlpha, *pSource, *pDest;
  CRect DestRect;

  width = pSrcRect->Width(); height = pSrcRect->Height();
  DestRect.SetRect(lX, lY, lX+width, lY+height);
  pAlpha = pSrcSurface->m_pAlphaChannel;
  dwAPitch = pSrcSurface->GetWidth();
  pSource = (LPBYTE)pSrcSurface->Lock(pSrcRect, 0, dwSPitch);
  pAlpha = pAlpha + (pSrcRect->left + (pSrcRect->top * dwAPitch));
  pDest = (LPBYTE)Lock(&DestRect, 0, dwDPitch);
  
  dwSOff = (DWORD)pSource;
  dwDOff = (DWORD)pDest;
  dwAOff = (DWORD)pAlpha;
  if(g_pDirectDraw->Is32BitMode()){
//    for(i = 0; i < height; i++){
    dwSDif = dwSPitch - (width * 4);
    dwDDif = dwDPitch - (width * 4);
    dwADif = dwAPitch - width;

    __asm{
      mov eax, height
      mov i, eax

      mov EDI, dwDOff;
      mov ESI, dwSOff;
      mov EDX, dwAOff;
      xor ecx, ecx;
      xor ebx, ebx;
    }

#ifndef ALPHA_BLIT_QUICK
BigLoop:;
      __asm{
        mov eax, width
        push EBP
        mov EBP, eax
      }
LoopIt:;
       __asm{
         
         xor eax, eax;
         mov bl, [EDI];

         mov cl, [EDX];

         mov al, [ESI];
         
         sub eax, ebx;
         imul eax, ecx;
//         imul cl;
//         sar eax, 8;
         add ah, bl;
         mov [EDI], ah;
         
         xor eax, eax;
         mov bl, [EDI+1];
         mov al, [ESI+1];
         
         sub eax, ebx;
         imul eax, ecx;
//         imul cl;
//         sar eax, 8;
         add ah, bl;
         mov [EDI+1], ah;
         
         xor eax, eax;
         mov bl, [EDI+2];
         mov al, [ESI+2];
         
         sub eax, ebx;
         imul eax, ecx;
//         imul cl;
//         sar eax, 8;
         add ah, bl;
         mov [EDI+2], ah;
         
         inc EDX;
         add ESI,4;   /// this is the 32bit mode
         add EDI,4;   /// so one pixel is 4 bytes
         
         dec ebp;
         jnz LoopIt;

         pop ebp
       }
    __asm{
      add edi, dwDDif
      add esi, dwSDif
      add edx, dwADif 
      dec i
      jnz BigLoop;
    }

#else

BigLoop:;
      __asm{
        mov eax, width
        push EBP
        mov EBP, eax
      }
LoopIt:;
       __asm{

         xor ebx, ebx
         mov cl, [EDX]  // load alpha
         mov al, [EDI + 2]  // load dest byte
         
         mul cl  // mul them
         mov bl, ah

         mov al, [EDI + 1]
         shl ebx, 8
         mul cl
         mov bl, ah

         mov al, [EDI]
         shl ebx, 8
         mul cl
         mov bl, ah

         add ebx, [ESI]
         mov [EDI], ebx

         inc EDX;
         add ESI,4;   /// this is the 32bit mode
         add EDI,4;   /// so one pixel is 4 bytes
         
         dec ebp;
         jnz LoopIt;

         pop ebp
       }
    __asm{
      add edi, dwDDif
      add esi, dwSDif
      add edx, dwADif 
      dec i
      jnz BigLoop;
    }
#endif

  }
  else{
    
    dwSDif = dwSPitch - (width * 3);
    dwDDif = dwDPitch - (width * 3);
    dwADif = dwAPitch - width;
      
    __asm{
      mov eax, height
      mov i, eax

      mov EDI, dwDOff;
      mov ESI, dwSOff;
      mov EDX, dwAOff;
      xor ecx, ecx;
      xor ebx, ebx;
    }

#ifndef ALPHA_BLIT_QUICK

BigLoop2:;
      __asm{
        mov eax, width;
        push ebp
        mov ebp, eax
      }
LoopIt2:;
       __asm{
         mov cl, [EDX];
         
         xor eax, eax;
         mov bl, [EDI];
         mov al, [ESI];
         
         sub eax, ebx;
         imul eax, ecx;
//         sar eax, 8;
         add ah, bl;
         mov [EDI], ah;
         
         xor eax, eax;
         mov bl, [EDI+1];
         mov al, [ESI+1];
         
         sub eax, ebx;
         imul eax, ecx;
//         sar eax, 8;
         add ah, bl;
         mov [EDI+1], ah;
         
         xor eax, eax;
         mov bl, [EDI+2];
         mov al, [ESI+2];
         
         sub eax, ebx;
         imul eax, ecx;
//         sar eax, 8;
         add ah, bl;
         mov [EDI+2], ah;
         
         inc EDX;
         add ESI, 3;  /// This is only 24bit mode
         add EDI, 3;  /// one pixel 3 bytes
         
         dec ebp;
         jnz LoopIt2;
         pop ebp
       }
       
       // this is it ^ in C
       /*  for(j = 0; j < width; j++){
       *pD = *pD + ((((*pS) - (*pD)) * (*pA)) >> 8); pD++; pS++;
       *pD = *pD + ((((*pS) - (*pD)) * (*pA)) >> 8); pD++; pS++;
       *pD = *pD + ((((*pS) - (*pD)) * (*pA)) >> 8); pD++; pS++;
       pA++;
    }*/
    __asm{
      add edi, dwDDif
      add esi, dwSDif
      add edx, dwADif 
      dec i
      jnz BigLoop2
    }

#else

BigLoop2:;
      __asm{
        mov eax, width;
        push ebp
        mov ebp, eax
      }
LoopIt2:;
       __asm{
         mov cl, [EDX]

         mov al, [EDI + 2]
         mov bl, [ESI + 2]
         mul cl
         add bl, ah
         mov [EDI + 2], bl

         mov al, [EDI + 1]
         mov bl, [ESI + 1]
         mul cl
         add bl, ah
         mov [EDI + 1], bl

         mov al, [EDI]
         mov bl, [ESI]
         mul cl
         add bl, ah
         mov [EDI], bl

         inc EDX;
         add ESI, 3;  /// This is only 24bit mode
         add EDI, 3;  /// one pixel 3 bytes
         
         dec ebp;
         jnz LoopIt2;
         pop ebp
       }
       
       // this is it ^ in C
       /*  for(j = 0; j < width; j++){
       *pD = *pD + ((((*pS) - (*pD)) * (*pA)) >> 8); pD++; pS++;
       *pD = *pD + ((((*pS) - (*pD)) * (*pA)) >> 8); pD++; pS++;
       *pD = *pD + ((((*pS) - (*pD)) * (*pA)) >> 8); pD++; pS++;
       pA++;
    }*/
    __asm{
      add edi, dwDDif
      add esi, dwSDif
      add edx, dwADif 
      dec i
      jnz BigLoop2
    }
#endif

  }

  Unlock(pDest);
  pSrcSurface->Unlock(pSource);
}

void CDDrawSurface::GetClipRect(CRect * pRect)
{
  *pRect = m_rectClip;
  pRect->OffsetRect(-m_pointTransformation.x, -m_pointTransformation.y);
}

void CDDrawSurface::SetClipRect(CRect * pRectNew, CRect * pRectOld)
{
  if(pRectOld != NULL){
    *pRectOld = m_rectClip;
    pRectOld->OffsetRect(-m_pointTransformation.x, -m_pointTransformation.y);
  }

  if(pRectNew == NULL)
    m_rectClip = m_rectAll;
  else{
    m_rectClip = *pRectNew;
    m_rectClip.OffsetRect(m_pointTransformation);
  }
  m_rectClip.IntersectRect(&m_rectClip, &m_rectAll);
}

BOOL CDDrawSurface::IsTransparent()
{
  if(m_bHasAlphaChannel) return TRUE;
  if(GetColorKey() != m_dwNoKeyColor) return TRUE;

  return FALSE;
}

BOOL CDDrawSurface::Create()
{
  ASSERT(g_pDirectDraw != NULL);
  ASSERT_VALID(g_pDirectDraw);

  if(!g_pDirectDraw->CreateSurface(this)) return FALSE;

  return TRUE;
}

void CDDrawSurface::SetClipper(CDDrawClipper *pDDClipper)
{
  if(pDDClipper == NULL){
    m_hResult = m_lpDDSurface->SetClipper(NULL);
    DIRECTDRAW_ERROR(m_hResult);

    return;
  }

  m_hResult = m_lpDDSurface->SetClipper(pDDClipper->m_lpDDClipper);
  DIRECTDRAW_ERROR(m_hResult);
}

BOOL CDDrawSurface::IsValid() const
{
  if(m_lpDDSurface == NULL) return FALSE;
  return TRUE;
}

void CDDrawSurface::BltStretch(CRect *pRectDest, CDDrawSurface *pSource, CRect *pRectSource)
{
  ASSERT_VALID(pSource);
  if(g_pDirectDraw->Disabled()) return;

  if(pRectDest == NULL) pRectDest = GetAllRect();
  if(pRectSource == NULL) pRectSource = pSource->GetAllRect();

  DWORD dwDOff, dwSOff, dwSOffC;
  DWORD dwDPitch, dwSPitch;
  LPBYTE pDest, pSrc;
  int x, y;
  double dbXKoef, dbYKoef;
  DWORD dwX, dwY;
  int nWidth, nHeight;

  dbXKoef = (double)pRectSource->Width() / (double)pRectDest->Width();
  dbYKoef = (double)pRectSource->Height() / (double)pRectDest->Height();

  pDest = (LPBYTE)Lock(pRectDest, 0, dwDPitch);
  pSrc = (LPBYTE)pSource->Lock(pRectSource, 0, dwSPitch);

  nWidth = pRectDest->Width();
  nHeight = pRectDest->Height();

  if(g_pDirectDraw->Is32BitMode()){
    for(y = 0; y < nHeight; y++){
      dwY = (DWORD)(y * dbYKoef);
      dwSOff = dwY * dwSPitch;
      dwDOff = y * dwDPitch;
      for(x = 0; x < nWidth; x++){
        dwX = (DWORD)(x * dbXKoef);
        dwSOffC = dwSOff + (dwX * 4);
        pDest[dwDOff] = pSrc[dwSOffC];
        pDest[dwDOff + 1] = pSrc[dwSOffC + 1];
        pDest[dwDOff + 2] = pSrc[dwSOffC + 2];
        dwDOff += 4;
      }
    }
  }
  else{
    for(y = 0; y < nHeight; y++){
      dwY = (DWORD)(y * dbYKoef);
      dwSOff = dwY * dwSPitch;
      dwDOff = y * dwDPitch;
      for(x = 0; x < nWidth; x++){
        dwX = (DWORD)(x * dbXKoef);
        dwSOffC = dwSOff + (dwX * 3);
        pDest[dwDOff] = pSrc[dwSOffC];
        pDest[dwDOff + 1] = pSrc[dwSOffC + 1];
        pDest[dwDOff + 2] = pSrc[dwSOffC + 2];
        dwDOff += 3;
      }
    }
  }

  pSource->Unlock(pSrc);
  Unlock(pDest);
}

BOOL CDDrawSurface::HasAlphaChannel()
{
  return m_bHasAlphaChannel;
}

void CDDrawSurface::CorrectSurfaceAfterBlit(CRect *pRect)
{
#ifdef ALPHA_BLIT_QUICK
  if(!m_bHasAlphaChannel) return;

  if(g_pDirectDraw->Is32BitMode()){
    DWORD dwPitch;
    LPBYTE pData = (LPBYTE)Lock(GetAllRect(), 0, dwPitch);
  
    DWORD x, y;
    DWORD dwImageOff, dwAlphaOff;
    DWORD dwHlp;
    for(y = (DWORD)pRect->top; y < (DWORD)pRect->bottom; y++){
      dwImageOff = y * dwPitch + pRect->left * 4;
      dwAlphaOff = y * m_dwWidth + pRect->left;
      for(x = (DWORD)pRect->left; x < (DWORD)pRect->right; x++, dwImageOff += 4, dwAlphaOff++){
        dwHlp = 255 - m_pAlphaChannel[dwAlphaOff];
        pData[dwImageOff] = (BYTE)((pData[dwImageOff] * dwHlp) >> 8);
        pData[dwImageOff + 1] = (BYTE)((pData[dwImageOff + 1] * dwHlp) >> 8);
        pData[dwImageOff + 2] = (BYTE)((pData[dwImageOff + 2] * dwHlp) >> 8);
      }
    }

    Unlock(pData);
  }
  else{
    DWORD dwPitch;
    LPBYTE pData = (LPBYTE)Lock(GetAllRect(), 0, dwPitch);
  
    DWORD x, y;
    DWORD dwImageOff, dwAlphaOff;
    DWORD dwHlp;
    for(y = (DWORD)pRect->top; y < (DWORD)pRect->bottom; y++){
      dwImageOff = y * dwPitch + pRect->left * 3;
      dwAlphaOff = y * m_dwWidth + pRect->left;
      for(x = (DWORD)pRect->left; x < (DWORD)pRect->right; x++, dwImageOff += 3, dwAlphaOff++){
        dwHlp = 255 - m_pAlphaChannel[dwAlphaOff];
        pData[dwImageOff] = (BYTE)((pData[dwImageOff] * dwHlp) >> 8);
        pData[dwImageOff + 1] = (BYTE)((pData[dwImageOff + 1] * dwHlp) >> 8);
        pData[dwImageOff + 2] = (BYTE)((pData[dwImageOff + 2] * dwHlp) >> 8);
      }
    }

    Unlock(pData);
  }
#endif
}