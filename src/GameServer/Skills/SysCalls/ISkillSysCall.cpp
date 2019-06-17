/***********************************************************
 * 
 * Projekt: Strategická hra
 *    Èást: Interpret
 *   Autor: Karel Zikmund
 * 
 *   Popis: Tøída systémového volání na skille
 * 
 ***********************************************************/

#include "StdAfx.h"

#include "ISkillSysCall.h"

IMPLEMENT_DYNAMIC ( CISkillSysCall, CISyscall )

//////////////////////////////////////////////////////////////////////
// Operace se systémovým voláním
//////////////////////////////////////////////////////////////////////

// vytvoøení systémového volání na skille typu "pSkillType" jednotky "pUnit" s volajícím 
//		"pProcess" (zvyšuje poèet odkazù na objekt)
bool CISkillSysCall::Create ( CIProcess *pProcess, CSUnit *pUnit, 
	CSSkillType *pSkillType ) 
{
	ASSERT ( ( m_pUnit == NULL ) && ( m_pSkillType == NULL ) );
	ASSERT ( ( pUnit != NULL ) && ( pSkillType != NULL ) );
	ASSERT ( pUnit->GetUnitType () == pSkillType->GetUnitType () );

	// nechá vytvoøit pøedka
	VERIFY ( CISyscall::Create ( pProcess ) );

	// uschová si informaci o volaném
	m_pUnit = pUnit;
	m_pSkillType = pSkillType;

	// zvýší poèet odkazù na jednotku
	pUnit->AddRef ();

	return true;
}

// znièí objekt systémového volání
void CISkillSysCall::Delete () 
{
	// zjistí, je-li ukazatel na jednotku platný
	if ( m_pUnit != NULL )
	{	// ukazatel na jednotku je platný
		// sníží poèet odkazù na jednotku
		m_pUnit->Release ();
		// znehodnotí ukazatel na jednotku
		m_pUnit = NULL;
	}
	// znehodnotí ukazatel na typ skilly
	m_pSkillType = NULL;

	// znièí pøedka
	CISyscall::Delete ();
}

// ukonèení systémového volání ze strany volané skilly na jednotce (jednotka musí být 
//		zamèena pro zápis) (ukazatel na volanou skillu dále není platný)
void CISkillSysCall::FinishWaiting ( EInterpretStatus eStatus ) 
{
	ASSERT ( m_pSkillType != NULL );

	// oznaèí volanou skillu jednotky za neplatnou
	m_pSkillType = NULL;

	// ukonèí systémové volání na pøedkovi
	CISyscall::FinishWaiting ( eStatus );
}

// zrušení systémového volání volajícím (zámek skilly=zámek jednotky musí být zamèen pro 
//		zápis) (ukazatel na jednotku dále není platný)
void CISkillSysCall::Cancel ( CIProcess *pProcess ) 
{
	ASSERT ( m_pUnit != NULL );

	// sníží poèet odkazù na jednotku
	m_pUnit->Release ();
	// zruší odkaz na jednotku
	m_pUnit = NULL;

	// zruší systémové volání na pøedkovi
	CISyscall::Cancel ( pProcess );
}

//////////////////////////////////////////////////////////////////////
// Ukládání dat (CPersistentObject metody)
//////////////////////////////////////////////////////////////////////

/*
CISkillSysCall stored

	CISyscall stored
	CSUnit *m_pUnit
	CSSkillType *m_pSkillType
*/

// ukládání dat
void CISkillSysCall::PersistentSave ( CPersistentStorage &storage ) 
{
	BRACE_BLOCK ( storage );

	ASSERT ( m_pSkillType != NULL );

	CISyscall::PersistentSave ( storage );

	storage << m_pUnit;
	storage << m_pSkillType;
}

// nahrávání pouze uložených dat
void CISkillSysCall::PersistentLoad ( CPersistentStorage &storage ) 
{
	BRACE_BLOCK ( storage );

	ASSERT ( ( m_pUnit == NULL ) && ( m_pSkillType == NULL ) );

	CISyscall::PersistentLoad ( storage );

	storage >> (void *&)m_pUnit;
	storage >> (void *&)m_pSkillType;
	LOAD_ASSERT ( m_pSkillType != NULL );
}

// pøeklad ukazatelù
void CISkillSysCall::PersistentTranslatePointers ( CPersistentStorage &storage ) 
{
	CISyscall::PersistentTranslatePointers ( storage );

	m_pUnit = (CSUnit *)storage.TranslatePointer ( m_pUnit );
	m_pSkillType = (CSSkillType *)storage.TranslatePointer ( m_pSkillType );
}

// inicializace nahraného objektu
void CISkillSysCall::PersistentInit () 
{
	CISyscall::PersistentInit ();
}
