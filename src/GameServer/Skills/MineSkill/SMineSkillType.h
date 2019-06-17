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

// Inicializace a zni�en� dat objektu
public:
	// inicializuje data typu skilly z dat "pData" velikosti "dwDataSize"
	//		v�jimky: CPersistentLoadException
	virtual void CreateSkillType ( void *pData, DWORD dwDataSize );
	// inicializuje data typu skilly po vytvo�en� typu skilly
	//		v�jimky: CPersistentLoadException    
	virtual void PostCreateSkillType();
	// zni�� data typu skilly
	virtual void DeleteSkillType ();

// Ukl�d�n� dat potomk� (analogie CPersistentObject metod)
public:
	// ukl�d�n� dat potomka
	virtual void PersistentSaveSkillType ( CPersistentStorage &storage );
	// nahr�v�n� pouze ulo�en�ch dat potomka
	virtual void PersistentLoadSkillType ( CPersistentStorage &storage );
	// p�eklad ukazatel� potomka
	virtual void PersistentTranslatePointersSkillType ( CPersistentStorage &storage );
	// inicializace nahran�ho objektu potomka
	virtual void PersistentInitSkillType ();

// Get/Set metody serverov�ch informac� o typu skilly jednotky
public:
	// vr�t� velikost dat skilly pro jednotku
	virtual DWORD GetSkillDataSize ();
	// vr�t� masku skupin skill
	virtual DWORD GetGroupMask ();

// Metody skilly
public:
	// provede TimeSlice na skille jednotky "pUnit"
	// jednotka "pUnit" je ji� zam�en� pro z�pis, metoda m��e zamykat libovoln� po�et jin�ch
	//		jednotek pro z�pis/�ten� (v�etn� jednotek ze sv�ho bloku)
	virtual void DoTimeSlice ( CSUnit *pUnit );

	// inicializuje skillu jednotky "pUnit"
	virtual void InitializeSkill ( CSUnit *pUnit );
	// zni�� skillu jednotky "pUnit"
	virtual void DestructSkill ( CSUnit *pUnit );

	// Aktivuje skillu na zadane jednotce
	BOOL ActivateSkill(CSUnit *pUnit, CSUnit *pMineUnit, BOOL bUnload, CISysCallMine *pMineSysCall);
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

// Full infa
public:
	// vypln� full info "strInfo" skilly jednotky "pUnit"
	virtual void FillFullInfo ( CSUnit *pUnit, CString &strInfo );
	// vypln� enemy full info "strInfo" skilly jednotky "pUnit"
	virtual void FillEnemyFullInfo ( CSUnit *pUnit, CString &strInfo ) { strInfo=""; };

// Zmena modu
public:
	// Je potreba najit starou mine skillu a nahrat si jeji data
	virtual void ModeChanged ( CSUnit *pUnit, BYTE nOldMode );

// Interface vol�n� ze skript�
public:

	// interface vol�n� ze skript�
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
	// aktivuje skillu pro tezeni (jednotka je zam�eno pro z�pis)
	ESyscallResult SCI_Mine ( CSUnit *pUnit, CIProcess *pProcess, CISyscall **ppSysCall, CIBag *pBag, CIDataStackItem *pReturn, CZUnit *pUnitToMine);

	// aktivuje skillu pro vylozeni (jednotka je zam�eno pro z�pis)
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
