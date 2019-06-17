/***********************************************************
 * 
 * Projekt: Strategick� hra
 *    ��st: Server hry
 *   Autor: Karel Zikmund
 * 
 *   Popis: T��da seznamu sledovan�ch jednotek
 * 
 ***********************************************************/

#ifndef __SERVER_WATCHED_UNIT_LIST__HEADER_INCLUDED__
#define __SERVER_WATCHED_UNIT_LIST__HEADER_INCLUDED__

//////////////////////////////////////////////////////////////////////
// Dop�edn� deklarace

// t��da jednotky serveru
class CSUnit;
// t��da rozes�latele informac� o jednotk�ch klientovi civilizace
class CZClientUnitInfoSender;

//////////////////////////////////////////////////////////////////////
// Datov� typy

// informace o sledovan� jednotce
struct SWatchedUnitInfo 
{
	// ukazatel na sledovanou jednotku
	CSUnit *pUnit;
	// ukazatel na klienta civilizace
	CZClientUnitInfoSender *pClient;

// Metody
public:
	// oper�tor porovn�n�
	BOOL operator == ( SWatchedUnitInfo &sUnitInfo ) const 
		{ return ( pUnit == sUnitInfo.pUnit ) && ( pClient == sUnitInfo.pClient ); };
};

//////////////////////////////////////////////////////////////////////
// T��da seznamu sledovan�ch jednotek
class CSWatchedUnitList : public CPooledList<struct SWatchedUnitInfo, 0> 
{
// Metody
public:
// Operace se seznamem

	// najde jednotku "dwID" sledovanou klientem "pClient" v seznamu a vr�t� jej� 
	//		pozici (NULL=nenalezeno)
	POSITION FindUnitByID ( DWORD dwID, CZClientUnitInfoSender *pClient ) const;
	// najde jednotku "pUnit" v seznamu od pozice "rPosition" a vr�t� jej� pozici 
	//		(FALSE=nenalezeno)
	static BOOL FindUnit ( CSUnit *pUnit, POSITION &rPosition );
};

#endif //__SERVER_WATCHED_UNIT_LIST__HEADER_INCLUDED__
