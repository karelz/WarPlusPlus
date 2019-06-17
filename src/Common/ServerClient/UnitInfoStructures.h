/***********************************************************
 * 
 * Projekt: Strategick� hra
 *    ��st: Klient-serverov� informace o jednotce
 *   Autor: Karel Zikmund
 * 
 *   Popis: Klient-serverov� stuktury informac� o jednotce
 * 
 ***********************************************************/

#ifndef __UNIT_INFO_STRUCTURES__HEADER_INCLUDED__
#define __UNIT_INFO_STRUCTURES__HEADER_INCLUDED__

// zarovn�v�n� struktur na byty
#pragma pack ( 1 )

// brief info jednotky
struct SUnitBriefInfo 
{
  // ID jednotky
	DWORD dwID;
	// um�st�n� jednotky na map�
	DWORD dwPositionX;
	DWORD dwPositionY;
	// vertik�ln� um�st�n� jednotky v UnitLevelu
	DWORD dwVerticalPosition;
	// aktu�ln� po�et �ivot� (n�jak� abstraktn� veli�ina) (kv�li ukazov�n� prav�tka 
	//		s �ivotem na GameClientovi)
	int nLives;
	// maxim�ln� po�et �ivot� (n�jak� abstraktn� veli�ina) (kv�li ukazov�n� prav�tka 
	//		s �ivotem na GameClientovi)
	int nLivesMax;
	// polom�r viditelnosti (v MapCellech)
	DWORD dwViewRadius;
	// ID vzhledu jednotky
	DWORD dwAppearanceID;

  // m�d jednotky
  BYTE m_nMode;

	// orientace jednotky (kv�li zobrazov�n� na GameClientovi)
	BYTE nDirection;
};

struct SUnitCheckPointInfo : SUnitBriefInfo 
{
	// prvn� kontroln� bod jednotky
	DWORD dwFirstPositionX;
	DWORD dwFirstPositionY;
	DWORD dwFirstTime;
	// druh� kontroln� bod jednotky
	DWORD dwSecondPositionX;
	DWORD dwSecondPositionY;
	DWORD dwSecondTime;
};

// info o jednotce, kterou klient vid� poprv�
struct SUnitStartInfo : SUnitCheckPointInfo 
{
	// ID typu jednotky
	DWORD dwUnitTypeID;
	// ID civilizace
	DWORD dwCivilizationID;
};

struct SUnitFullInfo : SUnitBriefInfo 
{
	// bitov� maska p��znak� neviditelnosti jednotky (tj. jak je jednotka neviditeln�)
	DWORD dwInvisibilityState;
	// bitov� maska p��znak� detekce neviditelnosti ostatn�ch jednotek (tj. kter� 
	//		neviditelnosti jednotka vid�)
	DWORD dwInvisibilityDetection;
};

struct SUnitEnemyFullInfo : SUnitBriefInfo 
{
	// bitov� maska p��znak� neviditelnosti jednotky (tj. jak je jednotka neviditeln�)
	DWORD dwInvisibilityState;
};

struct SUnitStartFullInfo : SUnitStartInfo 
{
	// bitov� maska p��znak� neviditelnosti jednotky (tj. jak je jednotka neviditeln�)
	DWORD dwInvisibilityState;
	// bitov� maska p��znak� detekce neviditelnosti ostatn�ch jednotek (tj. kter� 
	//		neviditelnosti jednotka vid�)
	DWORD dwInvisibilityDetection;
};

struct SUnitStartEnemyFullInfo : SUnitStartInfo 
{
	// bitov� maska p��znak� neviditelnosti jednotky (tj. jak je jednotka neviditeln�)
	DWORD dwInvisibilityState;
};

// zarovn�v�n� struktur podle kompil�toru
#pragma pack ()

#endif //__UNIT_INFO_STRUCTURES__HEADER_INCLUDED__
