/***********************************************************
 * 
 * Projekt: Strategická hra
 *    Èást: Server hry
 *   Autor: Karel Zikmund
 * 
 *   Popis: Tøída jednotky
 * 
 ***********************************************************/

#ifndef __SERVER_UNIT__HEADER_INCLUDED__
#define __SERVER_UNIT__HEADER_INCLUDED__

#include <stdlib.h>

#include "AbstractDataClasses\BitArray.h"
#include "AbstractDataClasses\PrivilegedThreadReadWriteLock.h"

#include "SUnitType.h"
#include "SUnitList.h"
#include "SCivilizationBitArray.h"
#include "SCheckPoint.h"
#include "SPosition.h"

#include "Common\Map\MapDataTypes.h"

//////////////////////////////////////////////////////////////////////
// Dopøedné deklarace

// tøída MapSquaru na mapì
class CSMapSquare;
// tøída typu skilly
class CSSkillType;
// tøída jednotky na civilizaci
class CZUnit;
// tøída rozesílatele informací o jednotkách klientovi civilizace
class CZClientUnitInfoSender;

// start info jednotky
struct SUnitStartInfo;
// brief info jednotky
struct SUnitBriefInfo;
// check point info jednotky
struct SUnitCheckPointInfo;
// full info jednotky
struct SUnitFullInfo;
// enemy full info jednotky
struct SUnitEnemyFullInfo;
// start full info jednotky
struct SUnitStartFullInfo;
// start enemy full info jednotky
struct SUnitStartEnemyFullInfo;

//////////////////////////////////////////////////////////////////////
// Tøída jednotky na serveru hry. Obsahuje všechny informace o jednotce, 
//		které potøebuje GameServer. Každá jednotka na mapì (vojáèek, budova, 
//		atp.) je reprezentována svoji vlastní instancí této tøídy.
class CSUnit : public CPersistentObject 
{
	friend class CSMap;
	friend class CSSkillType;
	friend class CSUnitType;

	DECLARE_DYNAMIC ( CSUnit );

// Datové typy
public:
	// výèet pøíznakù zmìn jednotky
	enum // EModificationFlag
	{
		MF_Clear = 0x00,
		MF_PositionChanged = 0x01,
		MF_CheckPointInfoModified = 0x02,
		MF_BriefInfoModified = 0x04,
		MF_FullInfoModified = 0x08,	// obsahuje check point a brief info jednotky
		MF_CommonClientInfoModified = MF_CheckPointInfoModified | MF_BriefInfoModified
	};

// Metody
protected:
// Konstrukce a destrukce

	// konstruktor
	CSUnit ();
	// destruktor
	~CSUnit ();

// Inicializace a znièení dat objektu

	// pøedvytvoøí jednotku typu "pUnitType"
	void PreCreate ( CSUnitType *pUnitType );
	// vytvoøí jednotku typu "pUnitType" (typ jednotky musí být zamèen pro zápis)
	void Create ( CSUnitType *pUnitType );
	// znièí data jednotky
	void Delete ();
	// vyplní objekt neškodnými daty (znièí èásteènì inicializavaná data)
	void SetEmptyData ();

	// vygeneruje a zaregistruje nové ID jednotky
	void GenerateID ();
	// zaregistruje ID jednotky
	void RegisterID ();
	// odregistruje ID jednotky
	void UnregisterID ();

private:
	// zpracuje zabití jednotky (lze volat pouze z MainLoop threadu, jednotka musí být 
	//		zamèena pro zápis, MapSquary musí být zamèeny)
	void UnitDied ();
public:
	// informuje jednotku, že jednotka na civilizaci ztratila poslední odkaz na sebe
	void ZUnitRefCountIsZero ();

protected:
// Ukládání dat (CPersistentObject metody)

	// ukládání dat
	void PersistentSave ( CPersistentStorage &storage );
	// nahrávání pouze uložených dat s "dwCivilizationCount" civilizacemi
	void PersistentLoad ( CPersistentStorage &storage, DWORD dwCivilizationCount );
	// pøeklad ukazatelù
	void PersistentTranslatePointers ( CPersistentStorage &storage );
	// inicializace nahraného objektu
	void PersistentInit ( BOOL bDeadDeletedUnit = FALSE );
	// inicializace seznamu vidìných jednotek
	void PersistentInitSeenUnitList ();
private:
	// zakázaná metoda pøedka - nahrávání pouze uložených dat
	void PersistentLoad ( CPersistentStorage &storage ) { ASSERT ( FALSE ); };

public:
// Odkazy na instanci jednotky

	// zaznamená uschování odkazu na jednotku
	inline void AddRef ();
	// zaznamená uvolnìní odkazu na jednotku (ukazatel na jednotku mùže být dále neplatný)
	void Release ();

protected:
// Info struktury jednotky

	// vyplní brief info jednotky (mimo MainLoop thread musí být jednotka zamèena pro ètení)
	inline void GetBriefInfo ( struct SUnitBriefInfo *pBriefInfo );
	// vyplní check point info jednotky (mimo MainLoop thread musí být jednotka zamèena 
	//		pro ètení)
	inline void GetCheckPointInfo ( struct SUnitCheckPointInfo *pCheckPointInfo );
	// vyplní start info jednotky (mimo MainLoop thread musí být jednotka zamèena pro ètení)
	inline void GetStartInfo ( struct SUnitStartInfo *pStartInfo );
	// pošle full info jednotky klientovi "pClient" (jednotka musí být zamèena pro ètení)
	void SendFullInfo ( struct SUnitFullInfo *pFullInfo, CZClientUnitInfoSender *pClient );
	// pošle enemy full info jednotky klientovi "pClient" (jednotka musí být zamèena 
	//		pro ètení)
	void SendEnemyFullInfo ( struct SUnitEnemyFullInfo *pEnemyFullInfo, CZClientUnitInfoSender *pClient );
	// pošle start full info jednotky klientovi "pClient" (jednotka musí být zamèena 
	//		pro ètení)
	void SendStartFullInfo ( struct SUnitStartFullInfo *pStartFullInfo, CZClientUnitInfoSender *pClient );
	// pošle start enemy full info jednotky klientovi "pClient" (jednotka musí být zamèena 
	//		pro ètení)
	void SendStartEnemyFullInfo ( struct SUnitStartEnemyFullInfo *pStartEnemyFullInfo, CZClientUnitInfoSender *pClient );

public:
// Operace se zámkem jednotky

	// zamkne zámek pro ètenáøe (s timeoutem "dwTimeOut" v milisekundách, INFINITE=èeká 
	//		dokud se zámek neuvolní) (FALSE=timeout)
	BOOL ReaderLock ( DWORD dwTimeOut = INFINITE ) 
		{ return m_pLock->ReaderLock ( dwTimeOut ); };
	// odemkne zámek pro ètenáøe
	void ReaderUnlock () { m_pLock->ReaderUnlock (); };
	// zamkne zámek pro zapisovatele (s timeoutem "dwTimeOut" v milisekundách, 
	//		INFINITE=èeká dokud se zámek neuvolní - skuteèná doba timeoutu mùže být
	//		až dvojnásobná) (FALSE=timeout)
	BOOL WriterLock ( DWORD dwTimeOut = INFINITE ) 
		{ return m_pLock->WriterLock ( dwTimeOut ); };
	// odemkne zámek pro zapisovatele
	void WriterUnlock () { m_pLock->WriterUnlock (); };
	// zamkne zámek pro ètenáøe z privilegovaného threadu (s timeoutem "dwTimeOut" 
	//		v milisekundách, INFINITE=èeká dokud se zámek neuvolní) (FALSE=timeout)
	BOOL PrivilegedThreadReaderLock ( DWORD dwTimeOut = INFINITE ) 
		{ return m_pLock->PrivilegedThreadReaderLock ( dwTimeOut ); };
	// odemkne zámek pro ètenáøe z privilegovaného threadu
	void PrivilegedThreadReaderUnlock () { m_pLock->PrivilegedThreadReaderUnlock (); };
	// zamkne zámek pro zapisovatele z privilegovaného threadu (s timeoutem "dwTimeOut" 
	//		v milisekundách, INFINITE=èeká dokud se zámek neuvolní - skuteèná doba timeoutu 
	//		mùže být až dvojnásobná) (FALSE=timeout)
	BOOL PrivilegedThreadWriterLock ( DWORD dwTimeOut = INFINITE ) 
		{ return m_pLock->PrivilegedThreadWriterLock ( dwTimeOut ); };
	// odemkne zámek pro zapisovatele z privilegovaného threadu
	void PrivilegedThreadWriterUnlock () { m_pLock->PrivilegedThreadWriterUnlock (); };

// Informace o typech skill jednotky

	// vrátí typ skilly s ID jména "pNameID" (upøednostòuje typ skilly povolený v aktuálním 
	//		módu jednotky) (NULL=typ skilly neexistuje)
	CSSkillType *GetSkillTypeByNameID ( CStringTableItem *pNameID );

// Get/Set metody serverových informací o jednotce

	// vrátí jednotku na civilizaci
	CZUnit *GetZUnit () { return m_pZUnit; };
	// vrátí ID jednotky
	DWORD GetID () { return m_dwID; };
	// vrátí ukazatel na tøídu typu jednotky
	CSUnitType *GetUnitType () 
		{ ASSERT ( m_pUnitType != NULL ); return m_pUnitType; };
	// vrátí index civilizace jednotky
	DWORD GetCivilizationIndex () { return m_dwCivilizationIndex; };
	// vrátí ukazatel na civilizaci
	inline CSCivilization *GetCivilization ();
	// vrátí ukazatel na graf pro hledání cesty v tomto módu jednotky
	CSFindPathGraph *GetFindPathGraph () 
		{ return m_pUnitType->GetFindPathGraph ( GetMode () ); };
protected:
	// zjistí, je-li skilla "dwIndex" aktivní
	BOOL IsSkillActive ( DWORD dwIndex ) { return m_cActiveSkills.GetAt ( dwIndex ); };
	// aktivuje skillu "dwIndex"
	void ActivateSkill ( DWORD dwIndex ) { m_cActiveSkills.SetAt ( dwIndex ); };
	// deaktivuje skillu "dwIndex"
	void DeactivateSkill ( DWORD dwIndex ) { m_cActiveSkills.ClearAt ( dwIndex ); };
	// vrátí ukazatel na data skillù
	BYTE *GetSkillData () { ASSERT ( m_pSkillData != NULL ); return m_pSkillData; };
	// vrátí seznam vidìných jednotek (lze volat pouze z MainLoop threadu)
	CSUnitList *GetSeenUnitList () { return &m_cSeenUnitList; };

public:
// Get/Set metody herních informací o jednotce

	// vrátí poèet životù jednotky
	int GetLives () { return m_nLives; };
	// sníží poèet životù jednotky o "nLivesDecrement" (lze volat jen z MainLoop threadu, 
	//		jednotka musí být zamèena pro zápis) (TRUE=jednotka je mrtvá)
	inline BOOL DecreaseLives ( int nLivesDecrement );
	// vrátí maximální poèet životù jednotky
	int GetLivesMax () { return m_nLivesMax; };
	// nastaví maximální poèet životù jednotky
	void SetLivesMax ( int nLivesMax ) { m_nLivesMax = nLivesMax; };
	// vrátí polomìr viditelnosti jednotky
	DWORD GetViewRadius () { return m_dwViewRadius; };
	// vrátí druhou mocninu polomìru viditelnosti jednotky
	DWORD GetViewRadiusSquare () { return m_dwViewRadiusSquare; };
	// nastaví polomìr viditelnosti jednotky
	void SetViewRadius ( DWORD dwViewRadius ) 
		{ m_dwViewRadius = dwViewRadius; m_dwViewRadiusSquare = dwViewRadius * dwViewRadius; };
	// vrátí umístìní jednotky na mapì (x=y=NO_MAP_POSITION - jednotka je mimo mapu) 
	//		(mimo MainLoop thread musí být jednotka zamèena pro ètení)
	CPointDW GetPosition () { return m_pointPosition; };
	// vrátí MapSquare umístìní jednotky na mapì (mimo MainLoop thread musí být jednotka 
	//		zamèena pro ètení) (NULL=jednotka není na mapì)
	inline CSMapSquare *GetMapSquare ();
protected:
	// umístí jednotku na mapì (jednotka musí být zamèena pro zápis)
	void SetPosition ( CPointDW pointPosition ) { m_pointPosition = pointPosition; };
public:
	// vrátí vertikální umístìní jednotky na mapì
	DWORD GetVerticalPosition () { return m_dwVerticalPosition; };
	// umístí vertikálnì jednotku na mapì
	void SetVerticalPosition ( DWORD dwVerticalPosition ) 
		{ m_dwVerticalPosition = dwVerticalPosition; };
	// vrátí nasmìrování jednotky
	BYTE GetDirection () { return m_nDirection; };
	// nastaví nasmìrování jednotky (volá SetBriefInfoModified, je-li potøeba)
	inline void SetDirection ( BYTE nDirection );
	// nastaví nasmìrování jednotky na pozici "pointPosition" (volá SetBriefInfoModified, 
	//		je-li potøeba)
	inline void SetDirection ( CPointDW pointPosition );
	// nastaví nasmìrování jednotky smìrem o "nXDifference" a "nYDifference" (volá 
	//		SetBriefInfoModified, je-li potøeba)
	inline void SetDirectionFromDifference ( int nXDifference, int nYDifference );
	// vrátí mód jednotky (0-7)
	BYTE GetMode () { ASSERT ( m_nMode < 8 ); return m_nMode; };
	// nastaví mód jednotky (0-7) (jednotka musí být zamèena pro zápis, volá 
	//		SetBriefInfoModified, je-li potøeba)
	void SetMode ( BYTE nMode );
	// vrátí masku módu jednotky
	BYTE GetModeMask () { ASSERT ( m_nMode < 8 ); return ( 0x01 << m_nMode ); };
	// nastaví masku módu jednotky (musí být nastaven právì jeden bit) (jednotka musí být 
	//		zamèena pro zápis, volá SetBriefInfoModified, je-li potøeba)
	void SetModeMask ( BYTE nModeMask );
	// vrátí šíøku pøi pohybu
	DWORD GetMoveWidth () { return m_pUnitType->GetMoveWidth ( GetMode () ); };
	// vrátí pøíznaky detekce neviditelnost
	DWORD GetInvisibilityDetection () { return m_dwInvisibilityDetection; };
	// vrátí ID vzhledu jednotky
	DWORD GetAppearanceID () { return m_dwAppearanceID; };
	// nastaví ID vzhledu jednotky (0=defaultní vzhled jednotky) (volá SetBriefInfoModified, 
	//		je-li potøeba)
	inline void SetAppearanceID ( DWORD dwAppearanceID );
	// nastaví ID defaultního vzhledu jednotky pro aktuální mód jednotky (volá 
	//		SetBriefInfoModified, je-li potøeba)
	inline void SetDefaultAppearanceID ();
protected:
	// vrátí pøíznak, je-li jednotka vidìna jednotkou s "dwInvisibilityDetection"
	BOOL IsVisible ( DWORD dwInvisibilityDetection ) 
		{ return ( dwInvisibilityDetection & m_dwInvisibilityState ) == m_dwInvisibilityState; };
public:
	// vrátí první kontrolní bod cesty jednotky (jednotka musí být zamèena pro ètení)
	CSCheckPoint &GetFirstCheckPoint () { return m_cFirstCheckPoint; };
	// nastaví první kontrolní bod cesty jednotky (jednotka musí být zamèena pro zápis)
	void SetFirstCheckPoint ( CPointDW pointPosition, DWORD dwTime ) 
		{ m_cFirstCheckPoint.Set ( pointPosition, dwTime ); };
	// vrátí druhý kontrolní bod cesty jednotky (jednotka musí být zamèena pro ètení)
	CSCheckPoint &GetSecondCheckPoint () { return m_cSecondCheckPoint; };
	// nastaví druhý kontrolní bod cesty jednotky (jednotka musí být zamèena pro zápis)
	void SetSecondCheckPoint ( CPointDW pointPosition, DWORD dwTime ) 
		{ m_cSecondCheckPoint.Set ( pointPosition, dwTime ); };
	// vyplní naposledy vidìnou pozici "cPosition" jednotky civilizací 
	//		"dwCivilizationIndex" (mimo MainLoop thread musí být jednotka zamèena pro ètení)
	inline void GetLastSeenPosition ( DWORD dwCivilizationIndex, CSPosition &cPosition );
	// vrátí pøíznak, je-li jednotka vidìt civilizací "dwCivilizationIndex"
	BOOL IsVisibleByCivilization ( DWORD dwCivilizationIndex ) 
		{ return ( m_aCivilizationVisibility[dwCivilizationIndex] > 0 ); };

protected:
// Get/Set metody pøíznakù jednotky

	// vrátí pøíznaky zmìn jednotky
	DWORD GetModificationFlags () { return m_dwModificationFlags; };
	// vynuluje pøíznaky zmìn jednotky
	void ClearModificationFlags () { m_dwModificationFlags = MF_Clear; };

	// vrátí pøíznak zmìny pozice jednotky
	BOOL IsPositionChanged () 
		{ return m_dwModificationFlags & MF_PositionChanged; };
	// smaže pøíznak zmìny pozice jednotky
	void ClearPositionChanged () 
		{ m_dwModificationFlags &= ~(BYTE)MF_PositionChanged; };
	// nastaví pøíznak zmìny pozice jednotky
	void SetPositionChanged () 
		{ m_dwModificationFlags |= MF_PositionChanged; };

	// vrátí pøíznak zmìny bìžných informací jednotky pro klienta
	BOOL IsCommonClientInfoModified () 
		{ return m_dwModificationFlags & MF_CommonClientInfoModified; };
	// vrátí pøíznak zmìny check point infa jednotky
	BOOL IsCheckPointInfoModified () 
		{ return m_dwModificationFlags & MF_CheckPointInfoModified; };
	// smaže pøíznak zmìny check point infa jednotky
	void ClearCheckPointInfoModified () 
		{ m_dwModificationFlags &= ~(BYTE)MF_CheckPointInfoModified; };
	// vrátí pøíznak zmìny brief infa jednotky
	BOOL IsBriefInfoModified () 
		{ return m_dwModificationFlags & MF_BriefInfoModified; };
	// smaže pøíznak zmìny brief infa jednotky
	void ClearBriefInfoModified () 
		{ m_dwModificationFlags &= ~(BYTE)MF_BriefInfoModified; };
	// vrátí pøíznak zmìny full infa jednotky
	BOOL IsFullInfoModified () 
		{ return m_dwModificationFlags & MF_FullInfoModified; };
	// smaže pøíznak zmìny full infa jednotky
	void ClearFullInfoModified () 
		{ m_dwModificationFlags &= ~(BYTE)MF_FullInfoModified; };

public:
	// nastaví pøíznak zmìny check point infa jednotky
	void SetCheckPointInfoModified () 
		{ m_dwModificationFlags |= MF_CheckPointInfoModified | MF_FullInfoModified; };
	// nastaví pøíznak zmìny brief infa jednotky
	void SetBriefInfoModified () 
		{ m_dwModificationFlags |= MF_BriefInfoModified | MF_CheckPointInfoModified | MF_FullInfoModified; };
	// nastaví pøíznak zmìny full infa jednotky
	void SetFullInfoModified () 
		{ m_dwModificationFlags |= MF_FullInfoModified; };

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
// Herní informace o jednotce

	// aktuální poèet životù (nìjaká abstraktní velièina) (kvùli ukazování pravítka 
	//		s životem na GameClientovi)
	int m_nLives;
	// maximální poèet životù (nìjaká abstraktní velièina) (kvùli ukazování pravítka 
	//		s životem na GameClientovi)
	int m_nLivesMax;
	// polomìr viditelnosti (v MapCellech)
	DWORD m_dwViewRadius;
	// druhá mocnina polomìru viditelnosti (v MapCellech)
	DWORD m_dwViewRadiusSquare;
	// umístìní jednotky na mapì
	CPointDW m_pointPosition;
	// vertikální umístìní jednotky v UnitLevelu
	DWORD m_dwVerticalPosition;
	// jméno jednotky (kvùli zobrazování na GameClientovi)
	CString m_strUnitName;
	// nasmìrování jednotky (kvùli zobrazování na GameClientovi)
	BYTE m_nDirection;
	// aktuální mód jednotky
	BYTE m_nMode;
	// bitová maska pøíznakù neviditelnosti jednotky (tj. jak je jednotka neviditelná)
	DWORD m_dwInvisibilityState;
	// bitová maska pøíznakù detekce neviditelnosti ostatních jednotek (tj. které 
	//		neviditelnosti jednotka vidí)
	DWORD m_dwInvisibilityDetection;
	// ID vzhledu jednotky
	DWORD m_dwAppearanceID;

	// první kontrolní bod cesty jednotky
	CSCheckPoint m_cFirstCheckPoint;
	// druhý kontrolní bod cesty jednotky
	CSCheckPoint m_cSecondCheckPoint;

// Serverové informace o jednotce

	// jednotka na civilizaci
	CZUnit *m_pZUnit;
	// identifikaèní èíslo jednotky
	DWORD m_dwID;
	// další pøidìlované ID jednotky
	static DWORD m_dwNextGeneratedID;
	// ukazatel na zámek jednotky s privilegovaným threadem (zámek je spoleèný více 
	//		jednotkám) (inicializuje mapa bìhem zaøazování jednotky do blokù)
	CPrivilegedThreadReadWriteLock *m_pLock;
	// ukazatel na typ jednotky (každý druh vojáèka, budovy má v každé civilizaci 
	//		svùj vlastní typ jednotky)
	CSUnitType *m_pUnitType;
	// èíslo civilizace jednotky - získává se z typu jednotky
	DWORD m_dwCivilizationIndex;

	// pøíznaky zmìn dat jednotky (bitové masky = výètový datový typ jednotky)
	DWORD m_dwModificationFlags;

	// pole pøíznakù active/deactive skillù jednotky
	CBitArray m_cActiveSkills;
	// data skillù jednotky
	BYTE *m_pSkillData;

	// zámek memory poolù polí jednotky
	CMutex m_mutexCivilizationArrayMemoryPoolLock;
	// pole poètu jednotek jednotlivých civilizací, které jednotku vidí
	WORD *m_aCivilizationVisibility;
	// memory pool polí poètu jednotek civilizací vidících jednotku
	static CMemoryPool m_cCivilizationVisibilityArrayMemoryPool;
	// pole posledních pozic, kde byla jednotka civilizacemi vidìna
	CSPosition *m_aCivilizationLastSeenPosition;
	// memory pool polí posledních pozic, kde byla jednotka civilizacemi vidìna
	static CMemoryPool m_cCivilizationLastSeenPositionArrayMemoryPool;
	// pøíznaky zmìny viditelnosti jednotky civilizacemi
	CSCivilizationBitArray m_cCivilizationVisibilityChanged;
	// pøíznaky zmìny pozice jednotky z/do sledovaných MapSquarù
	CSCivilizationBitArray m_cWatchedMapSquareChanged;
	// poèet klientù civilizací sledujících jednotku
	DWORD m_dwWatchingCivilizationClientCount;

	// seznam vidìných jednotek
	CSUnitList m_cSeenUnitList;

// Serverová data

	// èítaè referencí jednotky
	DWORD m_dwReferenceCounter;

	// tabulka smìrù kladné poloroviny osy X
	static BYTE m_aDirectionTableXPositive[5];
	// tabulka smìrù záporné poloroviny osy X
	static BYTE m_aDirectionTableXNegative[5];

// Informace typu jednotky

	// ukazatel na další jednotku seznamu jednotek typu jednotky (typ jednotky musí být 
	//		zamèen pro ètení/zápis, jednotka ne)
	CSUnit *m_pNextUnit;
};

#endif //__SERVER_UNIT__HEADER_INCLUDED__
