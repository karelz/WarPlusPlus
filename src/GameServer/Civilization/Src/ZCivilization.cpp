// ZCivilization.cpp: implementation of the CZCivilization class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include "..\ZCivilization.h"
#include "ZUnit.h"
#include "ZConnectedClient.h"
#include "GameServer\GameServer\SMap.h"

#include "GameServer\GameServer\SCivilization.h"
#include "GameServer\GameServer\SFileManager.h"

#include "Triples\Interpret\Src\SysObjs\SystemObjects.h"

#include "LogFile\LogFile.h"

#include "GameServer\GameServer\SMap.h"

// Vitek begin
BEGIN_OBSERVER_MAP(CZCivilization, CNotifier)
case -1: break;
END_OBSERVER_MAP(CZCivilization, CNotifier)
// Vitek end

#define TIMER_ELAPSED_NOTIFICATION "TIMER"

//////////////////////////////////////////////////////////////////////
// Makra pro trasování
//////////////////////////////////////////////////////////////////////

#ifdef _DEBUG
	extern BOOL g_bTraceLoggedErrors;

	#define TRACE_LOGGEDERROR if ( g_bTraceLoggedErrors ) TRACE_NEXT 
	#define TRACE_LOGGEDERROR0(text) do { if ( g_bTraceLoggedErrors ) { TRACE0_NEXT(text); } } while ( 0 )
	#define TRACE_LOGGEDERROR1(text,p1) do { if ( g_bTraceLoggedErrors ) { TRACE1_NEXT(text,p1); } } while ( 0 )
	#define TRACE_LOGGEDERROR2(text,p1,p2) do { if ( g_bTraceLoggedErrors ) { TRACE2_NEXT(text,p1,p2); } } while ( 0 )
	#define TRACE_LOGGEDERROR3(text,p1,p2,p3) do { if ( g_bTraceLoggedErrors ) { TRACE3_NEXT(text,p1,p2,p3); } } while ( 0 )
#else //!_DEBUG
	#define TRACE_LOGGEDERROR TRACE
	#define TRACE_LOGGEDERROR0 TRACE0
	#define TRACE_LOGGEDERROR1 TRACE1
	#define TRACE_LOGGEDERROR2 TRACE2
	#define TRACE_LOGGEDERROR3 TRACE3
#endif //!_DEBUG

//////////////////////////////////////////////////////////////////////

CStringTableItem* CZCivilization::m_stiGAME_INITIALIZED = NULL;
CStringTableItem* CZCivilization::m_stiUNIT_DIED = NULL;
CStringTableItem* CZCivilization::m_stiUNIT_UPDATE = NULL;
CStringTableItem* CZCivilization::m_stiINFERIOR_CREATED = NULL;
CStringTableItem* CZCivilization::m_stiINFERIOR_ADDED = NULL;
CStringTableItem* CZCivilization::m_stiINFERIOR_REMOVED = NULL;
CStringTableItem* CZCivilization::m_stiINFERIOR_DIED = NULL;
CStringTableItem* CZCivilization::m_stiCOMMANDER_CHANGED = NULL;
CStringTableItem* CZCivilization::m_stiIDLE = NULL;
long CZCivilization::m_nCivilizationInstancesCount = 0;

DWORD CZCivilization::m_dwNewZUnitID = 1;

bool g_bGameClosing = false;
bool g_bGameLoading = false;

// Class for receiving map notifications (like timeslice tick)
class CZCivilizationMapObserver : public CSMap::CSMapObserver
{
public:
	typedef struct tagSTimerNode
	{
		// Only one of these can be != NULL
		// Unit for which the node is registered or NULL if it's syscall timer
		CZUnit * m_pUnit;
		// Syscall to wake up when timeout elapses or NULL if it's notification timer
		CISyscall * m_pSyscall;

		// User specified ID of the timer node (used only in notification timer)
		int m_nUserID;

		// Timeslice in which the timeout is elapsed
		DWORD m_dwTimeslice;
		
		// Next timer node in the list
		struct tagSTimerNode * m_pNext;
	} STimerNode;
	
	CZCivilizationMapObserver ();
	virtual ~CZCivilizationMapObserver ();
	
	void Create ();
	virtual void Delete ();
	
	// Sets the timer to send ON_TIMER notification to specified unit
	// at given timeslice with specified user ID
	// Can be called multiple times for one unit
	void SetTimer ( CZUnit *pUnit, int nUserID, DWORD dwTimeslice );

	// Sets the timer to wake given syscall at given timeslice
	void SetTimer ( CISyscall *pSyscall, DWORD dwTimeslice );
	
	// Persistent load/save
	void PersistentSave( CPersistentStorage &storage);
	void PersistentLoad( CPersistentStorage &storage);
	void PersistentTranslatePointers( CPersistentStorage &storage);
	void PersistentInit();
	
protected:
	virtual void OnTimeslice ( DWORD dwTimeslice );
	virtual void OnUserLoggedOn ( DWORD dwUserID ) {};
	virtual void OnUserLoggedOff ( DWORD dwUserID ) {};
	
private:
	// Pool for the timer nodes
	static CTypedMemoryPool<STimerNode> m_cTimerNodesPool;
	// list of registered timer nodes
	STimerNode * m_pTimerNodesList;
	// Lock for the timer structures exclusive access
	CMutex m_lockTimer;
	
	// The ON_TIMER sti
	CStringTableItem * m_stiON_TIMER;

	// Helper function to insert new timer nodes to the sorted list
	// The timer lock must be locked
	void InsertNewNode ( STimerNode * pNewNode );
};

CZCivilizationMapObserver * CZCivilization::m_pMapObserver = NULL;

//////////////////////////////////////////////////////////////////////


UINT CZCivilization::RunInterpret( LPVOID pCivilization)
{
    ASSERT( pCivilization != NULL);
    ASSERT( ((CZCivilization*)pCivilization)->m_pInterpret != NULL);

    // attach eventmanager
    ((CZCivilization*)pCivilization)->m_EventManager.AttachToThread();

#ifdef _DEBUG
    // zapamatuj thread ID
    ((CZCivilization*)pCivilization)->m_pInterpret->m_dwInterpretThreadID = AfxGetThread()->m_nThreadID;
#endif //_DEBUG

    // Zapauzovat interpret 
    // Odpauzovani == CZCivilization::Unpause.
    SIPauseInterpret *pPauseInterpret = new SIPauseInterpret;
    pPauseInterpret->m_pIntepretPaused = &((CZCivilization*)pCivilization)->m_GamePausedEvent;
    pPauseInterpret->m_pFinishPause = &((CZCivilization*)pCivilization)->m_GameUnpauseEvent;

    pPauseInterpret->m_pIntepretPaused->ResetEvent();    
    pPauseInterpret->m_pFinishPause->ResetEvent();
    
    ((CZCivilization*)pCivilization)->m_pInterpret->OnPauseInterpret( pPauseInterpret);

    // Rozbehnout interpret
    ((CZCivilization*)pCivilization)->m_pInterpret->Run();

    return 0;
}

//////////////////////////////////////////////////////////////////////

CZCivilization::CZCivilization()
{
    m_pInterpret = NULL;
    m_pCodeManager = NULL;
    m_hInterpretThreadHandle = NULL;

    m_pGeneralCommander = NULL;
    m_nUnitCount = 0;

	m_pLoadedUnits = NULL;

    m_pRootDataArchive = NULL;

    m_pSCivilization = NULL;

	 m_pLogFileObject = NULL;
	 m_pCompiler = NULL;

// Vitek
    m_pConnectedClients = NULL;

}

CZCivilization::~CZCivilization()
{
}

void CZCivilization::CreateStatic()
{
    ASSERT( sizeof(bool) == sizeof(BYTE));

    g_bGameLoading = false;

	// INICIALIZACE KONSTANTNICH STRINGU
	ASSERT( m_stiGAME_INITIALIZED == NULL);
	
	m_stiGAME_INITIALIZED = g_StringTable.AddItem("GAME_INITIALIZED");
	m_stiUNIT_DIED        = g_StringTable.AddItem("UNIT_DIED");
	m_stiUNIT_UPDATE	  = g_StringTable.AddItem("UNIT_UPDATE");
	m_stiINFERIOR_CREATED = g_StringTable.AddItem("INFERIOR_CREATED");
	m_stiINFERIOR_ADDED   = g_StringTable.AddItem("INFERIOR_ADDED");
	m_stiINFERIOR_REMOVED = g_StringTable.AddItem("INFERIOR_REMOVED");
	m_stiINFERIOR_DIED    = g_StringTable.AddItem("INFERIOR_DIED");
	m_stiCOMMANDER_CHANGED= g_StringTable.AddItem("COMMANDER_CHANGED");
    m_stiIDLE             = g_StringTable.AddItem("IDLE");

    ASSERT ( m_pMapObserver == NULL );
    m_pMapObserver = new CZCivilizationMapObserver ();
    m_pMapObserver->Create ();

    if (CCodeManager::m_pDefaultUnitType == NULL)
    {
        CCodeManager::LoadDefaultUnit();
        ASSERT( CCodeManager::m_pDefaultUnitType != NULL);
    }
}

void CZCivilization::PreDeleteStatic ()
{
	if ( m_pMapObserver != NULL )
	{
		m_pMapObserver->Delete();
		delete m_pMapObserver;
		m_pMapObserver = NULL;
	}
}

void CZCivilization::DeleteStatic()
{
   	// uvolnit defaultni jednotku
    ASSERT( CCodeManager::m_nCodeManagersCount == 0);
    if ( CCodeManager::m_pDefaultUnitType != NULL)
    {
        delete CCodeManager::m_pDefaultUnitType;
	    CCodeManager::m_pDefaultUnitType = NULL;
    }

	// UVOLNENI KONSTANTNICH STRINGU
	ASSERT( m_nCivilizationInstancesCount == 0);

	if ( m_stiGAME_INITIALIZED != NULL ) m_stiGAME_INITIALIZED->Release(), m_stiGAME_INITIALIZED = NULL;
	if ( m_stiUNIT_DIED != NULL ) m_stiUNIT_DIED->Release(), m_stiUNIT_DIED = NULL;	
	if ( m_stiUNIT_UPDATE != NULL ) m_stiUNIT_UPDATE->Release(), m_stiUNIT_UPDATE = NULL;
	if ( m_stiINFERIOR_CREATED != NULL ) m_stiINFERIOR_CREATED->Release(), m_stiINFERIOR_CREATED = NULL;
	if ( m_stiINFERIOR_ADDED != NULL ) m_stiINFERIOR_ADDED->Release(),  m_stiINFERIOR_ADDED = NULL;	
	if ( m_stiINFERIOR_REMOVED != NULL )  m_stiINFERIOR_REMOVED->Release(), m_stiINFERIOR_REMOVED = NULL;
	if ( m_stiINFERIOR_DIED != NULL ) m_stiINFERIOR_DIED->Release(), m_stiINFERIOR_DIED = NULL;	
	if ( m_stiCOMMANDER_CHANGED != NULL ) m_stiCOMMANDER_CHANGED->Release(), m_stiCOMMANDER_CHANGED = NULL;	
    if ( m_stiIDLE != NULL) m_stiIDLE->Release(), m_stiIDLE = NULL;
}

void CZCivilization::Create( CString &strScriptSetName, CSCivilization *pSCivilization)
{
	// global flag for events
	g_bGameClosing = false;

    // global flag pro error handling when loading from savegame
    g_bGameLoading = false;

	ASSERT( m_stiGAME_INITIALIZED != NULL);

    ASSERT(m_pInterpret == NULL);
    ASSERT(m_pCodeManager == NULL);
    ASSERT(!strScriptSetName.IsEmpty());

    ASSERT(pSCivilization != NULL);

    TRACE_INTERPRET1("- Civilization \"%s\": creating...\n", (pSCivilization != NULL) ? 
            pSCivilization->GetCivilizationName() : "[unknown]");

    try
    {
    // Vitek begin
        m_dwNetworkEventLoopThreadID = GetCurrentThreadId();
    // Vitek end

        // SCivilization
        m_pSCivilization = pSCivilization;

        // Root adresar civilizace
        m_strRootPath = g_cSFileManager.GetCivilizationDirectory( pSCivilization->GetCivilizationName());

        // Open Root Data Archive
        m_pRootDataArchive = new CDataArchive;
        if (!m_pRootDataArchive->Create( m_strRootPath, CArchiveFile::modeReadWrite, 
                                         CDataArchiveInfo::archiveDirectory)) 
        {
            delete m_pRootDataArchive;
            throw new CStringException("Can't open archive %s.\n", m_strRootPath);
        }

        // LogFile
        m_LogArchiveFile = m_pRootDataArchive->CreateFile(CIVILIZATION_LOG_FILENAME, 
            CArchiveFile::modeCreate | CArchiveFile::modeReadWrite | 
            CArchiveFile::modeUncompressed | CFile::shareDenyWrite | CFile::typeText);

        m_pLogFileObject = new CLogFile;
        m_pLogFileObject->Create(m_LogArchiveFile);

        // Script Set Name and Path
        m_strScriptSetName = strScriptSetName;
        m_strScriptSetPath = g_cSFileManager.GetScriptSetDirectory( strScriptSetName);

        // Event Manager
        m_EventManager.Create();

        // Intepret
        m_pInterpret = new CInterpret();
        m_pCodeManager = new CCodeManager;

        m_pInterpret->Create( this, &m_EventManager);

        // Pripojit se na interpreta.
        Connect( m_pInterpret, CIVILIZATION_OR_INTERPRET_NOTIFIER_ID);

        // Code Manager
        m_pCodeManager->Create( m_strRootPath, m_strScriptSetPath, this);

        // Vytvoreni startup velitele
        m_pGeneralCommander = CreateUnit( NULL, NULL, STARTUP_COMMANDER_NAME, false);
        if (m_pGeneralCommander == NULL)
        {
            throw CStringException("Fatal error: Default unit parent (%s) not found.", DEFAULT_UNIT_PARENT);
        }

        m_nUnitCount = 1;

        // Compiler
		  m_pCompiler = new CCompiler;
        m_pCompiler->Create( m_pCodeManager->GetCodeManagerForCompiler());

        // Spusteni intepreta - v novem threadu
        CWinThread *pInterpretThread = ::AfxBeginThread( RunInterpret, this);
		m_hInterpretThreadHandle = pInterpretThread->m_hThread;
		m_nInterpretThreadID = pInterpretThread->m_nThreadID;

		::InterlockedIncrement( &m_nCivilizationInstancesCount);

        TRACE_INTERPRET2("- Civilization \"%s\": created, ID = %d\n", GetCivilizationName(), GetCivilizationIndex());
    }
    catch (...)
    {
        TRACE_INTERPRET1("- !!! Failed to create civilization \"%s\".\n", (m_pSCivilization != NULL) ? 
            m_pSCivilization->GetCivilizationName() : "[unknown]");

		if (m_pLogFileObject != NULL)
		{
			m_pLogFileObject->Delete();
			delete m_pLogFileObject;
			m_pLogFileObject = NULL;
		}

        if (m_pInterpret != NULL)
        {
    		Disconnect( m_pInterpret);
            m_pInterpret->Delete();
            delete m_pInterpret;
			m_pInterpret = NULL;
        }

        if (m_pCodeManager != NULL)
        {
            m_pCodeManager->Delete();
		    delete m_pCodeManager;
			m_pCodeManager = NULL;
        }

        m_EventManager.Delete();

        throw;
    }
}

void CZCivilization::PreDelete()    
{
    // Osetreni padu pri loadovani hry
    if ( g_bGameLoading)
    {
        try
        {
            // interpret
            if ( m_pInterpret != NULL)
            {
                m_pInterpret->Delete();
            }
    
            // jednotky
            if (m_pLoadedUnits != NULL) 
            {
					 ULONG i;
    				 for (i=0; i<m_nUnitCount; i++) delete m_pLoadedUnits[i];
	    			 delete [] m_pLoadedUnits;
                m_pLoadedUnits = NULL;
            }

            // code manager
            if ( m_pCodeManager != NULL)
            {
                m_pCodeManager->Delete();
                delete m_pCodeManager;
                m_pCodeManager = NULL;
            }
        }
        catch (...)
        {
        }

        return;
    }

    // normalni PreDelete
    // Odpojeni od interpreta.
    Disconnect( m_pInterpret);

    // Ukonceni interpreta.
    m_EventManager.InlayQuitEvent(0);

    if (m_hInterpretThreadHandle != NULL)
    {
        ::WaitForSingleObject(m_hInterpretThreadHandle, INFINITE);
        m_hInterpretThreadHandle = NULL;
    }

// Vitek
  // Disconnect all clients
    {
      VERIFY(m_lockConnectedClients.Lock());

      CZConnectedClient *pClient = m_pConnectedClients, *pDel;
      CEvent cEvent;

      while(pClient != NULL){
        pDel = pClient;
        pClient = pClient->m_pNext;

        // Just send an event to that one
        cEvent.ResetEvent ();
        InlayEvent ( CZConnectedClient::E_DisconnectClient, (DWORD)&cEvent, pClient );
        VERIFY ( cEvent.Lock () );
      }

      m_pConnectedClients = NULL;
 
      VERIFY(m_lockConnectedClients.Unlock());
    }
// Vitek end

    // Interpret
    m_pInterpret->Delete();

	// zamcit
	LockHierarchyAndEvents();

	// pre-delete jednotek
	if (m_pGeneralCommander != NULL)
    {
        AddRefUnits( m_pGeneralCommander);
        PrepareToDeleteUnit( m_pGeneralCommander);
    }
    m_pGeneralCommander = NULL;

	// odemcit
	UnlockHierarchyAndEvents();

    // Compiler
    if ( m_pCompiler != NULL )
    {
        m_pCompiler->Delete();
        delete m_pCompiler;
        m_pCompiler = NULL;
    }

    // Event Manager
    m_EventManager.Delete();

    // LogFile
	 if ( m_pLogFileObject != NULL )
	 {
		m_pLogFileObject->Delete();
		delete m_pLogFileObject;
		m_pLogFileObject = NULL;
	 }
    if (m_LogArchiveFile.IsOpened()) m_LogArchiveFile.Close();

    // Root civilizace
    if (m_pRootDataArchive != NULL) 
	{
		if( m_pRootDataArchive->IsOpened()) m_pRootDataArchive->Close();
		delete m_pRootDataArchive;
        m_pRootDataArchive = NULL;
	}

    m_strRootPath.Empty();

	::InterlockedDecrement( &m_nCivilizationInstancesCount);
}

void CZCivilization::Delete()
{
    // Osetreni padu pri loadovani hry
    if ( g_bGameLoading) return;

    // assert jestli bylo zavolano PreDelete
    ASSERT( m_pGeneralCommander == NULL);

   	// Uvolnit Code Manager
    m_pCodeManager->Delete();
    delete m_pCodeManager;
	m_pCodeManager = NULL;

    // uvolnit zbytek interpreta
   	delete m_pInterpret;
}    

void CZCivilization::AddRefUnits( CZUnit *pUnit)
{
    ASSERT( pUnit != NULL);

    CZUnit *pAct;
    
	// projit potomky
    if (pUnit->m_pFirstInferior != NULL)
    {
		pAct = pUnit->m_pFirstInferior;
		while (pAct != NULL)
		{
            AddRefUnits( pAct);
			pAct = pAct->m_pNextSibbling;
		}
    }

    pUnit->AddRef();
}

void CZCivilization::PrepareToDeleteUnit(CZUnit *pUnit)
{
	 ASSERT( pUnit != NULL);

    CZUnit *pPom, *pAct;
    
	 if ( pUnit->m_bPreDeleted)
	 {
		 ASSERT( pUnit->m_pFirstInferior == NULL);
		 pUnit->Release();
		 return;
	 }

	// projit potomky
    if (pUnit->m_pFirstInferior != NULL)
    {
		pAct = pUnit->m_pFirstInferior;
		while (pAct != NULL)
		{
			pPom = pAct;
			pAct = pAct->m_pNextSibbling;
			PrepareToDeleteUnit(pPom);
		}
    }

	pUnit->PreDelete();
	pUnit->Release();
}

// Vitek begin

CZConnectedClient * CZCivilization::ConnectClient(CString strUserName)
{
  // create the new connected client object
  CZConnectedClient *pClient = new CZConnectedClient();
  pClient->Create(strUserName, this);

  VERIFY(m_lockConnectedClients.Lock());
  // add it to our list
  pClient->m_pNext = m_pConnectedClients;
  m_pConnectedClients = pClient;

  VERIFY(m_lockConnectedClients.Unlock());

#ifndef PROJEKTAPP
  g_cMap.UserLoggedOn(strUserName);
#endif

  return pClient;
}

//extern BOOL g_bLoggedOn;

void CZCivilization::DisconnectClient(CZConnectedClient *pClient)
{
  CZConnectedClient *pHlp, *pPrev = NULL;
  
  
  VERIFY(m_lockConnectedClients.Lock());

  pHlp = m_pConnectedClients;

  // remove it from the list
  while(pHlp != NULL){
    if(pHlp == pClient) break;
    pPrev = pHlp;
    pHlp = pHlp->m_pNext;
  }

  if ( pHlp != pClient )
  {
	  VERIFY(m_lockConnectedClients.Unlock());
	  return;
  }

  if(pPrev == NULL){
    m_pConnectedClients = pClient->m_pNext;
  }
  else{
    pPrev->m_pNext = pClient->m_pNext;
  }

  VERIFY(m_lockConnectedClients.Unlock());

  CString strUserName = pClient->GetUserName();

  // delete it
  pClient->Delete();
  delete pClient;

#ifndef PROJEKTAPP
 	g_cMap.UserLoggedOff(strUserName);
#endif

// small hack
//  g_bLoggedOn = FALSE;
}

void CZCivilization::DisconnectUser( CString strUserName )
{
  LockConnectedClients();

  // Find the right client
  CZConnectedClient *pClient;

  pClient = m_pConnectedClients;
  while(pClient != NULL){
    if(pClient->GetUserName() == strUserName) break;

    pClient = pClient->m_pNext;
  }

  if(pClient == NULL){
    // The user is not connected to this civilization
    UnlockConnectedClients();
    return;
  }

  // disconnect the user's client
  {
	  CEvent cEvent;
	  // Just send an event to that one
	  cEvent.ResetEvent ();
	  InlayEvent ( CZConnectedClient::E_DisconnectClient, (DWORD)&cEvent, pClient );
	  VERIFY ( cEvent.Lock () );
  }

  UnlockConnectedClients();
}

CZConnectedClient *CZCivilization::GetConnectedClient(CString strUserName)
{
  CZConnectedClient *pClient;

  pClient = m_pConnectedClients;
  while(pClient != NULL){
    if(pClient->GetUserName() == strUserName) return pClient;
    pClient = pClient->m_pNext;
  }

  return pClient;
}

CZConnectedClient *CZCivilization::GetNextConnectedClient(CZConnectedClient *pClient)
{
  return pClient->m_pNext;
}
// Vitek end

//////////////////////////////////////////////////////////////////////
//	SAVE & LOAD
//////////////////////////////////////////////////////////////////////

void CZCivilization::PersistentSaveStatic ( CPersistentStorage & storage )
{
	BRACE_BLOCK ( storage );

    // Save the contant strings
    storage << (DWORD)m_stiGAME_INITIALIZED;
	storage << (DWORD)m_stiUNIT_DIED;
	storage << (DWORD)m_stiUNIT_UPDATE;
	storage << (DWORD)m_stiINFERIOR_CREATED;
	storage << (DWORD)m_stiINFERIOR_ADDED;
	storage << (DWORD)m_stiINFERIOR_REMOVED;
	storage << (DWORD)m_stiINFERIOR_DIED;
	storage << (DWORD)m_stiCOMMANDER_CHANGED;
    storage << (DWORD)m_stiIDLE;

	// Save the map observer
	ASSERT ( m_pMapObserver != NULL );
	m_pMapObserver->PersistentSave ( storage );
}

void CZCivilization::PersistentLoadStatic ( CPersistentStorage & storage )
{
    g_bGameLoading = true;

	BRACE_BLOCK ( storage );

    // Load the contant strings
    storage >> (DWORD&)m_stiGAME_INITIALIZED;
	storage >> (DWORD&)m_stiUNIT_DIED;
	storage >> (DWORD&)m_stiUNIT_UPDATE;
	storage >> (DWORD&)m_stiINFERIOR_CREATED;
	storage >> (DWORD&)m_stiINFERIOR_ADDED;
	storage >> (DWORD&)m_stiINFERIOR_REMOVED;
	storage >> (DWORD&)m_stiINFERIOR_DIED;
	storage >> (DWORD&)m_stiCOMMANDER_CHANGED;
    storage >> (DWORD&)m_stiIDLE;

    // Load the map observer
	ASSERT ( m_pMapObserver == NULL );
    m_pMapObserver = new CZCivilizationMapObserver ();
	m_pMapObserver->PersistentLoad ( storage );

    // Load the default unit
    CCodeManager::LoadDefaultUnit();
}

void CZCivilization::PersistentTranslatePointersStatic ( CPersistentStorage & storage )
{
    // Translate the contant strings
    m_stiGAME_INITIALIZED    = (CStringTableItem*)storage.TranslatePointer( m_stiGAME_INITIALIZED);
	m_stiUNIT_DIED           = (CStringTableItem*)storage.TranslatePointer( m_stiUNIT_DIED);
	m_stiUNIT_UPDATE         = (CStringTableItem*)storage.TranslatePointer( m_stiUNIT_UPDATE);
	m_stiINFERIOR_CREATED    = (CStringTableItem*)storage.TranslatePointer( m_stiINFERIOR_CREATED);
	m_stiINFERIOR_ADDED      = (CStringTableItem*)storage.TranslatePointer( m_stiINFERIOR_ADDED);
	m_stiINFERIOR_REMOVED    = (CStringTableItem*)storage.TranslatePointer( m_stiINFERIOR_REMOVED);
	m_stiINFERIOR_DIED       = (CStringTableItem*)storage.TranslatePointer( m_stiINFERIOR_DIED);
	m_stiCOMMANDER_CHANGED   = (CStringTableItem*)storage.TranslatePointer( m_stiCOMMANDER_CHANGED);
    m_stiIDLE                = (CStringTableItem*)storage.TranslatePointer( m_stiIDLE);

    // Translate pointers in the map observer
	ASSERT ( m_pMapObserver != NULL );
	m_pMapObserver->PersistentTranslatePointers ( storage );
}

void CZCivilization::PersistentInitStatic ()
{
	// Init the map observer
	ASSERT ( m_pMapObserver != NULL );
	m_pMapObserver->PersistentInit ();

    g_bGameLoading = false;
}

void CZCivilization::PersistentSave( CPersistentStorage &storage)
{
    BRACE_BLOCK(storage);

    // save civilizace
	// pointer na sebe
	storage << (DWORD)this;

	// notifier
	CNotifier::PersistentSave( storage);

    // pointer na SCivilization
    storage << (DWORD)m_pSCivilization;

    // script set name
    ASSERT( m_strScriptSetName.GetLength() <256);
    storage << m_strScriptSetName.GetLength();
    storage.Write( (LPCTSTR)m_strScriptSetName, m_strScriptSetName.GetLength());

    // Event Manager 
    storage << (DWORD)&m_EventManager;
    m_EventManager.PersistentSave( storage);

    // Code Manager
    m_pCodeManager->PersistentSave( storage);

    // Compiler - neni treba ukladat

    // Interpret
    m_pInterpret->PersistentSave( storage);

    // GeneralCommander, pocet jednotek
    storage << (DWORD)m_pGeneralCommander << m_nUnitCount;

	// vsechny jednotky
	if (m_pGeneralCommander != NULL) PersistentSaveUnit( storage, m_pGeneralCommander);

    // ID of the new unit
    storage << m_dwNewZUnitID;

    // zkopirovat log file
    CDataArchive *pDataArchive = storage.GetArchive();
    pDataArchive->AppendFile( m_LogArchiveFile, GetCivilizationName() + "\\" + CIVILIZATION_LOG_FILENAME);
}

void CZCivilization::PersistentSaveUnit( CPersistentStorage &storage, CZUnit *pUnit)
{
    ASSERT( pUnit != NULL);

	pUnit->PersistentSave( storage);

	CZUnit *pInferiorUnit = pUnit->m_pFirstInferior;
	while (pInferiorUnit != NULL)
	{
		PersistentSaveUnit( storage, pInferiorUnit);
		pInferiorUnit = pInferiorUnit->m_pNextSibbling;
	}
}

void CZCivilization::PersistentLoad( CPersistentStorage &storage, CString &strCivilizationName)
{
    g_bGameLoading = true;

    BRACE_BLOCK(storage);
    
    char str[256];
    int len;

	// pointer na sebe
	CZCivilization *pThis;
	storage >> (DWORD&)pThis;
	storage.RegisterPointer( pThis, this);

	// notifier
	CNotifier::PersistentLoad( storage);

    // pointer na SCivilization
    storage >> (DWORD&)m_pSCivilization;

    // Root adresar civilizace
    m_strRootPath = g_cSFileManager.GetCivilizationDirectory( strCivilizationName);

    // Script Set Name and Path
    storage >> len;
    storage.Read( str, len);
    str[len] = 0;
    m_strScriptSetName = str;
    
    m_strScriptSetPath = g_cSFileManager.GetScriptSetDirectory( m_strScriptSetName);

    // Event Manager 
    CEventManager *pEventManager;
    storage >> (DWORD&)pEventManager;
    storage.RegisterPointer( pEventManager, &m_EventManager);
    
    m_EventManager.PersistentLoad( storage);

    // Code Manager
	m_pCodeManager = new CCodeManager;
    m_pCodeManager->PersistentLoad( storage, m_strRootPath, m_strScriptSetPath, strCivilizationName);

    // Interpret
	m_pInterpret = new CInterpret;
    m_pInterpret->PersistentLoad( storage);

    // GeneralCommander, pocet jednotek
    storage >> (DWORD&)m_pGeneralCommander >> m_nUnitCount;

	// naloadovat vsechny jednotky do pole
	if (m_nUnitCount != 0)
	{
		m_pLoadedUnits = (CZUnit**) new char [sizeof(CZUnit*) * m_nUnitCount];
		for ( ULONG i=0; i<m_nUnitCount; i++)
		{
			m_pLoadedUnits[i] = new CZUnit();
			m_pLoadedUnits[i]->PersistentLoad( storage);
		}
	}
	else m_pLoadedUnits = NULL;

    // load the new unit ID
    // All civilization will do it - but all of them with same number
    // it doesn't metter
    storage >> m_dwNewZUnitID;

    // zkopirovat log file
    CDataArchive *pDataArchive = storage.GetArchive();

    pDataArchive->ExtractFile( m_strRootPath + "\\" + CIVILIZATION_LOG_FILENAME,
                               strCivilizationName + "\\" + CIVILIZATION_LOG_FILENAME);
}

void CZCivilization::PersistentTranslatePointers( CPersistentStorage &storage)
{
	// notifier
	CNotifier::PersistentTranslatePointers( storage);

    // pointer na SCivilization
    m_pSCivilization = (CSCivilization *)storage.TranslatePointer(m_pSCivilization);

    // Event Manager 
    m_EventManager.PersistentTranslatePointers( storage);

    // Code Manager
    m_pCodeManager->PersistentTranslatePointers( storage);

    // Interpret
    m_pInterpret->PersistentTranslatePointers( storage);

    // super Commander
    m_pGeneralCommander = (CZUnit*) storage.TranslatePointer( m_pGeneralCommander);

    // jednotky
	if (m_pLoadedUnits != NULL)
	{
		ULONG i;
		for (i=0; i<m_nUnitCount; i++)
		{
			m_pLoadedUnits[i]->PersistentTranslatePointers( storage);
		}
	}
}

void CZCivilization::PersistentInit()
{
    g_bGameLoading = false;

	// notifier
	CNotifier::PersistentInit();
  
    // EventManager
    m_EventManager.PersistentInit();
    CEventManager *pOldMan = m_EventManager.AttachToThread();

    // Code Manager
	m_pCodeManager->PersistentInit( m_pInterpret);

    // Interpret
	m_pInterpret->PersistentInit();

    // Compiler
	m_pCompiler = new CCompiler;
    m_pCompiler->Create( m_pCodeManager->GetCodeManagerForCompiler());

    // jednotky
	if (m_pLoadedUnits != NULL)
	{
		ULONG i;
		for (i=0; i<m_nUnitCount; i++)
		{
			m_pLoadedUnits[i]->PersistentInit();

            // now register the unit in our ID map
            m_mapUnitsByIDs.SetAt(m_pLoadedUnits[i]->GetID(), m_pLoadedUnits[i]);
		}
	}
	delete [] m_pLoadedUnits;
	m_pLoadedUnits = NULL;

    // Root civilizace
    m_pRootDataArchive = new CDataArchive;
    if (!m_pRootDataArchive->Create( m_strRootPath, CArchiveFile::modeReadWrite, 
                                   CDataArchiveInfo::archiveDirectory)) 
    {
        delete m_pRootDataArchive;
        throw new CStringException("Can't open archive %s.\n", m_strRootPath);
    }

    // LogFile - otevrit log
    m_LogArchiveFile = m_pRootDataArchive->CreateFile(CIVILIZATION_LOG_FILENAME, 
        CArchiveFile::modeReadWrite | CArchiveFile::modeUncompressed);

    m_pLogFileObject = new CLogFile;
    m_pLogFileObject->Create(m_LogArchiveFile);

    // spusteni threadu intepreta
    CWinThread *pInterpretThread = ::AfxBeginThread( RunInterpret, this);
	m_hInterpretThreadHandle = pInterpretThread->m_hThread;
	m_nInterpretThreadID = pInterpretThread->m_nThreadID;

    // obnoveni puvodniho event managera
    if (pOldMan != NULL) pOldMan->AttachToThread();
    else m_EventManager.DetachFromThread();
	
	// increment the count of civilization instances
	::InterlockedIncrement( &m_nCivilizationInstancesCount);
}

//
// METODY PRO SERVER
//

// Vytvoreni jednotky
CZUnit* CZCivilization::CreateUnit( CSUnit *pSUnit,  CZUnit *pCommander, LPCTSTR lpszScriptName, bool bPhysicalUnit)
{
    CIUnitType *pIUnitType = NULL;
    CStringTableItem *stiUnitTypeName = NULL;

    if (lpszScriptName == NULL) stiUnitTypeName = g_StringTable.AddItem(DEFAULT_UNIT_PARENT);
    else stiUnitTypeName = g_StringTable.AddItem(lpszScriptName);

    if( (pIUnitType = m_pCodeManager->GetUnitType(stiUnitTypeName)) == NULL) 
    {
        ReportError("Can't find unit script '%s'. Creating unit with no script.", lpszScriptName);

		pIUnitType = CCodeManager::m_pDefaultUnitType;
    }

    // kontrola velitele

    if (pCommander == NULL || pCommander->m_bPhysicalUnit) pCommander = m_pGeneralCommander;

    CZUnit *pZUnit = NULL;
    DWORD dwNewUnitID = (DWORD)InterlockedIncrement((LPLONG)&m_dwNewZUnitID);

    pZUnit = new CZUnit();
    pZUnit->Create( this, pSUnit, pCommander, pIUnitType, bPhysicalUnit, dwNewUnitID );
    LockHierarchyAndEvents();
    m_mapUnitsByIDs.SetAt(dwNewUnitID, pZUnit);
    UnlockHierarchyAndEvents();

    ::InterlockedIncrement( (LONG*)&m_nUnitCount);

    // je to nejvyssi velitel
    if (pCommander == NULL) 
    {
        ASSERT( !bPhysicalUnit );
        m_pGeneralCommander = pZUnit;
    }

    stiUnitTypeName->Release();

	// zaslat notifikaci
	if ( pZUnit->m_pCommander != NULL)
	{
		CIBag Bag;
		Bag.Create();
		Bag.AddUnit( pZUnit, CCodeManager::m_pDefaultUnitType);
		pZUnit->m_pCommander->SendNotification( m_stiINFERIOR_CREATED, &Bag);
		Bag.Delete();
	}

    TRACE_INTERPRET3("- Civilization \"%s\": created unit '%s' (0x%08x).\n", GetCivilizationName(),
        (lpszScriptName != NULL) ? lpszScriptName : DEFAULT_UNIT_PARENT, pZUnit);

    return pZUnit;
}

// Umreni jednotky
void CZCivilization::UnitDied( CZUnit *pZUnit, CPointDW& Position)
{
    pZUnit->m_bIsDead = true;
    
    LockHierarchyAndEvents();
    m_mapUnitsByIDs.RemoveKey(pZUnit->GetID());
    UnlockHierarchyAndEvents();

// vycistit fronty zprav
    // prikazy
    pZUnit->EmptyMandatoryCommandQueue( false );
    pZUnit->EmptyCurrentCommandQueue( false );
    pZUnit->EmptyNotificationQueue();

// poslat jednotce notifikaci o tom, ze umrela
    CIBag Bag;
    CISOPosition *pPosition;

    pPosition = (CISOPosition*)m_pInterpret->CreateSystemObject(SO_POSITION);
    ASSERT(pPosition != NULL);

	 pPosition->Set( Position.x, Position.y, 0);

    Bag.Create();
    Bag.AddSystemObject( pPosition);

    pZUnit->SendNotification( m_stiUNIT_DIED, &Bag);

    Bag.Delete();
    pPosition->Release();

// poslat notifikaci jejimu veliteli
	if (pZUnit->m_pCommander != NULL)
	{
		Bag.Create();
		Bag.AddUnit( pZUnit, CCodeManager::m_pDefaultUnitType);

		pPosition = (CISOPosition*)m_pInterpret->CreateSystemObject(SO_POSITION);
		ASSERT(pPosition != NULL);

		pPosition->Set( Position.x, Position.y, 0);

		Bag.AddSystemObject( pPosition);

		pZUnit->m_pCommander->SendNotification( m_stiINFERIOR_DIED, &Bag);

		Bag.Delete();
        pPosition->Release();
	}
}

CString CZCivilization::GetCivilizationName() 
{
#ifdef PROJEKTAPP
    return CString("[unknown]");
#else
    ASSERT( m_pSCivilization != NULL);
    return m_pSCivilization->GetCivilizationName();
#endif
}

DWORD CZCivilization::GetCivilizationIndex()
{
#ifdef PROJEKTAPP
    return 0;
#else
    ASSERT( m_pSCivilization != NULL);
    return m_pSCivilization->GetCivilizationIndex();
#endif
}

// hlaseni chyb
void CZCivilization::ReportError(LPCTSTR strErrorMsgFormat, ...)
{
    va_list args;
    va_start(args, strErrorMsgFormat);

    ReportError(strErrorMsgFormat, args);
}

// hlaseni chyb
void CZCivilization::ReportError(LPCTSTR strErrorMsgFormat, va_list args)
{
    // zamknuti logfile
    LockLogFile();

    m_pLogFileObject->AddRecordFormat(strErrorMsgFormat, args);

    // odemknuti logfile
    UnlockLogFile();


#ifdef _DEBUG
    CString str;
    str.FormatV( strErrorMsgFormat, args);
    TRACE_LOGGEDERROR1("LOGGED ERROR: %s\n", str);
#endif
}

CEvent* CZCivilization::Pause()
{
  // Disconnect all clients
    {
      VERIFY(m_lockConnectedClients.Lock());

      CZConnectedClient *pClient = m_pConnectedClients, *pDel;
      CEvent cEvent;

      while(pClient != NULL)
      {
            pDel = pClient;
            pClient = pClient->m_pNext;

            CString strUserName = pDel->GetUserName();
            // Just send an event to that one
            cEvent.ResetEvent ();
            InlayEvent ( CZConnectedClient::E_DisconnectClient, (DWORD)&cEvent, pClient );
            VERIFY ( cEvent.Lock () );

#ifndef PROJEKTAPP
            g_cMap.UserLoggedOff(strUserName);
#endif
      }

      m_pConnectedClients = NULL;
 
      VERIFY(m_lockConnectedClients.Unlock());
    }

    SIPauseInterpret *pPauseInterpret = new SIPauseInterpret;
    pPauseInterpret->m_pIntepretPaused = &m_GamePausedEvent;
    pPauseInterpret->m_pFinishPause = &m_GameUnpauseEvent;

    m_GamePausedEvent.ResetEvent();
    m_GameUnpauseEvent.ResetEvent();

    InlayEvent( PAUSE_INTERPRET, (DWORD) pPauseInterpret);

    return &m_GamePausedEvent;
}

void CZCivilization::Unpause( bool bContinue)
{
    // je konec?
    if (!bContinue)
    {
		g_bGameClosing = true;

        // rict interpretovi, ze se konci
        m_EventManager.InlayQuitEvent(0);
    }

    // vzbudit interpret z pauzy
    m_GameUnpauseEvent.SetEvent();

    // je konec?
    if (!bContinue)
    {
        // pockat na nej
        if (m_hInterpretThreadHandle != NULL)
        {
            ::WaitForSingleObject(m_hInterpretThreadHandle, INFINITE);
    
            // aby se necekalo v Delete, ktere bude vzapeti zavolano
            m_hInterpretThreadHandle = NULL;
        }
    }
}

void CZCivilization::OnGameInitialized()
{
    ASSERT(m_pGeneralCommander != NULL);
    
    CIBag Bag;  // prazdny bag, muze byt lokalni promenna, protoze si ho jednotka zkopiruje
    Bag.Create();
    m_pGeneralCommander->SendNotification( m_stiGAME_INITIALIZED, &Bag);
    Bag.Delete();
}

//
// PRO INTERPRET
//

CZUnitListItem* CZCivilization::GetAllUnitsOfTypeAndChildren( CIUnitType *pIUnitType)
{
    if (m_pGeneralCommander == NULL) return NULL;
    else 
    {
        // zamceni vojenske hierarchie
        LockHierarchyAndEvents();

        CZUnitListItem *pRet = GetAllUnitsOfTypeAndChildren_HelperResursiveMethod( m_pGeneralCommander,
                                                                                   pIUnitType, NULL);
        // odemceni vojenske hierarchie
        UnlockHierarchyAndEvents();

        return pRet;
    }
}

CZUnitListItem* CZCivilization::GetAllUnitsOfTypeAndChildren_HelperResursiveMethod( CZUnit *pUnit,
                                    CIUnitType *pISearchedUnitType, CZUnitListItem *pUnitListItem)
{
    ASSERT( pUnit != NULL);
    ASSERT( pISearchedUnitType != NULL);

    CIUnitType *pIUnitType = pUnit->m_pIUnitType;
    
    // zkontrolovat sebe
    while ( pIUnitType != NULL)
    {
        if (pIUnitType == pISearchedUnitType)
        {
            pUnitListItem = new CZUnitListItem( pUnit, pUnitListItem);
            break;
        }

        pIUnitType = pIUnitType->m_pParent;
    }

    // zkontrolovat potomky
    CZUnit *pPomUnit = pUnit->m_pFirstInferior;
    while (pPomUnit != NULL)
    {
        pUnitListItem = GetAllUnitsOfTypeAndChildren_HelperResursiveMethod( pPomUnit, 
                                                            pISearchedUnitType, pUnitListItem);
        pPomUnit = pPomUnit->m_pNextSibbling;
    }

    return pUnitListItem;
}

void CZCivilization::Compile(CCompilerInput *pStdIn, CCompilerErrorOutput *pStdErr, 
    bool bUpdateCodeOnSuccess,  bool bDebugOutput, CCompilerErrorOutput *pDbgOut, 
	 bool bDebugParser)
{
    m_pCompiler->Compile( pStdIn, pStdErr, bUpdateCodeOnSuccess, bDebugOutput, pDbgOut, bDebugParser);
}

CTypedMemoryPool<CZCivilizationMapObserver::tagSTimerNode> CZCivilizationMapObserver::m_cTimerNodesPool ( 50 );

void CZCivilization::SetTimer ( CZUnit *pUnit, int nUserID, DWORD dwTimeslice )
{
  ASSERT ( m_pMapObserver != NULL );
  m_pMapObserver->SetTimer ( pUnit, nUserID, dwTimeslice ); 
}

// Sets the timer to wakeup given syscall when it elapses
void CZCivilization::SetTimer ( CISyscall * pSyscall, DWORD dwTimeslice )
{
	ASSERT ( m_pMapObserver != NULL );
	m_pMapObserver->SetTimer ( pSyscall, dwTimeslice );
}

CZCivilizationMapObserver::CZCivilizationMapObserver ()
{
  m_pTimerNodesList = NULL;
  m_stiON_TIMER = NULL;
}

CZCivilizationMapObserver::~CZCivilizationMapObserver ()
{
  ASSERT ( m_pTimerNodesList == NULL );
}

void CZCivilizationMapObserver::Create ()
{
  m_pTimerNodesList = NULL;

  // Get the ON_TIMER sti
  m_stiON_TIMER = g_StringTable.AddItem ( TIMER_ELAPSED_NOTIFICATION );

  // Register us on the map
  g_cMap.RegisterMapObserver ( this );
}

void CZCivilizationMapObserver::Delete ()
{
  STimerNode *pNode, *pDel;

  // Unregister us from the map
  g_cMap.UnregisterMapObserver ( this );

  // Lock the timer
  VERIFY ( m_lockTimer.Lock () );

  // Release the ON_TIMER sti
  if ( m_stiON_TIMER != NULL )
  {
    m_stiON_TIMER->Release ();
    m_stiON_TIMER = NULL;
  }

  // Go through all nodes and delete them
  pNode = m_pTimerNodesList;
  while ( pNode != NULL )
  {
    pDel = pNode;
    pNode = pNode->m_pNext;

    if ( pDel->m_pUnit != NULL )
    {
      pDel->m_pUnit->Release ();
    }
    else
    {
      ASSERT ( pDel->m_pSyscall );
      pDel->m_pSyscall->Release ();
    }
    m_cTimerNodesPool.Free ( pDel );
  }
  m_pTimerNodesList = NULL;

  // Unlock the timer
  VERIFY ( m_lockTimer.Unlock () );
}

void CZCivilizationMapObserver::OnTimeslice ( DWORD dwTimeslice )
{
  // Go through all timer nodes and test them

  STimerNode *pNode, *pDel;

  // First lock the timer nodes
  VERIFY ( m_lockTimer.Lock () );

  pNode = m_pTimerNodesList;
  while ( pNode != NULL )
  {
	// If the first node has to wait for now
	// it means that all nodes have to wait -> do nothing
    if ( pNode->m_dwTimeslice > dwTimeslice )
		break;

	// Now we can remove the first node and move to the next one
	pDel = pNode;
	pNode = pNode->m_pNext;

	if ( pDel->m_pUnit != NULL )
	{
		// Send the notification here
		// So first create the bag
		CIBag cBag;
		cBag.Create ();
		
		// Add the user ID to the bag
		cBag.AddConstantInt ( pDel->m_nUserID );
		// Add the current timeslice to the bag
		cBag.AddConstantInt ( dwTimeslice );
		
		// Send the notification
		pDel->m_pUnit->SendNotification ( m_stiON_TIMER, &cBag );
		
		// Delete the bag
		cBag.Delete ();

		// Release the unit
		pDel->m_pUnit->Release ();
	}
	else
	{
		// Wakeup the syscall here

		ASSERT ( pDel->m_pSyscall != NULL );
		pDel->m_pSyscall->FinishWaiting ( STATUS_SUCCESS );
    pDel->m_pSyscall->Release ();
	}
	
	// Now remove the node from the list
	m_pTimerNodesList = pDel->m_pNext;
	
	// Delete the node itself
	m_cTimerNodesPool.Free ( pDel );
  }

  VERIFY ( m_lockTimer.Unlock () );
}

// Helper function to insert new timer nodes to the sorted list
// The timer lock must be locked
void CZCivilizationMapObserver::InsertNewNode ( STimerNode * pNewNode )
{
  // Add it to the list
  STimerNode **pNode = &m_pTimerNodesList;
  while ( (*pNode) != NULL )
  {
	  // If we've found node that will wait longer than us ->
	  // insert us before it
	  if ( (*pNode)->m_dwTimeslice >= pNewNode->m_dwTimeslice )
		  break;

	  // Go to the next node
	  pNode = &((*pNode)->m_pNext);
  }
  // Now insert the new node before the *pNode
  pNewNode->m_pNext = *pNode;
  *pNode = pNewNode;
}

void CZCivilizationMapObserver::SetTimer ( CZUnit *pUnit, int nUserID, DWORD dwTimeslice )
{
  ASSERT ( pUnit != NULL );

  // Lock the timer
  VERIFY ( m_lockTimer.Lock () );

  // Create new timer node
  STimerNode *pNewNode;

  pNewNode = m_cTimerNodesPool.Allocate ();

  // We must addref the unit, cause it could died
  pUnit->AddRef ();
  // Fill the structure
  pNewNode->m_pSyscall = NULL;
  pNewNode->m_pUnit = pUnit;
  pNewNode->m_nUserID = nUserID;
  pNewNode->m_dwTimeslice = dwTimeslice;

  // Add it
  InsertNewNode ( pNewNode );

  VERIFY ( m_lockTimer.Unlock () );
}

void CZCivilizationMapObserver::SetTimer ( CISyscall * pSyscall, DWORD dwTimeslice )
{
	ASSERT ( pSyscall );

	// Lock the timer
	VERIFY ( m_lockTimer.Lock () );

	STimerNode * pNewNode = m_cTimerNodesPool.Allocate ();

	// Fill the structure
	pNewNode->m_pSyscall = pSyscall;
	pNewNode->m_pUnit = NULL;
	pNewNode->m_nUserID = 0;
	pNewNode->m_dwTimeslice = dwTimeslice;

	// Add it
	InsertNewNode ( pNewNode );

	// Unlock the timer
	VERIFY ( m_lockTimer.Unlock () );
}

// Persistent load/save
void CZCivilizationMapObserver::PersistentSave( CPersistentStorage &storage)
{
  BRACE_BLOCK ( storage );

  // Count the number of nodes
  DWORD dwCount = 0;
  STimerNode *pNode = m_pTimerNodesList;
  while ( pNode != NULL )
  {
    dwCount++;
    pNode = pNode->m_pNext;
  }

  // Write number of nodes
  storage << dwCount;

  // Go through all nodes and write them
  pNode = m_pTimerNodesList;
  while ( pNode != NULL )
  {
    storage << (void *)(pNode->m_pUnit);
    storage << pNode->m_dwTimeslice;
    storage << pNode->m_nUserID;
	// Save the syscall
	storage << (void *)(pNode->m_pSyscall);
	if ( pNode->m_pSyscall != NULL )
	{
		pNode->m_pSyscall->PersistentSave ( storage );
	}

    pNode = pNode->m_pNext;
  }
}

void CZCivilizationMapObserver::PersistentLoad( CPersistentStorage &storage )
{
  BRACE_BLOCK ( storage );

  m_pTimerNodesList = NULL;

  // Load number of nodes
  DWORD dwCount, dwI;

  storage >> dwCount;

  // Create all nodes
  STimerNode *pNewNode;
  for ( dwI = 0; dwI < dwCount; dwI ++ )
  {
    pNewNode = m_cTimerNodesPool.Allocate ();
    storage >> (void *&)(pNewNode->m_pUnit);
    storage >> pNewNode->m_dwTimeslice;
    storage >> pNewNode->m_nUserID;

	// Load the syscall
	{
		void * pSyscallSaved;
		storage >> (void *&)pSyscallSaved;
		if ( pSyscallSaved != NULL )
		{
			// There was some syscall
			// -> create it
			CISyscall * pSyscall = new CISyscall ();
			pSyscall->PersistentLoad ( storage );
			storage.RegisterPointer ( pSyscallSaved, (void *)pSyscall );

			pNewNode->m_pSyscall = pSyscall;
		}
		else 
		{
			pNewNode->m_pSyscall = NULL;
		}
	}

	// Add it to the list
	InsertNewNode ( pNewNode );
  }
}

void CZCivilizationMapObserver::PersistentTranslatePointers( CPersistentStorage &storage)
{
  // Go through all nodes and translate pointers for them
  STimerNode *pNode = m_pTimerNodesList;
  while ( pNode != NULL )
  {
    pNode->m_pUnit = (CZUnit *)storage.TranslatePointer ( (void *)(pNode->m_pUnit) );
	// Translate poitners in the syscall (if some)
	if ( pNode->m_pSyscall != NULL )
	{
		pNode->m_pSyscall->PersistentTranslatePointers ( storage );
	}

    pNode = pNode->m_pNext;
  }
}

void CZCivilizationMapObserver::PersistentInit()
{
  // Get the ON_TIMER sti
  m_stiON_TIMER = g_StringTable.AddItem ( TIMER_ELAPSED_NOTIFICATION );

  STimerNode *pNode = m_pTimerNodesList;
  while ( pNode != NULL )
  {
	  if ( pNode->m_pSyscall != NULL )
	  {
		  pNode->m_pSyscall->PersistentInit ();
	  }
	  pNode = pNode->m_pNext;
  }

  // Register us on the map
  g_cMap.RegisterMapObserver ( this );
}
