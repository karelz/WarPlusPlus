// EResource.cpp: implementation of the CEResource class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "EResource.h"

#include "..\MapFormats.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

IMPLEMENT_DYNAMIC(CEResource, CObject);

CEResource::CEResource()
{
  m_dwIndex = 0;
}

CEResource::~CEResource()
{
}

#ifdef _DEBUG

void CEResource::AssertValid() const
{
  CObject::AssertValid();
}

void CEResource::Dump(CDumpContext &dc) const
{
  CObject::Dump(dc);
}

#endif

void CEResource::Create(DWORD dwIndex, CString strName)
{
  // Copy the name
  m_strName = strName;
  m_dwIndex = dwIndex;

  // Create new empty scratch surface
  m_Icon.SetWidth(RESOURCE_ICON_WIDTH);
  m_Icon.SetHeight(RESOURCE_ICON_HEIGHT);
  m_Icon.SetAlphaChannel(TRUE);
  m_Icon.Create(); // This will create black rectangle
}

void CEResource::Create(DWORD dwIndex, CArchiveFile file)
{
  m_dwIndex = dwIndex;

  if(g_dwMapFileVersion < MAP_FILE_VERSION(1, 0)){
    SResource_Old h;
    memset(&h, 0, sizeof(h));
    
    // Read the header
    file.Read(&h, sizeof(h));
    
    // copy the name
    m_strName = h.m_szName;
    
    // read the icon image
    {
      BYTE pData[RESOURCE_ICON_WIDTH * RESOURCE_ICON_HEIGHT * 4];
      memset(pData, 0, RESOURCE_ICON_WIDTH * RESOURCE_ICON_HEIGHT * 4);
      
      file.Read(pData, RESOURCE_ICON_WIDTH * RESOURCE_ICON_HEIGHT * 4);
      
      // create the surface
      m_Icon.SetWidth(RESOURCE_ICON_WIDTH);
      m_Icon.SetHeight(RESOURCE_ICON_HEIGHT);
      m_Icon.SetAlphaChannel(TRUE);
      m_Icon.Create();
      
      // copy the RGB information
      DWORD dwPitch;
      LPBYTE pDest;
      pDest = (LPBYTE)m_Icon.Lock(m_Icon.GetAllRect(), 0, dwPitch);
      DWORD x, y, dwSOff, dwDOff;
      if(g_pDirectDraw->Is32BitMode()){
        for(y = 0; y < RESOURCE_ICON_HEIGHT; y++){
          for(x = 0; x < RESOURCE_ICON_WIDTH; x++){
            dwSOff = (y * RESOURCE_ICON_WIDTH + x) * 4;
            dwDOff = y * dwPitch + x * 4;
            pDest[dwDOff] = pData[dwSOff];
            pDest[dwDOff + 1] = pData[dwSOff + 1];
            pDest[dwDOff + 2] = pData[dwSOff + 2];
          }
        }
      }
      else{
        for(y = 0; y < RESOURCE_ICON_HEIGHT; y++){
          for(x = 0; x < RESOURCE_ICON_WIDTH; x++){
            dwSOff = (y * RESOURCE_ICON_WIDTH + x) * 4;
            dwDOff = y * dwPitch + x * 3;
            pDest[dwDOff] = pData[dwSOff];
            pDest[dwDOff + 1] = pData[dwSOff + 1];
            pDest[dwDOff + 2] = pData[dwSOff + 2];
          }
        }
      }
      m_Icon.Unlock(pDest);
      
      // copy the alpha channel
      LPBYTE pAlpha = m_Icon.GetAlphaChannel(FALSE);
      for(y = 0; y < RESOURCE_ICON_HEIGHT; y++){
        for(x = 0; x < RESOURCE_ICON_WIDTH; x++){
          pAlpha[y * RESOURCE_ICON_WIDTH + x] = pData[(y * RESOURCE_ICON_WIDTH + x) * 4 + 3];
        }
      }
      m_Icon.ReleaseAlphaChannel(TRUE);
    }
  }
  else{
    SResource h;
    memset(&h, 0, sizeof(h));
    
    // Read the header
    file.Read(&h, sizeof(h));
    
    // copy the name
    m_strName = h.m_szName;
    
    // read the icon image
    {
      BYTE pData[RESOURCE_ICON_WIDTH * RESOURCE_ICON_HEIGHT * 4];
      memset(pData, 0, RESOURCE_ICON_WIDTH * RESOURCE_ICON_HEIGHT * 4);
      
      file.Read(pData, RESOURCE_ICON_WIDTH * RESOURCE_ICON_HEIGHT * 4);
      
      // create the surface
      m_Icon.SetWidth(RESOURCE_ICON_WIDTH);
      m_Icon.SetHeight(RESOURCE_ICON_HEIGHT);
      m_Icon.SetAlphaChannel(TRUE);
      m_Icon.Create();
      
      // copy the RGB information
      DWORD dwPitch;
      LPBYTE pDest;
      pDest = (LPBYTE)m_Icon.Lock(m_Icon.GetAllRect(), 0, dwPitch);
      DWORD x, y, dwSOff, dwDOff;
      if(g_pDirectDraw->Is32BitMode()){
        for(y = 0; y < RESOURCE_ICON_HEIGHT; y++){
          for(x = 0; x < RESOURCE_ICON_WIDTH; x++){
            dwSOff = (y * RESOURCE_ICON_WIDTH + x) * 4;
            dwDOff = y * dwPitch + x * 4;
            pDest[dwDOff] = pData[dwSOff];
            pDest[dwDOff + 1] = pData[dwSOff + 1];
            pDest[dwDOff + 2] = pData[dwSOff + 2];
          }
        }
      }
      else{
        for(y = 0; y < RESOURCE_ICON_HEIGHT; y++){
          for(x = 0; x < RESOURCE_ICON_WIDTH; x++){
            dwSOff = (y * RESOURCE_ICON_WIDTH + x) * 4;
            dwDOff = y * dwPitch + x * 3;
            pDest[dwDOff] = pData[dwSOff];
            pDest[dwDOff + 1] = pData[dwSOff + 1];
            pDest[dwDOff + 2] = pData[dwSOff + 2];
          }
        }
      }
      m_Icon.Unlock(pDest);

      // copy the alpha channel
      LPBYTE pAlpha = m_Icon.GetAlphaChannel(FALSE);
      for(y = 0; y < RESOURCE_ICON_HEIGHT; y++){
        for(x = 0; x < RESOURCE_ICON_WIDTH; x++){
          pAlpha[y * RESOURCE_ICON_WIDTH + x] = pData[(y * RESOURCE_ICON_WIDTH + x) * 4 + 3];
        }
      }
      m_Icon.ReleaseAlphaChannel(TRUE);
    }
  }
}

void CEResource::Save(CArchiveFile file)
{
  SResource h;

  memset(&h, 0, sizeof(h));

  // copy the name
  strncpy(h.m_szName, m_strName, 30);

  // write the header
  file.Write(&h, sizeof(h));

  // write the icon
  {
    // create the icon buffer
    BYTE pData[RESOURCE_ICON_WIDTH * RESOURCE_ICON_HEIGHT * 4];

    // first copy the RGB
    DWORD dwPitch;
    LPBYTE pDest;
    pDest = (LPBYTE)m_Icon.Lock(m_Icon.GetAllRect(), 0, dwPitch);
    DWORD x, y, dwSOff, dwDOff;
    if(g_pDirectDraw->Is32BitMode()){
      for(y = 0; y < RESOURCE_ICON_HEIGHT; y++){
        for(x = 0; x < RESOURCE_ICON_WIDTH; x++){
          dwDOff = (y * RESOURCE_ICON_WIDTH + x) * 4;
          dwSOff = y * dwPitch + x * 4;
          pData[dwDOff] = pDest[dwSOff];
          pData[dwDOff + 1] = pDest[dwSOff + 1];
          pData[dwDOff + 2] = pDest[dwSOff + 2];
        }
      }
    }
    else{
      for(y = 0; y < RESOURCE_ICON_HEIGHT; y++){
        for(x = 0; x < RESOURCE_ICON_WIDTH; x++){
          dwDOff = (y * RESOURCE_ICON_WIDTH + x) * 4;
          dwSOff = y * dwPitch + x * 3;
          pData[dwDOff] = pDest[dwSOff];
          pData[dwDOff + 1] = pDest[dwSOff + 1];
          pData[dwDOff + 2] = pDest[dwSOff + 2];
        }
      }
    }
    m_Icon.Unlock(pDest);

    // copy the alpha channel
    LPBYTE pAlpha = m_Icon.GetAlphaChannel(TRUE);
    for(y = 0; y < RESOURCE_ICON_HEIGHT; y++){
      for(x = 0; x < RESOURCE_ICON_WIDTH; x++){
        pData[(y * RESOURCE_ICON_WIDTH + x) * 4 + 3] = pAlpha[y * RESOURCE_ICON_WIDTH + x];
      }
    }
    m_Icon.ReleaseAlphaChannel(TRUE);

    // Write the image to the file
    file.Write(pData, RESOURCE_ICON_WIDTH * RESOURCE_ICON_HEIGHT * 4);
  }
}

void CEResource::Delete()
{
  m_Icon.Delete();
}

void CEResource::SetIcon(CDDrawSurface *pIcon)
{
  m_Icon.ClearAlphaChannel();

  m_Icon.Fill(0);
  CRect rc(0, 0, RESOURCE_ICON_WIDTH, RESOURCE_ICON_HEIGHT);
  m_Icon.Paste(0, 0, pIcon, &rc);

  DWORD dwSourceWidth, dwCopyWidth;
  dwSourceWidth = pIcon->GetAllRect()->Width();
  if(dwSourceWidth < RESOURCE_ICON_WIDTH) dwCopyWidth = dwSourceWidth;
  else dwCopyWidth = RESOURCE_ICON_WIDTH;

  // copy the alpha channel
  LPBYTE pDest = m_Icon.GetAlphaChannel(FALSE);
  if(!pIcon->HasAlphaChannel()){
    memset(pDest, 255, RESOURCE_ICON_WIDTH * RESOURCE_ICON_HEIGHT);
    m_Icon.ReleaseAlphaChannel(TRUE);
    return;
  }

  LPBYTE pSource = pIcon->GetAlphaChannel(TRUE);

  // clear the dest alpha channel
  memset(pDest, 0, RESOURCE_ICON_WIDTH * RESOURCE_ICON_HEIGHT);

  DWORD y;
  for(y = 0; y < RESOURCE_ICON_HEIGHT; y++){
    memcpy(&pDest[y * RESOURCE_ICON_WIDTH], &pSource[y * dwSourceWidth], dwCopyWidth);
  }

  pIcon->ReleaseAlphaChannel(TRUE);
  m_Icon.ReleaseAlphaChannel(TRUE);
}

BOOL CEResource::CheckValid()
{
  return TRUE;
}