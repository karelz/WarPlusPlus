/***********************************************************
 * 
 * Projekt: Strategick· hra
 *    »·st: MapViewer
 *   Autor: Karel Zikmund
 * 
 *   Popis: Popis mapy
 * 
 ***********************************************************/

#ifndef __MAPVIEWER_MAP_DESCRIPTION__HEADER_INCLUDED__
#define __MAPVIEWER_MAP_DESCRIPTION__HEADER_INCLUDED__

#define SHARED_MEMORY_LOCK_NAME					"MapViewer SharedMemory Lock"
#define SHARED_MEMORY_MAP_NAME					"MapViewer SharedMemory Map"
#define SHARED_MEMORY_MAPDESCRIPTION_NAME		"MapViewer SharedMemory MapDescription"
#define SHARED_MEMORY_MAP_SIZE					1100 * 1100
#define SHARED_MEMORY_MAPDESCRIPTION_SIZE		sizeof ( struct SMapDescription )

// popis mapy
struct SMapDescription 
{
	int m_nSizeX;
	int m_nSizeY;
	int m_nMapSizeX;
	int m_nMapSizeY;
	int m_nMapBorder;
	int m_nMapXMax;
	int m_nMapXMin;
	int m_nMapYMax;
	int m_nMapYMin;
};

#endif //__MAPVIEWER_MAP_DESCRIPTION__HEADER_INCLUDED__
