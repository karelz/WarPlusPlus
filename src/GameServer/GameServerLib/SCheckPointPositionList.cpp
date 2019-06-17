/***********************************************************
 * 
 * Projekt: Strategick� hra
 *    ��st: Server hry
 *   Autor: Karel Zikmund
 * 
 *   Popis: T��da seznamu pozic kontroln�ch bod� cesty
 * 
 ***********************************************************/

#include "StdAfx.h"

#include "SCheckPointPositionList.h"

//////////////////////////////////////////////////////////////////////
// Memory pool seznamu pozic kontroln�ch bod� cesty

// memory pool seznamu pozic kontroln�ch bod� cesty
CTypedMemoryPool<struct CSCheckPointPositionList::SListMember> 
	CSCheckPointPositionList::m_cPool ( 1000 );
