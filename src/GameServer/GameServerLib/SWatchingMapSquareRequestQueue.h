/***********************************************************
 * 
 * Projekt: Strategick� hra
 *    ��st: Server hry
 *   Autor: Karel Zikmund
 * 
 *   Popis: T��da fronty po�adavk� na sledov�n� MapSquar�
 * 
 ***********************************************************/

#ifndef __SERVER_WATCHING_MAP_SQUARE_REQUEST_QUEUE__HEADER_INCLUDED__
#define __SERVER_WATCHING_MAP_SQUARE_REQUEST_QUEUE__HEADER_INCLUDED__

//////////////////////////////////////////////////////////////////////
// Dop�edn� deklarace

// t��da rozes�latele informac� o jednotk�ch klientovi civilizace
class CZClientUnitInfoSender;
// t��da �tverce mapy na serveru hry
class CSMapSquare;

//////////////////////////////////////////////////////////////////////
// Datov� typy

// informace o po�adavku na sledov�n� MapSquaru
struct SWatchingMapSquareRequestInfo 
{
	// typ po�adavku na sledov�n� MapSquaru (StartWatchingMapSquare/StopWatchingMapSquare)
	BOOL bStartWatchingMapSquare;
	// ukazatel na klienta
	CZClientUnitInfoSender *pClient;
	// ukazatel na MapSquare (StartWatchingMapSquare a StopWatchingMapSquare)
	CSMapSquare *pMapSquare;
};

//////////////////////////////////////////////////////////////////////
// T��da fronty po�adavk� na sledov�n� MapSquaru
class CSWatchingMapSquareRequestQueue : 
	public CPooledQueue<struct SWatchingMapSquareRequestInfo, 0> 
{
// Metody
public:
// Operace s frontou

	// sma�e v�echny po�adavky na sledov�n� MapSquaru klientem "pClient"
	void RemoveAll ( CZClientUnitInfoSender *pClient );
};

#endif //__SERVER_WATCHING_MAP_SQUARE_REQUEST_QUEUE__HEADER_INCLUDED__
