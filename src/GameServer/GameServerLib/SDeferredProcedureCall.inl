/***********************************************************
 * 
 * Projekt: Strategick� hra
 *    ��st: Server hry
 *   Autor: Karel Zikmund
 * 
 *   Popis: Abstraktn� t��da odlo�en�ho vol�n� procedury
 * 
 ***********************************************************/

#ifndef __SERVER_DEFERRED_PROCEDURE_CALL__INLINE_INCLUDED__
#define __SERVER_DEFERRED_PROCEDURE_CALL__INLINE_INCLUDED__

#include "SDeferredProcedureCall.h"
#include "SUnit.h"
#include "SUnit.inl"

//////////////////////////////////////////////////////////////////////
// Konstrukce a destrukce
//////////////////////////////////////////////////////////////////////

// konstruktor
inline CSDeferredProcedureCall::CSDeferredProcedureCall ( CSUnit *pUnit ) 
{
	// inicializuje ukazatel na asociovanou jednotku
	m_pUnit = pUnit;
	// zjist�, je-li asociov�na n�jak� jednotka
	if ( pUnit != NULL )
	{	// jednotka je asociov�na
		// zv��� po�et odkaz� na jednotku
		pUnit->AddRef ();
	}
}

// vr�t� index civilizace pr�v� prov�d�n�ho DPC
inline DWORD CSDeferredProcedureCall::GetRunningDPCCivilizationIndex () 
{
	// vr�t� index civilizace prov�d�n�ho DPC
	return g_cMap.m_dwRunningDPCCivilizationIndex;
};

#endif //__SERVER_DEFERRED_PROCEDURE_CALL__INLINE_INCLUDED__
