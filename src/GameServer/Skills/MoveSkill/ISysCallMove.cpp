/***********************************************************
 * 
 * Projekt: Strategická hra
 *    Èást: Interpret
 *   Autor: Karel Zikmund
 * 
 *   Popis: Tøída systémového volání pohybové skilly
 * 
 ***********************************************************/

#include "StdAfx.h"

#include "ISysCallMove.h"

#include "SMoveSkillType.h"
#include "SMoveSkill.h"

IMPLEMENT_DYNAMIC(CISysCallMove, CIBagSkillSysCall)

//////////////////////////////////////////////////////////////////////
// Operace se systémovým voláním
//////////////////////////////////////////////////////////////////////

// zrušení systémového volání volajícím
void CISysCallMove::Cancel ( CIProcess *pProcess ) 
{
	// získá ukazatel na jednotku
	CSUnit *pUnit = GetUnit ();
	ASSERT ( pUnit != NULL );

	// zamkne jednotku pro zápis
	VERIFY ( pUnit->WriterLock () );

	// zjistí ukazatel na typ skilly
	CSSkillType *pSkillType = GetSkillType ();

	// zjistí, je-li volaná skilla jednotky platná
	if ( pSkillType != NULL )
	{	// volaná skilla jednotky je platná
		ASSERT ( pUnit->GetUnitType () == pSkillType->GetUnitType () );
		// zjistí ukazatel na data skilly
		SSMoveSkill *pMoveSkill = (SSMoveSkill *)(pSkillType)->GetSkillData ( pUnit );

		// zruší systémové volání skilly
		ASSERT ( pMoveSkill->m_pSysCall == this );
		(void)Release ();
		pMoveSkill->m_pSysCall = NULL;

		// deaktivuje pohybovou skillu
		((CSMoveSkillType *)pSkillType)->DeactivateSkill ( pUnit );
	}

	// zrušení systémového volání volajícím (ukazatel na jednotku a bag parametrù 
	//		dále nejsou platné)
	CIBagSkillSysCall::Cancel ( pProcess );

	// odemkne jednotku pro zápis
	pUnit->WriterUnlock ();
}
