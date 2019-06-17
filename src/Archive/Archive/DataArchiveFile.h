// DataArchiveFile.h: interface for the CDataArchiveFile class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_DATAARCHIVEFILE_H__80C53A04_1D97_11D3_8901_00C04F512EC3__INCLUDED_)
#define AFX_DATAARCHIVEFILE_H__80C53A04_1D97_11D3_8901_00C04F512EC3__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CDataArchiveFileDirectory;
class CDataArchiveFileDirectoryItem;
class CDataArchiveFileCommon;

#include <afx.h>
#include "DataArchiveInfo.h"
#include "MappedFile.h"
#include "DataArchiveFileMainHeader.h"
#include "DataArchiveFileDirectoryCache.h"
#include "EAppendModes.h"

class CDataArchiveFile : public CDataArchiveInfo  
{
	friend class CDataArchiveFilePrefixed; // Aby mohl pristupovat na flagy
	friend class CDataArchiveFileCommon; // Pokracovani nasi tridy

public:
	// Vytvori archiv (volano konstruktorem)
	virtual BOOL Create();	

	// Vytvori archiv se zadanymi flagy a typem
	virtual BOOL Create(CString strFileName, DWORD dwFlags=CArchiveFile::modeReadWrite, DWORD dwArchiveType=CDataArchiveInfo::archiveFile);
	
    // Je archiv otevreny?
    BOOL IsOpened();

	// Otevre archiv
	virtual void Open();

	// Uzavre archiv
	virtual void Close();

	// Rebuildne archiv
	virtual void Rebuild();

	// Vrati soubor z archivu otevreny s danymi flagy
	virtual CArchiveFile CreateFile(LPCTSTR lpcszFileName, DWORD dwFlags) {
		return CreateFile(lpcszFileName, dwFlags, DATA_ARCHIVE_FILE_FILE_INITIAL_DATA_SIZE, FALSE);
	}

	// Vrati soubor z archivu otevreny s danymi flagy s pocatecni velikosti
	virtual CArchiveFile CreateFile(LPCTSTR lpcszFileName, DWORD dwFlags, DWORD dwInitialSize)
	{
		return CreateFile(lpcszFileName, dwFlags, dwInitialSize, TRUE);
	}

	// Vrati soubor z archivu otevreny s danymi flagy s pocatecni velikosti, ktery se bude plnit na pocatecni velikost
	virtual CArchiveFile CreateFile(LPCTSTR lpcszFileName, DWORD dwFlags, DWORD dwInitialSize, BOOL bGonnaFill);

	// Otevreni podarchivu v archivu
	virtual CDataArchive CreateArchive(LPCTSTR lpcszPath, BOOL bCreate);

	// Prida soubor do archivu
	virtual void AppendFile(LPCTSTR lpcszFileName, LPCTSTR lpcszFilePath="", DWORD dwFlags=0);

	// Prida soubor z archivu do archivu
	virtual void AppendFile(CArchiveFile file, LPCTSTR lpcszFilePath="", DWORD dwFlags=0);

	// Pridani celeho podadresare lpcszDirPathFrom (rekurzivne) z archivu ArchiveFrom do adresare lpcszDirPathTo aktualniho archivu
	virtual void AppendDir(LPCTSTR lpcszDirPathTo, CDataArchive ArchiveFrom, LPCTSTR lpcszDirPathFrom, EAppendModes nFlags=appendRecursive);

	// Pridani celeho podadresare lpcszDirPathFrom (rekurzivne) z archivu ArchiveFrom do adresare lpcszDirPathTo aktualniho archivu
	// vhodne pro volani z destruktoru, nepridava k archivu reference
	virtual void AppendDirNoRef(LPCTSTR lpcszDirPathTo, CDataArchiveInfo *pArchiveFrom, LPCTSTR lpcszDirPathFrom, EAppendModes nFlags=appendRecursive);

	// Extrahuje soubor z archivu
	virtual void ExtractFile(LPCTSTR lpcszFileName, LPCTSTR lpcszFilePath="");

	// Odstrani soubor z archivu
	virtual void RemoveFile(LPCTSTR lpcszFileName);

	// Vytvori adresar
	virtual void MakeDirectory(LPCTSTR lpcszDirectoryName);

	// Zrusi adresar
	virtual void RemoveDirectory(LPCTSTR lpcszDirectoryName);

	// Vraci klon CDataArchiveFile pro dalsi thread
	virtual CDataArchiveInfo *CreateArchiveInfoClone();

	// Adoptuje soubor !!! neni nutne zamkovat
	virtual void AdoptFile(CArchiveFile &file);

	// Vraci obsah adresare
	CDataArchiveDirContents *GetDirContents(CString strPath);

	// Vraci mapped file, ktery mu patri
	virtual CMappedFile *GetMappedFile() { return m_pMappedFile; }

	// Konstruktor
	CDataArchiveFile();

	// Destruktor
	virtual ~CDataArchiveFile();

public:
	// NEPOUZIVAT RUCNE

	// Vraci FileInfo pro dany soubor
	CDataArchiveFileDirectoryItem *FileInfo(CString strName);

	// Vraci Directory pro dany adresar
	CDataArchiveFileDirectory DirInfo(CString strName);

public:
	// Enum pro flagy
	enum EDataArchiveFileFlags {
		fileFlag=0x00,			// soubor
		directoryFlag=0x01,		// adresar
		compressedFlag=0x02,	// pakovany soubor
		deletedFlag=0x04,		// Smazany soubor
		readOnlyFlag=0x08,		// Read only soubor

		allowedFlags=0x0F		// Maska pro povolene flagy souboru
	};

protected:
	// Udela vlastni rebuild
	void PerformRebuild();

    // Vytvori adresar bez hazeni exceptions (vraci navratove kody z errno)
    int MakeDirNoExceptions(LPCTSTR lpcszDirectoryName);

protected:
	// Spolecna data vsem threadum
	CDataArchiveFileCommon *m_pCommon;

	// Soubor, ve kterem je archiv obsazen
	CMappedFile *m_pMappedFile;

	// Korenovy adresar archivu
	CDataArchiveFileDirectory *m_pRootDirectory;

	// Cache na otevrene adresare
	CDataArchiveFileDirectoryCache *m_pCache;

	// Identifikace threadu
	DWORD m_dwThreadID;

private:
	// Prekopiruje zadany adresar do zadaneho archivu
	void FlushData(CDataArchive Archive, CString strDir);

	// Rozdeli zadanou cestu k souboru(adresari) (strPath) na vlastni cestu a jmeno souboru.
	// Pritom se predela i puvodni cesta
	static void SplitPath(CString &strPath, CString &strDir, CString &strFile);
	
	// Smaze vsechny datove polozky (pouze pro interni pouziti)
	void CleanupDataMembers();
};

#endif // !defined(AFX_DATAARCHIVEFILE_H__80C53A04_1D97_11D3_8901_00C04F512EC3__INCLUDED_)

