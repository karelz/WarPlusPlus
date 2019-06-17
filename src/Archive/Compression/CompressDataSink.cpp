// CompressDataSink.cpp: implementation of the CCompressDataSink class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "CompressDataSink.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CCompressDataSink::CCompressDataSink()
{
	m_pStoreBuffer=NULL;
	m_dwBufferSize=0;
}

CCompressDataSink::~CCompressDataSink()
{
	if(m_pStoreBuffer) {
		delete m_pStoreBuffer;
	}
}

void CCompressDataSink::GetStoreBuffer(void *&pBuffer, DWORD &dwSize)
{
	if(!m_pStoreBuffer) {
		m_pStoreBuffer=new char[COMPRESSION_STORE_BUFFER_DEFAULT_SIZE];
		m_dwBufferSize=COMPRESSION_STORE_BUFFER_DEFAULT_SIZE;
	}
	pBuffer=m_pStoreBuffer;
	dwSize=m_dwBufferSize;
}
