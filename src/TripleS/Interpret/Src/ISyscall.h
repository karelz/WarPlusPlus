/***********************************************************
 * 
 * Projekt: Strategick� hra
 *    ��st: Interpret
 *   Autor: Helena Kupkov� & Karel Zikmund
 * 
 *   Popis: T��da syst�mov�ho vol�n�
 * 
 ***********************************************************/

#ifndef __INTERPRET_SYSCALL__HEADER_INCLUDED__
#define __INTERPRET_SYSCALL__HEADER_INCLUDED__

#include "IWaitingInterface.h"

//////////////////////////////////////////////////////////////////////
// T��da syst�mov�ho vol�n�.
class CISyscall : public CIWaitingInterface 
{
	DECLARE_DYNAMIC ( CISyscall )

// Metody
public:
// Konstrukce a destrukce

	// konstruktor
	CISyscall () : CIWaitingInterface() { m_eWaitingType = SYSCALL_WAITING; };
	// destruktor
	virtual ~CISyscall () {};

private:
// Zak�zan� metody vytv��en� objektu

	// vytvo�en� syst�mov�ho vol�n� - zak�z�no
	virtual bool Create () { ASSERT ( false ); return false; };

public:
// Operace se syst�mov�m vol�n�m

	// vytvo�en� syst�mov�ho vol�n� s volaj�c�m "pProcess"
	inline bool Create ( CIProcess *pProcess );

	// ukon�en� syst�mov�ho vol�n� ze strany volan�ho
	virtual void FinishWaiting ( EInterpretStatus eStatus ) 
	{
		// informuje proces o ukon�en� syst�mov�ho vol�n�
		InlayEvent ( WAITING_FINISHED, eStatus );
	};

/* D�le�it� zd�d�n� metody

// Operace se syst�mov�m vol�n�m

	// zni�� objekt syst�mov�ho vol�n�
	virtual void Delete ();

	// zru�en� syst�mov�ho vol�n� volaj�c�m
	virtual void Cancel ( CIProcess *pProcess );

// Ukl�d�n� dat (CPersistentObject metody)

	// ukl�d�n� dat
	void PersistentSave ( CPersistentStorage &storage );
	// nahr�v�n� pouze ulo�en�ch dat
	void PersistentLoad ( CPersistentStorage &storage );
	// p�eklad ukazatel�
	void PersistentTranslatePointers ( CPersistentStorage &storage );
	// inicializace nahran�ho objektu
	void PersistentInit ();
*/
};

//////////////////////////////////////////////////////////////////////
// Operace se syst�mov�m vol�n�m
//////////////////////////////////////////////////////////////////////

// vytvo�en� syst�mov�ho vol�n� s volaj�c�m "pProcess"
inline bool CISyscall::Create ( CIProcess *pProcess ) 
{
	// nech� vytvo�it p�edka
	VERIFY ( CIWaitingInterface::Create () );

	// p�ipoj� proces interpretu k syst�mov�mu vol�n�
	AddWaitingProcess ( pProcess );

	return true;
}

#endif //__INTERPRET_SYSCALL__HEADER_INCLUDED__
