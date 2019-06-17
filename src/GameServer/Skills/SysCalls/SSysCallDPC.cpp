/***********************************************************
 * 
 * Projekt: Strategická hra
 *    Èást: Server hry
 *   Autor: Roman Ženka & Karel Zikmund
 * 
 *   Popis: Tøída DPC se systémovým voláním
 * 
 ***********************************************************/

#include "StdAfx.h"

#include "SSysCallDPC.h"

#include "GameServer\GameServer\SDeferredProcedureCall.inl"

#include "IDPCSysCall.h"

IMPLEMENT_DYNAMIC ( CSSysCallDPC, CSDeferredProcedureCall )

//////////////////////////////////////////////////////////////////////
// Konstrukce a destrukce
//////////////////////////////////////////////////////////////////////

// konstruktor
CSSysCallDPC::CSSysCallDPC ( CIProcess *pProcess, CIDataStackItem *pReturnValue, 
	CSUnit *pUnit ) : CSDeferredProcedureCall ( pUnit ) 
{
	// vytvoøí nové systémové volání DPC
	m_pDPCSysCall = new CIDPCSysCall;
	VERIFY ( m_pDPCSysCall->Create ( pProcess, pReturnValue, this ) );
}

// destruktor
CSSysCallDPC::~CSSysCallDPC ()
{ 
    if (m_pDPCSysCall != NULL) 
    {
        m_pDPCSysCall->Release();
    }
}

//////////////////////////////////////////////////////////////////////
// // vrátí ukazatel na systémové volání DPC s návratovou hodnotou
//////////////////////////////////////////////////////////////////////

CIDPCSysCall* CSSysCallDPC::GetDPCSysCall ()
{
	ASSERT ( m_pDPCSysCall != NULL ); 
    m_pDPCSysCall->AddRef();
    return m_pDPCSysCall;
}

//////////////////////////////////////////////////////////////////////
// Ukládání dat potomkù (analogie CPersistentObjectPlain metod)
//////////////////////////////////////////////////////////////////////

/* CSSysCallDPC stored

	CIDPCSysCall stored
*/

// ukládání dat DPC
void CSSysCallDPC::PersistentSaveDPC ( CPersistentStorage &storage ) 
{
    BRACE_BLOCK(storage);

	ASSERT ( m_pDPCSysCall != NULL );

	// uloží systémové volání DPC
	m_pDPCSysCall->PersistentSave ( storage );
}

// nahrávání pouze uložených dat DPC
void CSSysCallDPC::PersistentLoadDPC ( CPersistentStorage &storage ) 
{    
    BRACE_BLOCK(storage);

	ASSERT ( m_pDPCSysCall == NULL );

	// nahraje systémové volání DPC
	m_pDPCSysCall = new CIDPCSysCall;
	m_pDPCSysCall->PersistentLoad ( storage );
}

// pøeklad ukazatelù DPC
void CSSysCallDPC::PersistentTranslatePointersDPC ( CPersistentStorage &storage ) 
{
	ASSERT ( m_pDPCSysCall != NULL );

	// pøeloží ukazatele systémové volání DPC
	m_pDPCSysCall->PersistentTranslatePointers ( storage );
}

// inicializace nahraného DPC
void CSSysCallDPC::PersistentInitDPC () 
{
	ASSERT ( m_pDPCSysCall != NULL );

	// inicializuje systémové volání DPC
	m_pDPCSysCall->PersistentInit ( this );
}
