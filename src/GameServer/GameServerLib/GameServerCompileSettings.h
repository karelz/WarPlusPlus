/***********************************************************
 * 
 * Projekt: Strategická hra
 *    Èást: Server hry
 *   Autor: Karel Zikmund
 * 
 *   Popis: Parametry kompilace serveru hry
 * 
 ***********************************************************/

#ifndef __GAME_SERVER_COMPILE_SETTINGS__HEADER_INCLUDED__
#define __GAME_SERVER_COMPILE_SETTINGS__HEADER_INCLUDED__

// pomìr prodlevy mezi testováním novì pøíchozích DPC (vùèi délce TimeSlicu)
#define DPC_INCOME_TEST_DELAY_RATE		10

// maximální poèet civilizací (nejlépe dìlitelný 32)
#define CIVILIZATION_COUNT_MAX			32

// velikost MapSquaru v MapCellech
#define MAP_SQUARE_SIZE						64

// nejvìtší povolená velikost polomìru zjištìní jednotek v oblasti
#define MAX_GET_UNITS_IN_AREA_RADIUS	( 6 * MAP_SQUARE_SIZE )

// poèet jednotek v bloku na zamykání (na mapì)
#define UNITS_IN_BLOCK						10

// funkce vrací, má-li se v TimeSlicu "dwTimeSlice" poèítat viditelnost
inline BOOL IsVisibilityTimeSlice ( DWORD dwTimeSlice ) 
{
	return ( dwTimeSlice % 2 == 0 );
}

// pozice mimo mapu
#define NO_MAP_POSITION						( ((DWORD)-1) - 4 * MAP_SQUARE_SIZE )

// maximální velikost full infa skilly
#define SKILL_FULL_INFO_MAX_SIZE			1024

// zaèátek vzdušné vertikální pozice
#define AIR_VERTICAL_POSITION				40000

// maximální vzdálenost kontrolních bodù cesty
#define MAX_CHECK_POINT_DISTANCE			30

// životy mrtvé jednotky
#define DEAD_UNIT_LIVES		((int)-0x3fffffff)

#endif //__GAME_SERVER_COMPILE_SETTINGS__HEADER_INCLUDED__
