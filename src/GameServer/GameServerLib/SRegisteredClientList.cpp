/***********************************************************
 * 
 * Projekt: Strategická hra
 *    Èást: Server hry
 *   Autor: Karel Zikmund
 * 
 *   Popis: Tøída seznamu zaregistrovaných klientù civilizace
 * 
 ***********************************************************/

#include "StdAfx.h"

#include "SRegisteredClientList.h"

//////////////////////////////////////////////////////////////////////
// Memory pool seznamu zaregistrovaných klientù civilizace
//////////////////////////////////////////////////////////////////////

// memory pool seznamu zaregistrovaných klientù civilizace
CTypedMemoryPool<struct CSRegisteredClientList::SListMember> 
	CSRegisteredClientList::m_cPool ( 2 * CIVILIZATION_COUNT_MAX );

//////////////////////////////////////////////////////////////////////
// Operace se seznamem
//////////////////////////////////////////////////////////////////////

// najde klienta "pClient" v seznamu a vrátí jeho pozici (NULL=nenalezeno)
POSITION CSRegisteredClientList::FindClient ( CZClientUnitInfoSender *pClient ) const 
{
	// ukazatel na odkaz na prvek seznamu
	struct SListMember * const *pMember = &m_pFirst;

	// projede seznam
	while ( *pMember != NULL )
	{
		// zjistí, jedná-li se o hledaný prvek
		if ( (*pMember)->cElement.pClient == pClient )
		{	// jedná se o hledaný prvek
			return (POSITION)pMember;
		}

		// posune ukazatel na další prvek seznamu
		pMember = &(*pMember)->pNext;
	}
	// prvek nebyl nalezen
	return NULL;
}
