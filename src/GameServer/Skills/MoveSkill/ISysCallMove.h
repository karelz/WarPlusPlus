/***********************************************************
 * 
 * Projekt: Strategická hra
 *    Èást: Interpret
 *   Autor: Karel Zikmund
 * 
 *   Popis: Tøída systémového volání pohybové skilly
 * 
 ***********************************************************/

#ifndef __INTERPRET_SYSCALL_MOVE__HEADER_INCLUDED__
#define __INTERPRET_SYSCALL_MOVE__HEADER_INCLUDED__

#include "GameServer\Skills\SysCalls\IBagSkillSysCall.h"

//////////////////////////////////////////////////////////////////////
// Dopøedné deklarace

// tøída typu pohybové skilly
class CSMoveSkillType;

//////////////////////////////////////////////////////////////////////
// Tøída systémového volání pohybové skilly.
class CISysCallMove : public CIBagSkillSysCall 
{
	DECLARE_DYNAMIC ( CISysCallMove )

// Metody
public:
// Konstrukce a destrukce

	// kontruktor
	CISysCallMove () {};
	// destruktor
	virtual ~CISysCallMove () {};

protected:
// Operace se systémovým voláním

	// zrušení systémového volání volajícím
	virtual void Cancel ( CIProcess *pProcess );
};

#endif //__INTERPRET_SYSCALL_MOVE__HEADER_INCLUDED__
