/***********************************************************
 * 
 * Projekt: Strategick� hra
 *    ��st: Server hry
 *   Autor: Karel Zikmund
 * 
 *   Popis: T��da fronty po�adavk� na hled�n� cesty
 * 
 ***********************************************************/

#ifndef __SERVER_FIND_PATH_REQUEST_QUEUE__HEADER_INCLUDED__
#define __SERVER_FIND_PATH_REQUEST_QUEUE__HEADER_INCLUDED__

#include "Common\MemoryPool\PooledQueue.h"

#include "AbstractDataClasses\256BitArray.h"
#include "GeneralClasses\GeneralClasses.h"

//////////////////////////////////////////////////////////////////////
// Dop�edn� deklarace

// t��da cesty na serveru hry
class CSPath;
// t��da grafu pro hled�n� cesty
class CSFindPathGraph;
// t��da jednotky
class CSUnit;

//////////////////////////////////////////////////////////////////////
// Datov� typy

// informace o po�adavku na nalezen� cesty
struct SFindPathRequestInfo 
{
	// pozice, odkud se m� cesta hledat
	CPointDW pointPosition;
	// pozice, kam se m� cesta hledat
	CPointDW pointDestination;
	// ukazatel na graf pro hled�n� cesty
	CSFindPathGraph *pFindPathGraph;
	// ���ka cesty
	DWORD dwWidth;
	// ukazatel na jednotku (NULL=norm�ln� cesta, jinak lok�ln� cesta)
	CSUnit *pUnit;
	// v�sledn� cesta
	CSPath *pPath;
};

//////////////////////////////////////////////////////////////////////
// T��da fronty po�adavk� na hled�n� cesty
class CSFindPathRequestQueue : public CPooledQueue<struct SFindPathRequestInfo, 0> 
{
// Metody
public:
// Operace s frontou

	// sma�e po�adavek na hled�n� cesty "pPath" (TRUE=po�adavek byl smaz�n)
	BOOL RemoveFindPathRequest ( CSPath *pPath );
};

#endif //__SERVER_FIND_PATH_REQUEST_QUEUE__HEADER_INCLUDED__
