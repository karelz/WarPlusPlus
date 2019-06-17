/***********************************************************
 * 
 * Projekt: Strategická hra
 *    Èást: Server hry
 *   Autor: Karel Zikmund
 * 
 *   Popis: Tøída seznamu sledovaných jednotek
 * 
 ***********************************************************/

#ifndef __SERVER_WATCHED_UNIT_LIST__HEADER_INCLUDED__
#define __SERVER_WATCHED_UNIT_LIST__HEADER_INCLUDED__

//////////////////////////////////////////////////////////////////////
// Dopøedné deklarace

// tøída jednotky serveru
class CSUnit;
// tøída rozesílatele informací o jednotkách klientovi civilizace
class CZClientUnitInfoSender;

//////////////////////////////////////////////////////////////////////
// Datové typy

// informace o sledované jednotce
struct SWatchedUnitInfo 
{
	// ukazatel na sledovanou jednotku
	CSUnit *pUnit;
	// ukazatel na klienta civilizace
	CZClientUnitInfoSender *pClient;

// Metody
public:
	// operátor porovnání
	BOOL operator == ( SWatchedUnitInfo &sUnitInfo ) const 
		{ return ( pUnit == sUnitInfo.pUnit ) && ( pClient == sUnitInfo.pClient ); };
};

//////////////////////////////////////////////////////////////////////
// Tøída seznamu sledovaných jednotek
class CSWatchedUnitList : public CPooledList<struct SWatchedUnitInfo, 0> 
{
// Metody
public:
// Operace se seznamem

	// najde jednotku "dwID" sledovanou klientem "pClient" v seznamu a vrátí její 
	//		pozici (NULL=nenalezeno)
	POSITION FindUnitByID ( DWORD dwID, CZClientUnitInfoSender *pClient ) const;
	// najde jednotku "pUnit" v seznamu od pozice "rPosition" a vrátí její pozici 
	//		(FALSE=nenalezeno)
	static BOOL FindUnit ( CSUnit *pUnit, POSITION &rPosition );
};

#endif //__SERVER_WATCHED_UNIT_LIST__HEADER_INCLUDED__
