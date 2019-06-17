/***********************************************************
 * 
 * Projekt: Strategická hra
 *    Èást: Interpret
 *   Autor: Roman Zenka
 * 
 *   Popis: Syscall pro storage skillu (okopirovany CISysCallMake)
 * 
 ***********************************************************/

#ifndef __INTERPRET_SYSCALL_STORAGE__HEADER_INCLUDED__
#define __INTERPRET_SYSCALL_STORAGE__HEADER_INCLUDED__

#include "GameServer\Skills\SysCalls\IBagSkillSysCall.h"

//////////////////////////////////////////////////////////////////////
// Dopøedné deklarace

// tøída typu pohybové skilly
class CSStorageSkillType;

//////////////////////////////////////////////////////////////////////
// Tøída systémového volání pohybové skilly.
class CISysCallStorage : public CIBagSkillSysCall 
{
	DECLARE_DYNAMIC ( CISysCallStorage )

// Metody
// Konstrukce a destrukce
public:
	// kontruktor
	CISysCallStorage () {};
	// destruktor
	virtual ~CISysCallStorage () {};

// Operace se systémovým voláním
protected:
	// zrušení systémového volání volajícím
	virtual void Cancel ( CIProcess *pProcess );
};

#endif //__INTERPRET_SYSCALL_STORAGE__HEADER_INCLUDED__
