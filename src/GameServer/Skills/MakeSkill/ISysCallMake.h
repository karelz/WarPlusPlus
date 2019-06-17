/***********************************************************
 * 
 * Projekt: Strategická hra
 *    Èást: Interpret
 *   Autor: Roman Zenka
 * 
 *   Popis: Syscall pro Make skillu (okopirovany CISysCallMove
 * 
 ***********************************************************/

#ifndef __INTERPRET_SYSCALL_MAKE__HEADER_INCLUDED__
#define __INTERPRET_SYSCALL_MAKE__HEADER_INCLUDED__

#include "GameServer\Skills\SysCalls\IBagSkillSysCall.h"

//////////////////////////////////////////////////////////////////////
// Dopøedné deklarace

// tøída typu pohybové skilly
class CSMakeSkillType;

//////////////////////////////////////////////////////////////////////
// Tøída systémového volání pohybové skilly.
class CISysCallMake : public CIBagSkillSysCall 
{
	DECLARE_DYNAMIC ( CISysCallMake )

// Metody
// Konstrukce a destrukce
public:
	// kontruktor
	CISysCallMake () {};
	// destruktor
	virtual ~CISysCallMake () {};

// Operace se systémovým voláním
protected:
	// zrušení systémového volání volajícím
	virtual void Cancel ( CIProcess *pProcess );
};

#endif //__INTERPRET_SYSCALL_MAKE__HEADER_INCLUDED__
