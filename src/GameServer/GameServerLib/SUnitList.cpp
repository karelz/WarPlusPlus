/***********************************************************
 * 
 * Projekt: Strategick� hra
 *    ��st: Server hry
 *   Autor: Karel Zikmund
 * 
 *   Popis: T��da seznamu jednotek
 * 
 ***********************************************************/

#include "StdAfx.h"

#include "SUnitList.h"

//////////////////////////////////////////////////////////////////////
// Memory pool seznamu jednotek

// memory pool seznamu jednotek na map�
CTypedMemoryPool<struct CSUnitList::SListMember> CSUnitList::m_cPool ( 50 );
