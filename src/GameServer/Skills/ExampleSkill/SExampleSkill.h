/***********************************************************
 * 
 * Projekt: Strategická hra
 *    Èást: Server hry - skilly
 *   Autor: Karel Zikmund
 * 
 *   Popis: Pøíklad skilly
 * 
 ***********************************************************/

#ifndef __SERVER_EXAMPLE_SKILL__HEADER_INCLUDED__
#define __SERVER_EXAMPLE_SKILL__HEADER_INCLUDED__

/////////////////////////////////////////////////////////////////////
// Pøíklad skilly
typedef struct tagSSExampleSkill
{
	// doba pøed výpisem (v TimeSlicech)
	DWORD m_dwWaitCount;
} SSExampleSkill;

#endif //__SERVER_EXAMPLE_SKILL__HEADER_INCLUDED__
