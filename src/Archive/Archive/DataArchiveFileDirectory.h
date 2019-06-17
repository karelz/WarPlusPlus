// DataArchiveFileDirectory.h: interface for the CDataArchiveFileDirectory class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_DATAARCHIVEFILEDIRECTORY_H__56FB0E06_8AD7_11D3_8964_00C04F513C03__INCLUDED_)
#define AFX_DATAARCHIVEFILEDIRECTORY_H__56FB0E06_8AD7_11D3_8964_00C04F513C03__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <afxtempl.h>
#include "Definitions.h"
#include "DataArchiveFileDataList.h"
#include "DataArchiveFileDirectoryItem.h"
#include "DataArchiveFileDirectoryInfo.h"
#include "DataArchiveDirContents.h"

class CDataArchiveFileDirectory : public CObject  
{
public:
	// Konstruktor
	CDataArchiveFileDirectory() { 
		m_pDirInfo=(CDataArchiveFileDirectoryInfo *)NULL; 
	}

	// Copy konstruktor
	CDataArchiveFileDirectory(CDataArchiveFileDirectory &CopyFrom);

	// Destruktor
	virtual ~CDataArchiveFileDirectory();

	// Nacteni dat z disku
	void Open(CMappedFile &File, DWORD dwOffset);

	// Uzavreni adresare
	void Close() {
		ASSERT(m_pDirInfo);
		m_pDirInfo->Close();
	}

	// Flushnuti zmen
	void Flush() {
		ASSERT(m_pDirInfo);
		m_pDirInfo->Flush();
	}

	// Informace o souboru. Vraci NULL pokud soubor neni v tomto adresari
	CDataArchiveFileDirectoryItem *FileInfo(CString strName) {
		ASSERT(m_pDirInfo);
		return m_pDirInfo->FileInfo(strName);
	}

	// Vytvori root adresar v danem CMappedFile na danem offsetu - alternativa k Open
	void CreateRoot(CMappedFile &File, DWORD dwOffset);

	// Vytvori novy soubor
	CArchiveFile CreateFile(CString strFileName, CString strFullName, CDataArchiveInfo *pDataArchiveInfo, DWORD dwFlags) {
		ASSERT(m_pDirInfo);
		return m_pDirInfo->CreateFile(strFileName, strFullName, pDataArchiveInfo, dwFlags);
	}

	// Vytvori novy soubor zadane velikosti
	CArchiveFile CreateFile(CString strFileName, CString strFullName, CDataArchiveInfo *pDataArchiveInfo, DWORD dwFlags, DWORD dwInitialSize, BOOL bGonnaFill=FALSE) {
		ASSERT(m_pDirInfo);
		return m_pDirInfo->CreateFile(strFileName, strFullName, pDataArchiveInfo, dwFlags, dwInitialSize, bGonnaFill);
	}

	// Vytvori novy adresar
	void CreateDirectory(CString strDirName) {
		ASSERT(m_pDirInfo);
		m_pDirInfo->CreateDirectory(strDirName);
	}
	
	// Prida novy soubor z disku
	void AppendFile(CString strFileName) {
		ASSERT(m_pDirInfo);
		m_pDirInfo->AppendFile(strFileName);
	}

	// Smaze soubor z archivu
	void RemoveFile(CString strFileName) {
		ASSERT(m_pDirInfo);
		m_pDirInfo->RemoveFile(strFileName);
	}

	// Smaze adresar z archivu
	void RemoveDirectory(CString strDirName) {
		ASSERT(m_pDirInfo);
		m_pDirInfo->RemoveDirectory(strDirName);
	}

	// Vrati presnou velikost, jakou bude adresar zabirat na disku
	DWORD GetStorageSize() { ASSERT(m_pDirInfo); return m_pDirInfo->GetStorageSize(); }

	// Vrati pocet nesmazanych polozek adresare
	DWORD GetNumFiles() { ASSERT(m_pDirInfo); return m_pDirInfo->GetNumFiles(); }

	// Vrati info o svem obsahu
	CDataArchiveDirContents *GetDirContents() { ASSERT(m_pDirInfo); return m_pDirInfo->GetDirContents(); }
	
	// Vrati 'cas' posledniho pristupu
	DWORD GetLastAccessTime() { ASSERT(m_pDirInfo); return m_pDirInfo->GetLastAccessTime(); }

	// Nastavi 'cas' posledniho pristupu
	void SetLastAccessTime(DWORD dwLastAccessTime) { ASSERT(m_pDirInfo); m_pDirInfo->SetLastAccessTime(dwLastAccessTime); }

	// Test na prazdnost adresare
	BOOL IsEmpty() { ASSERT(m_pDirInfo); return m_pDirInfo->IsEmpty(); }

	// Operator =
	CDataArchiveFileDirectory& operator =(const CDataArchiveFileDirectory & Directory)
	{
		// Prirazuji sam do sebe
		if(((CDataArchiveFileDirectory*)&Directory)==this) return *this;

		(*this)=Directory.m_pDirInfo; // Priradime archive info

		return *this;
	}

	// Operator = pro nastaveni m_pArchiveInfo
	CDataArchiveFileDirectory& operator =(CDataArchiveFileDirectoryInfo *pDirectoryInfo)
	{
		// Prirazuji sam do sebe
		if(pDirectoryInfo==m_pDirInfo) return *this;

		// Byli jsme pripojeni k objektu CDataArchiveInfo.
		if(m_pDirInfo) {
			m_pDirInfo->DetachFromLetter(this); // Odpojime se
			m_pDirInfo=(CDataArchiveFileDirectoryInfo*)NULL; // A vynulujeme odkaz
		}

		ASSERT(!m_pDirInfo);
		
		// Zkopiruji se informace o souboru
		m_pDirInfo=pDirectoryInfo;

		// DirInfo musi byt vyplnene, aby se s nim neco delalo
		// Jestli neni - patologicky pripad, kopiruji se dva nevyplnene archivy 
		if(m_pDirInfo) {
			m_pDirInfo->AttachToLetter(this);
		}
		return *this;
	}

	// Nastavi priznak zmeny
	void Touch() { ASSERT(m_pDirInfo); m_pDirInfo->Touch(); };

	BOOL operator==(CDataArchiveFileDirectory &Directory) {
		return Directory.m_pDirInfo==m_pDirInfo;
	}

	BOOL operator!=(CDataArchiveFileDirectory &Directory) {
		return !(*this==Directory);
	}

private:
	// Odkaz na 'letter'
	CDataArchiveFileDirectoryInfo *m_pDirInfo;
};

#endif // !defined(AFX_DATAARCHIVEFILEDIRECTORY_H__56FB0E06_8AD7_11D3_8964_00C04F513C03__INCLUDED_)
