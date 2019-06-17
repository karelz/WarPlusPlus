/***********************************************************
 * 
 * Projekt: Strategick· hra
 *    »·st: Server hry - skilly
 *   Autor: Roman Zenka
 * 
 *   Popis: Storage skilla
 * 
 ***********************************************************/

#ifndef __SERVER_STORAGE_SKILL__HEADER_INCLUDED__
#define __SERVER_STORAGE_SKILL__HEADER_INCLUDED__

#include "GameServer/GameServer/SResources.h"

class CISysCallStorage;
class CZUnit;

#pragma pack(push, 1)

/////////////////////////////////////////////////////////////////////
// Storage skilla - data ukladana u jednotky
struct SSStorageSkillUnit
{   

// Data
public:    
	// Pocet jednotek, ktere se momentalne unloadi
	int m_nUnloaders;
};

#pragma pack(pop)

#endif // __SERVER_Storage_SKILL__HEADER_INCLUDED__
