/***********************************************************
 * 
 * Projekt: Strategická hra
 *    Èást: Server hry
 *   Autor: Karel Zikmund
 * 
 *   Popis: Tøída fronty požadavkù na sledování MapSquarù
 * 
 ***********************************************************/

#ifndef __SERVER_WATCHING_MAP_SQUARE_REQUEST_QUEUE__HEADER_INCLUDED__
#define __SERVER_WATCHING_MAP_SQUARE_REQUEST_QUEUE__HEADER_INCLUDED__

//////////////////////////////////////////////////////////////////////
// Dopøedné deklarace

// tøída rozesílatele informací o jednotkách klientovi civilizace
class CZClientUnitInfoSender;
// tøída ètverce mapy na serveru hry
class CSMapSquare;

//////////////////////////////////////////////////////////////////////
// Datové typy

// informace o požadavku na sledování MapSquaru
struct SWatchingMapSquareRequestInfo 
{
	// typ požadavku na sledování MapSquaru (StartWatchingMapSquare/StopWatchingMapSquare)
	BOOL bStartWatchingMapSquare;
	// ukazatel na klienta
	CZClientUnitInfoSender *pClient;
	// ukazatel na MapSquare (StartWatchingMapSquare a StopWatchingMapSquare)
	CSMapSquare *pMapSquare;
};

//////////////////////////////////////////////////////////////////////
// Tøída fronty požadavkù na sledování MapSquaru
class CSWatchingMapSquareRequestQueue : 
	public CPooledQueue<struct SWatchingMapSquareRequestInfo, 0> 
{
// Metody
public:
// Operace s frontou

	// smaže všechny požadavky na sledování MapSquaru klientem "pClient"
	void RemoveAll ( CZClientUnitInfoSender *pClient );
};

#endif //__SERVER_WATCHING_MAP_SQUARE_REQUEST_QUEUE__HEADER_INCLUDED__
