/***********************************************************
 * 
 * Projekt: Strategick� hra
 *    ��st: Server hry
 *   Autor: Karel Zikmund
 * 
 *   Popis: T��da fronty po�adavk� na hled�n� cesty
 * 
 ***********************************************************/

#include "StdAfx.h"

#include "SFindPathRequestQueue.h"

//////////////////////////////////////////////////////////////////////
// Memory pool fronty po�adavk� na hled�n� cesty
//////////////////////////////////////////////////////////////////////

// memory pool fronty po�adavk� na hled�n� cesty
CTypedMemoryPool<struct CSFindPathRequestQueue::SQueueMember> 
	CSFindPathRequestQueue::m_cPool ( 50 );

//////////////////////////////////////////////////////////////////////
// Operace s frontou
//////////////////////////////////////////////////////////////////////

// sma�e po�adavek na hled�n� cesty "pPath" (TRUE=po�adavek byl smaz�n)
BOOL CSFindPathRequestQueue::RemoveFindPathRequest ( CSPath *pPath ) 
{
	// ukazatel na odkaz na prvn� prvek fronty
	struct SQueueMember **pMember = &m_pFirst;

	// projede seznam prvk� fronty
	while ( *pMember != NULL )
	{
		// zjist�, jedn�-li se o mazan� po�adacek na hled�n� cesty
		if ( (*pMember)->cElement.pPath == pPath )
		{	// jedn� se o mazan� po�adavek na hled�n� cesty
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

			// vr�t� p��znak smaz�n� po�adavku na hled�n� cesty
			return TRUE;
		}

		// posune se na dal�� prvek seznamu fronty
		pMember = &(*pMember)->pNext;
	}

	// vr�t� p��znak nenalezen� mazan�ho po�adavku na hled�n� cesty
	return FALSE;
}
