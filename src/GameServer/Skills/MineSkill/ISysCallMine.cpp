/***********************************************************
 * 
 * Projekt: Strategická hra
 *    Èást: Interpret
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
// Operace se systémovým voláním
//////////////////////////////////////////////////////////////////////

// zrušení systémového volání volajícím
void CISysCallMine::Cancel ( CIProcess *pProcess ) 
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
		SSMineSkillUnit *pMineSkillUnit = (SSMineSkillUnit *)(pSkillType)->GetSkillData ( pUnit );

		// zruší systémové volání skilly
		ASSERT ( pMineSkillUnit->m_pSysCall == this );
		(void)Release ();
		pMineSkillUnit->m_pSysCall = NULL;

		// deaktivuje vyrabeci skillu
		((CSMineSkillType *)pSkillType)->DeactivateSkill ( pUnit );
	}

	// zrušení systémového volání volajícím (ukazatele na jednotku a na bag parametrù dále 
	//		nejsou platné)
	CIBagSkillSysCall::Cancel ( pProcess );

	// odemkne jednotku pro zápis
	pUnit->WriterUnlock ();
}
