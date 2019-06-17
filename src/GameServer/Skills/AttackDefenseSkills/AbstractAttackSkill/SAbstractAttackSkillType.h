/***********************************************************
 * 
 * Projekt: Strategicka hra
 *    Cast: Server hry - skilly
 *   Autor: Tomas Karban
 * 
 *   Popis: Abstraktni predek vsech attack skill -- deklarace typu skilly
 * 
 ***********************************************************/

#ifndef __SERVER_ABSTRACTATTACK_SKILL_TYPE__HEADER_INCLUDED__
#define __SERVER_ABSTRACTATTACK_SKILL_TYPE__HEADER_INCLUDED__

#include "..\AttackDefenseInteractions.h"
#include "SAbstractAttackSkill.h"

/////////////////////////////////////////////////////////////////////
// Abstraktni predek vsech attack skills

class CSAbstractAttackSkillType : public CSSkillType
{
    DECLARE_DYNAMIC(CSAbstractAttackSkillType);

    // tridy CSAttackableUnitsDPC a CSCanAttackUnitDPC jsou kamaradi,
    // protoze potrebuji zjistovat informace z typu skilly
    friend class CSAttackableUnitsDPC;
    friend class CSCanAttackUnitDPC;
    friend class CISysCallAttack;

// Metody

// Konstrukce a destrukce
protected:
    // konstruktor
    //     ten je protected, protoze nechceme vytvaret instance teto tridy,
    //     vzdy jen instance nektereho z potomku
    CSAbstractAttackSkillType() :
        m_dwAttackRadius(0),
        m_dwAttackRadiusSquare(0),
        m_dwAttackMinAltitude(0),
        m_dwAttackMaxAltitude(0)
    { };
    // destruktor
    virtual ~CSAbstractAttackSkillType();

// Inicializace a zniceni typu skilly pri vytvoreni a zniceni hry (ne pri loadu)

    // inicializuje data typu skilly z dat "pData" velikosti "dwDataSize"
    //      vyjimky: CPersistentLoadException
    virtual void CreateSkillType(void *pData, DWORD dwDataSize);
    // znici data typu skilly
    virtual void DeleteSkillType();

// Ukladani dat typu skilly

    // ukladani dat potomka
    virtual void PersistentSaveSkillType(CPersistentStorage &storage);
    // nahravani pouze ulozenych dat potomka
    virtual void PersistentLoadSkillType(CPersistentStorage &storage);
    // preklad ukazatelu potomka
    virtual void PersistentTranslatePointersSkillType(CPersistentStorage &storage);
    // inicializace nahraneho objektu potomka
    virtual void PersistentInitSkillType();

// Get/Set metody serverovych informaci o typu skilly jednotky

    // vrati velikost dat skilly pro jednotku
    virtual DWORD GetSkillDataSize() = 0;
    // vrati masku skupin skill
    virtual DWORD GetGroupMask() { return Group_Attack; }

// Metody skilly

    // provede time slice na skille jednotky "pUnit"
    // jednotka "pUnit" je jiz zamcena pro zapis, metoda muze zamykat libovolny pocet jinych
    //      jednotek pro zapis/cteni (vcetne jednotek ze sveho bloku)
    virtual void DoTimeSlice(CSUnit *pUnit) = 0;

    // inicializuje skillu jednotky "pUnit"
    virtual void InitializeSkill(CSUnit *pUnit);
    // znici skillu jednotky "pUnit"
    virtual void DestructSkill(CSUnit *pUnit);

    // aktivuje skillu jednotky "pUnit" (jednotka musi byt zamcena pro zapis)
    void ActivateSkill(CSUnit *pUnit, CISysCallAttack *pSysCall);
    // deaktivuje skillu jednotky "pUnit" (jednotka musi byt zamcena pro zapis)
    virtual void DeactivateSkill(CSUnit *pUnit) { CSSkillType::DeactivateSkill(pUnit); }

    // notifikuje napadenou jednotku
    void NotifyAttackedUnit(CSUnit *pAggressor, CSUnit *pAttackedUnit, int nAttackIntensity);
    // vrati vsechny interakce, kterymi tato attack skilla utoci
    virtual DWORD GetAttackInteractionsMask() = 0;
    
// Ukladani dat skilly u jednotky
    
    // ukladani dat skilly
    virtual void PersistentSaveSkill(CPersistentStorage &storage, CSUnit *pUnit) = 0;
    // nahravani pouze ulozenych dat skilly
    virtual void PersistentLoadSkill(CPersistentStorage &storage, CSUnit *pUnit) = 0;
    // preklad ukazatelu dat skilly
    virtual void PersistentTranslatePointersSkill(CPersistentStorage &storage, CSUnit *pUnit) = 0;
    // inicializace nahranych dat skilly
    virtual void PersistentInitSkill(CSUnit *pUnit) = 0;

// Interface volani ze skriptu
public:
    // interface volani ze skriptu
    DECLARE_SKILLCALL_INTERFACE_MAP(CSAbstractAttackSkillType, 8);
    DECLARE_SKILLCALL_METHOD0_READ_SOFT(SCI_GetAttackableUnits);
    DECLARE_SKILLCALL_METHOD1_READ_SOFT(SCI_CanAttackUnit, Unit, CZUnit *);
    DECLARE_SKILLCALL_METHOD0_READ_SOFT(SCI_GetAttackInteractionsMask);
    DECLARE_SKILLCALL_METHOD0_READ_SOFT(SCI_GetAttackRadius);
    DECLARE_SKILLCALL_METHOD0_READ_SOFT(SCI_GetAttackAltMin);
    DECLARE_SKILLCALL_METHOD0_READ_SOFT(SCI_GetAttackAltMax);

// metody volane ze skriptu
protected:
    // aktivuje metodu GetAttackableUnits
    ESyscallResult SCI_GetAttackableUnits(CSUnit *pUnit, CIProcess *pProcess, CISyscall **ppSysCall, CIBag *pBag, CIDataStackItem *pReturn);
    ESyscallResult SCI_CanAttackUnit(CSUnit *pUnit, CIProcess *pProcess, CISyscall **ppSysCall, CIBag *pBag, CIDataStackItem *pReturn, CZUnit *pAttackableUnit);
    ESyscallResult SCI_GetAttackInteractionsMask(CSUnit *pUnit, CIProcess *pProcess, CISyscall **ppSysCall, CIBag *pBag, CIDataStackItem *pReturn);
    ESyscallResult SCI_GetAttackRadius(CSUnit *pUnit, CIProcess *pProcess, CISyscall **ppSysCall, CIBag *pBag, CIDataStackItem *pReturn);
    ESyscallResult SCI_GetAttackAltMin(CSUnit *pUnit, CIProcess *pProcess, CISyscall **ppSysCall, CIBag *pBag, CIDataStackItem *pReturn);
    ESyscallResult SCI_GetAttackAltMax(CSUnit *pUnit, CIProcess *pProcess, CISyscall **ppSysCall, CIBag *pBag, CIDataStackItem *pReturn);

// data typu skilly
protected:
    // akcni polomer jednotky
    DWORD m_dwAttackRadius;
    DWORD m_dwAttackRadiusSquare;

    // vyskovy interval, kam muze dany attack utocit
    DWORD m_dwAttackMinAltitude;
    DWORD m_dwAttackMaxAltitude;

public:
    // debugovaci metody
#ifdef _DEBUG
    virtual void AssertValid() const { }
    void AssertValidUnit(SSAbstractAttackSkill *pUnitData) const;
    virtual void Dump(CDumpContext &dc) const { }
#endif  // _DEBUG

};

#endif  // __SERVER_ABSTRACTATTACK_SKILL_TYPE__HEADER_INCLUDED__
