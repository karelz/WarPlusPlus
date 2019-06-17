/***********************************************************
 * 
 * Projekt: Strategick� hra
 *    ��st: Server hry
 *   Autor: Karel Zikmund
 * 
 *   Popis: T��da fronty odlo�en�ch vol�n� procedur
 * 
 ***********************************************************/

#ifndef __SERVER_DEFERRED_PROCEDURE_CALL_QUEUE__HEADER_INCLUDED__
#define __SERVER_DEFERRED_PROCEDURE_CALL_QUEUE__HEADER_INCLUDED__

#include "Common\MemoryPool\PooledQueue.h"

//////////////////////////////////////////////////////////////////////
// Dop�edn� deklarace

// t��da odlo�en�ho vol�n� procedury (DPC)
class CSDeferredProcedureCall;

//////////////////////////////////////////////////////////////////////
// T��da fronty odlo�en�ch vol�n� procedur
typedef CPooledQueue<CSDeferredProcedureCall *, 0> CSDeferredProcedureCallQueue;

#endif //__SERVER_DEFERRED_PROCEDURE_CALL_QUEUE__HEADER_INCLUDED__
