/***********************************************************
 * 
 * Projekt: Strategická hra
 *    Èást: Server hry
 *   Autor: Karel Zikmund
 * 
 *   Popis: Tøída seznamu pozic kontrolních bodù
 * 
 ***********************************************************/

#ifndef __SERVER_CHECK_POINT_POSITION_LIST__HEADER_INCLUDED__
#define __SERVER_CHECK_POINT_POSITION_LIST__HEADER_INCLUDED__

#include "Common\MemoryPool\PooledList.h"

//////////////////////////////////////////////////////////////////////
// seznam pozic kontrolních bodù
typedef CPooledList<CPointDW, 0> CSCheckPointPositionList;

#endif //__SERVER_CHECK_POINT_POSITION_LIST__HEADER_INCLUDED__
