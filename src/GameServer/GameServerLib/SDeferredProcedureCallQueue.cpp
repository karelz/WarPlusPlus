/***********************************************************
 * 
 * Projekt: Strategická hra
 *    Èást: Server hry
 *   Autor: Karel Zikmund
 * 
 *   Popis: Tøída fronty odložených volání procedur
 * 
 ***********************************************************/

#include "StdAfx.h"

#include "SDeferredProcedureCallQueue.h"

//////////////////////////////////////////////////////////////////////
// Memory pool fronty odložených volání procedur
//////////////////////////////////////////////////////////////////////

// memory pool fronty odložených volání procedur
CTypedMemoryPool<struct CSDeferredProcedureCallQueue::SQueueMember> 
	CSDeferredProcedureCallQueue::m_cPool ( 50 );
