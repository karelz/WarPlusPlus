/***********************************************************
 * 
 * Projekt: Strategická hra
 *    Èást: Server hry
 *   Autor: Karel Zikmund
 * 
 *   Popis: Tøída kontrolního bodu cesty
 * 
 ***********************************************************/

#ifndef __SERVER_CHECK_POINT__HEADER_INCLUDED__
#define __SERVER_CHECK_POINT__HEADER_INCLUDED__

// obecné tøídy a šablony
#include "GeneralClasses\GeneralClasses.h"

//////////////////////////////////////////////////////////////////////
// Tøída kontrolního bodu cesty na serveru hry.
class CSCheckPoint 
{
// Datové typy
private:

// Metody
public:
// Konstrukce a destrukce

	// konstruktor
	CSCheckPoint ();
	// konstruktor
	CSCheckPoint ( CPointDW pointPosition );
	// konstruktor
	CSCheckPoint ( CPointDW pointPosition, DWORD dwTime );
	// destruktor
	~CSCheckPoint ();

// Pøístup k datùm

	// zjistí pozici a èas kontrolního bodu cesty
	void Get ( CPointDW &pointPosition, DWORD &dwTime ) 
		{ pointPosition = m_pointPosition; dwTime = m_dwTime; };
	// zmìní pozici a èas kontrolního bodu cesty
	void Set ( CPointDW pointPosition, DWORD dwTime ) 
		{ m_pointPosition = pointPosition; m_dwTime = dwTime; };
	// zjistí pozici kontrolního bodu cesty na mapì
	CPointDW GetPosition () { return m_pointPosition; };
	// zmìní pozici kontrolního bodu cesty na mapì
	void SetPosition ( CPointDW pointPosition ) { m_pointPosition = pointPosition; };
	// zjistí èas kontrolního bodu cesty
	DWORD GetTime () { return m_dwTime; };
	// zmìní èas kontrolního bodu cesty
	void SetTime ( DWORD dwTime ) { m_dwTime = dwTime; };

// Data
private:
	// pozice na mapì
	CPointDW m_pointPosition;
	// èasová hodnota kontrolního bodu cesty
	DWORD m_dwTime;
};

#endif //__SERVER_CHECK_POINT__HEADER_INCLUDED__
