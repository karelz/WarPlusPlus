// CompressDataSinkFile.cpp: implementation of the CCompressDataSinkFile class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "CompressDataSinkFile.h"
#include "CompressException.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CCompressDataSinkFile::CCompressDataSinkFile()
{
	m_bOpened=FALSE;
}

CCompressDataSinkFile::~CCompressDataSinkFile()
{
	if(m_bOpened)
		Close();
}

CString CCompressDataSinkFile::GetID() {
	if(m_bOpened) {
		return m_File.GetFilePath();
	} else {
		return "<unopened>";
	}
}

void CCompressDataSinkFile::Open(LPCTSTR lpcszFileName, DWORD dwUncompressedSize) {
	ASSERT(!m_bOpened);
	
	CFileException ex;
	if(m_File.Open(lpcszFileName, CFile::modeWrite | CFile::modeCreate, &ex)) {
		m_bOpened=TRUE;
	} else {
		CString strMsg;
		strMsg.Format("unable to open '%s'", lpcszFileName);
		throw new CCompressException(CCompressException::dataSink, strMsg);
	}	
	m_File.Write(&dwUncompressedSize, sizeof(dwUncompressedSize));
}

void CCompressDataSinkFile::Close() {
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

ECompressData CCompressDataSinkFile::StoreData(void *lpBuf, DWORD dwSize) {
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

