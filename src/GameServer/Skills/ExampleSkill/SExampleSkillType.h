/***********************************************************
 * 
 * Projekt: Strategick� hra
 *    ��st: Server hry - skilly
 *   Autor: Karel Zikmund
 * 
 *   Popis: P��klad typu skilly
 * 
 ***********************************************************/

#ifndef __SERVER_EXAMPLE_SKILL_TYPE__HEADER_INCLUDED__
#define __SERVER_EXAMPLE_SKILL_TYPE__HEADER_INCLUDED__

#include "GameServer\GameServer\SSkillType.h"

/////////////////////////////////////////////////////////////////////
// P��klad typu skilly
class CSExampleSkillType : public CSSkillType 
{
	// deklarace typu skilly
	DECLARE_SKILL_TYPE ( CSExampleSkillType )

// Metody
public:
// Konstrukce a destrukce

	// konstruktor
	CSExampleSkillType ();
	// destruktor
	virtual ~CSExampleSkillType ();

// Inicializace a zni�en� dat objektu

	// inicializuje data typu skilly z dat "pData" velikosti "dwDataSize"
	//		v�jimky: CPersistentLoadException
	virtual void CreateSkillType ( void *pData, DWORD dwDataSize );
	// zni�� data typu skilly
	virtual void DeleteSkillType ();

// Ukl�d�n� dat potomk� (analogie CPersistentObject metod)

	// ukl�d�n� dat potomka
	virtual void PersistentSaveSkillType ( CPersistentStorage &storage );
	// nahr�v�n� pouze ulo�en�ch dat potomka
	virtual void PersistentLoadSkillType ( CPersistentStorage &storage );
	// p�eklad ukazatel� potomka
	virtual void PersistentTranslatePointersSkillType ( CPersistentStorage &storage );
	// inicializace nahran�ho objektu potomka
	virtual void PersistentInitSkillType ();

// Get/Set metody serverov�ch informac� o typu skilly jednotky

	// vr�t� velikost dat skilly pro jednotku
	virtual DWORD GetSkillDataSize ();
	// vr�t� masku skupin skill
	virtual DWORD GetGroupMask ();

// Metody skilly

	// provede TimeSlice na skille jednotky "pUnit"
	// jednotka "pUnit" je ji� zam�en� pro z�pis, metoda m��e zamykat libovoln� po�et jin�ch
	//		jednotek pro z�pis/�ten� (v�etn� jednotek ze sv�ho bloku)
	virtual void DoTimeSlice ( CSUnit *pUnit );

	// inicializuje skillu jednotky "pUnit"
	virtual void InitializeSkill ( CSUnit *pUnit );
	// zni�� skillu jednotky "pUnit"
	virtual void DestructSkill ( CSUnit *pUnit );

	// aktivuje skillu jednotky "pUnit" (jednotka mus� b�t zam�ena pro z�pis)
	void ActivateSkill ( CSUnit *pUnit );
	// deaktivuje skillu jednotky "pUnit" (jednotka mus� b�t zam�ena pro z�pis)
	virtual void DeactivateSkill ( CSUnit *pUnit );

	// ukl�d�n� dat skilly
	virtual void PersistentSaveSkill ( CPersistentStorage &storage, CSUnit *pUnit );
	// nahr�v�n� pouze ulo�en�ch dat skilly
	virtual void PersistentLoadSkill ( CPersistentStorage &storage, CSUnit *pUnit );
	// p�eklad ukazatel� dat skilly
	virtual void PersistentTranslatePointersSkill ( CPersistentStorage &storage, CSUnit *pUnit );
	// inicializace nahran�ch dat skilly
	virtual void PersistentInitSkill ( CSUnit *pUnit );

public:
// Interface vol�n� ze skript�

	// interface vol�n� ze skript�
	DECLARE_SKILLCALL_INTERFACE_MAP ( CSExampleSkillType, 4 );
	DECLARE_SKILLCALL_METHOD1_WRITE_HARD ( SCI_Start, Int, int * );
	DECLARE_SKILLCALL_METHOD1_WRITE_HARD ( SCI_Stop, Int, int * );

protected:
	// nastartuje skillu (jednotka je zam�ena pro z�pis)
	ESyscallResult SCI_Start ( CSUnit *pUnit, CIProcess *pProcess, CISyscall **ppSyscall, CIBag *pBag, CIDataStackItem *pReturn, int *pParam );
	// ukon�� skillu (jednotka je zam�ena pro z�pis)
	ESyscallResult SCI_Stop ( CSUnit *pUnit, CIProcess *pProcess, CISyscall **ppSyscall, CIBag *pBag, CIDataStackItem *pReturn, int *pParam );

// Data
protected:

	// doba �ek�n� mezi v�pisy (v TimeSlicech)
	DWORD m_dwWaitCount;
	// po�et pou�it� v�pisu
	int m_nOutputUseCount;
	// popis Example skilly
	char m_szDescription[31];
};

#endif //__SERVER_EXAMPLE_SKILL_TYPE__HEADER_INCLUDED__
