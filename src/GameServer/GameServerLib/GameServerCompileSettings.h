/***********************************************************
 * 
 * Projekt: Strategick� hra
 *    ��st: Server hry
 *   Autor: Karel Zikmund
 * 
 *   Popis: Parametry kompilace serveru hry
 * 
 ***********************************************************/

#ifndef __GAME_SERVER_COMPILE_SETTINGS__HEADER_INCLUDED__
#define __GAME_SERVER_COMPILE_SETTINGS__HEADER_INCLUDED__

// pom�r prodlevy mezi testov�n�m nov� p��choz�ch DPC (v��i d�lce TimeSlicu)
#define DPC_INCOME_TEST_DELAY_RATE		10

// maxim�ln� po�et civilizac� (nejl�pe d�liteln� 32)
#define CIVILIZATION_COUNT_MAX			32

// velikost MapSquaru v MapCellech
#define MAP_SQUARE_SIZE						64

// nejv�t�� povolen� velikost polom�ru zji�t�n� jednotek v oblasti
#define MAX_GET_UNITS_IN_AREA_RADIUS	( 6 * MAP_SQUARE_SIZE )

// po�et jednotek v bloku na zamyk�n� (na map�)
#define UNITS_IN_BLOCK						10

// funkce vrac�, m�-li se v TimeSlicu "dwTimeSlice" po��tat viditelnost
inline BOOL IsVisibilityTimeSlice ( DWORD dwTimeSlice ) 
{
	return ( dwTimeSlice % 2 == 0 );
}

// pozice mimo mapu
#define NO_MAP_POSITION						( ((DWORD)-1) - 4 * MAP_SQUARE_SIZE )

// maxim�ln� velikost full infa skilly
#define SKILL_FULL_INFO_MAX_SIZE			1024

// za��tek vzdu�n� vertik�ln� pozice
#define AIR_VERTICAL_POSITION				40000

// maxim�ln� vzd�lenost kontroln�ch bod� cesty
#define MAX_CHECK_POINT_DISTANCE			30

// �ivoty mrtv� jednotky
#define DEAD_UNIT_LIVES		((int)-0x3fffffff)

#endif //__GAME_SERVER_COMPILE_SETTINGS__HEADER_INCLUDED__
