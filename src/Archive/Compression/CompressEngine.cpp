// CompressEngine.cpp: implementation of the CCompressEngine class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "CompressException.h"
#include "CompressEngine.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CCompressEngine::CCompressEngine()
{
	m_eActivity=ECompressEngineActivity::inactiveEngine;
	m_pSink=NULL;
	m_pSource=NULL;
	m_pBuffer=NULL;
}

CCompressEngine::~CCompressEngine()
{
}

CString CCompressEngine::GetID() {
	CString strId;

	switch(m_eActivity) {
	case ECompressEngineActivity::inactiveEngine:
		strId="inactive engine";
		break;
	case ECompressEngineActivity::compressingEngine:
		ASSERT(m_pSink);
		strId="compression to '" + m_pSink->GetID()+"'";
		break;
	case ECompressEngineActivity::decompressingEngine:
		ASSERT(m_pSource);
		strId="decompression from '" + m_pSource->GetID()+"'";
		break;
	}
	return strId;
}

DWORD CCompressEngine::GetPosition() const {
	switch(m_eActivity) {
	case CCompressEngine::ECompressEngineActivity::inactiveEngine:
		return 0;
		break;
	case CCompressEngine::ECompressEngineActivity::compressingEngine:
		return m_ZStream.total_in;
		break;
	case CCompressEngine::ECompressEngineActivity::decompressingEngine:
		return m_ZStream.total_out;
		break;
	default:
		ASSERT(FALSE);
		return 0;
	}
}

void CCompressEngine::Reset() {
	switch(m_eActivity) {
	case CCompressEngine::ECompressEngineActivity::inactiveEngine:
		break;
	case CCompressEngine::ECompressEngineActivity::compressingEngine:
		DoneCompression();
		break;
	case CCompressEngine::ECompressEngineActivity::decompressingEngine:
		DoneDecompression();
		break;
	default:
		ASSERT(FALSE);
		break;
	}
}

voidpf compress_engine_default_alloc_func(voidpf opaque, uInt items, uInt size) {
	opaque=NULL;
	void *ptr=(void *)new char[items*size];
	if(ptr) 
		ZeroMemory(ptr, items*size);
	return ptr;
}

void compress_engine_default_free_func(voidpf opaque, voidpf address) {
	opaque=NULL;
	delete [] (char *)address;
}

void CCompressEngine::InitCompression(CCompressDataSink &DataSink, int nQuality /*=Z_DEFAULT_COMPRESSION*/) {
	ASSERT(m_eActivity==CCompressEngine::ECompressEngineActivity::inactiveEngine);

	m_ZStream.zalloc=(alloc_func)compress_engine_default_alloc_func;
	m_ZStream.zfree=(free_func)compress_engine_default_free_func;
	
	m_pSink=&DataSink;

	// Inicializujeme kompresi
	int result=deflateInit(&m_ZStream, nQuality);
	if(result!=Z_OK) {
		throw new CCompressException((CCompressException::ECompressExceptionCode)result, this);
	}

	// Vyplnime vystupni buffer
	void *pOut; DWORD availOut;
	DataSink.GetStoreBuffer(pOut, availOut);	
	m_pBuffer=pOut;
	m_dwBufferSize=availOut;
	m_ZStream.next_out=(Bytef*)pOut;
	m_ZStream.avail_out=availOut;

	m_eActivity=CCompressEngine::ECompressEngineActivity::compressingEngine;
}


void CCompressEngine::Write(void *lpBuf, DWORD dwSize) {
	m_ZStream.next_in=(Bytef*)lpBuf;
	m_ZStream.avail_in=dwSize;

	while(m_ZStream.avail_in!=0) {
		int result=deflate(&m_ZStream, Z_NO_FLUSH);
		if(result>=0) { // Vsechno okay
			if(m_ZStream.avail_out==0) { // Uz neni co psat
				// Zapiseme buffer do sinku
				m_pSink->StoreData(m_pBuffer, m_dwBufferSize);
				// A zeptame se na dalsi buffer (typicky ten samy)
				m_pSink->GetStoreBuffer(m_pBuffer, m_dwBufferSize);
				m_ZStream.next_out=(Bytef*)m_pBuffer;
				m_ZStream.avail_out=m_dwBufferSize;
			}
		} else {
			throw new CCompressException((CCompressException::ECompressExceptionCode)result, this);
		}
	}
}

void CCompressEngine::DoneCompression() {
	int result=-1;
	while(result!=Z_STREAM_END) {
		result=deflate(&m_ZStream, Z_FINISH);
		if(result>=0) { // Vsechno okay
			if(m_ZStream.avail_out==0 || result==Z_STREAM_END) { // Uz neni co psat
				// Zapiseme buffer do sinku a resetujeme ho
				m_pSink->StoreData(m_pBuffer, m_dwBufferSize-m_ZStream.avail_out);
				m_ZStream.next_out=(Bytef*)m_pBuffer;
				m_ZStream.avail_out=m_dwBufferSize;
			}
		} else {
			throw new CCompressException((CCompressException::ECompressExceptionCode)result, this);
		}
	}
	result=deflateEnd(&m_ZStream);
	m_eActivity=CCompressEngine::ECompressEngineActivity::inactiveEngine;
	m_pSink=NULL;
	if(result<0) {
		throw new CCompressException((CCompressException::ECompressExceptionCode)result, this);		
	}
}

void CCompressEngine::InitDecompression(CCompressDataSource &DataSource) {
	ASSERT(m_eActivity==CCompressEngine::ECompressEngineActivity::inactiveEngine);

	m_ZStream.zalloc=(alloc_func)compress_engine_default_alloc_func;
	m_ZStream.zfree=(free_func)compress_engine_default_free_func;
	
	m_pSource=&DataSource;

	// Inicializujeme kompresi
	int result=inflateInit(&m_ZStream);
	if(result!=Z_OK) {
		throw new CCompressException((CCompressException::ECompressExceptionCode)result, this);
	}

	m_ZStream.next_in=NULL;
	m_ZStream.avail_in=0;

	m_eActivity=CCompressEngine::ECompressEngineActivity::decompressingEngine;
}

DWORD CCompressEngine::Read(void *lpBuf, DWORD dwSize) {
	BOOL isEof=FALSE;
	m_ZStream.next_out=(Bytef*)lpBuf;
	m_ZStream.avail_out=dwSize;
	int result=Z_OK;
	while(m_ZStream.avail_out!=0 &&
		result!=Z_STREAM_END) {
		if(m_ZStream.avail_in==0) { // Dosla mi vstupni data
			void *pIn;
			DWORD dwAvail;
			m_pSource->GiveData(pIn, dwAvail);
			m_ZStream.avail_in=dwAvail;
			m_ZStream.next_in=(Bytef*)pIn;
		}
		result=inflate(&m_ZStream, Z_NO_FLUSH);
		if(m_ZStream.avail_in==0 && result==-5) { // Stale nejsou zadna vstupni data
			// No tak nic, vracime nulu
			return 0;
		}
		if(result>=0) { // Vsechno okay
		} else {
			throw new CCompressException((CCompressException::ECompressExceptionCode)result, this);
		}
	}
	return dwSize-m_ZStream.avail_out;
}

DWORD CCompressEngine::GetUncompressedSize() const {
	switch(m_eActivity) {
	case CCompressEngine::ECompressEngineActivity::decompressingEngine:
		ASSERT(m_pSource);
		return m_pSource->GetUncompressedSize();
		break;
	case CCompressEngine::ECompressEngineActivity::compressingEngine:
		ASSERT(m_pSink);
		return m_ZStream.total_in;
		break;
	default:		
		return 0;
	}
}

void CCompressEngine::DoneDecompression() {
	int result=inflateEnd(&m_ZStream);
	m_eActivity=CCompressEngine::ECompressEngineActivity::inactiveEngine;
	m_pSource=NULL;
	if(result<0) {
		throw new CCompressException((CCompressException::ECompressExceptionCode)result, this);		
	}
}

const char * CCompressEngine::m_strArchiveExtension=".wz";
