/***********************************************************
 * 
 * Projekt: Strategick� hra
 *    ��st: Server hry
 *   Autor: Karel Zikmund
 * 
 *   Popis: T��da seznamu zaregistrovan�ch klient� civilizace
 * 
 ***********************************************************/

#ifndef __SERVER_REGISTERED_CLIENT_LIST__HEADER_INCLUDED__
#define __SERVER_REGISTERED_CLIENT_LIST__HEADER_INCLUDED__

#include "SWatchedMapSquareArray.h"

//////////////////////////////////////////////////////////////////////
// Dop�edn� deklarace

// t��da rozes�latele informac� o jednotk�ch klientovi civilizace
class CZClientUnitInfoSender;

//////////////////////////////////////////////////////////////////////
// Datov� typy

// informace o zaregistrovan�m klientovi
struct SRegisteredClientInfo 
{
	// ukazatel na klienta civilizace
	CZClientUnitInfoSender *pClient;
	// pole sledovan�ch MapSquar�
	CSWatchedMapSquareArray *pWatchedMapSquareArray;
};

//////////////////////////////////////////////////////////////////////
// T��da seznamu zaregistrovan�ch klient� civilziace
class CSRegisteredClientList : public CPooledList<struct SRegisteredClientInfo, 0> 
{
// Metody
public:
// Operace se seznamem

	// najde klienta "pClient" v seznamu a vr�t� jeho pozici (NULL=nenalezeno)
	POSITION FindClient ( CZClientUnitInfoSender *pClient ) const;
};

#endif //__SERVER_REGISTERED_CLIENT_LIST__HEADER_INCLUDED__
