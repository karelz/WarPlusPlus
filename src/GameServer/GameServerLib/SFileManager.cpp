/***********************************************************
 * 
 * Projekt: Strategická hra
 *    Èást: Server hry
 *   Autor: Karel Zikmund
 * 
 *   Popis: Správce souborù
 * 
 ***********************************************************/

#include "StdAfx.h"

#include "SFileManager.h"
#include "Common\Map\Map.h"

#define WORKING_SCRIPT_SETS_DIRECTORY		"ScriptSets"
#define WORKING_CIVILIZATIONS_DIRECTORY	"Civilizations"

//////////////////////////////////////////////////////////////////////
// Globální promìnné
//////////////////////////////////////////////////////////////////////

// správce souborù
CSFileManager g_cSFileManager;

//////////////////////////////////////////////////////////////////////
// Konstrukce a destrukce
//////////////////////////////////////////////////////////////////////

// konstruktor
CSFileManager::CSFileManager () 
{
	m_bDataFilesArchive = FALSE;
	m_bWorkingArchive = FALSE;
}

// destruktor
CSFileManager::~CSFileManager () 
{
	ASSERT ( !m_bDataFilesArchive && !m_bWorkingArchive );
	ASSERT ( m_cScriptSetNameArray.GetSize () == 0 );
}

//////////////////////////////////////////////////////////////////////
// Inicializace a znièení dat objektu
//////////////////////////////////////////////////////////////////////

// inicializuje správce souborù nové hry s archivem datových souborù 
//		"cDataFilesArchive" a s pracovním archivem "cWorkingArchive", který musí být 
//		prázdný
// výjimky: CDataArchiveException
void CSFileManager::Create ( CDataArchive cDataFilesArchive, CDataArchive 
	cWorkingArchive ) 
{
	ASSERT ( !m_bDataFilesArchive && !m_bWorkingArchive );
	ASSERT ( m_cScriptSetNameArray.GetSize () == 0 );

	// inicializuje archivy
	m_cDataFilesArchive = cDataFilesArchive;
	m_bDataFilesArchive = TRUE;
	m_cWorkingArchive = cWorkingArchive;
	m_bWorkingArchive = TRUE;

	// vytvoøí adresáø ScriptSetù
	m_cWorkingArchive.MakeDirectory ( WORKING_SCRIPT_SETS_DIRECTORY );
	// vytvoøí adresáø civilizací
	m_cWorkingArchive.MakeDirectory ( WORKING_CIVILIZATIONS_DIRECTORY );
}

// inicializuje správce souborù pro kompilování ScriptSetù s archivem datových 
//		souborù "cDataFilesArchive"
void CSFileManager::Create ( CDataArchive cDataFilesArchive ) 
{
	ASSERT ( !m_bDataFilesArchive && !m_bWorkingArchive );
	ASSERT ( m_cScriptSetNameArray.GetSize () == 0 );

	// inicializuje archiv datových souborù
	m_cDataFilesArchive = cDataFilesArchive;
	m_bDataFilesArchive = TRUE;
}

// znièí správce souborù
void CSFileManager::Delete () 
{
	// znièí pole jmen ScriptSetù
	m_cScriptSetNameArray.RemoveAll ();

	m_bDataFilesArchive = FALSE;
	m_bWorkingArchive = FALSE;

	// zavøe archivy
	if ( m_bDataFilesArchive )
	{
		m_cDataFilesArchive.Close ();
	}
	if ( m_bWorkingArchive )
	{
		m_cWorkingArchive.Close ();
	}
}

//////////////////////////////////////////////////////////////////////
// Informace o datových souborech a pracovních adresáøích
//////////////////////////////////////////////////////////////////////

// otevøe datový soubor "lpcszFileName"
// výjimky: CDataArchiveException
CArchiveFile CSFileManager::GetFile ( LPCTSTR lpcszFileName ) 
{
	ASSERT ( m_bDataFilesArchive );
	// vrátí požadovaný datový soubor
	return m_cDataFilesArchive.CreateFile ( lpcszFileName, CFile::modeRead | 
		CFile::shareDenyWrite );
}

// vrátí pracovní adresáø civilizace jménem "lpcszCivilizationName"
// výjimky: CDataArchiveException
CString CSFileManager::GetCivilizationDirectory ( LPCTSTR lpcszCivilizationName ) 
{
	ASSERT ( m_bWorkingArchive );
	ASSERT ( strlen ( lpcszCivilizationName ) > 0 );

	// jméno pracovního adresáøe civilizace
	CString strCivilizationDirectoryName ( "\\" WORKING_CIVILIZATIONS_DIRECTORY "\\" );
	strCivilizationDirectoryName += lpcszCivilizationName;

	// vytvoøí pracovní adresáø civilizace
	m_cWorkingArchive.MakeDirectory ( strCivilizationDirectoryName );

	// vrátí pracovní adresáø civilizace
	return m_cWorkingArchive.GetArchivePath () + strCivilizationDirectoryName;
}

// vrátí adresáø ScriptSetu jménem "lpcszScriptSetName"
// výjimky: CDataArchiveException, CPersistentLoadException
CString CSFileManager::GetScriptSetDirectory ( LPCTSTR lpcszScriptSetName ) 
{
	ASSERT ( m_bWorkingArchive );
	ASSERT ( strlen ( lpcszScriptSetName ) > 0 );

	// najde jméno ScriptSetu
	for ( int nIndex = m_cScriptSetNameArray.GetSize (); nIndex-- > 0; )
	{
		// zjistí, jedná-li se o hledané jméno ScriptSetu
		if ( m_cScriptSetNameArray.GetAt ( nIndex ) == lpcszScriptSetName )
		{	// jedná se o hledané jméno ScriptSetu
			// vrátí adresáø ScriptSetu
			return m_cWorkingArchive.GetArchivePath () + 
				"\\" WORKING_SCRIPT_SETS_DIRECTORY "\\" + lpcszScriptSetName;
		}
	}
	LOAD_ASSERT ( FALSE );
}

//////////////////////////////////////////////////////////////////////
// Operace s mapou
//////////////////////////////////////////////////////////////////////

// otevøe archiv "cMapArchive" mapy "lpcszMapArchiveName" s èíslem verze mapy 
//		"dwMapVersion" (FALSE=chybný soubor mapy)
BOOL CSFileManager::OpenMap ( CDataArchive &cMapArchive, LPCTSTR lpcszMapArchiveName, 
	DWORD dwMapVersion ) 
{
	// jméno archivu mapy
	CString strMapArchiveName ( lpcszMapArchiveName );

	try
	{
		// pokusí se otevøít archiv mapy
		if ( !cMapArchive.Create ( strMapArchiveName, CFile::modeRead | 
			CFile::shareDenyWrite, CDataArchiveInfo::archiveFile ) )
		{	// nepodaøilo se otevøít archiv mapy
			// vrátí pøíznak chybného souboru mapy
			return FALSE;
		}
	}
	catch ( CException * )
	{
		// vrátí pøíznak chybného souboru mapy
		return FALSE;
	}

	try
	{
		// pokusí se otevøít soubor mapy
		CArchiveFile cMapFile = cMapArchive.CreateFile ( MAP_FILE_NAME, CFile::modeRead | 
			CFile::shareDenyWrite );

	// naète hlavièku verze souboru
		{
			SFileVersionHeader sFileVersionHeader;

			// pokusí se naèíst hlavièku verze souboru
			if ( cMapFile.Read ( &sFileVersionHeader, sizeof ( sFileVersionHeader ) ) != 
				sizeof ( sFileVersionHeader ) )
			{	// nepodaøilo se naèíst hlavièku
				// vrátí pøíznak chybného souboru mapy
				return FALSE;
			}

			// identifikátor souboru mapy
			BYTE aMapFileID[16] = MAP_FILE_IDENTIFIER;
			ASSERT ( sizeof ( aMapFileID ) == sizeof ( sFileVersionHeader.m_aMapFileID ) );

			// zkontroluje identifikátor souboru mapy
			for ( int nIndex = 16; nIndex-- > 0; )
			{
				if ( aMapFileID[nIndex] != sFileVersionHeader.m_aMapFileID[nIndex] )
				{
					// vrátí pøíznak chybného souboru mapy
					return FALSE;
				}
			}

			ASSERT ( CURRENT_MAP_FILE_VERSION >= COMPATIBLE_MAP_FILE_VERSION );
			// naète èísla verzí formátu mapy
			DWORD dwMapFormatVersion = sFileVersionHeader.m_dwFormatVersion;
			DWORD dwMapCompatibleFormatVersion = 
				sFileVersionHeader.m_dwCompatibleFormatVersion;
			// zkontroluje èísla verzí formátu mapy
			if ( dwMapFormatVersion < dwMapCompatibleFormatVersion )
			{	// èísla verzí formátu mapy jsou chybná
				// vrátí pøíznak chybného souboru mapy
				return FALSE;
			}

			// zjistí, jedná-li se o správnou verzi formátu mapy
			if ( dwMapFormatVersion != CURRENT_MAP_FILE_VERSION )
			{	// nejedná se o správnou verzi formátu mapy
				// zjistí, jedná-li se o starou verzi formátu mapy
				if ( dwMapFormatVersion < CURRENT_MAP_FILE_VERSION )
				{	// jedná se o starou verzi formátu mapy
					// zkontroluje kompatabilitu verze formátu mapy
					if ( dwMapFormatVersion < COMPATIBLE_MAP_FILE_VERSION )
					{
						// vrátí pøíznak chybného souboru mapy
						return FALSE;
					}
				}
				else
				{	// jedná se o mladší verzi formátu mapy
					// zkontroluje kompatabilitu verze formátu mapy
					if ( dwMapCompatibleFormatVersion > CURRENT_MAP_FILE_VERSION )
					{
						// vrátí pøíznak chybného souboru mapy
						return FALSE;
					}
				}
			}
		}

	// naète hlavièku mapy
		{
			SMapHeader sMapHeader;

			// pokusí se naèíst hlavièku mapy
			if ( cMapFile.Read ( &sMapHeader, sizeof ( sMapHeader ) ) != 
				sizeof ( sMapHeader ) )
			{	// nepodaøilo se naèíst hlavièku mapy
				// vrátí pøíznak chybného souboru mapy
				return FALSE;
			}

			// zkontroluje verzi mapy
			if ( sMapHeader.m_dwMapVersion != dwMapVersion )
			{	// nejedná se o požadovanou verzi mapy
				// vrátí pøíznak chybného souboru mapy
				return FALSE;
			}
		}

		// zavøe soubor mapy
		cMapFile.Close ();
	}
	catch ( CException * )
	{
		// zavøe archiv mapy
		cMapArchive.Close ();

		// vrátí pøíznak chybného souboru mapy
		return FALSE;
	}

	// vrátí pøíznak úspìšného naètení souboru mapy
	return TRUE;
}

// nahraje z archivu mapy "cMapArchive" ScriptSet "strScriptSetName"
// výjimky: CDataArchiveException
void CSFileManager::LoadScriptSet ( CDataArchive cMapArchive, 
	CString &strScriptSetName ) 
{
	ASSERT ( m_bWorkingArchive );

	// jméno pracovního adresáøe ScriptSetu
	CString strWorkingScriptSetDirectoryName ( WORKING_SCRIPT_SETS_DIRECTORY "\\" );
	strWorkingScriptSetDirectoryName += strScriptSetName;
	// jméno zdrojového adresáøe ScriptSetu
	CString strSourceScriptSetDirectoryName ( SCRIPT_SETS_DIRECTORY "\\" );
	strSourceScriptSetDirectoryName += strScriptSetName;

	// vytvoøí pracovní adresáø ScriptSetu
	m_cWorkingArchive.MakeDirectory ( strWorkingScriptSetDirectoryName );

	// vykopíruje ScriptSet do pracovního archivu
	m_cWorkingArchive.AppendDir ( strWorkingScriptSetDirectoryName, cMapArchive, 
		strSourceScriptSetDirectoryName, (EAppendModes)( appendRecursive | 
		appendUncompressed ) );

	// pøidá jméno ScriptSetu do pole jmen ScriptSetù
	(void)m_cScriptSetNameArray.Add ( strScriptSetName );
}
