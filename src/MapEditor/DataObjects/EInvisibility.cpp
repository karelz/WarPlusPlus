// EInvisibility.cpp: implementation of the CEVisibility class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "EInvisibility.h"

#include "..\MapFormats.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

IMPLEMENT_DYNAMIC(CEInvisibility, CObject);

CEInvisibility::CEInvisibility()
{
  m_dwIndex = 0;
}

CEInvisibility::~CEInvisibility()
{
}

#ifdef _DEBUG

void CEInvisibility::AssertValid() const
{
  CObject::AssertValid();
}

void CEInvisibility::Dump(CDumpContext &dc) const
{
  CObject::Dump(dc);
}

#endif

void CEInvisibility::Create(DWORD dwIndex)
{
  m_dwIndex = dwIndex;
  m_strName.Format("Viditelnost %d", dwIndex + 1);
}

void CEInvisibility::Create(DWORD dwIndex, CArchiveFile file)
{
  m_dwIndex = dwIndex;

  if(g_dwMapFileVersion < MAP_FILE_VERSION(1, 0)){
    SInvisibility h;

    memset(&h, 0, sizeof(h));
    file.Read(&h, sizeof(h));
    m_strName = h.m_szName;
  }
  else{
    SInvisibility_Old h;

    memset(&h, 0, sizeof(h));
    file.Read(&h, sizeof(h));
    m_strName = h.m_szName;
  }
}

void CEInvisibility::Delete()
{
}

void CEInvisibility::Save(CArchiveFile file)
{
  SInvisibility h;

  memset(&h, 0, sizeof(h));
  strncpy(h.m_szName, m_strName, 31);

  file.Write(&h, sizeof(h));
}

BOOL CEInvisibility::CheckValid()
{
  return TRUE;
}