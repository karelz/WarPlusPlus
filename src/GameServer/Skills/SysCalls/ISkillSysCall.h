/***********************************************************
 * 
 * Projekt: Strategick� hra
 *    ��st: Skilly a interpret
 *   Autor: Karel Zikmund
 * 
 *   Popis: T��da syst�mov�ho vol�n� na skille
 * 
 ***********************************************************/

#ifndef __INTERPRET_SKILL_SYSCALL__HEADER_INCLUDED__
#define __INTERPRET_SKILL_SYSCALL__HEADER_INCLUDED__

#include "TripleS\Interpret\Src\ISyscall.h"

//////////////////////////////////////////////////////////////////////
// T��da syst�mov�ho vol�n� na skille.
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
// Zak�zan� metody vytv��en� objektu

	// vytvo�en� syst�mov�ho vol�n� - zak�z�no
	bool Create ( CIProcess *pProcess ) { ASSERT ( false ); return false; };

protected:
// Operace se syst�mov�m vol�n�m

	// vytvo�en� syst�mov�ho vol�n� na skille typu "pSkillType" jednotky "pUnit" 
	//		s volaj�c�m "pProcess" (zvy�uje po�et odkaz� na objekt)
	bool Create ( CIProcess *pProcess, CSUnit *pUnit, CSSkillType *pSkillType );
	// zni�� objekt syst�mov�ho vol�n�
	virtual void Delete ();

public:
	// ukon�en� syst�mov�ho vol�n� ze strany volan� skilly na jednotce (jednotka mus� b�t 
	//		zam�ena pro z�pis) (ukazatel na volanou skillu d�le nen� platn�)
	virtual void FinishWaiting ( EInterpretStatus eStatus );
protected:
	// zru�en� syst�mov�ho vol�n� volaj�c�m (z�mek skilly=z�mek jednotky mus� b�t zam�en 
	//		pro z�pis) (ukazatel na jednotku d�le nen� platn�)
	virtual void Cancel ( CIProcess *pProcess );

// Informace o syst�mov�m vol�n�

	// vr�t� ukazatel na volanou jednotku (nevrac� NULL)
	CSUnit *GetUnit () { ASSERT ( m_pUnit != NULL ); return m_pUnit; };
	// vr�t� ukazatel na volanou skillu (vrac� i NULL)
	CSSkillType *GetSkillType () { return m_pSkillType; };

// Ukl�d�n� dat (CPersistentObject metody)

	// ukl�d�n� dat
	void PersistentSave ( CPersistentStorage &storage );
	// nahr�v�n� pouze ulo�en�ch dat
	void PersistentLoad ( CPersistentStorage &storage );
	// p�eklad ukazatel�
	void PersistentTranslatePointers ( CPersistentStorage &storage );
	// inicializace nahran�ho objektu
	void PersistentInit ();

// Data
private:

	// ukazatel na jednotku syst�mov�ho vol�n�
	CSUnit *m_pUnit;
	// ukazatel na typ skilly syst�mov�ho vol�n�
	CSSkillType *m_pSkillType;
};

#endif //__INTERPRET_SYSCALL__HEADER_INCLUDED__
