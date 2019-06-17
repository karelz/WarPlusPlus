/***********************************************************
 * 
 * Projekt: Strategick� hra
 *    ��st: Interpret
 *   Autor: Karel Zikmund
 * 
 *   Popis: T��da syst�mov�ho vol�n� pohybov� skilly
 * 
 ***********************************************************/

#include "StdAfx.h"

#include "ISysCallMove.h"

#include "SMoveSkillType.h"
#include "SMoveSkill.h"

IMPLEMENT_DYNAMIC(CISysCallMove, CIBagSkillSysCall)

//////////////////////////////////////////////////////////////////////
// Operace se syst�mov�m vol�n�m
//////////////////////////////////////////////////////////////////////

// zru�en� syst�mov�ho vol�n� volaj�c�m
void CISysCallMove::Cancel ( CIProcess *pProcess ) 
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
		SSMoveSkill *pMoveSkill = (SSMoveSkill *)(pSkillType)->GetSkillData ( pUnit );

		// zru�� syst�mov� vol�n� skilly
		ASSERT ( pMoveSkill->m_pSysCall == this );
		(void)Release ();
		pMoveSkill->m_pSysCall = NULL;

		// deaktivuje pohybovou skillu
		((CSMoveSkillType *)pSkillType)->DeactivateSkill ( pUnit );
	}

	// zru�en� syst�mov�ho vol�n� volaj�c�m (ukazatel na jednotku a bag parametr� 
	//		d�le nejsou platn�)
	CIBagSkillSysCall::Cancel ( pProcess );

	// odemkne jednotku pro z�pis
	pUnit->WriterUnlock ();
}
