/***********************************************************
 * 
 * Projekt: Strategická hra
 *    Èást: MapEditor
 *   Autor: Karel Zikmund
 * 
 *   Popis: Data grafù pro hledání cesty
 * 
 ***********************************************************/

#ifndef __MAPEDITOR_FIND_PATH_GRAPHS__HEADER_INCLUDED__
#define __MAPEDITOR_FIND_PATH_GRAPHS__HEADER_INCLUDED__

#define FIND_PATH_GRAPHS_DIRECTORY				_T("FindPathGraphs")
#define FIND_PATH_GRAPH_HEADER_FILE_NAME		_T("header")

// nekoneèná vzdálenost
#define INFINITE_DISTANCE	0x0fffffff
// prázdný hint cesty
#define INVALID_PATH_HINT	0xffffffff

#pragma pack(1)

/*
header

	SMFindPathGraphHeader
	DWORD aAreaTable[m_dwAreaTableSize]
	DWORD aBridgeGateTable[m_dwBridgeGateTableSize]
	DWORD aHintTable[m_dwHintTableSize]

Area table record

	DWORD dwBridgeGateCount
	DWORD dwWidth
	DWORD aBridgeGateIndex[dwBridgeGateCount]
	[dwBridgeGateCount*(dwBridgeGateCount-1)/2]
		// 0,1; 0,2; 0,3; ...; 0,(dwBridgeGateCount-1); 1,2; 1,3; ...; 
		//		1,(dwBridgeGateCount-1); ...; (dwBridgeGateCount-2),(dwBridgeGateCount-1)
		DWORD dwBridgeGateDistance
		DWORD dwPathHintIndex		// i INVALID_PATH_HINT hodnota

BridgeGate table record

	DWORD dwX
	DWORD dwY
	DWORD dwBridgeAreaIndex
	DWORD dwAreaIndex

PathHint table record

	DWORD dwCheckPointCount
	[dwCheckPointCount]
		DWORD dwX
		DWORD dwY
*/

struct SMFindPathGraphHeader 
{
	// velikost tabulky oblastí
	DWORD m_dwAreaTableSize;
	// velikost tabulky bran mostù
	DWORD m_dwBridgeGateTableSize;
	// velikost tabulky hintù cest
	DWORD m_dwPathHintTableSize;

	BYTE m_aReserved[64-3*4];
};

#pragma pack()

#endif //__MAPEDITOR_FIND_PATH_GRAPHS__HEADER_INCLUDED__
