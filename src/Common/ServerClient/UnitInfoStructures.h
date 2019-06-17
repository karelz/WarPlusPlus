/***********************************************************
 * 
 * Projekt: Strategická hra
 *    Èást: Klient-serverové informace o jednotce
 *   Autor: Karel Zikmund
 * 
 *   Popis: Klient-serverové stuktury informací o jednotce
 * 
 ***********************************************************/

#ifndef __UNIT_INFO_STRUCTURES__HEADER_INCLUDED__
#define __UNIT_INFO_STRUCTURES__HEADER_INCLUDED__

// zarovnávání struktur na byty
#pragma pack ( 1 )

// brief info jednotky
struct SUnitBriefInfo 
{
  // ID jednotky
	DWORD dwID;
	// umístìní jednotky na mapì
	DWORD dwPositionX;
	DWORD dwPositionY;
	// vertikální umístìní jednotky v UnitLevelu
	DWORD dwVerticalPosition;
	// aktuální poèet životù (nìjaká abstraktní velièina) (kvùli ukazování pravítka 
	//		s životem na GameClientovi)
	int nLives;
	// maximální poèet životù (nìjaká abstraktní velièina) (kvùli ukazování pravítka 
	//		s životem na GameClientovi)
	int nLivesMax;
	// polomìr viditelnosti (v MapCellech)
	DWORD dwViewRadius;
	// ID vzhledu jednotky
	DWORD dwAppearanceID;

  // mód jednotky
  BYTE m_nMode;

	// orientace jednotky (kvùli zobrazování na GameClientovi)
	BYTE nDirection;
};

struct SUnitCheckPointInfo : SUnitBriefInfo 
{
	// první kontrolní bod jednotky
	DWORD dwFirstPositionX;
	DWORD dwFirstPositionY;
	DWORD dwFirstTime;
	// druhý kontrolní bod jednotky
	DWORD dwSecondPositionX;
	DWORD dwSecondPositionY;
	DWORD dwSecondTime;
};

// info o jednotce, kterou klient vidí poprvé
struct SUnitStartInfo : SUnitCheckPointInfo 
{
	// ID typu jednotky
	DWORD dwUnitTypeID;
	// ID civilizace
	DWORD dwCivilizationID;
};

struct SUnitFullInfo : SUnitBriefInfo 
{
	// bitová maska pøíznakù neviditelnosti jednotky (tj. jak je jednotka neviditelná)
	DWORD dwInvisibilityState;
	// bitová maska pøíznakù detekce neviditelnosti ostatních jednotek (tj. které 
	//		neviditelnosti jednotka vidí)
	DWORD dwInvisibilityDetection;
};

struct SUnitEnemyFullInfo : SUnitBriefInfo 
{
	// bitová maska pøíznakù neviditelnosti jednotky (tj. jak je jednotka neviditelná)
	DWORD dwInvisibilityState;
};

struct SUnitStartFullInfo : SUnitStartInfo 
{
	// bitová maska pøíznakù neviditelnosti jednotky (tj. jak je jednotka neviditelná)
	DWORD dwInvisibilityState;
	// bitová maska pøíznakù detekce neviditelnosti ostatních jednotek (tj. které 
	//		neviditelnosti jednotka vidí)
	DWORD dwInvisibilityDetection;
};

struct SUnitStartEnemyFullInfo : SUnitStartInfo 
{
	// bitová maska pøíznakù neviditelnosti jednotky (tj. jak je jednotka neviditelná)
	DWORD dwInvisibilityState;
};

// zarovnávání struktur podle kompilátoru
#pragma pack ()

#endif //__UNIT_INFO_STRUCTURES__HEADER_INCLUDED__
