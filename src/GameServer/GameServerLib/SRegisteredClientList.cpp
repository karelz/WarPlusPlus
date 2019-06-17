/***********************************************************
 * 
 * Projekt: Strategick� hra
 *    ��st: Server hry
 *   Autor: Karel Zikmund
 * 
 *   Popis: T��da seznamu zaregistrovan�ch klient� civilizace
 * 
 ***********************************************************/

#include "StdAfx.h"

#include "SRegisteredClientList.h"

//////////////////////////////////////////////////////////////////////
// Memory pool seznamu zaregistrovan�ch klient� civilizace
//////////////////////////////////////////////////////////////////////

// memory pool seznamu zaregistrovan�ch klient� civilizace
CTypedMemoryPool<struct CSRegisteredClientList::SListMember> 
	CSRegisteredClientList::m_cPool ( 2 * CIVILIZATION_COUNT_MAX );

//////////////////////////////////////////////////////////////////////
// Operace se seznamem
//////////////////////////////////////////////////////////////////////

// najde klienta "pClient" v seznamu a vr�t� jeho pozici (NULL=nenalezeno)
POSITION CSRegisteredClientList::FindClient ( CZClientUnitInfoSender *pClient ) const 
{
	// ukazatel na odkaz na prvek seznamu
	struct SListMember * const *pMember = &m_pFirst;

	// projede seznam
	while ( *pMember != NULL )
	{
		// zjist�, jedn�-li se o hledan� prvek
		if ( (*pMember)->cElement.pClient == pClient )
		{	// jedn� se o hledan� prvek
			return (POSITION)pMember;
		}

		// posune ukazatel na dal�� prvek seznamu
		pMember = &(*pMember)->pNext;
	}
	// prvek nebyl nalezen
	return NULL;
}
