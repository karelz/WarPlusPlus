/************************************************************
 *
 * Projekt: Strategicka hra
 *    Cast: Prekladac
 *   Autor: Helena Kupková
 *  
 *   Popis: CCompilerOutput - abstraktni trida pro vystup kompilatoru.
 *          Potomci jsou CCompilerFileOutput a CCompilerMemoryOutput 
 *          (v jinem souboru)
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

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CCompilerOutput::CCompilerOutput()
	: CCompilerErrorOutput()
{
	m_nPos=0;
}

CCompilerOutput::~CCompilerOutput()
{
//	ASSERT(!m_bOpen);
}

bool CCompilerOutput::Open(CString &strFilename)
{
	CFileStatus FS;
	CFileException *pFE=new CFileException();
	
	ASSERT(!m_bOpen);

	// otevreni souboru
	if (m_fileOut.Open(strFilename, CFile::modeCreate | CFile::modeWrite, pFE)) m_bOpen=true;
	else throw pFE;

	pFE->Delete();
	return true;
}

void CCompilerOutput::Close()
{
	ASSERT(m_bOpen);

	if (m_nPos!=0) Flush();

	m_fileOut.Close();

	m_bOpen=false;
}

void CCompilerOutput::PutChars(const char *pBuffer, int nCount)
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

void CCompilerOutput::Flush()
{
	m_fileOut.Write((void*)m_pBuffer, m_nPos);
	m_nPos=0;
}


