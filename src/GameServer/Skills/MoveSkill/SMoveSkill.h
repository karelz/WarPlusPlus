/***********************************************************
 * 
 * Projekt: Strategická hra
 *    Èást: Server hry - skilly
 *   Autor: Vít Karas & Karel Zikmund
 * 
 *   Popis: Pohybová skilly (move skilla)
 * 
 ***********************************************************/

#ifndef __SERVER_MOVE_SKILL__HEADER_INCLUDED__
#define __SERVER_MOVE_SKILL__HEADER_INCLUDED__

//////////////////////////////////////////////////////////////////////
// Dopøedná deklarace tøíd

// tøída cesty na mapì
class CSPath;
// tøída systémového volání
class CISysCallMove;

/////////////////////////////////////////////////////////////////////
// Pohybová skilla
struct SSMoveSkill 
{
	// rychlost jednotky (v MapCellech za 100 TimeSlicù)
	DWORD m_dwSpeed;

	// pøesná pozice jednotky bìhem pohybu
	double m_dbXPosition;
	double m_dbYPosition;

	// pøírùstek pozice jednotky za jeden TimeSlice
	double m_dbXStep;
	double m_dbYStep;

	// doba èekání (v TimeSlicech) pøed novým pokusem pohnout jednotkou
	DWORD m_dwWaitCount;

	// cesta pohybu
	CSPath *m_pPath;

	// èekající systémové volání
	CISysCallMove *m_pSysCall;

	// cílová pozice
	CPointDW m_pointDestination;
	// limit ètverce tolerance vzdálenosti (ètverec tolerance vzdálenosti zvìtšený o 1)
	DWORD m_dwToleranceSquareLimit;

#ifdef _DEBUG
	// výchozí pozice
	CPointDW m_pointStart;
#endif //_DEBUG
};

#endif //__SERVER_MOVE_SKILL__HEADER_INCLUDED__
