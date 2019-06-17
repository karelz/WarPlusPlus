/***********************************************************
 * 
 * Projekt: Strategick� hra
 *    ��st: Interpret
 *   Autor: Karel Zikmund
 * 
 *   Popis: T��da syst�mov�ho vol�n� na skille s n�vratovou hodnotou
 * 
 ***********************************************************/

#include "StdAfx.h"

#include "IBagSkillSysCall.h"

IMPLEMENT_DYNAMIC ( CIBagSkillSysCall, CISkillSysCall )

//////////////////////////////////////////////////////////////////////
// Operace se syst�mov�m vol�n�m
//////////////////////////////////////////////////////////////////////

// vytvo�en� syst�mov�ho vol�n� na skille typu "pSkillType" jednotky "pUnit" s parametry 
//		"pBag" a s volaj�c�m "pProcess" (zvy�uje po�et odkaz� na objekt)
bool CIBagSkillSysCall::Create ( CIProcess *pProcess, CSUnit *pUnit, 
	CSSkillType *pSkillType, CIBag *pBag ) 
{
	ASSERT ( m_pBag == NULL );
	ASSERT ( pBag != NULL );

	// nech� vytvo�it p�edka
	VERIFY ( CISkillSysCall::Create ( pProcess, pUnit, pSkillType ) );

	// uschov� si parametry syst�mov�ho vol�n�
	m_pBag = pBag;

	return true;
}

// zru�en� syst�mov�ho vol�n� volaj�c�m (z�mek skilly=z�mek jednotky mus� b�t zam�en pro 
//		z�pis) (ukazatele na jednotku a na bag parametr� d�le nejsou platn�)
void CIBagSkillSysCall::Cancel ( CIProcess *pProcess ) 
{
	ASSERT ( m_pBag != NULL );

	// znehodnot� parametry syst�mov�ho vol�n�
	m_pBag = NULL;

	// zru�en� syst�mov�ho vol�n� volaj�c�m (z�mek skilly=z�mek jednotky mus� b�t zam�en 
	//		pro z�pis) (ukazatel na jednotku d�le nen� platn�)
	CISkillSysCall::Cancel ( pProcess );
}

//////////////////////////////////////////////////////////////////////
// Ukl�d�n� dat (CPersistentObject metody)
//////////////////////////////////////////////////////////////////////

/*
CIBagSkillSysCall stored

	CISkillSysCall stored
	CIBag *m_pBag
*/

// ukl�d�n� dat
void CIBagSkillSysCall::PersistentSave ( CPersistentStorage &storage ) 
{
	BRACE_BLOCK ( storage );

	CISkillSysCall::PersistentSave ( storage );

	storage << m_pBag;
}

// nahr�v�n� pouze ulo�en�ch dat
void CIBagSkillSysCall::PersistentLoad ( CPersistentStorage &storage ) 
{
	BRACE_BLOCK ( storage );

	ASSERT ( m_pBag == NULL );

	CISkillSysCall::PersistentLoad ( storage );

	storage >> (void *&)m_pBag;
}

// p�eklad ukazatel�
void CIBagSkillSysCall::PersistentTranslatePointers ( CPersistentStorage &storage ) 
{
	CISkillSysCall::PersistentTranslatePointers ( storage );

	m_pBag = (CIBag *)storage.TranslatePointer ( m_pBag );
}

// inicializace nahran�ho objektu
void CIBagSkillSysCall::PersistentInit () 
{
	CISkillSysCall::PersistentInit ();
}
