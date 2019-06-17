// DataArchiveDirectory.h: interface for the CDataArchiveDirectory class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_DATAARCHIVEDIRECTORY_H__80C53A03_1D97_11D3_8901_00C04F512EC3__INCLUDED_)
#define AFX_DATAARCHIVEDIRECTORY_H__80C53A03_1D97_11D3_8901_00C04F512EC3__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <afx.h>
#include "DataArchiveInfo.h"
#include "ArchiveFile.h"	// Added by ClassView

class CDataArchive;

class CDataArchiveDirectory : public CDataArchiveInfo  
{
public:
	// Vytvori archiv (volano konstruktorem)
	virtual BOOL Create();	

	// Vytvori archiv se zadanymi flagy a typem
	virtual BOOL Create(CString lpcszFileName, DWORD dwFlags=CArchiveFile::modeReadWrite, DWORD dwArchiveType=CDataArchiveInfo::archiveDirectory);
	
	// Otevre archiv (prazdna funkce)
	virtual void Open();

	// Uzavre archiv (prazdna funkce)
	virtual void Close();

	// Rebuildne archiv (prazdna funkce)
	virtual void Rebuild();

	// Vrati soubor z archivu otevreny s danymi flagy
	virtual CArchiveFile CreateFile(LPCTSTR lpcszFileName, DWORD dwFlags);

	// Vrati podarchiv archivu otevreny se stejnymi flagy, jako mame my	
	virtual CDataArchive CreateArchive(LPCTSTR lpcszPath, BOOL bCreate);

	// Prida soubor do archivu
	virtual void AppendFile(LPCTSTR lpcszFileName, LPCTSTR lpcszFilePath="", DWORD dwFlags=0);

	// Prida soubor z archivu do archivu
	virtual void AppendFile(CArchiveFile file, LPCTSTR lpcszFilePath="", DWORD dwFlags=0);

	// Pridani celeho podadresare lpcszDirPathFrom (rekurzivne) z archivu ArchiveFrom do adresare lpcszDirPathTo aktualniho archivu
	virtual void AppendDir(LPCTSTR lpcszDirPathTo, CDataArchive ArchiveFrom, LPCTSTR lpcszDirPathFrom, EAppendModes nFlags=appendRecursive);

	// Extrahuje soubor z archivu
	virtual void ExtractFile(LPCTSTR lpcszFileName, LPCTSTR lpcszFilePath="");

	// Odstrani soubor z archivu
	virtual void RemoveFile(LPCTSTR lpcszFileName);

	// Vytvori adresar
	virtual void MakeDirectory(LPCTSTR lpcszDirectoryName);

	// Zrusi adresar
	virtual void RemoveDirectory(LPCTSTR lpcszDirectoryName);

	// Kopiruje soubor
	static BOOL CopyFile(LPCTSTR lpcszSourceFileName, LPCTSTR lpcszDestFileName);

	// Vraci obsah daneho adresare
	virtual CDataArchiveDirContents *GetDirContents(CString strPath);

	// Konstruktor
	CDataArchiveDirectory();

	// Destruktor
	virtual ~CDataArchiveDirectory();

protected:
    // Vytvori adresar bez hazeni exceptions (vraci navratove kody z errno)
    int MakeDirNoExceptions(LPCTSTR lpcszDirectoryName);
};

#endif // !defined(AFX_DATAARCHIVEDIRECTORY_H__80C53A03_1D97_11D3_8901_00C04F512EC3__INCLUDED_)
