/***********************************************************
 * 
 * Projekt: Strategick� hra
 *    ��st: Server hry - skilly
 *   Autor: Karel Zikmund
 * 
 *   Popis: P��klad skilly
 * 
 ***********************************************************/

#ifndef __SERVER_EXAMPLE_SKILL__HEADER_INCLUDED__
#define __SERVER_EXAMPLE_SKILL__HEADER_INCLUDED__

/////////////////////////////////////////////////////////////////////
// P��klad skilly
typedef struct tagSSExampleSkill
{
	// doba p�ed v�pisem (v TimeSlicech)
	DWORD m_dwWaitCount;
} SSExampleSkill;

#endif //__SERVER_EXAMPLE_SKILL__HEADER_INCLUDED__
