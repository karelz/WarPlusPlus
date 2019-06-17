/***********************************************************
 * 
 * Projekt: Strategická hra
 *    Èást: Interpret
 *   Autor: Helena Kupková & Karel Zikmund
 * 
 *   Popis: Tøída systémového volání
 * 
 ***********************************************************/

#ifndef __INTERPRET_SYSCALL__HEADER_INCLUDED__
#define __INTERPRET_SYSCALL__HEADER_INCLUDED__

#include "IWaitingInterface.h"

//////////////////////////////////////////////////////////////////////
// Tøída systémového volání.
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
// Zakázané metody vytváøení objektu

	// vytvoøení systémového volání - zakázáno
	virtual bool Create () { ASSERT ( false ); return false; };

public:
// Operace se systémovým voláním

	// vytvoøení systémového volání s volajícím "pProcess"
	inline bool Create ( CIProcess *pProcess );

	// ukonèení systémového volání ze strany volaného
	virtual void FinishWaiting ( EInterpretStatus eStatus ) 
	{
		// informuje proces o ukonèení systémového volání
		InlayEvent ( WAITING_FINISHED, eStatus );
	};

/* Dùležité zdìdìné metody

// Operace se systémovým voláním

	// znièí objekt systémového volání
	virtual void Delete ();

	// zrušení systémového volání volajícím
	virtual void Cancel ( CIProcess *pProcess );

// Ukládání dat (CPersistentObject metody)

	// ukládání dat
	void PersistentSave ( CPersistentStorage &storage );
	// nahrávání pouze uložených dat
	void PersistentLoad ( CPersistentStorage &storage );
	// pøeklad ukazatelù
	void PersistentTranslatePointers ( CPersistentStorage &storage );
	// inicializace nahraného objektu
	void PersistentInit ();
*/
};

//////////////////////////////////////////////////////////////////////
// Operace se systémovým voláním
//////////////////////////////////////////////////////////////////////

// vytvoøení systémového volání s volajícím "pProcess"
inline bool CISyscall::Create ( CIProcess *pProcess ) 
{
	// nechá vytvoøit pøedka
	VERIFY ( CIWaitingInterface::Create () );

	// pøipojí proces interpretu k systémovému volání
	AddWaitingProcess ( pProcess );

	return true;
}

#endif //__INTERPRET_SYSCALL__HEADER_INCLUDED__
