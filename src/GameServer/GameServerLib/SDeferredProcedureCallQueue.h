/***********************************************************
 * 
 * Projekt: Strategická hra
 *    Èást: Server hry
 *   Autor: Karel Zikmund
 * 
 *   Popis: Tøída fronty odložených volání procedur
 * 
 ***********************************************************/

#ifndef __SERVER_DEFERRED_PROCEDURE_CALL_QUEUE__HEADER_INCLUDED__
#define __SERVER_DEFERRED_PROCEDURE_CALL_QUEUE__HEADER_INCLUDED__

#include "Common\MemoryPool\PooledQueue.h"

//////////////////////////////////////////////////////////////////////
// Dopøedné deklarace

// tøída odloženého volání procedury (DPC)
class CSDeferredProcedureCall;

//////////////////////////////////////////////////////////////////////
// Tøída fronty odložených volání procedur
typedef CPooledQueue<CSDeferredProcedureCall *, 0> CSDeferredProcedureCallQueue;

#endif //__SERVER_DEFERRED_PROCEDURE_CALL_QUEUE__HEADER_INCLUDED__
