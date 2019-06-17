// DataArchiveFileDirectoryInfo.h: interface for the CDataArchiveFileDirectoryInfo class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_DATAARCHIVEFILEDIRECTORYINFO_H__56FB0E05_8AD7_11D3_8964_00C04F513C03__INCLUDED_)
#define AFX_DATAARCHIVEFILEDIRECTORYINFO_H__56FB0E05_8AD7_11D3_8964_00C04F513C03__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <afxtempl.h>
#include "Definitions.h"
#include "DataArchiveFileDataList.h"
#include "DataArchiveFileDirectoryItem.h"

class CDataArchiveFileDirectory;
class CDataArchiveInfo;

#pragma pack(1) // Struktura je zarovnana na bajty

// Hlavicka adresare
struct SDataArchiveFileDirectoryHeader {
	int nFiles; // Pocet souboru v adresari (vcetne . a ..)
	DWORD dwDataOffset; // Offset seznamu bloku dat pro adresar
};

#pragma pack() // Konec zarovnani pro strukturu

class CDataArchiveFileDirectoryInfo : public CObject
{
public:
	// Konstruktor
	CDataArchiveFileDirectoryInfo();

	// Destruktor
	virtual ~CDataArchiveFileDirectoryInfo();

	// Nacteni dat z disku
	void Open(CMappedFile &File, DWORD dwOffset);

	// Uzavreni adresare
	void Close();

	// Flushnuti zmen
	void Flush();

	// Informace o souboru. Vraci NULL pokud soubor neni v tomto adresari
	CDataArchiveFileDirectoryItem *FileInfo(CString strName);

	// Vytvori root adresar v danem CMappedFile na danem offsetu - alternativa k Open
	void CreateRoot(CMappedFile &File, DWORD dwOffset);

	// Vytvori novy soubor (strFileName je jmeno v ramci adresare, strFullName je plna cesta)
	CArchiveFile CreateFile(CString strFileName, CString strFullName, CDataArchiveInfo *pDataArchiveInfo, DWORD dwFlags) {
		return CreateFile(strFileName, strFullName, pDataArchiveInfo, dwFlags, DATA_ARCHIVE_FILE_FILE_INITIAL_DATA_SIZE);
	}

	// Vytvori novy soubor dane velikosti
	CArchiveFile CreateFile(CString strFileName, CString strFullName, CDataArchiveInfo *pDataArchiveInfo, DWORD dwFlags, DWORD dwInitialSize, BOOL bGonnaFill=FALSE);

	// Vytvori novy adresar
	void CreateDirectory(CString strDirName);
	
	// Prida novy soubor z disku
	void AppendFile(CString strFileName);

	// Smaze soubor z archivu
	void RemoveFile(CString strFileName);

	// Smaze adresar z archivu
	void RemoveDirectory(CString strDirName);

	// Vrati presnou velikost, jakou bude adresar zabirat na disku
	DWORD GetStorageSize();

	// Vrati pocet souboru v adresari (nesmazanych)
	int GetNumFiles();

	// Vrati info o svem obsahu
	CDataArchiveDirContents *GetDirContents();

	// Vrati TRUE, pokud je adresar prazdny
	BOOL IsEmpty() { return GetNumFiles()==2; }

//
	// Odpoji info od letteru
	void DetachFromLetter(CDataArchiveFileDirectory *pLetter);

	// Propoji info k letteru
	void AttachToLetter(CDataArchiveFileDirectory *pLetter);

	// Vrati 'cas' posledniho pristupu
	DWORD GetLastAccessTime() { return m_dwLastAccessTime; }

	// Nastavi 'cas' posledniho pristupu
	void SetLastAccessTime(DWORD dwLastAccessTime) { m_dwLastAccessTime=dwLastAccessTime; }

	// Nastavi priznak zmeny
	void Touch() { m_bModified=TRUE; };

private:
	// Zapise do tabulky
	void SetAt(CString strName, CDataArchiveFileDirectoryItem *pItem);

private:
	// Pozice hlavicky adresare
	DWORD m_dwHeaderOffset;
	
	// Data hlavicky adresare
	SDataArchiveFileDirectoryHeader m_DirHeader;

	// Tabulka souboru
	CMapStringToPtr m_FileTable;

	// Data o adresari na disku
	CDataArchiveFileDataList m_FileList;

	// Reference count
	int m_nPointerCount;

	// 'Cas' posledniho pristupu do adresare
	DWORD m_dwLastAccessTime;

	// Priznak, ze doslo k modifikaci od posledniho flushnuti
	BOOL m_bModified;
};

#endif // !defined(AFX_DATAARCHIVEFILEDIRECTORYINFO_H__56FB0E05_8AD7_11D3_8964_00C04F513C03__INCLUDED_)
