/***********************************************************
 * 
 * Projekt: Strategická hra
 *    Èást: Server hry
 *   Autor: Karel Zikmund
 * 
 *   Popis: Tøída civilizace
 * 
 ***********************************************************/

#ifndef __SERVER_CIVILIZATION__HEADER_INCLUDED__
#define __SERVER_CIVILIZATION__HEADER_INCLUDED__

#include "SUnitType.h"
#include "SResources.h"

//////////////////////////////////////////////////////////////////////
// Dopøedné deklarace tøíd

// tøída mapy
class CSMap;
// tøída civilizace na civilizaci
class CZCivilization;
// tøída jednotky
class CSUnit;

//////////////////////////////////////////////////////////////////////
// Deklarace pomocných typù

// tøída pole ukazatelù na archivy
typedef CTypedPtrArray<CPtrArray, CDataArchive *> CDataArchivePtrArray;

//////////////////////////////////////////////////////////////////////
// Tøída civilizace na serveru hry.
class CSCivilization : public CPersistentObject 
{
	friend class CSMap;
	friend class CSUnit;

	DECLARE_DYNAMIC ( CSCivilization )

// Metody
protected:
// Konstrukce a destrukce

	// konstruktor
	CSCivilization ();
	// destruktor
	~CSCivilization ();

// Inicializace a znièení dat objektu

	// vytvoøí civilizaci ze souboru mapy "cMapFile" s indexem civilizace 
	//		"dwCivilizationIndex" a pøekladovou tabulkou ID na jméno ScriptSetu 
	//		"cScriptSetTable"
	void Create ( CArchiveFile cMapFile, DWORD dwCivilizationIndex, 
		CMap<DWORD, DWORD, CString, CString &> &cScriptSetTable );
	// vytvoøí typy jednotek civilizace z pole archivù knihoven civilizací 
	//		"pUnitTypeLibraryArchiveArray" obsahující "dwUnitTypeCount" typù jednotek
	void CreateUnitTypes ( CDataArchivePtrArray *pUnitTypeLibraryArchiveArray, 
		DWORD dwUnitTypeCount );
	// dokonèí vytvoøení jednotek civilizace
	void PostCreateUnitTypes ();
	// znièí data civilizace
	void Delete ();
	// vyplní objekt neškodnými daty (znièí èásteènì inicializavaná data)
	void SetEmptyData ();

// Ukládání dat (CPersistentObject metody)

	// ukládání dat
	void PersistentSave ( CPersistentStorage &storage );
	// nahrávání pouze uložených dat
	void PersistentLoad ( CPersistentStorage &storage );
	// pøeklad ukazatelù
	void PersistentTranslatePointers ( CPersistentStorage &storage );
	// inicializace nahraného objektu
	void PersistentInit ();

// Operace s civilizací

	// nastaví èíslo civilizace
	void SetCivilizationIndex ( DWORD dwCivilizationIndex ) 
		{ m_dwCivilizationIndex = dwCivilizationIndex; };
public:
	// vrátí civilizaci na civilizaci
	CZCivilization *GetZCivilization () { return m_pZCivilization; };
	// vrátí ID civilizace
	DWORD GetCivilizationID () { return m_dwCivilizationID; };
	// vrátí èíslo civilizace
	DWORD GetCivilizationIndex () { return m_dwCivilizationIndex; };
	// vrátí jméno civilizace
	CString GetCivilizationName () { return m_strCivilizationName; };
	// vrátí barvu civilizace
	DWORD GetCivilizationColor () { return m_dwCivilizationColor; }
	// vrátí typ jednotky s indexem "dwUnitTypeIndex"
	CSUnitType *GetUnitType ( DWORD dwUnitTypeIndex ) 
		{ ASSERT ( dwUnitTypeIndex < m_dwUnitTypeCount ); return &m_pUnitTypes[dwUnitTypeIndex]; };
	// vrátí typ jednotky s ID "dwUnitTypeID" (NULL=neexistující typ jednotky)
	CSUnitType *GetUnitTypeByID ( DWORD dwUnitTypeID );
	// vrátí zámek civilizace (po uzamèení civilizace se nesmí zamykat žádné jiné zámky 
	//		kvùli deadlockùm! - na civilizaci se nevztahují pravidla o zamykání jednotek)
	CReadWriteLock *GetCivilizationLock () { return &m_cCivilizationLock; };
	// vrátí surovinu "dwResourceIndex" civilizace (pokud výsledek bude zneužíván, 
	//		civilizace musí být zamèena pro ètení)
	int GetResource ( DWORD dwResourceIndex ) 
		{ ASSERT ( dwResourceIndex < RESOURCE_COUNT ); return m_aResources[dwResourceIndex]; };
	// pøidá civilizaci surovinu "dwResourceIndex" v množství "nResourceAmount" (civilizace 
	//		musí být zamèena pro zápis)
	void AddResource ( DWORD dwResourceIndex, int nResourceAmount ) 
		{ ASSERT ( dwResourceIndex < RESOURCE_COUNT ); m_aResources[dwResourceIndex] += nResourceAmount; };
	// zjistí, lze-li civilizaci odebrat surovinu "dwResourceIndex" v množství 
	//		"nResourceAmount" (bude-li výsledek zneužíván, civilizace musí být zamèena 
	//		pro ètení)
	BOOL CanRemoveResource ( DWORD dwResourceIndex, int nResourceAmount ) 
		{ ASSERT ( dwResourceIndex < RESOURCE_COUNT ); return ( ( m_aResources[dwResourceIndex] - nResourceAmount ) > 0 ); };
	// odebere civilizaci surovinu "dwResourceIndex" v množství "nResourceAmount" (civilizace 
	//		musí být zamèena pro zápis)
	void RemoveResource ( DWORD dwResourceIndex, int nResourceAmount ) 
		{ ASSERT ( dwResourceIndex < RESOURCE_COUNT ); m_aResources[dwResourceIndex] -= nResourceAmount; };
	// vrátí suroviny civilizace (civilizace musí být zamèena pro ètení)
	TSResources *GetResources () { return &m_aResources; };
	// pøidá civilizaci suroviny "aResources" (civilizace musí být zamèena pro zápis)
	void AddResources ( TSResources &aResources );
	// zjistí, lze-li civilizaci odebrat suroviny "aResources" (civilizace musí být 
	//		zamèena pro ètení)
	BOOL CanRemoveResources ( TSResources &aResources );
	// odebere civilizaci suroviny "aResources" (civilizace musí být zamèena pro zápis)
	void RemoveResources ( TSResources &aResources );
	// vrátí startovací pozici klienta na mapì
	CPointDW GetClientStartMapPosition () { return m_pointClientStartMapPosition; };

// Debuggovací informace
private:
#ifdef _DEBUG
	// zkontroluje správná data objektu (TRUE=OK)
	BOOL CheckValidData ();
	// zkontroluje neškodná data objektu (TRUE=OK)
	BOOL CheckEmptyData ();
#endif //_DEBUG

// Data
private:
// Informace o civilizaci

	// ID civilizace
	DWORD m_dwCivilizationID;
	// civilizace na civilizaci
	CZCivilization *m_pZCivilization;
	// index civilizace
	DWORD m_dwCivilizationIndex;
	// poèet typù jednotek
	DWORD m_dwUnitTypeCount;
	// pole typù jednotek
	CSUnitType *m_pUnitTypes;
	// jméno civilizace
	CString m_strCivilizationName;
	// barva civilizace
	DWORD m_dwCivilizationColor;
	// zámek civilizace
	CReadWriteLock m_cCivilizationLock;
	// množství zdrojù civilizace
	TSResources m_aResources;
	// startovací pozice klienta civilizace na mapì
	CPointDW m_pointClientStartMapPosition;
};

#endif //__SERVER_CIVILIZATION__HEADER_INCLUDED__
