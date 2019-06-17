// DataArchiveFileManager.h: interface for the CDataArchiveFileManager class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_DATAARCHIVEFILEMANAGER_H__25B75556_92B8_11D3_BF67_AE14368AF507__INCLUDED_)
#define AFX_DATAARCHIVEFILEMANAGER_H__25B75556_92B8_11D3_BF67_AE14368AF507__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "DataArchive.h"

class CDataArchiveFileManager : public CObject  
{
public:
	// Konstruktor
	CDataArchiveFileManager();

	// Destruktor
	virtual ~CDataArchiveFileManager();

	// Vypis hlasky o pouzivani
	void PrintUsage();

	// Preparsovani command line optionu a provedeni jich
	int Parse(int argc, TCHAR *argv[]);

	// Vypise obsah archivu
	void PrintContents(CDataArchive archive, CString strPath);

protected:
	// Prida objekt na zadane ceste do archivu
	void Append(CString strPath, CString strWhere);

	// Smaze objekt na zadanem miste v archivu
	void Delete(CString strPath);

	// Extractne objekt z archivu
	void Extract(CString strPath, CString strWhere);

	// Rebuildne archiv
	void Rebuild();

private:
	// Vraci true, pokud na disku existuje soubor daneho jmena
	BOOL FileExists(CString strFileName);

	// Rozseka cestu na cestu a jmeno souboru
	void SplitPath(CString &strPath, CString &strDir, CString &strFile);

private:
	// Prikaz, ktery jsme dostali
	char m_cCommand;

	// Archiv, se kterym se pracuje
	CDataArchive m_Archive;
	
	// Recurse subdirectories
	BOOL m_bRecurse;

	// Cesta pro extrahovane soubory
	CString m_strPath;
};

#endif // !defined(AFX_DATAARCHIVEFILEMANAGER_H__25B75556_92B8_11D3_BF67_AE14368AF507__INCLUDED_)
