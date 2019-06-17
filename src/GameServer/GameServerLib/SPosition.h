/***********************************************************
 * 
 * Projekt: Strategick� hra
 *    ��st: Server hry
 *   Autor: Karel Zikmund
 * 
 *   Popis: T��da 3D pozice na map�
 * 
 ***********************************************************/

#ifndef __SERVER_POSITION__HEADER_INCLUDED__
#define __SERVER_POSITION__HEADER_INCLUDED__

#include "GeneralClasses\PointDW.h"

//////////////////////////////////////////////////////////////////////
// T��da 3D pozice na map�
class CSPosition : public CPointDW 
{
public:
	// z-ov� sou�adnice pozice na map�
	DWORD z;
};

#endif //__SERVER_POSITION__HEADER_INCLUDED__
