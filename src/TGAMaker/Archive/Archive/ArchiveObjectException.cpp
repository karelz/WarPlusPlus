// ArchiveException.cpp: implementation of the CArchiveException class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ArchiveObjectException.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CArchiveObjectException::CArchiveObjectException(int nType, CString strArchive, CString strFile)
{
  m_nType = nType;
  m_strArchive = strArchive;
  m_strFile = strFile;
}

CArchiveObjectException::CArchiveObjectException(CArchiveObjectException &source)
{
  m_nType = source.m_nType;
  m_strArchive = source.m_strArchive;
  m_strFile = source.m_strFile;
}

CArchiveObjectException::~CArchiveObjectException()
{
}

BOOL CArchiveObjectException::GetErrorMessage(LPSTR lpszError, UINT nMaxError, PUINT pnHelpContext)
{
  char *txt;

  switch(m_nType){
  case NoError: txt = "No error."; break;
  case FileNotExists: txt = "Requested file does not exist."; break;
  case ArchiveNotExists: txt = "Archive or directory not exist."; break;
  default: return FALSE;
  }

  CString msg;
  msg.Format("Archive exception occured :\n  %s\n\n  Archive : '%s'\n  File : '%s'", txt, m_strArchive, m_strFile);
  strncpy(lpszError, msg, nMaxError);

  return TRUE;
}
