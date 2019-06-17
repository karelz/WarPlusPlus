/***********************************************************
 * 
 * Projekt: Strategick� hra
 *    ��st: Server hry
 *   Autor: Karel Zikmund
 * 
 *   Popis: T��da seznamu jednotek
 * 
 ***********************************************************/

#ifndef __SERVER_UNIT_LIST__HEADER_INCLUDED__
#define __SERVER_UNIT_LIST__HEADER_INCLUDED__

#include "Common\MemoryPool\PooledList.h"

//////////////////////////////////////////////////////////////////////
// Dop�edn� deklarace

// t��da jednotky
class CSUnit;

//////////////////////////////////////////////////////////////////////
// Datov� typy

// seznam jednotek na map�
typedef CPooledList<CSUnit *, 0> CSUnitList;

#endif //__SERVER_UNIT_LIST__HEADER_INCLUDED__
