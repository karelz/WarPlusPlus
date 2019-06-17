/***********************************************************
 * 
 * Projekt: Strategick� hra
 *    ��st: Server hry
 *   Autor: Roman �enka & Karel Zikmund
 * 
 *   Popis: T��da DPC se syst�mov�m vol�n�m
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
	// vytvo�� nov� syst�mov� vol�n� DPC
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
// // vr�t� ukazatel na syst�mov� vol�n� DPC s n�vratovou hodnotou
//////////////////////////////////////////////////////////////////////

CIDPCSysCall* CSSysCallDPC::GetDPCSysCall ()
{
	ASSERT ( m_pDPCSysCall != NULL ); 
    m_pDPCSysCall->AddRef();
    return m_pDPCSysCall;
}

//////////////////////////////////////////////////////////////////////
// Ukl�d�n� dat potomk� (analogie CPersistentObjectPlain metod)
//////////////////////////////////////////////////////////////////////

/* CSSysCallDPC stored

	CIDPCSysCall stored
*/

// ukl�d�n� dat DPC
void CSSysCallDPC::PersistentSaveDPC ( CPersistentStorage &storage ) 
{
    BRACE_BLOCK(storage);

	ASSERT ( m_pDPCSysCall != NULL );

	// ulo�� syst�mov� vol�n� DPC
	m_pDPCSysCall->PersistentSave ( storage );
}

// nahr�v�n� pouze ulo�en�ch dat DPC
void CSSysCallDPC::PersistentLoadDPC ( CPersistentStorage &storage ) 
{    
    BRACE_BLOCK(storage);

	ASSERT ( m_pDPCSysCall == NULL );

	// nahraje syst�mov� vol�n� DPC
	m_pDPCSysCall = new CIDPCSysCall;
	m_pDPCSysCall->PersistentLoad ( storage );
}

// p�eklad ukazatel� DPC
void CSSysCallDPC::PersistentTranslatePointersDPC ( CPersistentStorage &storage ) 
{
	ASSERT ( m_pDPCSysCall != NULL );

	// p�elo�� ukazatele syst�mov� vol�n� DPC
	m_pDPCSysCall->PersistentTranslatePointers ( storage );
}

// inicializace nahran�ho DPC
void CSSysCallDPC::PersistentInitDPC () 
{
	ASSERT ( m_pDPCSysCall != NULL );

	// inicializuje syst�mov� vol�n� DPC
	m_pDPCSysCall->PersistentInit ( this );
}
