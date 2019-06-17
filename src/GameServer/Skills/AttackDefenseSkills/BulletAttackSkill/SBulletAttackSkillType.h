/***********************************************************
 * 
 * Projekt: Strategicka hra
 *    Cast: Server hry - skilly
 *   Autor: Tomas Karban
 * 
 *   Popis: Bullet Attack skilla -- deklarace typu skilly
 * 
 ***********************************************************/

#ifndef __SERVER_BULLETATTACK_SKILL_TYPE__HEADER_INCLUDED__
#define __SERVER_BULLETATTACK_SKILL_TYPE__HEADER_INCLUDED__

#include "..\AbstractAttackSkill\SAbstractAttackSkillType.h"
#include "SBulletAttackSkill.h"

/////////////////////////////////////////////////////////////////////
// Deklarace typu skilly

class CSBulletAttackSkillType : public CSAbstractAttackSkillType
{
	DECLARE_DYNAMIC(CSBulletAttackSkillType);
	DECLARE_SKILL_TYPE(CSBulletAttackSkillType);

// Konstruktor a destruktor
public:
    // konstruktor
    CSBulletAttackSkillType() :
        m_dwLoadTime(0),
        m_dwFirePeriod(0),
        m_nAttackIntensity(0),
        m_dwAppearanceID(0),
        m_dwAnimationOffset(0),
        m_dwAnimationLength(0),
        m_dwAnimationEnd(0)
    { }
	// destruktor
	virtual ~CSBulletAttackSkillType();

protected:
    // inicializuje data typu skilly z dat "pData" velikosti "dwDataSize"
	//		vyjimky: CPersistentLoadException
	virtual void CreateSkillType(void *pData, DWORD dwDataSize);
	// znici data typu skilly
	virtual void DeleteSkillType();

    // ukladani dat typu skilly
	virtual void PersistentSaveSkillType(CPersistentStorage &storage);
	// nahravani ulozenych dat typu skilly
	virtual void PersistentLoadSkillType(CPersistentStorage &storage);
	// preklad ukazatelu
	virtual void PersistentTranslatePointersSkillType(CPersistentStorage &storage);
	// inicializace nahraneho objektu
	virtual void PersistentInitSkillType();

	// vrati velikost dat skilly pro jednotku
    virtual DWORD GetSkillDataSize() { return sizeof(SSBulletAttackSkill); }

	// provede time slice na skille jednotky "pUnit"
	// jednotka "pUnit" je jiz zamcena pro zapis, metoda muze zamykat libovolny pocet jinych
	//		jednotek pro zapis/cteni (vcetne jednotek ze sveho bloku)
	virtual void DoTimeSlice(CSUnit *pUnit);

	// inicializuje skillu jednotky "pUnit"
	virtual void InitializeSkill(CSUnit *pUnit);
	// znici skillu jednotky "pUnit"
	virtual void DestructSkill(CSUnit *pUnit);

	// aktivuje skillu jednotky "pUnit" (jednotka musi byt zamcena pro zapis)
	void ActivateSkill(CSUnit *pUnit, CSUnit *pAttackedUnit, CISysCallAttack *pSysCall);
	// deaktivuje skillu jednotky "pUnit" (jednotka musi byt zamcena pro zapis)
	virtual void DeactivateSkill(CSUnit *pUnit);

    // vrati masku interakci, na ktere utoci tato skilla
    virtual DWORD GetAttackInteractionsMask() { return Interaction_Bullet; }

	// ukladani dat skilly
	virtual void PersistentSaveSkill(CPersistentStorage &storage, CSUnit *pUnit);
	// nahravani pouze ulozenych dat skilly
	virtual void PersistentLoadSkill(CPersistentStorage &storage, CSUnit *pUnit);
	// preklad ukazatelu dat skilly
	virtual void PersistentTranslatePointersSkill(CPersistentStorage &storage, CSUnit *pUnit);
	// inicializace nahranych dat skilly
	virtual void PersistentInitSkill(CSUnit *pUnit);

	// vyplni full info "strInfo" skilly jednotky "pUnit"
	virtual void FillFullInfo(CSUnit *pUnit, CString &strInfo);
	// vyplni enemy full info "strInfo" skilly jednotky "pUnit"
	virtual void FillEnemyFullInfo(CSUnit *pUnit, CString &strInfo);

public:
	// zjistí, mùže-li skillu volat nepøítel
	virtual BOOL CanBeCalledByEnemy () { return TRUE; };

// Interface volani ze skriptu
public:
	DECLARE_SKILLCALL_INTERFACE_MAP(CSBulletAttackSkillType, 9);
    DECLARE_SKILLCALL_METHOD1_WRITE_HARD(SCI_Fire, Unit, CZUnit *);

// Metody skilly volane ze skriptu
protected:
    ESyscallResult SCI_Fire(CSUnit *pUnit, CIProcess *pProcess, CISyscall **ppSysCall, CIBag *pBag, CIDataStackItem *pReturn, CZUnit *pAttackedUnit);

// Data typu skilly
protected:
    // doba pred prvnim vystrelem ("delka nabijeni")
    DWORD m_dwLoadTime;
    
    // doba mezi dvema vystrely 
    // (0 = strelba kazdy timeslice, n = pauza n timeslicu mezi dvema vystrely)
    DWORD m_dwFirePeriod;

    // sila jednoho vystrelu
    int m_nAttackIntensity;

    // ID vzhledu, ktery ma jednotka, kdyz strili
    DWORD m_dwAppearanceID;

    // offset zacatku animace (o kolik timeslicu zacne animace drive
    // pred samotnym vystrelem)
    DWORD m_dwAnimationOffset;

    // delka animace v timeslicech
    DWORD m_dwAnimationLength;
    
    // konec animace (porovnava se s m_dwRemainingTime u jednotky)
    DWORD m_dwAnimationEnd;

public:
    // debugovaci metody
#ifdef _DEBUG
    virtual void AssertValid() const;
    void AssertValidUnit(SSBulletAttackSkill *pUnitData) const;
    virtual void Dump(CDumpContext &dc) const;
#endif  // _DEBUG

};

#endif  // __SERVER_BULLETATTACK_SKILL_TYPE__HEADER_INCLUDED__
