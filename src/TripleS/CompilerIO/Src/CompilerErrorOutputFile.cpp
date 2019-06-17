/************************************************************
 *
 * Projekt: Strategicka hra
 *    Cast: Prekladac
 *   Autor: Helena Kupková
 *  
 *   Popis: Implementace tridy CCompilerErrorOutputFile - vystup kompilatoru do souboru
 * 
 ***********************************************************/

#include "stdafx.h"
#include "..\CompilerIO.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CCompilerErrorOutputFile::CCompilerErrorOutputFile()
{
	m_nPos=0;
}

CCompilerErrorOutputFile::~CCompilerErrorOutputFile()
{

}

bool CCompilerErrorOutputFile::Open(CString& strFilename, CFileException* pFileException/*=NULL*/)
{
	ASSERT(!m_bOpen);

	if (!m_fileOut.Open(strFilename, CFile::modeWrite | CFile::modeCreate | CFile::shareExclusive,
		pFileException)) return false;

	m_bOpen=true;
	
	m_nPos=0;

	return true;
}

void CCompilerErrorOutputFile::Close()
{
	ASSERT(m_bOpen);

	if (m_nPos!=0) Flush();
	
	m_fileOut.Close();

	m_bOpen=false;
}

void CCompilerErrorOutputFile::PutChars(const char *pBuffer, int nCount)
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

void CCompilerErrorOutputFile::Flush()
{
	m_fileOut.Write((void*)m_pBuffer, m_nPos);
	m_nPos=0;
}
