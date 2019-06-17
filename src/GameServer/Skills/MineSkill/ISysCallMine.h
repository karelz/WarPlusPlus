/***********************************************************
 * 
 * Projekt: Strategická hra
 *    Èást: Interpret
 *   Autor: Roman Zenka
 * 
 *   Popis: Syscall pro Mine skillu (okopirovany CISysCallMake)
 * 
 ***********************************************************/

#ifndef __INTERPRET_SYSCALL_MINE__HEADER_INCLUDED__
#define __INTERPRET_SYSCALL_MINE__HEADER_INCLUDED__

#include "GameServer\Skills\SysCalls\IBagSkillSysCall.h"

//////////////////////////////////////////////////////////////////////
// Dopøedné deklarace

// tøída typu pohybové skilly
class CSMineSkillType;

//////////////////////////////////////////////////////////////////////
// Tøída systémového volání pohybové skilly.
class CISysCallMine : public CIBagSkillSysCall 
{
	DECLARE_DYNAMIC ( CISysCallMine )

// Metody
// Konstrukce a destrukce
public:
	// kontruktor
	CISysCallMine () {};
	// destruktor
	virtual ~CISysCallMine () {};

// Operace se systémovým voláním
protected:
	// zrušení systémového volání volajícím
	virtual void Cancel ( CIProcess *pProcess );
};

#endif //__INTERPRET_SYSCALL_MINE__HEADER_INCLUDED__
