/***********************************************************
 * 
 * Projekt: Strategicka hra
 *    Cast: Server hry - skilly
 *   Autor: Tomas Karban
 * 
 *   Popis: Abstraktni predek vsech defense skill -- implementace typu skilly
 * 
 ***********************************************************/

#include "StdAfx.h"

#include "SAbstractDefenseSkillType.h"

#define SKILLDATA(pUnit) (SSAbstractDefenseSkill *)GetSkillData(pUnit)

// implementace interfacu volani ze skriptu
BEGIN_SKILLCALL_INTERFACE_MAP(CSAbstractDefenseSkillType, CSSkillType)
	SKILLCALL_BASECLASS,  // IsActive()
	SKILLCALL_BASECLASS,  // GetName()
  SKILLCALL_METHOD(SCI_GetDefenseInteractionsMask),
  SKILLCALL_METHOD(SCI_QueryDefense)
END_SKILLCALL_INTERFACE_MAP ()

IMPLEMENT_DYNAMIC(CSAbstractDefenseSkillType, CSSkillType);

/////////////////////////////////////////////////////////////////////
// Interface do skriptu -- SCI_GetDefenseInteractionsMask
/////////////////////////////////////////////////////////////////////

ESyscallResult CSAbstractDefenseSkillType::SCI_GetDefenseInteractionsMask(CSUnit * /*pUnit*/, CIProcess * /*pProcess*/, CISyscall ** /*ppSysCall*/, CIBag * /*pBag*/, CIDataStackItem *pReturn)
{
	TRACE_SKILLCALL("@ Abstract defense - SCI_GetDefenseInteractionsMask() -- returns %#.4x\n", GetDefenseInteractionsMask());

    // pripravime navratovou hodnotu
    pReturn->Set((int) GetDefenseInteractionsMask());

	// uspech
	return SYSCALL_SUCCESS;
}


/////////////////////////////////////////////////////////////////////
// Interface do skriptu -- SCI_GetDefenseInteractionsMask
/////////////////////////////////////////////////////////////////////

ESyscallResult CSAbstractDefenseSkillType::SCI_QueryDefense(CSUnit *pUnit, CIProcess * /*pProcess*/, CISyscall ** /*ppSysCall*/, CIBag * /*pBag*/, CIDataStackItem *pReturn, int *pnAttackIntensity, int *pnInteractionsMask)
{
    // zavolame QueryDefense()
    int nDefenseIntensity = QueryDefense(pUnit, *pnAttackIntensity, (DWORD) *pnInteractionsMask);

    TRACE_SKILLCALL("@ Abstract attack - SCI_QueryDefense(unit %d, intensity %d, mask %#.4x) -- returns %d\n", pUnit->GetID(), *pnAttackIntensity, *pnInteractionsMask, nDefenseIntensity);

    // pripravime navratovou hodnotu
    pReturn->Set(nDefenseIntensity);

    // uspech
    return SYSCALL_SUCCESS;
}
