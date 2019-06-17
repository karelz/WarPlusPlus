/***********************************************************
 * 
 * Projekt: Strategick· hra
 *    »·st: Server hry - skilly
 *   Autor: Roman Zenka
 * 
 *   Popis: Resource skilla
 * 
 ***********************************************************/

#ifndef __SERVER_RESOURCE_SKILL__HEADER_INCLUDED__
#define __SERVER_RESOURCE_SKILL__HEADER_INCLUDED__

#include "GameServer/GameServer/SResources.h"

class CISysCallMine;
class CZUnit;

#pragma pack(push, 1)

/////////////////////////////////////////////////////////////////////
// Resource skilla - data ukladana u jednotky
struct SSResourceSkillUnit
{   
// Data
public:    
	// Aktualni zasoba resourcu
	CSResources m_Resources;	

	// Pocet jednotek, ktere ze mne momentalne tezi
	int m_nMiners;

	// Pocet timeslicu pro vzhled po dotezeni
	DWORD m_dwTimeslicesAfterMining;

	// Cislo appearance, zvysuje se, jak se postupne vytezuje
	int m_nAppearance;
};

#pragma pack(pop)

#endif // __SERVER_RESOURCE_SKILL__HEADER_INCLUDED__
