/***********************************************************
 * 
 * Projekt: Strategická hra
 *    Èást: Server hry
 *   Autor: Karel Zikmund
 * 
 *   Popis: Tøída fronty požadavkù na sledování jednotek
 * 
 ***********************************************************/

#include "StdAfx.h"

#include "SWatchingUnitRequestQueue.h"

//////////////////////////////////////////////////////////////////////
// Memory pool fronty požadavkù na sledování jednotek
//////////////////////////////////////////////////////////////////////

// memory pool fronty požadavkù na sledování jednotek
CTypedMemoryPool<struct CSWatchingUnitRequestQueue::SQueueMember> 
	CSWatchingUnitRequestQueue::m_cPool ( 30 );
