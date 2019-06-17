// DataArchiveInfo.h: interface for the CDataArchiveInfo class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_DATAARCHIVEINFO_H__E8731111_1D8F_11D3_8901_00C04F512EC3__INCLUDED_)
#define AFX_DATAARCHIVEINFO_H__E8731111_1D8F_11D3_8901_00C04F512EC3__INCLUDED_


#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <afx.h>
#include "DataArchiveException.h"
#include "ArchiveFile.h"
#include "EAppendModes.h"

class CDataArchiveDirContents;
class CMappedFile;

class CDataArchiveInfo : public CObject  
{
friend class CDataArchive; // Ten jediny muze volat moje Attach a Detach
friend class CDataArchiveFilePrefixed; // A jeste tenhle muze (navic pouziva ChangeDataArchive)
friend class CArchiveFileInfo; // Ten muze volat FileDestructed
friend class CArchiveFileCompressed; // ....

public:
	// Vytvoreni prazdneho archivu
	virtual BOOL Create(void);

	// Otevreni archivu v souboru daneho jmena
	virtual BOOL Create(CString lpcszFileName, DWORD dwFlags=CArchiveFile::modeReadWrite, DWORD dwArchiveType=CDataArchiveInfo::archiveDirectory);
	
	// Zruseni archivu
	virtual BOOL Delete();

	// Otevreni archivu
	virtual void Open();

	// Zavreni archivu
	virtual void Close();

	// Prebudovani archivu pro rychlejsi pristup
	virtual void Rebuild();

	// Otvirani souboru v archivu
	virtual CArchiveFile CreateFile(LPCTSTR lpcszFileName, DWORD dwFlags=CArchiveFile::modeRead | CFile::shareDenyWrite);

	// Otevreni podarchivu v archivu
	virtual CDataArchive CreateArchive(LPCTSTR lpcszPath, BOOL bCreate=FALSE);

	// Prida externi soubor do archivu
	virtual void AppendFile(LPCTSTR lpcszFileName, LPCTSTR lpcszFilePath="", DWORD dwFlags=0);

	// Prida soubor z archivu do archivu
	virtual void AppendFile(CArchiveFile file, LPCTSTR lpcszFilePath="", DWORD dwFlags=0);

	// Pridani celeho podadresare lpcszDirPathFrom (rekurzivne) z archivu ArchiveFrom do adresare lpcszDirPathTo aktualniho archivu
    virtual void AppendDir(LPCTSTR lpcszDirPathTo, CDataArchive ArchiveFrom, LPCTSTR lpcszDirPathFrom, EAppendModes nFlags=appendRecursive);

	// Pridani celeho podadresare lpcszDirPathFrom (rekurzivne) z archivu ArchiveFrom do adresare lpcszDirPathTo aktualniho archivu
	// Vhodne pro pouziti v destruktoru archivu, na kterem se pracuje - nezveda reference count
    virtual void AppendDirNoRef(LPCTSTR lpcszDirPathTo, CDataArchiveInfo *pArchiveFrom, LPCTSTR lpcszDirPathFrom, EAppendModes nFlags=appendRecursive);

	// Extrahuje soubor z archivu
	virtual void ExtractFile(LPCTSTR lpcszFileName, LPCTSTR lpcszFilePath="");

	// Odstrani soubor z archivu
	virtual void RemoveFile(LPCTSTR lpcszFileName);

	// Vytvori adresar
	virtual void MakeDirectory(LPCTSTR lpcszDirectoryName);

	// Zrusi adresar
	virtual void RemoveDirectory(LPCTSTR lpcszDirectoryName);

	// Vraci, je-li archiv otevreny/uzavreny
	virtual BOOL IsOpened() { return m_bOpened; }

	// Vraci cestu a jmeno archivu
	virtual CString GetArchivePath() { return m_strArchiveName; }

	// Vraci obsah daneho adresare
	virtual CDataArchiveDirContents *GetDirContents(CString strPath);

	// Vraci klon CDataArchiveInfo pro dalsi thread
	virtual CDataArchiveInfo *CreateArchiveInfoClone() { return this; }

	// Adoptuje soubor
	virtual void AdoptFile(CArchiveFile &file) { }

	// Diagnostic support		
#ifdef _DEBUG
	virtual void AssertValid(void) const;	
	virtual void Dump(CDumpContext& dc) const;
#endif

	// Konstruktor
	CDataArchiveInfo();

	// Destruktor
	virtual ~CDataArchiveInfo();

// Zamykani
public:
	// Zamkne archiv
	BOOL Lock(DWORD dwTimeOut = INFINITE) {
		return m_Mutex.Lock(dwTimeOut);
	}

	// Odemkne archiv
	BOOL Unlock() {
		return m_Mutex.Unlock();
	}

public:
	// Typy archivu
	enum EArchiveTypes {
		archiveNone=0x00,			// Zadny archiv
		archiveDirectory=0x01,		// Archiv je adresar se soubory
		archiveFile=0x02			// Archiv je soubor
	};

  	// Vrati ukazatel na mapped file archivu
	virtual CMappedFile *GetMappedFile() { return NULL; }


protected:	
	// Interni funkce pro vytvareni adresaru
    int MakeDirNoExceptions(LPCTSTR lpcszDirectoryName);

	// Jmeno a cesta k archivu
	CString m_strArchiveName;

	// Je archiv otevreny
	BOOL m_bOpened;
	
	// Typ archivu
	EArchiveTypes m_eArchiveType;

	// Pocet souboru v archivu
	int m_nCreatedFiles;

	// Destruktor jiz bezi, nevolat podruhe
	BOOL m_bDestructorRunning;

	// Priznaky archivu
	DWORD m_dwFlags;

	// Zamek na archiv
	CMutex m_Mutex;

private:
	// Interni citac odkazu na letter
	int m_nPointerCounter;

	// Pripoji tento objekt k DataArchive
	void AttachToDataArchive(CDataArchive *Archive);

	// Odpoji tento objekt od DataArchive
	void DetachFromDataArchive(CDataArchive *DataArchive);

	// Volano ArchiveFileInfem pri konstrukci
	virtual void FileConstructed(CArchiveFileInfo *ArchiveFileInfo);

	// Volano ArchiveFileInfem pri destrukci
	virtual void FileDestructed(CArchiveFileInfo *ArchiveFileInfo);
};

#endif // !defined(AFX_DATAARCHIVEINFO_H__E8731111_1D8F_11D3_8901_00C04F512EC3__INCLUDED_)
