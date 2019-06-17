/***********************************************************
 * 
 * Projekt: Strategick� hra
 *    ��st: Interpret
 *   Autor: Roman Zenka
 * 
 *   Popis: Syscall pro Make skillu (okopirovany CISysCallMove
 * 
 ***********************************************************/

#ifndef __INTERPRET_SYSCALL_MAKE__HEADER_INCLUDED__
#define __INTERPRET_SYSCALL_MAKE__HEADER_INCLUDED__

#include "GameServer\Skills\SysCalls\IBagSkillSysCall.h"

//////////////////////////////////////////////////////////////////////
// Dop�edn� deklarace

// t��da typu pohybov� skilly
class CSMakeSkillType;

//////////////////////////////////////////////////////////////////////
// T��da syst�mov�ho vol�n� pohybov� skilly.
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

// Operace se syst�mov�m vol�n�m
protected:
	// zru�en� syst�mov�ho vol�n� volaj�c�m
	virtual void Cancel ( CIProcess *pProcess );
};

#endif //__INTERPRET_SYSCALL_MAKE__HEADER_INCLUDED__
