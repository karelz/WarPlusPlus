/***********************************************************
 * 
 * Projekt: Strategick� hra
 *    ��st: Server hry
 *   Autor: Karel Zikmund
 * 
 *   Popis: T��da fronty po�adavk� na sledov�n� MapSquar�
 * 
 ***********************************************************/

#include "StdAfx.h"

#include "SWatchingMapSquareRequestQueue.h"

//////////////////////////////////////////////////////////////////////
// Memory pool fronty po�adavk� na sledov�n� MapSquar�
//////////////////////////////////////////////////////////////////////

// memory pool fronty po�adavk� na sledov�n� MapSquar�
CTypedMemoryPool<struct CSWatchingMapSquareRequestQueue::SQueueMember> 
	CSWatchingMapSquareRequestQueue::m_cPool ( 30 );

//////////////////////////////////////////////////////////////////////
// Operace s frontou
//////////////////////////////////////////////////////////////////////

// sma�e v�echny po�adavky na sledov�n� MapSquaru klientem "pClient"
void CSWatchingMapSquareRequestQueue::RemoveAll ( CZClientUnitInfoSender *pClient ) 
{
	// ukazatel na odkaz na prvn� prvek fronty
	struct SQueueMember **pMember = &m_pFirst;

	// projede seznam prvk� fronty
	while ( *pMember != NULL )
	{
		// zjist�, jedn�-li se o mazan�ho klienta
		if ( (*pMember)->cElement.pClient == pClient )
		{	// jedn� se o po�adavek mazan�ho klienta
			// zjist� mazan� prvek seznamu fronty
			struct SQueueMember *pDeletedMember = *pMember;

			// vypoj� prvek ze seznamu fronty
			*pMember = pDeletedMember->pNext;

			// sma�e prvek seznamu fronty
			m_cPool.Free ( pDeletedMember );

			// zjist�, jedn�-li se o posledn� prvek fronty
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

			// pokra�uje v proj�d�n� seznamu fronty
			continue;
		}

		// posune se na dal�� prvek seznamu fronty
		pMember = &(*pMember)->pNext;
	}
}
