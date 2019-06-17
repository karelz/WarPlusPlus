// CompressDataSinkArchiveFile.cpp: implementation of the CCompressDataSinkArchiveFile class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "CompressDataSinkArchiveFile.h"
#include "../Compression/CompressException.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CCompressDataSinkArchiveFile::CCompressDataSinkArchiveFile()
{
	m_bOpened=FALSE;
}

CCompressDataSinkArchiveFile::~CCompressDataSinkArchiveFile()
{
	if(m_bOpened)
		Close();
}

CString CCompressDataSinkArchiveFile::GetID() {
	if(m_bOpened) {
		return m_File.GetFilePath();
	} else {
		return "<unopened>";
	}
}

void CCompressDataSinkArchiveFile::Open(CArchiveFile file, DWORD dwUncompressedSize) {
	ASSERT(!m_bOpened);
	
	m_File=file;
	m_File.Write(&dwUncompressedSize, sizeof(dwUncompressedSize));
	m_bOpened=TRUE;
}

void CCompressDataSinkArchiveFile::Close() {
	if(m_bOpened) {
		try {
			m_File.Close();
			m_bOpened=FALSE;
		} catch(...) {
			CString strMsg;
			strMsg.Format("unable to close '%s'", m_File.GetFilePath());
			throw new CCompressException(CCompressException::dataSink, strMsg);
		}
	}
}

ECompressData CCompressDataSinkArchiveFile::StoreData(void *lpBuf, DWORD dwSize) {
	ASSERT(m_bOpened);
	try {
		m_File.Write(lpBuf, dwSize);
	} catch(...) {
		CString strMsg;
		strMsg.Format("unable to write to '%s'", m_File.GetFilePath());
		throw new CCompressException(CCompressException::dataSink, strMsg);
	}
	return compressDataOK;
}

