/***********************************************************
 * 
 * Projekt: Strategická hra
 *    Èást: MapEditor
 *   Autor: Karel Zikmund
 * 
 *   Popis: Vytváøení grafù pro hledání cest
 * 
 ***********************************************************/

#ifndef __MAPEDITOR_FIND_PATH_GRAPH_CREATION__HEADER_INCLUDED__
#define __MAPEDITOR_FIND_PATH_GRAPH_CREATION__HEADER_INCLUDED__

#include "Archive\Archive\Archive.h"

// chyba vytvárení grafu pro hledání cesty
enum ECreateFindPathGraphError 
{
	ECFPGE_OK,							// OK
	ECFPGE_BadMapFile,				// chybný MapFile
	ECFPGE_NotEnoughMemory,			// nedostatek pamìti
	ECFPGE_IncompleteMapLand,		// nedokonèený povrch mapy
	ECFPGE_UnknownError,				// neznámá chyba
};

// vytvoøí grafy pro hledání cesty na mapì "cMapArchive"
enum ECreateFindPathGraphError CreateFindPathGraphs ( CDataArchive cMapArchive );

#endif //__MAPEDITOR_FIND_PATH_GRAPH_CREATION__HEADER_INCLUDED__
