/***********************************************************
 * 
 * Projekt: Strategická hra
 *    Èást: Server hry
 *   Autor: Karel Zikmund
 * 
 *   Popis: Tøída seznamu jednotek
 * 
 ***********************************************************/

#ifndef __SERVER_UNIT_LIST__HEADER_INCLUDED__
#define __SERVER_UNIT_LIST__HEADER_INCLUDED__

#include "Common\MemoryPool\PooledList.h"

//////////////////////////////////////////////////////////////////////
// Dopøedné deklarace

// tøída jednotky
class CSUnit;

//////////////////////////////////////////////////////////////////////
// Datové typy

// seznam jednotek na mapì
typedef CPooledList<CSUnit *, 0> CSUnitList;

#endif //__SERVER_UNIT_LIST__HEADER_INCLUDED__
