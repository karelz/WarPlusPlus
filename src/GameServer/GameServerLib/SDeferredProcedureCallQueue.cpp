/***********************************************************
 * 
 * Projekt: Strategick� hra
 *    ��st: Server hry
 *   Autor: Karel Zikmund
 * 
 *   Popis: T��da fronty odlo�en�ch vol�n� procedur
 * 
 ***********************************************************/

#include "StdAfx.h"

#include "SDeferredProcedureCallQueue.h"

//////////////////////////////////////////////////////////////////////
// Memory pool fronty odlo�en�ch vol�n� procedur
//////////////////////////////////////////////////////////////////////

// memory pool fronty odlo�en�ch vol�n� procedur
CTypedMemoryPool<struct CSDeferredProcedureCallQueue::SQueueMember> 
	CSDeferredProcedureCallQueue::m_cPool ( 50 );
