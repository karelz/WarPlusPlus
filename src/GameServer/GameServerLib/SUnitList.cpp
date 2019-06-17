/***********************************************************
 * 
 * Projekt: Strategická hra
 *    Èást: Server hry
 *   Autor: Karel Zikmund
 * 
 *   Popis: Tøída seznamu jednotek
 * 
 ***********************************************************/

#include "StdAfx.h"

#include "SUnitList.h"

//////////////////////////////////////////////////////////////////////
// Memory pool seznamu jednotek

// memory pool seznamu jednotek na mapì
CTypedMemoryPool<struct CSUnitList::SListMember> CSUnitList::m_cPool ( 50 );
