// EFindPathGraph.cpp: implementation of the CEFindPathGraph class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "EFindPathGraph.h"

#include "..\MapFormats.h"
#include "..\MapEditor.h"
#include "..\DataObjects\EMap.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

IMPLEMENT_DYNAMIC(CEFindPathGraph, CObject);

CEFindPathGraph::CEFindPathGraph()
{
  m_dwID = 0;
}

CEFindPathGraph::~CEFindPathGraph()
{
    ASSERT ( m_dwID == 0);
}

#ifdef _DEBUG

void CEFindPathGraph::AssertValid() const
{
  CObject::AssertValid();
}

void CEFindPathGraph::Dump(CDumpContext &dc) const
{
  CObject::Dump(dc);
}

#endif

void CEFindPathGraph::Create(DWORD dwID, CString strName)
{
  m_dwID = dwID;
  m_strName = strName;
  m_LandTypes.FillArray(FALSE);
}

void CEFindPathGraph::Create(CArchiveFile file)
{
  if ( g_dwMapFileVersion <= MAP_FILE_VERSION ( 1, 3 ) )
  {
      SFindPathGraphHeader1p3 header;

      VERIFY(file.Read(&header, sizeof(header)) == sizeof(header));

      m_dwID = AppGetActiveMap()->GetNewUnitTypeID ();
      m_strName = header.m_pName;
      memcpy ( m_LandTypes.GetData(), header.m_aAllowedLandTypes, sizeof ( DWORD ) * 8 );
  }
  else
  {
    SFindPathGraphHeader header;

    VERIFY(file.Read(&header, sizeof(header)) == sizeof(header));

    m_dwID = header.m_dwID;
    m_strName = header.m_pName;
    memcpy(m_LandTypes.GetData(), header.m_aAllowedLandTypes, sizeof(DWORD) * 8);
  }
}

void CEFindPathGraph::Delete()
{
    m_dwID = 0;
}

void CEFindPathGraph::Save(CArchiveFile file)
{
  SFindPathGraphHeader header;

  memset(&header, 0, sizeof(header));
  header.m_dwID = m_dwID;
  strncpy(header.m_pName, m_strName, 63);
  memcpy(header.m_aAllowedLandTypes, m_LandTypes.GetData(), sizeof(DWORD) * 8);

  file.Write(&header, sizeof(header));
}

BOOL CEFindPathGraph::CheckValid()
{
  return TRUE;
}