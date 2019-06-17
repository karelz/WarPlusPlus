/***********************************************************
 * 
 * Projekt: Strategická hra
 *    Èást: Interpret
 *   Autor: Karel Zikmund
 * 
 *   Popis: Tøída systémového volání na skille s parametry
 * 
 ***********************************************************/

#ifndef __INTERPRET_BAG_SKILL_SYSCALL__HEADER_INCLUDED__
#define __INTERPRET_BAG_SKILL_SYSCALL__HEADER_INCLUDED__

#include "ISkillSysCall.h"

//////////////////////////////////////////////////////////////////////
// Dopøedné deklarace

// tøída bagu parametrù
class CIBag;

//////////////////////////////////////////////////////////////////////
// Tøída systémového volání na skille s parametry.
class CIBagSkillSysCall : public CISkillSysCall 
{
	DECLARE_DYNAMIC ( CIBagSkillSysCall )

// Metody
protected:
// Konstrukce a destrukce

	// kontruktor
	CIBagSkillSysCall () { m_pBag = NULL; };
	// destruktor
	virtual ~CIBagSkillSysCall () {};

private:
// Zakázané metody vytváøení objektu

	// vytvoøení systémového volání - zakázáno
	bool Create ( CIProcess *pProcess, CSUnit *pUnit, CSSkillType *pSkillType ) 
		{ ASSERT ( false ); return false; };

public:
// Operace se systémovým voláním

	// vytvoøení systémového volání na skille typu "pSkillType" jednotky "pUnit" 
	//		s parametry "pBag" a s volajícím "pProcess" (zvyšuje poèet odkazù na objekt)
	bool Create ( CIProcess *pProcess, CSUnit *pUnit, CSSkillType *pSkillType, 
		CIBag *pBag );

protected:
	// zrušení systémového volání volajícím (zámek skilly=zámek jednotky musí být zamèen 
	//		pro zápis) (ukazatele na jednotku a na bag parametrù dále nejsou platné)
	virtual void Cancel ( CIProcess *pProcess );

public:
// Operace s parametry systémového volání

	// získá parametry systémového volání (nevrací NULL)
	CIBag *GetBag () { ASSERT ( m_pBag != NULL ); return m_pBag; };

// Ukládání dat (CPersistentObject metody)
	// ukládání dat
	void PersistentSave ( CPersistentStorage &storage );
	// nahrávání pouze uložených dat
	void PersistentLoad ( CPersistentStorage &storage );
	// pøeklad ukazatelù
	void PersistentTranslatePointers ( CPersistentStorage &storage );
	// inicializace nahraného objektu
	void PersistentInit ();

/* Dùležité zdìdìné metody

// Operace se systémovým voláním

	// ukonèení systémového volání ze strany volané skilly na jednotce (jednotka musí být 
	//		zamèena pro zápis) (ukazatel na volanou skillu dále není platný)
	virtual void FinishWaiting ( EInterpretStatus eStatus );
*/

// Data
private:

	// bag parametrù systémového volání
	CIBag *m_pBag;
};

#endif //__INTERPRET_BAG_SKILL_SYSCALL__HEADER_INCLUDED__
