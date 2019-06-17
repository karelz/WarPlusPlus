/************************************************************
 *
 * Projekt: Strategicka hra
 *    Cast: Prekladac
 *   Autor: Helena Kupková
 *  
 *   Popis: Implementace tridy CCompilerInputMemory - vstup kompilatoru
 *          z pametovych bufferu
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

CCompilerInputMemory::CCompilerInputMemory()
{
	m_nPos=0;
	m_nBufferLength=0;
	m_pBuffer=NULL;
	m_pOnEndOfBufferCallback=NULL;
	m_pUserData=NULL;
}

CCompilerInputMemory::~CCompilerInputMemory()
{
}

void CCompilerInputMemory::Open(char *pBuffer, long nBufferLength, 
	 COMPILERMEMORYINPUT_ENDOFBUFFER_CALLBACK pEndOfBufferCallback, void *pUserData/*=NULL*/)
{
	ASSERT(!m_bOpen);

	m_pBuffer=pBuffer;
	m_nBufferLength=nBufferLength;
	m_pOnEndOfBufferCallback=pEndOfBufferCallback;
	m_pUserData=pUserData;

	m_nPos=0;

	m_bOpen=true;
}

void CCompilerInputMemory::Close()
{
	ASSERT(m_bOpen);
	
	m_bOpen=false;
}

int CCompilerInputMemory::GetChar()
{
	ASSERT(m_bOpen);

	if (m_nPos==m_nBufferLength)
	{
		if (m_pOnEndOfBufferCallback!=NULL) (*m_pOnEndOfBufferCallback)(this,false,m_pUserData);
		if (m_nPos==m_nBufferLength) 
        {
            m_chLastChar=EOF;
            return EOF;
        }
	}

	return m_chLastChar=m_pBuffer[m_nPos++];
}

void CCompilerInputMemory::Reset()
{
    (*m_pOnEndOfBufferCallback)(this,true,m_pUserData);
    m_nPos=0;
    m_chLastChar=0;

}

void CCompilerInputMemory::SetBuffer(char *pBuffer, long nBufferLength)
{
	ASSERT(m_bOpen);

	m_pBuffer=pBuffer;
	m_nBufferLength=nBufferLength;

	m_nPos=0;
}

void CCompilerInputMemory::SetEndOfBufferCallback(COMPILERMEMORYINPUT_ENDOFBUFFER_CALLBACK pEndOfBufferCallback, 
	void *pUserData/*=NULL*/)
{
	m_pOnEndOfBufferCallback=pEndOfBufferCallback;
	m_pUserData=pUserData;
}
