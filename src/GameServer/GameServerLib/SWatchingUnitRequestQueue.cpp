/***********************************************************
 * 
 * Projekt: Strategick� hra
 *    ��st: Server hry
 *   Autor: Karel Zikmund
 * 
 *   Popis: T��da fronty po�adavk� na sledov�n� jednotek
 * 
 ***********************************************************/

#include "StdAfx.h"

#include "SWatchingUnitRequestQueue.h"

//////////////////////////////////////////////////////////////////////
// Memory pool fronty po�adavk� na sledov�n� jednotek
//////////////////////////////////////////////////////////////////////

// memory pool fronty po�adavk� na sledov�n� jednotek
CTypedMemoryPool<struct CSWatchingUnitRequestQueue::SQueueMember> 
	CSWatchingUnitRequestQueue::m_cPool ( 30 );
