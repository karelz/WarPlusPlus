/***********************************************************
 * 
 * Projekt: Strategicka hra
 *    Cast: Server hry - skilly
 *   Autor: Tomas Karban
 * 
 *   Popis: Abstraktni predek vsech defense skill -- deklarace typu skilly
 * 
 ***********************************************************/

#ifndef __SERVER_ABSTRACTDEFENSE_SKILL_TYPE__HEADER_INCLUDED__
#define __SERVER_ABSTRACTDEFENSE_SKILL_TYPE__HEADER_INCLUDED__

#include "..\AttackDefenseInteractions.h"

/////////////////////////////////////////////////////////////////////
// Abstraktni predek vsech defense skill

class CSAbstractDefenseSkillType : public CSSkillType
{
    DECLARE_DYNAMIC(CSAbstractDefenseSkillType);

// Konstrukce a destrukce
protected:
	// konstruktor
    //     ten je protected, protoze nechceme vytvaret instance teto tridy,
    //     vzdy jen instance nektereho z potomku
    CSAbstractDefenseSkillType() { };
	// destruktor
    virtual ~CSAbstractDefenseSkillType() { };

// Inicializace a zniceni dat objektu pri vytvareni hry

    // inicializuje data typu skilly z dat "pData" velikosti "dwDataSize"
	//		vyjimky: CPersistentLoadException
	virtual void CreateSkillType(void *pData, DWORD dwDataSize) = 0;
	// znici data typu skilly
	virtual void DeleteSkillType() = 0;

// Ukladani dat typu skilly

	// ukladani dat potomka
	virtual void PersistentSaveSkillType(CPersistentStorage &storage) = 0;
	// nahravani pouze ulozenych dat potomka
	virtual void PersistentLoadSkillType(CPersistentStorage &storage) = 0;
	// preklad ukazatelu potomka
	virtual void PersistentTranslatePointersSkillType(CPersistentStorage &storage) = 0;
	// inicializace nahraneho objektu potomka
	virtual void PersistentInitSkillType() = 0;

// Get/Set metody serverovych informaci o typu skilly jednotky

	// vrati velikost dat skilly pro jednotku
	virtual DWORD GetSkillDataSize() = 0;
	// vrati masku skupin skill
	virtual DWORD GetGroupMask() { return Group_Defense; }

// Metody skilly

	// provede time slice na skille jednotky "pUnit"
	// jednotka "pUnit" je jiz zamcena pro zapis, metoda muze zamykat libovolny pocet jinych
	//		jednotek pro zapis/cteni (vcetne jednotek ze sveho bloku)
	virtual void DoTimeSlice(CSUnit *pUnit) = 0;

	// inicializuje skillu jednotky "pUnit"
	virtual void InitializeSkill(CSUnit *pUnit) = 0;
	// znici skillu jednotky "pUnit"
	virtual void DestructSkill(CSUnit *pUnit) = 0;

// Aktivace a deaktivace skilly
public:	
    // aktivuje skillu jednotky "pUnit" (jednotka musi byt zamcena pro zapis)
    void ActivateSkill(CSUnit *pUnit) { CSSkillType::ActivateSkill(pUnit); }
	// deaktivuje skillu jednotky "pUnit" (jednotka musi byt zamcena pro zapis)
    virtual void DeactivateSkill(CSUnit *pUnit) { CSSkillType::DeactivateSkill(pUnit); }

// Ukladani dat skilly u jednotky
protected:	
    // ukladani dat skilly
	virtual void PersistentSaveSkill(CPersistentStorage &storage, CSUnit *pUnit) = 0;
	// nahravani pouze ulozenych dat skilly
	virtual void PersistentLoadSkill(CPersistentStorage &storage, CSUnit *pUnit) = 0;
	// preklad ukazatelu dat skilly
	virtual void PersistentTranslatePointersSkill(CPersistentStorage &storage, CSUnit *pUnit) = 0;
	// inicializace nahranych dat skilly
	virtual void PersistentInitSkill(CSUnit *pUnit) = 0;

// Metody skilly pro attack skilly
public:
    // vraci masku interakci, ktere je schopna tato defense skilla branit
    virtual DWORD GetDefenseInteractionsMask() = 0;
    // dotaz na silu utoku
    virtual int QueryDefense(CSUnit *pUnit, int nAttackIntensity, DWORD dwInteractionMask) = 0;
    // provedeni vypoctu obrany na utok zadany v parametrech
    virtual int DoDefense(CSUnit *pAttackedUnit, CSUnit *pAggressor, int nAttackIntensity, DWORD dwInteractionMask) = 0;

// Interface volani ze skriptu
public:
	DECLARE_SKILLCALL_INTERFACE_MAP(CSAbstractDefenseSkillType, 4);
    DECLARE_SKILLCALL_METHOD0_READ_SOFT(SCI_GetDefenseInteractionsMask);
    DECLARE_SKILLCALL_METHOD2_READ_SOFT(SCI_QueryDefense, Int, int *, Int, int *);

// Metody volane ze skriptu
protected:
    ESyscallResult SCI_GetDefenseInteractionsMask(CSUnit *pUnit, CIProcess *pProcess, CISyscall **ppSysCall, CIBag *pBag, CIDataStackItem *pReturn);
    ESyscallResult SCI_QueryDefense(CSUnit *pUnit, CIProcess *pProcess, CISyscall **ppSysCall, CIBag *pBag, CIDataStackItem *pReturn, int *pnAttackIntensity, int *pnInteractionsMask);

// Data
protected:

};

#endif  // __SERVER_ABSTRACTDEFENSE_SKILL_TYPE__HEADER_INCLUDED__
