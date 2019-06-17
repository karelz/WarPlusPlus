/***********************************************************
 * 
 * Projekt: Strategick� hra
 *    ��st: Server hry
 *   Autor: Karel Zikmund
 * 
 *   Popis: T��da jednotky
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
// Dop�edn� deklarace

// t��da MapSquaru na map�
class CSMapSquare;
// t��da typu skilly
class CSSkillType;
// t��da jednotky na civilizaci
class CZUnit;
// t��da rozes�latele informac� o jednotk�ch klientovi civilizace
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
// T��da jednotky na serveru hry. Obsahuje v�echny informace o jednotce, 
//		kter� pot�ebuje GameServer. Ka�d� jednotka na map� (voj��ek, budova, 
//		atp.) je reprezentov�na svoji vlastn� instanc� t�to t��dy.
class CSUnit : public CPersistentObject 
{
	friend class CSMap;
	friend class CSSkillType;
	friend class CSUnitType;

	DECLARE_DYNAMIC ( CSUnit );

// Datov� typy
public:
	// v��et p��znak� zm�n jednotky
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

// Inicializace a zni�en� dat objektu

	// p�edvytvo�� jednotku typu "pUnitType"
	void PreCreate ( CSUnitType *pUnitType );
	// vytvo�� jednotku typu "pUnitType" (typ jednotky mus� b�t zam�en pro z�pis)
	void Create ( CSUnitType *pUnitType );
	// zni�� data jednotky
	void Delete ();
	// vypln� objekt ne�kodn�mi daty (zni�� ��ste�n� inicializavan� data)
	void SetEmptyData ();

	// vygeneruje a zaregistruje nov� ID jednotky
	void GenerateID ();
	// zaregistruje ID jednotky
	void RegisterID ();
	// odregistruje ID jednotky
	void UnregisterID ();

private:
	// zpracuje zabit� jednotky (lze volat pouze z MainLoop threadu, jednotka mus� b�t 
	//		zam�ena pro z�pis, MapSquary mus� b�t zam�eny)
	void UnitDied ();
public:
	// informuje jednotku, �e jednotka na civilizaci ztratila posledn� odkaz na sebe
	void ZUnitRefCountIsZero ();

protected:
// Ukl�d�n� dat (CPersistentObject metody)

	// ukl�d�n� dat
	void PersistentSave ( CPersistentStorage &storage );
	// nahr�v�n� pouze ulo�en�ch dat s "dwCivilizationCount" civilizacemi
	void PersistentLoad ( CPersistentStorage &storage, DWORD dwCivilizationCount );
	// p�eklad ukazatel�
	void PersistentTranslatePointers ( CPersistentStorage &storage );
	// inicializace nahran�ho objektu
	void PersistentInit ( BOOL bDeadDeletedUnit = FALSE );
	// inicializace seznamu vid�n�ch jednotek
	void PersistentInitSeenUnitList ();
private:
	// zak�zan� metoda p�edka - nahr�v�n� pouze ulo�en�ch dat
	void PersistentLoad ( CPersistentStorage &storage ) { ASSERT ( FALSE ); };

public:
// Odkazy na instanci jednotky

	// zaznamen� uschov�n� odkazu na jednotku
	inline void AddRef ();
	// zaznamen� uvoln�n� odkazu na jednotku (ukazatel na jednotku m��e b�t d�le neplatn�)
	void Release ();

protected:
// Info struktury jednotky

	// vypln� brief info jednotky (mimo MainLoop thread mus� b�t jednotka zam�ena pro �ten�)
	inline void GetBriefInfo ( struct SUnitBriefInfo *pBriefInfo );
	// vypln� check point info jednotky (mimo MainLoop thread mus� b�t jednotka zam�ena 
	//		pro �ten�)
	inline void GetCheckPointInfo ( struct SUnitCheckPointInfo *pCheckPointInfo );
	// vypln� start info jednotky (mimo MainLoop thread mus� b�t jednotka zam�ena pro �ten�)
	inline void GetStartInfo ( struct SUnitStartInfo *pStartInfo );
	// po�le full info jednotky klientovi "pClient" (jednotka mus� b�t zam�ena pro �ten�)
	void SendFullInfo ( struct SUnitFullInfo *pFullInfo, CZClientUnitInfoSender *pClient );
	// po�le enemy full info jednotky klientovi "pClient" (jednotka mus� b�t zam�ena 
	//		pro �ten�)
	void SendEnemyFullInfo ( struct SUnitEnemyFullInfo *pEnemyFullInfo, CZClientUnitInfoSender *pClient );
	// po�le start full info jednotky klientovi "pClient" (jednotka mus� b�t zam�ena 
	//		pro �ten�)
	void SendStartFullInfo ( struct SUnitStartFullInfo *pStartFullInfo, CZClientUnitInfoSender *pClient );
	// po�le start enemy full info jednotky klientovi "pClient" (jednotka mus� b�t zam�ena 
	//		pro �ten�)
	void SendStartEnemyFullInfo ( struct SUnitStartEnemyFullInfo *pStartEnemyFullInfo, CZClientUnitInfoSender *pClient );

public:
// Operace se z�mkem jednotky

	// zamkne z�mek pro �ten��e (s timeoutem "dwTimeOut" v milisekund�ch, INFINITE=�ek� 
	//		dokud se z�mek neuvoln�) (FALSE=timeout)
	BOOL ReaderLock ( DWORD dwTimeOut = INFINITE ) 
		{ return m_pLock->ReaderLock ( dwTimeOut ); };
	// odemkne z�mek pro �ten��e
	void ReaderUnlock () { m_pLock->ReaderUnlock (); };
	// zamkne z�mek pro zapisovatele (s timeoutem "dwTimeOut" v milisekund�ch, 
	//		INFINITE=�ek� dokud se z�mek neuvoln� - skute�n� doba timeoutu m��e b�t
	//		a� dvojn�sobn�) (FALSE=timeout)
	BOOL WriterLock ( DWORD dwTimeOut = INFINITE ) 
		{ return m_pLock->WriterLock ( dwTimeOut ); };
	// odemkne z�mek pro zapisovatele
	void WriterUnlock () { m_pLock->WriterUnlock (); };
	// zamkne z�mek pro �ten��e z privilegovan�ho threadu (s timeoutem "dwTimeOut" 
	//		v milisekund�ch, INFINITE=�ek� dokud se z�mek neuvoln�) (FALSE=timeout)
	BOOL PrivilegedThreadReaderLock ( DWORD dwTimeOut = INFINITE ) 
		{ return m_pLock->PrivilegedThreadReaderLock ( dwTimeOut ); };
	// odemkne z�mek pro �ten��e z privilegovan�ho threadu
	void PrivilegedThreadReaderUnlock () { m_pLock->PrivilegedThreadReaderUnlock (); };
	// zamkne z�mek pro zapisovatele z privilegovan�ho threadu (s timeoutem "dwTimeOut" 
	//		v milisekund�ch, INFINITE=�ek� dokud se z�mek neuvoln� - skute�n� doba timeoutu 
	//		m��e b�t a� dvojn�sobn�) (FALSE=timeout)
	BOOL PrivilegedThreadWriterLock ( DWORD dwTimeOut = INFINITE ) 
		{ return m_pLock->PrivilegedThreadWriterLock ( dwTimeOut ); };
	// odemkne z�mek pro zapisovatele z privilegovan�ho threadu
	void PrivilegedThreadWriterUnlock () { m_pLock->PrivilegedThreadWriterUnlock (); };

// Informace o typech skill jednotky

	// vr�t� typ skilly s ID jm�na "pNameID" (up�ednost�uje typ skilly povolen� v aktu�ln�m 
	//		m�du jednotky) (NULL=typ skilly neexistuje)
	CSSkillType *GetSkillTypeByNameID ( CStringTableItem *pNameID );

// Get/Set metody serverov�ch informac� o jednotce

	// vr�t� jednotku na civilizaci
	CZUnit *GetZUnit () { return m_pZUnit; };
	// vr�t� ID jednotky
	DWORD GetID () { return m_dwID; };
	// vr�t� ukazatel na t��du typu jednotky
	CSUnitType *GetUnitType () 
		{ ASSERT ( m_pUnitType != NULL ); return m_pUnitType; };
	// vr�t� index civilizace jednotky
	DWORD GetCivilizationIndex () { return m_dwCivilizationIndex; };
	// vr�t� ukazatel na civilizaci
	inline CSCivilization *GetCivilization ();
	// vr�t� ukazatel na graf pro hled�n� cesty v tomto m�du jednotky
	CSFindPathGraph *GetFindPathGraph () 
		{ return m_pUnitType->GetFindPathGraph ( GetMode () ); };
protected:
	// zjist�, je-li skilla "dwIndex" aktivn�
	BOOL IsSkillActive ( DWORD dwIndex ) { return m_cActiveSkills.GetAt ( dwIndex ); };
	// aktivuje skillu "dwIndex"
	void ActivateSkill ( DWORD dwIndex ) { m_cActiveSkills.SetAt ( dwIndex ); };
	// deaktivuje skillu "dwIndex"
	void DeactivateSkill ( DWORD dwIndex ) { m_cActiveSkills.ClearAt ( dwIndex ); };
	// vr�t� ukazatel na data skill�
	BYTE *GetSkillData () { ASSERT ( m_pSkillData != NULL ); return m_pSkillData; };
	// vr�t� seznam vid�n�ch jednotek (lze volat pouze z MainLoop threadu)
	CSUnitList *GetSeenUnitList () { return &m_cSeenUnitList; };

public:
// Get/Set metody hern�ch informac� o jednotce

	// vr�t� po�et �ivot� jednotky
	int GetLives () { return m_nLives; };
	// sn�� po�et �ivot� jednotky o "nLivesDecrement" (lze volat jen z MainLoop threadu, 
	//		jednotka mus� b�t zam�ena pro z�pis) (TRUE=jednotka je mrtv�)
	inline BOOL DecreaseLives ( int nLivesDecrement );
	// vr�t� maxim�ln� po�et �ivot� jednotky
	int GetLivesMax () { return m_nLivesMax; };
	// nastav� maxim�ln� po�et �ivot� jednotky
	void SetLivesMax ( int nLivesMax ) { m_nLivesMax = nLivesMax; };
	// vr�t� polom�r viditelnosti jednotky
	DWORD GetViewRadius () { return m_dwViewRadius; };
	// vr�t� druhou mocninu polom�ru viditelnosti jednotky
	DWORD GetViewRadiusSquare () { return m_dwViewRadiusSquare; };
	// nastav� polom�r viditelnosti jednotky
	void SetViewRadius ( DWORD dwViewRadius ) 
		{ m_dwViewRadius = dwViewRadius; m_dwViewRadiusSquare = dwViewRadius * dwViewRadius; };
	// vr�t� um�st�n� jednotky na map� (x=y=NO_MAP_POSITION - jednotka je mimo mapu) 
	//		(mimo MainLoop thread mus� b�t jednotka zam�ena pro �ten�)
	CPointDW GetPosition () { return m_pointPosition; };
	// vr�t� MapSquare um�st�n� jednotky na map� (mimo MainLoop thread mus� b�t jednotka 
	//		zam�ena pro �ten�) (NULL=jednotka nen� na map�)
	inline CSMapSquare *GetMapSquare ();
protected:
	// um�st� jednotku na map� (jednotka mus� b�t zam�ena pro z�pis)
	void SetPosition ( CPointDW pointPosition ) { m_pointPosition = pointPosition; };
public:
	// vr�t� vertik�ln� um�st�n� jednotky na map�
	DWORD GetVerticalPosition () { return m_dwVerticalPosition; };
	// um�st� vertik�ln� jednotku na map�
	void SetVerticalPosition ( DWORD dwVerticalPosition ) 
		{ m_dwVerticalPosition = dwVerticalPosition; };
	// vr�t� nasm�rov�n� jednotky
	BYTE GetDirection () { return m_nDirection; };
	// nastav� nasm�rov�n� jednotky (vol� SetBriefInfoModified, je-li pot�eba)
	inline void SetDirection ( BYTE nDirection );
	// nastav� nasm�rov�n� jednotky na pozici "pointPosition" (vol� SetBriefInfoModified, 
	//		je-li pot�eba)
	inline void SetDirection ( CPointDW pointPosition );
	// nastav� nasm�rov�n� jednotky sm�rem o "nXDifference" a "nYDifference" (vol� 
	//		SetBriefInfoModified, je-li pot�eba)
	inline void SetDirectionFromDifference ( int nXDifference, int nYDifference );
	// vr�t� m�d jednotky (0-7)
	BYTE GetMode () { ASSERT ( m_nMode < 8 ); return m_nMode; };
	// nastav� m�d jednotky (0-7) (jednotka mus� b�t zam�ena pro z�pis, vol� 
	//		SetBriefInfoModified, je-li pot�eba)
	void SetMode ( BYTE nMode );
	// vr�t� masku m�du jednotky
	BYTE GetModeMask () { ASSERT ( m_nMode < 8 ); return ( 0x01 << m_nMode ); };
	// nastav� masku m�du jednotky (mus� b�t nastaven pr�v� jeden bit) (jednotka mus� b�t 
	//		zam�ena pro z�pis, vol� SetBriefInfoModified, je-li pot�eba)
	void SetModeMask ( BYTE nModeMask );
	// vr�t� ���ku p�i pohybu
	DWORD GetMoveWidth () { return m_pUnitType->GetMoveWidth ( GetMode () ); };
	// vr�t� p��znaky detekce neviditelnost
	DWORD GetInvisibilityDetection () { return m_dwInvisibilityDetection; };
	// vr�t� ID vzhledu jednotky
	DWORD GetAppearanceID () { return m_dwAppearanceID; };
	// nastav� ID vzhledu jednotky (0=defaultn� vzhled jednotky) (vol� SetBriefInfoModified, 
	//		je-li pot�eba)
	inline void SetAppearanceID ( DWORD dwAppearanceID );
	// nastav� ID defaultn�ho vzhledu jednotky pro aktu�ln� m�d jednotky (vol� 
	//		SetBriefInfoModified, je-li pot�eba)
	inline void SetDefaultAppearanceID ();
protected:
	// vr�t� p��znak, je-li jednotka vid�na jednotkou s "dwInvisibilityDetection"
	BOOL IsVisible ( DWORD dwInvisibilityDetection ) 
		{ return ( dwInvisibilityDetection & m_dwInvisibilityState ) == m_dwInvisibilityState; };
public:
	// vr�t� prvn� kontroln� bod cesty jednotky (jednotka mus� b�t zam�ena pro �ten�)
	CSCheckPoint &GetFirstCheckPoint () { return m_cFirstCheckPoint; };
	// nastav� prvn� kontroln� bod cesty jednotky (jednotka mus� b�t zam�ena pro z�pis)
	void SetFirstCheckPoint ( CPointDW pointPosition, DWORD dwTime ) 
		{ m_cFirstCheckPoint.Set ( pointPosition, dwTime ); };
	// vr�t� druh� kontroln� bod cesty jednotky (jednotka mus� b�t zam�ena pro �ten�)
	CSCheckPoint &GetSecondCheckPoint () { return m_cSecondCheckPoint; };
	// nastav� druh� kontroln� bod cesty jednotky (jednotka mus� b�t zam�ena pro z�pis)
	void SetSecondCheckPoint ( CPointDW pointPosition, DWORD dwTime ) 
		{ m_cSecondCheckPoint.Set ( pointPosition, dwTime ); };
	// vypln� naposledy vid�nou pozici "cPosition" jednotky civilizac� 
	//		"dwCivilizationIndex" (mimo MainLoop thread mus� b�t jednotka zam�ena pro �ten�)
	inline void GetLastSeenPosition ( DWORD dwCivilizationIndex, CSPosition &cPosition );
	// vr�t� p��znak, je-li jednotka vid�t civilizac� "dwCivilizationIndex"
	BOOL IsVisibleByCivilization ( DWORD dwCivilizationIndex ) 
		{ return ( m_aCivilizationVisibility[dwCivilizationIndex] > 0 ); };

protected:
// Get/Set metody p��znak� jednotky

	// vr�t� p��znaky zm�n jednotky
	DWORD GetModificationFlags () { return m_dwModificationFlags; };
	// vynuluje p��znaky zm�n jednotky
	void ClearModificationFlags () { m_dwModificationFlags = MF_Clear; };

	// vr�t� p��znak zm�ny pozice jednotky
	BOOL IsPositionChanged () 
		{ return m_dwModificationFlags & MF_PositionChanged; };
	// sma�e p��znak zm�ny pozice jednotky
	void ClearPositionChanged () 
		{ m_dwModificationFlags &= ~(BYTE)MF_PositionChanged; };
	// nastav� p��znak zm�ny pozice jednotky
	void SetPositionChanged () 
		{ m_dwModificationFlags |= MF_PositionChanged; };

	// vr�t� p��znak zm�ny b�n�ch informac� jednotky pro klienta
	BOOL IsCommonClientInfoModified () 
		{ return m_dwModificationFlags & MF_CommonClientInfoModified; };
	// vr�t� p��znak zm�ny check point infa jednotky
	BOOL IsCheckPointInfoModified () 
		{ return m_dwModificationFlags & MF_CheckPointInfoModified; };
	// sma�e p��znak zm�ny check point infa jednotky
	void ClearCheckPointInfoModified () 
		{ m_dwModificationFlags &= ~(BYTE)MF_CheckPointInfoModified; };
	// vr�t� p��znak zm�ny brief infa jednotky
	BOOL IsBriefInfoModified () 
		{ return m_dwModificationFlags & MF_BriefInfoModified; };
	// sma�e p��znak zm�ny brief infa jednotky
	void ClearBriefInfoModified () 
		{ m_dwModificationFlags &= ~(BYTE)MF_BriefInfoModified; };
	// vr�t� p��znak zm�ny full infa jednotky
	BOOL IsFullInfoModified () 
		{ return m_dwModificationFlags & MF_FullInfoModified; };
	// sma�e p��znak zm�ny full infa jednotky
	void ClearFullInfoModified () 
		{ m_dwModificationFlags &= ~(BYTE)MF_FullInfoModified; };

public:
	// nastav� p��znak zm�ny check point infa jednotky
	void SetCheckPointInfoModified () 
		{ m_dwModificationFlags |= MF_CheckPointInfoModified | MF_FullInfoModified; };
	// nastav� p��znak zm�ny brief infa jednotky
	void SetBriefInfoModified () 
		{ m_dwModificationFlags |= MF_BriefInfoModified | MF_CheckPointInfoModified | MF_FullInfoModified; };
	// nastav� p��znak zm�ny full infa jednotky
	void SetFullInfoModified () 
		{ m_dwModificationFlags |= MF_FullInfoModified; };

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
// Hern� informace o jednotce

	// aktu�ln� po�et �ivot� (n�jak� abstraktn� veli�ina) (kv�li ukazov�n� prav�tka 
	//		s �ivotem na GameClientovi)
	int m_nLives;
	// maxim�ln� po�et �ivot� (n�jak� abstraktn� veli�ina) (kv�li ukazov�n� prav�tka 
	//		s �ivotem na GameClientovi)
	int m_nLivesMax;
	// polom�r viditelnosti (v MapCellech)
	DWORD m_dwViewRadius;
	// druh� mocnina polom�ru viditelnosti (v MapCellech)
	DWORD m_dwViewRadiusSquare;
	// um�st�n� jednotky na map�
	CPointDW m_pointPosition;
	// vertik�ln� um�st�n� jednotky v UnitLevelu
	DWORD m_dwVerticalPosition;
	// jm�no jednotky (kv�li zobrazov�n� na GameClientovi)
	CString m_strUnitName;
	// nasm�rov�n� jednotky (kv�li zobrazov�n� na GameClientovi)
	BYTE m_nDirection;
	// aktu�ln� m�d jednotky
	BYTE m_nMode;
	// bitov� maska p��znak� neviditelnosti jednotky (tj. jak je jednotka neviditeln�)
	DWORD m_dwInvisibilityState;
	// bitov� maska p��znak� detekce neviditelnosti ostatn�ch jednotek (tj. kter� 
	//		neviditelnosti jednotka vid�)
	DWORD m_dwInvisibilityDetection;
	// ID vzhledu jednotky
	DWORD m_dwAppearanceID;

	// prvn� kontroln� bod cesty jednotky
	CSCheckPoint m_cFirstCheckPoint;
	// druh� kontroln� bod cesty jednotky
	CSCheckPoint m_cSecondCheckPoint;

// Serverov� informace o jednotce

	// jednotka na civilizaci
	CZUnit *m_pZUnit;
	// identifika�n� ��slo jednotky
	DWORD m_dwID;
	// dal�� p�id�lovan� ID jednotky
	static DWORD m_dwNextGeneratedID;
	// ukazatel na z�mek jednotky s privilegovan�m threadem (z�mek je spole�n� v�ce 
	//		jednotk�m) (inicializuje mapa b�hem za�azov�n� jednotky do blok�)
	CPrivilegedThreadReadWriteLock *m_pLock;
	// ukazatel na typ jednotky (ka�d� druh voj��ka, budovy m� v ka�d� civilizaci 
	//		sv�j vlastn� typ jednotky)
	CSUnitType *m_pUnitType;
	// ��slo civilizace jednotky - z�sk�v� se z typu jednotky
	DWORD m_dwCivilizationIndex;

	// p��znaky zm�n dat jednotky (bitov� masky = v��tov� datov� typ jednotky)
	DWORD m_dwModificationFlags;

	// pole p��znak� active/deactive skill� jednotky
	CBitArray m_cActiveSkills;
	// data skill� jednotky
	BYTE *m_pSkillData;

	// z�mek memory pool� pol� jednotky
	CMutex m_mutexCivilizationArrayMemoryPoolLock;
	// pole po�tu jednotek jednotliv�ch civilizac�, kter� jednotku vid�
	WORD *m_aCivilizationVisibility;
	// memory pool pol� po�tu jednotek civilizac� vid�c�ch jednotku
	static CMemoryPool m_cCivilizationVisibilityArrayMemoryPool;
	// pole posledn�ch pozic, kde byla jednotka civilizacemi vid�na
	CSPosition *m_aCivilizationLastSeenPosition;
	// memory pool pol� posledn�ch pozic, kde byla jednotka civilizacemi vid�na
	static CMemoryPool m_cCivilizationLastSeenPositionArrayMemoryPool;
	// p��znaky zm�ny viditelnosti jednotky civilizacemi
	CSCivilizationBitArray m_cCivilizationVisibilityChanged;
	// p��znaky zm�ny pozice jednotky z/do sledovan�ch MapSquar�
	CSCivilizationBitArray m_cWatchedMapSquareChanged;
	// po�et klient� civilizac� sleduj�c�ch jednotku
	DWORD m_dwWatchingCivilizationClientCount;

	// seznam vid�n�ch jednotek
	CSUnitList m_cSeenUnitList;

// Serverov� data

	// ��ta� referenc� jednotky
	DWORD m_dwReferenceCounter;

	// tabulka sm�r� kladn� poloroviny osy X
	static BYTE m_aDirectionTableXPositive[5];
	// tabulka sm�r� z�porn� poloroviny osy X
	static BYTE m_aDirectionTableXNegative[5];

// Informace typu jednotky

	// ukazatel na dal�� jednotku seznamu jednotek typu jednotky (typ jednotky mus� b�t 
	//		zam�en pro �ten�/z�pis, jednotka ne)
	CSUnit *m_pNextUnit;
};

#endif //__SERVER_UNIT__HEADER_INCLUDED__
