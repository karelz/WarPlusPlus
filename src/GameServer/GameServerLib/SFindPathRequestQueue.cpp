/***********************************************************
 * 
 * Projekt: Strategická hra
 *    Èást: Server hry
 *   Autor: Karel Zikmund
 * 
 *   Popis: Tøída fronty požadavkù na hledání cesty
 * 
 ***********************************************************/

#include "StdAfx.h"

#include "SFindPathRequestQueue.h"

//////////////////////////////////////////////////////////////////////
// Memory pool fronty požadavkù na hledání cesty
//////////////////////////////////////////////////////////////////////

// memory pool fronty požadavkù na hledání cesty
CTypedMemoryPool<struct CSFindPathRequestQueue::SQueueMember> 
	CSFindPathRequestQueue::m_cPool ( 50 );

//////////////////////////////////////////////////////////////////////
// Operace s frontou
//////////////////////////////////////////////////////////////////////

// smaže požadavek na hledání cesty "pPath" (TRUE=požadavek byl smazán)
BOOL CSFindPathRequestQueue::RemoveFindPathRequest ( CSPath *pPath ) 
{
	// ukazatel na odkaz na první prvek fronty
	struct SQueueMember **pMember = &m_pFirst;

	// projede seznam prvkù fronty
	while ( *pMember != NULL )
	{
		// zjistí, jedná-li se o mazaný požadacek na hledání cesty
		if ( (*pMember)->cElement.pPath == pPath )
		{	// jedná se o mazaný požadavek na hledání cesty
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

			// vrátí pøíznak smazání požadavku na hledání cesty
			return TRUE;
		}

		// posune se na další prvek seznamu fronty
		pMember = &(*pMember)->pNext;
	}

	// vrátí pøíznak nenalezení mazaného požadavku na hledání cesty
	return FALSE;
}
