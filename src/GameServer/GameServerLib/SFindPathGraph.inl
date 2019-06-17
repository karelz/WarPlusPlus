/***********************************************************
 * 
 * Projekt: Strategická hra
 *    Èást: Server hry
 *   Autor: Karel Zikmund
 * 
 *   Popis: Tøída grafu pro hledání cesty
 * 
 ***********************************************************/

#ifndef __SERVER_FIND_PATH_GRAPH__INLINE_INCLUDED__
#define __SERVER_FIND_PATH_GRAPH__INLINE_INCLUDED__

#include "SFindPathGraph.h"

// vrátí ukazatel na pozici "pointPosition" na mapì
inline signed char *CSFindPathGraph::GetAt ( CPointDW pointPosition ) 
{
	// zjistí, jedná-li se o pozici mimo mapu
	if ( ( (DWORD)pointPosition.x >= m_sizeMap.cx ) || 
		( (DWORD)pointPosition.y >= m_sizeMap.cy ) )
	{	// jedná se o pozici mimo mapu
		return &m_cNoMapPosition;
	}

	// vrátí ukazatel na pozici na mapì
	return m_pMap + pointPosition.y * m_sizeMap.cx + pointPosition.x;
}

#endif //__SERVER_FIND_PATH_GRAPH__INLINE_INCLUDED__
