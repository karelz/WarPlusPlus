/***********************************************************
 * 
 * Projekt: Strategická hra
 *    Èást: Interpret
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
// Operace se systémovým voláním
//////////////////////////////////////////////////////////////////////

// zrušení systémového volání volajícím
void CISysCallStorage::Cancel ( CIProcess *pProcess ) 
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
		SSStorageSkillUnit *pStorageSkillUnit = (SSStorageSkillUnit *)(pSkillType)->GetSkillData ( pUnit );

		// deaktivuje vyrabeci skillu
		((CSStorageSkillType *)pSkillType)->DeactivateSkill ( pUnit );
	}

	// zrušení systémového volání volajícím (ukazatele na jednotku a na bag parametrù dále 
	//		nejsou platné)
	CIBagSkillSysCall::Cancel ( pProcess );

	// odemkne jednotku pro zápis
	pUnit->WriterUnlock ();
}
