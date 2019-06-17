/***********************************************************
 * 
 * Projekt: Strategická hra
 *    Èást: Server hry
 *   Autor: Karel Zikmund
 * 
 *   Popis: Tøída fronty požadavkù na hledání cesty
 * 
 ***********************************************************/

#ifndef __SERVER_FIND_PATH_REQUEST_QUEUE__HEADER_INCLUDED__
#define __SERVER_FIND_PATH_REQUEST_QUEUE__HEADER_INCLUDED__

#include "Common\MemoryPool\PooledQueue.h"

#include "AbstractDataClasses\256BitArray.h"
#include "GeneralClasses\GeneralClasses.h"

//////////////////////////////////////////////////////////////////////
// Dopøedné deklarace

// tøída cesty na serveru hry
class CSPath;
// tøída grafu pro hledání cesty
class CSFindPathGraph;
// tøída jednotky
class CSUnit;

//////////////////////////////////////////////////////////////////////
// Datové typy

// informace o požadavku na nalezení cesty
struct SFindPathRequestInfo 
{
	// pozice, odkud se má cesta hledat
	CPointDW pointPosition;
	// pozice, kam se má cesta hledat
	CPointDW pointDestination;
	// ukazatel na graf pro hledání cesty
	CSFindPathGraph *pFindPathGraph;
	// šíøka cesty
	DWORD dwWidth;
	// ukazatel na jednotku (NULL=normální cesta, jinak lokální cesta)
	CSUnit *pUnit;
	// výsledná cesta
	CSPath *pPath;
};

//////////////////////////////////////////////////////////////////////
// Tøída fronty požadavkù na hledání cesty
class CSFindPathRequestQueue : public CPooledQueue<struct SFindPathRequestInfo, 0> 
{
// Metody
public:
// Operace s frontou

	// smaže požadavek na hledání cesty "pPath" (TRUE=požadavek byl smazán)
	BOOL RemoveFindPathRequest ( CSPath *pPath );
};

#endif //__SERVER_FIND_PATH_REQUEST_QUEUE__HEADER_INCLUDED__
