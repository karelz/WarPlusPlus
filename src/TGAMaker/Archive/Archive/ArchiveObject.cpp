// ArchiveObject.cpp: implementation of the CArchiveObject class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ArchiveObject.h"
#include "ArchiveObjectException.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CArchiveObject::CArchiveObject()
{

}

CArchiveObject::~CArchiveObject()
{
  Delete();
}

BOOL CArchiveObject::Create(CString strFile)
{
  CFileFind finder;

  if(!finder.FindFile(strFile + "\\*.*")) return FALSE;

  m_strPath = strFile;

  return TRUE;
}

void CArchiveObject::Delete()
{

}

void CArchiveObject::Open()
{

}

void CArchiveObject::Close()
{

}

BOOL CArchiveObject::CreateFile(CString strFile, CArchiveFile *pFile)
{
  CString strPath(m_strPath);
  if(strPath.GetAt(strPath.GetLength() - 1) != '\\')
    strPath += "\\";
  strPath += strFile;
  
  CFileFind finder;
  if(!finder.FindFile(strPath)){
    THROW(new CArchiveObjectException(CArchiveObjectException::FileNotExists, m_strPath, strFile));
  }

  if(!pFile) return FALSE;

  pFile->m_strFileName = strPath;

  return TRUE;
}

BOOL CArchiveObject::CreateDirectory(CString strDirectory, CArchiveDirectory *pDirectory)
{
  if(!pDirectory) return FALSE;

  CString strPath(m_strPath);
  strPath += "\\";
  strPath += strDirectory;

  CFileFind finder;
  BOOL bContinue = TRUE;

  pDirectory->m_aNames.RemoveAll();
  if(!finder.FindFile(strPath)) return FALSE;
  while(bContinue){
    bContinue = finder.FindNextFile();
    pDirectory->m_aNames.Add(finder.GetFileName());
  }

  return TRUE;
}
