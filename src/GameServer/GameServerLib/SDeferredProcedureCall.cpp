/***********************************************************
 * 
 * Projekt: Strategická hra
 *    Èást: Server hry
 *   Autor: Karel Zikmund
 * 
 *   Popis: Abstraktní tøída odloženého volání procedury
 * 
 ***********************************************************/

#include "StdAfx.h"
#include "SDeferredProcedureCall.h"
#include "SUnit.h"

#include "SUnit.inl"
#include "SDeferredProcedureCall.inl"

IMPLEMENT_DYNAMIC ( CSDeferredProcedureCall, CPersistentObject )

// implementace tøídy, jejíž potomky lze vytváøet za bìhu programu z ID potomka
IMPLEMENT_RUN_TIME_ID_STORAGE ( CSDeferredProcedureCall )

//////////////////////////////////////////////////////////////////////
// Konstrukce a destrukce
//////////////////////////////////////////////////////////////////////

// destruktor
CSDeferredProcedureCall::~CSDeferredProcedureCall () 
{
	// zjistí, je-li asociována nìjaká jednotka
	if ( m_pUnit != NULL )
	{	// jednotka je asociována
		// sníží poèet odkazù na jednotku
		m_pUnit->Release ();
	}
}

//////////////////////////////////////////////////////////////////////
// Ukládání dat (CPersistentObjectPlain metody)
//////////////////////////////////////////////////////////////////////

// ukládání dat
void CSDeferredProcedureCall::PersistentSave ( CPersistentStorage &storage ) 
{
	BRACE_BLOCK ( storage );

	// uloží asociovanou jednotku
	storage << m_pUnit;

	// uloží data potomka
	PersistentSaveDPC ( storage );
}

// nahrávání pouze uložených dat
void CSDeferredProcedureCall::PersistentLoad ( CPersistentStorage &storage ) 
{
	BRACE_BLOCK ( storage );

	// zkontroluje neškodná data
	ASSERT ( m_pUnit == NULL );

	// nahraje asociovanou jednotku (mùže být i NULL)
	storage >> (void *&)m_pUnit;

	// nahraje data potomka
	PersistentLoadDPC ( storage );
}

// pøeklad ukazatelù
void CSDeferredProcedureCall::PersistentTranslatePointers ( CPersistentStorage &storage ) 
{
	// pøeloží ukazatel na asociovanou jednotku
	m_pUnit = (CSUnit *)storage.TranslatePointer ( m_pUnit );

	// pøeloží ukazatele dat potomka
	PersistentTranslatePointersDPC ( storage );
}

// inicializace nahraného objektu
void CSDeferredProcedureCall::PersistentInit () 
{
	// zjistí, je-li asociována nìjaká jednotka
	if ( m_pUnit != NULL )
	{	// jednotka je asociována
		// zvýší poèet odkazù na jednotku
		m_pUnit->AddRef ();
	}

	// inicializuje data potomka
	PersistentInitDPC ();
}
