#ifndef __MSTORAGESKILLTYPE_HEADER_INCLUDED__
#define __MSTORAGESKILLTYPE_HEADER_INCLUDED__

#pragma pack(push, 1)

/////////////////////////////////////////////////////////////////////
// Resource skilla

#include "Common\Map\MapDataTypes.h"

struct SMStorageSkillType
{   
	// Kolik resourcu je povoleno pro lib. jednotku unloadnout za timeslice
	// (kapacita storage)
	int m_AcceptedPerTimeslice[RESOURCE_COUNT];

	// Jaky appearance se ma nastavit, kdyz do mne nekdo unloadi
	DWORD m_dwAppearanceWhileUnloading;
};

#pragma pack(pop)

#endif // __MSTORAGESKILLTYPE_HEADER_INCLUDED__