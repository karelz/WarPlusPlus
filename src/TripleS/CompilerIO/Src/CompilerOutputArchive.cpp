// CompilerOutputArchive.cpp: implementation of the CCompilerOutputArchive class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "CompilerOutputArchive.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CCompilerOutputArchive::CCompilerOutputArchive()
{
	m_bOpen = false;
}

CCompilerOutputArchive::~CCompilerOutputArchive()
{
	ASSERT( !m_bOpen);
}

bool CCompilerOutputArchive::Open(CDataArchive Archive, CString &strFilename)
{
	ASSERT(!m_bOpen);

	try
	{
		m_fileOut = Archive.CreateFile( strFilename, CArchiveFile::modeCreate | CArchiveFile::modeWrite );
	
		m_bOpen = true;
	
		return true;
	}
	catch (CException *e) 
	{
		e->ReportError();
		e->Delete();

		return false;
	}
}

void CCompilerOutputArchive::Close()
{
	if ( m_nPos != 0) Flush();
  if(m_bOpen)
	  m_fileOut.Close();
	m_bOpen = false;
}

void CCompilerOutputArchive::PutChars(const char *pBuffer, int nCount)
{
	int nToWrite;
	int nBufPos=0;

	ASSERT(m_bOpen);

	while (nCount>0)
	{
		nToWrite=min(nCount, BUFFER_SIZE-m_nPos);

		::CopyMemory(m_pBuffer+m_nPos, pBuffer+nBufPos, nToWrite);
		nBufPos+=nToWrite;
		m_nPos+=nToWrite;
		nCount-=nToWrite;

		if (m_nPos==BUFFER_SIZE) 
		{
			m_fileOut.Write((void*)m_pBuffer, BUFFER_SIZE);
			m_nPos=0;
		}
	}
}

void CCompilerOutputArchive::Flush()
{
	m_fileOut.Write((void*)m_pBuffer, m_nPos);
	m_nPos=0;
}

