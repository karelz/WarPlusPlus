/***********************************************************
 * 
 * Projekt: Strategicka hra
 *    Cast: Server hry
 *   Autor: Tomas Karban
 * 
 *   Popis: DPC pro zjisteni, zdali mohu utocit na danou jednotku
 * 
 ***********************************************************/

#ifndef __SERVER_CANATTACKUNIT_DPC__HEADER_INCLUDED__
#define __SERVER_CANATTACKUNIT_DPC__HEADER_INCLUDED__

#include "GameServer\Skills\SysCalls\SSysCallDPC.h"

//////////////////////////////////////////////////////////////////////
// Trida odlozeneho volani procedury
// pro zjisteni, zdali mohu utocit na danou jednotku

class CSCanAttackUnitDPC : public CSSysCallDPC
{
	DECLARE_DYNAMIC(CSCanAttackUnitDPC)
	DECLARE_DPC(CSCanAttackUnitDPC)

// Konstrukce a destrukce
public:
	// konstruktor
	CSCanAttackUnitDPC();
    CSCanAttackUnitDPC(CIProcess *pProcess, CIDataStackItem *pReturnValue, CSUnit *pUnit, CSAbstractAttackSkillType *pSkillType, CSUnit *pAttackableUnit);
protected:
    // destruktor
	virtual ~CSCanAttackUnitDPC();

// Ukladani dat
protected:
	// ulozeni DPS
	virtual void PersistentSaveDPC(CPersistentStorage &storage);
	// nahrani DPC
	virtual void PersistentLoadDPC(CPersistentStorage &storage);
	// preklad ukazatelu DPC
	virtual void PersistentTranslatePointersDPC(CPersistentStorage &storage);
	// inicializace po nahrani DPC
	virtual void PersistentInitDPC();

// Operace s DPC
	// vrati priznak, ma-li se jednotka zamykat pro zapis/cteni
	virtual BOOL NeedWriterLock () { return FALSE; }

	// provede odlozene volani procedury
    // (stara se o zneplatneni vsech odkazu na DPC)
	virtual void Run ();

// Data pro DPC
protected:
    // pointer na abstraktni attack skillu vlastni jednotky
    CSAbstractAttackSkillType *m_pSkillType;
    // pointer na jednotku, na kterou se ptam
    CSUnit *m_pAttackableUnit;

// Debugovaci metody
public:
#ifdef _DEBUG
    void AssertValid();
#endif  // _DEBUG
};

#endif  // __SERVER_CANATTACKUNIT_DPC__HEADER_INCLUDED__
