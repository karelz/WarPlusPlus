/***********************************************************
 * 
 * Projekt: Strategick� hra
 *    ��st: Server hry - skilly
 *   Autor: V�t Karas & Karel Zikmund
 * 
 *   Popis: Typ pohybov� skilly (move skilla)
 * 
 ***********************************************************/

#ifndef __SERVER_MOVE_SKILL_TYPE__INLINE_INCLUDED__
#define __SERVER_MOVE_SKILL_TYPE__INLINE_INCLUDED__

#include "TripleS\Interpret\Src\ISyscall.h"

/////////////////////////////////////////////////////////////////////
// Pomocn� metody skilly
/////////////////////////////////////////////////////////////////////

// ukon�� skillu s v�sledkem "eResult" (jednotka "pUnit" mus� b�t zam�ena pro z�pis)
inline void CSMoveSkillType::Finish ( CSUnit *pUnit, enum EGoResult eResult ) 
{
	SSMoveSkill *pData = (SSMoveSkill *)GetSkillData ( pUnit );

	// zjist�, je-li platn� syst�mov� vol�n�
	if ( pData->m_pSysCall != NULL )
	{	// syst�mov� vol�n� je platn�

		TRACE_SKILL2 ( "@ MoveSkill on unit %d returned %d\n", pUnit->GetID (), eResult );

		// nastav� n�vratovou hodnotu
		pData->m_pSysCall->GetBag ()->ReturnInt ( eResult );
		// ukon�� �ekaj�c� syst�mov� vol�n�
		pData->m_pSysCall->FinishWaiting ( STATUS_SUCCESS );
		// zru�� odkaz na �ekaj�c� syst�mov� vol�n�
		pData->m_pSysCall->Release ();
		pData->m_pSysCall = NULL;
	}

	// nastav� ID defaultn�ho vzhledu jednotky (vol� SetBriefInfoModified, je-li pot�eba)
	pUnit->SetDefaultAppearanceID ();

	// ukon�� skillu
	DeactivateSkill ( pUnit );
}

#endif //__SERVER_MOVE_SKILL_TYPE__INLINE_INCLUDED__
