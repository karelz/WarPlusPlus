/***********************************************************
 * 
 * Projekt: Strategick· hra
 *    »·st: Server hry - skilly
 *   Autor: Roman Zenka
 * 
 *   Popis: Mine skilla
 * 
 ***********************************************************/

#ifndef __SERVER_MINE_SKILL__HEADER_INCLUDED__
#define __SERVER_MINE_SKILL__HEADER_INCLUDED__

#include "GameServer/GameServer/SResources.h"

class CISysCallMine;
class CZUnit;

#pragma pack(push, 1)

/////////////////////////////////////////////////////////////////////
// Mine skilla - data ukladana u jednotky
struct SSMineSkillUnit
{   
// Enum pro cinnost, kterou zrovna vykonavame
public:
	enum EMineDoing {
		nothing=0,			// Nedeje se nic
		mining=1,			// Tezime
		unloading=2,		// Vykladame natezene
	};

	enum EMineState {
		undefined=0,		// Neznamy stav
		initial=1,			// Pocatecni stav
		before=2,			// Cinnost se teprve zacine
		working=3,			// Cinnost se provadi
		after=4,			// Cinnost se konci
		done=5,				// Cinnost je skoncena
	};

// Data
public:    
	// Cinnost, kterou provadime
	EMineDoing m_nDoing;

	// Jak jsme s provadenou cinnosti daleko
	EMineState m_nState;

	// Kolik timeslicu zbyva do zmeny stavu
	DWORD m_dwRemainingTime;

	// Byli jsme nalozeni pred zacatkem tezby?
	BOOL m_bWasFull;

	// Jednotka, na ktere cinnost provadime
	CSUnit *m_pUnit;

	// Aktualni zasoba resourcu
	CSResources m_Resources;
	
	// Syscall, na kterem spime
	CISysCallMine *m_pSysCall;

	// Doslo behem tezeni/unloadu k nejake zmene ve stavu resourcu?
	bool m_bChange;
};

#pragma pack(pop)

#endif // __SERVER_MINE_SKILL__HEADER_INCLUDED__
