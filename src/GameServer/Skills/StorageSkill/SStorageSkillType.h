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

	// Aktivuje skillu na zadane jednotce
	BOOL ActivateSkill(CSUnit *pUnit, CZUnit *pStorageUnit);
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

// Full infa
public:
	// vypln� full info "strInfo" skilly jednotky "pUnit"
	virtual void FillFullInfo ( CSUnit *pUnit, CString &strInfo );
	// vypln� enemy full info "strInfo" skilly jednotky "pUnit"
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

// Interface vol�n� ze skript�
public:

	// interface vol�n� ze skript�
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
