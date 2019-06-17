/***********************************************************
 * 
 * Projekt: Strategická hra
 *    Èást: Server hry
 *   Autor: Karel Zikmund
 * 
 *   Popis: Tøída seznamu sledovaných jednotek
 * 
 ***********************************************************/

#include "StdAfx.h"

#include "SWatchedUnitList.h"
#include "SUnit.h"
#include "SUnit.inl"

//////////////////////////////////////////////////////////////////////
// Memory pool seznamu sledovaných jednotek
//////////////////////////////////////////////////////////////////////

// memory pool seznamu sledovaných jednotek
CTypedMemoryPool<struct CSWatchedUnitList::SListMember> 
	CSWatchedUnitList::m_cPool ( 50 );

//////////////////////////////////////////////////////////////////////
// Operace se seznamem
//////////////////////////////////////////////////////////////////////

// najde jednotku "dwID" sledovanou klientem "pClient" v seznamu a vrátí její 
//		pozici (NULL=nenalezeno)
POSITION CSWatchedUnitList::FindUnitByID ( DWORD dwID, 
	CZClientUnitInfoSender *pClient ) const 
{
	// ukazatel na odkaz na prvek seznamu
	struct SListMember * const *pMember = &m_pFirst;

	// projede seznam
	while ( *pMember != NULL )
	{
		// zjistí, jedná-li se o hledaný prvek
		if ( ( (*pMember)->cElement.pClient == pClient ) && 
			( (*pMember)->cElement.pUnit->GetID () == dwID ) )
		{	// jedná se o hledaný prvek
			return (POSITION)pMember;
		}

		// posune ukazatel na další prvek seznamu
		pMember = &(*pMember)->pNext;
	}
	// prvek nebyl nalezen
	return NULL;
}

// najde jednotku "pUnit" v seznamu od pozice "rPosition" a vrátí její pozici 
//		(FALSE=nenalezeno)
BOOL CSWatchedUnitList::FindUnit ( CSUnit *pUnit, POSITION &rPosition ) 
{
	// ukazatel na odkaz na prvek seznamu
	struct SListMember **pMember = (SListMember **)rPosition;

	// projede seznam
	while ( *pMember != NULL )
	{
		// zjistí, jedná-li se o hledaný prvek
		if ( (*pMember)->cElement.pUnit == pUnit )
		{	// jedná se o hledaný prvek
			// vrátí pøíznak nalezení jednotky
			rPosition = (POSITION)pMember;
			return TRUE;
		}

		// posune ukazatel na další prvek seznamu
		pMember = &(*pMember)->pNext;
	}

	// vrátí pøíznak nenalezení jednotky
	rPosition = (POSITION)pMember;
	return FALSE;
}
