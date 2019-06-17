/***********************************************************
 * 
 * Projekt: Strategick� hra
 *    ��st: Interpret
 *   Autor: Roman Zenka
 * 
 *   Popis: Syscall pro Make skillu (okopirovany CISysCallMove)
 * 
 ***********************************************************/

#include "StdAfx.h"

#include "ISysCallMake.h"

#include "SMakeSkillType.h"
#include "SMakeSkill.h"

IMPLEMENT_DYNAMIC(CISysCallMake, CIBagSkillSysCall)

//////////////////////////////////////////////////////////////////////
// Operace se syst�mov�m vol�n�m
//////////////////////////////////////////////////////////////////////

// zru�en� syst�mov�ho vol�n� volaj�c�m
void CISysCallMake::Cancel ( CIProcess *pProcess ) 
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
		SSMakeSkillUnit *pMakeSkillUnit = (SSMakeSkillUnit *)(pSkillType)->GetSkillData ( pUnit );

		// zru�� syst�mov� vol�n� skilly
		ASSERT ( pMakeSkillUnit->m_pSysCall == this );
		(void)Release ();
		pMakeSkillUnit->m_pSysCall = NULL;

		// deaktivuje vyrabeci skillu
		((CSMakeSkillType *)pSkillType)->DeactivateSkill ( pUnit );
	}

	// zru�en� syst�mov�ho vol�n� volaj�c�m (ukazatele na jednotku a na bag parametr� d�le 
	//		nejsou platn�)
	CIBagSkillSysCall::Cancel ( pProcess );

	// odemkne jednotku pro z�pis
	pUnit->WriterUnlock ();
}
