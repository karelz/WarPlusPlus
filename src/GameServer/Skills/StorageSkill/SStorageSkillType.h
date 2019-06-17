// SStorageSkillType.h: interface for the CSStorageSkillType class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SSTORAGESKILLTYPE_H__74FE6113_A282_11D4_8112_0000B48431EC__INCLUDED_)
#define AFX_SSTORAGESKILLTYPE_H__74FE6113_A282_11D4_8112_0000B48431EC__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "GameServer\GameServer\SSkillType.h"
#include "Common\Map\MStorageSkillType.h"
#include "SStorageSkill.h"
#include "GameServer\GameServer\SResources.h"

class CSStorageSkillType : public CSSkillType
{
	friend class CSMineSkillType; // Ta muze cist ma soukroma data o rychlosti unloadu

	DECLARE_DYNAMIC(CSStorageSkillType);
	// deklarace typu skilly
	DECLARE_SKILL_TYPE ( CSStorageSkillType )

// Enumy
public:
	// Vysledek tezeni
	enum EStoreResult {
		// Nema smysl dal tezit		
		error=0,			// Chyba, vsechno spatne
		incompatible,		// Nejsem kompatibilni s tvymi pozadavky
		didMyBest,			// Svou cast jsem splnila, uz nemuzu vic nalozit
		
		// Jeste ma smysl dal tezit
		continueWork=0x10,	// Znacka, ze ma smysl pokracovat
		canAcceptMoreLater,	// Mas volne misto na resourcy, ktere jsem v budoucnu schopna vyprodukovat, ale ne ted hned
		canAcceptMore,		// V pristim timeslicu dam dalsi davku, zatim na ni mas misto
		allNeedsSatisfied,	// Tva kapacita byla zaplnena na maximum
	};

// Metody
public:

// Konstrukce a destrukce
public:
	// Konstruktor
	CSStorageSkillType();
	// Destruktor
	virtual ~CSStorageSkillType();

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

	// Aktivuje skillu na zadane jednotce
	BOOL ActivateSkill(CSUnit *pUnit, CZUnit *pStorageUnit);
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

// Full infa
public:
	// vyplní full info "strInfo" skilly jednotky "pUnit"
	virtual void FillFullInfo ( CSUnit *pUnit, CString &strInfo );
	// vyplní enemy full info "strInfo" skilly jednotky "pUnit"
	virtual void FillEnemyFullInfo ( CSUnit *pUnit, CString &strInfo );

// Komunikace s Mine skillou
public:
	// Volano, kdyz do nas jednotka zacala unloadit
	void UnloadStarted(CSUnit *pUnit);

	// Volano, kdyz do nas jednotka unloadit prestala
	void UnloadEnded(CSUnit *pUnit);
	
	// Jednotka zada o ulozeni daneho mnozstvi resourcu, rika
	// kolik je schopna unloadnout za timeslice
	EStoreResult StoreResources(CSUnit *pStorageUnit, CSResources &Storage, CSResources &UnloadPerTimeslice);

// Interface volání ze skriptù
public:

	// interface volání ze skriptù
	DECLARE_SKILLCALL_INTERFACE_MAP ( CSStorageSkillType, 2 );

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
	// Kolik jsme schopni pojmout za timeslice
	CSResources m_AcceptedPerTimeslice;

	// Vzhled, kdyz do mne unloadi
	DWORD m_dwAppearanceWhileUnloading;
};

#endif // !defined(AFX_SSTORAGESKILLTYPE_H__74FE6113_A282_11D4_8112_0000B48431EC__INCLUDED_)
