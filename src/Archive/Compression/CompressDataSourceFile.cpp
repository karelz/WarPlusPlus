// CompressDataSourceFile.cpp: implementation of the CCompressDataSourceFile class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "CompressDataSourceFile.h"
#include "CompressException.h"
#include "includes.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CCompressDataSourceFile::CCompressDataSourceFile()
{
	m_bOpened=FALSE;
	m_dwBufSize=COMPRESSION_READ_BUFFER_DEFAULT_SIZE;
	m_pBuffer=NULL;
	m_dwUncompressedSize=0;
}

CCompressDataSourceFile::~CCompressDataSourceFile()
{
	if(m_bOpened)
		Close();
	if(m_pBuffer)
			delete [] m_pBuffer;
}

CString CCompressDataSourceFile::GetID() {
	if(m_bOpened) {
		return m_File.GetFilePath();
	} else {
		return "<unopened>";
	}
}

void CCompressDataSourceFile::Open(LPCTSTR lpcszFileName) {
	ASSERT(!m_bOpened);
	
	CFileException ex;
	if(m_File.Open(lpcszFileName, CFile::modeRead | CFile::shareDenyWrite, &ex)) {
		m_bOpened=TRUE;
	} else {
		CString strMsg;
		strMsg.Format("unable to open '%s'", lpcszFileName);
		throw new CCompressException(CCompressException::dataSource, strMsg);
	}
	m_pBuffer=new char[m_dwBufSize];
	m_File.Read(&m_dwUncompressedSize, sizeof(m_dwUncompressedSize));
}

void CCompressDataSourceFile::Close() {
	if(m_bOpened) {
		try {
			m_File.Close();
			m_bOpened=FALSE;
		} catch(...) {
			CString strMsg;
			strMsg.Format("unable to close '%s'", m_File.GetFilePath());
			throw new CCompressException(CCompressException::dataSource, strMsg);
		}
		if(m_pBuffer) delete [] m_pBuffer;
		m_pBuffer=NULL;
		m_dwUncompressedSize=0;
	}
}

ECompressData CCompressDataSourceFile::GiveData(void *&lpBuf, DWORD &dwSize) {
	ASSERT(m_bOpened);
	try {
		lpBuf=m_pBuffer;		
		dwSize=m_File.Read(lpBuf, m_dwBufSize);
		if(dwSize==0) {
			return compressDataEOF;
		}
	} catch(...) {
		CString strMsg;
		strMsg.Format("unable to write to '%s'", m_File.GetFilePath());
		throw new CCompressException(CCompressException::dataSink, strMsg);
	}
	return compressDataOK;
}
