// SMakeSkillType.h: interface for the CSMakeSkillType class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SMAKESKILLTYPE_H__791E9AF3_7DDB_11D4_80D5_0000B4A08F9A__INCLUDED_)
#define AFX_SMAKESKILLTYPE_H__791E9AF3_7DDB_11D4_80D5_0000B4A08F9A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "GameServer\GameServer\SSkillType.h"
#include "Common\Map\MMakeSkillType.h"
#include "SMakeSkill.h"

class CISysCallMake;

class CSMakeSkillType : public CSSkillType  
{
	// deklarace typu skilly
	DECLARE_SKILL_TYPE ( CSMakeSkillType )

// Metody
public:

// Konstrukce a destrukce
public:
    // Konstruktor
	CSMakeSkillType();
    // Destruktor
	virtual ~CSMakeSkillType();

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

	// aktivuje skillu jednotky "pUnit" (jednotka musí být zamèena pro zápis)
	// Pokud nebylo dost resourcu, vrati FALSE
	BOOL ActivateSkill(CSUnit *pUnit, SSMakeSkillRecord *pMakeSkillRecord, CISOString *pScriptName, CZUnit *pCommander, CISysCallMake *pSysCall);
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
	// Vyplni full info
	virtual void FillFullInfo ( CSUnit *pUnit, CString &strInfo );

// Interface volání ze skriptù
public:

	// interface volání ze skriptù
	DECLARE_SKILLCALL_INTERFACE_MAP ( CSMakeSkillType, 4 );
	DECLARE_SKILLCALL_METHOD3_WRITE_HARD ( SCI_Make, SystemObject, CISystemObject*, SystemObject, CISystemObject*, Unit, CZUnit*);
    DECLARE_SKILLCALL_METHOD1_READ_SOFT ( SCI_GetCost, SystemObject, CISystemObject*);

// Deklarace funkci pro volani ze skriptu
protected:
	// aktivuje skillu pohybu (jednotka je zamèeno pro zápis)
	ESyscallResult SCI_Make ( CSUnit *pUnit, CIProcess *pProcess, CISyscall **ppSysCall, CIBag *pBag, CIDataStackItem *pReturn, CISystemObject *pUnitTypeName, CISystemObject *pScriptName, CZUnit *pCommander);

    // Zjisti, kolik resourcu je potreba na vybudovani jednotky daneho typu
    ESyscallResult SCI_GetCost (CSUnit *pUnit, CIProcess *pProcess, CISyscall **ppSysCall, CIBag *pBag, CIDataStackItem *pReturn, CISystemObject *pUnitTypeName);

// Pomocne metody
private:
    // Najde zaznam pro zadane jmeno unit typu
    SSMakeSkillRecord *FindRecord (CSUnit *pUnit, CString strUnitType);

    // Ukonci skilu na jednotce, vraci nove udelanou jednotku
    void Finish ( CSUnit *pUnit, CSUnit *pRet);

// Debugovani
#ifdef _DEBUG
private:
    // Kontrola, ze data jednotky jsou vynulovana
    BOOL CheckEmptySkillData ( CSUnit *pUnit );

#endif

// Data
protected:
    // Pocet zaznamu
    int m_nRecords;

    // Vlastni zaznamy
    SSMakeSkillRecord *m_pRecords;
};

#endif // !defined(AFX_SMAKESKILLTYPE_H__791E9AF3_7DDB_11D4_80D5_0000B4A08F9A__INCLUDED_)
