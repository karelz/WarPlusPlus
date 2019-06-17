/***********************************************************
 * 
 * Projekt: Strategick� hra
 *    ��st: Server hry
 *   Autor: Karel Zikmund
 * 
 *   Popis: T��da civilizace na serveru hry
 * 
 ***********************************************************/

#include "StdAfx.h"
#include "SCivilization.h"

#include "Common\Map\Map.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

IMPLEMENT_DYNAMIC ( CSCivilization, CPersistentObject )

//////////////////////////////////////////////////////////////////////
// Konstrukce a destrukce
//////////////////////////////////////////////////////////////////////

// konstruktor
CSCivilization::CSCivilization () 
{
	// zne�kodn� data civilizace
	m_dwCivilizationID = 0;
	m_pZCivilization = NULL;
	m_dwCivilizationIndex = CIVILIZATION_COUNT_MAX;
	m_dwUnitTypeCount = 0;
	m_pUnitTypes = NULL;
	m_strCivilizationName.Empty ();

	// vynuluje mno�stv� surovin civilizace
	for ( DWORD dwResourceIndex = RESOURCE_COUNT; dwResourceIndex-- > 0; )
	{
		m_aResources[dwResourceIndex] = 0;
	}
}

// destruktor
CSCivilization::~CSCivilization () 
{
	ASSERT ( CheckEmptyData () );
}

//////////////////////////////////////////////////////////////////////
// Inicializace a zni�en� dat objektu
//////////////////////////////////////////////////////////////////////

// vytvo�� civilizaci ze souboru mapy "cMapFile" s indexem civilizace 
//		"dwCivilizationIndex" a p�ekladovou tabulkou ID na jm�no ScriptSetu 
//		"cScriptSetTable"
void CSCivilization::Create ( CArchiveFile cMapFile, DWORD dwCivilizationIndex, 
	CMap<DWORD, DWORD, CString, CString &> &cScriptSetTable ) 
{
	ASSERT ( dwCivilizationIndex < CIVILIZATION_COUNT_MAX );

	ASSERT ( CheckEmptyData () );

	// na�te hlavi�ku civilizace
	SCivilizationHeader sCivilizationHeader;
	LOAD_ASSERT ( cMapFile.Read ( &sCivilizationHeader, sizeof ( sCivilizationHeader ) ) == sizeof ( sCivilizationHeader ) );

	// zjist� ID civilizace
	m_dwCivilizationID = sCivilizationHeader.m_dwID;

	// zjist� jm�no civilizace
	m_strCivilizationName = sCivilizationHeader.m_aName;
	LOAD_ASSERT ( m_strCivilizationName.GetLength () < sizeof ( sCivilizationHeader.m_aName ) );
	LOAD_ASSERT ( !m_strCivilizationName.IsEmpty () );

	// okop�ruje barvu civilizace
	m_dwCivilizationColor = sCivilizationHeader.m_dwColor;

	// zjist� mno�stv� surovin civilizace
	ASSERT ( sizeof ( m_aResources ) == sizeof ( sCivilizationHeader.m_aResources ) );
	memcpy ( m_aResources, sCivilizationHeader.m_aResources, sizeof ( m_aResources ) );

	// zjist� startovac� pozici klienta na map�
	m_pointClientStartMapPosition.x = sCivilizationHeader.m_dwStartPositionX;
	m_pointClientStartMapPosition.y = sCivilizationHeader.m_dwStartPositionY;

	// aktualizuje index civilizace
	m_dwCivilizationIndex = dwCivilizationIndex;

	// jm�no ScriptSetu
	CString strScriptSetName;
	// p�elo�� ID ScriptSetu na jm�no
	LOAD_ASSERT ( cScriptSetTable.Lookup ( sCivilizationHeader.m_dwScriptSetID, 
		strScriptSetName ) );

	// vytvo�� civilizaci na civilizaci
	m_pZCivilization = new CZCivilization;

	try
	{
		// vytvo�� civilizaci na civilizaci
		m_pZCivilization->Create ( strScriptSetName, this );
	}
	catch ( CException * )
	{
		// zni�� civilizaci na civilizaci
		delete m_pZCivilization;
		m_pZCivilization = NULL;

		// pokra�uje ve zpracov�n� v�jimky
		throw;
	}
}

// vytvo�� typy jednotek civilizace z pole archiv� knihoven civilizac� 
//		"pUnitTypeLibraryArchiveArray" obsahuj�c� "dwUnitTypeCount" typ� jednotek
void CSCivilization::CreateUnitTypes ( 
	CDataArchivePtrArray *pUnitTypeLibraryArchiveArray, DWORD dwUnitTypeCount ) 
{
	ASSERT ( pUnitTypeLibraryArchiveArray != NULL );
	ASSERT ( pUnitTypeLibraryArchiveArray->GetSize () > 0 );
	ASSERT ( dwUnitTypeCount > 0 );

	ASSERT ( m_dwCivilizationIndex < CIVILIZATION_COUNT_MAX );
	ASSERT ( !m_strCivilizationName.IsEmpty () );

	ASSERT ( m_dwUnitTypeCount == 0 );
	ASSERT ( m_pUnitTypes == NULL );

	// aktualizuje velikost pole typ� jednotek
	m_dwUnitTypeCount = dwUnitTypeCount;
	// vytvo�� pole typ� jednotek
	m_pUnitTypes = new CSUnitType[m_dwUnitTypeCount];

	// ukazatel na typ jednotky
	CSUnitType *pUnitType = m_pUnitTypes;

	// projede archivy typ� jednotek
	for ( int nUnitTypeLibraryArchiveIndex = pUnitTypeLibraryArchiveArray->GetSize (); 
		nUnitTypeLibraryArchiveIndex-- > 0; )
	{
		// z�sk� ukazatel na archiv knihovny typ� jednotek
		CDataArchive *pUnitTypeLibraryArchive = pUnitTypeLibraryArchiveArray->GetAt ( 
			nUnitTypeLibraryArchiveIndex );
		ASSERT ( pUnitTypeLibraryArchive != NULL );

		// otev�e hlavi�kov� soubor knihovny
		CArchiveFile cUnitTypeLibraryHeaderFile = 
			pUnitTypeLibraryArchive->CreateFile ( _T("Library.Header"), CFile::modeRead | 
			CFile::shareDenyWrite );

		// p�e�te hlavi�ku verze knihovny typ� jednotek
		{
			SUnitTypeFileVersionHeader sUnitTypeLibraryVersionHeader;
			LOAD_ASSERT ( cUnitTypeLibraryHeaderFile.Read ( &sUnitTypeLibraryVersionHeader, sizeof ( sUnitTypeLibraryVersionHeader ) ) == sizeof ( sUnitTypeLibraryVersionHeader ) );

			// identifik�tor souboru knihovny typ� jednotek
			BYTE aUnitTypeLibraryFileID[16] = UNITTYPE_FILE_IDENTIFIER;
			ASSERT ( sizeof ( aUnitTypeLibraryFileID ) == 
				sizeof ( sUnitTypeLibraryVersionHeader.m_aFileID ) );

			// zkontroluje identifik�tor souboru knihovny typ� jednotek
			for ( int nIndex = 16; nIndex-- > 0; )
			{
				LOAD_ASSERT ( aUnitTypeLibraryFileID[nIndex] == 
					sUnitTypeLibraryVersionHeader.m_aFileID[nIndex] );
			}

			ASSERT ( CURRENT_MAP_FILE_VERSION >= COMPATIBLE_MAP_FILE_VERSION );
			// zkontroluje ��sla verz� form�tu knihovny typ� jednotek
			LOAD_ASSERT ( sUnitTypeLibraryVersionHeader.m_dwFormatVersion >= 
				sUnitTypeLibraryVersionHeader.m_dwCompatibleFormatVersion );

			// zjist�, jedn�-li se o spr�vnou verzi form�tu knihovny typ� jednotek
			if ( sUnitTypeLibraryVersionHeader.m_dwFormatVersion != CURRENT_MAP_FILE_VERSION )
			{	// nejedn� se o spr�vnou verzi form�tu knihovny typ� jednotek
				// zjist�, jedn�-li se o starou verzi form�tu knihovny typ� jednotek
				if ( sUnitTypeLibraryVersionHeader.m_dwFormatVersion < CURRENT_MAP_FILE_VERSION )
				{	// jedn� se o starou verzi form�tu knihovny typ� jednotek
					// zkontroluje kompatabilitu verze form�tu knihovny typ� jednotek
					LOAD_ASSERT ( sUnitTypeLibraryVersionHeader.m_dwFormatVersion >= 
						COMPATIBLE_MAP_FILE_VERSION );
				}
				else
				{	// jedn� se o mlad�� verzi form�tu knihovny typ� jednotek
					// zkontroluje kompatabilitu verze form�tu knihovny typ� jednotek
					LOAD_ASSERT ( sUnitTypeLibraryVersionHeader.m_dwCompatibleFormatVersion <= 
						CURRENT_MAP_FILE_VERSION );
				}
			}
		}

		// p�e�te hlavi�ku knihovny typ� jednotek
		SUnitTypeLibraryHeader sUnitTypeLibraryHeader;
		LOAD_ASSERT ( cUnitTypeLibraryHeaderFile.Read ( &sUnitTypeLibraryHeader, sizeof ( sUnitTypeLibraryHeader ) ) == sizeof ( sUnitTypeLibraryHeader ) );

		// na�te jm�na soubor� s typem jednotky
		for ( int nUnitTypeLibraryIndex = sUnitTypeLibraryHeader.m_dwUnitTypeCount; 
			nUnitTypeLibraryIndex-- > 0; )
		{
			// zkontroluje, m�-li se na��st dal�� typ jednotky
			LOAD_ASSERT ( dwUnitTypeCount-- > 0 );

			// p�e�te jm�no dal��ho souboru s typem jednotky
			char szUnitTypeFileName[30];
			LOAD_ASSERT ( cUnitTypeLibraryHeaderFile.Read ( &szUnitTypeFileName, sizeof ( szUnitTypeFileName ) ) == sizeof ( szUnitTypeFileName ) );

			// zkontroluje d�lku jm�na souboru s typem jednotky
			int nUnitTypeFileNameLength = strlen ( szUnitTypeFileName );
			LOAD_ASSERT ( ( nUnitTypeFileNameLength > 0 ) && ( nUnitTypeFileNameLength < sizeof ( szUnitTypeFileName ) ) );

			// otev�e soubor s typem jednotky
			CArchiveFile cUnitTypeFile = pUnitTypeLibraryArchive->CreateFile ( 
				szUnitTypeFileName, CFile::modeRead | CFile::shareDenyWrite );

			// na�te typ jednotky
			pUnitType->Create ( cUnitTypeFile, this );

			// aktualizuje ukazatel na dal�� typ jednotky
			pUnitType++;

			// zkontroluje konec souboru s typem jednotky
			LOAD_ASSERT ( cUnitTypeFile.GetPosition () == cUnitTypeFile.GetLength () );

			// zav�e soubor s typem jednotky
			cUnitTypeFile.Close ();
		}

		// zkontroluje konec hlavi�kov�ho souboru knihovny
		LOAD_ASSERT ( cUnitTypeLibraryHeaderFile.GetPosition () == 
			cUnitTypeLibraryHeaderFile.GetLength () );

		// zav�e hlavi�kov� soubor knihovny
		cUnitTypeLibraryHeaderFile.Close ();
	}
}

// dokon�� vytvo�en� jednotek civilizace
//		v�jimky: CPersistentLoadException
void CSCivilization::PostCreateUnitTypes () 
{
	ASSERT ( m_dwUnitTypeCount > 0 );
	ASSERT ( m_pUnitTypes != NULL );

	// ukazatel na typ jednotky
	CSUnitType *pUnitType = m_pUnitTypes;

	// projede typy jednotek
	for ( DWORD dwUnitTypeIndex = m_dwUnitTypeCount; dwUnitTypeIndex-- > 0; pUnitType++ )
	{
		// nech� dokon�it vytvo�en� jednotky
		pUnitType->PostCreate ();
	}
}

// zni�� data civilizace
void CSCivilization::Delete () 
{
	// zjist�, je-li civilizace na civilizaci platn�
	if ( m_pZCivilization != NULL )
	{	// civilizace na civilizaci je platn�
		// zni�� civilizaci na civilizaci
		m_pZCivilization->Delete ();
		delete m_pZCivilization;
		m_pZCivilization = NULL;
	}

	// zjist�, je-li pole typ� jednotek platn�
	if ( m_pUnitTypes != NULL )
	{	// pole typ� jednotek je platn�
		ASSERT ( m_dwUnitTypeCount != 0 );
		// projede v�echny typy jednotek
		for ( DWORD dwUnitTypeIndex = m_dwUnitTypeCount; dwUnitTypeIndex-- > 0; )
		{
			// zni�� dal�� typ jednotky
			m_pUnitTypes[dwUnitTypeIndex].Delete ();
		}

		// zni�� pole typ� jednotek
		delete [] m_pUnitTypes;
		m_pUnitTypes = NULL;
		m_dwUnitTypeCount = 0;
	}

	// zne�kodn� data civilizace
	SetEmptyData ();
}

// vypln� objekt ne�kodn�mi daty (zni�� ��ste�n� inicializavan� data)
void CSCivilization::SetEmptyData () 
{
	// zne�kodn� civilizaci na civilizaci
	if ( m_pZCivilization != NULL )
	{
		// odpausuje civilizaci
		m_pZCivilization->Unpause ( false );
		// zni�� civilizaci na civilizaci
		m_pZCivilization->Delete ();
		delete m_pZCivilization;
		m_pZCivilization = NULL;
	}

	// zne�kodn� pole typ� jednotek
	if ( m_pUnitTypes != NULL )
	{
		ASSERT ( m_dwUnitTypeCount > 0 );
		// projede v�echny typy jednotek
		for ( DWORD dwUnitTypeIndex = m_dwUnitTypeCount; dwUnitTypeIndex-- > 0; )
		{
			// zne�kodn� data dal��ho typu jednotky
			m_pUnitTypes[dwUnitTypeIndex].SetEmptyData ();
		}
		// zni�� pole typ� jednotek
		delete [] m_pUnitTypes;
		m_pUnitTypes = NULL;
	}
	m_dwUnitTypeCount = 0;

	m_dwCivilizationID = 0;

	m_dwCivilizationIndex = CIVILIZATION_COUNT_MAX;
	m_strCivilizationName.Empty ();

	// vynuluje mno�stv� surovin civilizace
	for ( DWORD dwResourceIndex = RESOURCE_COUNT; dwResourceIndex-- > 0; )
	{
		m_aResources[dwResourceIndex] = 0;
	}
}

//////////////////////////////////////////////////////////////////////
// Ukl�d�n� dat (CPersistentObject metody)
//////////////////////////////////////////////////////////////////////

/*
CSCivilization stored

	DWORD m_dwID;
	int nCivilizationNameSize						// mus� b�t > 0
		char aCivilizationName[nCivilizationNameSize]
	int m_aResources[RESOURCE_COUNT]
	DWORD dwClientStartPositionX
	DWORD dwClientStartPositionY
	DWORD m_dwUnitTypeCount
	[m_dwUnitTypeCount]
		CSUnitType *
		CSUnitType stored
	CZCivilization stored
*/

// ukl�d�n� dat
void CSCivilization::PersistentSave ( CPersistentStorage &storage ) 
{
	ASSERT ( CheckValidData () );

	BRACE_BLOCK ( storage );

	// ulo�� ID civilizace
	storage << m_dwCivilizationID;

	// zjist� d�lku jm�na civilizace
	int nCivilizationNameSize = m_strCivilizationName.GetLength ();
	ASSERT ( nCivilizationNameSize > 0 );
	// ulo�� d�lku jm�na civilizace
	storage << nCivilizationNameSize;
	// ulo�� jm�no civilizace
	storage.Write ( m_strCivilizationName, nCivilizationNameSize );

	// ulo�� barvu civilizace
	storage << m_dwCivilizationColor;

	// ulo�� mno�stv� surovin civilizace
	storage.Write ( m_aResources, sizeof ( m_aResources ) );

	// ulo�� startovac� pozici klienta na map�
	storage << m_pointClientStartMapPosition.x;
	storage << m_pointClientStartMapPosition.y;

	// ulo�� po�et typ� jednotek
	storage << m_dwUnitTypeCount;

	// zjist� ukazatel na prvn� typ jednotky
	CSUnitType *pUnitType = m_pUnitTypes;

	// projede v�echny typy jednotek
	for ( DWORD dwUnitTypeIndex = m_dwUnitTypeCount; dwUnitTypeIndex-- > 0; pUnitType++ )
	{
		// ulo�� ukazatel na typ jednotky
		storage << pUnitType;
		// ulo�� typ jednotky
		pUnitType->PersistentSave ( storage );
	}

	// ulo�� civilizaci na civilizaci
	m_pZCivilization->PersistentSave ( storage );
}

// nahr�v�n� pouze ulo�en�ch dat
void CSCivilization::PersistentLoad ( CPersistentStorage &storage ) 
{
	ASSERT ( CheckEmptyData () );

	BRACE_BLOCK ( storage );

	// na�te ID civilizace
	storage >> m_dwCivilizationID;

	// d�lka jm�na civilizace
	int nCivilizationNameSize;
	// na�te d�lku jm�na civilizace
	storage >> nCivilizationNameSize;
	LOAD_ASSERT ( nCivilizationNameSize > 0 );
	// z�sk� ukazatel na na��tan� jm�no civilizace
	char *szCivilizationName = m_strCivilizationName.GetBufferSetLength ( nCivilizationNameSize );
	// na�te jm�no civilizace
	storage.Read ( szCivilizationName, nCivilizationNameSize );
	// zkontroluje d�lku jm�na civilizace
	for ( int nCharacterIndex = nCivilizationNameSize; nCharacterIndex-- > 0; )
	{
		// zkontroluje dal�� znak jm�na civilizace
		LOAD_ASSERT ( szCivilizationName[nCharacterIndex] != 0 );
	}
	// ukon�� na��t�n� jm�na civilizace
	m_strCivilizationName.ReleaseBuffer ( nCivilizationNameSize );

	// na�te barvu civilizace
	storage >> m_dwCivilizationColor;

	// na�te mno�stv� surovin civilizace
	storage.Read ( m_aResources, sizeof ( m_aResources ) );

	// na�te startovac� pozici klienta na map�
	storage >> m_pointClientStartMapPosition.x;
	storage >> m_pointClientStartMapPosition.y;

	// ulo�en� ukazatel
	void *pSavedPointer;

	// na�te po�et typ� jednotek
	storage >> m_dwUnitTypeCount;
	LOAD_ASSERT ( m_dwUnitTypeCount > 0 );

	// vytvo�� pole typ� jednotek
	m_pUnitTypes = new CSUnitType[m_dwUnitTypeCount];

	// zjist� ukazatel na prvn� typ jednotky
	CSUnitType *pUnitType = m_pUnitTypes;

	// projede v�echny typy jednotek
	for ( DWORD dwUnitTypeIndex = m_dwUnitTypeCount; dwUnitTypeIndex-- > 0; pUnitType++ )
	{
		// na�te star� ukazatel na typ jednotky
		storage >> pSavedPointer;
		LOAD_ASSERT ( pSavedPointer != NULL );
		// zaregistruje ukazatel na typ jednotky
		storage.RegisterPointer ( pSavedPointer, pUnitType );

		// na�te typ jednotky
		pUnitType->PersistentLoad ( storage, (CSUnitType *)pSavedPointer );
	}

	// vytvo�� civilizaci na civilizaci
	m_pZCivilization = new CZCivilization;

	try
	{
		// na�te civilizaci na civilizaci
		m_pZCivilization->PersistentLoad ( storage, m_strCivilizationName );
	}
	catch ( CException * )
	{
		// zni�� civilizaci na civilizaci
		delete m_pZCivilization;
		m_pZCivilization = NULL;

		// pokra�uje ve zpracov�n� v�jimky
		throw;
	}
}

// p�eklad ukazatel�
void CSCivilization::PersistentTranslatePointers ( CPersistentStorage &storage ) 
{
	// zjist� ukazatel na prvn� typ jednotky
	CSUnitType *pUnitType = m_pUnitTypes;

	// projede v�echny typy jednotek
	for ( DWORD dwUnitTypeIndex = m_dwUnitTypeCount; dwUnitTypeIndex-- > 0; pUnitType++ )
	{
		// p�elo�� ukazatele typu jednotky
		pUnitType->PersistentTranslatePointers ( storage );
	}

	// p�elo�� ukazatele civilizace na civilizaci
	m_pZCivilization->PersistentTranslatePointers ( storage );
}

// inicializace nahran�ho objektu
void CSCivilization::PersistentInit () 
{
	// zjist� ukazatel na prvn� typ jednotky
	CSUnitType *pUnitType = m_pUnitTypes;

	// projede v�echny typy jednotek
	for ( DWORD dwUnitTypeIndex = m_dwUnitTypeCount; dwUnitTypeIndex-- > 0; pUnitType++ )
	{
		// inicializuje typ jednotky
		pUnitType->PersistentInit ( this );
	}

	// inicializuje civilizaci na civilizaci
	m_pZCivilization->PersistentInit ();
}

//////////////////////////////////////////////////////////////////////
// Operace s civilizac�
//////////////////////////////////////////////////////////////////////

// vr�t� typ jednotky s ID "dwUnitTypeID" (NULL=neexistuj�c� typ jednotky)
CSUnitType *CSCivilization::GetUnitTypeByID ( DWORD dwUnitTypeID ) 
{
	ASSERT ( CheckValidData () );

	// projede typy jednotek civilizace
	CSUnitType *pUnitType = m_pUnitTypes;
	for ( DWORD dwUnitTypeIndex = m_dwUnitTypeCount; dwUnitTypeIndex--; pUnitType++ )
	{
		// zjist� jedn�-li se o hledan� typ jednotky
		if ( pUnitType->GetID () == dwUnitTypeID )
		{	// jedn� se o hledan� typ jednotky
			return pUnitType;
		}
	}

	// vr�t� p��znak neexistuj�c�ho typu jednotky
	return NULL;
}

// p�id� civilizaci suroviny "pResources" (civlizace mus� b�t zam�ena pro z�pis)
void CSCivilization::AddResources ( TSResources &aResources ) 
{
	// projede suroviny
	for ( DWORD dwResourceIndex = RESOURCE_COUNT; dwResourceIndex-- > 0; )
	{
		// p�id� civilizaci surovinu "dwResourceIndex"
		m_aResources[dwResourceIndex] += aResources[dwResourceIndex];
	}
}

// zjist�, lze-li civilizaci odebrat suroviny "aResources" (civilizace mus� b�t 
//		zam�ena pro �ten�)
BOOL CSCivilization::CanRemoveResources ( TSResources &aResources ) 
{
	// projede suroviny
	for ( DWORD dwResourceIndex = RESOURCE_COUNT; dwResourceIndex-- > 0; )
	{
		// zjist�, lze-li surovinu civilizaci odebrat
		if ( ( aResources[dwResourceIndex] != 0 ) && ( ( m_aResources[dwResourceIndex] - 
			aResources[dwResourceIndex] ) < 0 ) )
		{	// surovinu nelze civilizaci odebrat
			// vr�t� p��znak nemo�nosti odebr�n� surovin civilizaci
			return FALSE;
		}
	}
	// vr�t� p��znak mo�nosti odebr�n� surovin civilizaci
	return TRUE;
}

// odebere civilizaci suroviny "aResources" (civilizace mus� b�t zam�ena pro z�pis)
void CSCivilization::RemoveResources ( TSResources &aResources ) 
{
	// projede suroviny
	for ( DWORD dwResourceIndex = RESOURCE_COUNT; dwResourceIndex-- > 0; )
	{
		// odebere civilizaci surovinu "dwResourceIndex"
		m_aResources[dwResourceIndex] -= aResources[dwResourceIndex];
	}
}

//////////////////////////////////////////////////////////////////////
// Debuggovac� informace
//////////////////////////////////////////////////////////////////////

#ifdef _DEBUG

// zkontroluje spr�vn� data objektu (TRUE=OK)
BOOL CSCivilization::CheckValidData () 
{
	ASSERT ( m_pZCivilization != NULL );
	ASSERT ( m_dwCivilizationIndex < CIVILIZATION_COUNT_MAX );
	ASSERT ( m_dwUnitTypeCount != 0 );
	ASSERT ( m_pUnitTypes != NULL );
	ASSERT ( !m_strCivilizationName.IsEmpty () );

	return TRUE;
}

// zkontroluje ne�kodn� data objektu (TRUE=OK)
BOOL CSCivilization::CheckEmptyData () 
{
	ASSERT ( m_dwCivilizationID == 0 );
	ASSERT ( m_pZCivilization == NULL );
	ASSERT ( m_dwCivilizationIndex == CIVILIZATION_COUNT_MAX );
	ASSERT ( m_dwUnitTypeCount == 0 );
	ASSERT ( m_pUnitTypes == NULL );
	ASSERT ( m_strCivilizationName.IsEmpty () );

	for ( DWORD dwResourceIndex = RESOURCE_COUNT; dwResourceIndex-- > 0; )
	{
		ASSERT ( m_aResources[dwResourceIndex] == 0 );
	}

	return TRUE;
}

#endif //_DEBUG
