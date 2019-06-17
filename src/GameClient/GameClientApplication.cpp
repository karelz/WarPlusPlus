// GameClientApplication.cpp: implementation of the CGameClientApplication class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "gameclient.h"
#include "GameClientApplication.h"

#include "GameClient\Common\CommonExceptions.h"
#include "Controls\ConnectDlg.h"

#include "DownloadMap.h"

#include "Common\ServerClient\ControlConnection.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

#define REGISTRY_CONNECT_SECTION "Connect"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

IMPLEMENT_DYNAMIC(CGameClientApplication, CObserver)

// observer map
BEGIN_OBSERVER_MAP(CGameClientApplication, CObserver)
  BEGIN_NOTIFIER(_Self_NotID_)
    EVENT(E_Connect)
      OnConnect(dwParam); return FALSE;
    EVENT(E_Connected)
      OnConnected((CNetworkException *)dwParam); return FALSE;
    EVENT(E_ScriptEditor)
      OnScriptEditor(); return FALSE;
    EVENT(E_MapView)
      OnMapView(); return FALSE;
    EVENT(E_CreateClient)
      OnCreateClient(); return FALSE;
    EVENT(E_ClientCreated)
      OnClientCreated((CException *)dwParam); return FALSE;
    EVENT(E_DownloadMap)
      OnDownloadMap(); return FALSE;
    EVENT(E_MapDownloaded)
      OnMapDownloaded((CException *)dwParam); return FALSE;
    EVENT(E_Init)
      OnInit(); return FALSE;
    EVENT(E_InitArrived)
      OnInitArrived(dwParam == 0); return FALSE;
    EVENT(E_NetworkError)
      OnNetworkErrorSecondary(); return FALSE;
  END_NOTIFIER()

  BEGIN_NOTIFIER(ID_Network)
    EVENT(E_NETWORKERROR)
    EVENT(E_ABORTIVECLOSE)
    EVENT(E_NORMALCLOSE)
    EVENT(E_WAITERROR)
    EVENT(E_MEMORYERROR)
    EVENT(E_NETWORKEVENTSELECTERROR)
      OnNetworkError(); return FALSE;
  END_NOTIFIER()

  BEGIN_NOTIFIER(IDC_MAPVIEW)
    EVENT(CMapView::E_SCRIPTEDITOR)
      InlayEvent(E_ScriptEditor, 0, this); return FALSE;
  END_NOTIFIER()

  BEGIN_NOTIFIER(IDC_SCRIPTEDITOR)
    EVENT(CScriptEditor::E_MAPVIEW)
      InlayEvent(E_MapView, 0, this); return FALSE;
  END_NOTIFIER()

END_OBSERVER_MAP(CGameClientApplication, CObserver)


//------------------------------------------------------------------------
// Global variables

// the only one network connection
CNetworkConnection * g_pNetworkConnection = NULL;

// the only network event thread ID
DWORD g_dwNetworkEventThreadID;


//------------------------------------------------------------------------
// Constructor and destructor

// Constructor
CGameClientApplication::CGameClientApplication()
{
  m_pRootArchive = NULL;
  m_pGraphicsArchive = NULL;

  m_bNetworkInitialized = FALSE;
  m_pMainEventThread = NULL;
  m_pDownloadMapThread = NULL;
  m_pNetworkEventThread = NULL;

  m_ControlConnectionObserver.m_pGameClientApplication = this;
}

// Destructor
CGameClientApplication::~CGameClientApplication()
{
  ASSERT(m_pMainEventThread == NULL);
  ASSERT(m_pGraphicsArchive == NULL);
  ASSERT(m_pRootArchive == NULL);
  ASSERT(m_pDownloadMapThread == NULL);
  ASSERT(m_pNetworkEventThread == NULL);
}


//------------------------------------------------------------------------
// Debug functions
#ifdef _DEBUG
  
void CGameClientApplication::AssertValid() const
{
  CNotifier::AssertValid();
}

void CGameClientApplication::Dump(CDumpContext &dc) const
{
  CNotifier::Dump(dc);
}

#endif


#include "ScriptEditor\HelpBrowser\HelpBrowserWindow.h"
extern CHelpBrowserWindow *g_pHelpBrowserWindow;

//------------------------------------------------------------------------
// The main thread init and close

// Initializes the application - must be run in the main thread of the application
BOOL CGameClientApplication::Init(BOOL bExclusive, CWnd *pMainWnd)
{
  try{
	CNotifier::Create();

    TRACE("Initializing application...\n");
    // create the event manager for this thread
    // in future it will be attached to the thread
    // in which we will process events
    if(!m_MainEventManager.Create()){
      TRACE("Failed to create EventManager.\n");
      return FALSE;
    }

    // get current directory, we will take all files starting here
    char pcurdir[1025];
    GetCurrentDirectory(1024, pcurdir);
    m_strStartDirectory = pcurdir;
    g_strStartDirectory = m_strStartDirectory;

    // initialize the archive library
    CDataArchive::Initialize(m_strStartDirectory + "\\Temp");

    // create the root archive (start directory)
    m_pRootArchive = new CDataArchive();
    m_pRootArchive->Create(m_strStartDirectory);

    // create the direct input
    if(!m_DirectInput.Create(AfxGetInstanceHandle(), pMainWnd)){
      TRACE("Can't create DirectInput.\n");
      return FALSE;
    }

    // start the timer system
    if(!CTimer::Init()){
      TRACE("Failed to initilize Timer.\n");
      return FALSE;
    }

    // init the keyboard system
    if(!CKeyboard::Init((CKeyboardWindow *)pMainWnd)){
      TRACE("Error occured while initializing keyboard.\n");
      return FALSE;
    }
    m_KeyAcc.Create();

    // init the mouse - now only starting
    if(!CMouse::Init((CMouseWindow *)pMainWnd, bExclusive)){
      TRACE("Error occured while initializing mouse.\n");
      return FALSE;
    }

    // try to create the direct sound
    try{
      if(!m_DirectSound.Create(pMainWnd)){
        m_DirectSound.CreateDummy();
      }
    }
    catch(CException *e){
      // if failed - create it as dummy one (no sound will appear)
      m_DirectSound.CreateDummy();
      e->Delete();
    }

    // start the direct draw
    // switch to the resolution and so on
    if(!InitDirectDraw(800, 600, bExclusive, pMainWnd)){
      TRACE("Can't create DirectDraw.\n");
      return FALSE;
    }

    // create the mouse(primary) surface and cursor
    if(!g_pMouse->CreateMouseSurface(1)){
      TRACE("Mouse failed to create its surface.\n");
      return FALSE;
    }

    // Test the color depth
    {
      DDSURFACEDESC ddsd;
      ddsd.dwSize = sizeof(ddsd);
      g_pDDPrimarySurface->GetSurfaceDesc(&ddsd);
      if(!((ddsd.ddpfPixelFormat.dwRGBBitCount == 32) || (ddsd.ddpfPixelFormat.dwRGBBitCount == 24))){
         throw new CStringException("This application needs 24/32 bits graphics mode to run.");
      }
    }

    // set the mouse sensitivity)
    g_pMouse->SetSensitivity(6, 6);

    // clear the primary surface
    g_pDDPrimarySurface->Fill(0);
    // and if not in exclusive mode - set the screen size
    if(!bExclusive){
      CRect rcScreen;
      pMainWnd->GetClientRect(&rcScreen);
      rcScreen.SetRect(0, 0, rcScreen.Width(), rcScreen.Height());
      g_pDDPrimarySurface->SetScreenRect(rcScreen);
    }

    // create the default system font
    g_pSystemFont = new CGraphicFont();
    ((CGraphicFont *)g_pSystemFont)->Create(m_pRootArchive->CreateFile("standard.fnt"), RGB32(16, 208, 128));

    // create the desktop window
    // this actualy enables the window system
    m_Desktop.Create();

    TRACE("Initializing done.\n");
    TRACE("  Starting main event thread to do the rest...\n");
    // create the second thread
    // we will create it suspended to wait to the application start
    m_pMainEventThread = AfxBeginThread(_MainEventThreadProc, this, THREAD_PRIORITY_NORMAL, 0, CREATE_SUSPENDED);
    m_pMainEventThread->m_bAutoDelete = FALSE;
  }
  catch(CException *e){
    AbortApplication();
    e->ReportError();
    e->Delete();
    return FALSE;
  }

	return TRUE;
}

// Closes the application - must be called from the main application thread
void CGameClientApplication::Close()
{
  try{
    TRACE("Closing the application...\n");

    // delete the desktop window
    m_Desktop.Delete();

    if(g_pSystemFont){
      g_pSystemFont->Delete();
      delete g_pSystemFont;
      g_pSystemFont = NULL;
    }

    if(g_pMouse){
      g_pMouse->DeleteMouseSurface();
    }

    CDirectDraw::Close();

    CMouse::Close();
    m_KeyAcc.Delete();
    CKeyboard::Close();
    CTimer::Close();

    m_DirectSound.Delete();
    m_DirectInput.Delete();

    if(m_pRootArchive){ delete m_pRootArchive; m_pRootArchive = NULL; }
    CDataArchive::Done();

    m_MainEventManager.Delete();

    // now abort the application
    // if there is something left we forgot to close
    AbortApplication();

	CNotifier::Delete();

    TRACE("Closing done.\n");
  }
  catch(CException *e){
    AbortApplication();
    e->ReportError();
    e->Delete();
  }
}


// Initializes the DirectDraw - for nonexclusive mode it's quite a hard work to be done
BOOL CGameClientApplication::InitDirectDraw(DWORD dwWidth, DWORD dwHeight, BOOL bExclusive, CWnd *pMainWnd)
{
  if(bExclusive){
    CDirectDraw::EDisplayModes eMode;
    switch(dwWidth){
    case 640:
      eMode = CDirectDraw::DM_640x480x24;
      break;
    case 800:
      eMode = CDirectDraw::DM_800x600x24;
      break;
    case 1024:
      eMode = CDirectDraw::DM_1024x768x24;
      break;
    default:
      return FALSE;
    }
    return CDirectDraw::Init(pMainWnd, eMode, DDSCL_FULLSCREEN | DDSCL_EXCLUSIVE);
  }
  else{
    CRect rcWindow, rcClient;
    CRect rcNewClient;
    CWnd *pDesktop = pMainWnd->GetDesktopWindow();
    CRect rcDesktop;

    pDesktop->GetWindowRect(&rcDesktop);

    pMainWnd->GetWindowRect(&rcWindow);
    pMainWnd->GetClientRect(&rcClient);
    pMainWnd->ClientToScreen(&rcClient);

    rcNewClient.left = (rcDesktop.Width() - dwWidth) / 2;
    rcNewClient.top = (rcDesktop.Height() - dwHeight) / 2;
    rcNewClient.right = rcNewClient.left + dwWidth;
    rcNewClient.bottom = rcNewClient.top + dwHeight;

    pMainWnd->SetWindowPos(NULL, rcNewClient.left + (rcWindow.left - rcClient.left),
      rcNewClient.top + (rcWindow.top - rcClient.top),
      rcNewClient.Width() + (rcWindow.Width() - rcClient.Width()),
      rcNewClient.Height() + (rcWindow.Height() - rcClient.Height()),
      SWP_NOZORDER);

    BOOL bRet = CDirectDraw::Init(pMainWnd, CDirectDraw::DM_NoChange, DDSCL_NORMAL);

    return bRet;
  }
  return FALSE;
}


//------------------------------------------------------------------------
// Starting functions

// Starts the application main event thread -> the real app start
void CGameClientApplication::Start()
{
  // just resume the second thread
  ASSERT_VALID(m_pMainEventThread);
  m_pMainEventThread->ResumeThread();
}

// Stops the application main event thread -> all ends here
void CGameClientApplication::Stop()
{
  // tell the second thread to exit
  m_MainEventManager.InlayQuitEvent(0);

  // wait for it to finish
  if(m_pMainEventThread != NULL){
    if(::WaitForSingleObject(m_pMainEventThread->m_hThread, INFINITE) != WAIT_OBJECT_0){
      TRACE("Shooting the thread to the head.\n");
      ::TerminateThread(m_pMainEventThread->m_hThread, -1);
    }
    delete m_pMainEventThread;
    m_pMainEventThread = NULL;
  }
}




//------------------------------------------------------------------------
// Main event thread

// the main event thread proc - system one
UINT CGameClientApplication::_MainEventThreadProc(LPVOID pParam)
{
  CGameClientApplication *pThis = (CGameClientApplication *)pParam;
  
  return pThis->MainEventThreadProc();
}

// the main event thread proc - real one
UINT CGameClientApplication::MainEventThreadProc()
{
  try{
    // first attach the manager to this thread
    m_MainEventManager.AttachToThread();
    
    // now do some more initializing
    // this could be done here, we don;t need to do it in the Init function
    // which is called from the main app thread and blocks it

    m_Desktop.SetImage(CDataArchive::GetRootArchive()->CreateFile(g_strStartDirectory + "\\Background.tga"));
    m_Desktop.UpdateRect(NULL);
    m_Desktop.UpdateNow();
    
    // create graphics archive
    // still this is a directory of all graphics and so on
    m_pGraphicsArchive = new CDataArchive();
    m_pGraphicsArchive->Create(m_strStartDirectory + "\\Graphics");
    
    // create the default mouse cursor and set it
    m_Cursor.Create(m_pGraphicsArchive->CreateFile("Mouse\\Arrow.cursor"));
    g_pMouse->SetDefaultCursor(&m_Cursor);
    g_pMouse->SetCursor(&m_Cursor);
    g_pMousePrimarySurface->ShowMouse();

    // Set the waiting cursor
    m_WaitingCursor.Create(m_pGraphicsArchive->CreateFile("Mouse\\Waiting.cursor"));
    g_pMouse->SetWaitingCursor(&m_WaitingCursor);
    
    {
      CWaitingCursor cWait;

      // load all default layouts
      // (this is quite expensive - many disk I/O operations)
      CLayouts::Init(m_pGraphicsArchive->CreateFile("Standard.layouts"));
    }

    // create our application objects
    Create();
    
RedoTheLoop:;
    // now start the event loop
    // we have done all the initilizing things and so on...
    // so we can give this thread to the event manager to resolve the events
    // this will return only after the quit event will be recieved
    UINT ret = m_MainEventManager.DoEventLoop();
    
    // We must activate out window to exit (needed for frame loop thread and especialy for the script editor)
    g_pMainWindow->SetActiveWindow();

    // first stop all hardwork (if some)
    StopHardwork();

    // ask all of us to confirm the exit
    if(!CanClose()){
      // no close -> restart the hardwork
      StartHardwork();

      goto RedoTheLoop;
    }

    // delete the application objects
    Delete();

    // close all layouts
    CLayouts::Close();

    // delete the default cursor
    g_pMouse->SetDefaultCursor(NULL);
    g_pMouse->SetCursor(NULL);
    m_Cursor.Delete();

    g_pMouse->SetWaitingCursor(NULL);
    m_WaitingCursor.Delete();

    // delete the graphics archive
    if(m_pGraphicsArchive){ delete m_pGraphicsArchive; m_pGraphicsArchive = NULL; }

    // and thats all
    // so make sure that the main window was closed
    if(::IsWindow(AfxGetMainWnd()->GetSafeHwnd()))
      AfxGetMainWnd()->PostMessage(WM_CLOSEWINDOW);
  }
  catch(CException *e)
  {
    Delete();

    CLayouts::Close();

    // delete the default cursor
    g_pMouse->SetDefaultCursor(NULL);
    g_pMouse->SetCursor(NULL);
    m_Cursor.Delete();

    g_pMouse->SetWaitingCursor(NULL);
    m_WaitingCursor.Delete();

    // delete the graphics archive
    if(m_pGraphicsArchive){ delete m_pGraphicsArchive; m_pGraphicsArchive = NULL; }

    ::ShowWindow(AfxGetMainWnd()->GetSafeHwnd(), SW_HIDE);
    
    // oops - some fatal error
    // abort the application
    AbortApplication();

    TRACE("Reporting error. ********************************\n");
    // show the error in normal window
    e->ReportError();
    e->Delete();
    TRACE("Reporting done. *****************************\n");

    // and thats all
    // so make sure that the main window was closed
    if(::IsWindow(AfxGetMainWnd()->GetSafeHwnd()))
      AfxGetMainWnd()->PostMessage(WM_CLOSEWINDOW);

    // and escape
    return -1;
  }

  // All was done - return OK
  return 0;
}


//------------------------------------------------------------------------
// Creation

// Creates all objects for the application
// and inlay the first event to start he application work
BOOL CGameClientApplication::Create()
{
  Connect(this, _Self_NotID_);

  // Some network events
  {
    m_eventNetworkEventThreadStopped.SetEvent();
  }

  InlayEvent(E_Connect, 0);

  return TRUE;
}

// Deletes all objects here
void CGameClientApplication::Delete()
{
  // First stop the network event thread
  if(m_pNetworkEventThread != NULL)
  {
    m_NetworkEventManager.InlayQuitEvent(0);
    m_eventRunNetworkEventThread.SetEvent();
    if(WaitForSingleObject(m_eventNetworkEventThreadStopped.m_hObject, INFINITE) != WAIT_OBJECT_0){
      ASSERT(FALSE);
    }
  }

  m_ScriptEditor.Delete();
  m_ScriptEditorScreenWindow.Delete();

  m_MapView.Delete();
  m_MapViewScreenWindow.Delete();

  m_UnitHierarchy.Delete();
  
  // Close the control connection
  {
    VERIFY(m_lockControlConnectionRequest.Lock());
    m_eControlConnectionRequest = ControlConnectionRequest_None;
    m_dwControlConnectionDoneEvent = 0;
    m_ControlConnection.DeleteVirtualConnection();
    VERIFY(m_lockControlConnectionRequest.Unlock());
  }

  DeleteNetwork();

  m_KeyAcc.Disconnect(this);
}

// starts all hardwork
void CGameClientApplication::StartHardwork()
{
  m_MapView.Start();
}

// stops al hard work
void CGameClientApplication::StopHardwork()
{
  m_MapView.Stop();
}

BOOL CGameClientApplication::CanClose()
{
  if(!m_ScriptEditor.CanClose()) return FALSE;

  return TRUE;
}

//------------------------------------------------------------------------
// Map download
//------------------------------------------------------------------------

// Downloads the map from the server
void CGameClientApplication::OnDownloadMap()
{
  // just start the new thread
  m_pDownloadMapThread = AfxBeginThread(_DownloadMapThreadProc, (LPVOID)this);
  m_pDownloadMapThread->m_bAutoDelete = FALSE;
}

// the download is done
void CGameClientApplication::OnMapDownloaded(CException *pException)
{
  // wait for the thread and delete it
  if(WaitForSingleObject((HANDLE)(*m_pDownloadMapThread), INFINITE) != WAIT_OBJECT_0){
    TerminateThread((HANDLE)(*m_pDownloadMapThread), -1);
  }
  delete m_pDownloadMapThread;
  m_pDownloadMapThread = NULL;

  if(pException != NULL){
    // Ooops some error
    char txt[1024];
    pException->GetErrorMessage(txt, 1024, NULL);

    // some message box
    CString strCaption;
    strCaption.LoadString(IDR_MAINFRAME);
    CMessageBox::OK(txt, strCaption, &m_Desktop);

    pException->Delete();

    // close the network
    DeleteNetwork();

    // and connect dialog again
    // 1 - don't read the registry info
    InlayEvent(E_Connect, 1);

    return;
  }

  // just do the next step in the startup
  InlayEvent(E_Init, 0, this);
}

// the helper thread functions for the download
UINT CGameClientApplication::_DownloadMapThreadProc(LPVOID pParam)
{
  CGameClientApplication *pThis = (CGameClientApplication *)pParam;
  ASSERT(pThis != NULL);

  return pThis->DownloadMapThreadProc();
}

// The thread function for the download
UINT CGameClientApplication::DownloadMapThreadProc()
{
  CEventManager EventManager;
  EventManager.Create();

  // just create the download object
  CDownloadMap DownloadMap;

  try{
    if(DownloadMap.DoModal(g_pDesktopWindow) != CDialogWindow::IDC_OK){
      throw new CStringException(IDS_DOWNLOADMAP_NETWORKERROR);
    }
  }
  catch(CException *e){
    DeleteFile(DownloadMap.GetMapPath());
    DownloadMap.Delete();
    InlayEvent(E_MapDownloaded, (DWORD)e, this);
    EventManager.Delete();
    return 0;
  }
  DownloadMap.Delete();
  m_strMapPath = DownloadMap.GetMapPath();

  InlayEvent(E_MapDownloaded, 0, this);
  EventManager.Delete();
  return 0;
}

//------------------------------------------------------------------------
// Init request
//------------------------------------------------------------------------

// Reaction oon start the init info request
void CGameClientApplication::OnInit()
{
  // show the connection window
  m_ProgressWindow.Create(IDS_LOADING, &m_Desktop);

  VERIFY(m_lockControlConnectionRequest.Lock());
  
  // Don't do it if some error in network
  if(!g_pNetworkConnection->IsNetworkReady()){
    InlayEvent(E_InitArrived, 1);
    VERIFY(m_lockControlConnectionRequest.Unlock());
    return;
  }

  // Init the request on control connection
  m_eControlConnectionRequest = ControlConnectionRequest_Init;
  m_dwControlConnectionDoneEvent = E_InitArrived;

  // And send the request to server
  DWORD dwRequest = ControlRequest_Init;
  VERIFY(m_ControlConnection.BeginSendCompoundBlock());
  VERIFY(m_ControlConnection.SendBlock(&dwRequest, sizeof(dwRequest)));
  VERIFY(m_ControlConnection.EndSendCompoundBlock());

  VERIFY(m_lockControlConnectionRequest.Unlock());

  // Start the network event thread
  m_eventRunNetworkEventThread.SetEvent();
}

// Reaction on arriving the init info from server
void CGameClientApplication::OnInitArrived(BOOL bSuccess)
{
  if(!bSuccess){
    // Delete the progress window if some
    m_ProgressWindow.Delete();
    // Display the message box
    CMessageBox::OK(IDS_APPLICATION_INITFAILED, IDR_MAINFRAME, g_pDesktopWindow);
    // End the application
    CEventManager::FindEventManager(GetCurrentThreadId())->InlayQuitEvent(1);
  }
  else{
    // just continue with the process

    // Pause the network again
    m_eventRunNetworkEventThread.ResetEvent();
    InlayEvent(CControlConnectionObserver::E_PauseNetwork, 0, &m_ControlConnectionObserver);

    // Show the mapview
    InlayEvent(E_CreateClient, 0);
  }
}

//------------------------------------------------------------------------
// Network connection
//------------------------------------------------------------------------

// Connection event - show the dialog and start connecting
void CGameClientApplication::OnConnect(DWORD dwParam)
{
  CConnectDlg ConnectDlg;
  CWinApp *pApp = AfxGetApp();

  ASSERT(g_pNetworkConnection == NULL);

  if(dwParam == 0)
    ReadRegistryConnectInfo();

Again:
  ;

  // copy connect info to the dialog
  ConnectDlg.m_strServerName = m_strServerName;
  ConnectDlg.m_dwServerPort = m_wServerPort;
  ConnectDlg.m_strLogin = m_strLogin;
  ConnectDlg.m_strPassword = m_strPassword;

  // open the dialog
  if(ConnectDlg.DoModal(&m_Desktop) != CDialogWindow::IDC_OK){
    // ask to really quit
    if(CMessageBox::YesNo(IDS_REALLYQUIT, IDR_MAINFRAME, &m_Desktop) != CMessageBox::IDC_Yes)
      goto Again;

    // cancel all of us
    AfxGetMainWnd()->PostMessage(WM_CLOSE);
    return;
  }
  
  // copy the connect info back to our variables
  m_strServerName = ConnectDlg.m_strServerName;
  m_wServerPort = (WORD)ConnectDlg.m_dwServerPort;
  m_strLogin = ConnectDlg.m_strLogin;
  m_strPassword = ConnectDlg.m_strPassword;

  // show the connection window
  m_ProgressWindow.Create(IDS_CONNECT_PROGRESS, &m_Desktop);

  // start the network thread
  // we will use it first for the connection
  // and then it will become the network event thread
  m_pNetworkEventThread = AfxBeginThread(_ConnectThreadProc, (LPVOID)this, THREAD_PRIORITY_NORMAL, 0, CREATE_SUSPENDED);
  m_pNetworkEventThread->m_bAutoDelete = FALSE;

  // remember the thread in global variable
  g_dwNetworkEventThreadID = m_pNetworkEventThread->m_nThreadID;

  m_pNetworkEventThread->ResumeThread();
}

// the connection was done
// if the e is not NULL -> some error
// elseway all was successfull
void CGameClientApplication::OnConnected(CNetworkException *e)
{
  // destroy the connection window
  m_ProgressWindow.Delete();

  if((DWORD)e == 1){
    // some message box
    CMessageBox::OK(IDS_CONNECT_NETWORKERROR, IDR_MAINFRAME, &m_Desktop);

    // and connect dialog again
    // 1 - don't read the registry info
    InlayEvent(E_Connect, 1);

    // Delete the network ervent thread
    if(m_pNetworkEventThread != NULL){
      if(WaitForSingleObject(m_pNetworkEventThread->m_hThread, INFINITE) != WAIT_OBJECT_0){
        ASSERT(FALSE);
      }
      delete m_pNetworkEventThread;
      m_pNetworkEventThread = NULL;
    }

    return;
  }
  else if(e != NULL){
    // some message box
    switch(e->GetError()){
    case CNetworkException::eDNSError:
      CMessageBox::OK(IDS_CONNECT_NETWORKERROR_DNSERROR, IDR_MAINFRAME, &m_Desktop);
      break;
    case CNetworkException::eTCPSendError:
      CMessageBox::OK(IDS_CONNECT_NETWORKERROR_SENDTCP, IDR_MAINFRAME, &m_Desktop);
      break;
    case CNetworkException::eTCPReceiveError:
      CMessageBox::OK(IDS_CONNECT_NETWORKERROR_RECEIVETCP, IDR_MAINFRAME, &m_Desktop);
      break;
    case CNetworkException::eCreateSocketError:
      CMessageBox::OK(IDS_CONNECT_NETWORKERROR_CREATESOCKETERROR, IDR_MAINFRAME, &m_Desktop);
      break;
    case CNetworkException::eConnectError:
      CMessageBox::OK(IDS_CONNECT_NETWORKERROR_CONNECT, IDR_MAINFRAME, &m_Desktop);
      break;
    case CNetworkException::eGetPortError:
      CMessageBox::OK(IDS_CONNECT_NETWORKERROR_GETPORTERROR, IDR_MAINFRAME, &m_Desktop);
      break;
    case CNetworkException::eBindError:
      CMessageBox::OK(IDS_CONNECT_NETWORKERROR_BIND, IDR_MAINFRAME, &m_Desktop);
      break;
    case CNetworkException::eBadChallenge:
      CMessageBox::OK(IDS_CONNECT_NETWORKERROR_BADCHALLENGE, IDR_MAINFRAME, &m_Desktop);
      break;
    case CNetworkException::eServerError:
      CMessageBox::OK(IDS_CONNECT_NETWORKERROR_SERVERERROR, IDR_MAINFRAME, &m_Desktop);
      break;
    case CNetworkException::eUnknownHostError:
      CMessageBox::OK(IDS_CONNECT_NETWORKERROR_UNKNOWNHOST, IDR_MAINFRAME, &m_Desktop);
      break;
    case CNetworkException::eWinsockStartupError:
      CMessageBox::OK(IDS_CONNECT_NETWORKERROR_WSASTARTUP, IDR_MAINFRAME, &m_Desktop);
      break;
    case CNetworkException::eWinsockCleanupError:
      CMessageBox::OK(IDS_CONNECT_NETWORKERROR_WSACLEANUP, IDR_MAINFRAME, &m_Desktop);
      break;
    case CNetworkException::eLoginFailed:
      CMessageBox::OK ( IDS_CONNECT_NETWORKERROR_LOGINFAILED, IDR_MAINFRAME, &m_Desktop );
      break;
    case CNetworkException::eLoginBadPassword:
      CMessageBox::OK ( IDS_CONNECT_NETWORKERROR_LOGINBADPASSWORD, IDR_MAINFRAME, &m_Desktop );
      break;
    case CNetworkException::eLoginUnknownUser:
      CMessageBox::OK ( IDS_CONNECT_NETWORKERROR_LOGINUNKNOWNUSER, IDR_MAINFRAME, &m_Desktop );
      break;
      case CNetworkException::eLoginAlreadyLoggedOn:
      CMessageBox::OK ( IDS_CONNECT_NETWORKERROR_LOGINALREADYLOGGEDON, IDR_MAINFRAME, &m_Desktop );
      break;
    case CNetworkException::eLoginGameNotRunning:
      CMessageBox::OK ( IDS_CONNECT_NETWORKERROR_LOGINGAMENOTRUNNING, IDR_MAINFRAME, &m_Desktop );
      break;
    case CNetworkException::eLoginUserDisabled:
      CMessageBox::OK ( IDS_CONNECT_NETWORKERROR_LOGINUSERDISABLED, IDR_MAINFRAME, &m_Desktop );
      break;
    case CNetworkException::eUnknownError:
    default:
      CMessageBox::OK(IDS_CONNECT_NETWORKERROR, IDR_MAINFRAME, &m_Desktop);
      break;
    }

    e->Delete();

    // and connect dialog again
    // 1 - don't read the registry info
    InlayEvent(E_Connect, 1);

    // Delete the network ervent thread
    if(m_pNetworkEventThread != NULL){
      if(WaitForSingleObject(m_pNetworkEventThread->m_hThread, INFINITE) != WAIT_OBJECT_0){
        ASSERT(FALSE);
      }
      delete m_pNetworkEventThread;
      m_pNetworkEventThread = NULL;
    }

    return;
  }

  // save the connect info to registry
  WriteRegistryConnectInfo();

  // Create control connection
  {
    m_eControlConnectionRequest = ControlConnectionRequest_None;
    m_dwControlConnectionDoneEvent = 0;

    DWORD UserData = VirtualConnection_Control;
    m_ControlConnection = g_pNetworkConnection->CreateVirtualConnectionEx(PACKET_GUARANTEED_DELIVERY | PACKET_GUARANTEED_ORDER,
      &m_ControlConnectionObserver, ID_CONTROL_CONNECTION, g_dwNetworkEventThreadID, &UserData, sizeof(UserData));
  }

  // Connect the application to this observer
  // This is the way to pause the network whenever we want
  Connect(&m_ControlConnectionObserver, ID_CONTROL_APPLICATION, g_dwNetworkEventThreadID);

  InlayEvent(E_DownloadMap, 0);
}

BEGIN_OBSERVER_MAP(CGameClientApplication::CControlConnectionObserver, CObserver)
  BEGIN_NOTIFIER(ID_CONTROL_CONNECTION)
    EVENT(E_NETWORKERROR)
      OnNetworkError(); return FALSE;
    EVENT(E_PACKETAVAILABLE)
      OnPacketAvailable(); return FALSE;
  END_NOTIFIER()

  BEGIN_NOTIFIER(ID_CONTROL_APPLICATION)
    EVENT(E_PauseNetwork)
      OnWaitForNetworkToRun(); return FALSE;
  END_NOTIFIER()
END_OBSERVER_MAP(CGameClientApplication::CControlConnectionObserver, CObserver)

// disconnects from network
void CGameClientApplication::DeleteNetwork()
{
  if(!m_bNetworkInitialized) return;

  m_bNetworkInitialized = FALSE;

  // stop the network thread
  if(m_pNetworkEventThread != NULL){
    // stop the network thread event manager
    m_NetworkEventManager.InlayQuitEvent(0);

    m_eventRunNetworkEventThread.SetEvent();
    m_eventNetworkEventThreadExit.SetEvent();

    // wait for it to finish
    if(::WaitForSingleObject(m_pNetworkEventThread->m_hThread, INFINITE) != WAIT_OBJECT_0){
      ::TerminateThread(m_pNetworkEventThread->m_hThread, 0);
    }

    // delete the thread
    delete m_pNetworkEventThread;
    m_pNetworkEventThread = NULL;
  }

}

// connects to network
BOOL CGameClientApplication::ConnectNetwork(CString strServerName, WORD wPort,
                                            CString strLogin, CString strPassword)
{
  int nRetryCount = 1;

RetryIt:
  ;
  try{
    CWaitingCursor cWait;

    m_NetworkClient.Create();
    g_pNetworkConnection = m_NetworkClient.ConnectToServer(strServerName, wPort, strLogin, strPassword,
      this, ID_Network);

#ifdef _DEBUG
    g_pNetworkConnection->SetUDPAliveChecking( FALSE );
#endif

    m_bNetworkInitialized = TRUE;
  }
  catch(CNetworkException *e){
    m_NetworkClient.Delete();

    // if we can retry it -> do it
    nRetryCount--;
    if(nRetryCount > 0){
      e->Delete();
      Sleep(100);
      goto RetryIt;
    }

    InlayEvent(E_Connected, (DWORD)e);

    return FALSE;
  }
  catch(CException *e){
    e->Delete();
    m_NetworkClient.Delete();

    // if we can retry it -> do it
    nRetryCount--;
    if(nRetryCount > 0){
      e->Delete();
      Sleep(100);
      goto RetryIt;
    }

    InlayEvent(E_Connected, (DWORD)1);

    return FALSE;
  }

  InlayEvent(E_Connected, 0);
  return TRUE;
}

UINT CGameClientApplication::_ConnectThreadProc(LPVOID pParam)
{
  CGameClientApplication *pThis = (CGameClientApplication *)pParam;

  return pThis->ConnectThreadProc();
}

// we will run the connection routine in another thread
UINT CGameClientApplication::ConnectThreadProc()
{
  m_eventNetworkEventThreadExit.ResetEvent();
  m_eventNetworkEventThreadStopped.ResetEvent();

  // first create the event manager for this thread
  m_NetworkEventManager.Create();

  try{
    // connect the network
    if(!ConnectNetwork(m_strServerName, m_wServerPort, m_strLogin, m_strPassword))
    {
      m_NetworkEventManager.Delete ();
      return 0;
    }
  
    VERIFY(m_eventRunNetworkEventThread.Lock());

    // start the event manager main loop
    m_NetworkEventManager.DoEventLoop();

  }
  catch(CException *e){
    // report all errors
    char txt[1024];
    e->GetErrorMessage(txt, 1024);
    TRACE("Exception caugth: %s\n", txt);
    ASSERT(FALSE);
    e->Delete();
  }

  m_eventNetworkEventThreadStopped.SetEvent();
  VERIFY(m_eventNetworkEventThreadExit.Lock());

  // Delete the network
  if(g_pNetworkConnection != NULL){
    g_pNetworkConnection->Delete();
    g_pNetworkConnection = NULL;
  }
  m_NetworkClient.DisconnectFromServer();
  m_NetworkClient.Delete();

  m_NetworkEventManager.Delete();

  return 0;
}

// reaction on network error event
void CGameClientApplication::OnNetworkError()
{
  // Send us event to display a special dialog
  InlayEvent(E_NetworkError, 0, this);
}

// reaction on network error in main event thread
void CGameClientApplication::OnNetworkErrorSecondary()
{
  // show the dialog
  CMessageBox::OK(IDS_NETWORKERROR_NOTIFICATION, AFX_IDS_APP_TITLE, g_pDesktopWindow);

  // Show the script editor
  InlayEvent(E_ScriptEditor, 0, this);
}

//------------------------------------------------------------------------
// Control connection routines

void CGameClientApplication::OnControlConnectionError()
{
  // Just inlay event that we're finished
  VERIFY(m_lockControlConnectionRequest.Lock());

  if(m_dwControlConnectionDoneEvent != 0){
    // Inlay the event with error state
    InlayEvent(m_dwControlConnectionDoneEvent, 1);
  }

  m_dwControlConnectionDoneEvent = 0;
  m_eControlConnectionRequest = ControlConnectionRequest_None;

  VERIFY(m_lockControlConnectionRequest.Unlock());
}

void CGameClientApplication::OnControlConnectionPacketAvailable()
{
  VERIFY(m_lockControlConnectionRequest.Lock());

  switch(m_eControlConnectionRequest){

  case ControlConnectionRequest_Init:
    {
      // Receive the command ID
      DWORD dwAnswer;
      DWORD dwSize = 4;
      VERIFY(m_ControlConnection.BeginReceiveCompoundBlock());
      VERIFY(m_ControlConnection.ReceiveBlock(&dwAnswer, dwSize));
      if(dwSize != 4){
        TRACE("ControlConnection - recieved packet is too small.\n");
        break;
      }
      if(dwAnswer != ControlAnswer_Init){
        TRACE("ControlConnection - recieved unexpected answer.\n");
        break;
      }
      
      // Recieve the data

      // First the structure
      SControlAnswerInit h;
      dwSize = sizeof(h);
      VERIFY(m_ControlConnection.ReceiveBlock(&h, dwSize));
      ASSERT(dwSize == sizeof(h));

      // Just store the values for later use in create
      m_dwInitCivilizationID = h.m_dwCivilizationID;
      m_dwInitTimesliceInterval = h.m_dwTimesliceInterval;
      m_dwInitCurrentTimeslice = h.m_dwTimeslice;
      m_dwInitStartPositionX = h.m_dwStartPositionX;
      m_dwInitStartPositionY = h.m_dwStartPositionY;

      // Send event that it's done
      InlayEvent(m_dwControlConnectionDoneEvent, 0);
      // Clear the request
      m_dwControlConnectionDoneEvent = 0;
      m_eControlConnectionRequest = ControlConnectionRequest_None;

      VERIFY(m_ControlConnection.EndReceiveCompoundBlock());
    }
    break;

  case ControlConnectionRequest_None:
    // Ooops some error
    TRACE("ControlConnection - recieved packet when none was expected.\n");
    // Do nothing
    break;
  }

  VERIFY(m_lockControlConnectionRequest.Unlock());
}


//------------------------------------------------------------------------
// Client creation
//------------------------------------------------------------------------

// reaction on the create client event
void CGameClientApplication::OnCreateClient()
{
  // start another thread to create the client (load it)
  m_pClientCreationThread = AfxBeginThread(_ClientCreationThread, (LPVOID)this, THREAD_PRIORITY_NORMAL, 0, CREATE_SUSPENDED);
  m_pClientCreationThread->ResumeThread();
}

// the system creat client thread proc
UINT CGameClientApplication::_ClientCreationThread(LPVOID pParam)
{
  CGameClientApplication *pThis = (CGameClientApplication *)pParam;

  return pThis->ClientCreationThread();
}

// the real client creation thread proc
UINT CGameClientApplication::ClientCreationThread()
{
  // first attach the main event manager to us
  m_MainEventManager.AttachToThread();

  try{
    CWaitingCursor cWait;

    // here create whole client
  // Unit hierarchy
    m_UnitHierarchy.Create();

  // Script editor
    // create the script editor screen window
    m_ScriptEditorScreenWindow.Create(&m_Desktop);
    // hide it
    m_ScriptEditorScreenWindow.HideWindow();

    // create the script editor
    {
      CString strBackupDirectory = m_strStartDirectory + "\\ScriptBackup";
      // create the backup directory
      if(!CreateDirectory(strBackupDirectory, NULL)){
        if(GetLastError() != ERROR_ALREADY_EXISTS)
          throw new CStringException("Can't create directory.\n");
      }

      CString strShadowDirectory = m_strStartDirectory + "\\ScriptShadow";
      // create the backup directory
      if(!CreateDirectory(strShadowDirectory, NULL)){
        if(GetLastError() != ERROR_ALREADY_EXISTS)
          throw new CStringException("Can't create directory.\n");
      }

      m_ScriptEditor.Create(*m_pGraphicsArchive, strBackupDirectory, strShadowDirectory,
        &m_ScriptEditorScreenWindow, m_pMainEventThread->m_nThreadID, &m_UnitHierarchy);
      m_ScriptEditor.Connect(this, IDC_SCRIPTEDITOR);
    }

  // Map view
    // create the map view screen window
    m_MapViewScreenWindow.Create(&m_Desktop);
    // hide it
    m_MapViewScreenWindow.HideWindow();

    // create the map view itself
    {
      m_MapView.Create(&m_MapViewScreenWindow, m_strMapPath, m_pGraphicsArchive,
        m_dwInitCivilizationID, m_dwInitCurrentTimeslice, m_dwInitTimesliceInterval,
        &m_UnitHierarchy);
      m_MapView.Connect(this, IDC_MAPVIEW);
      m_MapView.SetViewportPosition(m_dwInitStartPositionX, m_dwInitStartPositionY);
    }

  }
  catch(CException *e){
    // send an error event
    InlayEvent(E_ClientCreated, (DWORD)e);

    // close us
    return 0;
  }

  // inlay event that we are done
  InlayEvent(E_ClientCreated, 0);

  return 0;
}

// the client was created
void CGameClientApplication::OnClientCreated(CException *e)
{
  // if the e is not NULL -> some error
  if(e != NULL){
    // throw it agaion
    throw e;
  }
  
  m_ProgressWindow.Delete();

  // start the network event loop
  m_eventRunNetworkEventThread.SetEvent();

  // clear the desktop
  m_Desktop.ClearImage();

  // now ping the init request
  InlayEvent(E_MapView, 0);
}


//------------------------------------------------------------------------
// Script editor
//------------------------------------------------------------------------

// Show the script editor
void CGameClientApplication::OnScriptEditor()
{
  m_MapView.Stop();
  m_MapViewScreenWindow.HideWindow();
  m_ScriptEditorScreenWindow.ShowWindow();
}

//------------------------------------------------------------------------
// Map view
//------------------------------------------------------------------------

// Show the map view
void CGameClientApplication::OnMapView()
{
  if(!g_pNetworkConnection->IsNetworkReady()){
    // Can't open the mapview if the network is offline
    return;
  }
  m_ScriptEditorScreenWindow.HideWindow();
  m_MapViewScreenWindow.ShowWindow();
  m_MapView.Start();
}

//------------------------------------------------------------------------
// Registry routines

// reads the connect info from the registry
void CGameClientApplication::ReadRegistryConnectInfo()
{
  CWinApp *pApp = AfxGetApp();

  m_strServerName = pApp->GetProfileString(REGISTRY_CONNECT_SECTION, "Server", "localhost");
  m_wServerPort = (WORD)pApp->GetProfileInt(REGISTRY_CONNECT_SECTION, "Port", 3456);
  m_strLogin = pApp->GetProfileString(REGISTRY_CONNECT_SECTION, "User");
  m_strPassword.Empty();
}

// writes the connection info to the registry
void CGameClientApplication::WriteRegistryConnectInfo()
{
  CWinApp *pApp = AfxGetApp();

  pApp->WriteProfileString(REGISTRY_CONNECT_SECTION, "Server", m_strServerName);
  pApp->WriteProfileInt(REGISTRY_CONNECT_SECTION, "Port", m_wServerPort);
  pApp->WriteProfileString(REGISTRY_CONNECT_SECTION, "User", m_strLogin);
  m_strPassword.Empty();
}


