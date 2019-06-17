/***********************************************************
 * 
 * Projekt: Strategick� hra
 *    ��st: Interpret
 *   Autor: Roman Zenka
 * 
 *   Popis: Syscall pro Mine skillu (okopirovany CISysCallMake)
 * 
 ***********************************************************/

#ifndef __INTERPRET_SYSCALL_MINE__HEADER_INCLUDED__
#define __INTERPRET_SYSCALL_MINE__HEADER_INCLUDED__

#include "GameServer\Skills\SysCalls\IBagSkillSysCall.h"

//////////////////////////////////////////////////////////////////////
// Dop�edn� deklarace

// t��da typu pohybov� skilly
class CSMineSkillType;

//////////////////////////////////////////////////////////////////////
// T��da syst�mov�ho vol�n� pohybov� skilly.
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

// Operace se syst�mov�m vol�n�m
protected:
	// zru�en� syst�mov�ho vol�n� volaj�c�m
	virtual void Cancel ( CIProcess *pProcess );
};

#endif //__INTERPRET_SYSCALL_MINE__HEADER_INCLUDED__
