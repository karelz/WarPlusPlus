// DataArchiveFilePrefixed.h: interface for the CDataArchiveFilePrefixed class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_DATAARCHIVEFILEPREFIXED_H__96109BD3_1B4F_11D4_805F_0000B48431EC__INCLUDED_)
#define AFX_DATAARCHIVEFILEPREFIXED_H__96109BD3_1B4F_11D4_805F_0000B48431EC__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "DataArchiveFile.h"
#include "ArchiveUtils.h"
#include "DataArchive.h"

#define CATPATH(x) CArchiveUtils::ConcatenatePaths(strPrefix, (x))

class CDataArchiveFilePrefixed : public CDataArchiveInfo  
{
public:
	// Vytvori archiv (volano konstruktorem)
	virtual BOOL Create();	

	// Vytvori archiv se zadanymi flagy a typem
	virtual BOOL Create(CString strDirPath, CDataArchiveFile *pHostArchive);
	
	// Otevre archiv
	virtual void Open() { ASSERT(m_pArchiveFile); m_pArchiveFile->Open(); }

	// Uzavre archiv
	virtual void Close() { ASSERT(m_pArchiveFile); m_pArchiveFile->Close(); }

	// Rebuildne archiv
	virtual void Rebuild() { ASSERT(m_pArchiveFile); m_pArchiveFile->Rebuild(); }

	// Vrati soubor z archivu otevreny s danymi flagy
	virtual CArchiveFile CreateFile(LPCTSTR lpcszFileName, DWORD dwFlags) {
		return CreateFile(lpcszFileName, dwFlags, DATA_ARCHIVE_FILE_FILE_INITIAL_DATA_SIZE);
	}

	// Vrati soubor z archivu otevreny s danymi flagy s pocatecni velikosti
	virtual CArchiveFile CreateFile(LPCTSTR lpcszFileName, DWORD dwFlags, DWORD dwInitialSize);

	// Otevreni podarchivu v archivu
	virtual CDataArchive CreateArchive(LPCTSTR lpcszPath, BOOL bCreate) { ASSERT(m_pArchiveFile); return m_pArchiveFile->CreateArchive(CATPATH(lpcszPath), bCreate); }

	// Pridani celeho podadresare lpcszDirPathFrom (rekurzivne) z archivu ArchiveFrom do adresare lpcszDirPathTo aktualniho archivu
    virtual void AppendDir(LPCTSTR lpcszDirPathTo, CDataArchive ArchiveFrom, LPCTSTR lpcszDirPathFrom, EAppendModes nFlags=appendRecursive) { ASSERT(m_pArchiveFile); m_pArchiveFile->AppendDir(CATPATH(lpcszDirPathTo), ArchiveFrom, lpcszDirPathFrom, nFlags); }

	// Prida soubor do archivu
	virtual void AppendFile(LPCTSTR lpcszFileName, LPCTSTR lpcszFilePath="", DWORD dwFlags=0) { ASSERT(m_pArchiveFile); m_pArchiveFile->AppendFile(lpcszFileName, CATPATH(lpcszFilePath), dwFlags); }

	// Prida soubor z archivu do archivu
	virtual void AppendFile(CArchiveFile file, LPCTSTR lpcszFilePath="", DWORD dwFlags=0) { ASSERT(m_pArchiveFile); m_pArchiveFile->AppendFile(file, CATPATH(lpcszFilePath), dwFlags); }

	// Extrahuje soubor z archivu
	virtual void ExtractFile(LPCTSTR lpcszFileName, LPCTSTR lpcszFilePath="") { ASSERT(m_pArchiveFile); m_pArchiveFile->ExtractFile(lpcszFileName, CATPATH(lpcszFilePath)); }

	// Odstrani soubor z archivu
	virtual void RemoveFile(LPCTSTR lpcszFileName) { ASSERT(m_pArchiveFile); m_pArchiveFile->RemoveFile(CATPATH(lpcszFileName)); }

	// Vytvori adresar
	virtual void MakeDirectory(LPCTSTR lpcszDirectoryName) { ASSERT(m_pArchiveFile); m_pArchiveFile->MakeDirectory(CATPATH(lpcszDirectoryName)); }

	// Zrusi adresar
	virtual void RemoveDirectory(LPCTSTR lpcszDirectoryName) { ASSERT(m_pArchiveFile); m_pArchiveFile->RemoveDirectory(CATPATH(lpcszDirectoryName)); }

	// Vraci obsah adresare
	CDataArchiveDirContents *GetDirContents(CString strPath) { ASSERT(m_pArchiveFile); return m_pArchiveFile->GetDirContents(CATPATH(strPath)); }

	// Vraci naklonovany vnitrek archivu
	CDataArchiveInfo *CreateArchiveInfoClone();

	// Adoptuje soubor
	virtual void AdoptFile(CArchiveFile &file);

	// Konstruktor
	CDataArchiveFilePrefixed();

	// Destruktor
	virtual ~CDataArchiveFilePrefixed();

protected:
	// Vrati rodinku, do ktere archiv patri
	virtual CMappedFile *GetMappedFile() { ASSERT(m_pArchiveFile); return m_pArchiveFile->GetMappedFile(); }

private:
	// Prefix archivu
	CString strPrefix;

	// Odkaz na archive file
	CDataArchiveFile *m_pArchiveFile;
};

#undef CATPATH

#endif // !defined(AFX_DATAARCHIVEFILEPREFIXED_H__96109BD3_1B4F_11D4_805F_0000B48431EC__INCLUDED_)
