/***********************************************************
 * 
 * Projekt: Strategická hra
 *    Èást: Skilly a interpret
 *   Autor: Karel Zikmund
 * 
 *   Popis: Tøída systémového volání na skille
 * 
 ***********************************************************/

#ifndef __INTERPRET_SKILL_SYSCALL__HEADER_INCLUDED__
#define __INTERPRET_SKILL_SYSCALL__HEADER_INCLUDED__

#include "TripleS\Interpret\Src\ISyscall.h"

//////////////////////////////////////////////////////////////////////
// Tøída systémového volání na skille.
class CISkillSysCall : public CISyscall 
{
	DECLARE_DYNAMIC ( CISkillSysCall )

// Metody
protected:
// Konstrukce a destrukce

	// konstruktor
	CISkillSysCall () { m_pUnit = NULL; m_pSkillType = NULL; };
	// destruktor
	virtual ~CISkillSysCall () {};

private:
// Zakázané metody vytváøení objektu

	// vytvoøení systémového volání - zakázáno
	bool Create ( CIProcess *pProcess ) { ASSERT ( false ); return false; };

protected:
// Operace se systémovým voláním

	// vytvoøení systémového volání na skille typu "pSkillType" jednotky "pUnit" 
	//		s volajícím "pProcess" (zvyšuje poèet odkazù na objekt)
	bool Create ( CIProcess *pProcess, CSUnit *pUnit, CSSkillType *pSkillType );
	// znièí objekt systémového volání
	virtual void Delete ();

public:
	// ukonèení systémového volání ze strany volané skilly na jednotce (jednotka musí být 
	//		zamèena pro zápis) (ukazatel na volanou skillu dále není platný)
	virtual void FinishWaiting ( EInterpretStatus eStatus );
protected:
	// zrušení systémového volání volajícím (zámek skilly=zámek jednotky musí být zamèen 
	//		pro zápis) (ukazatel na jednotku dále není platný)
	virtual void Cancel ( CIProcess *pProcess );

// Informace o systémovém volání

	// vrátí ukazatel na volanou jednotku (nevrací NULL)
	CSUnit *GetUnit () { ASSERT ( m_pUnit != NULL ); return m_pUnit; };
	// vrátí ukazatel na volanou skillu (vrací i NULL)
	CSSkillType *GetSkillType () { return m_pSkillType; };

// Ukládání dat (CPersistentObject metody)

	// ukládání dat
	void PersistentSave ( CPersistentStorage &storage );
	// nahrávání pouze uložených dat
	void PersistentLoad ( CPersistentStorage &storage );
	// pøeklad ukazatelù
	void PersistentTranslatePointers ( CPersistentStorage &storage );
	// inicializace nahraného objektu
	void PersistentInit ();

// Data
private:

	// ukazatel na jednotku systémového volání
	CSUnit *m_pUnit;
	// ukazatel na typ skilly systémového volání
	CSSkillType *m_pSkillType;
};

#endif //__INTERPRET_SYSCALL__HEADER_INCLUDED__
