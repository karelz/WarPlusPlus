/***********************************************************
 * 
 * Projekt: Strategick� hra
 *    ��st: Interpret
 *   Autor: Karel Zikmund
 * 
 *   Popis: T��da syst�mov�ho vol�n� na skille
 * 
 ***********************************************************/

#include "StdAfx.h"

#include "ISkillSysCall.h"

IMPLEMENT_DYNAMIC ( CISkillSysCall, CISyscall )

//////////////////////////////////////////////////////////////////////
// Operace se syst�mov�m vol�n�m
//////////////////////////////////////////////////////////////////////

// vytvo�en� syst�mov�ho vol�n� na skille typu "pSkillType" jednotky "pUnit" s volaj�c�m 
//		"pProcess" (zvy�uje po�et odkaz� na objekt)
bool CISkillSysCall::Create ( CIProcess *pProcess, CSUnit *pUnit, 
	CSSkillType *pSkillType ) 
{
	ASSERT ( ( m_pUnit == NULL ) && ( m_pSkillType == NULL ) );
	ASSERT ( ( pUnit != NULL ) && ( pSkillType != NULL ) );
	ASSERT ( pUnit->GetUnitType () == pSkillType->GetUnitType () );

	// nech� vytvo�it p�edka
	VERIFY ( CISyscall::Create ( pProcess ) );

	// uschov� si informaci o volan�m
	m_pUnit = pUnit;
	m_pSkillType = pSkillType;

	// zv��� po�et odkaz� na jednotku
	pUnit->AddRef ();

	return true;
}

// zni�� objekt syst�mov�ho vol�n�
void CISkillSysCall::Delete () 
{
	// zjist�, je-li ukazatel na jednotku platn�
	if ( m_pUnit != NULL )
	{	// ukazatel na jednotku je platn�
		// sn�� po�et odkaz� na jednotku
		m_pUnit->Release ();
		// znehodnot� ukazatel na jednotku
		m_pUnit = NULL;
	}
	// znehodnot� ukazatel na typ skilly
	m_pSkillType = NULL;

	// zni�� p�edka
	CISyscall::Delete ();
}

// ukon�en� syst�mov�ho vol�n� ze strany volan� skilly na jednotce (jednotka mus� b�t 
//		zam�ena pro z�pis) (ukazatel na volanou skillu d�le nen� platn�)
void CISkillSysCall::FinishWaiting ( EInterpretStatus eStatus ) 
{
	ASSERT ( m_pSkillType != NULL );

	// ozna�� volanou skillu jednotky za neplatnou
	m_pSkillType = NULL;

	// ukon�� syst�mov� vol�n� na p�edkovi
	CISyscall::FinishWaiting ( eStatus );
}

// zru�en� syst�mov�ho vol�n� volaj�c�m (z�mek skilly=z�mek jednotky mus� b�t zam�en pro 
//		z�pis) (ukazatel na jednotku d�le nen� platn�)
void CISkillSysCall::Cancel ( CIProcess *pProcess ) 
{
	ASSERT ( m_pUnit != NULL );

	// sn�� po�et odkaz� na jednotku
	m_pUnit->Release ();
	// zru�� odkaz na jednotku
	m_pUnit = NULL;

	// zru�� syst�mov� vol�n� na p�edkovi
	CISyscall::Cancel ( pProcess );
}

//////////////////////////////////////////////////////////////////////
// Ukl�d�n� dat (CPersistentObject metody)
//////////////////////////////////////////////////////////////////////

/*
CISkillSysCall stored

	CISyscall stored
	CSUnit *m_pUnit
	CSSkillType *m_pSkillType
*/

// ukl�d�n� dat
void CISkillSysCall::PersistentSave ( CPersistentStorage &storage ) 
{
	BRACE_BLOCK ( storage );

	ASSERT ( m_pSkillType != NULL );

	CISyscall::PersistentSave ( storage );

	storage << m_pUnit;
	storage << m_pSkillType;
}

// nahr�v�n� pouze ulo�en�ch dat
void CISkillSysCall::PersistentLoad ( CPersistentStorage &storage ) 
{
	BRACE_BLOCK ( storage );

	ASSERT ( ( m_pUnit == NULL ) && ( m_pSkillType == NULL ) );

	CISyscall::PersistentLoad ( storage );

	storage >> (void *&)m_pUnit;
	storage >> (void *&)m_pSkillType;
	LOAD_ASSERT ( m_pSkillType != NULL );
}

// p�eklad ukazatel�
void CISkillSysCall::PersistentTranslatePointers ( CPersistentStorage &storage ) 
{
	CISyscall::PersistentTranslatePointers ( storage );

	m_pUnit = (CSUnit *)storage.TranslatePointer ( m_pUnit );
	m_pSkillType = (CSSkillType *)storage.TranslatePointer ( m_pSkillType );
}

// inicializace nahran�ho objektu
void CISkillSysCall::PersistentInit () 
{
	CISyscall::PersistentInit ();
}
