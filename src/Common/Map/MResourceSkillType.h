#ifndef __MRESOURCESKILLTYPE_HEADER_INCLUDED__
#define __MRESOURCESKILLTYPE_HEADER_INCLUDED__

#pragma pack(push, 1)

/////////////////////////////////////////////////////////////////////
// Resource skilla

#include "Common\Map\MapDataTypes.h"

struct SMResourceSkillType
{   
	// Kolik resourcu ma jednotka k dispozici
	int m_StorageSize[RESOURCE_COUNT];

	// Kolik resourcu je povoleno pro lib. jednotku natezit za timeslice
	// (kapacita resourcu)
	int m_AvailablePerTimeslice[RESOURCE_COUNT];

	// Limity pro jednotlive appearance
	int m_ResourcesLimit[3][RESOURCE_COUNT];

	// Vzhledy normalni
	DWORD m_dwAppearance[3];

	// Vzhledy behem tezeni
	DWORD m_dwAppearanceMine[3];

	// Vzhled po dotezeni
	DWORD m_dwAppearanceAfterMining;

	// Pocet timeslicu pro vzhled po dotezeni
	DWORD m_dwTimeslicesAfterMining;
};

#pragma pack(pop)

#endif // __MRESOURCESKILLTYPE_HEADER_INCLUDED__