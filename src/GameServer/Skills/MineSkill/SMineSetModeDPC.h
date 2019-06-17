/***********************************************************
 * 
 * Projekt: Strategicka hra
 *    Cast: Server hry
 *   Autor: Roman Zenka
 * 
 *   Popis: DPC pro Vitka, aby si mohl hrat s modama. :)
 * 
 ***********************************************************/

#ifndef __SERVER_MINESETMODE_DPC__HEADER_INCLUDED__
#define __SERVER_MINESETMODE_DPC__HEADER_INCLUDED__

#include "GameServer\Skills\SysCalls\SSysCallDPC.h"

class CSMineSkillType;

//////////////////////////////////////////////////////////////////////
// Trida odlozeneho volani procedury
// pro nastaveni modu jednotky

class CSMineSetModeDPC : public CSSysCallDPC
{
	DECLARE_DYNAMIC(CSMineSetModeDPC);
	DECLARE_DPC(CSMineSetModeDPC);

// Konstrukce a destrukce
public:
	// konstruktor
	CSMineSetModeDPC();
    CSMineSetModeDPC(CIProcess *pProcess, CIDataStackItem *pReturnValue, CSUnit *pUnit, CSMineSkillType *pSkillType, int nMode);
protected:
    // destruktor
	virtual ~CSMineSetModeDPC();

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
	// provede odlozene volani procedury
    // (stara se o zneplatneni vsech odkazu na DPC)
	virtual void Run ();

  // We need a writer lock on the unit
  virtual BOOL NeedWriterLock () { return TRUE; }

// Data pro DPC
protected:
    // pointer na mine skillu vlastni jednotky
    CSMineSkillType *m_pSkillType;
    // cislo modu
    int m_nMode;

// Debugovaci metody
public:
#ifdef _DEBUG
    void AssertValid();
#endif  // _DEBUG
};

#endif  // __SERVER_MINESETMODE_DPC__HEADER_INCLUDED__
