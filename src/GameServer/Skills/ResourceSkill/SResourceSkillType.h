// SResourceSkillType.h: interface for the CSResourceSkillType class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SRESOURCESKILLTYPE_H__74FE6113_A282_11D4_8112_0000B48431EC__INCLUDED_)
#define AFX_SRESOURCESKILLTYPE_H__74FE6113_A282_11D4_8112_0000B48431EC__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "GameServer\GameServer\SSkillType.h"
#include "Common\Map\MResourceSkillType.h"
#include "SResourceSkill.h"
#include "GameServer\GameServer\SResources.h"

class CSResourceSkillType : public CSSkillType
{
	friend class CSMineSkillType; // Ta muze cist ma soukroma data o rychlosti tezeni

	DECLARE_DYNAMIC(CSResourceSkillType);
	// deklarace typu skilly
	DECLARE_SKILL_TYPE ( CSResourceSkillType )

// Enumy
public:
	// Vysledek tezeni
	enum ELoadResult {
		// Nema smysl dal tezit		
		error=0,			// Chyba, vsechno spatne
		incompatible,		// Nejsem kompatibilni s tvymi pozadavky
		didMyBest,			// Svou cast jsem splnila, uz nemuzu vic nalozit

		// Jeste ma smysl dal tezit
		continueWork=0x10,	// Znacka, ze ma smysl pokracovat
		canOfferMoreLater,	// Mas volne misto na resourcy, ktere jsem v budoucnu schopna vyprodukovat, ale ne ted hned
		canOfferMore,		// V pristim timeslicu dam dalsi davku, zatim na ni mas misto
		allNeedsSatisfied,	// Tva kapacita byla zaplnena na maximum
	};

// Konstrukce a destrukce
public:
	// Konstruktor
	CSResourceSkillType();
	// Destruktor
	virtual ~CSResourceSkillType();

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

public:
	// dotaz zda je skilla videt nepratelskou jednotkou
	virtual BOOL CanBeCalledByEnemy()  {  return TRUE; }

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
	BOOL ActivateSkill(CSUnit *pUnit);
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

// Komunikace s Mine skillou
public:
	// Notifikace, ze tezeni zacina
	void LoadStarted(CSUnit *pUnit);
	// Notifikace, ze tezeni konci
	void LoadEnded(CSUnit *pUnit);
	// Zadost o predani resourcu jednotce
	ELoadResult LoadResources(CSUnit *pMinedUnit, CSResources &Storage, CSResources &StorageSize, CSResources &LoadPerTimeslice);

// Full infa
public:
	// vypln� full info "strInfo" skilly jednotky "pUnit"
	virtual void FillFullInfo ( CSUnit *pUnit, CString &strInfo );
	// vypln� enemy full info "strInfo" skilly jednotky "pUnit"
	virtual void FillEnemyFullInfo ( CSUnit *pUnit, CString &strInfo );


// Interface vol�n� ze skript�
public:

	// interface vol�n� ze skript�
	DECLARE_SKILLCALL_INTERFACE_MAP ( CSResourceSkillType, 3 );
	DECLARE_SKILLCALL_METHOD0_READ_SOFT_EEC ( SCI_GetResources );

// Deklarace funkci pro volani ze skriptu
protected:

	// Zjisti, kolik resourcu je momentalne natezenych
	ESyscallResult SCI_GetResources (CSUnit *pUnit, CIProcess *pProcess, CISyscall **ppSysCall, CIBag *pBag, bool bCalledByEnemy, CIDataStackItem *pReturn);

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
	// Kolik mame na zacatku
	CSResources m_StorageSize;

	// Kolik jsme schopni dat jedne jednotce za timeslice
	CSResources m_AvailablePerTimeslice;
	
	// Limity pro jednotlive appearance
	CSResources m_ResourcesLimit[3];

	// Vzhledy normalni
	DWORD m_dwAppearance[3];

	// Vzhledy behem tezeni
	DWORD m_dwAppearanceMine[3];

	// Vzhled po dotezeni
	DWORD m_dwAppearanceAfterMining;

	// Pocet timeslicu pro vzhled po dotezeni
	DWORD m_dwTimeslicesAfterMining;
};

#endif // !defined(AFX_SRESOURCESKILLTYPE_H__74FE6113_A282_11D4_8112_0000B48431EC__INCLUDED_)
