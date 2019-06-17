/***********************************************************
 * 
 * Projekt: Strategick� hra
 *    ��st: Server hry
 *   Autor: Karel Zikmund
 * 
 *   Popis: T��da grafu pro hled�n� cesty
 * 
 ***********************************************************/

#ifndef __SERVER_FIND_PATH_GRAPH__INLINE_INCLUDED__
#define __SERVER_FIND_PATH_GRAPH__INLINE_INCLUDED__

#include "SFindPathGraph.h"

// vr�t� ukazatel na pozici "pointPosition" na map�
inline signed char *CSFindPathGraph::GetAt ( CPointDW pointPosition ) 
{
	// zjist�, jedn�-li se o pozici mimo mapu
	if ( ( (DWORD)pointPosition.x >= m_sizeMap.cx ) || 
		( (DWORD)pointPosition.y >= m_sizeMap.cy ) )
	{	// jedn� se o pozici mimo mapu
		return &m_cNoMapPosition;
	}

	// vr�t� ukazatel na pozici na map�
	return m_pMap + pointPosition.y * m_sizeMap.cx + pointPosition.x;
}

#endif //__SERVER_FIND_PATH_GRAPH__INLINE_INCLUDED__
