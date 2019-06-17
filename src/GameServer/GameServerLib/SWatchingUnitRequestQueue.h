/***********************************************************
 * 
 * Projekt: Strategická hra
 *    Èást: Server hry
 *   Autor: Karel Zikmund
 * 
 *   Popis: Tøída fronty požadavkù na sledování jednotek
 * 
 ***********************************************************/

#ifndef __SERVER_WATCHING_UNIT_REQUEST_QUEUE__HEADER_INCLUDED__
#define __SERVER_WATCHING_UNIT_REQUEST_QUEUE__HEADER_INCLUDED__

//////////////////////////////////////////////////////////////////////
// Dopøedné deklarace

// tøída rozesílatele informací o jednotkách klientovi civilizace
class CZClientUnitInfoSender;
// tøída jednotky
class CSUnit;

//////////////////////////////////////////////////////////////////////
// Datové typy

// informace o požadavku na sledování jednotky
struct SWatchingUnitRequestInfo 
{
	// typ požadavku na sledování jednotky (StartWatchingUnit/StopWatchingUnit)
	BOOL bStartWatchingUnit;
	// ukazatel na klienta
	CZClientUnitInfoSender *pClient;
	// data jednotlivých požadavkù na sledování
	union 
	{
		// ukazatel na jednotku (StartWatchingUnit)
		CSUnit *pUnit;
		// ID jednotky (StopWatchingUnit)
		DWORD dwID;
	};
};

//////////////////////////////////////////////////////////////////////
// Tøída fronty požadavkù na sledování jednotek
typedef CPooledQueue<struct SWatchingUnitRequestInfo, 0> CSWatchingUnitRequestQueue;

#endif //__SERVER_WATCHING_UNIT_REQUEST_QUEUE__HEADER_INCLUDED__
