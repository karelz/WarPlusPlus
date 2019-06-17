// DataArchive.h: interface for the CDataArchive class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_DATAARCHIVE_H__DBBEBA55_0135_11D3_8DA0_00E0290A6C43__INCLUDED_)
#define AFX_DATAARCHIVE_H__DBBEBA55_0135_11D3_8DA0_00E0290A6C43__INCLUDED_

#include "DataArchiveInfo.h"	// Added by ClassView
#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include <afx.h>
#include "ArchiveFile.h"	// Added by ClassView
#include "DataArchiveException.h" // Tohle je potreba vsude, proc to nedat do hlavicky, ze?

#ifndef SUCCESS
#define SUCCESS	TRUE
#endif
#ifndef FAILURE
#define FAILURE FALSE
#endif

class CDataArchive : public CObject  
{
friend class CDataArchiveFile; // Ten muze cist moje m_pDataArchiveInfo

// Staticke metody
public:
	// Inicializace promennych tridy
	static void Initialize(LPCTSTR lpcszTemporaryDirectory);

	// Pouzit pri niceni tridy
	static void Done();

	// Maze adresar i s obsahem
	static void RemoveDirectoryWithContents(CString strDirName);

	// Vraci nove jmeno souboru v ramci temporary archivu
	static CString GetNewTemporaryName();

	// Vraci nove jmeno souboru s plnou cestou k temporary archivu
	static CString GetNewTemporaryPath() {
		return m_strArchiveTemp + '\\' + GetNewTemporaryName();
	}

public:
	// Vytvoreni prazdneho, nepouzitelneho archivu
	virtual BOOL Create(void);

	// Vytvoreni read-write souboroveho archivu
	virtual BOOL Create(LPCTSTR lpcszFileName) { return Create(lpcszFileName, CArchiveFile::modeRead, CDataArchiveInfo::archiveFile); }

	// Vytvoreni archivu v souboru/adresari daneho jmena
	virtual BOOL Create(LPCTSTR lpcszFileName, DWORD dwFlags, DWORD dwArchiveType);

	// Zruseni archivu
	virtual BOOL Delete();

	// Otevreni archivu
	void Open() { ASSERT(m_pArchiveInfo); m_pArchiveInfo->Open(); }

	// Zavreni archivu
	void Close() { ASSERT(m_pArchiveInfo); m_pArchiveInfo->Close();}

	// Prekopani archivu pro rychlejsi pristup
	void Rebuild() { ASSERT(m_pArchiveInfo); m_pArchiveInfo->Rebuild(); }

	// Otvirani souboru v archivu
    CArchiveFile CreateFile(LPCTSTR lpcszFileName, DWORD dwFlags=CArchiveFile::modeRead | CFile::shareDenyWrite) { ASSERT(m_pArchiveInfo); return m_pArchiveInfo->CreateFile(lpcszFileName, dwFlags); }

	// Otevreni podarchivu v archivu (ma stejne flagy a pristupova prava, jako cely archiv)
	// Specialne se da zadat priznak pro vytvareni
	CDataArchive CreateArchive(LPCTSTR lpcszPath, BOOL bCreate=FALSE) { ASSERT(m_pArchiveInfo); return m_pArchiveInfo->CreateArchive(lpcszPath, bCreate); }

	// Pridani externiho souboru do archivu
	void AppendFile(LPCTSTR lpcszFileName, LPCTSTR lpcszFilePath="", DWORD dwFlags=0) { ASSERT(m_pArchiveInfo); m_pArchiveInfo->AppendFile(lpcszFileName, lpcszFilePath, dwFlags); }

	// Pridani souboru z archivu do archivu
	void AppendFile(CArchiveFile file, LPCTSTR lpcszFilePath="", DWORD dwFlags=appendAuto) { ASSERT(m_pArchiveInfo); m_pArchiveInfo->AppendFile(file, lpcszFilePath, dwFlags); }

	// Pridani celeho podadresare lpcszDirPathFrom (rekurzivne) z archivu ArchiveFrom do adresare lpcszDirPathTo aktualniho archivu
	void AppendDir(LPCTSTR lpcszDirPathTo, CDataArchive ArchiveFrom, LPCTSTR lpcszDirPathFrom, EAppendModes nFlags=appendRecursive) { ASSERT(m_pArchiveInfo); m_pArchiveInfo->AppendDir(lpcszDirPathTo, ArchiveFrom, lpcszDirPathFrom, nFlags); }

	// Pridani celeho podadresare lpcszDirPathFrom (rekurzivne) z archivu ArchiveFrom do adresare lpcszDirPathTo aktualniho archivu
	// Vhodne pro volani v destruktoru archivu, na kterem se pracuje - nezveda reference
	void AppendDirNoRef(LPCTSTR lpcszDirPathTo, CDataArchiveInfo *pArchiveFrom, LPCTSTR lpcszDirPathFrom, EAppendModes nFlags=appendRecursive) { ASSERT(m_pArchiveInfo); m_pArchiveInfo->AppendDirNoRef(lpcszDirPathTo, pArchiveFrom, lpcszDirPathFrom, nFlags); }

	// Extrahovani souboru
	void ExtractFile(LPCTSTR lpcszFileName, LPCTSTR lpcszFilePath="") { ASSERT(m_pArchiveInfo); m_pArchiveInfo->ExtractFile(lpcszFileName, lpcszFilePath); }

	// Odstraneni souboru z archivu
	void RemoveFile(LPCTSTR lpcszFileName) { ASSERT(m_pArchiveInfo); m_pArchiveInfo->RemoveFile(lpcszFileName); }

	// Vytvoreni adresare
	void MakeDirectory(LPCTSTR lpcszDirectoryName) { ASSERT(m_pArchiveInfo); m_pArchiveInfo->MakeDirectory(lpcszDirectoryName); }

	// Zruseni adresare
	void RemoveDirectory(LPCTSTR lpcszDirectoryName) { ASSERT(m_pArchiveInfo); m_pArchiveInfo->RemoveDirectory(lpcszDirectoryName); }

	// Vraci jmeno a cestu k archivu
	CString GetArchivePath() {  ASSERT(m_pArchiveInfo); return m_pArchiveInfo->GetArchivePath(); }

	// Vraci, zdali je archiv otevreny
	BOOL IsOpened() { if(!m_pArchiveInfo) return FALSE; return m_pArchiveInfo->IsOpened(); }

	// ZATIM NEPOUZIVEJTE!
	CDataArchiveDirContents *GetDirContents(CString strPath) {  ASSERT(m_pArchiveInfo); return m_pArchiveInfo->GetDirContents(strPath); }

	// Vytvori klon archivu pro dalsi thread
	CDataArchive CreateArchiveClone() { ASSERT(m_pArchiveInfo); return CDataArchive(m_pArchiveInfo->CreateArchiveInfoClone()); }

	// Adoptuje soubor
	void AdoptFile(CArchiveFile &file) { ASSERT(m_pArchiveInfo); m_pArchiveInfo->AdoptFile(file); }
	
	// Diagnostic support		
#ifdef _DEBUG
	virtual void AssertValid(void) const;	
	virtual void Dump(CDumpContext& dc) const;
#endif

	// Konstruktor
	CDataArchive();

	// Konstruktor s okamzitym otevrenim ve souboru
	CDataArchive(LPCTSTR lpcszFileName);

	// Konstruktor s okamzitym otevrenim ve souboru
	CDataArchive(LPCTSTR lpcszFileName, DWORD dwFlags, DWORD dwArchiveType);

	// Copy konstruktor
	CDataArchive(const CDataArchive& DataArchive);

	// Operator =
	CDataArchive& operator =(const CDataArchive & DataArchive);

	// Konstruktor obalovaci (NEPOUZIVEJTE)
	CDataArchive(CDataArchiveInfo *pDataArchiveInfo);

	// Destruktor
	virtual ~CDataArchive();

	// Vraci ukazatel na temporary archiv
	static CDataArchive *GetTempArchive();

	// Vraci ukazatel na globalni archiv
	static CDataArchive *GetRootArchive();

private:
	// Volat pokazde, kdyz se zrusi archiv
	static void DecreaseNumberOfArchives();

	// Archiv pro temporary adresar
	static CDataArchive *m_pTemporaryArchive;

	// Globalni archiv
	static CDataArchive *m_pGlobalArchive;

	// Priznak, zda byl temporary adresar rucne vytvoren
	static BOOL m_bTempCreated;

	// Cislo v ramci 'session' pro vytvareni temporary jmen
	static DWORD m_dwSessionNr;

	// Operator = pro CDataArchiveInfo
	CDataArchive& operator =(CDataArchiveInfo *pDataArchiveInfo);
	
	// Ukazatel na informace o Archivu - letter
	CDataArchiveInfo* m_pArchiveInfo;

	// Drzi si jeden globalni ukazatel na prazdny archiv
	static CDataArchiveInfo* m_pEmptyArchiveInfo;

	// Temporary adresar pro vsechny archivy
	static CString m_strArchiveTemp;

	// Pocet existujicich odkazu na archivy (zvyseny o jedna, pokud jsme mezi Init a Done)
	static DWORD m_dwNumDataArchives;

	// Bylo jiz zavolano Initialize?
	static BOOL m_bInitializeWasCalled;

	// Bylo jiz zavolano Done?
	static BOOL m_bDoneWasCalled;
};

#endif // !defined(AFX_DATAARCHIVE_H__DBBEBA55_0135_11D3_8DA0_00E0290A6C43__INCLUDED_)
