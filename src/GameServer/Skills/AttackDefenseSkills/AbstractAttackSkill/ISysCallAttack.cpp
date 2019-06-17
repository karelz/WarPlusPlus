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

#include "ISysCallAttack.h"

#include "SAbstractAttackSkillType.h"
#include "SAbstractAttackSkill.h"

IMPLEMENT_DYNAMIC(CISysCallAttack, CIBagSkillSysCall);

//////////////////////////////////////////////////////////////////////
// zruseni systemoveho volani volajicim
//////////////////////////////////////////////////////////////////////

void CISysCallAttack::Cancel(CIProcess *pProcess)
{
	// sezeneme ukazatel na jednotku
	CSUnit *pUnit = GetUnit();
	ASSERT(pUnit != NULL);

	// zamkneme jednotku pro zapis
	VERIFY(pUnit->WriterLock());

	// sezeneme ukazatel na typ skilly
	CSAbstractAttackSkillType *pSkillType = (CSAbstractAttackSkillType *) GetSkillType();

	// zjist�, je-li volan� skilla jednotky platn�
	if (pSkillType != NULL)
	{
		// je to spravny skilltype
		ASSERT_KINDOF(CSAbstractAttackSkillType, pSkillType);
		// volana skilla jednotky je platna
		ASSERT(pUnit->GetUnitType() == pSkillType->GetUnitType());

		// zjistime ukazatel na data skilly
		SSAbstractAttackSkill *pSkill = (SSAbstractAttackSkill *) pSkillType->GetSkillData(pUnit);

		// osetrime, ze je syscall opravdu aktivni
		ASSERT(pSkill->m_pSysCall == this);

		// zrusime syscall u jednotky
		(void)Release ();
		pSkill->m_pSysCall = NULL;

		// deaktivuje attack skillu
		pSkillType->DeactivateSkill(pUnit);
	}

	// zavolame predka (ukazatele na jednotku a na bag parametru dale nejsou platne)
	CIBagSkillSysCall::Cancel(pProcess);

	// odemkne jednotku
	pUnit->WriterUnlock();
}
