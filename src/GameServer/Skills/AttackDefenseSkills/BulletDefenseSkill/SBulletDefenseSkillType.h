/***********************************************************
* 
* Projekt: Strategicka hra
*    Cast: Server hry - skilly
*   Autor: Tomas Karban
* 
*   Popis: Bullet Defense skilla -- deklarace typu skilly
* 
***********************************************************/

#ifndef __SERVER_BULLETDEFENSE_SKILL_TYPE__HEADER_INCLUDED__
#define __SERVER_BULLETDEFENSE_SKILL_TYPE__HEADER_INCLUDED__

#include "..\AbstractDefenseSkill\SAbstractDefenseSkillType.h"
#include "SBulletDefenseSkill.h"

/////////////////////////////////////////////////////////////////////
// Deklarace typu skilly

class CSBulletDefenseSkillType : public CSAbstractDefenseSkillType
{
    DECLARE_DYNAMIC(CSBulletDefenseSkillType);
    DECLARE_SKILL_TYPE(CSBulletDefenseSkillType);
        
// Konstruktor a destruktor
public:
    // konstruktor
    CSBulletDefenseSkillType() :
        m_nLinearCoefficient(1000),
        m_nAbsoluteCoefficient(0)
    { }
    // destruktor
    virtual ~CSBulletDefenseSkillType();
    
    
    // inicializuje data typu skilly z dat "pData" velikosti "dwDataSize"
    //   vyjimky: CPersistentLoadException
    virtual void CreateSkillType(void *pData, DWORD dwDataSize);
    // znici data typu skilly (nuluje objekt)
    virtual void DeleteSkillType();
    
    // save/load dat typu skilly
    
    // ukladani dat potomka
    virtual void PersistentSaveSkillType(CPersistentStorage &storage);
    // nahravani pouze ulozenych dat potomka
    virtual void PersistentLoadSkillType(CPersistentStorage &storage);
    // preklad ukazatelu potomka
    virtual void PersistentTranslatePointersSkillType(CPersistentStorage &storage);
    // inicializace nahraneho objektu potomka
    virtual void PersistentInitSkillType();
    
    // informace o teto skille
    
    // vrati velikost dat skilly pro jednotku
    virtual DWORD GetSkillDataSize() { return sizeof(SSBulletDefenseSkill); }
    
    // obecne operace na skille
    
    // provede time slice na skille jednotky "pUnit"
    // jednotka "pUnit" je jiz zamcena pro zapis, metoda muze zamykat libovolny pocet jinych
    // jednotek pro zapis/cteni (vcetne jednotek ze sveho bloku)
    virtual void DoTimeSlice(CSUnit *pUnit);
    
    // inicializuje skillu jednotky "pUnit"
    virtual void InitializeSkill(CSUnit *pUnit);
    // znici skillu jednotky "pUnit"
    virtual void DestructSkill(CSUnit *pUnit);
    
    // aktivuje skillu jednotky "pUnit" (jednotka musi byt zamcena pro zapis)
    void ActivateSkill(CSUnit *pUnit);
    // deaktivuje skillu jednotky "pUnit" (jednotka musi byt zamcena pro zapis)
    virtual void DeactivateSkill(CSUnit *pUnit);
    
    // save/load dat skilly u jednotky
    
    // ukladani dat skilly
    virtual void PersistentSaveSkill(CPersistentStorage &storage, CSUnit *pUnit);
    // nahravani pouze ulozenych dat skilly
    virtual void PersistentLoadSkill(CPersistentStorage &storage, CSUnit *pUnit);
    // preklad ukazatelu dat skilly
    virtual void PersistentTranslatePointersSkill(CPersistentStorage &storage, CSUnit *pUnit);
    // inicializace nahranych dat skilly
    virtual void PersistentInitSkill(CSUnit *pUnit);
    
    // specificke operace na skille
    
    // vraci masku interakci, ktere je schopna tato defense skilla branit
    virtual DWORD GetDefenseInteractionsMask();
    // dotaz na silu obrany pri zadanem utoku
    virtual int QueryDefense(CSUnit *pUnit, int nAttackIntensity, DWORD dwInteractionMask);
    // provedeni vypoctu obrany na utok zadany v parametrech
    virtual int DoDefense(CSUnit *pAttackedUnit, CSUnit *pAggressor, int nAttackIntensity, DWORD dwInteractionMask);

	// vyplni full info "strInfo" skilly jednotky "pUnit"
	virtual void FillFullInfo(CSUnit *pUnit, CString &strInfo);
    // enemy full info zustane prazdne (tj. nepredefinujeme predka)
    
public:
    // interface volani ze skriptu
    DECLARE_SKILLCALL_INTERFACE_MAP(CSAbstractDefenseSkillType, 4);
    
protected:
    // metody volane pouze ze skriptu
    
protected:
    // data typu skilly
    
    // linearni koeficient vynasobeny 1000
    // (nulova obrana = 1000, absolutni obrana = 0)
    int m_nLinearCoefficient;
    // absolutni koeficient
    int m_nAbsoluteCoefficient;
    
public:
    // debugovaci metody
#ifdef _DEBUG
    virtual void AssertValid() const;
    void AssertValidUnit(SSBulletDefenseSkill *pUnitData) const;
    virtual void Dump(CDumpContext &dc) const;
#endif  // _DEBUG
};

#endif  // __SERVER_BULLETDEFENSE_SKILL_TYPE__HEADER_INCLUDED__
