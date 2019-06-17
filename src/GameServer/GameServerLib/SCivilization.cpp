/***********************************************************
 * 
 * Projekt: Strategická hra
 *    Èást: Server hry
 *   Autor: Karel Zikmund
 * 
 *   Popis: Tøída civilizace na serveru hry
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
	// zneškodní data civilizace
	m_dwCivilizationID = 0;
	m_pZCivilization = NULL;
	m_dwCivilizationIndex = CIVILIZATION_COUNT_MAX;
	m_dwUnitTypeCount = 0;
	m_pUnitTypes = NULL;
	m_strCivilizationName.Empty ();

	// vynuluje množství surovin civilizace
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
// Inicializace a znièení dat objektu
//////////////////////////////////////////////////////////////////////

// vytvoøí civilizaci ze souboru mapy "cMapFile" s indexem civilizace 
//		"dwCivilizationIndex" a pøekladovou tabulkou ID na jméno ScriptSetu 
//		"cScriptSetTable"
void CSCivilization::Create ( CArchiveFile cMapFile, DWORD dwCivilizationIndex, 
	CMap<DWORD, DWORD, CString, CString &> &cScriptSetTable ) 
{
	ASSERT ( dwCivilizationIndex < CIVILIZATION_COUNT_MAX );

	ASSERT ( CheckEmptyData () );

	// naète hlavièku civilizace
	SCivilizationHeader sCivilizationHeader;
	LOAD_ASSERT ( cMapFile.Read ( &sCivilizationHeader, sizeof ( sCivilizationHeader ) ) == sizeof ( sCivilizationHeader ) );

	// zjistí ID civilizace
	m_dwCivilizationID = sCivilizationHeader.m_dwID;

	// zjistí jméno civilizace
	m_strCivilizationName = sCivilizationHeader.m_aName;
	LOAD_ASSERT ( m_strCivilizationName.GetLength () < sizeof ( sCivilizationHeader.m_aName ) );
	LOAD_ASSERT ( !m_strCivilizationName.IsEmpty () );

	// okopíruje barvu civilizace
	m_dwCivilizationColor = sCivilizationHeader.m_dwColor;

	// zjistí množství surovin civilizace
	ASSERT ( sizeof ( m_aResources ) == sizeof ( sCivilizationHeader.m_aResources ) );
	memcpy ( m_aResources, sCivilizationHeader.m_aResources, sizeof ( m_aResources ) );

	// zjistí startovací pozici klienta na mapì
	m_pointClientStartMapPosition.x = sCivilizationHeader.m_dwStartPositionX;
	m_pointClientStartMapPosition.y = sCivilizationHeader.m_dwStartPositionY;

	// aktualizuje index civilizace
	m_dwCivilizationIndex = dwCivilizationIndex;

	// jméno ScriptSetu
	CString strScriptSetName;
	// pøeloží ID ScriptSetu na jméno
	LOAD_ASSERT ( cScriptSetTable.Lookup ( sCivilizationHeader.m_dwScriptSetID, 
		strScriptSetName ) );

	// vytvoøí civilizaci na civilizaci
	m_pZCivilization = new CZCivilization;

	try
	{
		// vytvoøí civilizaci na civilizaci
		m_pZCivilization->Create ( strScriptSetName, this );
	}
	catch ( CException * )
	{
		// znièí civilizaci na civilizaci
		delete m_pZCivilization;
		m_pZCivilization = NULL;

		// pokraèuje ve zpracování výjimky
		throw;
	}
}

// vytvoøí typy jednotek civilizace z pole archivù knihoven civilizací 
//		"pUnitTypeLibraryArchiveArray" obsahující "dwUnitTypeCount" typù jednotek
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

	// aktualizuje velikost pole typù jednotek
	m_dwUnitTypeCount = dwUnitTypeCount;
	// vytvoøí pole typù jednotek
	m_pUnitTypes = new CSUnitType[m_dwUnitTypeCount];

	// ukazatel na typ jednotky
	CSUnitType *pUnitType = m_pUnitTypes;

	// projede archivy typù jednotek
	for ( int nUnitTypeLibraryArchiveIndex = pUnitTypeLibraryArchiveArray->GetSize (); 
		nUnitTypeLibraryArchiveIndex-- > 0; )
	{
		// získá ukazatel na archiv knihovny typù jednotek
		CDataArchive *pUnitTypeLibraryArchive = pUnitTypeLibraryArchiveArray->GetAt ( 
			nUnitTypeLibraryArchiveIndex );
		ASSERT ( pUnitTypeLibraryArchive != NULL );

		// otevøe hlavièkový soubor knihovny
		CArchiveFile cUnitTypeLibraryHeaderFile = 
			pUnitTypeLibraryArchive->CreateFile ( _T("Library.Header"), CFile::modeRead | 
			CFile::shareDenyWrite );

		// pøeète hlavièku verze knihovny typù jednotek
		{
			SUnitTypeFileVersionHeader sUnitTypeLibraryVersionHeader;
			LOAD_ASSERT ( cUnitTypeLibraryHeaderFile.Read ( &sUnitTypeLibraryVersionHeader, sizeof ( sUnitTypeLibraryVersionHeader ) ) == sizeof ( sUnitTypeLibraryVersionHeader ) );

			// identifikátor souboru knihovny typù jednotek
			BYTE aUnitTypeLibraryFileID[16] = UNITTYPE_FILE_IDENTIFIER;
			ASSERT ( sizeof ( aUnitTypeLibraryFileID ) == 
				sizeof ( sUnitTypeLibraryVersionHeader.m_aFileID ) );

			// zkontroluje identifikátor souboru knihovny typù jednotek
			for ( int nIndex = 16; nIndex-- > 0; )
			{
				LOAD_ASSERT ( aUnitTypeLibraryFileID[nIndex] == 
					sUnitTypeLibraryVersionHeader.m_aFileID[nIndex] );
			}

			ASSERT ( CURRENT_MAP_FILE_VERSION >= COMPATIBLE_MAP_FILE_VERSION );
			// zkontroluje èísla verzí formátu knihovny typù jednotek
			LOAD_ASSERT ( sUnitTypeLibraryVersionHeader.m_dwFormatVersion >= 
				sUnitTypeLibraryVersionHeader.m_dwCompatibleFormatVersion );

			// zjistí, jedná-li se o správnou verzi formátu knihovny typù jednotek
			if ( sUnitTypeLibraryVersionHeader.m_dwFormatVersion != CURRENT_MAP_FILE_VERSION )
			{	// nejedná se o správnou verzi formátu knihovny typù jednotek
				// zjistí, jedná-li se o starou verzi formátu knihovny typù jednotek
				if ( sUnitTypeLibraryVersionHeader.m_dwFormatVersion < CURRENT_MAP_FILE_VERSION )
				{	// jedná se o starou verzi formátu knihovny typù jednotek
					// zkontroluje kompatabilitu verze formátu knihovny typù jednotek
					LOAD_ASSERT ( sUnitTypeLibraryVersionHeader.m_dwFormatVersion >= 
						COMPATIBLE_MAP_FILE_VERSION );
				}
				else
				{	// jedná se o mladší verzi formátu knihovny typù jednotek
					// zkontroluje kompatabilitu verze formátu knihovny typù jednotek
					LOAD_ASSERT ( sUnitTypeLibraryVersionHeader.m_dwCompatibleFormatVersion <= 
						CURRENT_MAP_FILE_VERSION );
				}
			}
		}

		// pøeète hlavièku knihovny typù jednotek
		SUnitTypeLibraryHeader sUnitTypeLibraryHeader;
		LOAD_ASSERT ( cUnitTypeLibraryHeaderFile.Read ( &sUnitTypeLibraryHeader, sizeof ( sUnitTypeLibraryHeader ) ) == sizeof ( sUnitTypeLibraryHeader ) );

		// naète jména souborù s typem jednotky
		for ( int nUnitTypeLibraryIndex = sUnitTypeLibraryHeader.m_dwUnitTypeCount; 
			nUnitTypeLibraryIndex-- > 0; )
		{
			// zkontroluje, má-li se naèíst další typ jednotky
			LOAD_ASSERT ( dwUnitTypeCount-- > 0 );

			// pøeète jméno dalšího souboru s typem jednotky
			char szUnitTypeFileName[30];
			LOAD_ASSERT ( cUnitTypeLibraryHeaderFile.Read ( &szUnitTypeFileName, sizeof ( szUnitTypeFileName ) ) == sizeof ( szUnitTypeFileName ) );

			// zkontroluje délku jména souboru s typem jednotky
			int nUnitTypeFileNameLength = strlen ( szUnitTypeFileName );
			LOAD_ASSERT ( ( nUnitTypeFileNameLength > 0 ) && ( nUnitTypeFileNameLength < sizeof ( szUnitTypeFileName ) ) );

			// otevøe soubor s typem jednotky
			CArchiveFile cUnitTypeFile = pUnitTypeLibraryArchive->CreateFile ( 
				szUnitTypeFileName, CFile::modeRead | CFile::shareDenyWrite );

			// naète typ jednotky
			pUnitType->Create ( cUnitTypeFile, this );

			// aktualizuje ukazatel na další typ jednotky
			pUnitType++;

			// zkontroluje konec souboru s typem jednotky
			LOAD_ASSERT ( cUnitTypeFile.GetPosition () == cUnitTypeFile.GetLength () );

			// zavøe soubor s typem jednotky
			cUnitTypeFile.Close ();
		}

		// zkontroluje konec hlavièkového souboru knihovny
		LOAD_ASSERT ( cUnitTypeLibraryHeaderFile.GetPosition () == 
			cUnitTypeLibraryHeaderFile.GetLength () );

		// zavøe hlavièkový soubor knihovny
		cUnitTypeLibraryHeaderFile.Close ();
	}
}

// dokonèí vytvoøení jednotek civilizace
//		výjimky: CPersistentLoadException
void CSCivilization::PostCreateUnitTypes () 
{
	ASSERT ( m_dwUnitTypeCount > 0 );
	ASSERT ( m_pUnitTypes != NULL );

	// ukazatel na typ jednotky
	CSUnitType *pUnitType = m_pUnitTypes;

	// projede typy jednotek
	for ( DWORD dwUnitTypeIndex = m_dwUnitTypeCount; dwUnitTypeIndex-- > 0; pUnitType++ )
	{
		// nechá dokonèit vytvoøení jednotky
		pUnitType->PostCreate ();
	}
}

// znièí data civilizace
void CSCivilization::Delete () 
{
	// zjistí, je-li civilizace na civilizaci platná
	if ( m_pZCivilization != NULL )
	{	// civilizace na civilizaci je platná
		// znièí civilizaci na civilizaci
		m_pZCivilization->Delete ();
		delete m_pZCivilization;
		m_pZCivilization = NULL;
	}

	// zjistí, je-li pole typù jednotek platné
	if ( m_pUnitTypes != NULL )
	{	// pole typù jednotek je platné
		ASSERT ( m_dwUnitTypeCount != 0 );
		// projede všechny typy jednotek
		for ( DWORD dwUnitTypeIndex = m_dwUnitTypeCount; dwUnitTypeIndex-- > 0; )
		{
			// znièí další typ jednotky
			m_pUnitTypes[dwUnitTypeIndex].Delete ();
		}

		// znièí pole typù jednotek
		delete [] m_pUnitTypes;
		m_pUnitTypes = NULL;
		m_dwUnitTypeCount = 0;
	}

	// zneškodní data civilizace
	SetEmptyData ();
}

// vyplní objekt neškodnými daty (znièí èásteènì inicializavaná data)
void CSCivilization::SetEmptyData () 
{
	// zneškodní civilizaci na civilizaci
	if ( m_pZCivilization != NULL )
	{
		// odpausuje civilizaci
		m_pZCivilization->Unpause ( false );
		// znièí civilizaci na civilizaci
		m_pZCivilization->Delete ();
		delete m_pZCivilization;
		m_pZCivilization = NULL;
	}

	// zneškodní pole typù jednotek
	if ( m_pUnitTypes != NULL )
	{
		ASSERT ( m_dwUnitTypeCount > 0 );
		// projede všechny typy jednotek
		for ( DWORD dwUnitTypeIndex = m_dwUnitTypeCount; dwUnitTypeIndex-- > 0; )
		{
			// zneškodní data dalšího typu jednotky
			m_pUnitTypes[dwUnitTypeIndex].SetEmptyData ();
		}
		// znièí pole typù jednotek
		delete [] m_pUnitTypes;
		m_pUnitTypes = NULL;
	}
	m_dwUnitTypeCount = 0;

	m_dwCivilizationID = 0;

	m_dwCivilizationIndex = CIVILIZATION_COUNT_MAX;
	m_strCivilizationName.Empty ();

	// vynuluje množství surovin civilizace
	for ( DWORD dwResourceIndex = RESOURCE_COUNT; dwResourceIndex-- > 0; )
	{
		m_aResources[dwResourceIndex] = 0;
	}
}

//////////////////////////////////////////////////////////////////////
// Ukládání dat (CPersistentObject metody)
//////////////////////////////////////////////////////////////////////

/*
CSCivilization stored

	DWORD m_dwID;
	int nCivilizationNameSize						// musí být > 0
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

// ukládání dat
void CSCivilization::PersistentSave ( CPersistentStorage &storage ) 
{
	ASSERT ( CheckValidData () );

	BRACE_BLOCK ( storage );

	// uloží ID civilizace
	storage << m_dwCivilizationID;

	// zjistí délku jména civilizace
	int nCivilizationNameSize = m_strCivilizationName.GetLength ();
	ASSERT ( nCivilizationNameSize > 0 );
	// uloží délku jména civilizace
	storage << nCivilizationNameSize;
	// uloží jméno civilizace
	storage.Write ( m_strCivilizationName, nCivilizationNameSize );

	// uloží barvu civilizace
	storage << m_dwCivilizationColor;

	// uloží množství surovin civilizace
	storage.Write ( m_aResources, sizeof ( m_aResources ) );

	// uloží startovací pozici klienta na mapì
	storage << m_pointClientStartMapPosition.x;
	storage << m_pointClientStartMapPosition.y;

	// uloží poèet typù jednotek
	storage << m_dwUnitTypeCount;

	// zjistí ukazatel na první typ jednotky
	CSUnitType *pUnitType = m_pUnitTypes;

	// projede všechny typy jednotek
	for ( DWORD dwUnitTypeIndex = m_dwUnitTypeCount; dwUnitTypeIndex-- > 0; pUnitType++ )
	{
		// uloží ukazatel na typ jednotky
		storage << pUnitType;
		// uloží typ jednotky
		pUnitType->PersistentSave ( storage );
	}

	// uloží civilizaci na civilizaci
	m_pZCivilization->PersistentSave ( storage );
}

// nahrávání pouze uložených dat
void CSCivilization::PersistentLoad ( CPersistentStorage &storage ) 
{
	ASSERT ( CheckEmptyData () );

	BRACE_BLOCK ( storage );

	// naète ID civilizace
	storage >> m_dwCivilizationID;

	// délka jména civilizace
	int nCivilizationNameSize;
	// naète délku jména civilizace
	storage >> nCivilizationNameSize;
	LOAD_ASSERT ( nCivilizationNameSize > 0 );
	// získá ukazatel na naèítané jméno civilizace
	char *szCivilizationName = m_strCivilizationName.GetBufferSetLength ( nCivilizationNameSize );
	// naète jméno civilizace
	storage.Read ( szCivilizationName, nCivilizationNameSize );
	// zkontroluje délku jména civilizace
	for ( int nCharacterIndex = nCivilizationNameSize; nCharacterIndex-- > 0; )
	{
		// zkontroluje další znak jména civilizace
		LOAD_ASSERT ( szCivilizationName[nCharacterIndex] != 0 );
	}
	// ukonèí naèítání jména civilizace
	m_strCivilizationName.ReleaseBuffer ( nCivilizationNameSize );

	// naète barvu civilizace
	storage >> m_dwCivilizationColor;

	// naète množství surovin civilizace
	storage.Read ( m_aResources, sizeof ( m_aResources ) );

	// naète startovací pozici klienta na mapì
	storage >> m_pointClientStartMapPosition.x;
	storage >> m_pointClientStartMapPosition.y;

	// uložený ukazatel
	void *pSavedPointer;

	// naète poèet typù jednotek
	storage >> m_dwUnitTypeCount;
	LOAD_ASSERT ( m_dwUnitTypeCount > 0 );

	// vytvoøí pole typù jednotek
	m_pUnitTypes = new CSUnitType[m_dwUnitTypeCount];

	// zjistí ukazatel na první typ jednotky
	CSUnitType *pUnitType = m_pUnitTypes;

	// projede všechny typy jednotek
	for ( DWORD dwUnitTypeIndex = m_dwUnitTypeCount; dwUnitTypeIndex-- > 0; pUnitType++ )
	{
		// naète starý ukazatel na typ jednotky
		storage >> pSavedPointer;
		LOAD_ASSERT ( pSavedPointer != NULL );
		// zaregistruje ukazatel na typ jednotky
		storage.RegisterPointer ( pSavedPointer, pUnitType );

		// naète typ jednotky
		pUnitType->PersistentLoad ( storage, (CSUnitType *)pSavedPointer );
	}

	// vytvoøí civilizaci na civilizaci
	m_pZCivilization = new CZCivilization;

	try
	{
		// naète civilizaci na civilizaci
		m_pZCivilization->PersistentLoad ( storage, m_strCivilizationName );
	}
	catch ( CException * )
	{
		// znièí civilizaci na civilizaci
		delete m_pZCivilization;
		m_pZCivilization = NULL;

		// pokraèuje ve zpracování výjimky
		throw;
	}
}

// pøeklad ukazatelù
void CSCivilization::PersistentTranslatePointers ( CPersistentStorage &storage ) 
{
	// zjistí ukazatel na první typ jednotky
	CSUnitType *pUnitType = m_pUnitTypes;

	// projede všechny typy jednotek
	for ( DWORD dwUnitTypeIndex = m_dwUnitTypeCount; dwUnitTypeIndex-- > 0; pUnitType++ )
	{
		// pøeloží ukazatele typu jednotky
		pUnitType->PersistentTranslatePointers ( storage );
	}

	// pøeloží ukazatele civilizace na civilizaci
	m_pZCivilization->PersistentTranslatePointers ( storage );
}

// inicializace nahraného objektu
void CSCivilization::PersistentInit () 
{
	// zjistí ukazatel na první typ jednotky
	CSUnitType *pUnitType = m_pUnitTypes;

	// projede všechny typy jednotek
	for ( DWORD dwUnitTypeIndex = m_dwUnitTypeCount; dwUnitTypeIndex-- > 0; pUnitType++ )
	{
		// inicializuje typ jednotky
		pUnitType->PersistentInit ( this );
	}

	// inicializuje civilizaci na civilizaci
	m_pZCivilization->PersistentInit ();
}

//////////////////////////////////////////////////////////////////////
// Operace s civilizací
//////////////////////////////////////////////////////////////////////

// vrátí typ jednotky s ID "dwUnitTypeID" (NULL=neexistující typ jednotky)
CSUnitType *CSCivilization::GetUnitTypeByID ( DWORD dwUnitTypeID ) 
{
	ASSERT ( CheckValidData () );

	// projede typy jednotek civilizace
	CSUnitType *pUnitType = m_pUnitTypes;
	for ( DWORD dwUnitTypeIndex = m_dwUnitTypeCount; dwUnitTypeIndex--; pUnitType++ )
	{
		// zjistí jedná-li se o hledaný typ jednotky
		if ( pUnitType->GetID () == dwUnitTypeID )
		{	// jedná se o hledaný typ jednotky
			return pUnitType;
		}
	}

	// vrátí pøíznak neexistujícího typu jednotky
	return NULL;
}

// pøidá civilizaci suroviny "pResources" (civlizace musí být zamèena pro zápis)
void CSCivilization::AddResources ( TSResources &aResources ) 
{
	// projede suroviny
	for ( DWORD dwResourceIndex = RESOURCE_COUNT; dwResourceIndex-- > 0; )
	{
		// pøidá civilizaci surovinu "dwResourceIndex"
		m_aResources[dwResourceIndex] += aResources[dwResourceIndex];
	}
}

// zjistí, lze-li civilizaci odebrat suroviny "aResources" (civilizace musí být 
//		zamèena pro ètení)
BOOL CSCivilization::CanRemoveResources ( TSResources &aResources ) 
{
	// projede suroviny
	for ( DWORD dwResourceIndex = RESOURCE_COUNT; dwResourceIndex-- > 0; )
	{
		// zjistí, lze-li surovinu civilizaci odebrat
		if ( ( aResources[dwResourceIndex] != 0 ) && ( ( m_aResources[dwResourceIndex] - 
			aResources[dwResourceIndex] ) < 0 ) )
		{	// surovinu nelze civilizaci odebrat
			// vrátí pøíznak nemožnosti odebrání surovin civilizaci
			return FALSE;
		}
	}
	// vrátí pøíznak možnosti odebrání surovin civilizaci
	return TRUE;
}

// odebere civilizaci suroviny "aResources" (civilizace musí být zamèena pro zápis)
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
// Debuggovací informace
//////////////////////////////////////////////////////////////////////

#ifdef _DEBUG

// zkontroluje správná data objektu (TRUE=OK)
BOOL CSCivilization::CheckValidData () 
{
	ASSERT ( m_pZCivilization != NULL );
	ASSERT ( m_dwCivilizationIndex < CIVILIZATION_COUNT_MAX );
	ASSERT ( m_dwUnitTypeCount != 0 );
	ASSERT ( m_pUnitTypes != NULL );
	ASSERT ( !m_strCivilizationName.IsEmpty () );

	return TRUE;
}

// zkontroluje neškodná data objektu (TRUE=OK)
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
