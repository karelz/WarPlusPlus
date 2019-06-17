/***********************************************************
 * 
 * Projekt: Strategická hra
 *    Èást: Server hry
 *   Autor: Karel Zikmund
 * 
 *   Popis: Abstraktní tøída odloženého volání procedury
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
	// zjistí, je-li asociována nìjaká jednotka
	if ( pUnit != NULL )
	{	// jednotka je asociována
		// zvýší poèet odkazù na jednotku
		pUnit->AddRef ();
	}
}

// vrátí index civilizace právì provádìného DPC
inline DWORD CSDeferredProcedureCall::GetRunningDPCCivilizationIndex () 
{
	// vrátí index civilizace provádìného DPC
	return g_cMap.m_dwRunningDPCCivilizationIndex;
};

#endif //__SERVER_DEFERRED_PROCEDURE_CALL__INLINE_INCLUDED__
