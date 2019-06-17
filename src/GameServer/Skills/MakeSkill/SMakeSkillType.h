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

	// aktivuje skillu jednotky "pUnit" (jednotka mus� b�t zam�ena pro z�pis)
	// Pokud nebylo dost resourcu, vrati FALSE
	BOOL ActivateSkill(CSUnit *pUnit, SSMakeSkillRecord *pMakeSkillRecord, CISOString *pScriptName, CZUnit *pCommander, CISysCallMake *pSysCall);
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
	// Vyplni full info
	virtual void FillFullInfo ( CSUnit *pUnit, CString &strInfo );

// Interface vol�n� ze skript�
public:

	// interface vol�n� ze skript�
	DECLARE_SKILLCALL_INTERFACE_MAP ( CSMakeSkillType, 4 );
	DECLARE_SKILLCALL_METHOD3_WRITE_HARD ( SCI_Make, SystemObject, CISystemObject*, SystemObject, CISystemObject*, Unit, CZUnit*);
    DECLARE_SKILLCALL_METHOD1_READ_SOFT ( SCI_GetCost, SystemObject, CISystemObject*);

// Deklarace funkci pro volani ze skriptu
protected:
	// aktivuje skillu pohybu (jednotka je zam�eno pro z�pis)
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
