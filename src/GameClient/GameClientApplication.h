// GameClientApplication.h: interface for the CGameClientApplication class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_GAMECLIENTAPPLICATION_H__0D19A3A3_BAEF_11D3_99EC_AB4BA3CB8B71__INCLUDED_)
#define AFX_GAMECLIENTAPPLICATION_H__0D19A3A3_BAEF_11D3_99EC_AB4BA3CB8B71__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "DesktopWindow.h"
#include "Controls\ProgressWindow.h"
#include "Controls\ScreenWindow.h"
#include "ScriptEditor\ScriptEditor.h"
#include "MapView\MapView.h"
#include "UnitHierarchy\UnitHierarchy.h"

class CGameClientApplication : public CNotifier
{
  DECLARE_DYNAMIC(CGameClientApplication);
  DECLARE_OBSERVER_MAP(CGameClientApplication);

public:
  // stops the application main event thread
	void Stop();
  // starts the application main event thread - it finishes the initialization
	void Start();

// These actions are done in new thread - in the furutre it will be the main event thread
// Do not call these from foreign objects
  // Deletes the application
	void Delete();
  // creates the application
	BOOL Create();
  // starts (enables) all hardwork
  void StartHardwork();
  // stops all hardwork (like frameloop)
  void StopHardwork();


  // closes the application - reverse to Init
	void Close();
  // initialize the application - here we do all init needed in the mainthread
	BOOL Init(BOOL bExclusive, CWnd *pMainWnd);


  // constructor & destructor
	CGameClientApplication();
	virtual ~CGameClientApplication();

#ifdef _DEBUG
  virtual void AssertValid() const;
  virtual void Dump(CDumpContext &dc) const;
#endif

//-------------------------------------------------------------------------------
//-------------------------------------------------------------------------------
private:


//-------------------------------------------------------------------------------
// events to ourself

  enum{ _Self_NotID_ = 0x0EFFFFFFF }; // notifier ID of the myself object

  enum{
    E_Connect = 1,  // event to show the connection dialog
    E_Connected = 2, // event to connect to network
    E_ScriptEditor = 5,  // event to start the script editor
    E_MapView = 6,       // event to start the map view
    E_CreateClient = 10,
    E_ClientCreated = 11,
    E_DownloadMap = 20,  // event to download the current map from the server
    E_MapDownloaded = 21, // event when the map is downloaded (param is pointer to CException) if NULL-> success
    E_Init = 30,        // event to request init info from server
    E_InitArrived = 31, // event when the init info arrived from server
    E_NetworkError = 100, // Network error -> show dialog
  };

//-------------------------------------------------------------------------------
// Main Event thread
  // system thread proc - the main event thread
	static UINT _MainEventThreadProc(LPVOID pParam);
  // actual thread proc - the main event thread
  UINT MainEventThreadProc();

  // closing the app
  BOOL CanClose();

// Main event manager
  // event manager object for the second thread (and also for the first one) - the main event thread
  CEventManager m_MainEventManager;
  // the win thread object of the second thread - main event thread
  CWinThread *m_pMainEventThread;


//-------------------------------------------------------------------------------
// DirectX
  // initialize the direct draw - sets the new resolution and so
	BOOL InitDirectDraw(DWORD dwWidth, DWORD dwHeight, BOOL bExclusive, CWnd *pMainWnd);

  // directX objects
  CDirectInput m_DirectInput;
  CDirectSound m_DirectSound;

  // the starting directory - all files from there
  CString m_strStartDirectory;


//-------------------------------------------------------------------------------
// Windows subsystem
  // the default cursor
  CCursor m_Cursor;
  // The waiting cursor
  CCursor m_WaitingCursor;

  // the desktop window
  CDesktopWindow m_Desktop;

	CKeyAccelerators m_KeyAcc;


//-------------------------------------------------------------------------------
// Archives
  // some archives
  // root - the root directory for our tree
  // graphics - all graphics from there
  CDataArchive *m_pRootArchive, *m_pGraphicsArchive;


//-------------------------------------------------------------------------------
// Script editor
  // the script editor screen window
  CScreenWindow m_ScriptEditorScreenWindow;
  // the script editor
  CScriptEditor m_ScriptEditor;
  
  enum{ IDC_SCRIPTEDITOR = 0x08000, };

//-------------------------------------------------------------------------------
// Map view
  // the map view screen window
  CScreenWindow m_MapViewScreenWindow;
  // the map view
  CMapView m_MapView;

  enum{ IDC_MAPVIEW = 0x07000, };

//-------------------------------------------------------------------------------
// Progress window
  // some progress window - used everywhere it's needed
  CProgressWindow m_ProgressWindow;

//-------------------------------------------------------------------------------
// Map download
//-------------------------------------------------------------------------------
// Map download thread
  UINT DownloadMapThreadProc();
  static UINT _DownloadMapThreadProc(LPVOID pParam);

  // the thead object
  CWinThread *m_pDownloadMapThread;

//-------------------------------------------------------------------------------
// reactions on events
  void OnDownloadMap();
  void OnMapDownloaded(CException *pException);

  // path to the map
  CString m_strMapPath;



//-------------------------------------------------------------------------------
// Network
//-------------------------------------------------------------------------------
// Connect & registry
  // writes the connect information to the registry
	void WriteRegistryConnectInfo();
  // reads the connection information from the registry
	void ReadRegistryConnectInfo();

  enum{ ID_Network = 0x05000 }; // the network notifier ID

//-------------------------------------------------------------------------------
// functions for the connection thread
	UINT ConnectThreadProc();
	static UINT _ConnectThreadProc(LPVOID pParam);

//-------------------------------------------------------------------------------
// network event thread
  // the thread itself
  CWinThread *m_pNetworkEventThread;
  // event manager for it
  CEventManager m_NetworkEventManager;
  // event to wait for before starting the network event loop
  CEvent m_eventRunNetworkEventThread;
  // event which is set when the network event thread ends it work (before cleanup)
  CEvent m_eventNetworkEventThreadStopped;
  // event to wait for to perform the cleanup and exit
  CEvent m_eventNetworkEventThreadExit;

//-------------------------------------------------------------------------------
// Network connection
	// connects the network to the server
  // at the end sends an event E_Connected with parameter CException *e
  //  if e is NULL all was OK
  //  elseway it's the exception
  // also starts special thread for network events
  BOOL ConnectNetwork(CString strServerName, WORD wPort, CString strLogin, CString strPassword);
  // closes the network connection
  // finishes network event thread
	void DeleteNetwork();

//-------------------------------------------------------------------------------
// reactions on network events
  void OnNetworkError();

  // Network erro reaction (in the main event loop) -> show dialog
  void OnNetworkErrorSecondary();

//-------------------------------------------------------------------------------
// Control Connection (used after login to ask for some info)
//   like Civilization and TimeSlice length and so on...
  // The virtual connection
  CVirtualConnection m_ControlConnection;

  // Reaction on events on this virt. conection  
  void OnControlConnectionError();
  void OnControlConnectionPacketAvailable();

  // ID of the control connection notifier
  enum{
    ID_CONTROL_CONNECTION = 0x05001,    // ID of control connection
    ID_CONTROL_APPLICATION = 0x05002,   // ID of the contifier (application)
                                        // for sending special events (like pause network)
  };

  // some helper variables
  // Number of event to send to us after the request is done
  // (if 0 no request is in progress)
  // The event will be sent with param 0 if sucess elseway it will send 1
  DWORD m_dwControlConnectionDoneEvent;

  enum EControlConnectionRequest{
    ControlConnectionRequest_None = 0,
    ControlConnectionRequest_Init = 1,
  };

  // Number of current control connection request
  EControlConnectionRequest m_eControlConnectionRequest;

  // Some lock for control connection data
  CMutex m_lockControlConnectionRequest;

  class CControlConnectionObserver : public CObserver
  {
    DECLARE_OBSERVER_MAP(CControlConnectionObserver);

  public:
    CControlConnectionObserver(){};
    ~CControlConnectionObserver(){};

    CGameClientApplication *m_pGameClientApplication;

    void OnPacketAvailable(){ m_pGameClientApplication->OnControlConnectionPacketAvailable(); }
    void OnNetworkError(){ m_pGameClientApplication->OnControlConnectionError(); }
    void OnWaitForNetworkToRun(){ VERIFY(m_pGameClientApplication->m_eventRunNetworkEventThread.Lock()); }

    enum{
      E_PauseNetwork = 1,  // Send this event to this notifier to pause the network
                           // it will wait till m_eventRunNetworkEventThread is set
    };
  };
  friend CControlConnectionObserver;
  CControlConnectionObserver m_ControlConnectionObserver;
  
//-------------------------------------------------------------------------------
// Unit hierarchy
  // The unit hierarchy object
  CUnitHierarchy m_UnitHierarchy;

//-------------------------------------------------------------------------------
// init packet
  // Here we store init data before the create is called
  DWORD m_dwInitCivilizationID;
  DWORD m_dwInitCurrentTimeslice;
  DWORD m_dwInitTimesliceInterval;
  DWORD m_dwInitStartPositionX;
  DWORD m_dwInitStartPositionY;

//-------------------------------------------------------------------------------
// some variables
  // network things
  // the network client object
  CNetworkClient m_NetworkClient;
  // TRUE if the network was initialized (connected)
  BOOL m_bNetworkInitialized;

  // connect information
  CString m_strServerName;
  WORD m_wServerPort;
  CString m_strLogin;
  CString m_strPassword;

//-------------------------------------------------------------------------------
// Client creation
//-------------------------------------------------------------------------------
  // the Client creation thread
  CWinThread *m_pClientCreationThread;
  static UINT _ClientCreationThread(LPVOID pParam);
  UINT ClientCreationThread();


//-------------------------------------------------------------------------------
//-------------------------------------------------------------------------------
protected:
// reactions on events
	void OnConnected(CNetworkException *e);
	void OnScriptEditor();
  void OnMapView();
	void OnConnect(DWORD dwParam);

  void OnCreateClient();
  void OnClientCreated(CException *e);

  void OnInit();
  void OnInitArrived(BOOL bSuccess);
};

// the only connection to the server -> global variable -> easy to use
extern CNetworkConnection *g_pNetworkConnection;

#endif // !defined(AFX_GAMECLIENTAPPLICATION_H__0D19A3A3_BAEF_11D3_99EC_AB4BA3CB8B71__INCLUDED_)
