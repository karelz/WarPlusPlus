// MappedFile.cpp: implementation of the CMappedFile class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "MappedFile.h"
#include "DataArchiveException.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

#define AUTO_OPEN Reopen();

CMappedFile::CMappedFile()
{
    m_strFileName="";
    m_nOpenFlags=0;
    m_dwLastPosition=0;
	m_dwAccesses=0;
}

CMappedFile::CMappedFile(CMappedFile *pMappedFile)
{
    m_strFileName=pMappedFile->m_strFileName;
    m_nOpenFlags=pMappedFile->m_nOpenFlags;
    m_dwLastPosition=pMappedFile->m_dwLastPosition;
	m_dwAccesses=0;

	Open(m_strFileName, m_nOpenFlags | CFile::shareDenyWrite);
}


CMappedFile::~CMappedFile()
{
	Close();
}

void CMappedFile::Open(LPCTSTR FileName, UINT nOpenFlags) {
	CFileException *pError=new CFileException();

	// Nyni je treba zkontrolovat flagy, aby to davalo smysl

	if(nOpenFlags & CFile::modeCreate) {
		// Pokud soubor vytvarime, tak ho budeme chtit cist i zapisovat
		nOpenFlags &= ~CFile::modeWrite;
		nOpenFlags |= CFile::modeReadWrite;
	}

	if(nOpenFlags & CFile::modeWrite) {
		// Soubor jenom pro zapis je nam vetsinou na houby
		nOpenFlags &= ~CFile::modeWrite;
		nOpenFlags |= CFile::modeReadWrite;
	}

	if(!m_File.Open(FileName, nOpenFlags | CFile::shareDenyWrite, pError)) { // Otevreme se
		throw new CDataArchiveException(pError); // Chyba, hazime vyjimku
	} else {
		pError->Delete();  // Mazeme nepotrebnou vyjimku
        m_nOpenFlags=nOpenFlags;
        m_strFileName=FileName;
	}

	// Vynulujeme cache
	m_dwCachePos=0;
	m_dwCacheUsed=0;
	
}

void CMappedFile::Close() {
	// Nikdy nezavirame 
/*	try { 
		if(IsOpened()) {
            m_dwLastPosition=Position();
			m_File.Close(); // Zavirame se
            m_bOpened=FALSE;
		}
	}
	catch(CFileException *fileException) { 
		throw new CDataArchiveException(fileException);
	} */
}

void CMappedFile::Flush() {
	try {
		Reopen();
		m_File.Flush();
	}
	catch(CFileException *fileException) { 
		throw new CDataArchiveException(fileException); 
	}
}

void CMappedFile::Seek(DWORD dwOffset, UINT nFrom) {
	try {
		m_File.Seek(dwOffset, nFrom);		
	}
	catch(CFileException *fileException) { 
		throw new CDataArchiveException(fileException); 
	}
}

UINT CMappedFile::Read(void *pData, UINT nCount) {	
	DWORD pos;
	UINT u;
#ifdef MULTITHREAD_PARANOIA
	m_dwAccesses++;
	if(m_dwAccesses!=1) {
		TRACE("Thread in %X\n", GetCurrentThreadId());
		ASSERT(m_dwAccesses==1);
	}
#endif
	if(nCount==0) return 0;
	try {
		pos=Position();
		// Nahodou muzeme vsechna data vratit z cache
		if(m_dwCacheUsed>0 && m_dwCachePos<=pos && m_dwCachePos+m_dwCacheUsed>=pos+nCount) {
			memcpy(pData, m_pCache+pos-m_dwCachePos, nCount);
			Seek(pos+nCount, CFile::begin);
#ifdef MULTITHREAD_PARANOIA
			m_dwAccesses--;
			if(m_dwAccesses!=0) {
				TRACE("Thread out %X\n", GetCurrentThreadId());
				ASSERT(m_dwAccesses==0);
			}
#endif
			return nCount;
		} else {
			Reopen();
			if(nCount<=CACHE_TRIGGER_SIZE) {
				// Cteme natolik malo, ze cache zbystrila a zacne se prosazovat
				// Nastavime cache
				m_dwCachePos=pos;
				m_dwCacheUsed=m_File.Read(m_pCache, MAPPED_FILE_CACHE_SIZE);				
				// V u bude to, co dostane uzivatel				
				u=nCount;
				if(m_dwCacheUsed<u) u=m_dwCacheUsed;
				memcpy(pData, m_pCache, u);
			} else {
				u=m_File.Read(pData, nCount);
			}
			// Musime nastavit pozici v souboru, jako kdyby probehlo normalni cteni
			Seek(pos+u, CFile::begin);
#ifdef MULTITHREAD_PARANOIA
			m_dwAccesses--;
			if(m_dwAccesses!=0) {
				TRACE("Thread out %X\n", GetCurrentThreadId());
				if ( m_dwAccesses != 0 )
				{
					__asm{
						int 3;
					}
				}
				ASSERT(m_dwAccesses==0);
			}
#endif
			return u;
		}
	}
	catch(CDataArchiveException *pException) { 
#ifdef MULTITHREAD_PARANOIA	
		m_dwAccesses--;
		if(m_dwAccesses!=0) {
			TRACE("Thread out %X\n", GetCurrentThreadId());
			ASSERT(m_dwAccesses==0);
		}
#endif
		throw pException; 
		return 0;
	}
#ifdef MULTITHREAD_PARANOIA	
	m_dwAccesses--;
	if(m_dwAccesses!=0) {
		TRACE("Thread out %X\n", GetCurrentThreadId());
		ASSERT(m_dwAccesses==0);
	}
#endif
}

void CMappedFile::ForceRead(void *pData, UINT nCount) {	
	if(nCount==0) return;
	if(Read(pData, nCount)!=nCount) { // Nenacetli jsme vsechno
		throw new CDataArchiveException(m_File.GetFilePath(), 
					CDataArchiveException::EDataArchiveExceptionTypes::endOfFile, 
					CDataArchiveException::EDataArchiveExceptionLocations::archiveItself);
	}
}

void CMappedFile::Write(void *pData, UINT nCount) {
	try {
		Reopen();
		m_File.Write(pData, nCount);
		// Zneplatnime data cache
		m_dwCacheUsed=0;
	}
	catch(CFileException *fileException) { 
		throw new CDataArchiveException(fileException); 
	}
}

void CMappedFile::Append(void *pData, UINT nCount) {
	try {
        Reopen();
		Seek(0, CFile::end);
		m_File.Write(pData, nCount);		
	}
	catch(CFileException *fileException) { 
		throw new CDataArchiveException(fileException); 
	}
}

DWORD CMappedFile::Length() {
	try {
        Reopen();
		return m_File.GetLength();
	}
	catch(CFileException *fileException) { 
		throw new CDataArchiveException(fileException); 
		return 0;
	}
}

void CMappedFile::SetLength(DWORD dwLength) {
	try {
        Reopen();
		m_File.SetLength(dwLength);
		// Pro jistotu zneplatnime data cache
		m_dwCacheUsed=0;
	}
	catch(CFileException *fileException) { 
		throw new CDataArchiveException(fileException); 
	}
}
	
CString CMappedFile::Path() {	
    if(IsOpened()) {
        return m_File.GetFilePath();
    } else {
        return m_strFileName;
    }
}
