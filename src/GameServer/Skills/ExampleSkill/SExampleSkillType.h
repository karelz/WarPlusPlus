/***********************************************************
 * 
 * Projekt: Strategická hra
 *    Èást: Server hry - skilly
 *   Autor: Karel Zikmund
 * 
 *   Popis: Pøíklad typu skilly
 * 
 ***********************************************************/

#ifndef __SERVER_EXAMPLE_SKILL_TYPE__HEADER_INCLUDED__
#define __SERVER_EXAMPLE_SKILL_TYPE__HEADER_INCLUDED__

#include "GameServer\GameServer\SSkillType.h"

/////////////////////////////////////////////////////////////////////
// Pøíklad typu skilly
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

// Inicializace a znièení dat objektu

	// inicializuje data typu skilly z dat "pData" velikosti "dwDataSize"
	//		výjimky: CPersistentLoadException
	virtual void CreateSkillType ( void *pData, DWORD dwDataSize );
	// znièí data typu skilly
	virtual void DeleteSkillType ();

// Ukládání dat potomkù (analogie CPersistentObject metod)

	// ukládání dat potomka
	virtual void PersistentSaveSkillType ( CPersistentStorage &storage );
	// nahrávání pouze uložených dat potomka
	virtual void PersistentLoadSkillType ( CPersistentStorage &storage );
	// pøeklad ukazatelù potomka
	virtual void PersistentTranslatePointersSkillType ( CPersistentStorage &storage );
	// inicializace nahraného objektu potomka
	virtual void PersistentInitSkillType ();

// Get/Set metody serverových informací o typu skilly jednotky

	// vrátí velikost dat skilly pro jednotku
	virtual DWORD GetSkillDataSize ();
	// vrátí masku skupin skill
	virtual DWORD GetGroupMask ();

// Metody skilly

	// provede TimeSlice na skille jednotky "pUnit"
	// jednotka "pUnit" je již zamèená pro zápis, metoda mùže zamykat libovolný poèet jiných
	//		jednotek pro zápis/ètení (vèetnì jednotek ze svého bloku)
	virtual void DoTimeSlice ( CSUnit *pUnit );

	// inicializuje skillu jednotky "pUnit"
	virtual void InitializeSkill ( CSUnit *pUnit );
	// znièí skillu jednotky "pUnit"
	virtual void DestructSkill ( CSUnit *pUnit );

	// aktivuje skillu jednotky "pUnit" (jednotka musí být zamèena pro zápis)
	void ActivateSkill ( CSUnit *pUnit );
	// deaktivuje skillu jednotky "pUnit" (jednotka musí být zamèena pro zápis)
	virtual void DeactivateSkill ( CSUnit *pUnit );

	// ukládání dat skilly
	virtual void PersistentSaveSkill ( CPersistentStorage &storage, CSUnit *pUnit );
	// nahrávání pouze uložených dat skilly
	virtual void PersistentLoadSkill ( CPersistentStorage &storage, CSUnit *pUnit );
	// pøeklad ukazatelù dat skilly
	virtual void PersistentTranslatePointersSkill ( CPersistentStorage &storage, CSUnit *pUnit );
	// inicializace nahraných dat skilly
	virtual void PersistentInitSkill ( CSUnit *pUnit );

public:
// Interface volání ze skriptù

	// interface volání ze skriptù
	DECLARE_SKILLCALL_INTERFACE_MAP ( CSExampleSkillType, 4 );
	DECLARE_SKILLCALL_METHOD1_WRITE_HARD ( SCI_Start, Int, int * );
	DECLARE_SKILLCALL_METHOD1_WRITE_HARD ( SCI_Stop, Int, int * );

protected:
	// nastartuje skillu (jednotka je zamèena pro zápis)
	ESyscallResult SCI_Start ( CSUnit *pUnit, CIProcess *pProcess, CISyscall **ppSyscall, CIBag *pBag, CIDataStackItem *pReturn, int *pParam );
	// ukonèí skillu (jednotka je zamèena pro zápis)
	ESyscallResult SCI_Stop ( CSUnit *pUnit, CIProcess *pProcess, CISyscall **ppSyscall, CIBag *pBag, CIDataStackItem *pReturn, int *pParam );

// Data
protected:

	// doba èekání mezi výpisy (v TimeSlicech)
	DWORD m_dwWaitCount;
	// poèet použití výpisu
	int m_nOutputUseCount;
	// popis Example skilly
	char m_szDescription[31];
};

#endif //__SERVER_EXAMPLE_SKILL_TYPE__HEADER_INCLUDED__
