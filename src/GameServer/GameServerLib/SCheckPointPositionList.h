/***********************************************************
 * 
 * Projekt: Strategick� hra
 *    ��st: Server hry
 *   Autor: Karel Zikmund
 * 
 *   Popis: T��da seznamu pozic kontroln�ch bod�
 * 
 ***********************************************************/

#ifndef __SERVER_CHECK_POINT_POSITION_LIST__HEADER_INCLUDED__
#define __SERVER_CHECK_POINT_POSITION_LIST__HEADER_INCLUDED__

#include "Common\MemoryPool\PooledList.h"

//////////////////////////////////////////////////////////////////////
// seznam pozic kontroln�ch bod�
typedef CPooledList<CPointDW, 0> CSCheckPointPositionList;

#endif //__SERVER_CHECK_POINT_POSITION_LIST__HEADER_INCLUDED__
