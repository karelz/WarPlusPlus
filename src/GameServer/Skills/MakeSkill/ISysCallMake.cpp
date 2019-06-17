/***********************************************************
 * 
 * Projekt: Strategická hra
 *    Èást: Interpret
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
// Operace se systémovým voláním
//////////////////////////////////////////////////////////////////////

// zrušení systémového volání volajícím
void CISysCallMake::Cancel ( CIProcess *pProcess ) 
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
		SSMakeSkillUnit *pMakeSkillUnit = (SSMakeSkillUnit *)(pSkillType)->GetSkillData ( pUnit );

		// zruší systémové volání skilly
		ASSERT ( pMakeSkillUnit->m_pSysCall == this );
		(void)Release ();
		pMakeSkillUnit->m_pSysCall = NULL;

		// deaktivuje vyrabeci skillu
		((CSMakeSkillType *)pSkillType)->DeactivateSkill ( pUnit );
	}

	// zrušení systémového volání volajícím (ukazatele na jednotku a na bag parametrù dále 
	//		nejsou platné)
	CIBagSkillSysCall::Cancel ( pProcess );

	// odemkne jednotku pro zápis
	pUnit->WriterUnlock ();
}
