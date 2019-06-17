/***********************************************************
 * 
 * Projekt: Strategick� hra
 *    ��st: Server hry
 *   Autor: Karel Zikmund
 * 
 *   Popis: T��da typu jednotky pro jednu civilizaci
 * 
 ***********************************************************/

#ifndef __SERVER_UNIT_TYPE__HEADER_INCLUDED__
#define __SERVER_UNIT_TYPE__HEADER_INCLUDED__

#include "GameServerCommon.h"

#include "AbstractDataClasses\256BitArray.h"

//////////////////////////////////////////////////////////////////////
// Dop�edn� deklarace t��d

// t��da civilizace
class CSCivilization;
// t��da typu skilly jednotky
class CSSkillType;
// t��da jednotky
class CSUnit;
// t��da grafu pro hled�n� cesty
class CSFindPathGraph;

// t��da ID jm�na typu skilly (ukazatel na z�znam v tabulce �et�zc�)
class CStringTableItem;
// t��da jednotky na civilizaci
class CZUnit;

//////////////////////////////////////////////////////////////////////
// T��da typu jednotky na serveru hry. Obsahuje v�echny informace o typu 
//		jednotky, kter� pot�ebuje GameServer. Ka�d� civilizace m� svoji 
//		vlastn� instanci t�to t��dy.
class CSUnitType : public CPersistentObject 
{
	friend class CSMap;
	friend class CSCivilization;
	friend class CSSkillType;
	friend class CSUnit;

	DECLARE_DYNAMIC ( CSUnitType )

// Datov� typy
protected:
	// popis typu skilly
	struct SSkillTypeDescription 
	{
		// ukazatel na typ skilly
		CSSkillType *pSkillType;
		// ID typu skilly
		DWORD dwID;
		// ID jm�na typu skilly
		CStringTableItem *pNameID;
		// p��znak, je-li skilla zapnuta
		BOOL bEnabled;
		// povolen� m�dy typu skilly
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

// Inicializace a zni�en� dat objektu

	// vytvo�� typ jednotky ze souboru typu jednotky "cUnitTypeFile" civilizace 
	//		"pCivilization"
	void Create ( CArchiveFile cUnitTypeFile, CSCivilization *pCivilization );
	// dokon�� vytvo�en� typu jednotky
	void PostCreate ();
	// zni�� data typu jednotky
	void Delete ();
	// vypln� objekt ne�kodn�mi daty (zni�� ��ste�n� inicializavan� data)
	void SetEmptyData ();

// Ukl�d�n� dat (CPersistentObject metody)

	// ukl�d�n� dat
	void PersistentSave ( CPersistentStorage &storage );
	// nahr�v�n� pouze ulo�en�ch dat
	void PersistentLoad ( CPersistentStorage &storage, CSUnitType *pOldUnitType );
	// p�eklad ukazatel�
	void PersistentTranslatePointers ( CPersistentStorage &storage );
	// inicializace nahran�ho objektu
	void PersistentInit ( CSCivilization *pCivilization );
	// inicializace nahran�ho objektu - nelze pou��t
	void PersistentInit () { ASSERT ( FALSE ); };

public:
// Operace se z�mkem typu jednotky

	// zamkne z�mek pro �ten��e (s timeoutem "dwTimeOut" v milisekund�ch, INFINITE=�ek� 
	//		dokud se z�mek neuvoln�) (nem� smysl volat z MainLoop threadu) (FALSE=timeout)
	inline BOOL ReaderLock ( DWORD dwTimeOut = INFINITE );
	// odemkne z�mek pro �ten��e
	inline void ReaderUnlock ();
	// zamkne z�mek pro zapisovatele (s timeoutem "dwTimeOut" v milisekund�ch, 
	//		INFINITE=�ek� dokud se z�mek neuvoln� - skute�n� doba timeoutu m��e b�t
	//		a� dvojn�sobn�) (nelze volat mimo MainLoop thread) (FALSE=timeout)
	inline BOOL WriterLock ( DWORD dwTimeOut = INFINITE );
	// odemkne z�mek pro zapisovatele
	inline void WriterUnlock ();

// Serverov� informace o typu jednotky

	// vr�t� ID typu jednotky
	DWORD GetID () { ASSERT ( m_dwSkillTypeCount > 0 ); return m_dwID; };
	// vr�t� ukazatel na civilizaci typu jednotky
	CSCivilization *GetCivilization () 
		{ ASSERT ( m_pCivilization != NULL ); return m_pCivilization; };
	// vr�t� po�et skill� typu jednotky
	DWORD GetSkillCount () 
		{ ASSERT ( m_dwSkillTypeCount > 0 ); return m_dwSkillTypeCount; };
	// vr�t� velikost dat skill� jednotky
	DWORD GetSkillDataSize () 
		{ ASSERT ( m_dwSkillDataSize > 0 ); return m_dwSkillDataSize; };
	// vr�t� typ skilly s indexem "dwIndex"
	CSSkillType *GetSkillTypeByIndex ( DWORD dwIndex )
		{ ASSERT ( dwIndex < m_dwSkillTypeCount ); return m_aSkillTypeDescription[dwIndex].pSkillType; };
	// initializuje skilly jednotky "pUnit" (vol�no po vytvo�en� objektu jednotky)
	void InitializeSkills ( CSUnit *pUnit );
	// zni�� skilly jednotky "pUnit" (vol�no p�ed zni�en�m objektu jednotky)
	void DestructSkills ( CSUnit *pUnit );
	// deaktivuje skupiny skill "dwGroupMask" jednotky "pUnit" (jednotka mus� b�t zam�ena 
	//		pro z�pis)
	void DeactivateSkillGroup ( CSUnit *pUnit, DWORD dwGroupMask );
	// vr�t� dal�� typ skilly ze skupiny skill "dwGroupMask" po��naje typem skilly s 
	//		indexem "dwIndex", kter� posune za nalezen� typ skilly (NULL=nenalezeno)
	CSSkillType *GetNextSkillType ( DWORD &dwIndex, DWORD dwGroupMask );
	// vr�t� ukazatel na graf pro hled�n� cesty v m�du "nMode" (0-7)
	CSFindPathGraph *GetFindPathGraph ( BYTE nMode ) 
		{ ASSERT ( nMode < 8 ); return m_aFindPathGraph[nMode]; };

// Get/Set metody hern�ch informac� o typu jednotky

	// vr�t� maxim�ln� po�et �ivot� jednotky
	int GetLivesMax () { ASSERT ( m_nLivesMax > 0 ); return m_nLivesMax; };
	// nastav� maxim�ln� po�et �ivot� jednotky
	void SetLivesMax ( int nLivesMax ) 
		{ ASSERT ( m_nLivesMax > 0 ); ASSERT ( nLivesMax > 0 ); m_nLivesMax = nLivesMax; };
	// vr�t� polom�r viditelnosti jednotky
	DWORD GetViewRadius () { return m_dwViewRadius; };
	// nastav� polom�r viditelnosti jednotky
	void SetViewRadius ( DWORD dwViewRadius ) { m_dwViewRadius = dwViewRadius; };
	// vr�t� vertik�ln� pozici jednotky
	DWORD GetVerticalPosition () 
		{ ASSERT ( m_dwVerticalPosition != 0 ); return m_dwVerticalPosition; };
	// nastav� vertik�ln� pozici jednotky
	void SetVerticalPosition ( DWORD dwVerticalPosition ) 
		{ ASSERT ( dwVerticalPosition != 0 ); m_dwVerticalPosition = dwVerticalPosition; };
	// vr�t� jm�no typu jednotky
	CString GetName () { return m_strUnitTypeName; };
	// vr�t� ���ku p�i pohybu v m�du "nMode" (0-7)
	DWORD GetMoveWidth ( BYTE nMode ) 
		{ ASSERT ( nMode < 8 ); return m_aMoveWidth[nMode]; };

public:
// Operace s jednotkami typu jednotky

	// vytvo�� jednotku na pozici "pointPosition", s velitelem "pCommander", skriptem 
	//		"lpszScriptName", sm�rem "nDirection" a vertik�ln� pozic� "dwVerticalPosition" 
	//		(0=implicitn� vertik�ln� pozice z typu jednotky) (typ jednotky mus� b�t zam�en pro 
	//		z�pis, mus� b�t zam�eny MapSquary, lze volat jen z MainLoop threadu) 
	//		(NULL=jednotku se nepoda�ilo vyrobit)
	CSUnit *CreateUnit ( CPointDW pointPosition, CZUnit *pCommander, LPCTSTR lpszScriptName, 
		BYTE nDirection, DWORD dwVerticalPosition = 0 );
protected:
	// p�id� jednotku "pUnit" do seznamu jednotek typu jednotky (typ jednotky mus� b�t 
	//		zam�en pro z�pis)
	void AddUnit ( CSUnit *pUnit );
	// odebere jednotku "pUnit" ze seznamu jednotek typu jednotky (typ jednotky mus� b�t 
	//		zam�en pro z�pis) (nen�-li jednotka v seznamu jednotek, neprov�d� nic)
	void DeleteUnit ( CSUnit *pUnit );

// Debuggovac� informace
protected:
#ifdef _DEBUG
	// zkontroluje spr�vn� data objektu (TRUE=OK)
	BOOL CheckValidData ();
	// zkontroluje ne�kodn� data objektu (TRUE=OK)
	BOOL CheckEmptyData ();
#endif //_DEBUG

// Data
private:
// Inicializa�n� data nov� jednotky tohoto typu

	// maxim�ln� po�et �ivot� jednotky (n�jak� abstraktn� veli�ina) (kv�li ukazov�n� 
	//		prav�tka s �ivotem jednotky na GameClientovi, inici�ln� hodnota nov� jednotky)
	int m_nLivesMax;
	// defaultn� polom�r viditelnosti nov� jednotky (v MapCellech)
	DWORD m_dwViewRadius;
	// defaultn� vertik�ln� um�st�n� nov� jednotky
	DWORD m_dwVerticalPosition;
	// jm�no typu jednotky (kv�li zobrazov�n� jednotky na GameClientovi)
	CString m_strUnitTypeName;

// Hern� informace o typu jednotky

	// ���ka jednotky pro pohyb na map� (v MapCellech) (kv�li hled�n� cesty na map� 
	//		odpov�daj�c� ���ky)
	DWORD m_aMoveWidth[8];
	// defaultn� ID vzhledu jednotky
	DWORD m_aAppearanceID[8];
	// bitov� masky p��znak� neviditelnosti jednotky (tj. jak je jednotka neviditeln�)
	DWORD m_aInvisibilityState[8];
	// bitov� masky p��znak� detekce neviditelnosti ostatn�ch jednotek (tj. kter� 
	//		neviditelnosti jednotka vid�)
	DWORD m_aInvisibilityDetection[8];
	// ukazatel na graf pro hled�n� cesty
	CSFindPathGraph *m_aFindPathGraph[8];
	// p��znaky typu jednotky (z MapEditoru - enum EUnitTypeFlags v Common\Map\Map.h)
	DWORD m_dwFlags;

// Serverov� informace o typu jednotky

	// identifika�n� ��slo typu jednotky
	DWORD m_dwID;
	// ukazatel na civilizaci typu jednotky
	CSCivilization *m_pCivilization;

	// po�et typ� skill� typu jednotky
	DWORD m_dwSkillTypeCount;
	// ukazatel na pole popis� typ� skill�
	SSkillTypeDescription *m_aSkillTypeDescription;
	// velikost dat skill� jednotky
	DWORD m_dwSkillDataSize;

	// z�mek typu jednotky
	CReadWriteLock m_cLock;

// Informace o jednotk�ch typu jednotky

	// ukazatel na prvn� jednotku seznamu jednotek typu jednotky
	CSUnit *m_pFirstUnit;
};

#endif //__SERVER_UNIT_TYPE__HEADER_INCLUDED__
