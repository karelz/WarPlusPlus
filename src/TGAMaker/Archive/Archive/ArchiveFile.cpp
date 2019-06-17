// ArchiveFile.cpp: implementation of the CArchiveFile class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ArchiveFile.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CArchiveFile::CArchiveFile()
{

}

CArchiveFile::CArchiveFile(const CArchiveFile &src)
{
  m_strFileName = src.m_strFileName;
}

CArchiveFile::~CArchiveFile()
{
  Delete();
}

BOOL CArchiveFile::Create()
{
  return TRUE;
}

void CArchiveFile::Delete()
{
  Close();
}

void CArchiveFile::Open()
{
  if(!CFile::Open(m_strFileName, CFile::modeRead)) return;
}

void CArchiveFile::Close()
{
  if(m_hFile != CFile::hFileNull)
    CFile::Close();
}

LONG CArchiveFile::Seek(LONG lOffset, UINT uFlags)
{
  return CFile::Seek(lOffset, uFlags);
}

UINT CArchiveFile::Read(void *pBuffer, UINT nCount)
{
  return CFile::Read(pBuffer, nCount);
}

CString CArchiveFile::GetFileName() const
{
  if(m_hFile != CFile::hFileNull)
    return CFile::GetFileName();
  return m_strFileName.Right(m_strFileName.GetLength() - m_strFileName.ReverseFind('\\') - 1);
}

CString CArchiveFile::GetFilePath() const
{
  if(m_hFile != CFile::hFileNull)
    return CFile::GetFilePath();
  return m_strFileName;
}

DWORD CArchiveFile::GetPosition() const
{
  return CFile::GetPosition();
}

DWORD CArchiveFile::GetLength() const
{
  return CFile::GetLength();
}
