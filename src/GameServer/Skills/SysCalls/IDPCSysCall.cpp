/***********************************************************
 * 
 * Projekt: Strategická hra
 *    Èást: Interpret
 *   Autor: Roman Ženka & Karel Zikmund
 * 
 *   Popis: Tøída systémového volání DPC s návratovou hodnotou
 * 
 ***********************************************************/

#include "StdAfx.h"

#include "SSysCallDPC.h"

#include "GameServer\GameServer\SMap.h"
#include "TripleS\Interpret\Src\IDataStack.h"
#include "IDPCSysCall.h"

IMPLEMENT_DYNAMIC ( CIDPCSysCall, CISyscall )

//////////////////////////////////////////////////////////////////////
// Operace se systémovým voláním
//////////////////////////////////////////////////////////////////////

// vytvoøení systémového volání DPC "pSysCallDPC" s návratovou hodnotou "pReturnValue" a 
//		s volajícím "pProcess" (zvyšuje poèet odkazù na objekt)
bool CIDPCSysCall::Create ( CIProcess *pProcess, CIDataStackItem *pReturnValue, 
	CSSysCallDPC *pSysCallDPC ) 
{
	ASSERT ( ( m_pSysCallDPC == NULL ) && ( m_pReturnValue == NULL ) );
	ASSERT ( ( pSysCallDPC != NULL ) && ( pReturnValue != NULL ) );

	// nechá vytvoøit pøedka
	VERIFY ( CISyscall::Create ( pProcess ) );

	// uschová si návratovou hodnotu systémového volání
	m_pReturnValue = pReturnValue;
	// uschová si odkaz na DPC
	m_pSysCallDPC = pSysCallDPC;

	return true;
}

// ukonèení systémového volání ze strany volaného (systémové volání DPC musí být zamèeno) 
//		(ukazatel na DPC dále není platný)
void CIDPCSysCall::FinishWaiting ( EInterpretStatus eStatus ) 
{
	ASSERT ( m_pSysCallDPC != NULL );

	// znièí odkaz na DPC
	m_pSysCallDPC = NULL;

	// ukonèí systémové volání
	CISyscall::FinishWaiting ( eStatus );
}

// zrušení systémového volání volajícím (ukazatele na DPC a návratovou hodnotu dále 
//		nejsou platné)
void CIDPCSysCall::Cancel ( CIProcess *pProcess ) 
{
	ASSERT ( m_pReturnValue != NULL );

	// zamkne zámek systémového volání
	VERIFY ( m_mutexLock.Lock () );

	// zjistí je-li ukazatel na DPC platný
	if ( m_pSysCallDPC != NULL )
	{	// ukazatel na DPC je platný
		// pokusí se odregistrovat DPC
		if ( g_cMap.UnregisterDPC ( m_pSysCallDPC ) )
		{	// podaøilo se odregistrovat DPC
			// znièí DPC
			delete m_pSysCallDPC;
		}
		// zneplatní ukazatel na DPC
		m_pSysCallDPC = NULL;
	}

	// zneplatní ukazatel na návratovou hodnotu
	m_pReturnValue = NULL;

	// zruší systémové volání
	CISyscall::Cancel ( pProcess );

	// odemkne zámek systémového volání
	VERIFY ( m_mutexLock.Unlock () );
}

//////////////////////////////////////////////////////////////////////
// Ukládání dat (CPersistentObject metody)
//////////////////////////////////////////////////////////////////////

/* CIDPCSysCall stored

	CISyscall stored
	CIDataStackItem *m_pReturnValue
*/

// ukládání dat
void CIDPCSysCall::PersistentSave ( CPersistentStorage &storage ) 
{
	BRACE_BLOCK ( storage );

	ASSERT ( ( m_pReturnValue != NULL ) && ( m_pSysCallDPC != NULL ) );

	CISyscall::PersistentSave ( storage );

	storage << m_pReturnValue;
}

// nahrávání pouze uložených dat
void CIDPCSysCall::PersistentLoad ( CPersistentStorage &storage ) 
{
	BRACE_BLOCK ( storage );

	ASSERT ( ( m_pReturnValue == NULL ) && ( m_pSysCallDPC == NULL ) );

	CISyscall::PersistentLoad ( storage );

	storage >> (void *&)m_pReturnValue;
	LOAD_ASSERT ( m_pReturnValue != NULL );
}

// pøeklad ukazatelù
void CIDPCSysCall::PersistentTranslatePointers ( CPersistentStorage &storage ) 
{
	CISyscall::PersistentTranslatePointers ( storage );

	m_pReturnValue = (CIDataStackItem *)storage.TranslatePointer ( m_pReturnValue );
}

// inicializace nahraného objektu
void CIDPCSysCall::PersistentInit ( CSSysCallDPC *pSysCallDPC ) 
{
	ASSERT ( pSysCallDPC != NULL );

	CISyscall::PersistentInit ();

	m_pSysCallDPC = pSysCallDPC;
}
