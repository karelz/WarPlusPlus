// ArchiveDirectory.cpp: implementation of the CArchiveDirectory class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ArchiveDirectory.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CArchiveDirectory::CArchiveDirectory()
{

}

CArchiveDirectory::~CArchiveDirectory()
{
  Delete();
}

BOOL CArchiveDirectory::Create()
{
  return TRUE;
}

void CArchiveDirectory::Delete()
{
  // clear the array
  m_aNames.RemoveAll();
}

CStringArray * CArchiveDirectory::GetNames()
{
  return &m_aNames;
}
