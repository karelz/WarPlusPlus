/***********************************************************
 * 
 * Projekt: Strategick� hra
 *    ��st: Interpret
 *   Autor: Roman Zenka
 * 
 *   Popis: Syscall pro Storage skillu (okopirovany CISysCallMake)
 * 
 ***********************************************************/

#include "StdAfx.h"

#include "ISysCallStorage.h"

#include "SStorageSkillType.h"
#include "SStorageSkill.h"

IMPLEMENT_DYNAMIC(CISysCallStorage, CIBagSkillSysCall)

//////////////////////////////////////////////////////////////////////
// Operace se syst�mov�m vol�n�m
//////////////////////////////////////////////////////////////////////

// zru�en� syst�mov�ho vol�n� volaj�c�m
void CISysCallStorage::Cancel ( CIProcess *pProcess ) 
{
	// z�sk� ukazatel na jednotku
	CSUnit *pUnit = GetUnit ();
	ASSERT ( pUnit != NULL );

	// zamkne jednotku pro z�pis
	VERIFY ( pUnit->WriterLock () );

	// zjist� ukazatel na typ skilly
	CSSkillType *pSkillType = GetSkillType ();

	// zjist�, je-li volan� skilla jednotky platn�
	if ( pSkillType != NULL )
	{	// volan� skilla jednotky je platn�
		ASSERT ( pUnit->GetUnitType () == pSkillType->GetUnitType () );
		// zjist� ukazatel na data skilly
		SSStorageSkillUnit *pStorageSkillUnit = (SSStorageSkillUnit *)(pSkillType)->GetSkillData ( pUnit );

		// deaktivuje vyrabeci skillu
		((CSStorageSkillType *)pSkillType)->DeactivateSkill ( pUnit );
	}

	// zru�en� syst�mov�ho vol�n� volaj�c�m (ukazatele na jednotku a na bag parametr� d�le 
	//		nejsou platn�)
	CIBagSkillSysCall::Cancel ( pProcess );

	// odemkne jednotku pro z�pis
	pUnit->WriterUnlock ();
}
