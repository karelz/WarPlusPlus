/************************************************************
 *
 * Projekt: Strategicka hra
 *    Cast: Prekladac
 *   Autor: Helena Kupková
 *  
 *   Popis: Implementace tridy CCompilerErrorOutputMemory - vystup kompilatoru
 *          do pametovych bufferu
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

CCompilerErrorOutputMemory::CCompilerErrorOutputMemory()
{
	m_nPos=0;
	m_nBufferLength=0;
	m_pBuffer=NULL;
	m_pOnBufferFullCallback=NULL;
	m_pUserData=NULL;
}

CCompilerErrorOutputMemory::~CCompilerErrorOutputMemory()
{
}

void CCompilerErrorOutputMemory::Open(char *pBuffer, long nBufferLength, 
	COMPILERMEMORYOUTPUT_BUFFERFULL_CALLBACK pBufferFullCallback, void *pUserData/*=NULL*/)
{
	ASSERT(!m_bOpen);
	ASSERT(pBufferFullCallback!=NULL);

	m_pBuffer=pBuffer;
	m_nBufferLength=nBufferLength;
	m_pOnBufferFullCallback=pBufferFullCallback;
	m_pUserData=pUserData;

	m_nPos=0;

	m_bOpen=true;
}

void CCompilerErrorOutputMemory::Close()  // returns length of the valid part in buffer
{
	ASSERT(m_bOpen);
	m_bOpen=false;
}

void CCompilerErrorOutputMemory::PutChars(const char *pBuffer, int nCount)
{
	int nToWrite, nPomCount=nCount;
	int nBufPos=0;

	ASSERT(m_bOpen);

	while (nCount>0)
	{
		nToWrite=min(nCount, m_nBufferLength-m_nPos);

		::CopyMemory(m_pBuffer+m_nPos, pBuffer+nBufPos, nToWrite);
		nBufPos+=nToWrite;
		m_nPos+=nToWrite;
		nCount-=nToWrite;

		if (m_nPos==m_nBufferLength) 
		{
			ASSERT(m_pOnBufferFullCallback!=NULL);
			(*m_pOnBufferFullCallback)(this, m_nPos, m_pUserData);
			m_nPos=0;
		}
	}
}

void CCompilerErrorOutputMemory::Flush()
{
	ASSERT(m_pOnBufferFullCallback!=NULL);
	if (m_nPos>0) (*m_pOnBufferFullCallback)(this, m_nPos, m_pUserData);
	m_nPos=0;
}

void CCompilerErrorOutputMemory::SetBuffer(char *pBuffer, long nBufferLength)
{
	m_pBuffer=pBuffer;
	m_nBufferLength=nBufferLength;

	m_nPos=0;
}

void CCompilerErrorOutputMemory::SetBufferFullCallback(COMPILERMEMORYOUTPUT_BUFFERFULL_CALLBACK pBufferFullCallback, 
	void *pUserData/*=NULL*/)
{
	ASSERT(pBufferFullCallback!=NULL);
	m_pOnBufferFullCallback=pBufferFullCallback;
	m_pUserData=pUserData;
}
