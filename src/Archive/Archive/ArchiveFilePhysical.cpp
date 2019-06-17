// ArchiveFilePhysical.cpp: implementation of the CArchiveFilePhysical class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ArchiveFile.h"
#include "ArchiveFilePhysical.h"
#include "ArchiveUtils.h"
#include "Definitions.h"

// Otevre soubor
BOOL CArchiveFilePhysical::Open() {
	if(m_strPhysName=="") { // Jmeno, ktere si pamatujeme, je prazdne!
		return FAILURE;
	} else if(!m_bWasOpened) { // Jeste nas nikdo nezkusil otevrit
		CFileException *pError=new CFileException();
		DWORD dwMode=m_dwMode;
		if(m_bReadOnly) dwMode &=~(CArchiveFile::modeWrite | CArchiveFile::modeReadWrite);
		if(!m_file.Open(m_strPhysName, (dwMode & ~CArchiveFile::modeCreate) & CArchiveFile::allowedFlagsForCFile, pError)) { // Otevreme se
			throw new CDataArchiveException(pError, m_pArchiveInfo); // Chyba, hazime vyjimku
			return FAILURE;
		} else {
			m_bWasOpened=TRUE; // Vse v pohode, byli jsme otevreni
			pError->Delete();  // Mazeme nepotrebnou vyjimku
			return SUCCESS;
		}
	}
	return SUCCESS;
}

// Zavre soubor
void CArchiveFilePhysical::Close() {
	try { 
		if(m_bWasOpened) { // Jeste jsme otevreni
			m_file.Close(); // Zavirame se
			m_bWasOpened=FALSE;
		} 
	}
	catch(CFileException *fileException) { 
		throw new CDataArchiveException(fileException, m_pArchiveInfo); 
	}
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CArchiveFilePhysical::CArchiveFilePhysical()
{
	m_strFileName="";
	m_strPhysName="";
	m_dwMode=0;
	m_bReadOnly=FALSE;
	m_bWasOpened=FALSE;
}

CArchiveFilePhysical::CArchiveFilePhysical(CString strFileName, CString strPhysName, DWORD dwOpenFlags)
{
	CFileException *pError=new CFileException();
	m_bReadOnly=FALSE;
	m_bWasOpened=FALSE;
	if(!m_file.Open(strPhysName, (dwOpenFlags & CArchiveFile::allowedFlagsForCFile), pError)) { // Doslo k chybe
		throw new CDataArchiveException(pError, m_pArchiveInfo);
	} else { 
		// smazeme nepotrebne misto na vyjimku
		m_bWasOpened=TRUE;
		m_strFileName=strFileName;
		m_strPhysName=strPhysName; // Fyzicke misto na disku
		m_dwMode=dwOpenFlags & ~CArchiveFile::modeCreate;
		pError->Delete();
	}
	// A hnedka soubor uzavreme, dokud se s nim nebude neco dit
}

CArchiveFilePhysical::~CArchiveFilePhysical()
{
}

// Input/Output
// ------------

// Nacteni bloku dat
UINT CArchiveFilePhysical::Read(void *lpBuf, UINT nCount) {
	if(!m_bWasOpened && !Open()) { // Nebylo otevreno, otevirame a nejde to
		throw new CDataArchiveException(GetFileName(), CDataArchiveException::fileNotAssigned, CDataArchiveException::fileInArchive, m_pArchiveInfo);
	} else { // Povedlo se otevrit
		try { 
			return m_file.Read(lpBuf, nCount);
		} catch(CFileException *pError) { throw new CDataArchiveException(pError, m_pArchiveInfo); }
	}
	return 0;
}

// Zapis bloku dat
void CArchiveFilePhysical::Write(const void *lpBuf, UINT nCount) {
	if(!m_bWasOpened && !Open()) { // Nebylo otevreno, otevirame a nejde to
		throw new CDataArchiveException(GetFileName(), CDataArchiveException::fileNotAssigned, CDataArchiveException::fileInArchive, m_pArchiveInfo);
	} else { // Povedlo se otevrit
		try { 
			m_file.Write(lpBuf, nCount);
		} catch(CFileException *pError) { throw new CDataArchiveException(pError, m_pArchiveInfo); }
	}
}

// Flushne soubor
void CArchiveFilePhysical::Flush() {
	if(m_bWasOpened && CArchiveUtils::FlagWrite(m_dwMode) && !m_bReadOnly) { 
		try { 
			m_file.Flush();
		} catch(CFileException *pError) { throw new CDataArchiveException(pError, m_pArchiveInfo); }
	}
}

// Position
// --------

// Seek
LONG CArchiveFilePhysical::Seek(LONG lOff, UINT nFrom) {
	if(!m_bWasOpened && !Open()) { // Nebylo otevreno, otevirame a nejde to
		throw new CDataArchiveException(GetFileName(), CDataArchiveException::fileNotAssigned, CDataArchiveException::fileInArchive, m_pArchiveInfo);
	} else { // Povedlo se otevrit
		try { 
			return m_file.Seek(lOff, nFrom);
		} catch(CFileException *pError) { throw new CDataArchiveException(pError, m_pArchiveInfo); }
	}
	return 0;
}


// Vraci delku souboru
DWORD CArchiveFilePhysical::GetLength() const {
	if(!m_bWasOpened && !((CArchiveFilePhysical*)this)->Open()) { // Nebylo otevreno, otevirame a nejde to
		throw new CDataArchiveException(GetFileName(), CDataArchiveException::fileNotAssigned, CDataArchiveException::fileInArchive, m_pArchiveInfo);
	} else { // Povedlo se otevrit
		try { 
			return m_file.GetLength();
		} catch(CFileException *pError) { throw new CDataArchiveException(pError, m_pArchiveInfo); }
	}
	return 0;
}


// Nastavuje delku souboru
void CArchiveFilePhysical::SetLength(DWORD dwNewLen) {
	if(!m_bWasOpened && !Open()) { // Nebylo otevreno, otevirame a nejde to
		throw new CDataArchiveException(GetFileName(), CDataArchiveException::fileNotAssigned, CDataArchiveException::fileInArchive, m_pArchiveInfo);
	} else { // Povedlo se otevrit
		try { 
			m_file.SetLength(dwNewLen);
		} catch(CFileException *pError) { throw new CDataArchiveException(pError, m_pArchiveInfo); }
	}
}


// Locking
// -------

// Zamyka kus souboru
void CArchiveFilePhysical::LockRange(DWORD dwPos, DWORD dwCount) {
	if(!m_bWasOpened && !Open()) { // Nebylo otevreno, otevirame a nejde to
		throw new CDataArchiveException(GetFileName(), CDataArchiveException::fileNotAssigned, CDataArchiveException::fileInArchive, m_pArchiveInfo);
	} else { // Povedlo se otevrit
		try { 
			m_file.LockRange(dwPos, dwCount);
		} catch(CFileException *pError) { throw new CDataArchiveException(pError, m_pArchiveInfo); }
	}
}

// Odemyka kus souboru
void CArchiveFilePhysical::UnlockRange(DWORD dwPos, DWORD dwCount){
	if(!m_bWasOpened && !Open()) { // Nebylo otevreno, otevirame a nejde to
		throw new CDataArchiveException(GetFileName(), CDataArchiveException::fileNotAssigned, CDataArchiveException::fileInArchive, m_pArchiveInfo);
	} else { // Povedlo se otevrit
		try { 
			m_file.UnlockRange(dwPos, dwCount);
		} catch(CFileException *pError) { throw new CDataArchiveException(pError, m_pArchiveInfo); }
	}
}


// Status
// ------

// Vraci aktualni pozici v souboru
DWORD CArchiveFilePhysical::GetPosition() const {
	if(!m_bWasOpened) { // Nebylo otevreno, otevirame a nejde to
		return 0;
	} else { // Povedlo se otevrit
		try { 
			return m_file.GetPosition();
		} catch(CFileException *pError) { throw new CDataArchiveException(pError, m_pArchiveInfo); }
	}
	return 0;
}

// Naplni status souboru
BOOL CArchiveFilePhysical::GetStatus(CFileStatus& rStatus) const {
	if(!m_bWasOpened && !((CArchiveFilePhysical*) this)->Open()) { // Nebylo otevreno, otevirame a nejde to
		throw new CDataArchiveException(GetFileName(), CDataArchiveException::fileNotAssigned, CDataArchiveException::fileInArchive, m_pArchiveInfo);
	} else { // Povedlo se otevrit
		try { 
			return m_file.GetStatus(rStatus);
		} catch(CFileException *pError) { throw new CDataArchiveException(pError, m_pArchiveInfo); }
	}
	return 0;
}

BOOL CArchiveFilePhysical::IsCompressed() const {
	return m_strPhysName.Right(3)!=m_strFileName.Right(3);	
}

BOOL CArchiveFilePhysical::IsOpened() const {
	return m_bWasOpened;
}

BOOL CArchiveFilePhysical::IsReadOnly() const {
	CFileStatus rStatus;
	if(GetStatus(rStatus)) {
		return rStatus.m_attribute & CFile::Attribute::readOnly;
	} else {
		throw new CDataArchiveException(GetFileName(), CDataArchiveException::generic, CDataArchiveException::fileInArchive, m_pArchiveInfo);
	}	
}

BOOL CArchiveFilePhysical::SetReadOnly(BOOL bReadOnly) {
	CFileStatus rStatus;
	DWORD pos=GetPosition();
	if(GetStatus(rStatus)) {
		DWORD ret;
		ret=rStatus.m_attribute & CFile::Attribute::readOnly;
		if(bReadOnly) {
			rStatus.m_attribute|=CFile::Attribute::readOnly;
		} else {
			rStatus.m_attribute&=~CFile::Attribute::readOnly;
		}
		try {
			// Jestlize doslo ke zmene (tohle je rozepsany logicky xor... )
			if((ret && !bReadOnly) || (!ret && bReadOnly)) {
				BOOL bOpen;
				bOpen=IsOpened();
				// Zavreme soubor
				if(bOpen) Close();
				// Nastavime read only flag
				CFile::SetStatus(m_strPhysName, rStatus);
				// Zapamatujeme si, jak jsme ho nastavili (pro otevirani)
				if(bReadOnly) m_bReadOnly=TRUE; else m_bReadOnly=FALSE;
				if(bOpen) {
					// Jestli byl otevreny, opet jej otevreme					
					Open();
					// A nastavime vse tak, jak to bylo
					Seek(pos, CFile::begin);
				}
			}
			return ret;
		} catch(CFileException *pError) { throw new CDataArchiveException(pError, m_pArchiveInfo); }		
	} else {
		throw new CDataArchiveException(GetFileName(), CDataArchiveException::generic, CDataArchiveException::fileInArchive, m_pArchiveInfo);
	}	
}