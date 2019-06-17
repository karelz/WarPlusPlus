/***********************************************************
 * 
 * Projekt: Strategick� hra
 *    ��st: Interpret
 *   Autor: Roman Zenka
 * 
 *   Popis: Syscall pro storage skillu (okopirovany CISysCallMake)
 * 
 ***********************************************************/

#ifndef __INTERPRET_SYSCALL_STORAGE__HEADER_INCLUDED__
#define __INTERPRET_SYSCALL_STORAGE__HEADER_INCLUDED__

#include "GameServer\Skills\SysCalls\IBagSkillSysCall.h"

//////////////////////////////////////////////////////////////////////
// Dop�edn� deklarace

// t��da typu pohybov� skilly
class CSStorageSkillType;

//////////////////////////////////////////////////////////////////////
// T��da syst�mov�ho vol�n� pohybov� skilly.
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

// Operace se syst�mov�m vol�n�m
protected:
	// zru�en� syst�mov�ho vol�n� volaj�c�m
	virtual void Cancel ( CIProcess *pProcess );
};

#endif //__INTERPRET_SYSCALL_STORAGE__HEADER_INCLUDED__
