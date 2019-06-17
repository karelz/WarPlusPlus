/***********************************************************
 * 
 * Projekt: Strategick� hra
 *    ��st: Interpret
 *   Autor: Roman �enka & Karel Zikmund
 * 
 *   Popis: T��da syst�mov�ho vol�n� DPC s n�vratovou hodnotou
 * 
 ***********************************************************/

#include "StdAfx.h"

#include "SSysCallDPC.h"

#include "GameServer\GameServer\SMap.h"
#include "TripleS\Interpret\Src\IDataStack.h"
#include "IDPCSysCall.h"

IMPLEMENT_DYNAMIC ( CIDPCSysCall, CISyscall )

//////////////////////////////////////////////////////////////////////
// Operace se syst�mov�m vol�n�m
//////////////////////////////////////////////////////////////////////

// vytvo�en� syst�mov�ho vol�n� DPC "pSysCallDPC" s n�vratovou hodnotou "pReturnValue" a 
//		s volaj�c�m "pProcess" (zvy�uje po�et odkaz� na objekt)
bool CIDPCSysCall::Create ( CIProcess *pProcess, CIDataStackItem *pReturnValue, 
	CSSysCallDPC *pSysCallDPC ) 
{
	ASSERT ( ( m_pSysCallDPC == NULL ) && ( m_pReturnValue == NULL ) );
	ASSERT ( ( pSysCallDPC != NULL ) && ( pReturnValue != NULL ) );

	// nech� vytvo�it p�edka
	VERIFY ( CISyscall::Create ( pProcess ) );

	// uschov� si n�vratovou hodnotu syst�mov�ho vol�n�
	m_pReturnValue = pReturnValue;
	// uschov� si odkaz na DPC
	m_pSysCallDPC = pSysCallDPC;

	return true;
}

// ukon�en� syst�mov�ho vol�n� ze strany volan�ho (syst�mov� vol�n� DPC mus� b�t zam�eno) 
//		(ukazatel na DPC d�le nen� platn�)
void CIDPCSysCall::FinishWaiting ( EInterpretStatus eStatus ) 
{
	ASSERT ( m_pSysCallDPC != NULL );

	// zni�� odkaz na DPC
	m_pSysCallDPC = NULL;

	// ukon�� syst�mov� vol�n�
	CISyscall::FinishWaiting ( eStatus );
}

// zru�en� syst�mov�ho vol�n� volaj�c�m (ukazatele na DPC a n�vratovou hodnotu d�le 
//		nejsou platn�)
void CIDPCSysCall::Cancel ( CIProcess *pProcess ) 
{
	ASSERT ( m_pReturnValue != NULL );

	// zamkne z�mek syst�mov�ho vol�n�
	VERIFY ( m_mutexLock.Lock () );

	// zjist� je-li ukazatel na DPC platn�
	if ( m_pSysCallDPC != NULL )
	{	// ukazatel na DPC je platn�
		// pokus� se odregistrovat DPC
		if ( g_cMap.UnregisterDPC ( m_pSysCallDPC ) )
		{	// poda�ilo se odregistrovat DPC
			// zni�� DPC
			delete m_pSysCallDPC;
		}
		// zneplatn� ukazatel na DPC
		m_pSysCallDPC = NULL;
	}

	// zneplatn� ukazatel na n�vratovou hodnotu
	m_pReturnValue = NULL;

	// zru�� syst�mov� vol�n�
	CISyscall::Cancel ( pProcess );

	// odemkne z�mek syst�mov�ho vol�n�
	VERIFY ( m_mutexLock.Unlock () );
}

//////////////////////////////////////////////////////////////////////
// Ukl�d�n� dat (CPersistentObject metody)
//////////////////////////////////////////////////////////////////////

/* CIDPCSysCall stored

	CISyscall stored
	CIDataStackItem *m_pReturnValue
*/

// ukl�d�n� dat
void CIDPCSysCall::PersistentSave ( CPersistentStorage &storage ) 
{
	BRACE_BLOCK ( storage );

	ASSERT ( ( m_pReturnValue != NULL ) && ( m_pSysCallDPC != NULL ) );

	CISyscall::PersistentSave ( storage );

	storage << m_pReturnValue;
}

// nahr�v�n� pouze ulo�en�ch dat
void CIDPCSysCall::PersistentLoad ( CPersistentStorage &storage ) 
{
	BRACE_BLOCK ( storage );

	ASSERT ( ( m_pReturnValue == NULL ) && ( m_pSysCallDPC == NULL ) );

	CISyscall::PersistentLoad ( storage );

	storage >> (void *&)m_pReturnValue;
	LOAD_ASSERT ( m_pReturnValue != NULL );
}

// p�eklad ukazatel�
void CIDPCSysCall::PersistentTranslatePointers ( CPersistentStorage &storage ) 
{
	CISyscall::PersistentTranslatePointers ( storage );

	m_pReturnValue = (CIDataStackItem *)storage.TranslatePointer ( m_pReturnValue );
}

// inicializace nahran�ho objektu
void CIDPCSysCall::PersistentInit ( CSSysCallDPC *pSysCallDPC ) 
{
	ASSERT ( pSysCallDPC != NULL );

	CISyscall::PersistentInit ();

	m_pSysCallDPC = pSysCallDPC;
}
