/***********************************************************
 * 
 * Projekt: Strategická hra
 *    Èást: Server hry
 *   Autor: Karel Zikmund
 * 
 *   Popis: Tøída fronty požadavkù na sledování MapSquarù
 * 
 ***********************************************************/

#include "StdAfx.h"

#include "SWatchingMapSquareRequestQueue.h"

//////////////////////////////////////////////////////////////////////
// Memory pool fronty požadavkù na sledování MapSquarù
//////////////////////////////////////////////////////////////////////

// memory pool fronty požadavkù na sledování MapSquarù
CTypedMemoryPool<struct CSWatchingMapSquareRequestQueue::SQueueMember> 
	CSWatchingMapSquareRequestQueue::m_cPool ( 30 );

//////////////////////////////////////////////////////////////////////
// Operace s frontou
//////////////////////////////////////////////////////////////////////

// smaže všechny požadavky na sledování MapSquaru klientem "pClient"
void CSWatchingMapSquareRequestQueue::RemoveAll ( CZClientUnitInfoSender *pClient ) 
{
	// ukazatel na odkaz na první prvek fronty
	struct SQueueMember **pMember = &m_pFirst;

	// projede seznam prvkù fronty
	while ( *pMember != NULL )
	{
		// zjistí, jedná-li se o mazaného klienta
		if ( (*pMember)->cElement.pClient == pClient )
		{	// jedná se o požadavek mazaného klienta
			// zjistí mazaný prvek seznamu fronty
			struct SQueueMember *pDeletedMember = *pMember;

			// vypojí prvek ze seznamu fronty
			*pMember = pDeletedMember->pNext;

			// smaže prvek seznamu fronty
			m_cPool.Free ( pDeletedMember );

			// zjistí, jedná-li se o poslední prvek fronty
			if ( pDeletedMember == m_pLast )
			{
				if ( m_pFirst == NULL )
				{
					m_pLast = NULL;
				}
				else
				{
					m_pLast = m_pFirst;
					while ( m_pLast->pNext != NULL )
					{
						m_pLast = m_pLast->pNext;
					}
				}
			}

			// pokraèuje v projíždìní seznamu fronty
			continue;
		}

		// posune se na další prvek seznamu fronty
		pMember = &(*pMember)->pNext;
	}
}
