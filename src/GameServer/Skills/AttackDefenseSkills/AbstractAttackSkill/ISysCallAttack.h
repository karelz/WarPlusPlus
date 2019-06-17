/***********************************************************
 * 
 * Projekt: Strategicka hra
 *    Cast: Interpret
 *   Autor: Tomas Karban
 * 
 *   Popis: SysCall pro bullet attack skillu
 * 
 ***********************************************************/

#ifndef __INTERPRET_SYSCALL_BULLET_ATTACK__HEADER_INCLUDED__
#define __INTERPRET_SYSCALL_BULLET_ATTACK__HEADER_INCLUDED__

#include "GameServer\Skills\SysCalls\IBagSkillSysCall.h"

// forward deklarace
class CSAttackSkillType;

//////////////////////////////////////////////////////////////////////
// SysCall pro bullet attack skillu

class CISysCallAttack : public CIBagSkillSysCall 
{
    DECLARE_DYNAMIC(CISysCallAttack);

	friend class CSAttackSkillType;

// Konstrukce a destrukce
public:
    // kontruktor
	CISysCallAttack() {};
	// destruktor
	virtual ~CISysCallAttack() {};

// Operace na syscallu
protected:
	// zruseni systemoveho volani volajicim
	virtual void Cancel(CIProcess *pProcess);
};

#endif  // __INTERPRET_SYSCALL_BULLET_ATTACK__HEADER_INCLUDED__
