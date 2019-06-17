// ArchiveFileInFile.cpp: implementation of the CArchiveFileInFile class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ArchiveFile.h"
#include "ArchiveFileInFile.h"
#include "DataArchiveFileDataList.h"
#include "DataArchiveException.h"
#include "DataArchiveFile.h"
#include "DataArchiveFileDirectory.h"
#include "DataArchiveFileDirectoryItem.h"
#include "ArchiveUtils.h"

// Odlapne vyjimku a hodi ji se spravnym jmenem souboru
#define AFIF_RETHROW_BEGIN \
	ASSERT(m_pDataList);	\
	try {


// Konec bloku odlapavani
#define AFIF_RETHROW_END	\
	} catch(CDataArchiveException *pError) {	\
	throw new CDataArchiveException(pError, m_strFileName, CDataArchiveException::fileInArchive, m_pArchiveInfo);	\
	}


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

// Construction
// ------------

CArchiveFileInFile::CArchiveFileInFile()
{
	m_bWasOpened=FALSE;
	m_pDataList=NULL;
	m_dwMode=0;
}

CArchiveFileInFile::~CArchiveFileInFile()
{
	if(m_bWasOpened) {
		Close();
	}
	if(m_pDataList) {
		delete m_pDataList;
	}
}

CArchiveFileInFile::CArchiveFileInFile(CString strFileName, CDataArchiveFileDataList *pDataList, DWORD dwOpenFlags) : CArchiveFileInfo(strFileName) {
	m_pDataList=pDataList;
	m_dwMode=dwOpenFlags;
	
	Open();
}

// Otevre soubor
BOOL CArchiveFileInFile::Open() 
{
	// NEIMPLEMENTOVANO
	m_bWasOpened=TRUE;
	return SUCCESS;
}

// Zavre soubor
void CArchiveFileInFile::Close() 
{
	if(m_bWasOpened) {
		Flush();
	}
	m_bWasOpened=FALSE;
}

// Input/Output
// ------------

// Nacteni bloku dat
UINT CArchiveFileInFile::Read(void* lpBuf, UINT nCount) {
	AFIF_RETHROW_BEGIN
		return m_pDataList->Read(lpBuf, nCount);
	AFIF_RETHROW_END
}


// Zapis bloku dat
void CArchiveFileInFile::Write(const void* lpBuf, UINT nCount) {
	AFIF_RETHROW_BEGIN
		m_pDataList->Write(const_cast <void *>(lpBuf), nCount);
	AFIF_RETHROW_END
}

// Flushne soubor
void CArchiveFileInFile::Flush() {
	AFIF_RETHROW_BEGIN
		if(CArchiveUtils::FlagWrite(m_dwMode)) {
			m_pDataList->Flush();
		}
	AFIF_RETHROW_END
}

// Position
// --------

// Seek
LONG CArchiveFileInFile::Seek(LONG lOff, UINT nFrom) {
	AFIF_RETHROW_BEGIN
		m_pDataList->Seek(lOff, nFrom);
		return m_pDataList->GetPosition();
	AFIF_RETHROW_END
}

// Vraci delku souboru
DWORD CArchiveFileInFile::GetLength() const {
	AFIF_RETHROW_BEGIN
		return m_pDataList->GetLength();
	AFIF_RETHROW_END
}

// Nastavuje delku souboru
void CArchiveFileInFile::SetLength(DWORD dwNewLen) {
	AFIF_RETHROW_BEGIN
		m_pDataList->SetLength(dwNewLen);
	AFIF_RETHROW_END
}

// Locking
// -------

// Zamyka kus souboru
void CArchiveFileInFile::LockRange(DWORD dwPos, DWORD dwCount) {
	TRACE("Funkce LockRange neni implementovana pro archiv v souboru (zatim)\n");
	ASSERT(FALSE);
}

// Odemyka kus souboru
void CArchiveFileInFile::UnlockRange(DWORD dwPos, DWORD dwCount) {
	TRACE("Funkce UnlockRange neni implementovana pro archiv v souboru (zatim)\n");
	ASSERT(FALSE);
}

// Status
// ------

// Vraci aktualni pozici v souboru
DWORD CArchiveFileInFile::GetPosition() const {
	AFIF_RETHROW_BEGIN
		return m_pDataList->GetPosition();
	AFIF_RETHROW_END
}

// Naplni status souboru
BOOL CArchiveFileInFile::GetStatus(CFileStatus& rStatus) const {
	AFIF_RETHROW_BEGIN
		CTime unknownTime;
		unknownTime=0;

		strncpy(rStatus.m_szFullName, (LPCTSTR)m_strFileName, sizeof(rStatus.m_szFullName)); // Jmeno v ramci archivu
		rStatus.m_size=m_pDataList->GetLength(); // Delka
		rStatus.m_attribute=CFile::normal; // Obycejny soubor

		rStatus.m_atime=rStatus.m_ctime=rStatus.m_mtime=unknownTime; // Casy nezname

		return SUCCESS;
	AFIF_RETHROW_END
}

// Vrati TRUE, pokud je soubor pakovany
BOOL CArchiveFileInFile::IsCompressed() const {		
	AFIF_RETHROW_BEGIN
		if(m_dwMode & CArchiveFile::modeUncompressed)
			return FALSE;
		else
			return TRUE;
	AFIF_RETHROW_END
}

BOOL CArchiveFileInFile::IsOpened() const {
	AFIF_RETHROW_BEGIN
		return m_bWasOpened;
	AFIF_RETHROW_END
}

BOOL CArchiveFileInFile::IsRaw() const {
	AFIF_RETHROW_BEGIN
		return m_dwMode & CArchiveFile::EArchiveFileFlags::modeRaw;
	AFIF_RETHROW_END
}

BOOL CArchiveFileInFile::SetReadOnly(BOOL bReadOnly) {
	AFIF_RETHROW_BEGIN
		BOOL ret;
		ASSERT(m_pArchiveInfo);
		CDataArchiveFileDirectoryItem *it;
		it=((CDataArchiveFile *)m_pArchiveInfo)->FileInfo(m_strFileName);
		ASSERT(it);
		ret=it->IsReadOnly();
		it->SetReadOnly(bReadOnly);
		CDataArchiveFileDirectory dir=((CDataArchiveFile *)m_pArchiveInfo)->DirInfo(m_strFileName);
		dir.Touch();
		return ret;
	AFIF_RETHROW_END
}

BOOL CArchiveFileInFile::IsReadOnly() const {
	AFIF_RETHROW_BEGIN
		ASSERT(m_pArchiveInfo);
		CDataArchiveFileDirectoryItem *it;
		it=((CDataArchiveFile *)m_pArchiveInfo)->FileInfo(m_strFileName);
		ASSERT(it);
		return it->IsReadOnly();
	AFIF_RETHROW_END
}

void CArchiveFileInFile::ChangeDataArchive(CDataArchiveInfo *pArchiveInfo, CString strNewFileName)
{
	CArchiveFileInfo::ChangeDataArchive(pArchiveInfo, strNewFileName);
	m_pDataList->SetFile(pArchiveInfo->GetMappedFile());
}