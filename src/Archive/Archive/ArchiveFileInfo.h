// ArchiveFileInfo.h: interface for the CArchiveFileInfo class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_ARCHIVEFILEINFO_H__08C5EB12_01FA_11D3_8D99_0000C0C40CDB__INCLUDED_)
#define AFX_ARCHIVEFILEINFO_H__08C5EB12_01FA_11D3_8D99_0000C0C40CDB__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include <afx.h>
#include <afxmt.h>

// Makra pro nedefinovane funkce
#define UNDEFINED_RETURN(ret) { TRACE("Tato funkce neni implementovana.\n"); ASSERT(FALSE); return ret; }
#define UNDEFINED { TRACE("Tato funkce neni implementovana.\n"); ASSERT(FALSE); return; }

class CDataArchiveInfo;
class CDataArchive;

class CArchiveFileInfo : public CObject  
{
friend class CArchiveFile; // Ten jediny muze volat moje metody Attach a Detach
friend class CDataArchiveFilePrefixed; // A tenhle zase ChangeDataArchive
friend class CDataArchiveFile; // Aby mohl prehakovavat infa sem a tam
friend class CArchiveFileCompressed; // ....

public:

// Construction
// ------------

	// Konstruktor
	CArchiveFileInfo();

	// Konstruktor s vyplnenim jmena souboru
	CArchiveFileInfo(CString strFileName);

	// Destruktor
	virtual ~CArchiveFileInfo();
 	
	// Otevre soubor
	virtual BOOL Open() { return TRUE; };

	// Zavre soubor
	virtual void Close() {  };

// Input/Output
// ------------

	// Nacteni bloku dat
	virtual UINT Read(void* lpBuf, UINT nCount) UNDEFINED_RETURN(0)

	// Zapis bloku dat
	virtual void Write(const void* lpBuf, UINT nCount) UNDEFINED;

	// Flushne soubor
	virtual void Flush() { }

// Position
// --------

	// Seek
	virtual LONG Seek(LONG lOff, UINT nFrom) UNDEFINED_RETURN(0);

	// Vraci delku souboru
	virtual DWORD GetLength() const UNDEFINED_RETURN(0);

	// Nastavuje delku souboru
	virtual void SetLength(DWORD dwNewLen) UNDEFINED;

// Locking
// -------

	// Zamyka kus souboru
	virtual void LockRange(DWORD dwPos, DWORD dwCount) UNDEFINED;

	// Odemyka kus souboru
	virtual void UnlockRange(DWORD dwPos, DWORD dwCount) UNDEFINED;

// Status
// ------

	// Vraci aktualni pozici v souboru
	virtual DWORD GetPosition() const UNDEFINED_RETURN(0);

	// Naplni status souboru
	virtual BOOL GetStatus(CFileStatus& rStatus) const UNDEFINED_RETURN(FALSE);

	// Vrati vlastni jmeno souboru (soubor.sbr)
	virtual CString GetFileName() const;

	// Vrati jmeno souboru bez pripon (soubor.sbr -> soubor)
	virtual CString GetFileTitle() const;

	// Vrati celou cestu k souboru v ramci archivu (cesta\soubor.sbr)
	virtual CString GetFilePath() const;

	// Vrati pouze cestu k souboru v ramci archivu bez posledniho lomitka (cesta\soubor.sbr -> cesta)
	virtual CString GetFileDir() const;

	// Vrati objekt CDataArchive reprezentujici archiv, ve kterem se tento soubor nachazi
	virtual CDataArchive GetDataArchive() const;

	// Vrati TRUE, pokud je soubor otevreny
	virtual BOOL IsOpened() const UNDEFINED_RETURN(FALSE);

	// Vrati TRUE, pokud je soubor pakovany
	virtual BOOL IsCompressed() const UNDEFINED_RETURN(FALSE);

	// Vrati TRUE, pokud je soubor otevren v raw modu
	virtual BOOL IsRaw() const UNDEFINED_RETURN(FALSE);

	// Nastavi soubor na read-only
	virtual BOOL SetReadOnly(BOOL bReadOnly=TRUE) UNDEFINED_RETURN(FALSE);

	// Vrati TRUE, pokud je soubor read only
	virtual BOOL IsReadOnly() const UNDEFINED_RETURN(FALSE);

	// Zamkne archiv, ve kterem soubor lezi
	virtual BOOL Lock(DWORD dwTimeOut = INFINITE);

	// Odemkne archiv
	virtual BOOL Unlock();

// Klonovani
// ---------
public:
	virtual CArchiveFile CreateFileClone();

protected:
	// Volano konstruktorem
	BOOL Create();

	// Volano konstruktorem
	BOOL Create(CString strFileName);

	// Volano destruktorem
	BOOL Delete();

	// Pripojeni k CArchiveFile
	virtual void AttachToArchiveFile(CArchiveFile *ArchiveFile);
	
	// Odpojeni od CArchiveFile
	virtual void DetachFromArchiveFile(CArchiveFile *ArchiveFile);

	// Pripojeni k archivu
	virtual void AttachToDataArchive(CDataArchiveInfo *pArchiveInfo);

	// Odpojeni od stavajiciho a pripojeni k jinemu archivu, se zmenou m_strFileName
	virtual void ChangeDataArchive(CDataArchiveInfo *pArchiveInfo, CString strNewFileName);

	// Pocet odkazu z ostatnich CArchiveFile
	int m_nPointerCounter;

	// Odkaz na informace o archivu
	CDataArchiveInfo* m_pArchiveInfo;

	// Jmeno a plna cesta k souboru v ramci archivu
	CString m_strFileName;
};

#endif // !defined(AFX_ARCHIVEFILEINFO_H__08C5EB12_01FA_11D3_8D99_0000C0C40CDB__INCLUDED_)
