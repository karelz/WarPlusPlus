// SMineSkillType.h: interface for the CSMineSkillType class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SMINESKILLTYPE_H__74FE6113_A282_11D4_8112_0000B48431EC__INCLUDED_)
#define AFX_SMINESKILLTYPE_H__74FE6113_A282_11D4_8112_0000B48431EC__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "GameServer\GameServer\SSkillType.h"
#include "GameServer\GameServer\SResources.h"
#include "Common\Map\MMineSkillType.h"
#include "SMineSkill.h"

class CSMineSkillType : public CSSkillType
{
	DECLARE_DYNAMIC(CSMineSkillType);
	// deklarace typu skilly
	DECLARE_SKILL_TYPE ( CSMineSkillType )

// Metody
public:

// Konstrukce a destrukce
public:
	// Konstruktor
	CSMineSkillType();
	// Destruktor
	virtual ~CSMineSkillType();

// Inicializace a znièení dat objektu
public:
	// inicializuje data typu skilly z dat "pData" velikosti "dwDataSize"
	//		výjimky: CPersistentLoadException
	virtual void CreateSkillType ( void *pData, DWORD dwDataSize );
	// inicializuje data typu skilly po vytvoøení typu skilly
	//		výjimky: CPersistentLoadException    
	virtual void PostCreateSkillType();
	// znièí data typu skilly
	virtual void DeleteSkillType ();

// Ukládání dat potomkù (analogie CPersistentObject metod)
public:
	// ukládání dat potomka
	virtual void PersistentSaveSkillType ( CPersistentStorage &storage );
	// nahrávání pouze uložených dat potomka
	virtual void PersistentLoadSkillType ( CPersistentStorage &storage );
	// pøeklad ukazatelù potomka
	virtual void PersistentTranslatePointersSkillType ( CPersistentStorage &storage );
	// inicializace nahraného objektu potomka
	virtual void PersistentInitSkillType ();

// Get/Set metody serverových informací o typu skilly jednotky
public:
	// vrátí velikost dat skilly pro jednotku
	virtual DWORD GetSkillDataSize ();
	// vrátí masku skupin skill
	virtual DWORD GetGroupMask ();

// Metody skilly
public:
	// provede TimeSlice na skille jednotky "pUnit"
	// jednotka "pUnit" je již zamèená pro zápis, metoda mùže zamykat libovolný poèet jiných
	//		jednotek pro zápis/ètení (vèetnì jednotek ze svého bloku)
	virtual void DoTimeSlice ( CSUnit *pUnit );

	// inicializuje skillu jednotky "pUnit"
	virtual void InitializeSkill ( CSUnit *pUnit );
	// znièí skillu jednotky "pUnit"
	virtual void DestructSkill ( CSUnit *pUnit );

	// Aktivuje skillu na zadane jednotce
	BOOL ActivateSkill(CSUnit *pUnit, CSUnit *pMineUnit, BOOL bUnload, CISysCallMine *pMineSysCall);
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

// Full infa
public:
	// vyplní full info "strInfo" skilly jednotky "pUnit"
	virtual void FillFullInfo ( CSUnit *pUnit, CString &strInfo );
	// vyplní enemy full info "strInfo" skilly jednotky "pUnit"
	virtual void FillEnemyFullInfo ( CSUnit *pUnit, CString &strInfo ) { strInfo=""; };

// Zmena modu
public:
	// Je potreba najit starou mine skillu a nahrat si jeji data
	virtual void ModeChanged ( CSUnit *pUnit, BYTE nOldMode );

// Interface volání ze skriptù
public:

	// interface volání ze skriptù
	DECLARE_SKILLCALL_INTERFACE_MAP ( CSMineSkillType, 9 );
	DECLARE_SKILLCALL_METHOD1_WRITE_HARD ( SCI_Mine, Unit, CZUnit*);
	DECLARE_SKILLCALL_METHOD1_WRITE_HARD ( SCI_Unload, Unit, CZUnit*);
	DECLARE_SKILLCALL_METHOD0_READ_SOFT ( SCI_GetResources );
	DECLARE_SKILLCALL_METHOD1_WRITE_HARD ( SCI_SetMode, Int, int*);
	DECLARE_SKILLCALL_METHOD0_READ_SOFT ( SCI_GetCapacity );
	DECLARE_SKILLCALL_METHOD0_READ_SOFT ( SCI_IsEmpty );
	DECLARE_SKILLCALL_METHOD0_READ_SOFT ( SCI_IsFull );

// Deklarace funkci pro volani ze skriptu
protected:
	// aktivuje skillu pro tezeni (jednotka je zamèeno pro zápis)
	ESyscallResult SCI_Mine ( CSUnit *pUnit, CIProcess *pProcess, CISyscall **ppSysCall, CIBag *pBag, CIDataStackItem *pReturn, CZUnit *pUnitToMine);

	// aktivuje skillu pro vylozeni (jednotka je zamèeno pro zápis)
	ESyscallResult SCI_Unload ( CSUnit *pUnit, CIProcess *pProcess, CISyscall **ppSysCall, CIBag *pBag, CIDataStackItem *pReturn, CZUnit *pUnitForUnload);

	// Zjisti, kolik resourcu je momentalne natezenych
	ESyscallResult SCI_GetResources (CSUnit *pUnit, CIProcess *pProcess, CISyscall **ppSysCall, CIBag *pBag, CIDataStackItem *pReturn);

	// Zjisti, kolik resourcu je momentalne natezenych
	ESyscallResult SCI_SetMode (CSUnit *pUnit, CIProcess *pProcess, CISyscall **ppSysCall, CIBag *pBag, CIDataStackItem *pReturn, int* pMode);

	// Zjisti, kolik resourcu muzeme maximalne natezit
	ESyscallResult SCI_GetCapacity (CSUnit *pUnit, CIProcess *pProcess, CISyscall **ppSysCall, CIBag *pBag, CIDataStackItem *pReturn);
	
	// Je jednotka uplne prazdna? (0 resourcu)
	ESyscallResult SCI_IsEmpty (CSUnit *pUnit, CIProcess *pProcess, CISyscall **ppSysCall, CIBag *pBag, CIDataStackItem *pReturn);
	
	// Je jednotka uplne plna? (resourcy==m_StorageSize)
	ESyscallResult SCI_IsFull (CSUnit *pUnit, CIProcess *pProcess, CISyscall **ppSysCall, CIBag *pBag, CIDataStackItem *pReturn);

// Pomocne metody
	
	// Ukonci skillu
	void Finish ( CSUnit *pUnit );	

// Debugovani
#ifdef _DEBUG
private:
    // Kontrola, ze data jednotky jsou vynulovana
    BOOL CheckEmptySkillData ( CSUnit *pUnit );

#endif

// Data
protected:
	DWORD m_dwAppearanceBeforeMining, m_dwAppearanceWhileMining, m_dwAppearanceAfterMining;
	DWORD m_dwAppearanceBeforeUnload, m_dwAppearanceWhileUnload, m_dwAppearanceAfterUnload;

	DWORD m_dwAppearanceBeforeMiningFull, m_dwAppearanceWhileMiningFull, m_dwAppearanceAfterUnloadFull;

	DWORD m_dwMaxMineDistance, m_dwMaxUnloadDistance;

	DWORD m_dwTimeslicesBeforeMining, m_dwTimeslicesAfterMining;
	DWORD m_dwTimeslicesBeforeUnload, m_dwTimeslicesAfterUnload;

	DWORD m_dwTimeslicesBeforeMiningFull, m_dwTimeslicesAfterUnloadFull;

	CSResources m_StorageSize, m_LoadPerTimeslice, m_UnloadPerTimeslice;

	// Mod, ve kterem je jednotka po naplneni
	int m_nModeFull;
};

#endif // !defined(AFX_SMINESKILLTYPE_H__74FE6113_A282_11D4_8112_0000B48431EC__INCLUDED_)
