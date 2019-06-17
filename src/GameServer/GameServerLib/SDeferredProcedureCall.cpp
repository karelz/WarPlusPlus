/***********************************************************
 * 
 * Projekt: Strategick� hra
 *    ��st: Server hry
 *   Autor: Karel Zikmund
 * 
 *   Popis: Abstraktn� t��da odlo�en�ho vol�n� procedury
 * 
 ***********************************************************/

#include "StdAfx.h"
#include "SDeferredProcedureCall.h"
#include "SUnit.h"

#include "SUnit.inl"
#include "SDeferredProcedureCall.inl"

IMPLEMENT_DYNAMIC ( CSDeferredProcedureCall, CPersistentObject )

// implementace t��dy, jej� potomky lze vytv��et za b�hu programu z ID potomka
IMPLEMENT_RUN_TIME_ID_STORAGE ( CSDeferredProcedureCall )

//////////////////////////////////////////////////////////////////////
// Konstrukce a destrukce
//////////////////////////////////////////////////////////////////////

// destruktor
CSDeferredProcedureCall::~CSDeferredProcedureCall () 
{
	// zjist�, je-li asociov�na n�jak� jednotka
	if ( m_pUnit != NULL )
	{	// jednotka je asociov�na
		// sn�� po�et odkaz� na jednotku
		m_pUnit->Release ();
	}
}

//////////////////////////////////////////////////////////////////////
// Ukl�d�n� dat (CPersistentObjectPlain metody)
//////////////////////////////////////////////////////////////////////

// ukl�d�n� dat
void CSDeferredProcedureCall::PersistentSave ( CPersistentStorage &storage ) 
{
	BRACE_BLOCK ( storage );

	// ulo�� asociovanou jednotku
	storage << m_pUnit;

	// ulo�� data potomka
	PersistentSaveDPC ( storage );
}

// nahr�v�n� pouze ulo�en�ch dat
void CSDeferredProcedureCall::PersistentLoad ( CPersistentStorage &storage ) 
{
	BRACE_BLOCK ( storage );

	// zkontroluje ne�kodn� data
	ASSERT ( m_pUnit == NULL );

	// nahraje asociovanou jednotku (m��e b�t i NULL)
	storage >> (void *&)m_pUnit;

	// nahraje data potomka
	PersistentLoadDPC ( storage );
}

// p�eklad ukazatel�
void CSDeferredProcedureCall::PersistentTranslatePointers ( CPersistentStorage &storage ) 
{
	// p�elo�� ukazatel na asociovanou jednotku
	m_pUnit = (CSUnit *)storage.TranslatePointer ( m_pUnit );

	// p�elo�� ukazatele dat potomka
	PersistentTranslatePointersDPC ( storage );
}

// inicializace nahran�ho objektu
void CSDeferredProcedureCall::PersistentInit () 
{
	// zjist�, je-li asociov�na n�jak� jednotka
	if ( m_pUnit != NULL )
	{	// jednotka je asociov�na
		// zv��� po�et odkaz� na jednotku
		m_pUnit->AddRef ();
	}

	// inicializuje data potomka
	PersistentInitDPC ();
}
