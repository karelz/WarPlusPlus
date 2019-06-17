/***********************************************************
 * 
 * Projekt: Strategick� hra
 *    ��st: Server hry
 *   Autor: Karel Zikmund
 * 
 *   Popis: Spr�vce soubor�
 * 
 ***********************************************************/

#ifndef __SERVER_FILE_MANAGER__HEADER_INCLUDED__
#define __SERVER_FILE_MANAGER__HEADER_INCLUDED__

#include <afxcoll.h>

#include "Archive\Archive\Archive.h"

//////////////////////////////////////////////////////////////////////
// Dop�edn� deklarace

// t��da mapy
class CSMap;

//////////////////////////////////////////////////////////////////////
// T��da spr�vce soubor� na serveru hry
class CSFileManager 
{
	friend class CSMap;

// Metody
public:
// Konstrukce a destrukce

	// konstruktor
	CSFileManager ();
	// destruktor
	~CSFileManager ();

// Inicializace a zni�en� dat objektu

	// inicializuje spr�vce soubor� nov� hry s archivem datov�ch soubor� 
	//		"cDataFilesArchive" a s pracovn�m archivem "cWorkingArchive", kter� mus� b�t 
	//		pr�zdn�
	// v�jimky: CDataArchiveException
	void Create ( CDataArchive cDataFilesArchive, CDataArchive cWorkingArchive );
	// inicializuje spr�vce soubor� pro kompilov�n� ScriptSet� s archivem datov�ch 
	//		soubor� "cDataFilesArchive"
	void Create ( CDataArchive cDataFilesArchive );
	// zni�� spr�vce soubor�
	void Delete ();

// Informace o datov�ch souborech a pracovn�ch adres���ch

	// otev�e datov� soubor "lpcszFileName"
	// v�jimky: CDataArchiveException
	CArchiveFile GetFile ( LPCTSTR lpcszFileName );
	// vr�t� pracovn� adres�� civilizace jm�nem "lpcszCivilizationName"
	// v�jimky: CDataArchiveException
	CString GetCivilizationDirectory ( LPCTSTR lpcszCivilizationName );
	// vr�t� adres�� ScriptSetu jm�nem "lpcszScriptSetName"
	// v�jimky: CDataArchiveException, CPersistentLoadException
	CString GetScriptSetDirectory ( LPCTSTR lpcszScriptSetName );

protected:
// Operace s mapou

	// otev�e archiv "cMapArchive" mapy "lpcszMapArchiveName" s ��slem verze mapy 
	//		"dwMapVersion" (FALSE=chybn� soubor mapy)
	BOOL OpenMap ( CDataArchive &cMapArchive, LPCTSTR lpcszMapArchiveName, 
		DWORD dwMapVersion );
	// nahraje z archivu mapy "cMapArchive" ScriptSet "strScriptSetName"
	// v�jimky: CDataArchiveException
	void LoadScriptSet ( CDataArchive cMapArchive, CString &strScriptSetName );

// Data
private:
	// archiv datov�ch soubor�
	CDataArchive m_cDataFilesArchive;
	// pracovn� archiv
	CDataArchive m_cWorkingArchive;
	// p��znak platnosti archivu datov�ch soubor�
	BOOL m_bDataFilesArchive;
	// p��znak platnosti pracovn�ho archivu
	BOOL m_bWorkingArchive;

	// pole jmen ScriptSet�
	CStringArray m_cScriptSetNameArray;
};

//////////////////////////////////////////////////////////////////////
// Glob�ln� prom�nn�

// spr�vce soubor�
extern CSFileManager g_cSFileManager;

#endif //__SERVER_FILE_MANAGER__HEADER_INCLUDED__
