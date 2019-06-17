/***********************************************************
 * 
 * Projekt: Strategick� hra
 *    ��st: Interpret
 *   Autor: Roman Zenka
 * 
 *   Popis: Syscall pro Mine skillu (okopirovany CISysCallMake)
 * 
 ***********************************************************/

#include "StdAfx.h"

#include "ISysCallMine.h"

#include "SMineSkillType.h"
#include "SMineSkill.h"

IMPLEMENT_DYNAMIC(CISysCallMine, CIBagSkillSysCall)

//////////////////////////////////////////////////////////////////////
// Operace se syst�mov�m vol�n�m
//////////////////////////////////////////////////////////////////////

// zru�en� syst�mov�ho vol�n� volaj�c�m
void CISysCallMine::Cancel ( CIProcess *pProcess ) 
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
		SSMineSkillUnit *pMineSkillUnit = (SSMineSkillUnit *)(pSkillType)->GetSkillData ( pUnit );

		// zru�� syst�mov� vol�n� skilly
		ASSERT ( pMineSkillUnit->m_pSysCall == this );
		(void)Release ();
		pMineSkillUnit->m_pSysCall = NULL;

		// deaktivuje vyrabeci skillu
		((CSMineSkillType *)pSkillType)->DeactivateSkill ( pUnit );
	}

	// zru�en� syst�mov�ho vol�n� volaj�c�m (ukazatele na jednotku a na bag parametr� d�le 
	//		nejsou platn�)
	CIBagSkillSysCall::Cancel ( pProcess );

	// odemkne jednotku pro z�pis
	pUnit->WriterUnlock ();
}
