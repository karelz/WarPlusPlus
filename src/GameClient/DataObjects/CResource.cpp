// CResource.cpp: implementation of the CCResource class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "CResource.h"

#include "Common\Map\Map.h"
#include "..\LoadException.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

IMPLEMENT_DYNAMIC(CCResource, CObject);

// Constructor
CCResource::CCResource()
{
}

// Destructor
CCResource::~CCResource()
{
  ASSERT(m_strName.IsEmpty());
}

// Debug functions
#ifdef _DEBUG

void CCResource::AssertValid() const
{
  CObject::AssertValid();

  ASSERT(m_Icon.IsValid());
}

void CCResource::Dump(CDumpContext &dc) const
{
  CObject::Dump(dc);
}

#endif

// Creates the object from the file (loads it)
void CCResource::Create(CArchiveFile MapFile)
{
  SResource h;

  memset(&h, 0, sizeof(h));

  LOAD_ASSERT ( MapFile.Read ( &h, sizeof ( h ) ) == sizeof ( h ) );

  // copy the name
  m_strName = h.m_szName;

  // read the icon image
  {
    BYTE pData[RESOURCE_ICON_WIDTH * RESOURCE_ICON_HEIGHT * 4];
    memset(pData, 0, RESOURCE_ICON_WIDTH * RESOURCE_ICON_HEIGHT * 4);

    LOAD_ASSERT ( MapFile.Read ( pData, RESOURCE_ICON_WIDTH * RESOURCE_ICON_HEIGHT * 4 ) ==
      RESOURCE_ICON_WIDTH * RESOURCE_ICON_HEIGHT * 4 );

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

// Deletes the object
void CCResource::Delete()
{
  m_strName.Empty();
  m_Icon.Delete();
}