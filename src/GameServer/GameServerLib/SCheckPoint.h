/***********************************************************
 * 
 * Projekt: Strategick� hra
 *    ��st: Server hry
 *   Autor: Karel Zikmund
 * 
 *   Popis: T��da kontroln�ho bodu cesty
 * 
 ***********************************************************/

#ifndef __SERVER_CHECK_POINT__HEADER_INCLUDED__
#define __SERVER_CHECK_POINT__HEADER_INCLUDED__

// obecn� t��dy a �ablony
#include "GeneralClasses\GeneralClasses.h"

//////////////////////////////////////////////////////////////////////
// T��da kontroln�ho bodu cesty na serveru hry.
class CSCheckPoint 
{
// Datov� typy
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

// P��stup k dat�m

	// zjist� pozici a �as kontroln�ho bodu cesty
	void Get ( CPointDW &pointPosition, DWORD &dwTime ) 
		{ pointPosition = m_pointPosition; dwTime = m_dwTime; };
	// zm�n� pozici a �as kontroln�ho bodu cesty
	void Set ( CPointDW pointPosition, DWORD dwTime ) 
		{ m_pointPosition = pointPosition; m_dwTime = dwTime; };
	// zjist� pozici kontroln�ho bodu cesty na map�
	CPointDW GetPosition () { return m_pointPosition; };
	// zm�n� pozici kontroln�ho bodu cesty na map�
	void SetPosition ( CPointDW pointPosition ) { m_pointPosition = pointPosition; };
	// zjist� �as kontroln�ho bodu cesty
	DWORD GetTime () { return m_dwTime; };
	// zm�n� �as kontroln�ho bodu cesty
	void SetTime ( DWORD dwTime ) { m_dwTime = dwTime; };

// Data
private:
	// pozice na map�
	CPointDW m_pointPosition;
	// �asov� hodnota kontroln�ho bodu cesty
	DWORD m_dwTime;
};

#endif //__SERVER_CHECK_POINT__HEADER_INCLUDED__
