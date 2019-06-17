#ifndef __MMINESKILLTYPE_HEADER_INCLUDED__
#define __MMINESKILLTYPE_HEADER_INCLUDED__

#pragma pack(push, 1)

/////////////////////////////////////////////////////////////////////
// Mine skilla

#include "Common\Map\MapDataTypes.h"

struct SMMineSkillType
{   
	// Kolik resourcu jednotka muze natezit celkem
	int m_StorageSize[RESOURCE_COUNT];

/////////////////////////////////////////////////////////////////////
// Tezeni

	// Z jake vzdalenosti od jednotky je mozne tezit
	int m_dwMaxMineDistance;

	// Kolik resourcu max. jednotka natezi za timeslice
	int m_LoadPerTimeslice[RESOURCE_COUNT];

	// Jaky appearance se ma nastavit pred tezenim
	DWORD m_dwAppearanceBeforeMining;

	// Kolik timeslicu pockat, nez se dokresli vzhled pred tezenim
	DWORD m_dwTimeslicesBeforeMining;

	// Jaky appearance nastavit pri tezeni
	DWORD m_dwAppearanceWhileMining;

	// Jaky appearance se ma nastavit po tezeni
	DWORD m_dwAppearanceAfterMining;

	// Kolik timeslicu pockat, nez se dokresli vzhled po tezeni
	DWORD m_dwTimeslicesAfterMining;

/////////////////////////////////////////////////////////////////////
// Unload

	// Z jake vzdalenosti od jednotky je mozne unloadit
	int m_dwMaxUnloadDistance;

	// Kolik resourcu max. jednotka unloadne za timeslice
	int m_UnloadPerTimeslice[RESOURCE_COUNT];

	// Jaky appearance se ma nastavit pred unloadem
	DWORD m_dwAppearanceBeforeUnload;

	// Kolik timeslicu pockat, nez se dokresli vzhled pred unloadem
	DWORD m_dwTimeslicesBeforeUnload;

	// Jaky appearance nastavit pri unloadu
	DWORD m_dwAppearanceWhileUnload;

	// Jaky appearance se ma nastavit po unloadu
	DWORD m_dwAppearanceAfterUnload;

	// Kolik timeslicu pockat, nez se dokresli vzhled po unloadu
	DWORD m_dwTimeslicesAfterUnload;

/////////////////////////////////////////////////////////////////////
// Prepnuti modu po natezeni. 
// Tenhle int je tam jenom ve verzi mapy vetsi nez 1.5

	int m_nModeFull;

/////////////////////////////////////////////////////////////////////
// Dalsi vzhledy
// Tuhle cast ma verze vetsi nez 1.6

	// Jaky appearance se ma nastavit pred tezenim, kdyz je jednotka uz naplnena
	DWORD m_dwAppearanceBeforeMiningFull;

	// Kolik timeslicu pockat, nez se dokresli vzhled pred tezenim, kdyz je jednotka uz naplnena
	DWORD m_dwTimeslicesBeforeMiningFull;

	// Jaky appearance nastavit pri tezeni, kdyz je jednotka uz natezena
	DWORD m_dwAppearanceWhileMiningFull;

	// Jaky appearance se ma nastavit po unloadu, kdyz je jednotka uz natezena
	DWORD m_dwAppearanceAfterUnloadFull;

	// Kolik timeslicu pockat, nez se dokresli vzhled po unloadu, kdyz je jednotka uz natezena
	DWORD m_dwTimeslicesAfterUnloadFull;
};

#pragma pack(pop)

#endif // __MINESKILLTYPE_HEADER_INCLUDED__