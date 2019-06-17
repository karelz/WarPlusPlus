/***********************************************************
 * 
 * Projekt: Strategická hra
 *    Èást: Server hry
 *   Autor: Karel Zikmund
 * 
 *   Popis: Tøída seznamu zaregistrovaných klientù civilizace
 * 
 ***********************************************************/

#ifndef __SERVER_REGISTERED_CLIENT_LIST__HEADER_INCLUDED__
#define __SERVER_REGISTERED_CLIENT_LIST__HEADER_INCLUDED__

#include "SWatchedMapSquareArray.h"

//////////////////////////////////////////////////////////////////////
// Dopøedné deklarace

// tøída rozesílatele informací o jednotkách klientovi civilizace
class CZClientUnitInfoSender;

//////////////////////////////////////////////////////////////////////
// Datové typy

// informace o zaregistrovaném klientovi
struct SRegisteredClientInfo 
{
	// ukazatel na klienta civilizace
	CZClientUnitInfoSender *pClient;
	// pole sledovaných MapSquarù
	CSWatchedMapSquareArray *pWatchedMapSquareArray;
};

//////////////////////////////////////////////////////////////////////
// Tøída seznamu zaregistrovaných klientù civilziace
class CSRegisteredClientList : public CPooledList<struct SRegisteredClientInfo, 0> 
{
// Metody
public:
// Operace se seznamem

	// najde klienta "pClient" v seznamu a vrátí jeho pozici (NULL=nenalezeno)
	POSITION FindClient ( CZClientUnitInfoSender *pClient ) const;
};

#endif //__SERVER_REGISTERED_CLIENT_LIST__HEADER_INCLUDED__
