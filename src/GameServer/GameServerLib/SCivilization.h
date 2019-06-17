/***********************************************************
 * 
 * Projekt: Strategick� hra
 *    ��st: Server hry
 *   Autor: Karel Zikmund
 * 
 *   Popis: T��da civilizace
 * 
 ***********************************************************/

#ifndef __SERVER_CIVILIZATION__HEADER_INCLUDED__
#define __SERVER_CIVILIZATION__HEADER_INCLUDED__

#include "SUnitType.h"
#include "SResources.h"

//////////////////////////////////////////////////////////////////////
// Dop�edn� deklarace t��d

// t��da mapy
class CSMap;
// t��da civilizace na civilizaci
class CZCivilization;
// t��da jednotky
class CSUnit;

//////////////////////////////////////////////////////////////////////
// Deklarace pomocn�ch typ�

// t��da pole ukazatel� na archivy
typedef CTypedPtrArray<CPtrArray, CDataArchive *> CDataArchivePtrArray;

//////////////////////////////////////////////////////////////////////
// T��da civilizace na serveru hry.
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

// Inicializace a zni�en� dat objektu

	// vytvo�� civilizaci ze souboru mapy "cMapFile" s indexem civilizace 
	//		"dwCivilizationIndex" a p�ekladovou tabulkou ID na jm�no ScriptSetu 
	//		"cScriptSetTable"
	void Create ( CArchiveFile cMapFile, DWORD dwCivilizationIndex, 
		CMap<DWORD, DWORD, CString, CString &> &cScriptSetTable );
	// vytvo�� typy jednotek civilizace z pole archiv� knihoven civilizac� 
	//		"pUnitTypeLibraryArchiveArray" obsahuj�c� "dwUnitTypeCount" typ� jednotek
	void CreateUnitTypes ( CDataArchivePtrArray *pUnitTypeLibraryArchiveArray, 
		DWORD dwUnitTypeCount );
	// dokon�� vytvo�en� jednotek civilizace
	void PostCreateUnitTypes ();
	// zni�� data civilizace
	void Delete ();
	// vypln� objekt ne�kodn�mi daty (zni�� ��ste�n� inicializavan� data)
	void SetEmptyData ();

// Ukl�d�n� dat (CPersistentObject metody)

	// ukl�d�n� dat
	void PersistentSave ( CPersistentStorage &storage );
	// nahr�v�n� pouze ulo�en�ch dat
	void PersistentLoad ( CPersistentStorage &storage );
	// p�eklad ukazatel�
	void PersistentTranslatePointers ( CPersistentStorage &storage );
	// inicializace nahran�ho objektu
	void PersistentInit ();

// Operace s civilizac�

	// nastav� ��slo civilizace
	void SetCivilizationIndex ( DWORD dwCivilizationIndex ) 
		{ m_dwCivilizationIndex = dwCivilizationIndex; };
public:
	// vr�t� civilizaci na civilizaci
	CZCivilization *GetZCivilization () { return m_pZCivilization; };
	// vr�t� ID civilizace
	DWORD GetCivilizationID () { return m_dwCivilizationID; };
	// vr�t� ��slo civilizace
	DWORD GetCivilizationIndex () { return m_dwCivilizationIndex; };
	// vr�t� jm�no civilizace
	CString GetCivilizationName () { return m_strCivilizationName; };
	// vr�t� barvu civilizace
	DWORD GetCivilizationColor () { return m_dwCivilizationColor; }
	// vr�t� typ jednotky s indexem "dwUnitTypeIndex"
	CSUnitType *GetUnitType ( DWORD dwUnitTypeIndex ) 
		{ ASSERT ( dwUnitTypeIndex < m_dwUnitTypeCount ); return &m_pUnitTypes[dwUnitTypeIndex]; };
	// vr�t� typ jednotky s ID "dwUnitTypeID" (NULL=neexistuj�c� typ jednotky)
	CSUnitType *GetUnitTypeByID ( DWORD dwUnitTypeID );
	// vr�t� z�mek civilizace (po uzam�en� civilizace se nesm� zamykat ��dn� jin� z�mky 
	//		kv�li deadlock�m! - na civilizaci se nevztahuj� pravidla o zamyk�n� jednotek)
	CReadWriteLock *GetCivilizationLock () { return &m_cCivilizationLock; };
	// vr�t� surovinu "dwResourceIndex" civilizace (pokud v�sledek bude zneu��v�n, 
	//		civilizace mus� b�t zam�ena pro �ten�)
	int GetResource ( DWORD dwResourceIndex ) 
		{ ASSERT ( dwResourceIndex < RESOURCE_COUNT ); return m_aResources[dwResourceIndex]; };
	// p�id� civilizaci surovinu "dwResourceIndex" v mno�stv� "nResourceAmount" (civilizace 
	//		mus� b�t zam�ena pro z�pis)
	void AddResource ( DWORD dwResourceIndex, int nResourceAmount ) 
		{ ASSERT ( dwResourceIndex < RESOURCE_COUNT ); m_aResources[dwResourceIndex] += nResourceAmount; };
	// zjist�, lze-li civilizaci odebrat surovinu "dwResourceIndex" v mno�stv� 
	//		"nResourceAmount" (bude-li v�sledek zneu��v�n, civilizace mus� b�t zam�ena 
	//		pro �ten�)
	BOOL CanRemoveResource ( DWORD dwResourceIndex, int nResourceAmount ) 
		{ ASSERT ( dwResourceIndex < RESOURCE_COUNT ); return ( ( m_aResources[dwResourceIndex] - nResourceAmount ) > 0 ); };
	// odebere civilizaci surovinu "dwResourceIndex" v mno�stv� "nResourceAmount" (civilizace 
	//		mus� b�t zam�ena pro z�pis)
	void RemoveResource ( DWORD dwResourceIndex, int nResourceAmount ) 
		{ ASSERT ( dwResourceIndex < RESOURCE_COUNT ); m_aResources[dwResourceIndex] -= nResourceAmount; };
	// vr�t� suroviny civilizace (civilizace mus� b�t zam�ena pro �ten�)
	TSResources *GetResources () { return &m_aResources; };
	// p�id� civilizaci suroviny "aResources" (civilizace mus� b�t zam�ena pro z�pis)
	void AddResources ( TSResources &aResources );
	// zjist�, lze-li civilizaci odebrat suroviny "aResources" (civilizace mus� b�t 
	//		zam�ena pro �ten�)
	BOOL CanRemoveResources ( TSResources &aResources );
	// odebere civilizaci suroviny "aResources" (civilizace mus� b�t zam�ena pro z�pis)
	void RemoveResources ( TSResources &aResources );
	// vr�t� startovac� pozici klienta na map�
	CPointDW GetClientStartMapPosition () { return m_pointClientStartMapPosition; };

// Debuggovac� informace
private:
#ifdef _DEBUG
	// zkontroluje spr�vn� data objektu (TRUE=OK)
	BOOL CheckValidData ();
	// zkontroluje ne�kodn� data objektu (TRUE=OK)
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
	// po�et typ� jednotek
	DWORD m_dwUnitTypeCount;
	// pole typ� jednotek
	CSUnitType *m_pUnitTypes;
	// jm�no civilizace
	CString m_strCivilizationName;
	// barva civilizace
	DWORD m_dwCivilizationColor;
	// z�mek civilizace
	CReadWriteLock m_cCivilizationLock;
	// mno�stv� zdroj� civilizace
	TSResources m_aResources;
	// startovac� pozice klienta civilizace na map�
	CPointDW m_pointClientStartMapPosition;
};

#endif //__SERVER_CIVILIZATION__HEADER_INCLUDED__
