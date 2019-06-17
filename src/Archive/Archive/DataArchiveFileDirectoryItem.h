// DataArchiveFileDirectoryItem.h: interface for the CDataArchiveFileDirectoryItem class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_DATAARCHIVEFILEDIRECTORYITEM_H__1FF566D1_7D76_11D3_8991_00C04F513B85__INCLUDED_)
#define AFX_DATAARCHIVEFILEDIRECTORYITEM_H__1FF566D1_7D76_11D3_8991_00C04F513B85__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "DataArchiveFile.h"
#include "DataArchiveFileMainHeader.h"
#include "Definitions.h"	// Added by ClassView

class CDataArchiveFileDirectoryItem {
public:
	// Konstruktor
	CDataArchiveFileDirectoryItem(CString strName, DWORD dwFlags, DWORD dwOffset) {
		m_strName=strName; m_dwFlags=dwFlags; m_dwOffset=dwOffset;
	}
	
	// Jmeno souboru
	CString GetName() { 
		return m_strName; 
	}

	// Offset souboru
	DWORD GetOffset() { 
		ASSERT(m_dwOffset>sizeof(SDataArchiveFileMainHeader));
		return m_dwOffset; 
	}

	// Kolik bajtu zabere tato polozka pri ukladani do souboru
	DWORD GetStorageSize() {
		return m_strName.GetLength()+1+sizeof(m_dwFlags)+sizeof(m_dwOffset);
	}

	// Adresar?
	BOOL IsDirectory() {
		return m_dwFlags & CDataArchiveFile::directoryFlag;
	}

	// Soubor? (ne adresar)
	BOOL IsFile() {
		return !(m_dwFlags & CDataArchiveFile::directoryFlag);
	}

	// Pakovany soubor?
	BOOL IsCompressed() {
		return m_dwFlags & CDataArchiveFile::compressedFlag;
	}

	// Smazany soubor?
	BOOL IsDeleted() {
		return m_dwFlags & CDataArchiveFile::deletedFlag;
	}

	// Read only soubor?
	BOOL IsReadOnly() {
		return m_dwFlags & CDataArchiveFile::readOnlyFlag;
	}

	// Vrati flagy
	DWORD GetFlags() {
		return m_dwFlags;
	}

	// Nastavi flagy (nic se nikdy nezapise na disk, jsou nastavene jenom v pameti)
	void SetFlags(DWORD dwFlags) {
		ASSERT(!(dwFlags & ~CDataArchiveFile::allowedFlags));
		m_dwFlags=dwFlags;
	}

	// Nastavi deleted flag (nic se nikdy nezapise na disk, je nastaveny jenom v pameti)
	void SetDeleted() {
		m_dwFlags|=CDataArchiveFile::deletedFlag;
	}

	// Nastavi read only flag (nic se nikdy nezapise na disk, je astavenyjenom v pameti)
	void SetReadOnly(BOOL bReadOnly=TRUE) {
		if(bReadOnly) {
			m_dwFlags|=CDataArchiveFile::readOnlyFlag;
		} else {
			m_dwFlags&=~CDataArchiveFile::readOnlyFlag;
		}
	}

	// Ulozi item do bufferu, vraci posunuty pointer
	char *Store(char *pBuf) {
		// ulozime nejdrive jmeno
		strcpy(pBuf, (LPCTSTR)m_strName);
		pBuf+=m_strName.GetLength()+1;
		
		// Pak flagy
		*(DWORD *)pBuf=m_dwFlags;
		pBuf+=sizeof(m_dwFlags);		
		
		// Nakonec offset
		*(DWORD *)pBuf=m_dwOffset;
		pBuf+=sizeof(m_dwOffset);

		return pBuf;
	}

private:
	CString m_strName;
	DWORD m_dwFlags;
	DWORD m_dwOffset;
};

#endif // !defined(AFX_DATAARCHIVEFILEDIRECTORYITEM_H__1FF566D1_7D76_11D3_8991_00C04F513B85__INCLUDED_)
