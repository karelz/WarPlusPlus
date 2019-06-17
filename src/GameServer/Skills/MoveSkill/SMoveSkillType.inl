/***********************************************************
 * 
 * Projekt: Strategická hra
 *    Èást: Server hry - skilly
 *   Autor: Vít Karas & Karel Zikmund
 * 
 *   Popis: Typ pohybové skilly (move skilla)
 * 
 ***********************************************************/

#ifndef __SERVER_MOVE_SKILL_TYPE__INLINE_INCLUDED__
#define __SERVER_MOVE_SKILL_TYPE__INLINE_INCLUDED__

#include "TripleS\Interpret\Src\ISyscall.h"

/////////////////////////////////////////////////////////////////////
// Pomocné metody skilly
/////////////////////////////////////////////////////////////////////

// ukonèí skillu s výsledkem "eResult" (jednotka "pUnit" musí být zamèena pro zápis)
inline void CSMoveSkillType::Finish ( CSUnit *pUnit, enum EGoResult eResult ) 
{
	SSMoveSkill *pData = (SSMoveSkill *)GetSkillData ( pUnit );

	// zjistí, je-li platné systémové volání
	if ( pData->m_pSysCall != NULL )
	{	// systémové volání je platné

		TRACE_SKILL2 ( "@ MoveSkill on unit %d returned %d\n", pUnit->GetID (), eResult );

		// nastaví návratovou hodnotu
		pData->m_pSysCall->GetBag ()->ReturnInt ( eResult );
		// ukonèí èekající systémové volání
		pData->m_pSysCall->FinishWaiting ( STATUS_SUCCESS );
		// zruší odkaz na èekající systémové volání
		pData->m_pSysCall->Release ();
		pData->m_pSysCall = NULL;
	}

	// nastaví ID defaultního vzhledu jednotky (volá SetBriefInfoModified, je-li potøeba)
	pUnit->SetDefaultAppearanceID ();

	// ukonèí skillu
	DeactivateSkill ( pUnit );
}

#endif //__SERVER_MOVE_SKILL_TYPE__INLINE_INCLUDED__
