/***********************************************************
 * 
 * Projekt: Strategická hra
 *    Èást: Server hry
 *   Autor: Karel Zikmund
 * 
 *   Popis: Tøída mapy
 * 
 ***********************************************************/

#ifndef __SERVER_MAP__HEADER_INCLUDED__
#define __SERVER_MAP__HEADER_INCLUDED__

#include "Common\MemoryPool\Array2DOnDemand.h"

#include "SMapSquare.h"
#include "SCivilization.h"
#include "SUnit.h"

#include "SDeferredProcedureCall.h"

#include "SWatchedUnitList.h"
#include "SRegisteredClientList.h"
#include "SDeferredProcedureCallQueue.h"
#include "SWatchingUnitRequestQueue.h"
#include "SWatchingMapSquareRequestQueue.h"

#include "Common\Map\MapDataTypes.h"

#include "SOneInstance.h"

//////////////////////////////////////////////////////////////////////
// Dopøedná deklarace tøíd

// tøída cesty na serveru hry
class CSPath;

// tøída rozesílatele informací o jednotkách klientovi civilizace
class CZClientUnitInfoSender;

//////////////////////////////////////////////////////////////////////
// Tøída mapy na serveru hry.
class CSMap : public CPersistentObject 
{
	friend class CSUnit;
	friend class CSUnitType;
	friend class CSMapSquare;
	friend class CSPath;
	friend class CSDeferredProcedureCall;

	DECLARE_DYNAMIC ( CSMap )

// Datové typy
private:
	// blok na zamykání jednotek
	struct SUnitBlock 
	{
		// zámek bloku jednotek
		CPrivilegedThreadReadWriteLock cLock;
		// pole ukazatelù na jednotky (NULL nebo ukazatel na jednotku)
		CSUnit **pUnits;
		// ukazatel na další blok jednotek
		SUnitBlock *pNext;
		// ukazatel na pøedcházející blok jednotek
		SUnitBlock *pPrevious;
	};
	// volná jednotka
	struct SFreeUnit 
	{
		// ukazatel na blok jednotek
		SUnitBlock *pBlock;
		// index volné jednotky v bloku jednotek
		int nIndex;
		// ukazatel na další volnou jednotku
		SFreeUnit *pNext;
	};
	// mrtvá jednotka
	struct SDeadUnit 
	{
		// ukazatel na jednotku
		CSUnit *pUnit;
		// TimeSlice smazání jednotky
		DWORD dwTimeSlice;

		// konstruktor
		SDeadUnit () {};
		// konstruktor
		SDeadUnit ( CSUnit *pInitUnit, DWORD dwInitTimeSlice ) 
			{ pUnit = pInitUnit; dwTimeSlice = dwInitTimeSlice; };
	};

	// popis uivatele
	struct SUserDescription 
	{
		// jméno uivatele
		CString strName;
		// heslo uivatele
		CString strPassword;
		// index civilizace
		DWORD dwCivilizationIndex;
		// TRUE pokud je uivatel povolen
		BOOL bEnabled;
		// TRUE pokud je uivatel nalogován
		BOOL bLoggedOn;
	};
public:
	// chybovı kód uivatele
	enum EUserError 
	{
		// uivatel je v poøádku
		UE_OK,
		// špatné jméno uivatele
		UE_BadName,
		// špatné heslo uivatele
		UE_BadPassword,
		// Uivatel je zakázán
		UE_UserDisabled,
	};

	// Tøída pro reakce na notifikace z mapy
	// Všechny reakce na zprávy mohou bıt volány z libovolného threadu
	// take to musí ustát
	class CSMapObserver
	{
	public:
		// Konstruktor a destruktor
		CSMapObserver() { m_pNextObserver = NULL; };
		virtual ~CSMapObserver() {};

		// Tato metoda je volána pokud se naloguje uivatel
		virtual void OnUserLoggedOn ( DWORD dwUserID ) = 0;
		// Tato metoda je volána pokud se uivatel odloguje
		virtual void OnUserLoggedOff ( DWORD dwUserID ) = 0;
		// Tato metoda je volána pøi kadém timeslicu
		virtual void OnTimeslice ( DWORD dwTimeslice ) = 0;

		// Ukazatel na další observer ve spojáku
		CSMapObserver *m_pNextObserver;
	};

// Metody
public:
// Konstrukce a destrukce

	// konstruktor
	CSMap ();
	// destruktor
	~CSMap ();

// Inicializace a znièení dat objektu

	// inicializuje data mapy z archivu mapy "cMapArchive"
	void Create ( CDataArchive cMapArchive );
	// znièí data mapy
	void Delete ();
private:
	// vyplní objekt neškodnımi daty (znièí èásteènì inicializavaná data)
	void SetEmptyData ();

public:
// Ukládání dat (CPersistentObject metody)

	// ukládání dat
	void PersistentSave ( CPersistentStorage &storage );
	// nahrávání pouze uloenıch dat, preferuje archiv mapy "strMapArchiveName", je-li 
	//		"strMapArchiveName" prázdnı, pouije uloené jméno archivu mapy (FALSE=špatná 
	//		verze archivu mapy, v "strMapArchiveName" vrací uloené jméno archivu mapy
	BOOL PersistentLoad ( CPersistentStorage &storage, CString &strMapArchiveName );
	// pøeklad ukazatelù
	void PersistentTranslatePointers ( CPersistentStorage &storage );
	// inicializace nahraného objektu
	void PersistentInit ();

private:
// Operace s mapou

	// vytvoøí novı blok jednotek (lze volat jen z MainLoop threadu)
	// vıjimky: CMemoryException
	void CreateUnitBlock ();
	// pøidá jednotku "pUnit" do jednotek mapy a aktualizuje její ukazatel na zámek
	//		(lze volat jen z MainLoop threadu)
	// vıjimky: CMemoryException
	void AddUnit ( CSUnit *pUnit );
	// smae jednotku "pUnit" z jednotek mapy (lze volat jen z MainLoop threadu)
	// vıjimky: CMemoryException
	void DeleteUnit ( CSUnit *pUnit );
	// vrátí MapSquare na pozici "pointPosition"
	inline CSMapSquare *GetMapSquareFromPosition ( CPointDW pointPosition ) const;
	// vrátí index MapSquaru na pozici "pointPosition"
	inline CPointDW GetMapSquareIndex ( CPointDW pointPosition ) const;
	// vrátí MapSquare s indexem "pointIndex"
	inline CSMapSquare *GetMapSquareFromIndex ( CPointDW pointIndex ) const;
	// vrátí MapSquare s indexem "pointIndex" nebo NULL
	inline CSMapSquare *GetMapSquareFromIndexNull ( CPointDW pointIndex ) const;
	// vrátí pole MapSquarù, na které jednotka vidí (mimo MainLoop thread musí bıt jednotka 
	//		zamèena pro ètení) (NULL=poslední MapSquare) 
	void GetSeenMapSquares ( CSUnit *pUnit, CSMapSquare *(&aSeenMapSquares)[4] ) const;

public:
// Informace o mapì

	// vrátí jméno archivu mapy
	CString GetMapArchiveName () { return m_strMapArchiveName; };
	// vrátí èíslo verze mapy
	DWORD GetMapVersion () { return m_dwMapVersion; };
	// vrátí jméno mapy
	CString GetMapName () { return m_strMapName; };
	// vrátí velikost mapy (v MapSquarech)
	CSizeDW GetMapSizeMapSquare () { return m_sizeMapMapSquare; };
	// vrátí velikost mapy (v MapCellech)
	CSizeDW GetMapSizeMapCell () { return m_sizeMapMapCell; };
	// vrátí pravı dolní roh mapy (v MapSquarech)
	CPointDW GetRightBottomCornerMapSquare () { return m_pointCornerMapSquare; };
	// vrátí pravı dolní roh mapy (v MapCellech)
	CPointDW GetRightBottomCornerMapCell () { return m_pointCornerMapCell; };

// Informace o høe

	// vrátí délku TimeSlicu
	DWORD GetTimeSliceLength () const { return m_dwTimeSliceLength; };
	// vrátí èíslo TimeSlicu
	DWORD GetTimeSlice () const { return m_dwTimeSlice; };
	// vrátí poèet civilizací
	DWORD GetCivilizationCount () const 
		{ ASSERT ( m_dwCivilizationCount > 0 ); return m_dwCivilizationCount; };
	// vrátí civilizaci "nIndex"
	CSCivilization *GetCivilization ( DWORD dwIndex ) const 
		{ ASSERT ( dwIndex < m_dwCivilizationCount ); return &m_pCivilizations[dwIndex]; };
	// zjistí, je-li pozice "pointPosition" na mapì
	inline BOOL IsMapPosition ( CPointDW pointPosition ) const;
	// zkontroluje, je-li nahraná pozice "pointPosition" na mapì
	// vıjimky: CPersistentLoadException
	inline BOOL CheckLoadedMapPosition ( CPointDW pointPosition ) const;
	// zjistí jednotku podle ID jednotky (NULL=jednotka neexistuje)
	CSUnit *GetUnitByID ( DWORD dwID );
	// pro civilizaci "dwCivilizationIndex" vyplní seznam "cUnitList" všech jednotek 
	//		(vlastních i nepøátelskıch) v oblasti se støedem "pointCenter" o polomìru 
	//		"dwRadius" (musí bıt zamèeny MapSquary, "dwRadius" musí bıt menší ne 
	//		MAX_GET_UNITS_IN_AREA_RADIUS)
	void GetUnitsInAreaAll ( DWORD dwCivilizationIndex, CPointDW pointCenter, 
		DWORD dwRadius, CSUnitList &cUnitList );
	// pro civilizaci "dwCivilizationIndex" vyplní seznam "cUnitList" nepøátelskıch 
	//		jednotek v oblasti se støedem "pointCenter" o polomìru "dwRadius" (musí bıt 
	//		zamèeny MapSquary, "dwRadius" musí bıt menší ne MAX_GET_UNITS_IN_AREA_RADIUS)
	void GetUnitsInAreaEnemy ( DWORD dwCivilizationIndex, CPointDW pointCenter, 
		DWORD dwRadius, CSUnitList &cUnitList );
	// pro civilizaci "dwCivilizationIndex" vyplní seznam "cUnitList" vlastních jednotek 
	//		v oblasti se støedem "pointCenter" o polomìru "dwRadius" (musí bıt zamèeny 
	//		MapSquary, "dwRadius" musí bıt menší ne MAX_GET_UNITS_IN_AREA_RADIUS)
	void GetUnitsInAreaOfMine ( DWORD dwCivilizationIndex, CPointDW pointCenter, 
		DWORD dwRadius, CSUnitList &cUnitList );

// Operace s jednotkou mapy

	// pohne s jednotkou "pUnit" na pozici "pointPosition" - pouze pohyb jednotky po mapì, 
	//		nikoli mimo mapu, voláno pro kadı pohyb jednotky (jednotka musí bıt zamèena 
	//		pro zápis, je-li nastaven pøíznak "bMapSquaresLocked", nezamyká MapSquary, lze 
	//		volat jen z MainLoop threadu) (TRUE=jednotka byla pøemístìna)
	BOOL MoveUnit ( CSUnit *pUnit, CPointDW pointPosition, BOOL bMapSquaresLocked = TRUE );
	// odebere jednotku "pUnit" z mapy (jednotka musí bıt zamèena pro zápis, je-li 
	//		nastaven pøíznak "bMapSquaresLocked", nezamyká MapSquary, lze volat jen 
	//		z MainLoop threadu)
	void RemoveUnit ( CSUnit *pUnit, BOOL bMapSquaresLocked = TRUE );
	// umístí jednotku "pUnit" na pozici "pointPosition" (jednotka musí bıt zamèena 
	//		pro zápis, je-li nastaven pøíznak "bMapSquaresLocked", nezamyká MapSquary, lze 
	//		volat jen z MainLoop threadu) (TRUE=jednotka byla umístìna)
	BOOL PlaceUnit ( CSUnit *pUnit, CPointDW pointPosition, BOOL bMapSquaresLocked = TRUE );
private:
	// zjistí, lze-li jednotku "pUnit" umístit na pozici "pointPosition" (je-li nastaven 
	//		pøíznak "bMapSquaresLocked", nezamyká MapSquary)
	inline BOOL CanPlaceUnitOnce ( CSUnit *pUnit, CPointDW pointPosition, 
		BOOL bMapSquaresLocked );
	// zpracuje umístìní jednotky "pUnit" na mapu (mimo MainLoop thread musí bıt jednotka 
	//		zamèena pro ètení)
	void UnitPlaced ( CSUnit *pUnit );
	// vrátí nejbliší umístìní jednotky "pUnit" na pozici "pointPosition" (jednotka musí 
	//		bıt zamèena pro ètení, MapSquary musí bıt zamèeny, lze volat jen z MainLoop 
	//		threadu) (NO_MAP_POSITION - jednotku nelze umístit)
	CPointDW GetNearestUnitPlace ( CSUnit *pUnit, CPointDW pointPosition );

public:
// Operace øízení hry (TimeSlicy a pausování hry)

	// nastaví délku TimeSlicu na "dwTimeSliceLength" (v milisekundách)
	inline void SetTimeSliceLength ( DWORD dwTimeSliceLength ) 
		{ ASSERT ( dwTimeSliceLength > 0 ); m_dwTimeSliceLength = dwTimeSliceLength; };
	// uspí volající thread a vrátí pøíznak pokraèování odpausované hry (TRUE=hra má 
	//		pokraèovat)
	inline BOOL CanContinueGame ();
	// zapausuje hru (poèká na zapausování hry)
	void PauseGame ();
	// zjistí, je-li hra zapausovaná
	BOOL IsGamePaused () { return m_eventPauseGame.Lock ( 0 ); };
	// odpausuje hru s pøíznakem pokraèování hry "bContinueGame"
	void UnpauseGame ( BOOL bContinueGame );
	// zjistí, jedná-li se o MainLoop thread
	BOOL IsMainLoopThread () 
		{ return ( m_dwMainLoopThreadID == GetCurrentThreadId () ); };
private:
	// initializuje hru (vytvoøí zapausovanou hru)
	void InitGame ();
	// spustí MainLoop mapy "pMap"
	static UINT RunMainLoop ( LPVOID pMap );
	// spustí ClientInfoSenderLoop mapy "pMap"
	static UINT RunClientInfoSenderLoop ( LPVOID pMap );
	// poèká na zaèátek nového TimeSlicu (s timeoutem "dwTimeout")
	static BOOL WaitForNewTimeSlice ( DWORD dwTimeout );

public:
// Operace pro klienta civilizace

	// zaregistruje klienta civilizace "pClient" (mùe trvat dlouho - èeká na dojetí 
	//		rozesílání informací o jednotkách klientùm)
	void RegisterClient ( CZClientUnitInfoSender *pClient );
	// odregistruje klienta civilizace "pClient" (mùe trvat dlouho - èeká na dojetí 
	//		rozesílání informací o jednotkách klientùm, bìhem volání této metody NESMÍ bıt 
	//		volány metody s klientem "pClient", ale server mùe rozeslat ještì nìkteré 
	//		informace - klient je musí stornovat) (odregistrováním klienta civilizace 
	//		ztrácí server všechny odkazy na klienta)
	void UnregisterClient ( CZClientUnitInfoSender *pClient );
	// zaène sledovat jednotku "dwID" klientem "pClient" (FALSE=jednotka ji neexistuje)
	BOOL StartWatchingUnit ( CZClientUnitInfoSender *pClient, DWORD dwID );
	// ukonèí sledování jednotky "dwID" klientem "pClient"
	void StopWatchingUnit ( CZClientUnitInfoSender *pClient, DWORD dwID );
	// zaène sledovat MapSquare "pointIndex" klientem "pClient"
	void StartWatchingMapSquare ( CZClientUnitInfoSender *pClient, CPointDW pointIndex );
	// ukonèí sledování MapSquaru "pointIndex" klientem "pClient"
	void StopWatchingMapSquare ( CZClientUnitInfoSender *pClient, CPointDW pointIndex );

// Operace s odloenım voláním procedury (DPC)

	// zaregistruje civilizaci "dwCivilizationIndex" odloené volání procedury "pDPC"
	void RegisterDPC ( CSDeferredProcedureCall *pDPC, DWORD dwCivilizationIndex );
	// odregistruje civilizaci "dwCivilizationIndex" odloené volání procedury "pDPC" 
	//		(FALSE=DPC nebylo zaregistrované)
	BOOL UnregisterDPC ( CSDeferredProcedureCall *pDPC, DWORD dwCivilizationIndex );
	// odregistruje odloené volání procedury "pDPC" (FALSE=DPC nebylo zaregistrované)
	BOOL UnregisterDPC ( CSDeferredProcedureCall *pDPC );

private:
// Vlákna vıpoètu serveru hry

	// hlavní smyèka mapy
	void MainLoop ();
	// smyèka rozesílání informací klientùm civilizací
	void ClientInfoSenderLoop ();

public:
// Operace s uivateli

	// zjistí poèet uivatelù (lze volat jen z aplikace serveru)
	DWORD GetUserCount ();
	// vyplní informace o uivateli èíslo "dwUserIndex" a vrátí identifikaci uivatele 
	//		"dwUserID", která je platná do smazání uivatele (lze volat jen z aplikace 
	//		serveru)
	void GetUser ( DWORD dwUserIndex, CString &strName, CString &strPassword, 
		DWORD &dwCivilizationIndex, DWORD &dwUserID );
	// vyplní informace o uivateli s identifikací "dwUserID" (lze volat jen z aplikace 
	//		serveru)
	void GetUser ( DWORD dwUserID, CString &strName, CString &strPassword, 
		DWORD &dwCivilizationIndex );
	// zmìní informace o uivateli s identifikací "dwUserID" (FALSE=duplicitní jméno) 
	//		(lze volat jen z aplikace serveru)
	BOOL EditUser ( DWORD dwUserID, LPCTSTR lpcszName, LPCTSTR lpcszPassword, 
		DWORD dwCivilizationIndex );
	// pøidá civilizaci "dwCivilizationIndex" uivatele se jménem "lpcszName" a heslem 
	//		"lpcszPassword", pøidá-li uivatele, vyplní jeho identifikaci "dwUserID", která 
	//		je platná do smazání uivatele (FALSE=duplicitní uivatel) (lze volat jen 
	//		z aplikace serveru)
	BOOL AddUser ( LPCTSTR lpcszName, LPCTSTR lpcszPassword, DWORD dwCivilizationIndex, 
		DWORD &dwUserID );
	// smae uivatele s identifikací "dwUserID" (identifikace je dále neplatná) (lze 
	//		volat jen z aplikace serveru)
	void DeleteUser ( DWORD dwUserID );
	// vravcí TRUE pokud je uivatel povolen
	BOOL IsUserEnabled ( DWORD dwUserID );
	// povolí uivatele
	void EnableUser ( DWORD dwUserID );
	// zakáe uivatele (pokud je uivatel nalogován odpojí ho)
	void DisableUser ( DWORD dwUserID );
	// vrací TRUE pokud je uivatel nalogován
	BOOL IsUserLoggedOn ( DWORD dwUserID );
	// najde uivatele "lpcszName" s heslem "lpcszPassword", vyplní civilizaci uivatele 
	//		"pZCivilization" a ID uivatele "dwUserID" (je-li uivatel nalezen), vrací: UE_OK, 
	//		UE_BadName, UE_BadPassword, UE_UserDisabled
	EUserError FindUser ( LPCTSTR lpcszName, LPCTSTR lpcszPassword, 
		CZCivilization *&pZCivilization, DWORD &dwUserID );
	// zaène logování uivatele (FALSE=hra nebìí, uivatel se nemùe nalogovat)
	BOOL StartUserLogin ();
	// ukonèí logování uivatele
	void FinishUserLogin ();
	// uivatel je nalogován (voláno po nalogování, pøed voláním FinishUserLogin)
	void UserLoggedOn ( LPCTSTR lpcszName );
	// uivatel je odlogován (voláno po odlogování)
	void UserLoggedOff ( LPCTSTR lpcszName );

// Operace okolo reakcí na notifikace

	// Pøidá objekt pro reakce na notifikace
	void RegisterMapObserver ( CSMapObserver *pMapObserver );
	// Odebere objekt pro reakce na notifikace
	void UnregisterMapObserver ( CSMapObserver *pMapObserver );

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
// Informace o mapì

	CSizeDW m_sizeMapMapSquare;
	// velikost mapy (v MapCellech)
	CSizeDW m_sizeMapMapCell;
	// index pravého dolního rohu (v MapSquarech)
	CPointDW m_pointCornerMapSquare;
	// index pravého dolního rohu (v MapCellech)
	CPointDW m_pointCornerMapCell;
	// pole MapSquarù
	CSMapSquare *m_pMapSquares;
	// jméno archivu mapy
	CString m_strMapArchiveName;
	// verze formátu mapy
	DWORD m_dwMapFormatVersion;
	// kompatabilní verze formátu mapy
	DWORD m_dwMapCompatibleFormatVersion;
	// jméno mapy
	CString m_strMapName;
	// popis mapy
	CString m_strMapDescription;
	// verze mapy
	DWORD m_dwMapVersion;
	// jména surovin
	CString m_aResourceName[RESOURCE_COUNT];
	// jména neviditelností
	CString m_aInvisibilityName[INVISIBILITY_COUNT];

// Informace o civilizacích

	// poèet civilizací
	DWORD m_dwCivilizationCount;
	// ukazatel na pole civilizací
	CSCivilization *m_pCivilizations;

// Informace o jednotkách

	// ukazatel na první blok jednotek - zápis pouze MainLoop thread (smazání jednotky 
	//		z bloku ještì neznamená znièení objektu jednotky, ten musí ještì chvíli ít), 
	//		ádnı blok jednotek se nikdy nesmí znièit!
	SUnitBlock *m_pFirstUnitBlock;
	// ukazatel na poslední blok jednotek - zápis pouze MainLoop thread (smazání jednotky 
	//		z bloku ještì neznamená znièení objektu jednotky, ten musí ještì chvíli ít), 
	//		ádnı blok jednotek se nikdy nesmí znièit!
	SUnitBlock *m_pLastUnitBlock;
	// ukazatel na první volnou jednotku - pøístup má pouze MainLoop thread
	SFreeUnit *m_pFirstFreeUnit;
	// fronta mrtvıch jednotek
	CSelfPooledQueue<struct SDeadUnit> m_cDeadUnitQueue;
	// seznam mrtvıch jednotek pro rozesílání stop infa klientùm
	CSelfPooledList<CSUnit *> m_cDeadUnitClientStopInfoList;
	// zámek seznamu mrtvıch jednotek pro rozesílání stop infa klientùm
	CMutex m_mutexDeadUnitClientStopInfoListLock;
	// seznam smazanıch jednotek
	CSelfPooledList<CSUnit *> m_cDeletedUnitList;
	// zámek seznamu smazanıch jednotek
	CMutex m_mutexDeletedUnitListLock;
	// zámek mrtvıch a smazanıch jednotek
	CPrivilegedThreadReadWriteLock m_cDeadDeletedUnitLock;

// Informace o øízení hry (TimeSlicy a pausování hry)

	// délka TimeSlicu v milisekundách
	DWORD m_dwTimeSliceLength;
	// èíslo provádìného TimeSlicu
	DWORD m_dwTimeSlice;
	// událost ukonèení TimeSlicu pro ClientInfoSender
	CEvent m_eventClientInfoSenderLoopTimeSliceFinished;
	// zámek MapSquarù
	CMutex m_mutexMapSquaresLock;

	// událast zapausování hry
	CEvent m_eventPauseGame;
	// událost zapausování MainLoop threadu
	CEvent m_eventMainLoopPaused;
	// událost zapausování ClientInfoSenderLoop threadu
	CEvent m_eventClientInfoSenderLoopPaused;
	// událost odpausování hry
	CEvent m_eventUnpauseGame;
	// pøíznak pokraèování odpausované hry (testuje se po odpausování hry)
	BOOL m_bContinueUnpausedGame;

	// MainLoop thread mapy
	CWinThread *m_pMainLoopThread;
	// ClientInfoSenderLoop thread mapy
	CWinThread *m_pClientInfoSenderLoopThread;
	// FindPathLoop thread mapy
	CWinThread *m_pFindPathLoopThread;

	// ID MainLoop threadu
	DWORD m_dwMainLoopThreadID;

	// pøíznak inicializované mapy
	BOOL m_bInitialized;

// Informace o jednotkách sledovanıch klientem civilizace

	// zámek seznamù zaregistrovanıch klientù civilizací
	CReadWriteLock m_cRegisteredClientListLock;
	// pole seznamù zaregistrovanıch klientù civilizací
	CSRegisteredClientList m_aRegisteredClientList[CIVILIZATION_COUNT_MAX];

	// zámek seznamu sledovanıch jednotek
	CMutex m_mutexWatchedUnitListLock;
	// seznam sledovanıch jednotek
	CSWatchedUnitList m_cWatchedUnitList;

	// zámek fronty poadavkù na sledování jednotek
	CMutex m_mutexWatchingUnitRequestQueueLock;
	// fronta poadavkù na sledování jednotek
	CSWatchingUnitRequestQueue m_cWatchingUnitRequestQueue;
	// zámek fronty poadavkù na sledování MapSquarù
	CMutex m_mutexWatchingMapSquareRequestQueueLock;
	// událost poadavku na sledování MapSquaru
	CEvent m_eventWatchingMapSquareRequest;
	// fronta poadavkù na sledování MapSquarù
	CSWatchingMapSquareRequestQueue m_cWatchingMapSquareRequestQueue;

// Informace o odloenıch voláních

	// zámek front odloenıch volání procedur civilizací
	CMutex m_mutexDPCQueueLock;
	// pole front odloenıch volání procedur civilizací
	CSDeferredProcedureCallQueue m_aDPCQueue[CIVILIZATION_COUNT_MAX];
	// odloené volání procedury pøipravované k provedení (zámkuje se jednotka asociovaná 
	//		s DPC)
	CSDeferredProcedureCall *m_pPreparingDPC;
	// index civilizace provádìného DPC
	DWORD m_dwRunningDPCCivilizationIndex;

// Informace o objektech CSMapObserver

	// Zámek spojáku observerù
	CMutex m_cMapObserverListLock;
	// Ukazatel na spoják observerù
	CSMapObserver *m_pFirstMapObserver;

// Informace o uivatelích

	// pole popisù uivatelù
	CTypedPtrArray<CPtrArray, struct SUserDescription *> m_aUserDescriptions;
	// zámek pro zápis do pole popisù uivatelù
	CMutex m_mutexUserDescriptionsWriterLock;
	// zámek pøíznaku povolení logování uivatelù
	CReadWriteLock m_cUserLoginAllowedLock;
	// pøíznak povolení logování uivatelù
	BOOL m_bUserLoginAllowed;

// Pomocná data

	// pole pøíznakù objevení pozic na mapì pro metodu "GetNearestUnitPlace"
	CArray2DOnDemand<BYTE> m_cGetNearestUnitPlacePositionArray;
};

//////////////////////////////////////////////////////////////////////
// Globální promìnné
//////////////////////////////////////////////////////////////////////

// Mapa hry
extern CSMap g_cMap;

//////////////////////////////////////////////////////////////////////
// Operace s mapou
//////////////////////////////////////////////////////////////////////

// vrátí MapSquare na pozici "pointPosition"
inline CSMapSquare *CSMap::GetMapSquareFromPosition ( CPointDW pointPosition ) const 
{
	ASSERT ( IsMapPosition ( pointPosition ) );

	// vrátí MapSquare na pozici "pointPosition"
	return &m_pMapSquares[( pointPosition.x / MAP_SQUARE_SIZE ) + 
		( pointPosition.y / MAP_SQUARE_SIZE ) * m_sizeMapMapSquare.cx];
}

// vrátí index MapSquaru na pozici "pointPosition"
inline CPointDW CSMap::GetMapSquareIndex ( CPointDW pointPosition ) const 
{
	ASSERT ( IsMapPosition ( pointPosition ) );

	// vrátí index MapSquaru na pozici "pointPosition"
	return CPointDW ( pointPosition.x / MAP_SQUARE_SIZE, 
		pointPosition.y / MAP_SQUARE_SIZE );
}

// vrátí MapSquare s indexem "pointIndex"
inline CSMapSquare *CSMap::GetMapSquareFromIndex ( CPointDW pointIndex ) const 
{
	ASSERT ( pointIndex.x < m_sizeMapMapSquare.cx );
	ASSERT ( pointIndex.y < m_sizeMapMapSquare.cy );

	// vrátí MapSquare s indexem "pointIndex"
	return &m_pMapSquares[pointIndex.x + pointIndex.y * m_sizeMapMapSquare.cx];
}

// vrátí MapSquare s indexem "pointIndex" nebo NULL
inline CSMapSquare *CSMap::GetMapSquareFromIndexNull ( CPointDW pointIndex ) const 
{
	// zjistí, jedná-li se o korektní MapSquare
	if ( ( pointIndex.x < m_sizeMapMapSquare.cx ) && ( pointIndex.y < 
		m_sizeMapMapSquare.cy ) )
	{	// jedná se o korektní MapSquare
		// vrátí MapSquare s indexem "pointIndex"
		return &m_pMapSquares[pointIndex.x + pointIndex.y * m_sizeMapMapSquare.cx];
	}
	else
	{	// nejedná se o korektní MapSquare
		// vrátí pøíznak nekorektního MapSquaru
		return NULL;
	}
}

//////////////////////////////////////////////////////////////////////
// Informace o høe
//////////////////////////////////////////////////////////////////////

// zjistí, je-li pozice "pointPosition" na mapì
inline BOOL CSMap::IsMapPosition ( CPointDW pointPosition ) const 
{
	// zjistí, je-li pozice na mapì
	if ( pointPosition.x != NO_MAP_POSITION )
	{	// jedná se o pozici na mapì
		ASSERT ( pointPosition.x < m_sizeMapMapCell.cx );
		ASSERT ( pointPosition.y < m_sizeMapMapCell.cy );

		// vrátí pøíznak pozice na mapì
		return TRUE;
	}
	else
	{	// jedná se o pozici mimo mapu
		ASSERT ( pointPosition.y == NO_MAP_POSITION );
		// vrátí pøíznak pozice mimo mapu
		return FALSE;
	}
}

// zkontroluje, je-li nahraná pozice "pointPosition" na mapì
// vıjimky: CPersistentLoadException
inline BOOL CSMap::CheckLoadedMapPosition ( CPointDW pointPosition ) const 
{
	// zjistí, je-li pozice na mapì
	if ( pointPosition.x != NO_MAP_POSITION )
	{	// jedná se o pozici na mapì
		LOAD_ASSERT ( pointPosition.x < m_sizeMapMapCell.cx );
		LOAD_ASSERT ( pointPosition.y < m_sizeMapMapCell.cy );

		// vrátí pøíznak pozice na mapì
		return TRUE;
	}
	else
	{	// jedná se o pozici mimo mapu
		LOAD_ASSERT ( pointPosition.y == NO_MAP_POSITION );
		// vrátí pøíznak pozice mimo mapu
		return FALSE;
	}
}

//////////////////////////////////////////////////////////////////////
// Operace s jednotkou mapy
//////////////////////////////////////////////////////////////////////

// zjistí, lze-li jednotku "pUnit" umístit na pozici "pointPosition" (je-li nastaven 
//		pøíznak "bMapSquaresLocked", nezamyká MapSquary) (není vhodné pro testování více 
//		pozic umístìní jedné jednotky)
inline BOOL CSMap::CanPlaceUnitOnce ( CSUnit *pUnit, CPointDW pointPosition, 
	BOOL bMapSquaresLocked ) 
{
	ASSERT ( IsMapPosition ( pointPosition ) );

	// zjistí, má-li jednotka vzdušnou vertikální pozici
	if ( pUnit->GetVerticalPosition () >= AIR_VERTICAL_POSITION )
	{	// jednotka má vzdušnou vertikální pozici
		// jednotku lze umístit na poadovanou pozici
		return TRUE;
	}

	// zjistí, lze-li jednotku umístit na pozici
	BOOL bCanPlaceUnit = CSMapSquare::MainLoopCanPlaceUnit ( pUnit, pointPosition, 
		bMapSquaresLocked );

	// ukonèí umísování jednotky
	CSMapSquare::MainLoopFinishPlacingUnit ();

	// vrátí pøíznak umístìní jednotky
	return bCanPlaceUnit;
}

//////////////////////////////////////////////////////////////////////
// Operace øízení hry (TimeSlicy a pausování hry)
//////////////////////////////////////////////////////////////////////

// uspí volající thread a vrátí pøíznak pokraèování odpausované hry (TRUE=hra má 
//		pokraèovat)
inline BOOL CSMap::CanContinueGame () 
{
	// poèká na událost odpausování hry
	VERIFY ( m_eventUnpauseGame.Lock () );

	// vrátí pøíznak pokraèování odpausované hry
	return m_bContinueUnpausedGame;
};

#endif //__SERVER_MAP__HEADER_INCLUDED__
