/***********************************************************
 * 
 * Projekt: Strategick� hra
 *    ��st: Server hry
 *   Autor: Karel Zikmund
 * 
 *   Popis: T��da fronty po�adavk� na sledov�n� jednotek
 * 
 ***********************************************************/

#ifndef __SERVER_WATCHING_UNIT_REQUEST_QUEUE__HEADER_INCLUDED__
#define __SERVER_WATCHING_UNIT_REQUEST_QUEUE__HEADER_INCLUDED__

//////////////////////////////////////////////////////////////////////
// Dop�edn� deklarace

// t��da rozes�latele informac� o jednotk�ch klientovi civilizace
class CZClientUnitInfoSender;
// t��da jednotky
class CSUnit;

//////////////////////////////////////////////////////////////////////
// Datov� typy

// informace o po�adavku na sledov�n� jednotky
struct SWatchingUnitRequestInfo 
{
	// typ po�adavku na sledov�n� jednotky (StartWatchingUnit/StopWatchingUnit)
	BOOL bStartWatchingUnit;
	// ukazatel na klienta
	CZClientUnitInfoSender *pClient;
	// data jednotliv�ch po�adavk� na sledov�n�
	union 
	{
		// ukazatel na jednotku (StartWatchingUnit)
		CSUnit *pUnit;
		// ID jednotky (StopWatchingUnit)
		DWORD dwID;
	};
};

//////////////////////////////////////////////////////////////////////
// T��da fronty po�adavk� na sledov�n� jednotek
typedef CPooledQueue<struct SWatchingUnitRequestInfo, 0> CSWatchingUnitRequestQueue;

#endif //__SERVER_WATCHING_UNIT_REQUEST_QUEUE__HEADER_INCLUDED__
