// ArchiveFile.h: interface for the CArchiveFile class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_ARCHIVEFILE_H__EFDD1492_0171_11D3_8E13_00E02909C98A__INCLUDED_)
#define AFX_ARCHIVEFILE_H__EFDD1492_0171_11D3_8E13_00E02909C98A__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include <afx.h>

#include "ArchiveFileInfo.h"	// Added by ClassView
#include "DataArchiveException.h"

class CDataArchive;

//////////////////////////////////////////////////////
// CArchiveFile definice
//////////////////////////////////////////////////////

class CArchiveFile : public CFile  
{
// Aby mohl presunout archive file pod sebe
friend class CDataArchiveFilePrefixed;
// Aby mohl prehakovavat infa sem a tam
friend class CDataArchiveFile;
// Aby mohl dostat file info z CArchiveFile pri ChangeDataArchive
friend class CArchiveFileCompressed;
public:

// Construction
// ------------

	// Konstruktor
	CArchiveFile();
	
	// Zavre soubor bez hazeni vyjimek
	virtual void Abort();

	// Otevre soubor
	virtual BOOL Open(void) { ASSERT(m_pFileInfo); return m_pFileInfo->Open(); }

	// Klasicke Open z CFile je ZABLOKOVANE
	virtual BOOL Open(LPCTSTR lpszFileName, UINT nOpenFlags, CFileException* pError = NULL) {
		TRACE("Klasicke CFile::Open() se u CArchiveFile NEPOUZIVA!\nLaskavy programator promine a pouzije radsi CDataArchive::CreateFile pro otevreni souboru daneho jmena.\n");
		ASSERT(FALSE);
		return FALSE;
	}

	// Zavre soubor
	virtual void Close(void) { ASSERT(m_pFileInfo); m_pFileInfo->Close(); }

	// Duplikuje obalku na soubor (NEPOUZIVEJTE, nema to smysl)
	virtual CFile* Duplicate() const;

	// Pomocne funkce

	// Konstruktor obalec jablecny (NEPOUZIVAT! To muze jenom autor...)
	CArchiveFile(CArchiveFileInfo *pArchiveFileInfo, CDataArchiveInfo *pDataArchiveInfo);
	
	// Tenhle zpusob konstrukce je zablokovany
	CArchiveFile(int hFile) { TRACE("CArchiveFile nelze konstruovat timto zpusobem.\n"); ASSERT(FALSE); }

	// Tenhle zpusob konstrukce je zablokovany
	CArchiveFile(LPCTSTR lpszFileName, UINT nOpenFlags) { TRACE("CArchiveFile nelze konstruovat timto zpusobem.\n"); ASSERT(FALSE); }

	// Copy konstruktor
	CArchiveFile(const CArchiveFile& ArchiveFile);
	
	// Operator =
	CArchiveFile& operator=(const CArchiveFile &ArchiveFile);

	// Destruktor
	virtual ~CArchiveFile();

// Input/Output
// ------------

	// Nacteni bloku dat
	virtual UINT Read(void* lpBuf, UINT nCount) { ASSERT(m_pFileInfo); return m_pFileInfo->Read(lpBuf, nCount); }

	// ZABLOKOVANO - ReadHuge nema smysl pouzivat
	DWORD ReadHuge(void* lpBuffer, DWORD dwCount) { 
		TRACE("CArchiveFile::ReadHuge je zablokovano - pouzijte Read\n"); ASSERT(FALSE);
		return 0;
	}

	// Zapis bloku dat
	virtual void Write(const void* lpBuf, UINT nCount) { ASSERT(m_pFileInfo); m_pFileInfo->Write(lpBuf, nCount); }

	// ZABLOKOVANO - WriteHuge nema smysl pouzivat
	DWORD WriteHuge(void* lpBuffer, DWORD dwCount) { 
		TRACE("CArchiveFile::WriteHuge je zablokovano - pouzijte Write\n"); ASSERT(FALSE);
		return 0;
	}

	// Flushne soubor
	virtual void Flush() { ASSERT(m_pFileInfo); m_pFileInfo->Flush(); }

// Position
// --------

	// Seek
	virtual LONG Seek(LONG lOff, UINT nFrom) { ASSERT(m_pFileInfo); return m_pFileInfo->Seek(lOff, nFrom); }

	// Vraci delku souboru
	virtual DWORD GetLength() const { ASSERT(m_pFileInfo); return m_pFileInfo->GetLength(); }

	// Nastavuje delku souboru
	virtual void SetLength(DWORD dwNewLen) { ASSERT(m_pFileInfo); m_pFileInfo->SetLength(dwNewLen); }

// Locking
// -------

	// Zamyka kus souboru
	virtual void LockRange(DWORD dwPos, DWORD dwCount) { ASSERT(m_pFileInfo); m_pFileInfo->LockRange(dwPos, dwCount); }

	// Odemyka kus souboru
	virtual void UnlockRange(DWORD dwPos, DWORD dwCount) { ASSERT(m_pFileInfo); m_pFileInfo->UnlockRange(dwPos, dwCount); }

	// Zamyka cely archiv, ve kterem soubor lezi
	virtual void Lock(DWORD dwTimeOut = INFINITE) { ASSERT(m_pFileInfo); VERIFY(m_pFileInfo->Lock(dwTimeOut)); }

	// Odemyka cely archiv, ve kterem soubor lezi
	virtual void Unlock() { ASSERT(m_pFileInfo); VERIFY(m_pFileInfo->Unlock()); }

// Status
// ------

	// Vraci aktualni pozici v souboru
	virtual DWORD GetPosition() const { ASSERT(m_pFileInfo); return m_pFileInfo->GetPosition(); }

	// Naplni status souboru
	virtual BOOL GetStatus(CFileStatus& rStatus) const { ASSERT(m_pFileInfo); return m_pFileInfo->GetStatus(rStatus); }

	// Vrati vlastni jmeno souboru (soubor.sbr)
	virtual CString GetFileName() const { ASSERT(m_pFileInfo); return m_pFileInfo->GetFileName(); }

	// Vrati jmeno souboru bez pripon (soubor.sbr -> soubor)
	virtual CString GetFileTitle() const { ASSERT(m_pFileInfo); return m_pFileInfo->GetFileTitle(); }

	// Vrati celou cestu k souboru v ramci archivu (cesta\soubor.sbr)
	virtual CString GetFilePath() const { ASSERT(m_pFileInfo); return m_pFileInfo->GetFilePath(); }

	// ZABLOKOVANO, dokud mi nekdo nevysvetli, k cemu to je
	virtual void SetFilePath(LPCTSTR lpszNewName) {
		TRACE("CArchiveFile::SetFilePath je zablokovana, dokud mi nekdo nevysvetli, k cemu je dobra. Mejte se krasne! --Roman\n");
		ASSERT(FALSE);
	}

	// Vrati pouze cestu k souboru v ramci archivu bez posledniho lomitka (cesta\soubor.sbr -> cesta)
	virtual CString GetFileDir() const { ASSERT(m_pFileInfo); return m_pFileInfo->GetFileDir(); }

	// Vrati objekt CDataArchive reprezentujici archiv, ve kterem se soubor nachazi
	virtual CDataArchive GetDataArchive() const;

	// Vrati TRUE, pokud je soubor otevreny
	virtual BOOL IsOpened() const { if(!m_pFileInfo) return FALSE; return m_pFileInfo->IsOpened(); }

	// Vrati TRUE, pokud je soubor pakovany
	virtual BOOL IsCompressed() const { ASSERT(m_pFileInfo); return m_pFileInfo->IsCompressed(); }

	// Vrati TRUE, pokud je soubor otevreny jako raw
	virtual BOOL IsRaw() const { ASSERT(m_pFileInfo); return m_pFileInfo->IsRaw(); }

	// Nastavi soubor na read-only
	virtual BOOL SetReadOnly(BOOL bReadOnly=TRUE) { ASSERT(m_pFileInfo); return m_pFileInfo->SetReadOnly(bReadOnly); }

	// Vrati TRUE, pokud je soubor read only
	virtual BOOL IsReadOnly() const { ASSERT(m_pFileInfo); return m_pFileInfo->IsReadOnly(); }

// Klonovani
// ---------

	virtual CArchiveFile CreateFileClone() const { ASSERT(m_pFileInfo); return m_pFileInfo->CreateFileClone(); }

public:
	enum EArchiveFileFlags {
		modeUnspecified	=0x00,
		modeRead		=CFile::modeRead,
		modeWrite		=CFile::modeWrite,
		modeReadWrite	=CFile::modeReadWrite,
		modeCreate		=CFile::modeCreate,
		modeNoTruncate	=CFile::modeNoTruncate,
		modeRaw			=CFile::typeBinary,
		modeUncompressed=CFile::typeText,

		allowedFlagsForCFile=~(modeRaw | modeUncompressed)
	};

	// Diagnostic support
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

private:	
	// Odkaz na informace o souboru		
	CArchiveFileInfo* m_pFileInfo;
};

#endif // !defined(AFX_ARCHIVEFILE_H__EFDD1492_0171_11D3_8E13_00E02909C98A__INCLUDED_)
