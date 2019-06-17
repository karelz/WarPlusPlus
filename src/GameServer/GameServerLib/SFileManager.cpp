/***********************************************************
 * 
 * Projekt: Strategick� hra
 *    ��st: Server hry
 *   Autor: Karel Zikmund
 * 
 *   Popis: Spr�vce soubor�
 * 
 ***********************************************************/

#include "StdAfx.h"

#include "SFileManager.h"
#include "Common\Map\Map.h"

#define WORKING_SCRIPT_SETS_DIRECTORY		"ScriptSets"
#define WORKING_CIVILIZATIONS_DIRECTORY	"Civilizations"

//////////////////////////////////////////////////////////////////////
// Glob�ln� prom�nn�
//////////////////////////////////////////////////////////////////////

// spr�vce soubor�
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
// Inicializace a zni�en� dat objektu
//////////////////////////////////////////////////////////////////////

// inicializuje spr�vce soubor� nov� hry s archivem datov�ch soubor� 
//		"cDataFilesArchive" a s pracovn�m archivem "cWorkingArchive", kter� mus� b�t 
//		pr�zdn�
// v�jimky: CDataArchiveException
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

	// vytvo�� adres�� ScriptSet�
	m_cWorkingArchive.MakeDirectory ( WORKING_SCRIPT_SETS_DIRECTORY );
	// vytvo�� adres�� civilizac�
	m_cWorkingArchive.MakeDirectory ( WORKING_CIVILIZATIONS_DIRECTORY );
}

// inicializuje spr�vce soubor� pro kompilov�n� ScriptSet� s archivem datov�ch 
//		soubor� "cDataFilesArchive"
void CSFileManager::Create ( CDataArchive cDataFilesArchive ) 
{
	ASSERT ( !m_bDataFilesArchive && !m_bWorkingArchive );
	ASSERT ( m_cScriptSetNameArray.GetSize () == 0 );

	// inicializuje archiv datov�ch soubor�
	m_cDataFilesArchive = cDataFilesArchive;
	m_bDataFilesArchive = TRUE;
}

// zni�� spr�vce soubor�
void CSFileManager::Delete () 
{
	// zni�� pole jmen ScriptSet�
	m_cScriptSetNameArray.RemoveAll ();

	m_bDataFilesArchive = FALSE;
	m_bWorkingArchive = FALSE;

	// zav�e archivy
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
// Informace o datov�ch souborech a pracovn�ch adres���ch
//////////////////////////////////////////////////////////////////////

// otev�e datov� soubor "lpcszFileName"
// v�jimky: CDataArchiveException
CArchiveFile CSFileManager::GetFile ( LPCTSTR lpcszFileName ) 
{
	ASSERT ( m_bDataFilesArchive );
	// vr�t� po�adovan� datov� soubor
	return m_cDataFilesArchive.CreateFile ( lpcszFileName, CFile::modeRead | 
		CFile::shareDenyWrite );
}

// vr�t� pracovn� adres�� civilizace jm�nem "lpcszCivilizationName"
// v�jimky: CDataArchiveException
CString CSFileManager::GetCivilizationDirectory ( LPCTSTR lpcszCivilizationName ) 
{
	ASSERT ( m_bWorkingArchive );
	ASSERT ( strlen ( lpcszCivilizationName ) > 0 );

	// jm�no pracovn�ho adres��e civilizace
	CString strCivilizationDirectoryName ( "\\" WORKING_CIVILIZATIONS_DIRECTORY "\\" );
	strCivilizationDirectoryName += lpcszCivilizationName;

	// vytvo�� pracovn� adres�� civilizace
	m_cWorkingArchive.MakeDirectory ( strCivilizationDirectoryName );

	// vr�t� pracovn� adres�� civilizace
	return m_cWorkingArchive.GetArchivePath () + strCivilizationDirectoryName;
}

// vr�t� adres�� ScriptSetu jm�nem "lpcszScriptSetName"
// v�jimky: CDataArchiveException, CPersistentLoadException
CString CSFileManager::GetScriptSetDirectory ( LPCTSTR lpcszScriptSetName ) 
{
	ASSERT ( m_bWorkingArchive );
	ASSERT ( strlen ( lpcszScriptSetName ) > 0 );

	// najde jm�no ScriptSetu
	for ( int nIndex = m_cScriptSetNameArray.GetSize (); nIndex-- > 0; )
	{
		// zjist�, jedn�-li se o hledan� jm�no ScriptSetu
		if ( m_cScriptSetNameArray.GetAt ( nIndex ) == lpcszScriptSetName )
		{	// jedn� se o hledan� jm�no ScriptSetu
			// vr�t� adres�� ScriptSetu
			return m_cWorkingArchive.GetArchivePath () + 
				"\\" WORKING_SCRIPT_SETS_DIRECTORY "\\" + lpcszScriptSetName;
		}
	}
	LOAD_ASSERT ( FALSE );
}

//////////////////////////////////////////////////////////////////////
// Operace s mapou
//////////////////////////////////////////////////////////////////////

// otev�e archiv "cMapArchive" mapy "lpcszMapArchiveName" s ��slem verze mapy 
//		"dwMapVersion" (FALSE=chybn� soubor mapy)
BOOL CSFileManager::OpenMap ( CDataArchive &cMapArchive, LPCTSTR lpcszMapArchiveName, 
	DWORD dwMapVersion ) 
{
	// jm�no archivu mapy
	CString strMapArchiveName ( lpcszMapArchiveName );

	try
	{
		// pokus� se otev��t archiv mapy
		if ( !cMapArchive.Create ( strMapArchiveName, CFile::modeRead | 
			CFile::shareDenyWrite, CDataArchiveInfo::archiveFile ) )
		{	// nepoda�ilo se otev��t archiv mapy
			// vr�t� p��znak chybn�ho souboru mapy
			return FALSE;
		}
	}
	catch ( CException * )
	{
		// vr�t� p��znak chybn�ho souboru mapy
		return FALSE;
	}

	try
	{
		// pokus� se otev��t soubor mapy
		CArchiveFile cMapFile = cMapArchive.CreateFile ( MAP_FILE_NAME, CFile::modeRead | 
			CFile::shareDenyWrite );

	// na�te hlavi�ku verze souboru
		{
			SFileVersionHeader sFileVersionHeader;

			// pokus� se na��st hlavi�ku verze souboru
			if ( cMapFile.Read ( &sFileVersionHeader, sizeof ( sFileVersionHeader ) ) != 
				sizeof ( sFileVersionHeader ) )
			{	// nepoda�ilo se na��st hlavi�ku
				// vr�t� p��znak chybn�ho souboru mapy
				return FALSE;
			}

			// identifik�tor souboru mapy
			BYTE aMapFileID[16] = MAP_FILE_IDENTIFIER;
			ASSERT ( sizeof ( aMapFileID ) == sizeof ( sFileVersionHeader.m_aMapFileID ) );

			// zkontroluje identifik�tor souboru mapy
			for ( int nIndex = 16; nIndex-- > 0; )
			{
				if ( aMapFileID[nIndex] != sFileVersionHeader.m_aMapFileID[nIndex] )
				{
					// vr�t� p��znak chybn�ho souboru mapy
					return FALSE;
				}
			}

			ASSERT ( CURRENT_MAP_FILE_VERSION >= COMPATIBLE_MAP_FILE_VERSION );
			// na�te ��sla verz� form�tu mapy
			DWORD dwMapFormatVersion = sFileVersionHeader.m_dwFormatVersion;
			DWORD dwMapCompatibleFormatVersion = 
				sFileVersionHeader.m_dwCompatibleFormatVersion;
			// zkontroluje ��sla verz� form�tu mapy
			if ( dwMapFormatVersion < dwMapCompatibleFormatVersion )
			{	// ��sla verz� form�tu mapy jsou chybn�
				// vr�t� p��znak chybn�ho souboru mapy
				return FALSE;
			}

			// zjist�, jedn�-li se o spr�vnou verzi form�tu mapy
			if ( dwMapFormatVersion != CURRENT_MAP_FILE_VERSION )
			{	// nejedn� se o spr�vnou verzi form�tu mapy
				// zjist�, jedn�-li se o starou verzi form�tu mapy
				if ( dwMapFormatVersion < CURRENT_MAP_FILE_VERSION )
				{	// jedn� se o starou verzi form�tu mapy
					// zkontroluje kompatabilitu verze form�tu mapy
					if ( dwMapFormatVersion < COMPATIBLE_MAP_FILE_VERSION )
					{
						// vr�t� p��znak chybn�ho souboru mapy
						return FALSE;
					}
				}
				else
				{	// jedn� se o mlad�� verzi form�tu mapy
					// zkontroluje kompatabilitu verze form�tu mapy
					if ( dwMapCompatibleFormatVersion > CURRENT_MAP_FILE_VERSION )
					{
						// vr�t� p��znak chybn�ho souboru mapy
						return FALSE;
					}
				}
			}
		}

	// na�te hlavi�ku mapy
		{
			SMapHeader sMapHeader;

			// pokus� se na��st hlavi�ku mapy
			if ( cMapFile.Read ( &sMapHeader, sizeof ( sMapHeader ) ) != 
				sizeof ( sMapHeader ) )
			{	// nepoda�ilo se na��st hlavi�ku mapy
				// vr�t� p��znak chybn�ho souboru mapy
				return FALSE;
			}

			// zkontroluje verzi mapy
			if ( sMapHeader.m_dwMapVersion != dwMapVersion )
			{	// nejedn� se o po�adovanou verzi mapy
				// vr�t� p��znak chybn�ho souboru mapy
				return FALSE;
			}
		}

		// zav�e soubor mapy
		cMapFile.Close ();
	}
	catch ( CException * )
	{
		// zav�e archiv mapy
		cMapArchive.Close ();

		// vr�t� p��znak chybn�ho souboru mapy
		return FALSE;
	}

	// vr�t� p��znak �sp�n�ho na�ten� souboru mapy
	return TRUE;
}

// nahraje z archivu mapy "cMapArchive" ScriptSet "strScriptSetName"
// v�jimky: CDataArchiveException
void CSFileManager::LoadScriptSet ( CDataArchive cMapArchive, 
	CString &strScriptSetName ) 
{
	ASSERT ( m_bWorkingArchive );

	// jm�no pracovn�ho adres��e ScriptSetu
	CString strWorkingScriptSetDirectoryName ( WORKING_SCRIPT_SETS_DIRECTORY "\\" );
	strWorkingScriptSetDirectoryName += strScriptSetName;
	// jm�no zdrojov�ho adres��e ScriptSetu
	CString strSourceScriptSetDirectoryName ( SCRIPT_SETS_DIRECTORY "\\" );
	strSourceScriptSetDirectoryName += strScriptSetName;

	// vytvo�� pracovn� adres�� ScriptSetu
	m_cWorkingArchive.MakeDirectory ( strWorkingScriptSetDirectoryName );

	// vykop�ruje ScriptSet do pracovn�ho archivu
	m_cWorkingArchive.AppendDir ( strWorkingScriptSetDirectoryName, cMapArchive, 
		strSourceScriptSetDirectoryName, (EAppendModes)( appendRecursive | 
		appendUncompressed ) );

	// p�id� jm�no ScriptSetu do pole jmen ScriptSet�
	(void)m_cScriptSetNameArray.Add ( strScriptSetName );
}
