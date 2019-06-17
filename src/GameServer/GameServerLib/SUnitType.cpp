/***********************************************************
 * 
 * Projekt: Strategická hra
 *    Èást: Server hry
 *   Autor: Karel Zikmund
 * 
 *   Popis: Tøída typu jednotky na serveru hry. Obsahuje všechny 
 *          informace o typu jednotky, které potøebuje GameServer. 
 *          Každá civilizace má svoji vlastní instanci této tøídy.
 * 
 ***********************************************************/

#include "StdAfx.h"
#include "SUnitType.h"

#include "SMap.h"
#include "SSkillType.h"
#include "Common\Map\Map.h"
#include "GameServer\Civilization\SkillTypeName.h"

#include "SUnit.inl"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

IMPLEMENT_DYNAMIC ( CSUnitType, CPersistentObject )

//////////////////////////////////////////////////////////////////////
// Konstrukce a destrukce
//////////////////////////////////////////////////////////////////////

// konstruktor
CSUnitType::CSUnitType () 
{
	// zneškodní data typu jednotky
	m_dwID = DWORD_MAX;
	m_pCivilization = NULL;

	m_dwSkillTypeCount = 0;
	m_aSkillTypeDescription = NULL;
	m_dwSkillDataSize = 0;

	m_pFirstUnit = NULL;

	m_nLivesMax = 0;
	m_dwViewRadius = 0;
	m_dwVerticalPosition = 0;
	m_strUnitTypeName.Empty ();

	for ( int nModeIndex = 8; nModeIndex-- > 0; )
	{
		m_aMoveWidth[nModeIndex] = 0;
		m_aAppearanceID[nModeIndex] = DWORD_MAX;
		m_aInvisibilityState[nModeIndex] = 0;
		m_aInvisibilityDetection[nModeIndex] = 0;
	}
	m_dwFlags = 0;
}

// destruktor
CSUnitType::~CSUnitType () 
{
	// zkontroluje neškodná data typu jednotky
	ASSERT ( CheckEmptyData () );
}

//////////////////////////////////////////////////////////////////////
// Inicializace a znièení dat objektu
//////////////////////////////////////////////////////////////////////

// vytvoøí typ jednotky ze souboru typu jednotky "cUnitTypeFile" civilizace 
//		"pCivilization"
void CSUnitType::Create ( CArchiveFile cUnitTypeFile, CSCivilization *pCivilization ) 
{
	// zkontroluje neškodná data typu jednotky
	ASSERT ( CheckEmptyData () );

	// poèet vzhledù jednotky
	DWORD dwAppearanceCount;

	// aktualizuje ukazatel na civilizaci
	m_pCivilization = pCivilization;

// naète hlavièku souboru s typem jednotky
	{
		SUnitTypeHeader sUnitTypeHeader;
		LOAD_ASSERT ( cUnitTypeFile.Read ( &sUnitTypeHeader, sizeof ( sUnitTypeHeader ) ) == sizeof ( sUnitTypeHeader ) );

		// zjistí ID typu jednotky
		m_dwID = sUnitTypeHeader.m_dwID;

		// zjistí jméno typu jednotky
		m_strUnitTypeName = sUnitTypeHeader.m_pName;
		LOAD_ASSERT ( m_strUnitTypeName.GetLength () < sizeof ( sUnitTypeHeader.m_pName ) );
		LOAD_ASSERT ( !m_strUnitTypeName.IsEmpty () );

		// zjistí informace o typu jednotky
		m_nLivesMax = (int)sUnitTypeHeader.m_dwLifeMax;
		LOAD_ASSERT ( m_nLivesMax > 0 );
		m_dwViewRadius = sUnitTypeHeader.m_dwViewRadius;
		m_dwVerticalPosition = sUnitTypeHeader.m_dwZPos;
		LOAD_ASSERT ( m_dwVerticalPosition != 0 );
		m_dwFlags = sUnitTypeHeader.m_dwFlags;

		// projede módy typu jednotky
		for ( int nModeIndex = 8; nModeIndex-- > 0; )
		{
			// zjistí šíøku jednotky pøi pohybu
			m_aMoveWidth[nModeIndex] = sUnitTypeHeader.m_aMoveWidth[nModeIndex];
			// zjistí ID vzhledu jednotky
			m_aAppearanceID[nModeIndex] = sUnitTypeHeader.m_aDefaultAppearances[nModeIndex];
			// zjistí bitovou masku pøíznakù neviditelnosti jednotky (tj. jak je 
			//		jednotka neviditelná)
			m_aInvisibilityState[nModeIndex] = sUnitTypeHeader.m_aInvisibilityFlags[nModeIndex];
			// zjistí bitovou masku pøíznakù detekce neviditelnosti ostatních jednotek 
			//		(tj. které neviditelnosti jednotka vidí)
			m_aInvisibilityDetection[nModeIndex] = sUnitTypeHeader.m_aDetectionFlags[nModeIndex];
			// zjistí ukazatel na graf pro hledání cesty
			m_aFindPathGraph[nModeIndex] = (CSFindPathGraph *)sUnitTypeHeader.m_aPathGraphs[nModeIndex];
		}

		LOAD_ASSERT ( m_aAppearanceID[0] != 0 );
		// projede módy typu jednotky
		for ( nModeIndex = 8; nModeIndex-- > 1; )
		{
			// zjistí, je-li ID vzhledu jednotky platné
			if ( m_aAppearanceID[nModeIndex] == 0 )
			{	// ID vzhledu jednotky není platné
				// inicializuje ID vzhledu jednotky
				m_aAppearanceID[nModeIndex] = m_aAppearanceID[0];
			}
		}

		// zjistí poèet vzhledù typu jednotky
		dwAppearanceCount = sUnitTypeHeader.m_dwAppearancesNumber;

		// zjistí poèet typù skillù typu jednotky
		m_dwSkillTypeCount = sUnitTypeHeader.m_dwSkillTypesCount;
		LOAD_ASSERT ( m_dwSkillTypeCount > 0 );
		LOAD_ASSERT ( m_dwSkillTypeCount < m_dwSkillTypeCount * sizeof ( SSkillTypeDescription ) );
	}

	// pøeskoèí vzhledy typu jednotky
	cUnitTypeFile.Seek ( dwAppearanceCount * sizeof ( SAppearance ), CFile::current );

// naète hlavièky typù skill
	{
		SSkillTypeHeader sSkillTypeHeader;

		// vytvoøí pole typù skillù typu jednotky
		m_aSkillTypeDescription = new SSkillTypeDescription[m_dwSkillTypeCount];

		// offset dat skilly v datech skillù jednotky
		DWORD dwSkillDataOffset = 0;
		// zjistí ukazatel na první popis typu skilly
		SSkillTypeDescription *pSkillTypeDescription = m_aSkillTypeDescription;

		// projede všechny typy skillù typu jednotky
		for ( DWORD dwSkillTypeIndex = 0; dwSkillTypeIndex < m_dwSkillTypeCount; 
			dwSkillTypeIndex++, pSkillTypeDescription++ )
		{
			// naète hlavièku typu skilly
			LOAD_ASSERT ( cUnitTypeFile.Read ( &sSkillTypeHeader, sizeof ( sSkillTypeHeader ) ) == sizeof ( sSkillTypeHeader ) );

			// zjistí jméno typu skilly
			CString strSkillTypeName = sSkillTypeHeader.m_pName;
			LOAD_ASSERT ( strSkillTypeName.GetLength () < sizeof ( sSkillTypeHeader.m_pName ) );
			LOAD_ASSERT ( !strSkillTypeName.IsEmpty () );

			// vytvoøí uložený typ skilly podle jména
			CSSkillType *pSkillType = CSSkillType::Create ( strSkillTypeName, 
				dwSkillDataOffset, dwSkillTypeIndex, this );
			LOAD_ASSERT ( pSkillType != NULL );
			// aktualizuje offset dat další skilly
			dwSkillDataOffset += pSkillType->GetSkillDataSize ();

			// ukazatel na pamì pro nahrání dat typu skilly
			BYTE *pSkillTypeData = NULL;

			try
			{
				// zjistí, má-li typ skilly data pro nahrání
				if ( sSkillTypeHeader.m_dwDataSize > 0 )
				{	// typ skilly má data pro nahrání
					// alokuje pamì pro nahrání dat typu skilly
					pSkillTypeData = new BYTE[sSkillTypeHeader.m_dwDataSize];

					// nahraje data typu skilly
					LOAD_ASSERT ( cUnitTypeFile.Read ( pSkillTypeData, sSkillTypeHeader.m_dwDataSize ) == sSkillTypeHeader.m_dwDataSize );
				}

				// inicializuje typ skilly
				pSkillType->Create ( pSkillTypeData, sSkillTypeHeader.m_dwDataSize );
			}
			catch ( CException * )
			{
				// zjistí, je-li alokována pamì nahraných dat typu skilly
				if ( pSkillTypeData != NULL )
				{	// pamì nahraných dat typu skilly je alokovaná
					// znièí pamì nahraných dat typu skilly
					delete pSkillTypeData;
				}
				// znièí typ skilly
				pSkillType->Delete();
				delete pSkillType;
				// pokraèuje ve zpracování výjimky
				throw;
			}

			// zjistí, jsou-li nahraná data typu skilly platná
			if ( pSkillTypeData != NULL )
			{	// nahraná data typu skilly jsou platná
				// znièí pamì nahraných dat typu skilly
				delete [] pSkillTypeData;
			}

			// inicializuje popis typu skilly
			pSkillTypeDescription->dwID = pSkillType->GetID ();
			pSkillTypeDescription->pNameID = RegisterSkillTypeName ( pSkillType->GetName () );
			pSkillTypeDescription->nAllowedModesMask = sSkillTypeHeader.m_nAllowedModes;
			pSkillTypeDescription->bEnabled = sSkillTypeHeader.m_bEnabled;
			pSkillTypeDescription->pSkillType = pSkillType;
			pSkillTypeDescription->dwGroupMask = pSkillType->GetGroupMask ();
		}

		// aktualizuje velikost dat skilly
		m_dwSkillDataSize = dwSkillDataOffset;
		LOAD_ASSERT ( m_dwSkillDataSize > 0 );
	}

// ukonèí naèítání souboru typu jednotky

	// zkontroluje konec souboru
	LOAD_ASSERT ( cUnitTypeFile.GetPosition () == cUnitTypeFile.GetLength () );
}

// dokonèí vytvoøení typu jednotky
void CSUnitType::PostCreate () 
{
	ASSERT ( m_dwSkillTypeCount != 0 );
	ASSERT ( m_aSkillTypeDescription != NULL );

	// ukazatel na popis typu skilly
	SSkillTypeDescription *pSkillTypeDescription = m_aSkillTypeDescription;

	// projede typy skill
	for ( DWORD dwSkillTypeIndex = m_dwSkillTypeCount; dwSkillTypeIndex-- > 0; 
		pSkillTypeDescription++ )
	{
		// dokonèí vytvoøení typu skilly
		pSkillTypeDescription->pSkillType->PostCreateSkillType ();
	}
}

// znièí data typu jednotky
void CSUnitType::Delete () 
{
	// zneškodní pole typù skillù
	if ( m_aSkillTypeDescription != NULL )
	{
		ASSERT ( m_dwSkillTypeCount > 0 );
		// projede všechny popisy typu skilly
		for ( DWORD dwSkillTypeIndex = 0; dwSkillTypeIndex < m_dwSkillTypeCount; 
			dwSkillTypeIndex++ )
		{
			// zjistí, je-li ID jména typu skilly platné
			if ( m_aSkillTypeDescription[dwSkillTypeIndex].pNameID != NULL )
			{	// ID jména typu skilly je platné
				// znièí ID jména typu skilly
				UnregisterSkillTypeName ( m_aSkillTypeDescription[dwSkillTypeIndex].pNameID );
			}
			// zjistí, je-li typ skilly platný
			if ( m_aSkillTypeDescription[dwSkillTypeIndex].pSkillType != NULL )
			{	// typ skilly je platný
				// znièí data typu skilly
				m_aSkillTypeDescription[dwSkillTypeIndex].pSkillType->Delete ();
				// znièí typ skilly
				delete m_aSkillTypeDescription[dwSkillTypeIndex].pSkillType;
			}
		}
		// znièí pole popisù typù skill
		delete [] m_aSkillTypeDescription;
		m_aSkillTypeDescription = NULL;
	}

	// zneškodní data typu jednotky
	SetEmptyData ();
}

// vyplní objekt neškodnými daty (znièí èásteènì inicializavaná data)
void CSUnitType::SetEmptyData () 
{
	m_dwID = DWORD_MAX;
	m_pCivilization = NULL;

	// zneškodní pole typù skillù
	if ( m_aSkillTypeDescription != NULL )
	{
		ASSERT ( m_dwSkillTypeCount > 0 );
		// projede všechny popisy typu skilly
		for ( DWORD dwSkillTypeIndex = 0; dwSkillTypeIndex < m_dwSkillTypeCount; 
			dwSkillTypeIndex++ )
		{
			// zjistí, je-li typ skilly platný
			if ( m_aSkillTypeDescription[dwSkillTypeIndex].pSkillType != NULL )
			{	// typ skilly je platný
				// znièí data typu skilly
				m_aSkillTypeDescription[dwSkillTypeIndex].pSkillType->Delete ();
				// znièí typ skilly
				delete m_aSkillTypeDescription[dwSkillTypeIndex].pSkillType;
			}
		}
		// znièí pole popisù typù skill
		delete [] m_aSkillTypeDescription;
		m_aSkillTypeDescription = NULL;
	}
	m_dwSkillTypeCount = 0;
	m_dwSkillDataSize = 0;

	m_nLivesMax = 0;
	m_dwViewRadius = 0;
	m_dwVerticalPosition = 0;
	m_strUnitTypeName.Empty ();

	for ( int nModeIndex = 8; nModeIndex-- > 0; )
	{
		m_aMoveWidth[nModeIndex] = 0;
		m_aAppearanceID[nModeIndex] = DWORD_MAX;
		m_aInvisibilityState[nModeIndex] = 0;
		m_aInvisibilityDetection[nModeIndex] = 0;
	}
	m_dwFlags = 0;
}

//////////////////////////////////////////////////////////////////////
// Ukládání dat (CPersistentObject metody)
//////////////////////////////////////////////////////////////////////

/*
CSUnitType stored

	DWORD m_dwID
	int m_nLivesMax
	DWORD m_dwViewRadius
	DWORD m_dwVerticalPosition
	int nUnitTypeNameSize					// musí být > 0
		char aUnitTypeName[nUnitTypeNameSize]
	DWORD m_aMoveWidth[8]
	DWORD m_aAppearanceID[8]
	DWORD m_aInvisibilityState[8]
	DWORD m_aInvisibilityDetection[8]
	CSFindPathGraph *m_aFindPathGraph[8]
	DWORD m_dwFlags
	DWORD m_dwSkillTypeCount
	[m_dwSkillTypeCount]
		CSSkillType *pSkillType
		DWORD dwID
		CStringTableItem *pNameID
		BOOL bEnabled
		BYTE nAllowedModesMask
		CSSkillType stored
*/

// ukládání dat
void CSUnitType::PersistentSave ( CPersistentStorage &storage ) 
{
	// zkontroluje platnost dat typu jednotky
	ASSERT ( CheckValidData () );

	BRACE_BLOCK ( storage );

	// uloží ID typu jednotky
	storage << m_dwID;
	// uloží maximální poèet životù jednotky
	storage << m_nLivesMax;
	// uloží polomìr viditelnosti jednotky
	storage << m_dwViewRadius;
	// uloží vertikální umístìní jednotky
	storage << m_dwVerticalPosition;

	// zjistí délku jména typu jednotky
	int nUnitTypeNameSize = m_strUnitTypeName.GetLength ();
	ASSERT ( nUnitTypeNameSize > 0 );
	// uloží délku jména typu jednotky
	storage << nUnitTypeNameSize;
	// zjistí, je-li jméno typu jednotky prázdné
	// uloží jméno typu jednotky
	storage.Write ( m_strUnitTypeName, nUnitTypeNameSize );

	// uloží šíøku jednotky
	ASSERT ( sizeof ( m_aMoveWidth ) == sizeof ( DWORD ) * 8 );
	storage.Write ( &m_aMoveWidth, sizeof ( m_aMoveWidth ) );

	// uloží ID vzhledu jednotky
	ASSERT ( sizeof ( m_aAppearanceID ) == sizeof ( DWORD ) * 8 );
	storage.Write ( &m_aAppearanceID, sizeof ( m_aAppearanceID ) );

	// uloží bitové masky pøíznakù neviditelnosti jednotky
	ASSERT ( sizeof ( m_aInvisibilityState ) == sizeof ( DWORD ) * 8 );
	storage.Write ( &m_aInvisibilityState, sizeof ( m_aInvisibilityState ) );

	// uloží bitové masky pøíznakù detekce neviditelnosti ostatních jednotek
	ASSERT ( sizeof ( m_aInvisibilityDetection ) == sizeof ( DWORD ) * 8 );
	storage.Write ( &m_aInvisibilityDetection, sizeof ( m_aInvisibilityDetection ) );

	// uloží ukazatele na grafy pro hledání cesty
	ASSERT ( sizeof ( m_aFindPathGraph ) == sizeof ( DWORD ) * 8 );
	storage.Write ( &m_aFindPathGraph, sizeof ( m_aFindPathGraph ) );

	// uloží pøíznaky typu jednotky
	storage << m_dwFlags;

// uloží typy skillù typu jednotky

	// uloží poèet typù skillù typu jednotky
	storage << m_dwSkillTypeCount;

	// zjistí ukazatel na první popis typu skilly
	SSkillTypeDescription *pSkillTypeDescription = m_aSkillTypeDescription;

	// projede všechny typy skillù typu jednotky
	for ( DWORD dwSkillTypeIndex = 0; dwSkillTypeIndex < m_dwSkillTypeCount; 
		dwSkillTypeIndex++, pSkillTypeDescription++ )
	{
		// uloží popis typu skilly
		storage << pSkillTypeDescription->pSkillType;
		storage << pSkillTypeDescription->dwID;
		storage << pSkillTypeDescription->pNameID;
		storage << pSkillTypeDescription->bEnabled;
		storage << pSkillTypeDescription->nAllowedModesMask;

		// uloží typ skilly
		pSkillTypeDescription->pSkillType->PersistentSave ( storage );
	}
}

// nahrávání pouze uložených dat
void CSUnitType::PersistentLoad ( CPersistentStorage &storage, CSUnitType *pOldUnitType ) 
{
	// zkontroluje neškodná data typu jednotky
	ASSERT ( CheckEmptyData () );

	BRACE_BLOCK ( storage );

	// naète ID typu jednotky
	storage >> m_dwID;
	// naète maximální poèet životù jednotky
	storage >> m_nLivesMax;
	LOAD_ASSERT ( m_nLivesMax > 0 );
	// naète polomìr viditelnosti jednotky
	storage >> m_dwViewRadius;
	// naète vertikální umístìní jednotky
	storage >> m_dwVerticalPosition;
	LOAD_ASSERT ( m_dwVerticalPosition != 0 );

	// zjistí délku jména typu jednotky
	int nUnitTypeNameSize;
	// naète délku jména typu jednotky
	storage >> nUnitTypeNameSize;
	LOAD_ASSERT ( nUnitTypeNameSize > 0 );
	// získá ukazatel na naèítané jméno typu jednotky
	char *szUnitTypeName = m_strUnitTypeName.GetBufferSetLength ( nUnitTypeNameSize );
	// naète jméno typu jednotky
	storage.Read ( szUnitTypeName, nUnitTypeNameSize );
	// ukonèí naèítání jména typu jednotky
	m_strUnitTypeName.ReleaseBuffer ( nUnitTypeNameSize );

	// naète šíøku jednotky
	ASSERT ( sizeof ( m_aMoveWidth ) == sizeof ( DWORD ) * 8 );
	storage.Read ( &m_aMoveWidth, sizeof ( m_aMoveWidth ) );

	// naète ID vzhledu jednotky
	ASSERT ( sizeof ( m_aAppearanceID ) == sizeof ( DWORD ) * 8 );
	storage.Read ( &m_aAppearanceID, sizeof ( m_aAppearanceID ) );

	// zkontroluje ID vzhledu jednotky
	for ( int nModeIndex = 0; nModeIndex < 8; nModeIndex++ )
	{
		LOAD_ASSERT ( m_aAppearanceID[nModeIndex] != 0 );
	}

	// naète bitové masky pøíznakù neviditelnosti jednotky
	ASSERT ( sizeof ( m_aInvisibilityState ) == sizeof ( DWORD ) * 8 );
	storage.Read ( &m_aInvisibilityState, sizeof ( m_aInvisibilityState ) );

	// naète bitové masky pøíznakù detekce neviditelnosti ostatních jednotek
	ASSERT ( sizeof ( m_aInvisibilityDetection ) == sizeof ( DWORD ) * 8 );
	storage.Read ( &m_aInvisibilityDetection, sizeof ( m_aInvisibilityDetection ) );

	// naète ukazatele na grafy pro hledání cesty
	ASSERT ( sizeof ( m_aFindPathGraph ) == sizeof ( DWORD ) * 8 );
	storage.Read ( &m_aFindPathGraph, sizeof ( m_aFindPathGraph ) );

	// uloží pøíznaky typu jednotky
	storage >> m_dwFlags;

// naète typy skillù typu jednotky

	// naète poèet typù skillù typu jednotky
	storage >> m_dwSkillTypeCount;
	LOAD_ASSERT ( m_dwSkillTypeCount > 0 );
	LOAD_ASSERT ( m_dwSkillTypeCount < m_dwSkillTypeCount * sizeof ( SSkillTypeDescription ) );

	// vytvoøí pole typù skillù typu jednotky
	m_aSkillTypeDescription = new SSkillTypeDescription[m_dwSkillTypeCount];

	// offset dat skilly v datech skillù jednotky
	DWORD dwSkillDataOffset = 0;
	// zjistí ukazatel na první popis typu skilly
	SSkillTypeDescription *pSkillTypeDescription = m_aSkillTypeDescription;

	// projede všechny typy skillù typu jednotky
	for ( DWORD dwSkillTypeIndex = 0; dwSkillTypeIndex < m_dwSkillTypeCount; 
		dwSkillTypeIndex++, pSkillTypeDescription++ )
	{
		// naète popis typu skilly
		storage >> (void *&)pSkillTypeDescription->pSkillType;
		storage >> pSkillTypeDescription->dwID;
		storage >> (void *&)pSkillTypeDescription->pNameID;
		storage >> pSkillTypeDescription->bEnabled;
		storage >> pSkillTypeDescription->nAllowedModesMask;

		// zkontroluje jméno typu skilly (ID jména typu skilly)
		LOAD_ASSERT ( pSkillTypeDescription->pNameID != NULL );

		// vytvoøí uložený typ skilly
		CSSkillType *pSkillType = CSSkillType::Create ( pSkillTypeDescription->dwID, 
			dwSkillDataOffset, dwSkillTypeIndex, this );
		LOAD_ASSERT ( pSkillType != NULL );
		// aktualizuje offset dat další skilly
		dwSkillDataOffset += pSkillType->GetSkillDataSize ();

		// zaregistruje ukazatel na typ skilly
		storage.RegisterPointer ( pSkillTypeDescription->pSkillType, pSkillType );

		try
		{
			// naète typ skilly
			pSkillType->PersistentLoad ( storage );
		}
		catch ( CException * )
		{
			// znièí typ skilly
			delete pSkillType;
			// pokraèuje ve zpracování výjimky
			throw;
		}

		// aktualizuje ukazatel na typ skilly
		pSkillTypeDescription->pSkillType = pSkillType;
	}

	// aktualizuje velikost dat skilly
	m_dwSkillDataSize = dwSkillDataOffset;
	LOAD_ASSERT ( m_dwSkillDataSize > 0 );
}

// pøeklad ukazatelù
void CSUnitType::PersistentTranslatePointers ( CPersistentStorage &storage ) 
{
	// zjistí ukazatel na první popis typu skilly
	SSkillTypeDescription *pSkillTypeDescription = m_aSkillTypeDescription;

    TRACE("*** 0x%x ***\n", this);

    // projede všechny typy skillù typu jednotky
	for ( DWORD dwSkillTypeIndex = 0; dwSkillTypeIndex < m_dwSkillTypeCount; 
		dwSkillTypeIndex++, pSkillTypeDescription++ )
	{
		// zjistí ukazatel na jméno typu skilly (ID jména typu skilly)
		pSkillTypeDescription->pNameID = (CStringTableItem *)storage.TranslatePointer ( 
			pSkillTypeDescription->pNameID );

		// pøeloží ukazatele typu skilly
		pSkillTypeDescription->pSkillType->PersistentTranslatePointers ( storage );
	}

	// zjistí ukazatele na grafy pro hledání cesty
	for ( int nModeIndex = 8; nModeIndex-- > 0; )
	{
		// zjistí ukazatel na graf pro hledání cesty
		m_aFindPathGraph[nModeIndex] = (CSFindPathGraph *)storage.TranslatePointer ( 
			m_aFindPathGraph[nModeIndex] );
	}
}

// inicializace nahraného objektu
void CSUnitType::PersistentInit ( CSCivilization *pCivilization ) 
{
	// inicializuje ukazatel na civilizaci
	m_pCivilization = pCivilization;

	// zjistí ukazatel na první popis typu skilly
	SSkillTypeDescription *pSkillTypeDescription = m_aSkillTypeDescription;

	// projede všechny typy skillù typu jednotky
	for ( DWORD dwSkillTypeIndex = 0; dwSkillTypeIndex < m_dwSkillTypeCount; 
		dwSkillTypeIndex++, pSkillTypeDescription++ )
	{
		// inicializuje masku skupin skilly
		pSkillTypeDescription->dwGroupMask = 
			pSkillTypeDescription->pSkillType->GetGroupMask ();
		// inicializuje typ skilly
		pSkillTypeDescription->pSkillType->PersistentInit ();
	}
}

//////////////////////////////////////////////////////////////////////
// Get/Set metody serverových informací o typu jednotky
//////////////////////////////////////////////////////////////////////

// initializuje skilly jednotky "pUnit" (voláno po vytvoøení objektu jednotky)
void CSUnitType::InitializeSkills ( CSUnit *pUnit ) 
{
	// zjistí ukazatel na první popis typu skilly
	SSkillTypeDescription *pSkillTypeDescription = m_aSkillTypeDescription;

	// projede všechny typy skillù typu jednotky
	for ( DWORD dwSkillTypeIndex = 0; dwSkillTypeIndex < m_dwSkillTypeCount; 
		dwSkillTypeIndex++, pSkillTypeDescription++ )
	{
		// inicializuje skillu jednotky
		pSkillTypeDescription->pSkillType->InitializeSkill ( pUnit );
	}
}

// znièí skilly jednotky "pUnit" (voláno pøed znièením objektu jednotky)
void CSUnitType::DestructSkills ( CSUnit *pUnit ) 
{
	// zjistí ukazatel na první popis typu skilly
	SSkillTypeDescription *pSkillTypeDescription = m_aSkillTypeDescription;

	// projede všechny typy skillù typu jednotky
	for ( DWORD dwSkillTypeIndex = 0; dwSkillTypeIndex < m_dwSkillTypeCount; 
		dwSkillTypeIndex++, pSkillTypeDescription++ )
	{
		// zjistí, je-li skilla aktivní
		if ( pSkillTypeDescription->pSkillType->IsSkillActive ( pUnit ) )
		{	// skilla je aktivní
			// deaktivuje skillu jednotky
			pSkillTypeDescription->pSkillType->DeactivateSkill ( pUnit );
		}

		// znièí skillu jednotky
		pSkillTypeDescription->pSkillType->DestructSkill ( pUnit );
	}
}

// deaktivuje skupiny skill "dwGroupMask" jednotky "pUnit" (jednotka musí být zamèena 
//		pro zápis)
void CSUnitType::DeactivateSkillGroup ( CSUnit *pUnit, DWORD dwGroupMask ) 
{
	// zjistí ukazatel na první popis typu skilly
	SSkillTypeDescription *pSkillTypeDescription = m_aSkillTypeDescription;

	// projede všechny typy skillù typu jednotky
	for ( DWORD dwSkillTypeIndex = 0; dwSkillTypeIndex < m_dwSkillTypeCount; 
		dwSkillTypeIndex++, pSkillTypeDescription++ )
	{
		// zjistí, jedná-li se o skillu deaktivovaných skupin
		if ( pSkillTypeDescription->dwGroupMask & dwGroupMask )
		{	// jedná se o skillu deaktivovaných skupin
			// zjistí, je-li skilla aktivní
			if ( pSkillTypeDescription->pSkillType->IsSkillActive ( pUnit ) )
			{	// skilla je aktivní
				// deaktivuje skillu jednotky
				pSkillTypeDescription->pSkillType->DeactivateSkill ( pUnit );
			}
		}
	}
}

// vrátí další typ skilly ze skupiny skill "dwGroupMask" poèínaje typem skilly s indexem 
//		"dwIndex", který posune za nalezený typ skilly (NULL=nenalezeno)
CSSkillType *CSUnitType::GetNextSkillType ( DWORD &dwIndex, DWORD dwGroupMask ) 
{
	// zjistí ukazatel na první popis typu skilly
	SSkillTypeDescription *pSkillTypeDescription = m_aSkillTypeDescription + dwIndex;

	// projede všechny typy skillù typu jednotky
	for ( ; dwIndex++ < m_dwSkillTypeCount; pSkillTypeDescription++ )
	{
		// zjistí, jedná-li se o typ skilly z hledané skupiny
		if ( pSkillTypeDescription->dwGroupMask & dwGroupMask )
		{	// jedná se o typ skilly z hledané skupiny
			// vrátí nalezený typ skilly
			return pSkillTypeDescription->pSkillType;
		}
	}
	// požadovaný typ skilly nebyl nalezen
	return NULL;
}

//////////////////////////////////////////////////////////////////////
// Operace s jednotkami typu jednotky
//////////////////////////////////////////////////////////////////////

// vytvoøí jednotku na pozici "pointPosition", s velitelem "pCommander", skriptem 
//		"lpszScriptName", smìrem "nDirection" a vertikální pozicí "dwVerticalPosition" 
//		(0=implicitní vertikální pozice z typu jednotky) (typ jednotky musí být zamèen pro 
//		zápis, musí být zamèeny MapSquary, lze volat jen z MainLoop threadu) (NULL=jednotku 
//		se nepodaøilo vyrobit)
CSUnit *CSUnitType::CreateUnit ( CPointDW pointPosition, CZUnit *pCommander, 
	LPCTSTR lpszScriptName, BYTE nDirection, DWORD dwVerticalPosition ) 
{
	ASSERT ( g_cMap.IsMainLoopThread () );

	// vytvoøí novou jednotku typu
	CSUnit *pUnit = new CSUnit;
	pUnit->PreCreate ( this );

	// inicializuje parametry jednotky
	pUnit->SetVerticalPosition ( ( dwVerticalPosition == 0 ) ? m_dwVerticalPosition : 
		dwVerticalPosition );

	// najde nejbližší pozici umístìní jednotky
	CPointDW pointNearestPosition = g_cMap.GetNearestUnitPlace ( pUnit, pointPosition );

	// zjistí, je-li nalezeno místo pro jednotku na mapì
	if ( !g_cMap.IsMapPosition ( pointNearestPosition ) )
	{	// místo pro jednotku na mapì nebylo nalezeno
		// znièí vytvoøenou jednotku
		pUnit->m_nLives = 0;
		pUnit->Delete ();
		delete pUnit;
		// vrátí pøíznak neúspìchu
		return NULL;
	}

	// umístí jednotku na mapu
	VERIFY ( g_cMap.PlaceUnit ( pUnit, pointNearestPosition, TRUE ) );

	// vytvoøí jednotku na civilizaci
	pUnit->m_pZUnit = m_pCivilization->GetZCivilization ()->CreateUnit ( pUnit, pCommander, 
		lpszScriptName );
	ASSERT ( pUnit->m_pZUnit != NULL );

	// inicializuje parametry jednotky
	pUnit->SetDirection ( nDirection );
	// nastaví kontrolní body jednotky
	pUnit->SetFirstCheckPoint ( pUnit->GetPosition (), 0 );
	pUnit->SetSecondCheckPoint ( pUnit->GetPosition (), 0 );

	// zvýší poèet odkazù na jednotku
	pUnit->AddRef ();
	// pøidá vytvoøenou jednotku do jednotek mapy
	g_cMap.AddUnit ( pUnit );

	// pøidá jednotku do seznamu jednotek typu jednotky
	AddUnit ( pUnit );

	// vrátí ukazatel na jednotku
	return pUnit;
}

// pøidá jednotku "pUnit" do seznamu jednotek typu jednotky (typ jednotky musí být 
//		zamèen pro zápis)
void CSUnitType::AddUnit ( CSUnit *pUnit ) 
{
	ASSERT ( pUnit != NULL );

	// pøidá jednotku na zaèátek seznamu jednotek
	pUnit->m_pNextUnit = m_pFirstUnit;
	m_pFirstUnit = pUnit->m_pNextUnit;
}

// odebere jednotku "pUnit" ze seznamu jednotek typu jednotky (typ jednotky musí být 
//		zamèen pro zápis) (není-li jednotka v seznamu jednotek, neprovádí nic)
void CSUnitType::DeleteUnit ( CSUnit *pUnit ) 
{
	ASSERT ( pUnit != NULL );

	// ukazatel na odkaz na jednotku v seznamu
	CSUnit **ppUnit = &m_pFirstUnit;

	// projede seznam jednotek
	while ( *ppUnit != NULL )
	{
		// zjistí, jedná-li se o hledanou jednotku
		if ( *ppUnit == pUnit )
		{	// jedná se o hledanou jednotku
			// vypojí jednotku ze seznamu jednotek typu jednotky
			*ppUnit = pUnit->m_pNextUnit;
			pUnit->m_pNextUnit = NULL;
			// ukonèí odebírání jednotky ze seznamu jednotek typu jednotky
			return;
		}

		// pøejde na další jednotku seznamu
		ppUnit = &(*ppUnit)->m_pNextUnit;
	}
}

//////////////////////////////////////////////////////////////////////
// Debuggovací informace
//////////////////////////////////////////////////////////////////////

#ifdef _DEBUG

// zkontroluje správná data objektu (TRUE=OK)
BOOL CSUnitType::CheckValidData () 
{
	ASSERT ( m_pCivilization != NULL );

	ASSERT ( m_dwSkillTypeCount > 0 );
	ASSERT ( m_aSkillTypeDescription != NULL );
	ASSERT ( m_dwSkillDataSize > 0 );

	ASSERT ( m_nLivesMax > 0 );
	ASSERT ( m_dwVerticalPosition != 0 );
	ASSERT ( !m_strUnitTypeName.IsEmpty () );

	return TRUE;
}

// zkontroluje neškodná data objektu (TRUE=OK)
BOOL CSUnitType::CheckEmptyData () 
{
	ASSERT ( m_dwID == DWORD_MAX );
	ASSERT ( m_pCivilization == NULL );

	ASSERT ( m_dwSkillTypeCount == 0 );
	ASSERT ( m_aSkillTypeDescription == NULL );
	ASSERT ( m_dwSkillDataSize == 0 );

	ASSERT ( m_nLivesMax == 0 );
	ASSERT ( m_dwViewRadius == 0 );
	ASSERT ( m_dwVerticalPosition == 0 );
	ASSERT ( m_strUnitTypeName.IsEmpty () );

	for ( int nModeIndex = 8; nModeIndex-- > 0; )
	{
		ASSERT ( m_aMoveWidth[nModeIndex] == 0 );
		ASSERT ( m_aAppearanceID[nModeIndex] == DWORD_MAX );
		ASSERT ( m_aInvisibilityState[nModeIndex] == 0 );
		ASSERT ( m_aInvisibilityDetection[nModeIndex] == 0 );
	}
	ASSERT ( m_dwFlags == 0 );

	return TRUE;
}

#endif //_DEBUG
