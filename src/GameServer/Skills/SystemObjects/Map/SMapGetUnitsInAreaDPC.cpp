/***********************************************************
 * 
 * Projekt: Strategick� hra
 *    ��st: Server hry
 *   Autor: Roman Zenka
 * 
 *   Popis: T��da odlo�en�ho vol�n� procedury GetUnitsInArea 
 *          na syst�mov�m objektu mapy
 * 
 ***********************************************************/

#include "StdAfx.h"
#include "SMapGetUnitsInAreaDPC.h"
#include "GameServer\GameServer\SDeferredProcedureCall.inl"

#include "GameServer\Skills\SysCalls\IDPCSysCall.h"
#include "TripleS\Interpret\Src\SysObjs\ISOSet.h"
#include "GameServer\GameServer\SUnitList.h"

IMPLEMENT_DYNAMIC ( CSMapGetUnitsInAreaDPC, CSSysCallDPC )

// implementace t��dy, jej� potomky lze vytv��et za b�hu programu z ID potomka
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
// Ukl�d�n� dat potomk� (analogie CPersistentObjectPlain metod)
//////////////////////////////////////////////////////////////////////

/* CSMapGetUnitsInAreaDPC stored

	CSSysCallDPC stored
	DWORD m_pointCenter.x
	DWORD m_pointCenter.y
	DWORD m_dwRadius
	enum ESelector m_eSelector
*/

// ukl�d�n� dat
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

// nahr�v�n� pouze ulo�en�ch dat
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

// provede odlo�en� vol�n� procedury (star� se o zneplatn�n� v�ech odkaz� na DPC)
void CSMapGetUnitsInAreaDPC::Run () 
{
	// seznam jednotek v oblasti
	CSUnitList cUnitList;

	// zjist� po�adovan� jednotky v zadan� oblasti
	switch ( m_eSelector )
	{
		// vlast� jednotky
		case MyUnitsInArea :
			g_cMap.GetUnitsInAreaOfMine ( GetRunningDPCCivilizationIndex (), 
				m_pointCenter, m_dwRadius, cUnitList);
			break;
		// nep��telsk� jednotky
		case EnemyUnitsInArea :
			g_cMap.GetUnitsInAreaEnemy ( GetRunningDPCCivilizationIndex (), 
				m_pointCenter, m_dwRadius, cUnitList );
			break;
		// v�echny jednotky
		case AllUnitsInArea :
			g_cMap.GetUnitsInAreaAll ( GetRunningDPCCivilizationIndex (), 
				m_pointCenter, m_dwRadius, cUnitList );
			break;
		// nemo�n� p��pad
		default:
			ASSERT ( FALSE );
	}

	// zamkne syst�mov� vol�n�
	VERIFY ( m_pDPCSysCall->Lock () );

	// zjist�, je-li syst�mov� vol�n� platn�
	if ( m_pDPCSysCall->IsValid () )
	{	// syst�mov� vol�n� je platn�
		// ukazatel na mno�inu vracen�ch jednotek
		CISOSet<CZUnit *> *pSet = (CISOSet<CZUnit *> *)m_pDPCSysCall->GetReturnValue ()->GetSystemObject ();
		ASSERT ( pSet != NULL );

		// jednotka ze seznamu jednotek
		CSUnit *pUnit;

		// vypln� mno�inu vracen�ch jednotek
		while ( cUnitList.RemoveFirst ( pUnit ) )
		{
			// p�i� jednotku do mno�iny vracen�ch jednotek
			pSet->AddItemToSet ( pUnit->GetZUnit () );
		}
	}
	else
	{	// syst�mov� vol�n� je neplatn�
		// zni�� seznam jednotek
		cUnitList.RemoveAll ();
	}

	// odemkne syst�mov� vol�n�
	VERIFY ( m_pDPCSysCall->Unlock () );

	// ukon�� syst�mov� vol�n� (v�etn� odem�en� syst�mov�ho vol�n�)
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
