/***********************************************************
 * 
 * Projekt: Strategická hra
 *    Èást: Server hry
 *   Autor: Karel Zikmund
 * 
 *   Popis: Tøída typu jednotky pro jednu civilizaci
 * 
 ***********************************************************/

#ifndef __SERVER_UNIT_TYPE__HEADER_INCLUDED__
#define __SERVER_UNIT_TYPE__HEADER_INCLUDED__

#include "GameServerCommon.h"

#include "AbstractDataClasses\256BitArray.h"

//////////////////////////////////////////////////////////////////////
// Dopøedné deklarace tøíd

// tøída civilizace
class CSCivilization;
// tøída typu skilly jednotky
class CSSkillType;
// tøída jednotky
class CSUnit;
// tøída grafu pro hledání cesty
class CSFindPathGraph;

// tøída ID jména typu skilly (ukazatel na záznam v tabulce øetìzcù)
class CStringTableItem;
// tøída jednotky na civilizaci
class CZUnit;

//////////////////////////////////////////////////////////////////////
// Tøída typu jednotky na serveru hry. Obsahuje všechny informace o typu 
//		jednotky, které potøebuje GameServer. Každá civilizace má svoji 
//		vlastní instanci této tøídy.
class CSUnitType : public CPersistentObject 
{
	friend class CSMap;
	friend class CSCivilization;
	friend class CSSkillType;
	friend class CSUnit;

	DECLARE_DYNAMIC ( CSUnitType )

// Datové typy
protected:
	// popis typu skilly
	struct SSkillTypeDescription 
	{
		// ukazatel na typ skilly
		CSSkillType *pSkillType;
		// ID typu skilly
		DWORD dwID;
		// ID jména typu skilly
		CStringTableItem *pNameID;
		// pøíznak, je-li skilla zapnuta
		BOOL bEnabled;
		// povolené módy typu skilly
		BYTE nAllowedModesMask;
		// maska skupin typu skill
		DWORD dwGroupMask;

	// Metody

		// konstruktor
		SSkillTypeDescription () { pSkillType = NULL; pNameID = NULL; };
	};

// Metody
protected:
// Konstrukce a destrukce

	// konstruktor
	CSUnitType ();
	// destruktor
	~CSUnitType ();

// Inicializace a znièení dat objektu

	// vytvoøí typ jednotky ze souboru typu jednotky "cUnitTypeFile" civilizace 
	//		"pCivilization"
	void Create ( CArchiveFile cUnitTypeFile, CSCivilization *pCivilization );
	// dokonèí vytvoøení typu jednotky
	void PostCreate ();
	// znièí data typu jednotky
	void Delete ();
	// vyplní objekt neškodnými daty (znièí èásteènì inicializavaná data)
	void SetEmptyData ();

// Ukládání dat (CPersistentObject metody)

	// ukládání dat
	void PersistentSave ( CPersistentStorage &storage );
	// nahrávání pouze uložených dat
	void PersistentLoad ( CPersistentStorage &storage, CSUnitType *pOldUnitType );
	// pøeklad ukazatelù
	void PersistentTranslatePointers ( CPersistentStorage &storage );
	// inicializace nahraného objektu
	void PersistentInit ( CSCivilization *pCivilization );
	// inicializace nahraného objektu - nelze použít
	void PersistentInit () { ASSERT ( FALSE ); };

public:
// Operace se zámkem typu jednotky

	// zamkne zámek pro ètenáøe (s timeoutem "dwTimeOut" v milisekundách, INFINITE=èeká 
	//		dokud se zámek neuvolní) (nemá smysl volat z MainLoop threadu) (FALSE=timeout)
	inline BOOL ReaderLock ( DWORD dwTimeOut = INFINITE );
	// odemkne zámek pro ètenáøe
	inline void ReaderUnlock ();
	// zamkne zámek pro zapisovatele (s timeoutem "dwTimeOut" v milisekundách, 
	//		INFINITE=èeká dokud se zámek neuvolní - skuteèná doba timeoutu mùže být
	//		až dvojnásobná) (nelze volat mimo MainLoop thread) (FALSE=timeout)
	inline BOOL WriterLock ( DWORD dwTimeOut = INFINITE );
	// odemkne zámek pro zapisovatele
	inline void WriterUnlock ();

// Serverové informace o typu jednotky

	// vrátí ID typu jednotky
	DWORD GetID () { ASSERT ( m_dwSkillTypeCount > 0 ); return m_dwID; };
	// vrátí ukazatel na civilizaci typu jednotky
	CSCivilization *GetCivilization () 
		{ ASSERT ( m_pCivilization != NULL ); return m_pCivilization; };
	// vrátí poèet skillù typu jednotky
	DWORD GetSkillCount () 
		{ ASSERT ( m_dwSkillTypeCount > 0 ); return m_dwSkillTypeCount; };
	// vrátí velikost dat skillù jednotky
	DWORD GetSkillDataSize () 
		{ ASSERT ( m_dwSkillDataSize > 0 ); return m_dwSkillDataSize; };
	// vrátí typ skilly s indexem "dwIndex"
	CSSkillType *GetSkillTypeByIndex ( DWORD dwIndex )
		{ ASSERT ( dwIndex < m_dwSkillTypeCount ); return m_aSkillTypeDescription[dwIndex].pSkillType; };
	// initializuje skilly jednotky "pUnit" (voláno po vytvoøení objektu jednotky)
	void InitializeSkills ( CSUnit *pUnit );
	// znièí skilly jednotky "pUnit" (voláno pøed znièením objektu jednotky)
	void DestructSkills ( CSUnit *pUnit );
	// deaktivuje skupiny skill "dwGroupMask" jednotky "pUnit" (jednotka musí být zamèena 
	//		pro zápis)
	void DeactivateSkillGroup ( CSUnit *pUnit, DWORD dwGroupMask );
	// vrátí další typ skilly ze skupiny skill "dwGroupMask" poèínaje typem skilly s 
	//		indexem "dwIndex", který posune za nalezený typ skilly (NULL=nenalezeno)
	CSSkillType *GetNextSkillType ( DWORD &dwIndex, DWORD dwGroupMask );
	// vrátí ukazatel na graf pro hledání cesty v módu "nMode" (0-7)
	CSFindPathGraph *GetFindPathGraph ( BYTE nMode ) 
		{ ASSERT ( nMode < 8 ); return m_aFindPathGraph[nMode]; };

// Get/Set metody herních informací o typu jednotky

	// vrátí maximální poèet životù jednotky
	int GetLivesMax () { ASSERT ( m_nLivesMax > 0 ); return m_nLivesMax; };
	// nastaví maximální poèet životù jednotky
	void SetLivesMax ( int nLivesMax ) 
		{ ASSERT ( m_nLivesMax > 0 ); ASSERT ( nLivesMax > 0 ); m_nLivesMax = nLivesMax; };
	// vrátí polomìr viditelnosti jednotky
	DWORD GetViewRadius () { return m_dwViewRadius; };
	// nastaví polomìr viditelnosti jednotky
	void SetViewRadius ( DWORD dwViewRadius ) { m_dwViewRadius = dwViewRadius; };
	// vrátí vertikální pozici jednotky
	DWORD GetVerticalPosition () 
		{ ASSERT ( m_dwVerticalPosition != 0 ); return m_dwVerticalPosition; };
	// nastaví vertikální pozici jednotky
	void SetVerticalPosition ( DWORD dwVerticalPosition ) 
		{ ASSERT ( dwVerticalPosition != 0 ); m_dwVerticalPosition = dwVerticalPosition; };
	// vrátí jméno typu jednotky
	CString GetName () { return m_strUnitTypeName; };
	// vrátí šíøku pøi pohybu v módu "nMode" (0-7)
	DWORD GetMoveWidth ( BYTE nMode ) 
		{ ASSERT ( nMode < 8 ); return m_aMoveWidth[nMode]; };

public:
// Operace s jednotkami typu jednotky

	// vytvoøí jednotku na pozici "pointPosition", s velitelem "pCommander", skriptem 
	//		"lpszScriptName", smìrem "nDirection" a vertikální pozicí "dwVerticalPosition" 
	//		(0=implicitní vertikální pozice z typu jednotky) (typ jednotky musí být zamèen pro 
	//		zápis, musí být zamèeny MapSquary, lze volat jen z MainLoop threadu) 
	//		(NULL=jednotku se nepodaøilo vyrobit)
	CSUnit *CreateUnit ( CPointDW pointPosition, CZUnit *pCommander, LPCTSTR lpszScriptName, 
		BYTE nDirection, DWORD dwVerticalPosition = 0 );
protected:
	// pøidá jednotku "pUnit" do seznamu jednotek typu jednotky (typ jednotky musí být 
	//		zamèen pro zápis)
	void AddUnit ( CSUnit *pUnit );
	// odebere jednotku "pUnit" ze seznamu jednotek typu jednotky (typ jednotky musí být 
	//		zamèen pro zápis) (není-li jednotka v seznamu jednotek, neprovádí nic)
	void DeleteUnit ( CSUnit *pUnit );

// Debuggovací informace
protected:
#ifdef _DEBUG
	// zkontroluje správná data objektu (TRUE=OK)
	BOOL CheckValidData ();
	// zkontroluje neškodná data objektu (TRUE=OK)
	BOOL CheckEmptyData ();
#endif //_DEBUG

// Data
private:
// Inicializaèní data nové jednotky tohoto typu

	// maximální poèet životù jednotky (nìjaká abstraktní velièina) (kvùli ukazování 
	//		pravítka s životem jednotky na GameClientovi, iniciální hodnota nové jednotky)
	int m_nLivesMax;
	// defaultní polomìr viditelnosti nové jednotky (v MapCellech)
	DWORD m_dwViewRadius;
	// defaultní vertikální umístìní nové jednotky
	DWORD m_dwVerticalPosition;
	// jméno typu jednotky (kvùli zobrazování jednotky na GameClientovi)
	CString m_strUnitTypeName;

// Herní informace o typu jednotky

	// šíøka jednotky pro pohyb na mapì (v MapCellech) (kvùli hledání cesty na mapì 
	//		odpovídající šíøky)
	DWORD m_aMoveWidth[8];
	// defaultní ID vzhledu jednotky
	DWORD m_aAppearanceID[8];
	// bitové masky pøíznakù neviditelnosti jednotky (tj. jak je jednotka neviditelná)
	DWORD m_aInvisibilityState[8];
	// bitové masky pøíznakù detekce neviditelnosti ostatních jednotek (tj. které 
	//		neviditelnosti jednotka vidí)
	DWORD m_aInvisibilityDetection[8];
	// ukazatel na graf pro hledání cesty
	CSFindPathGraph *m_aFindPathGraph[8];
	// pøíznaky typu jednotky (z MapEditoru - enum EUnitTypeFlags v Common\Map\Map.h)
	DWORD m_dwFlags;

// Serverové informace o typu jednotky

	// identifikaèní èíslo typu jednotky
	DWORD m_dwID;
	// ukazatel na civilizaci typu jednotky
	CSCivilization *m_pCivilization;

	// poèet typù skillù typu jednotky
	DWORD m_dwSkillTypeCount;
	// ukazatel na pole popisù typù skillù
	SSkillTypeDescription *m_aSkillTypeDescription;
	// velikost dat skillù jednotky
	DWORD m_dwSkillDataSize;

	// zámek typu jednotky
	CReadWriteLock m_cLock;

// Informace o jednotkách typu jednotky

	// ukazatel na první jednotku seznamu jednotek typu jednotky
	CSUnit *m_pFirstUnit;
};

#endif //__SERVER_UNIT_TYPE__HEADER_INCLUDED__
