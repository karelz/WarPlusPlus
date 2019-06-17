/***********************************************************
 * 
 * Projekt: Strategická hra
 *    Èást: Server hry
 *   Autor: Roman Zenka
 * 
 *   Popis: Tøída odloženého volání procedury GetUnitsInArea 
 *          na systémovém objektu mapy
 * 
 ***********************************************************/

#include "StdAfx.h"
#include "SMapGetUnitsInAreaDPC.h"
#include "GameServer\GameServer\SDeferredProcedureCall.inl"

#include "GameServer\Skills\SysCalls\IDPCSysCall.h"
#include "TripleS\Interpret\Src\SysObjs\ISOSet.h"
#include "GameServer\GameServer\SUnitList.h"

IMPLEMENT_DYNAMIC ( CSMapGetUnitsInAreaDPC, CSSysCallDPC )

// implementace tøídy, jejíž potomky lze vytváøet za bìhu programu z ID potomka
IMPLEMENT_DPC ( CSMapGetUnitsInAreaDPC, 0 )

//////////////////////////////////////////////////////////////////////
// Konstrukce a destrukce
//////////////////////////////////////////////////////////////////////

// konstruktor
CSMapGetUnitsInAreaDPC::CSMapGetUnitsInAreaDPC () 
{
	m_pointCenter = CPointDW ( 0, 0 );
	m_dwRadius = 0;
	m_eSelector = SelectorNotInitialized;
}

// konstruktor
CSMapGetUnitsInAreaDPC::CSMapGetUnitsInAreaDPC ( CIProcess *pProcess, 
	CIDataStackItem *pReturnValue, CPointDW pointCenter, DWORD dwRadius, 
	enum ESelector eSelector ) : CSSysCallDPC ( pProcess, pReturnValue ) 
{
	// zkopirujeme parametry
	ASSERT ( g_cMap.IsMapPosition ( pointCenter ) );
	m_pointCenter = pointCenter;
	ASSERT ( dwRadius <= MAX_GET_UNITS_IN_AREA_RADIUS );
	m_dwRadius = dwRadius;
	m_eSelector = eSelector;
}

// destruktor
CSMapGetUnitsInAreaDPC::~CSMapGetUnitsInAreaDPC () 
{
}

//////////////////////////////////////////////////////////////////////
// Ukládání dat potomkù (analogie CPersistentObjectPlain metod)
//////////////////////////////////////////////////////////////////////

/* CSMapGetUnitsInAreaDPC stored

	CSSysCallDPC stored
	DWORD m_pointCenter.x
	DWORD m_pointCenter.y
	DWORD m_dwRadius
	enum ESelector m_eSelector
*/

// ukládání dat
void CSMapGetUnitsInAreaDPC::PersistentSaveDPC ( CPersistentStorage &storage ) 
{
    BRACE_BLOCK(storage);

	ASSERT ( ( m_eSelector == AllUnitsInArea ) || ( m_eSelector == MyUnitsInArea ) || 
		( m_eSelector == EnemyUnitsInArea ) );
	ASSERT ( m_dwRadius <= MAX_GET_UNITS_IN_AREA_RADIUS );
	ASSERT ( g_cMap.IsMapPosition ( m_pointCenter ) );

	CSSysCallDPC::PersistentSaveDPC ( storage );

	storage << m_pointCenter.x << m_pointCenter.y << m_dwRadius << (int)m_eSelector;
}

// nahrávání pouze uložených dat
void CSMapGetUnitsInAreaDPC::PersistentLoadDPC ( CPersistentStorage &storage ) 
{
    BRACE_BLOCK(storage);

	// zavolame predka
	CSSysCallDPC::PersistentLoadDPC ( storage );

	// natahneme vlastni data
	storage >> m_pointCenter.x >> m_pointCenter.y >> m_dwRadius >> (int &)m_eSelector;

	// otestujeme jejich spravnost
	LOAD_ASSERT ( m_dwRadius <= MAX_GET_UNITS_IN_AREA_RADIUS );
	LOAD_ASSERT ( ( m_eSelector == AllUnitsInArea ) || 
		( m_eSelector == MyUnitsInArea ) || ( m_eSelector == EnemyUnitsInArea ) );
}

// preklad ukazatelu
void CSMapGetUnitsInAreaDPC::PersistentTranslatePointersDPC ( 
	CPersistentStorage &storage )
{
	// pouze zavolame predka
	CSSysCallDPC::PersistentTranslatePointersDPC ( storage );
}

// inicializace nahraneho objektu
void CSMapGetUnitsInAreaDPC::PersistentInitDPC () 
{
	LOAD_ASSERT ( g_cMap.CheckLoadedMapPosition ( m_pointCenter ) );

	// pouze zavolame predka
	CSSysCallDPC::PersistentInitDPC ();
}

//////////////////////////////////////////////////////////////////////
// Operace s DPC
//////////////////////////////////////////////////////////////////////

// provede odložené volání procedury (stará se o zneplatnìní všech odkazù na DPC)
void CSMapGetUnitsInAreaDPC::Run () 
{
	// seznam jednotek v oblasti
	CSUnitList cUnitList;

	// zjistí požadované jednotky v zadané oblasti
	switch ( m_eSelector )
	{
		// vlastí jednotky
		case MyUnitsInArea :
			g_cMap.GetUnitsInAreaOfMine ( GetRunningDPCCivilizationIndex (), 
				m_pointCenter, m_dwRadius, cUnitList);
			break;
		// nepøátelské jednotky
		case EnemyUnitsInArea :
			g_cMap.GetUnitsInAreaEnemy ( GetRunningDPCCivilizationIndex (), 
				m_pointCenter, m_dwRadius, cUnitList );
			break;
		// všechny jednotky
		case AllUnitsInArea :
			g_cMap.GetUnitsInAreaAll ( GetRunningDPCCivilizationIndex (), 
				m_pointCenter, m_dwRadius, cUnitList );
			break;
		// nemožný pøípad
		default:
			ASSERT ( FALSE );
	}

	// zamkne systémové volání
	VERIFY ( m_pDPCSysCall->Lock () );

	// zjistí, je-li systémové volání platné
	if ( m_pDPCSysCall->IsValid () )
	{	// systémové volání je platné
		// ukazatel na množinu vracených jednotek
		CISOSet<CZUnit *> *pSet = (CISOSet<CZUnit *> *)m_pDPCSysCall->GetReturnValue ()->GetSystemObject ();
		ASSERT ( pSet != NULL );

		// jednotka ze seznamu jednotek
		CSUnit *pUnit;

		// vyplní množinu vracených jednotek
		while ( cUnitList.RemoveFirst ( pUnit ) )
		{
			// pøiá jednotku do množiny vracených jednotek
			pSet->AddItemToSet ( pUnit->GetZUnit () );
		}
	}
	else
	{	// systémové volání je neplatné
		// znièí seznam jednotek
		cUnitList.RemoveAll ();
	}

	// odemkne systémové volání
	VERIFY ( m_pDPCSysCall->Unlock () );

	// ukonèí systémové volání (vèetnì odemèení systémového volání)
	m_pDPCSysCall->FinishWaiting ( STATUS_SUCCESS );
	m_pDPCSysCall->Release();
	m_pDPCSysCall = NULL;
}


//////////////////////////////////////////////////////////////////////
// Debuggovaci metody
//////////////////////////////////////////////////////////////////////

#ifdef _DEBUG

void CSMapGetUnitsInAreaDPC::AssertValid () 
{
	// zavolame predka
	CSSysCallDPC::AssertValid ();

    // otestujeme vlastni membery
	ASSERT ( g_cMap.IsMapPosition ( m_pointCenter ) );
	ASSERT ( m_dwRadius <= MAX_GET_UNITS_IN_AREA_RADIUS );
	ASSERT ( ( m_eSelector == AllUnitsInArea ) || ( m_eSelector == MyUnitsInArea ) || 
		( m_eSelector == EnemyUnitsInArea ) );
}

#endif  // _DEBUG
