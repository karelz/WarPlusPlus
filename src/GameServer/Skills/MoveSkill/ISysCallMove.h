/***********************************************************
 * 
 * Projekt: Strategick� hra
 *    ��st: Interpret
 *   Autor: Karel Zikmund
 * 
 *   Popis: T��da syst�mov�ho vol�n� pohybov� skilly
 * 
 ***********************************************************/

#ifndef __INTERPRET_SYSCALL_MOVE__HEADER_INCLUDED__
#define __INTERPRET_SYSCALL_MOVE__HEADER_INCLUDED__

#include "GameServer\Skills\SysCalls\IBagSkillSysCall.h"

//////////////////////////////////////////////////////////////////////
// Dop�edn� deklarace

// t��da typu pohybov� skilly
class CSMoveSkillType;

//////////////////////////////////////////////////////////////////////
// T��da syst�mov�ho vol�n� pohybov� skilly.
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
// Operace se syst�mov�m vol�n�m

	// zru�en� syst�mov�ho vol�n� volaj�c�m
	virtual void Cancel ( CIProcess *pProcess );
};

#endif //__INTERPRET_SYSCALL_MOVE__HEADER_INCLUDED__
