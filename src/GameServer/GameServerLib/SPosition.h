/***********************************************************
 * 
 * Projekt: Strategická hra
 *    Èást: Server hry
 *   Autor: Karel Zikmund
 * 
 *   Popis: Tøída 3D pozice na mapì
 * 
 ***********************************************************/

#ifndef __SERVER_POSITION__HEADER_INCLUDED__
#define __SERVER_POSITION__HEADER_INCLUDED__

#include "GeneralClasses\PointDW.h"

//////////////////////////////////////////////////////////////////////
// Tøída 3D pozice na mapì
class CSPosition : public CPointDW 
{
public:
	// z-ová souøadnice pozice na mapì
	DWORD z;
};

#endif //__SERVER_POSITION__HEADER_INCLUDED__
