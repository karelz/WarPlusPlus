/***********************************************************
 * 
 * Projekt: Strategická hra
 *    Èást: Server hry
 *   Autor: Karel Zikmund
 * 
 *   Popis: Tøída kontrolního bodu cesty na serveru hry
 * 
 ***********************************************************/

#include "StdAfx.h"
#include "SCheckPoint.h"

//////////////////////////////////////////////////////////////////////
// Konstrukce a destrukce
//////////////////////////////////////////////////////////////////////

// konstruktor
CSCheckPoint::CSCheckPoint () 
{
}

// konstruktor
CSCheckPoint::CSCheckPoint ( CPointDW pointPosition ) 
{
	m_pointPosition = pointPosition;
	m_dwTime = 0;
}

// konstruktor
CSCheckPoint::CSCheckPoint ( CPointDW pointPosition, DWORD dwTime ) 
{
	m_pointPosition = pointPosition;
	m_dwTime = dwTime;
}

// destruktor
CSCheckPoint::~CSCheckPoint () 
{
}
