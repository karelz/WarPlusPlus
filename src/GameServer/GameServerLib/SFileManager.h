/***********************************************************
 * 
 * Projekt: Strategická hra
 *    Èást: Server hry
 *   Autor: Karel Zikmund
 * 
 *   Popis: Správce souborù
 * 
 ***********************************************************/

#ifndef __SERVER_FILE_MANAGER__HEADER_INCLUDED__
#define __SERVER_FILE_MANAGER__HEADER_INCLUDED__

#include <afxcoll.h>

#include "Archive\Archive\Archive.h"

//////////////////////////////////////////////////////////////////////
// Dopøedné deklarace

// tøída mapy
class CSMap;

//////////////////////////////////////////////////////////////////////
// Tøída správce souborù na serveru hry
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

// Inicializace a znièení dat objektu

	// inicializuje správce souborù nové hry s archivem datových souborù 
	//		"cDataFilesArchive" a s pracovním archivem "cWorkingArchive", který musí být 
	//		prázdný
	// výjimky: CDataArchiveException
	void Create ( CDataArchive cDataFilesArchive, CDataArchive cWorkingArchive );
	// inicializuje správce souborù pro kompilování ScriptSetù s archivem datových 
	//		souborù "cDataFilesArchive"
	void Create ( CDataArchive cDataFilesArchive );
	// znièí správce souborù
	void Delete ();

// Informace o datových souborech a pracovních adresáøích

	// otevøe datový soubor "lpcszFileName"
	// výjimky: CDataArchiveException
	CArchiveFile GetFile ( LPCTSTR lpcszFileName );
	// vrátí pracovní adresáø civilizace jménem "lpcszCivilizationName"
	// výjimky: CDataArchiveException
	CString GetCivilizationDirectory ( LPCTSTR lpcszCivilizationName );
	// vrátí adresáø ScriptSetu jménem "lpcszScriptSetName"
	// výjimky: CDataArchiveException, CPersistentLoadException
	CString GetScriptSetDirectory ( LPCTSTR lpcszScriptSetName );

protected:
// Operace s mapou

	// otevøe archiv "cMapArchive" mapy "lpcszMapArchiveName" s èíslem verze mapy 
	//		"dwMapVersion" (FALSE=chybný soubor mapy)
	BOOL OpenMap ( CDataArchive &cMapArchive, LPCTSTR lpcszMapArchiveName, 
		DWORD dwMapVersion );
	// nahraje z archivu mapy "cMapArchive" ScriptSet "strScriptSetName"
	// výjimky: CDataArchiveException
	void LoadScriptSet ( CDataArchive cMapArchive, CString &strScriptSetName );

// Data
private:
	// archiv datových souborù
	CDataArchive m_cDataFilesArchive;
	// pracovní archiv
	CDataArchive m_cWorkingArchive;
	// pøíznak platnosti archivu datových souborù
	BOOL m_bDataFilesArchive;
	// pøíznak platnosti pracovního archivu
	BOOL m_bWorkingArchive;

	// pole jmen ScriptSetù
	CStringArray m_cScriptSetNameArray;
};

//////////////////////////////////////////////////////////////////////
// Globální promìnné

// správce souborù
extern CSFileManager g_cSFileManager;

#endif //__SERVER_FILE_MANAGER__HEADER_INCLUDED__
