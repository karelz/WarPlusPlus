/***********************************************************
 * 
 * Projekt: Strategická hra
 *    Èást: Server hry
 *   Autor: Karel Zikmund
 * 
 *   Popis: Tøída seznamu pozic kontrolních bodù cesty
 * 
 ***********************************************************/

#include "StdAfx.h"

#include "SCheckPointPositionList.h"

//////////////////////////////////////////////////////////////////////
// Memory pool seznamu pozic kontrolních bodù cesty

// memory pool seznamu pozic kontrolních bodù cesty
CTypedMemoryPool<struct CSCheckPointPositionList::SListMember> 
	CSCheckPointPositionList::m_cPool ( 1000 );
