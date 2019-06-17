/***********************************************************
 * 
 * Projekt: Strategicka hra
 *    Cast: Server hry
 *   Autor: Tomas Karban
 * 
 *   Popis: DPC pro zjistovani nepratelskych jednotkek
 *          v dosahu moji zbrane
 * 
 ***********************************************************/

#ifndef __SERVER_ATTACKABLEUNITS_DPC__HEADER_INCLUDED__
#define __SERVER_ATTACKABLEUNITS_DPC__HEADER_INCLUDED__

#include "GameServer\Skills\SysCalls\SSysCallDPC.h"

class CSAbstractAttackSkillType;

//////////////////////////////////////////////////////////////////////
// Trida odlozeneho volani procedury
// pro zjistovani jednotek, na ktere lze utocit

class CSAttackableUnitsDPC : public CSSysCallDPC
{
	DECLARE_DYNAMIC(CSAttackableUnitsDPC)
	DECLARE_DPC(CSAttackableUnitsDPC)

// Konstrukce a destrukce
public:
	// konstruktor
	CSAttackableUnitsDPC();
    CSAttackableUnitsDPC(CIProcess *pProcess, CIDataStackItem *pReturnValue, CSUnit *pUnit, CSAbstractAttackSkillType *pSkillType);
protected:
    // destruktor
    virtual ~CSAttackableUnitsDPC(); 

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
	virtual void Run();

// Data pro DPC
protected:
    // ukazatel na typ attack skilly, ve kterem jsou ulozeny informace
    // o schopnostech utoku jednotky
    CSAbstractAttackSkillType *m_pSkillType;

// Debugovaci metody
public:
#ifdef _DEBUG
    void AssertValid();
#endif  // _DEBUG
};

#endif  // __SERVER_ATTACKABLEUNITS_DPC__HEADER_INCLUDED__
