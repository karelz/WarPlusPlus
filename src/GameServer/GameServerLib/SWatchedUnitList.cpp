/***********************************************************
 * 
 * Projekt: Strategick� hra
 *    ��st: Server hry
 *   Autor: Karel Zikmund
 * 
 *   Popis: T��da seznamu sledovan�ch jednotek
 * 
 ***********************************************************/

#include "StdAfx.h"

#include "SWatchedUnitList.h"
#include "SUnit.h"
#include "SUnit.inl"

//////////////////////////////////////////////////////////////////////
// Memory pool seznamu sledovan�ch jednotek
//////////////////////////////////////////////////////////////////////

// memory pool seznamu sledovan�ch jednotek
CTypedMemoryPool<struct CSWatchedUnitList::SListMember> 
	CSWatchedUnitList::m_cPool ( 50 );

//////////////////////////////////////////////////////////////////////
// Operace se seznamem
//////////////////////////////////////////////////////////////////////

// najde jednotku "dwID" sledovanou klientem "pClient" v seznamu a vr�t� jej� 
//		pozici (NULL=nenalezeno)
POSITION CSWatchedUnitList::FindUnitByID ( DWORD dwID, 
	CZClientUnitInfoSender *pClient ) const 
{
	// ukazatel na odkaz na prvek seznamu
	struct SListMember * const *pMember = &m_pFirst;

	// projede seznam
	while ( *pMember != NULL )
	{
		// zjist�, jedn�-li se o hledan� prvek
		if ( ( (*pMember)->cElement.pClient == pClient ) && 
			( (*pMember)->cElement.pUnit->GetID () == dwID ) )
		{	// jedn� se o hledan� prvek
			return (POSITION)pMember;
		}

		// posune ukazatel na dal�� prvek seznamu
		pMember = &(*pMember)->pNext;
	}
	// prvek nebyl nalezen
	return NULL;
}

// najde jednotku "pUnit" v seznamu od pozice "rPosition" a vr�t� jej� pozici 
//		(FALSE=nenalezeno)
BOOL CSWatchedUnitList::FindUnit ( CSUnit *pUnit, POSITION &rPosition ) 
{
	// ukazatel na odkaz na prvek seznamu
	struct SListMember **pMember = (SListMember **)rPosition;

	// projede seznam
	while ( *pMember != NULL )
	{
		// zjist�, jedn�-li se o hledan� prvek
		if ( (*pMember)->cElement.pUnit == pUnit )
		{	// jedn� se o hledan� prvek
			// vr�t� p��znak nalezen� jednotky
			rPosition = (POSITION)pMember;
			return TRUE;
		}

		// posune ukazatel na dal�� prvek seznamu
		pMember = &(*pMember)->pNext;
	}

	// vr�t� p��znak nenalezen� jednotky
	rPosition = (POSITION)pMember;
	return FALSE;
}
