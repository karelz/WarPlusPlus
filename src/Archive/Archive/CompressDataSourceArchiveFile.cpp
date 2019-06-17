// CompressDataSourceArchiveFile.cpp: implementation of the CCompressDataSourceArchiveFile class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "CompressDataSourceArchiveFile.h"
#include "../Compression/CompressException.h"
#include "ArchiveFile.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CCompressDataSourceArchiveFile::CCompressDataSourceArchiveFile()
{
	m_bOpened=FALSE;
	m_dwBufSize=COMPRESSION_READ_BUFFER_DEFAULT_SIZE;
	m_pBuffer=NULL;
	m_dwUncompressedSize=0;
}

CCompressDataSourceArchiveFile::~CCompressDataSourceArchiveFile()
{
	if(m_bOpened)
		Close();
	if(m_pBuffer)
			delete [] m_pBuffer;
}

CString CCompressDataSourceArchiveFile::GetID() {
	if(m_bOpened) {
		return m_File.GetFilePath();
	} else {
		return "<unopened>";
	}
}

void CCompressDataSourceArchiveFile::Open(CArchiveFile file) {
	ASSERT(!m_bOpened);

	m_File=file;
	m_pBuffer=new char[m_dwBufSize];
	if(m_File.GetLength()==0) {
		// Prazdny soubor
		// Prvni vec co udelame - napiseme na zacatek nulu
		m_dwUncompressedSize=0;
		m_File.Write(&m_dwUncompressedSize, sizeof(m_dwUncompressedSize));
	} else {
		if(m_File.Read(&m_dwUncompressedSize, sizeof(m_dwUncompressedSize)) != sizeof(m_dwUncompressedSize)) {
			throw new CCompressException(CCompressException::dataSource, "wrong file format");
		}
	}
	m_bOpened=TRUE;
}

void CCompressDataSourceArchiveFile::Close() {
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

ECompressData CCompressDataSourceArchiveFile::GiveData(void *&lpBuf, DWORD &dwSize) {
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
