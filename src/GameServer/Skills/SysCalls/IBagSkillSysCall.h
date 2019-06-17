/***********************************************************
 * 
 * Projekt: Strategick� hra
 *    ��st: Interpret
 *   Autor: Karel Zikmund
 * 
 *   Popis: T��da syst�mov�ho vol�n� na skille s parametry
 * 
 ***********************************************************/

#ifndef __INTERPRET_BAG_SKILL_SYSCALL__HEADER_INCLUDED__
#define __INTERPRET_BAG_SKILL_SYSCALL__HEADER_INCLUDED__

#include "ISkillSysCall.h"

//////////////////////////////////////////////////////////////////////
// Dop�edn� deklarace

// t��da bagu parametr�
class CIBag;

//////////////////////////////////////////////////////////////////////
// T��da syst�mov�ho vol�n� na skille s parametry.
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
// Zak�zan� metody vytv��en� objektu

	// vytvo�en� syst�mov�ho vol�n� - zak�z�no
	bool Create ( CIProcess *pProcess, CSUnit *pUnit, CSSkillType *pSkillType ) 
		{ ASSERT ( false ); return false; };

public:
// Operace se syst�mov�m vol�n�m

	// vytvo�en� syst�mov�ho vol�n� na skille typu "pSkillType" jednotky "pUnit" 
	//		s parametry "pBag" a s volaj�c�m "pProcess" (zvy�uje po�et odkaz� na objekt)
	bool Create ( CIProcess *pProcess, CSUnit *pUnit, CSSkillType *pSkillType, 
		CIBag *pBag );

protected:
	// zru�en� syst�mov�ho vol�n� volaj�c�m (z�mek skilly=z�mek jednotky mus� b�t zam�en 
	//		pro z�pis) (ukazatele na jednotku a na bag parametr� d�le nejsou platn�)
	virtual void Cancel ( CIProcess *pProcess );

public:
// Operace s parametry syst�mov�ho vol�n�

	// z�sk� parametry syst�mov�ho vol�n� (nevrac� NULL)
	CIBag *GetBag () { ASSERT ( m_pBag != NULL ); return m_pBag; };

// Ukl�d�n� dat (CPersistentObject metody)
	// ukl�d�n� dat
	void PersistentSave ( CPersistentStorage &storage );
	// nahr�v�n� pouze ulo�en�ch dat
	void PersistentLoad ( CPersistentStorage &storage );
	// p�eklad ukazatel�
	void PersistentTranslatePointers ( CPersistentStorage &storage );
	// inicializace nahran�ho objektu
	void PersistentInit ();

/* D�le�it� zd�d�n� metody

// Operace se syst�mov�m vol�n�m

	// ukon�en� syst�mov�ho vol�n� ze strany volan� skilly na jednotce (jednotka mus� b�t 
	//		zam�ena pro z�pis) (ukazatel na volanou skillu d�le nen� platn�)
	virtual void FinishWaiting ( EInterpretStatus eStatus );
*/

// Data
private:

	// bag parametr� syst�mov�ho vol�n�
	CIBag *m_pBag;
};

#endif //__INTERPRET_BAG_SKILL_SYSCALL__HEADER_INCLUDED__
