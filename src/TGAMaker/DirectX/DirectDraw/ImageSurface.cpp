// ImageSurface.cpp : implementation file
//

#include "stdafx.h"
#include "ImageSurface.h"
#include "DirectDrawException.h"
#include "TIFFReader.h"
#include "TGAReader.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CImageSurface

IMPLEMENT_DYNAMIC(CImageSurface, CDDrawSurface)

CImageSurface::CImageSurface()
{
  m_dwWidth = 1; m_dwHeight = 1;
  m_eFileType = FileType_TIFF;
  m_pImageReader = NULL;
//  m_dwKeyColor = 0x00FF00FF; // Violet is default key color
  m_dwKeyColor = m_dwNoKeyColor;
}

CImageSurface::~CImageSurface()
{
  if(m_pImageReader){
    ASSERT_VALID(m_pImageReader);
    delete m_pImageReader;
  }
  m_pImageReader = NULL;
}


/////////////////////////////////////////////////////////////////////////////
// CImageSurface message handlers

BOOL CImageSurface::PreCreate(DDSURFACEDESC * ddsd)
{
  ddsd->dwFlags = DDSD_CAPS | DDSD_WIDTH | DDSD_HEIGHT | DDSD_PIXELFORMAT;
  ddsd->ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN;

  if(!StartImageReader()) return FALSE;

  ddsd->dwWidth = m_pImageReader->m_dwWidth;
  ddsd->dwHeight = m_pImageReader->m_dwHeight;
  ddsd->ddpfPixelFormat.dwSize = sizeof(ddsd->ddpfPixelFormat);
  ddsd->ddpfPixelFormat.dwFlags = DDPF_RGB;
  if(g_pDirectDraw->Is32BitMode())
    ddsd->ddpfPixelFormat.dwRGBBitCount = 32;
  else
    ddsd->ddpfPixelFormat.dwRGBBitCount = 24;
  ddsd->ddpfPixelFormat.dwRBitMask = 0x00FF0000;
  ddsd->ddpfPixelFormat.dwGBitMask = 0x0000FF00;
  ddsd->ddpfPixelFormat.dwBBitMask = 0x000000FF;
//  ddsd->ddpfPixelFormat.dwRGBAlphaBitMask = 0x000000;

  // if the image has alpha channel -> create it
  m_bHasAlphaChannel = m_pImageReader->m_bAlphaChannel;

  // all was OK
  return TRUE;
}

BOOL CImageSurface::PostCreate(LPDIRECTDRAWSURFACE lpDDSurface, BOOL bCallRelease)
{
  if(!CDDrawSurface::PostCreate(lpDDSurface, bCallRelease)) return FALSE;
  
  if(!FinishImageReader()) return FALSE;

  SetColorKey(m_dwKeyColor);
  // if the image doesn't contain pixel with the key color
  // we won't use the key color -> it's slower
  // and also we would think this surface is transparent
  if(!m_bHasAlphaChannel && !HasKeyColor())
    SetColorKey(m_dwNoKeyColor);

  return TRUE;
}

void CImageSurface::SetFile(CArchiveFile *pFile, EFileTypes eFileType)
{
  CString strFileName;

  m_File = *pFile;
  strFileName = pFile->GetFileName();

  // if the surface is not created we can change the value
  if(m_lpDDSurface == NULL){
    m_eFileType = eFileType;

    if(m_eFileType == FileType_Auto){
      char *hlp;
      hlp = strrchr((LPCSTR)strFileName, '.');
      if(hlp != NULL){
        hlp++;
        if(!strcmpi(hlp, "tif")) m_eFileType = FileType_TIFF;
        if(!strcmpi(hlp, "tiff")) m_eFileType = FileType_TIFF;
        if(!strcmpi(hlp, "tga")) m_eFileType = FileType_Targa;
      }
    }
  }
}

void CImageSurface::SetImageKeyColor(DWORD dwKeyColor)
{
  // if the surface is not created we can change the value
  if(m_lpDDSurface == NULL){
    m_dwKeyColor = dwKeyColor;
  }
}

/////////////////////////////////////////////////////////////////////////////
// Debug methods

#ifdef _DEBUG

void CImageSurface::AssertValid() const
{
  CDDrawSurface::AssertValid();
}

void CImageSurface::Dump(CDumpContext & dc) const
{
  CDDrawSurface::Dump(dc);
}

#endif

BOOL CImageSurface::HasKeyColor()
{
  DWORD x,y, color;
  DWORD dwPitch, dwPos;
  BYTE *pSurface;
  BYTE r, g, b;

  color = GetColorKey();
  if(color == m_dwNoKeyColor) return FALSE;

  r = R32(color); g = G32(color); b = B32(color);

  pSurface = (BYTE *)Lock(GetAllRect(), 0, dwPitch);
  for(y = 0; y < m_dwHeight; y++){
    dwPos = y * dwPitch;
    for(x = 0; x < m_dwWidth; x++){
      if((pSurface[dwPos] == b) && (pSurface[dwPos + 1] == g) &&
        (pSurface[dwPos + 2] == r)) return TRUE;
    }
  }

  return FALSE;
}

BOOL CImageSurface::Create(CArchiveFile *pFile, EFileTypes eFileType)
{
  SetFile(pFile, eFileType);

  return CDDrawSurface::Create();
}

void CImageSurface::Restore()
{
  CDDrawSurface::Restore();

  if(!StartImageReader()) return;
  if(!FinishImageReader()) return;
}

BOOL CImageSurface::StartImageReader()
{
  if(m_pImageReader){
    ASSERT_VALID(m_pImageReader);
    delete m_pImageReader;
  }
  m_pImageReader = NULL;

  // Choose the right file reader
  switch(m_eFileType){
  case FileType_TIFF:
    m_pImageReader = new CTIFFReader();
    break;
  case FileType_Targa:
    m_pImageReader = new CTGAReader();
    break;
  }

  // we didn't find the reader for given file
  if(m_pImageReader == NULL){
    TRACE("Unknown format of the image '%s'.\n", m_File.GetFileName());
    return FALSE;
  }

  // Open the file
  m_pImageReader->Open(&m_File);
  // Read the informations -> width and height
  if(!m_pImageReader->ReadInformations()) return FALSE;

  return TRUE;
}

BOOL CImageSurface::FinishImageReader()
{
  // if the reader was not created this call of Create is irelevant
  // no call of PreCreate was before
  if(!m_pImageReader){
    ASSERT(FALSE); // It's a fatal error
    return FALSE;
  }
  ASSERT_VALID(m_pImageReader);

  // Read the image to this surface
  if(!m_pImageReader->ReadImage(this)) return FALSE;
  // Close the reader and so the file
  m_pImageReader->Close();
  // delete the reader
  delete m_pImageReader; m_pImageReader = NULL;

  return TRUE;
}
