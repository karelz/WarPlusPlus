/***********************************************************
 * 
 * Projekt: Strategick� hra
 *    ��st: MapEditor
 *   Autor: Karel Zikmund
 * 
 *   Popis: Vytv��en� graf� pro hled�n� cest
 * 
 ***********************************************************/

#ifndef __MAPEDITOR_FIND_PATH_GRAPH_CREATION__HEADER_INCLUDED__
#define __MAPEDITOR_FIND_PATH_GRAPH_CREATION__HEADER_INCLUDED__

#include "Archive\Archive\Archive.h"

// chyba vytv�ren� grafu pro hled�n� cesty
enum ECreateFindPathGraphError 
{
	ECFPGE_OK,							// OK
	ECFPGE_BadMapFile,				// chybn� MapFile
	ECFPGE_NotEnoughMemory,			// nedostatek pam�ti
	ECFPGE_IncompleteMapLand,		// nedokon�en� povrch mapy
	ECFPGE_UnknownError,				// nezn�m� chyba
};

// vytvo�� grafy pro hled�n� cesty na map� "cMapArchive"
enum ECreateFindPathGraphError CreateFindPathGraphs ( CDataArchive cMapArchive );

#endif //__MAPEDITOR_FIND_PATH_GRAPH_CREATION__HEADER_INCLUDED__
