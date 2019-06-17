// ZCivilization.h: interface for the CZCivilization class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_ZCIVILIZATION_H__32218D30_ED20_11D3_AFF3_004F49068BD6__INCLUDED_)
#define AFX_ZCIVILIZATION_H__32218D30_ED20_11D3_AFF3_004F49068BD6__INCLUDED_
                                
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "GameServer\GameServer\GeneralClasses\GeneralClasses.h"

#include "TripleS\CompilerIO\Src\CompilerInputFile.h"
#include "TripleS\CompilerIO\Src\CompilerErrorOutputMemory.h"
#include "TripleS\Interpret\Src\IBag.h"

#define CIVILIZATION_LOG_FILENAME   "Errors.log"

#define STARTUP_COMMANDER_NAME "StartupCommander"

// priznak konce hry
extern bool g_bGameClosing;
extern bool g_bGameLoading;

// log file object
class CLogFile;
class CInterpret;
class CCodeManager;
class CCompiler;
class CIUnitType;
class CZConnectedClient;
class CZUnit;
class CZClientUnitInfoSender;
class CZUnitListItem;
class CSCivilization;
class CSUnit;
class CZCivilizationMapObserver;

class CZCivilization : public CNotifier
{
    friend CZUnit;
    friend CInterpret;
    friend CCodeManager;

// Vitek begin
  // declare the observer map to recieve the events
  DECLARE_OBSERVER_MAP(CZCivilization);
// Vitek end

public:
	CZCivilization();
	virtual ~CZCivilization();

	// inicializace a zniceni statickych polozek
	static void CreateStatic();
    static void PreDeleteStatic();
	static void DeleteStatic();
	static void PersistentSaveStatic ( CPersistentStorage & storage );
	static void PersistentLoadStatic ( CPersistentStorage & storage );
	static void PersistentTranslatePointersStatic ( CPersistentStorage & storage );
	static void PersistentInitStatic ();

public:
    // vytvoreni civilizace
    // throws CException, CStringException.. 
    void Create( CString &strScriptSetName, CSCivilization *pSCivilization);
    void PreDelete();
    void Delete();

    void AddRefUnits( CZUnit *pUnit);
	void PrepareToDeleteUnit(CZUnit *pUnit);

// Vitek begin
    // connects new client
    CZConnectedClient * ConnectClient(CString strUserName);
    // disconnects the client
    void DisconnectClient(CZConnectedClient *pClient);
    // disconnects the client by user name
    void DisconnectUser(CString strUserName);

    // These functions can be used only under the Connected Clients lock
    CZConnectedClient * GetConnectedClient(CString strUserName);
    CZConnectedClient * GetFirstConnectedClient(){ return m_pConnectedClients; }
    CZConnectedClient * GetNextConnectedClient(CZConnectedClient *pClient);

    void LockConnectedClients(){ VERIFY(m_lockConnectedClients.Lock()); }
    void UnlockConnectedClients(){ VERIFY(m_lockConnectedClients.Unlock()); }

    // returns the interpret object
    CInterpret *GetInterpret(){ return m_pInterpret; }
    // returns the compiler object
    CCompiler *GetCompiler(){ return m_pCompiler; }
    // returns the code manager object
    CCodeManager *GetCodeManager(){ return m_pCodeManager; }

    // returns pointer to SCivilization object associated with this one
    CSCivilization *GetSCivilization(){ return m_pSCivilization; }

    // Returns number of units in this civilization - unit hierarchy must be locked
    ULONG GetUnitCount(){ return (ULONG)m_nUnitCount; }

    // Returns the general commander - unit hierarchy must be locked
    CZUnit *GetGeneralCommander(){ return m_pGeneralCommander; }

    // Returns unit by its ID - unit hierarchy must be locked
    CZUnit *GetUnitByID(DWORD dwID){ CZUnit *pUnit;
      if(!m_mapUnitsByIDs.Lookup(dwID, pUnit)) return NULL; else return pUnit; }

    // Vitek end

// *ROMAN v--v
	// Zaregistruje u civilizace objekt pro komunikaci s klientem
	void RegisterClientUnitInfoSender(CZClientUnitInfoSender *pSender) {}
// *ROMAN ^--^

protected:
// DULEZITE PODOBJEKTY

    // Compiler
    CCompiler *m_pCompiler;

    // Interpret  (musi byt dynamicky, kvuli zacyklenym headerum)
    CInterpret  *m_pInterpret;
    HANDLE m_hInterpretThreadHandle;
	DWORD m_nInterpretThreadID;

    // Code Manager (musi byt dynamicky, kvuli zacyklenym headerum)
    CCodeManager *m_pCodeManager;

    // Event Manager - smycka vyzvedavani bezi ve threadu interpretu
    CEventManager m_EventManager;

// DULEZITE POINTERY
    CSCivilization *m_pSCivilization;

// ROOT CIVILIZACE
    // root path
    CString m_strRootPath;

    // root archive
    CDataArchive *m_pRootDataArchive;

// SCRIPT SET
    CString m_strScriptSetName, m_strScriptSetPath;

// LOG FILE
    // log file in archive
    CArchiveFile m_LogArchiveFile;

    // log file
    CLogFile *m_pLogFileObject;

// PAUSE GAME EVENTS
    CEvent m_GamePausedEvent;   // preda se serveru a ten an ni ceka, az se zapauzuje
    CEvent m_GameUnpauseEvent;  // na tomhle ceka interpret, kdyz je zapauzovano
public:
    CLogFile *GetLogFile() { return m_pLogFileObject; }

protected:
    // Jednotky
    CZUnit* m_pGeneralCommander;
	 ULONG m_nUnitCount;

    // Map units and unit IDs map
    CMap<DWORD, DWORD, CZUnit *, CZUnit *> m_mapUnitsByIDs;

public:
// METODY
    // vraci jmeno civilizace
    CString GetCivilizationName();

    // vraci index civilizace
    DWORD GetCivilizationIndex();

    // hlaseni chyb
    void ReportError(LPCTSTR strErrorMsgFormat, ...);
    void ReportError(LPCTSTR strErrorMsgFormat, va_list args);

// METODY PRO SERVER
public:
    // Vytvoreni jednotky. NULL znamena 'stala se chyba'.
    CZUnit* CreateUnit( CSUnit *pSUnit,  CZUnit *pCommander = NULL, LPCTSTR lpszScriptName = NULL,
                            bool bPhysicalUnit = true);

    // Umreni jednotky
    void UnitDied( CZUnit *pZUnit, CPointDW& Position);

    // Zapauzovani civilizace (interpreta). Vraci eventu, ktera kdyz je nahozena, tak
    // je zapauzovano.
    CEvent* Pause();
    // Odpauzovani civilizace. Pokud bContinue == false, thread interpreta je 
    // ukoncen a musi byt v blizke budoucnosti zavolano Delete na civilizaci.
    void Unpause( bool bContinue);

    // Tohle vola server kdyz je dokoncena inicializace hry.
    // Posila to zpravu (notification) GAME_INITIALIZED hlavnimu veliteli.
    void OnGameInitialized();

// SPUSTENI INTERPRETA - thread interpretu

    static UINT RunInterpret( LPVOID pCivilization);

//                               
// PRO INTERPRET
//
public:
    CZUnitListItem* GetAllUnitsOfTypeAndChildren( CIUnitType *pIUnitType);

protected:
    CZUnitListItem* GetAllUnitsOfTypeAndChildren_HelperResursiveMethod( CZUnit *pUnit,
                                                                        CIUnitType *pISearchedUnitType, 
                                                                        CZUnitListItem *pUnitListItem);

//
// ZAMKY pro civilizace a jednotky (CZUnit)
//
protected:
    // Zamek nad vsim, co se dela s vojenskou hierarchii
    CCriticalSection m_lockHierarchy;

    // Zamek nad vsim, co se dela s frontama zprav u jednotky
    CCriticalSection m_lockEvents;

    // Zamek na logfile
    CCriticalSection m_lockLogFile;

    // Zamek na misc. data u jednotky
    // (Name, Status, m_pIUnitType)
    CCriticalSection m_lockUnitData;

public:
    // Zamceni a odemceni vojenske hierarchie (a zaroven eventu, jako prevence proti deadlocku)
    // !!!!!
    // !!!!! Pokud potrebujete zaraz zamknout i misc data
    // tak HierarchyAndEvent se zamyka prvni
    void LockHierarchyAndEvents()   
    { 
        VERIFY( m_lockEvents.Lock() );
        VERIFY( m_lockHierarchy.Lock() ); 
    }
    void UnlockHierarchyAndEvents() 
    { 
        VERIFY( m_lockHierarchy.Unlock() ); 
        VERIFY( m_lockEvents.Unlock() );
    }

    // Zamceni a odemceni nad frontama zprav
    void LockEvents()   { VERIFY( m_lockEvents.Lock() ); }
    void UnlockEvents() { VERIFY( m_lockEvents.Unlock() ); }

    // Zamceni a odemceni logfile
    void LockLogFile()   { VERIFY( m_lockLogFile.Lock() ); }
    void UnlockLogFile() { VERIFY( m_lockLogFile.Unlock() ); }

    // Zamkne misc data jednotky (Name)
    // !!!!!
    // !!!!! pokud potrebujete zamknout zaraz i Hierarchy
    // tak Hierarchy se zamyka prvni... (tedy zamyka se pomoci LockEventsAndHierarchy
    // a UnlockEventsAndHierarchy).
    void LockUnitData()  { VERIFY( m_lockUnitData.Lock() ); }
    void UnlockUnitData() { VERIFY( m_lockUnitData.Unlock() ); }

    // Lock the ISOClients
    void LockISOClients() { VERIFY( m_lockISOClients.Lock() ); }
    void UnlockISOClients() { VERIFY(m_lockISOClients.Unlock() ) ; }

// Vitek begin
// Reactions on network events
protected:

  // helper variable - ID of the thread from where we was created -> this one will recieve our events
  DWORD m_dwNetworkEventLoopThreadID;

private:
  // lock for the connected clients list
  CCriticalSection m_lockConnectedClients;
  // list of connected clients
  CZConnectedClient *m_pConnectedClients;

  // lock for the ISOClients in all connected clients
  // (must be here - deadlocks)
  CCriticalSection m_lockISOClients;

  enum{ ID_CONNECTEDCLIENT = 0x200, };

// Vitek end

// ID of units - must be unique (better accross all civilizations)
  static DWORD m_dwNewZUnitID;

//
// KONSTANTNI STRINGY - nazvy notifikaci a tak
//

  static CStringTableItem* m_stiGAME_INITIALIZED;
  static CStringTableItem* m_stiUNIT_DIED;
  static CStringTableItem* m_stiUNIT_UPDATE;
  static CStringTableItem* m_stiINFERIOR_CREATED;
  static CStringTableItem* m_stiINFERIOR_ADDED;
  static CStringTableItem* m_stiINFERIOR_REMOVED;
  static CStringTableItem* m_stiINFERIOR_DIED;
  static CStringTableItem* m_stiCOMMANDER_CHANGED;
  static CStringTableItem* m_stiIDLE;
  static long m_nCivilizationInstancesCount;


private:
  // The only instance of the map observer
  static CZCivilizationMapObserver * m_pMapObserver;

public:
  // Sets the timer to send ON_TIMER notification to specified unit
  // at given timeslice with specified user ID
  // Can be called multiple times for one unit
  static void SetTimer ( CZUnit *pUnit, int nUserID, DWORD dwTimeslice );

  // Sets the timer to wakeup given syscall when it elapses
  static void SetTimer ( CISyscall * pSyscall, DWORD dwTimeslice );

public:
//
//	SAVE & LOAD
//
	void PersistentSave( CPersistentStorage &storage);
	void PersistentLoad( CPersistentStorage &storage, CString &strCivilizationName);
	void PersistentTranslatePointers( CPersistentStorage &storage);
	void PersistentInit();

	void PersistentSaveUnit( CPersistentStorage &storage, CZUnit *pUnit);

	// naloadovane jednotky - plni se v ParsistentLoad a uvolnuje se v PersistentTranslatePointers
	CZUnit **m_pLoadedUnits;

//
// PROZATIMNI - vetsinou pro ProjektApp aplikaci
//
    // vstup a vystupy prekladace
    CCompilerInputFile m_StdIn;
    CCompilerErrorOutputMemory m_StdErr, m_DbgOut;


// diry pro kontrolovani zvenku - pro ladeni
    void Compile(CCompilerInput *pStdIn, CCompilerErrorOutput *pStdErr, 
                 bool bUpdateCodeOnSuccess,  bool bDebugOutput=false, 
				 CCompilerErrorOutput *pDbgOut=NULL, bool bDebugParser=false);

};

#endif // !defined(AFX_ZCIVILIZATION_H__32218D30_ED20_11D3_AFF3_004F49068BD6__INCLUDED_)
