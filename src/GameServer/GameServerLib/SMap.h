/***********************************************************
 * 
 * Projekt: Strategick� hra
 *    ��st: Server hry
 *   Autor: Karel Zikmund
 * 
 *   Popis: T��da mapy
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
// Dop�edn� deklarace t��d

// t��da cesty na serveru hry
class CSPath;

// t��da rozes�latele informac� o jednotk�ch klientovi civilizace
class CZClientUnitInfoSender;

//////////////////////////////////////////////////////////////////////
// T��da mapy na serveru hry.
class CSMap : public CPersistentObject 
{
	friend class CSUnit;
	friend class CSUnitType;
	friend class CSMapSquare;
	friend class CSPath;
	friend class CSDeferredProcedureCall;

	DECLARE_DYNAMIC ( CSMap )

// Datov� typy
private:
	// blok na zamyk�n� jednotek
	struct SUnitBlock 
	{
		// z�mek bloku jednotek
		CPrivilegedThreadReadWriteLock cLock;
		// pole ukazatel� na jednotky (NULL nebo ukazatel na jednotku)
		CSUnit **pUnits;
		// ukazatel na dal�� blok jednotek
		SUnitBlock *pNext;
		// ukazatel na p�edch�zej�c� blok jednotek
		SUnitBlock *pPrevious;
	};
	// voln� jednotka
	struct SFreeUnit 
	{
		// ukazatel na blok jednotek
		SUnitBlock *pBlock;
		// index voln� jednotky v bloku jednotek
		int nIndex;
		// ukazatel na dal�� volnou jednotku
		SFreeUnit *pNext;
	};
	// mrtv� jednotka
	struct SDeadUnit 
	{
		// ukazatel na jednotku
		CSUnit *pUnit;
		// TimeSlice smaz�n� jednotky
		DWORD dwTimeSlice;

		// konstruktor
		SDeadUnit () {};
		// konstruktor
		SDeadUnit ( CSUnit *pInitUnit, DWORD dwInitTimeSlice ) 
			{ pUnit = pInitUnit; dwTimeSlice = dwInitTimeSlice; };
	};

	// popis u�ivatele
	struct SUserDescription 
	{
		// jm�no u�ivatele
		CString strName;
		// heslo u�ivatele
		CString strPassword;
		// index civilizace
		DWORD dwCivilizationIndex;
		// TRUE pokud je u�ivatel povolen
		BOOL bEnabled;
		// TRUE pokud je u�ivatel nalogov�n
		BOOL bLoggedOn;
	};
public:
	// chybov� k�d u�ivatele
	enum EUserError 
	{
		// u�ivatel je v po��dku
		UE_OK,
		// �patn� jm�no u�ivatele
		UE_BadName,
		// �patn� heslo u�ivatele
		UE_BadPassword,
		// U�ivatel je zak�z�n
		UE_UserDisabled,
	};

	// T��da pro reakce na notifikace z mapy
	// V�echny reakce na zpr�vy mohou b�t vol�ny z libovoln�ho threadu
	// tak�e to mus� ust�t
	class CSMapObserver
	{
	public:
		// Konstruktor a destruktor
		CSMapObserver() { m_pNextObserver = NULL; };
		virtual ~CSMapObserver() {};

		// Tato metoda je vol�na pokud se naloguje u�ivatel
		virtual void OnUserLoggedOn ( DWORD dwUserID ) = 0;
		// Tato metoda je vol�na pokud se u�ivatel odloguje
		virtual void OnUserLoggedOff ( DWORD dwUserID ) = 0;
		// Tato metoda je vol�na p�i ka�d�m timeslicu
		virtual void OnTimeslice ( DWORD dwTimeslice ) = 0;

		// Ukazatel na dal�� observer ve spoj�ku
		CSMapObserver *m_pNextObserver;
	};

// Metody
public:
// Konstrukce a destrukce

	// konstruktor
	CSMap ();
	// destruktor
	~CSMap ();

// Inicializace a zni�en� dat objektu

	// inicializuje data mapy z archivu mapy "cMapArchive"
	void Create ( CDataArchive cMapArchive );
	// zni�� data mapy
	void Delete ();
private:
	// vypln� objekt ne�kodn�mi daty (zni�� ��ste�n� inicializavan� data)
	void SetEmptyData ();

public:
// Ukl�d�n� dat (CPersistentObject metody)

	// ukl�d�n� dat
	void PersistentSave ( CPersistentStorage &storage );
	// nahr�v�n� pouze ulo�en�ch dat, preferuje archiv mapy "strMapArchiveName", je-li 
	//		"strMapArchiveName" pr�zdn�, pou�ije ulo�en� jm�no archivu mapy (FALSE=�patn� 
	//		verze archivu mapy, v "strMapArchiveName" vrac� ulo�en� jm�no archivu mapy
	BOOL PersistentLoad ( CPersistentStorage &storage, CString &strMapArchiveName );
	// p�eklad ukazatel�
	void PersistentTranslatePointers ( CPersistentStorage &storage );
	// inicializace nahran�ho objektu
	void PersistentInit ();

private:
// Operace s mapou

	// vytvo�� nov� blok jednotek (lze volat jen z MainLoop threadu)
	// v�jimky: CMemoryException
	void CreateUnitBlock ();
	// p�id� jednotku "pUnit" do jednotek mapy a aktualizuje jej� ukazatel na z�mek
	//		(lze volat jen z MainLoop threadu)
	// v�jimky: CMemoryException
	void AddUnit ( CSUnit *pUnit );
	// sma�e jednotku "pUnit" z jednotek mapy (lze volat jen z MainLoop threadu)
	// v�jimky: CMemoryException
	void DeleteUnit ( CSUnit *pUnit );
	// vr�t� MapSquare na pozici "pointPosition"
	inline CSMapSquare *GetMapSquareFromPosition ( CPointDW pointPosition ) const;
	// vr�t� index MapSquaru na pozici "pointPosition"
	inline CPointDW GetMapSquareIndex ( CPointDW pointPosition ) const;
	// vr�t� MapSquare s indexem "pointIndex"
	inline CSMapSquare *GetMapSquareFromIndex ( CPointDW pointIndex ) const;
	// vr�t� MapSquare s indexem "pointIndex" nebo NULL
	inline CSMapSquare *GetMapSquareFromIndexNull ( CPointDW pointIndex ) const;
	// vr�t� pole MapSquar�, na kter� jednotka vid� (mimo MainLoop thread mus� b�t jednotka 
	//		zam�ena pro �ten�) (NULL=posledn� MapSquare) 
	void GetSeenMapSquares ( CSUnit *pUnit, CSMapSquare *(&aSeenMapSquares)[4] ) const;

public:
// Informace o map�

	// vr�t� jm�no archivu mapy
	CString GetMapArchiveName () { return m_strMapArchiveName; };
	// vr�t� ��slo verze mapy
	DWORD GetMapVersion () { return m_dwMapVersion; };
	// vr�t� jm�no mapy
	CString GetMapName () { return m_strMapName; };
	// vr�t� velikost mapy (v MapSquarech)
	CSizeDW GetMapSizeMapSquare () { return m_sizeMapMapSquare; };
	// vr�t� velikost mapy (v MapCellech)
	CSizeDW GetMapSizeMapCell () { return m_sizeMapMapCell; };
	// vr�t� prav� doln� roh mapy (v MapSquarech)
	CPointDW GetRightBottomCornerMapSquare () { return m_pointCornerMapSquare; };
	// vr�t� prav� doln� roh mapy (v MapCellech)
	CPointDW GetRightBottomCornerMapCell () { return m_pointCornerMapCell; };

// Informace o h�e

	// vr�t� d�lku TimeSlicu
	DWORD GetTimeSliceLength () const { return m_dwTimeSliceLength; };
	// vr�t� ��slo TimeSlicu
	DWORD GetTimeSlice () const { return m_dwTimeSlice; };
	// vr�t� po�et civilizac�
	DWORD GetCivilizationCount () const 
		{ ASSERT ( m_dwCivilizationCount > 0 ); return m_dwCivilizationCount; };
	// vr�t� civilizaci "nIndex"
	CSCivilization *GetCivilization ( DWORD dwIndex ) const 
		{ ASSERT ( dwIndex < m_dwCivilizationCount ); return &m_pCivilizations[dwIndex]; };
	// zjist�, je-li pozice "pointPosition" na map�
	inline BOOL IsMapPosition ( CPointDW pointPosition ) const;
	// zkontroluje, je-li nahran� pozice "pointPosition" na map�
	// v�jimky: CPersistentLoadException
	inline BOOL CheckLoadedMapPosition ( CPointDW pointPosition ) const;
	// zjist� jednotku podle ID jednotky (NULL=jednotka neexistuje)
	CSUnit *GetUnitByID ( DWORD dwID );
	// pro civilizaci "dwCivilizationIndex" vypln� seznam "cUnitList" v�ech jednotek 
	//		(vlastn�ch i nep��telsk�ch) v oblasti se st�edem "pointCenter" o polom�ru 
	//		"dwRadius" (mus� b�t zam�eny MapSquary, "dwRadius" mus� b�t men�� ne� 
	//		MAX_GET_UNITS_IN_AREA_RADIUS)
	void GetUnitsInAreaAll ( DWORD dwCivilizationIndex, CPointDW pointCenter, 
		DWORD dwRadius, CSUnitList &cUnitList );
	// pro civilizaci "dwCivilizationIndex" vypln� seznam "cUnitList" nep��telsk�ch 
	//		jednotek v oblasti se st�edem "pointCenter" o polom�ru "dwRadius" (mus� b�t 
	//		zam�eny MapSquary, "dwRadius" mus� b�t men�� ne� MAX_GET_UNITS_IN_AREA_RADIUS)
	void GetUnitsInAreaEnemy ( DWORD dwCivilizationIndex, CPointDW pointCenter, 
		DWORD dwRadius, CSUnitList &cUnitList );
	// pro civilizaci "dwCivilizationIndex" vypln� seznam "cUnitList" vlastn�ch jednotek 
	//		v oblasti se st�edem "pointCenter" o polom�ru "dwRadius" (mus� b�t zam�eny 
	//		MapSquary, "dwRadius" mus� b�t men�� ne� MAX_GET_UNITS_IN_AREA_RADIUS)
	void GetUnitsInAreaOfMine ( DWORD dwCivilizationIndex, CPointDW pointCenter, 
		DWORD dwRadius, CSUnitList &cUnitList );

// Operace s jednotkou mapy

	// pohne s jednotkou "pUnit" na pozici "pointPosition" - pouze pohyb jednotky po map�, 
	//		nikoli mimo mapu, vol�no pro ka�d� pohyb jednotky (jednotka mus� b�t zam�ena 
	//		pro z�pis, je-li nastaven p��znak "bMapSquaresLocked", nezamyk� MapSquary, lze 
	//		volat jen z MainLoop threadu) (TRUE=jednotka byla p�em�st�na)
	BOOL MoveUnit ( CSUnit *pUnit, CPointDW pointPosition, BOOL bMapSquaresLocked = TRUE );
	// odebere jednotku "pUnit" z mapy (jednotka mus� b�t zam�ena pro z�pis, je-li 
	//		nastaven p��znak "bMapSquaresLocked", nezamyk� MapSquary, lze volat jen 
	//		z MainLoop threadu)
	void RemoveUnit ( CSUnit *pUnit, BOOL bMapSquaresLocked = TRUE );
	// um�st� jednotku "pUnit" na pozici "pointPosition" (jednotka mus� b�t zam�ena 
	//		pro z�pis, je-li nastaven p��znak "bMapSquaresLocked", nezamyk� MapSquary, lze 
	//		volat jen z MainLoop threadu) (TRUE=jednotka byla um�st�na)
	BOOL PlaceUnit ( CSUnit *pUnit, CPointDW pointPosition, BOOL bMapSquaresLocked = TRUE );
private:
	// zjist�, lze-li jednotku "pUnit" um�stit na pozici "pointPosition" (je-li nastaven 
	//		p��znak "bMapSquaresLocked", nezamyk� MapSquary)
	inline BOOL CanPlaceUnitOnce ( CSUnit *pUnit, CPointDW pointPosition, 
		BOOL bMapSquaresLocked );
	// zpracuje um�st�n� jednotky "pUnit" na mapu (mimo MainLoop thread mus� b�t jednotka 
	//		zam�ena pro �ten�)
	void UnitPlaced ( CSUnit *pUnit );
	// vr�t� nejbli��� um�st�n� jednotky "pUnit" na pozici "pointPosition" (jednotka mus� 
	//		b�t zam�ena pro �ten�, MapSquary mus� b�t zam�eny, lze volat jen z MainLoop 
	//		threadu) (NO_MAP_POSITION - jednotku nelze um�stit)
	CPointDW GetNearestUnitPlace ( CSUnit *pUnit, CPointDW pointPosition );

public:
// Operace ��zen� hry (TimeSlicy a pausov�n� hry)

	// nastav� d�lku TimeSlicu na "dwTimeSliceLength" (v milisekund�ch)
	inline void SetTimeSliceLength ( DWORD dwTimeSliceLength ) 
		{ ASSERT ( dwTimeSliceLength > 0 ); m_dwTimeSliceLength = dwTimeSliceLength; };
	// usp� volaj�c� thread a vr�t� p��znak pokra�ov�n� odpausovan� hry (TRUE=hra m� 
	//		pokra�ovat)
	inline BOOL CanContinueGame ();
	// zapausuje hru (po�k� na zapausov�n� hry)
	void PauseGame ();
	// zjist�, je-li hra zapausovan�
	BOOL IsGamePaused () { return m_eventPauseGame.Lock ( 0 ); };
	// odpausuje hru s p��znakem pokra�ov�n� hry "bContinueGame"
	void UnpauseGame ( BOOL bContinueGame );
	// zjist�, jedn�-li se o MainLoop thread
	BOOL IsMainLoopThread () 
		{ return ( m_dwMainLoopThreadID == GetCurrentThreadId () ); };
private:
	// initializuje hru (vytvo�� zapausovanou hru)
	void InitGame ();
	// spust� MainLoop mapy "pMap"
	static UINT RunMainLoop ( LPVOID pMap );
	// spust� ClientInfoSenderLoop mapy "pMap"
	static UINT RunClientInfoSenderLoop ( LPVOID pMap );
	// po�k� na za��tek nov�ho TimeSlicu (s timeoutem "dwTimeout")
	static BOOL WaitForNewTimeSlice ( DWORD dwTimeout );

public:
// Operace pro klienta civilizace

	// zaregistruje klienta civilizace "pClient" (m��e trvat dlouho - �ek� na dojet� 
	//		rozes�l�n� informac� o jednotk�ch klient�m)
	void RegisterClient ( CZClientUnitInfoSender *pClient );
	// odregistruje klienta civilizace "pClient" (m��e trvat dlouho - �ek� na dojet� 
	//		rozes�l�n� informac� o jednotk�ch klient�m, b�hem vol�n� t�to metody NESM� b�t 
	//		vol�ny metody s klientem "pClient", ale server m��e rozeslat je�t� n�kter� 
	//		informace - klient je mus� stornovat) (odregistrov�n�m klienta civilizace 
	//		ztr�c� server v�echny odkazy na klienta)
	void UnregisterClient ( CZClientUnitInfoSender *pClient );
	// za�ne sledovat jednotku "dwID" klientem "pClient" (FALSE=jednotka ji� neexistuje)
	BOOL StartWatchingUnit ( CZClientUnitInfoSender *pClient, DWORD dwID );
	// ukon�� sledov�n� jednotky "dwID" klientem "pClient"
	void StopWatchingUnit ( CZClientUnitInfoSender *pClient, DWORD dwID );
	// za�ne sledovat MapSquare "pointIndex" klientem "pClient"
	void StartWatchingMapSquare ( CZClientUnitInfoSender *pClient, CPointDW pointIndex );
	// ukon�� sledov�n� MapSquaru "pointIndex" klientem "pClient"
	void StopWatchingMapSquare ( CZClientUnitInfoSender *pClient, CPointDW pointIndex );

// Operace s odlo�en�m vol�n�m procedury (DPC)

	// zaregistruje civilizaci "dwCivilizationIndex" odlo�en� vol�n� procedury "pDPC"
	void RegisterDPC ( CSDeferredProcedureCall *pDPC, DWORD dwCivilizationIndex );
	// odregistruje civilizaci "dwCivilizationIndex" odlo�en� vol�n� procedury "pDPC" 
	//		(FALSE=DPC nebylo zaregistrovan�)
	BOOL UnregisterDPC ( CSDeferredProcedureCall *pDPC, DWORD dwCivilizationIndex );
	// odregistruje odlo�en� vol�n� procedury "pDPC" (FALSE=DPC nebylo zaregistrovan�)
	BOOL UnregisterDPC ( CSDeferredProcedureCall *pDPC );

private:
// Vl�kna v�po�tu serveru hry

	// hlavn� smy�ka mapy
	void MainLoop ();
	// smy�ka rozes�l�n� informac� klient�m civilizac�
	void ClientInfoSenderLoop ();

public:
// Operace s u�ivateli

	// zjist� po�et u�ivatel� (lze volat jen z aplikace serveru)
	DWORD GetUserCount ();
	// vypln� informace o u�ivateli ��slo "dwUserIndex" a vr�t� identifikaci u�ivatele 
	//		"dwUserID", kter� je platn� do smaz�n� u�ivatele (lze volat jen z aplikace 
	//		serveru)
	void GetUser ( DWORD dwUserIndex, CString &strName, CString &strPassword, 
		DWORD &dwCivilizationIndex, DWORD &dwUserID );
	// vypln� informace o u�ivateli s identifikac� "dwUserID" (lze volat jen z aplikace 
	//		serveru)
	void GetUser ( DWORD dwUserID, CString &strName, CString &strPassword, 
		DWORD &dwCivilizationIndex );
	// zm�n� informace o u�ivateli s identifikac� "dwUserID" (FALSE=duplicitn� jm�no) 
	//		(lze volat jen z aplikace serveru)
	BOOL EditUser ( DWORD dwUserID, LPCTSTR lpcszName, LPCTSTR lpcszPassword, 
		DWORD dwCivilizationIndex );
	// p�id� civilizaci "dwCivilizationIndex" u�ivatele se jm�nem "lpcszName" a heslem 
	//		"lpcszPassword", p�id�-li u�ivatele, vypln� jeho identifikaci "dwUserID", kter� 
	//		je platn� do smaz�n� u�ivatele (FALSE=duplicitn� u�ivatel) (lze volat jen 
	//		z aplikace serveru)
	BOOL AddUser ( LPCTSTR lpcszName, LPCTSTR lpcszPassword, DWORD dwCivilizationIndex, 
		DWORD &dwUserID );
	// sma�e u�ivatele s identifikac� "dwUserID" (identifikace je d�le neplatn�) (lze 
	//		volat jen z aplikace serveru)
	void DeleteUser ( DWORD dwUserID );
	// vravc� TRUE pokud je u�ivatel povolen
	BOOL IsUserEnabled ( DWORD dwUserID );
	// povol� u�ivatele
	void EnableUser ( DWORD dwUserID );
	// zak�e u�ivatele (pokud je u�ivatel nalogov�n odpoj� ho)
	void DisableUser ( DWORD dwUserID );
	// vrac� TRUE pokud je u�ivatel nalogov�n
	BOOL IsUserLoggedOn ( DWORD dwUserID );
	// najde u�ivatele "lpcszName" s heslem "lpcszPassword", vypln� civilizaci u�ivatele 
	//		"pZCivilization" a ID u�ivatele "dwUserID" (je-li u�ivatel nalezen), vrac�: UE_OK, 
	//		UE_BadName, UE_BadPassword, UE_UserDisabled
	EUserError FindUser ( LPCTSTR lpcszName, LPCTSTR lpcszPassword, 
		CZCivilization *&pZCivilization, DWORD &dwUserID );
	// za�ne logov�n� u�ivatele (FALSE=hra neb��, u�ivatel se nem��e nalogovat)
	BOOL StartUserLogin ();
	// ukon�� logov�n� u�ivatele
	void FinishUserLogin ();
	// u�ivatel je nalogov�n (vol�no po nalogov�n�, p�ed vol�n�m FinishUserLogin)
	void UserLoggedOn ( LPCTSTR lpcszName );
	// u�ivatel je odlogov�n (vol�no po odlogov�n�)
	void UserLoggedOff ( LPCTSTR lpcszName );

// Operace okolo reakc� na notifikace

	// P�id� objekt pro reakce na notifikace
	void RegisterMapObserver ( CSMapObserver *pMapObserver );
	// Odebere objekt pro reakce na notifikace
	void UnregisterMapObserver ( CSMapObserver *pMapObserver );

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
// Informace o map�

	CSizeDW m_sizeMapMapSquare;
	// velikost mapy (v MapCellech)
	CSizeDW m_sizeMapMapCell;
	// index prav�ho doln�ho rohu (v MapSquarech)
	CPointDW m_pointCornerMapSquare;
	// index prav�ho doln�ho rohu (v MapCellech)
	CPointDW m_pointCornerMapCell;
	// pole MapSquar�
	CSMapSquare *m_pMapSquares;
	// jm�no archivu mapy
	CString m_strMapArchiveName;
	// verze form�tu mapy
	DWORD m_dwMapFormatVersion;
	// kompatabiln� verze form�tu mapy
	DWORD m_dwMapCompatibleFormatVersion;
	// jm�no mapy
	CString m_strMapName;
	// popis mapy
	CString m_strMapDescription;
	// verze mapy
	DWORD m_dwMapVersion;
	// jm�na surovin
	CString m_aResourceName[RESOURCE_COUNT];
	// jm�na neviditelnost�
	CString m_aInvisibilityName[INVISIBILITY_COUNT];

// Informace o civilizac�ch

	// po�et civilizac�
	DWORD m_dwCivilizationCount;
	// ukazatel na pole civilizac�
	CSCivilization *m_pCivilizations;

// Informace o jednotk�ch

	// ukazatel na prvn� blok jednotek - z�pis pouze MainLoop thread (smaz�n� jednotky 
	//		z bloku je�t� neznamen� zni�en� objektu jednotky, ten mus� je�t� chv�li ��t), 
	//		��dn� blok jednotek se nikdy nesm� zni�it!
	SUnitBlock *m_pFirstUnitBlock;
	// ukazatel na posledn� blok jednotek - z�pis pouze MainLoop thread (smaz�n� jednotky 
	//		z bloku je�t� neznamen� zni�en� objektu jednotky, ten mus� je�t� chv�li ��t), 
	//		��dn� blok jednotek se nikdy nesm� zni�it!
	SUnitBlock *m_pLastUnitBlock;
	// ukazatel na prvn� volnou jednotku - p��stup m� pouze MainLoop thread
	SFreeUnit *m_pFirstFreeUnit;
	// fronta mrtv�ch jednotek
	CSelfPooledQueue<struct SDeadUnit> m_cDeadUnitQueue;
	// seznam mrtv�ch jednotek pro rozes�l�n� stop infa klient�m
	CSelfPooledList<CSUnit *> m_cDeadUnitClientStopInfoList;
	// z�mek seznamu mrtv�ch jednotek pro rozes�l�n� stop infa klient�m
	CMutex m_mutexDeadUnitClientStopInfoListLock;
	// seznam smazan�ch jednotek
	CSelfPooledList<CSUnit *> m_cDeletedUnitList;
	// z�mek seznamu smazan�ch jednotek
	CMutex m_mutexDeletedUnitListLock;
	// z�mek mrtv�ch a smazan�ch jednotek
	CPrivilegedThreadReadWriteLock m_cDeadDeletedUnitLock;

// Informace o ��zen� hry (TimeSlicy a pausov�n� hry)

	// d�lka TimeSlicu v milisekund�ch
	DWORD m_dwTimeSliceLength;
	// ��slo prov�d�n�ho TimeSlicu
	DWORD m_dwTimeSlice;
	// ud�lost ukon�en� TimeSlicu pro ClientInfoSender
	CEvent m_eventClientInfoSenderLoopTimeSliceFinished;
	// z�mek MapSquar�
	CMutex m_mutexMapSquaresLock;

	// ud�last zapausov�n� hry
	CEvent m_eventPauseGame;
	// ud�lost zapausov�n� MainLoop threadu
	CEvent m_eventMainLoopPaused;
	// ud�lost zapausov�n� ClientInfoSenderLoop threadu
	CEvent m_eventClientInfoSenderLoopPaused;
	// ud�lost odpausov�n� hry
	CEvent m_eventUnpauseGame;
	// p��znak pokra�ov�n� odpausovan� hry (testuje se po odpausov�n� hry)
	BOOL m_bContinueUnpausedGame;

	// MainLoop thread mapy
	CWinThread *m_pMainLoopThread;
	// ClientInfoSenderLoop thread mapy
	CWinThread *m_pClientInfoSenderLoopThread;
	// FindPathLoop thread mapy
	CWinThread *m_pFindPathLoopThread;

	// ID MainLoop threadu
	DWORD m_dwMainLoopThreadID;

	// p��znak inicializovan� mapy
	BOOL m_bInitialized;

// Informace o jednotk�ch sledovan�ch klientem civilizace

	// z�mek seznam� zaregistrovan�ch klient� civilizac�
	CReadWriteLock m_cRegisteredClientListLock;
	// pole seznam� zaregistrovan�ch klient� civilizac�
	CSRegisteredClientList m_aRegisteredClientList[CIVILIZATION_COUNT_MAX];

	// z�mek seznamu sledovan�ch jednotek
	CMutex m_mutexWatchedUnitListLock;
	// seznam sledovan�ch jednotek
	CSWatchedUnitList m_cWatchedUnitList;

	// z�mek fronty po�adavk� na sledov�n� jednotek
	CMutex m_mutexWatchingUnitRequestQueueLock;
	// fronta po�adavk� na sledov�n� jednotek
	CSWatchingUnitRequestQueue m_cWatchingUnitRequestQueue;
	// z�mek fronty po�adavk� na sledov�n� MapSquar�
	CMutex m_mutexWatchingMapSquareRequestQueueLock;
	// ud�lost po�adavku na sledov�n� MapSquaru
	CEvent m_eventWatchingMapSquareRequest;
	// fronta po�adavk� na sledov�n� MapSquar�
	CSWatchingMapSquareRequestQueue m_cWatchingMapSquareRequestQueue;

// Informace o odlo�en�ch vol�n�ch

	// z�mek front odlo�en�ch vol�n� procedur civilizac�
	CMutex m_mutexDPCQueueLock;
	// pole front odlo�en�ch vol�n� procedur civilizac�
	CSDeferredProcedureCallQueue m_aDPCQueue[CIVILIZATION_COUNT_MAX];
	// odlo�en� vol�n� procedury p�ipravovan� k proveden� (z�mkuje se jednotka asociovan� 
	//		s DPC)
	CSDeferredProcedureCall *m_pPreparingDPC;
	// index civilizace prov�d�n�ho DPC
	DWORD m_dwRunningDPCCivilizationIndex;

// Informace o objektech CSMapObserver

	// Z�mek spoj�ku observer�
	CMutex m_cMapObserverListLock;
	// Ukazatel na spoj�k observer�
	CSMapObserver *m_pFirstMapObserver;

// Informace o u�ivatel�ch

	// pole popis� u�ivatel�
	CTypedPtrArray<CPtrArray, struct SUserDescription *> m_aUserDescriptions;
	// z�mek pro z�pis do pole popis� u�ivatel�
	CMutex m_mutexUserDescriptionsWriterLock;
	// z�mek p��znaku povolen� logov�n� u�ivatel�
	CReadWriteLock m_cUserLoginAllowedLock;
	// p��znak povolen� logov�n� u�ivatel�
	BOOL m_bUserLoginAllowed;

// Pomocn� data

	// pole p��znak� objeven� pozic na map� pro metodu "GetNearestUnitPlace"
	CArray2DOnDemand<BYTE> m_cGetNearestUnitPlacePositionArray;
};

//////////////////////////////////////////////////////////////////////
// Glob�ln� prom�nn�
//////////////////////////////////////////////////////////////////////

// Mapa hry
extern CSMap g_cMap;

//////////////////////////////////////////////////////////////////////
// Operace s mapou
//////////////////////////////////////////////////////////////////////

// vr�t� MapSquare na pozici "pointPosition"
inline CSMapSquare *CSMap::GetMapSquareFromPosition ( CPointDW pointPosition ) const 
{
	ASSERT ( IsMapPosition ( pointPosition ) );

	// vr�t� MapSquare na pozici "pointPosition"
	return &m_pMapSquares[( pointPosition.x / MAP_SQUARE_SIZE ) + 
		( pointPosition.y / MAP_SQUARE_SIZE ) * m_sizeMapMapSquare.cx];
}

// vr�t� index MapSquaru na pozici "pointPosition"
inline CPointDW CSMap::GetMapSquareIndex ( CPointDW pointPosition ) const 
{
	ASSERT ( IsMapPosition ( pointPosition ) );

	// vr�t� index MapSquaru na pozici "pointPosition"
	return CPointDW ( pointPosition.x / MAP_SQUARE_SIZE, 
		pointPosition.y / MAP_SQUARE_SIZE );
}

// vr�t� MapSquare s indexem "pointIndex"
inline CSMapSquare *CSMap::GetMapSquareFromIndex ( CPointDW pointIndex ) const 
{
	ASSERT ( pointIndex.x < m_sizeMapMapSquare.cx );
	ASSERT ( pointIndex.y < m_sizeMapMapSquare.cy );

	// vr�t� MapSquare s indexem "pointIndex"
	return &m_pMapSquares[pointIndex.x + pointIndex.y * m_sizeMapMapSquare.cx];
}

// vr�t� MapSquare s indexem "pointIndex" nebo NULL
inline CSMapSquare *CSMap::GetMapSquareFromIndexNull ( CPointDW pointIndex ) const 
{
	// zjist�, jedn�-li se o korektn� MapSquare
	if ( ( pointIndex.x < m_sizeMapMapSquare.cx ) && ( pointIndex.y < 
		m_sizeMapMapSquare.cy ) )
	{	// jedn� se o korektn� MapSquare
		// vr�t� MapSquare s indexem "pointIndex"
		return &m_pMapSquares[pointIndex.x + pointIndex.y * m_sizeMapMapSquare.cx];
	}
	else
	{	// nejedn� se o korektn� MapSquare
		// vr�t� p��znak nekorektn�ho MapSquaru
		return NULL;
	}
}

//////////////////////////////////////////////////////////////////////
// Informace o h�e
//////////////////////////////////////////////////////////////////////

// zjist�, je-li pozice "pointPosition" na map�
inline BOOL CSMap::IsMapPosition ( CPointDW pointPosition ) const 
{
	// zjist�, je-li pozice na map�
	if ( pointPosition.x != NO_MAP_POSITION )
	{	// jedn� se o pozici na map�
		ASSERT ( pointPosition.x < m_sizeMapMapCell.cx );
		ASSERT ( pointPosition.y < m_sizeMapMapCell.cy );

		// vr�t� p��znak pozice na map�
		return TRUE;
	}
	else
	{	// jedn� se o pozici mimo mapu
		ASSERT ( pointPosition.y == NO_MAP_POSITION );
		// vr�t� p��znak pozice mimo mapu
		return FALSE;
	}
}

// zkontroluje, je-li nahran� pozice "pointPosition" na map�
// v�jimky: CPersistentLoadException
inline BOOL CSMap::CheckLoadedMapPosition ( CPointDW pointPosition ) const 
{
	// zjist�, je-li pozice na map�
	if ( pointPosition.x != NO_MAP_POSITION )
	{	// jedn� se o pozici na map�
		LOAD_ASSERT ( pointPosition.x < m_sizeMapMapCell.cx );
		LOAD_ASSERT ( pointPosition.y < m_sizeMapMapCell.cy );

		// vr�t� p��znak pozice na map�
		return TRUE;
	}
	else
	{	// jedn� se o pozici mimo mapu
		LOAD_ASSERT ( pointPosition.y == NO_MAP_POSITION );
		// vr�t� p��znak pozice mimo mapu
		return FALSE;
	}
}

//////////////////////////////////////////////////////////////////////
// Operace s jednotkou mapy
//////////////////////////////////////////////////////////////////////

// zjist�, lze-li jednotku "pUnit" um�stit na pozici "pointPosition" (je-li nastaven 
//		p��znak "bMapSquaresLocked", nezamyk� MapSquary) (nen� vhodn� pro testov�n� v�ce 
//		pozic um�st�n� jedn� jednotky)
inline BOOL CSMap::CanPlaceUnitOnce ( CSUnit *pUnit, CPointDW pointPosition, 
	BOOL bMapSquaresLocked ) 
{
	ASSERT ( IsMapPosition ( pointPosition ) );

	// zjist�, m�-li jednotka vzdu�nou vertik�ln� pozici
	if ( pUnit->GetVerticalPosition () >= AIR_VERTICAL_POSITION )
	{	// jednotka m� vzdu�nou vertik�ln� pozici
		// jednotku lze um�stit na po�adovanou pozici
		return TRUE;
	}

	// zjist�, lze-li jednotku um�stit na pozici
	BOOL bCanPlaceUnit = CSMapSquare::MainLoopCanPlaceUnit ( pUnit, pointPosition, 
		bMapSquaresLocked );

	// ukon�� um�s�ov�n� jednotky
	CSMapSquare::MainLoopFinishPlacingUnit ();

	// vr�t� p��znak um�st�n� jednotky
	return bCanPlaceUnit;
}

//////////////////////////////////////////////////////////////////////
// Operace ��zen� hry (TimeSlicy a pausov�n� hry)
//////////////////////////////////////////////////////////////////////

// usp� volaj�c� thread a vr�t� p��znak pokra�ov�n� odpausovan� hry (TRUE=hra m� 
//		pokra�ovat)
inline BOOL CSMap::CanContinueGame () 
{
	// po�k� na ud�lost odpausov�n� hry
	VERIFY ( m_eventUnpauseGame.Lock () );

	// vr�t� p��znak pokra�ov�n� odpausovan� hry
	return m_bContinueUnpausedGame;
};

#endif //__SERVER_MAP__HEADER_INCLUDED__
