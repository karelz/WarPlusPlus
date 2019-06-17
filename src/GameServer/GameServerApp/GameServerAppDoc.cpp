// GameServerAppDoc.cpp : implementation of the CGameServerAppDoc class
//

#include "stdafx.h"
#include "GameServerApp.h"

#include "GameServerAppDoc.h"
#include "MainFrm.h"

#include "..\GameServer\SMap.h"
#include "..\GameServer\SFileManager.h"

#include "UserDialog.h"
#include "CivilizationDialog.h"
#include "SendMessageDlg.h"

#include "OptionsPathsPage.h"
#include "OptionsNetworkPage.h"
#include "OptionsDebugPage.h"
#include "OptionsGamePage.h"

#include "ScriptSetMakerDlg.h"
#include "ScriptSetProgressDlg.h"
#include "EditErrorOutput.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define WM_MAPOBSERVEREVENT WM_USER + 1

#define DEFAULT_TIMESLICE_LENGTH 100
#define DEFAULT_SERVER_PORT 3456
#define DEFAULT_TEMP_SUBDIR "\\Temp"

// Registry storage
// Paths
#define PATHS_HIVE "Paths"
#define PATHS_GAMEDATA "GameData"
#define PATHS_GAMETEMP "GameTemp"
#define PATHS_ARCHIVETEMP "ArchiveTemp"
// Network
#define NETWORK_HIVE "Network"
#define NETWORK_PORT "Port"
// Game
#define GAME_HIVE "Game"
#define GAME_TIMESLICELENGTH "TimesliceLength"
#define GAME_SHOWPROFILING "ShowProfiling"
#define GAME_SHOWTRAYINFOS "ShowTrayInfos"
// Debug
#define DEBUG_HIVE "Debug"
#define DEBUG_TRACE_SERVER_LOOPS "TraceServerLoops"
#define DEBUG_TRACE_SERVER_CLIENT_COMMUNICATION "TraceServerClientCommunication"
#define DEBUG_TRACE_SKILL_CALLS "TraceSkillCalls"
#define DEBUG_TRACE_SKILLS "TraceSkills"
#define DEBUG_TRACE_CLIENT_REQUESTS "TraceClientRequests"
#define DEBUG_TRACE_INTERPRET "TraceInterpret"
#define DEBUG_TRACE_INTERPRET_DETAILS "TraceInterpretDetails"
#define DEBUG_TRACE_COMPILED_CODE "TraceCompiledCode"
#define DEBUG_TRACE_NOTIFICATIONS "TraceNotifications"
#define DEBUG_TRACE_NETWORK "TraceNetwork"
#define DEBUG_TRACE_ERRORLOGS "ErrorLogs"
#define DEBUG_DELETE_WORKING_DIRECTORY_ON_CREATE "DeleteWorkingDirectoryOnCreate"
#define DEBUG_DELETE_WORKING_DIRECTORY_ON_DELETE "DeleteWorkingDirectoryOnDelete"
#define DEBUG_UDP_ALIVE "UDPAlive"

#ifdef _DEBUG
	BOOL g_bTraceServerLoops = FALSE;
	BOOL g_bTraceServerClientCommunication = FALSE;
	BOOL g_bTraceSkillCalls = FALSE;
	BOOL g_bTraceSkills = FALSE;
	BOOL g_bDeleteWorkingDirectoryOnCreate = TRUE;
	BOOL g_bDeleteWorkingDirectoryOnDelete = TRUE;
	BOOL g_bTraceClientRequests = FALSE;
	BOOL g_bTraceInterpret = FALSE;
	BOOL g_bTraceInterpretDetails = FALSE;
	BOOL g_bTraceCompiledCode = FALSE;
	BOOL g_bTraceNotifications = FALSE;
	BOOL g_bTraceLoggedErrors = FALSE;
	BOOL g_bTraceNetwork = FALSE;
    BOOL g_bUDPAlive = FALSE;
#endif //_DEBUG

    CGameServerAppDoc::SProfileObserverNode * CGameServerAppDoc::m_pProfileObserverNodes = NULL;

/////////////////////////////////////////////////////////////////////////////
// CGameServerAppDoc

IMPLEMENT_DYNCREATE(CGameServerAppDoc, CDocument)

BEGIN_MESSAGE_MAP(CGameServerAppDoc, CDocument)
	//{{AFX_MSG_MAP(CGameServerAppDoc)
	ON_UPDATE_COMMAND_UI(ID_GAME_PAUSE, OnUpdateGamePause)
	ON_COMMAND(ID_GAME_PAUSE, OnGamePause)
	ON_UPDATE_COMMAND_UI(ID_GAME_STOP, OnUpdateGameStop)
	ON_COMMAND(ID_GAME_STOP, OnGameStop)
	ON_COMMAND(ID_GAME_UNPAUSE, OnGameUnpause)
	ON_UPDATE_COMMAND_UI(ID_GAME_UNPAUSE, OnUpdateGameUnpause)
	ON_UPDATE_COMMAND_UI(ID_FILE_SAVE, OnUpdateFileSave)
	ON_UPDATE_COMMAND_UI(ID_VIEW_OPTIONS, OnUpdateViewOptions)
	ON_COMMAND(ID_VIEW_OPTIONS, OnViewOptions)
	ON_COMMAND(ID_FILE_SAVE, OnFileSave)
	ON_COMMAND(ID_SCRIPTSET_MAKER, OnScriptSetMaker)
	ON_UPDATE_COMMAND_UI(ID_SCRIPTSET_MAKER, OnUpdateScriptSetMaker)
	ON_UPDATE_COMMAND_UI(ID_GAME_SENDMESSAGE, OnUpdateGameSendMessage)
	ON_COMMAND(ID_GAME_SENDMESSAGE, OnGameSendMessage)
	ON_COMMAND(ID_CIVILIZATION_RESETPROFILING, OnCivilizationResetProfiling)
	ON_UPDATE_COMMAND_UI(ID_CIVILIZATION_RESETPROFILING, OnUpdateCivilizationResetProfiling)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

// The Map observer class and its implementation
class CGameServerMapObserver : public CSMap::CSMapObserver
{
public:
  CGameServerMapObserver() { m_pWnd = NULL; };
  virtual ~CGameServerMapObserver() {};

  void Create(CWnd *pWnd){ m_pWnd = pWnd; }
  void Delete() { m_pWnd = NULL; }

 virtual void OnUserLoggedOn( DWORD dwUserID ){ m_pWnd->PostMessage( WM_MAPOBSERVEREVENT, CGameServerAppDoc::Event_UserLoggedOn, dwUserID); };
  virtual void OnUserLoggedOff( DWORD dwUserID ){ m_pWnd->PostMessage( WM_MAPOBSERVEREVENT, CGameServerAppDoc::Event_UserLoggedOff, dwUserID); };
  virtual void OnTimeslice( DWORD dwTimeslice ){ m_pWnd->PostMessage( WM_MAPOBSERVEREVENT, CGameServerAppDoc::Event_Timeslice, dwTimeslice); };

  // Window to send message to
  CWnd *m_pWnd;
};

// The only instance of the observer
CGameServerMapObserver g_cMapObserver;

/////////////////////////////////////////////////////////////////////////////
// CGameServerAppDoc construction/destruction

extern BOOL g_bWinVer50;

CGameServerAppDoc::CGameServerAppDoc()
{
  m_eGameStatus = Game_None;
  m_dwTimesliceLength = DEFAULT_TIMESLICE_LENGTH;

  m_strGameDataDir = g_strStartupDirectory;
  m_strGameTempDir = g_strStartupDirectory + DEFAULT_TEMP_SUBDIR;

  {
    char *pTxt = NULL;
    DWORD dwSize = GetTempPath(0, pTxt);
    pTxt = new char[dwSize + 1];
    GetTempPath(dwSize, pTxt);
    m_strArchiveTempDir = pTxt;
    m_strArchiveTempDir = AfxGetApp()->GetProfileString(PATHS_HIVE, PATHS_ARCHIVETEMP, m_strArchiveTempDir);

Retry:;
    try{
      // Initialize archive subsystem
      CDataArchive::Initialize(m_strArchiveTempDir);
    }
    catch(CException *e){
      if(m_strArchiveTempDir == pTxt){
        AfxMessageBox(IDS_APPLICATION_CANTOPENARCHIVETEMPDIR);
        throw;
      }
      e->Delete();
      m_strArchiveTempDir = pTxt;
      goto Retry;
    }

    delete pTxt;
  }

  m_pProfileObserverNodes = NULL;
  m_nProfilingTimer = -1;
  m_bShowProfiling = true;
  m_bShowTrayInfos = true;

  m_wServerPort = DEFAULT_SERVER_PORT;

  ReadSettingsFromRegistry();
}

CGameServerAppDoc::~CGameServerAppDoc()
{
  SProfileObserverNode * pNode = m_pProfileObserverNodes, *pDel;
  while ( pNode != NULL )
  {
    pDel = pNode;
    pNode = pNode->m_pNext;
    delete pDel;
  }
  m_pProfileObserverNodes = NULL;
  if ( m_nProfilingTimer != -1 )
    KillTimer ( NULL, m_nProfilingTimer );

  WriteSettingToRegistry();

  CDataArchive::Done();
}

BOOL CGameServerAppDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;

  // Set the game status to none
  m_eGameStatus = Game_None;
  SetGameStatusPane(Game_None);
  SetTimeslicePane(0);

	return TRUE;
}



/////////////////////////////////////////////////////////////////////////////
// CGameServerAppDoc serialization

void CGameServerAppDoc::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{
		// TODO: add storing code here
	}
	else
	{
		// TODO: add loading code here
	}
}

/////////////////////////////////////////////////////////////////////////////
// CGameServerAppDoc diagnostics

#ifdef _DEBUG
void CGameServerAppDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void CGameServerAppDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CGameServerAppDoc commands

void CGameServerAppDoc::PostCreate()
{
  g_cMapObserver.Create ( AfxGetMainWnd() );
}

void CGameServerAppDoc::RefreshFrameState()
{
  // Get the frame wnd
  CMainFrame *pFrame = (CMainFrame *)AfxGetMainWnd();
  if(pFrame == NULL) return;

  // Set the title
  {
    CString strTitle;
    if(m_eGameStatus == Game_None){
      strTitle.LoadString(AFX_IDS_APP_TITLE);
    }
    else{
      CString strHlp;
      strHlp.LoadString(AFX_IDS_APP_TITLE);
      strTitle = g_cMap.GetMapName() + " - " + strHlp;
    }
    pFrame->SetWindowText(strTitle);
  }

  SetGameStatusPane(m_eGameStatus);
  if(m_eGameStatus == Game_None) SetTimeslicePane(0);
}

void CGameServerAppDoc::SetGameStatusPane(EGameStatus eStatus)
{
  // Get the frame wnd
  CMainFrame *pFrame = (CMainFrame *)AfxGetMainWnd();
  if(pFrame == NULL) return;
  // Set the pane
  switch(eStatus){
  case Game_None:
    pFrame->SetGameStatusPane(IDS_GAMESTATUS_NONE, (HICON)::LoadImage(AfxGetInstanceHandle(),
      MAKEINTRESOURCE(IDI_GAMESTATUS_NONE), IMAGE_ICON, 0, 0, 0));
    break;
  case Game_Paused:
    pFrame->SetGameStatusPane(IDS_GAMESTATUS_PAUSED, (HICON)::LoadImage(AfxGetInstanceHandle(),
      MAKEINTRESOURCE(IDI_GAMESTATUS_PAUSED), IMAGE_ICON, 0, 0, 0));
    break;
  case Game_Unpaused:
    pFrame->SetGameStatusPane(IDS_GAMESTATUS_UNPAUSED, (HICON)::LoadImage(AfxGetInstanceHandle(),
      MAKEINTRESOURCE(IDI_GAMESTATUS_UNPAUSED), IMAGE_ICON, 0, 0, 0));
    break;
  }
}

void CGameServerAppDoc::SetTimeslicePane(DWORD dwTimeslice)
{
  // Get the frame wnd
  CMainFrame *pFrame = (CMainFrame *)AfxGetMainWnd();
  if(pFrame == NULL) return;

  // Set the pane text
  pFrame->SetTimeslicePane(dwTimeslice);
}

// Opens the document
BOOL CGameServerAppDoc::OnOpenDocument(LPCTSTR lpszPathName) 
{
  if ( strlen ( lpszPathName ) == 0 )
  {
    // Set the game status to none
    m_eGameStatus = Game_None;
    SetGameStatusPane ( Game_None );
    SetTimeslicePane ( 0 );
    return TRUE;
  }

  // First determine the type of the document
  {
    CString strTheMapExt;
    strTheMapExt.LoadString(IDS_FILE_THEMAPEXT);
    CString strSaveExt;
    strSaveExt.LoadString(IDS_FILE_SAVEEXT);

    CString strFileName = lpszPathName;
    CString strExt = strFileName.Mid(strFileName.ReverseFind('.') + 1);

    // Stop the previous game
    StopGame();

    if(strTheMapExt.CompareNoCase(strExt) == 0){

      // We've got TheMap file -> Create the new game
      try{
        CDataArchive Archive;
        Archive.Create(strFileName, CArchiveFile::modeRead | CFile::shareDenyWrite, CDataArchiveInfo::archiveFile);

        // Create the new one
        CreateGame(Archive);

        return TRUE;
      }
      catch(CException *e){
        SetStatusText(0);
        e->ReportError();
        e->Delete();

        // We must close the current document
        CGameServerAppApp *pApp = (CGameServerAppApp *)AfxGetApp();
        pApp->OnFileNew();
        return FALSE;
      }
    }
    else if(strSaveExt.CompareNoCase(strExt) == 0){
      // We've got the savegame
      // Try to open it
      {
        CFile file;
        CFileException e;
        if(!file.Open(strFileName, CFile::modeRead | CFile::shareDenyWrite, &e)){
          // Try to open it as a directory
          CString strDirName;
          strDirName = strFileName.Left(strFileName.ReverseFind('.'));

          try{
            CDataArchive Archive;
            Archive.Create(strDirName, CArchiveFile::modeRead | CFile::shareDenyWrite, CDataArchiveInfo::archiveDirectory);

            // Load it
            LoadGame(Archive);

            return TRUE;
          }
          catch(CException *e){
            SetStatusText(0);
            e->ReportError();
            e->Delete();
            // We must close the current document
            CGameServerAppApp *pApp = (CGameServerAppApp *)AfxGetApp();
            pApp->OnFileNew();
            return FALSE;
          }
        }
        else{
          // Yes, it's OK -> load the game
          try{
            file.Close();
            // Load the game from archive
            CDataArchive Archive;
            Archive.Create(strFileName, CArchiveFile::modeRead | CFile::shareDenyWrite, CDataArchiveInfo::archiveFile);
            // Load it
            LoadGame(Archive);

            return TRUE;
          }
          catch(CException *e){
            SetStatusText(0);
            e->ReportError();
            e->Delete();
            // We must close the current document
            CGameServerAppApp *pApp = (CGameServerAppApp *)AfxGetApp();
            pApp->OnFileNew();
            return FALSE;
          }
        }
      }
    }
  }
  // We must close the current document
  CGameServerAppApp *pApp = (CGameServerAppApp *)AfxGetApp();
  pApp->OnFileNew();
	return FALSE;
}

// The login function
CObserver *GameServerLoginFunction(CString strUser, CString strPassword)
{
  if(!g_cMap.StartUserLogin()) return (CObserver *)CNetworkServer::Login_GameNotRunning;

	CZCivilization *pCivilization;
  DWORD dwUserID;
	switch ( g_cMap.FindUser(strUser, strPassword, pCivilization, dwUserID) )
  {
  case CSMap::UE_OK:
    break;
  case CSMap::UE_BadName:
		g_cMap.FinishUserLogin();
    return (CObserver *)CNetworkServer::Login_UnknownUser;
  case CSMap::UE_BadPassword:
    // ####
		g_cMap.FinishUserLogin();
    return (CObserver *)CNetworkServer::Login_BadPassword;
  case CSMap::UE_UserDisabled:
		g_cMap.FinishUserLogin();
    return (CObserver *)CNetworkServer::Login_UserDisabled;
	}
  
  if(g_cMap.IsUserLoggedOn(dwUserID)){
    /// Already logged on -> no more logging enabled
    g_cMap.FinishUserLogin();
    return (CObserver *)CNetworkServer::Login_AlreadyLoggedOn;
  }

	CObserver *pObserver;
	pObserver = pCivilization->ConnectClient(strUser);

	g_cMap.FinishUserLogin();

	return pObserver;
}

void CGameServerAppDoc::InitGame()
{
  // Initialize network
  m_NetworkServer.Create();

/* modified by Karby
  if ( !m_NetworkServer.PrepareServer ( m_wServerPort, GameServerLoginFunction ) )
  {
    throw new CStringException( IDS_NETWORK_INITFAILED );
  }
*/
  m_NetworkServer.PrepareServer(m_wServerPort, GameServerLoginFunction);
}

void CGameServerAppDoc::CloseGame()
{
    // Close the network
	m_NetworkServer.CloseServer();
	m_NetworkServer.Delete();
}

void CGameServerAppDoc::CreateGame(CDataArchive TheMap)
{
  CWaitCursor cursor;
  SetStatusText(IDS_PROGRESS_CREATEGAME);
  ASSERT(m_eGameStatus == Game_None);

  InitGame();

  try
	{
    CDataArchive cGameData;
    cGameData.Create(m_strGameDataDir, CArchiveFile::modeRead | CFile::shareDenyWrite, CDataArchiveInfo::archiveNone);

    CDataArchive cWorking;
    try
    {
      cWorking.Create(m_strGameTempDir, CArchiveFile::modeReadWrite, CDataArchiveInfo::archiveNone);
    }
    catch ( CException * e )
    {
      // The temp directory doesn't exists -> create it
      CString str;
      str.Format ( IDS_CREATETEMPDIRECTORYQUESTION, m_strGameTempDir );
      if ( AfxMessageBox ( str, MB_YESNO ) == IDYES )
      {
        if ( !CreateDirectory ( m_strGameTempDir, NULL ) )
          throw;

        e->Delete ();
        cWorking.Create ( m_strGameTempDir, CArchiveFile::modeReadWrite, CDataArchiveInfo::archiveNone );
      }
      else
      {
        throw;
      }
    }

#ifdef _DEBUG
    if(g_bDeleteWorkingDirectoryOnCreate){
#endif
    if(!CleanDirectory( m_strGameTempDir )){
      throw new CStringException("Can't delete working directory.");
    }
#ifdef _DEBUG
    }
#endif

    // Create the filemanager
    g_cSFileManager.Create( cGameData, cWorking );
	}
	catch ( CException *)
	{
    CloseGame();
    // close it
    g_cSFileManager.Delete();
    throw;
	}

  try{
    // Set the timeslice length
    g_cMap.SetTimeSliceLength(m_dwTimesliceLength);
    // Create the game
    g_cMap.Create ( TheMap );

	// Register our observer on the map
	g_cMap.RegisterMapObserver( &g_cMapObserver );
  }
  catch(CException *)
  {
    CloseGame();
    // Close the filemanager
    g_cSFileManager.Delete();
    throw;
  }

#ifdef _DEBUG
  if(g_cMap.GetCivilizationCount() > 1){
    DWORD dwID;
    g_cMap.AddUser("Neo", "", 1, dwID);
  }
#endif _DEBUG

  // Start our clear profiling timer
  m_nProfilingTimer = SetTimer ( NULL, 1, 5000, (TIMERPROC)ProfileClearTimerProc );

  // Set the new status
  m_eGameStatus = Game_Paused;
  SetGameStatusPane(Game_Paused);
  SetStatusText(0);
}

void CGameServerAppDoc::StopGame()
{
  CWaitCursor cursor;
  SetStatusText(IDS_PROGRESS_STOPGAME);

  if ( m_nProfilingTimer != -1 )
  {
    KillTimer ( NULL, m_nProfilingTimer );
  }

  if(m_eGameStatus == Game_None){
    SetStatusText(0);
    return;
  }

  m_eGameStatus = Game_None;

  // Unregister our observer from the map
  g_cMap.UnregisterMapObserver(&g_cMapObserver);

  // Stop the game
	g_cMap.Delete ();
	g_cSFileManager.Delete ();

  CloseGame();

#ifdef _DEBUG
  if(g_bDeleteWorkingDirectoryOnDelete){
#endif
    CleanDirectory( m_strGameTempDir );
#ifdef _DEBUG
  }
#endif
  // Set the new status
  SetGameStatusPane ( Game_None );
  SetTimeslicePane ( 0 );
  ((CMainFrame *)AfxGetMainWnd () )->SetInstructionCountersPane ( 0, 0 );
  SetStatusText ( 0 );
  UpdateAllViews(NULL);
}

void CGameServerAppDoc::LoadGame(CDataArchive Archive)
{
  CWaitCursor cursor;
  ASSERT(m_eGameStatus == Game_None); 
  SetStatusText(IDS_PROGRESS_LOADGAME);

  InitGame();

  try
	{
    CDataArchive cGameData;
    cGameData.Create(m_strGameDataDir, CArchiveFile::modeRead | CFile::shareDenyWrite, CDataArchiveInfo::archiveNone);

    CDataArchive cWorking;
    cWorking.Create(m_strGameTempDir, CArchiveFile::modeReadWrite, CDataArchiveInfo::archiveNone);

#ifdef _DEBUG
    if(g_bDeleteWorkingDirectoryOnCreate){
#endif
    if(!CleanDirectory( m_strGameTempDir )){
      throw new CStringException("Can't delete working directory.");
    }
#ifdef _DEBUG
    }
#endif

    // Create the filemanager
    g_cSFileManager.Create( cGameData, cWorking );
	}
	catch ( CException *)
	{
    CloseGame();
    // close it
    g_cSFileManager.Delete();
    throw;
	}

  try{
    // Open the storage
	  CPersistentStorage cStorage;
	  cStorage.Open ( Archive, CPersistentStorage::LOAD );
    // Set the timeslice length
    g_cMap.SetTimeSliceLength(m_dwTimesliceLength);

    CString strFileName;
    // Load the map
    while(!g_cMap.PersistentLoad ( cStorage, strFileName )){
      if(AfxMessageBox(IDS_FILE_BADMAP, MB_YESNO, AFX_IDS_APP_TITLE) != IDYES){
        throw new CStringException("Error loading the game.");
      }
      CString strExt, strFilter, strTitle;
      strExt.LoadString(IDS_FILE_THEMAPEXT);
      strFilter.LoadString(IDS_FILE_THEMAPFILTER);
      strTitle.LoadString(IDS_FILE_THEMAPFIND_TITLE);
      CFileDialog dlg(TRUE, strExt, strFileName, OFN_HIDEREADONLY, strFilter, NULL);
      dlg.m_ofn.lpstrTitle = strTitle;

      if(dlg.DoModal() != IDOK){
        throw new CStringException("Error loading the game.");
      }

      strFileName = dlg.GetPathName();
    }
	  g_cMap.PersistentTranslatePointers ( cStorage );
	  g_cMap.PersistentInit ();
    // Close the storage
	  cStorage.Close ();
  }
  catch(CException *){
    CloseGame();
    // Close the filemanager
    g_cSFileManager.Delete();
    throw;
  }

  // Register us on the map
  g_cMap.RegisterMapObserver ( &g_cMapObserver );

  // Start our clear profiling timer
  m_nProfilingTimer = SetTimer ( NULL, 1, 5000, (TIMERPROC)ProfileClearTimerProc );

  // Set the new status
  m_eGameStatus = Game_Paused;
  SetGameStatusPane(Game_Paused);
  SetStatusText(0);
}

void CGameServerAppDoc::SaveGame(CDataArchive Archive)
{
  CWaitCursor cursor;
  ASSERT(m_eGameStatus != Game_None);
  SetStatusText(IDS_PROGRESS_SAVEGAME);

  try{
    // If the game was in progress we must first pause it
	  if ( m_eGameStatus == Game_Unpaused )
	  {
  		g_cMap.PauseGame ();
  	}

  	CPersistentStorage cStorage;
  	cStorage.Open ( Archive, CPersistentStorage::SAVE );
  	g_cMap.PersistentSave ( cStorage );
  	cStorage.Close ();

  	if ( m_eGameStatus == Game_Unpaused )
  	{
		  g_cMap.UnpauseGame ( TRUE );
	  }
  }
  catch(CException *){
  	if ( m_eGameStatus == Game_Unpaused )
  	{
        g_cMap.UnpauseGame ( TRUE );
    }

    SetStatusText(0);
    throw;
  }

  SetStatusText(0);
}

void CGameServerAppDoc::UnpauseGame()
{
  CWaitCursor cursor;
  ASSERT(m_eGameStatus == Game_Paused);
  SetStatusText(IDS_PROGRESS_UNPAUSEGAME);

  try{
    g_cMap.UnpauseGame ( TRUE );

    m_eGameStatus = Game_Unpaused;
    SetGameStatusPane(Game_Unpaused);
  }
  catch(CException *e){
    e->ReportError();
    e->Delete();
  }

  SetStatusText(0);
}

void CGameServerAppDoc::PauseGame()
{
  CWaitCursor cursor;
  ASSERT(m_eGameStatus == Game_Unpaused);
  SetStatusText(IDS_PROGRESS_PAUSEGAME);

  try{
    g_cMap.PauseGame ();

    m_eGameStatus = Game_Paused;
    SetGameStatusPane(Game_Paused);
  }
  catch(CException *e){
    e->ReportError();
    e->Delete();
  }

  SetStatusText(0);
}

void CGameServerAppDoc::OnCloseDocument() 
{
  StopGame();
  
	CDocument::OnCloseDocument();
}

void CGameServerAppDoc::WriteSettingToRegistry()
{
  CWinApp *pApp = AfxGetApp();

  // Write paths
  pApp->WriteProfileString(PATHS_HIVE, PATHS_GAMEDATA, m_strGameDataDir);
  pApp->WriteProfileString(PATHS_HIVE, PATHS_GAMETEMP, m_strGameTempDir);
  pApp->WriteProfileString(PATHS_HIVE, PATHS_ARCHIVETEMP, m_strArchiveTempDir);

  // Write network
  pApp->WriteProfileInt(NETWORK_HIVE, NETWORK_PORT, (int)m_wServerPort);

  // Write game
  pApp->WriteProfileInt(GAME_HIVE, GAME_TIMESLICELENGTH, (int)m_dwTimesliceLength);
  pApp->WriteProfileInt ( GAME_HIVE, GAME_SHOWPROFILING, m_bShowProfiling ? 1 : 0 );
  pApp->WriteProfileInt ( GAME_HIVE, GAME_SHOWTRAYINFOS, m_bShowTrayInfos ? 1 : 0 );

  // Write debug settings
#ifdef _DEBUG
  pApp->WriteProfileInt(DEBUG_HIVE, DEBUG_TRACE_SERVER_LOOPS, g_bTraceServerLoops ? 1 : 0);
  pApp->WriteProfileInt(DEBUG_HIVE, DEBUG_TRACE_SERVER_CLIENT_COMMUNICATION, g_bTraceServerClientCommunication ? 1 : 0);
  pApp->WriteProfileInt(DEBUG_HIVE, DEBUG_TRACE_SKILL_CALLS, g_bTraceSkillCalls ? 1 : 0);
  pApp->WriteProfileInt(DEBUG_HIVE, DEBUG_TRACE_SKILLS, g_bTraceSkills ? 1 : 0);
  pApp->WriteProfileInt(DEBUG_HIVE, DEBUG_TRACE_CLIENT_REQUESTS, g_bTraceClientRequests ? 1 : 0);
  pApp->WriteProfileInt(DEBUG_HIVE, DEBUG_TRACE_INTERPRET, g_bTraceInterpret ? 1 : 0);
  pApp->WriteProfileInt(DEBUG_HIVE, DEBUG_TRACE_INTERPRET_DETAILS, g_bTraceInterpretDetails ? 1 : 0);
  pApp->WriteProfileInt(DEBUG_HIVE, DEBUG_TRACE_COMPILED_CODE, g_bTraceCompiledCode ? 1 : 0);
  pApp->WriteProfileInt(DEBUG_HIVE, DEBUG_TRACE_NOTIFICATIONS, g_bTraceNotifications ? 1 : 0);
  pApp->WriteProfileInt(DEBUG_HIVE, DEBUG_TRACE_NETWORK, g_bTraceNetwork ? 1 : 0);
  pApp->WriteProfileInt(DEBUG_HIVE, DEBUG_TRACE_ERRORLOGS, g_bTraceLoggedErrors ? 1 : 0);
  pApp->WriteProfileInt(DEBUG_HIVE, DEBUG_DELETE_WORKING_DIRECTORY_ON_CREATE, g_bDeleteWorkingDirectoryOnCreate ? 1 : 0);
  pApp->WriteProfileInt(DEBUG_HIVE, DEBUG_DELETE_WORKING_DIRECTORY_ON_DELETE, g_bDeleteWorkingDirectoryOnDelete ? 1 : 0);
  pApp->WriteProfileInt(DEBUG_HIVE, DEBUG_UDP_ALIVE, g_bUDPAlive ? 1 : 0);
#endif
}

void CGameServerAppDoc::ReadSettingsFromRegistry()
{
  CWinApp *pApp = AfxGetApp();

  // Read paths
  m_strGameDataDir = pApp->GetProfileString(PATHS_HIVE, PATHS_GAMEDATA, m_strGameDataDir);
  m_strGameTempDir = pApp->GetProfileString(PATHS_HIVE, PATHS_GAMETEMP, m_strGameTempDir);
  m_strArchiveTempDir = pApp->GetProfileString(PATHS_HIVE, PATHS_ARCHIVETEMP, m_strArchiveTempDir);

  // Read network
  m_wServerPort = (WORD)pApp->GetProfileInt(NETWORK_HIVE, NETWORK_PORT, DEFAULT_SERVER_PORT);

  // Read game
  m_dwTimesliceLength = pApp->GetProfileInt(GAME_HIVE, GAME_TIMESLICELENGTH, DEFAULT_TIMESLICE_LENGTH);
  m_bShowProfiling = pApp->GetProfileInt ( GAME_HIVE, GAME_SHOWPROFILING, m_bShowProfiling ) == 1;
  m_bShowTrayInfos = pApp->GetProfileInt ( GAME_HIVE, GAME_SHOWTRAYINFOS, m_bShowTrayInfos ) == 1;

  // Read debug settings
#ifdef _DEBUG
  g_bTraceServerLoops = pApp->GetProfileInt(DEBUG_HIVE, DEBUG_TRACE_SERVER_LOOPS, 0) != 0;
  g_bTraceServerClientCommunication = pApp->GetProfileInt(DEBUG_HIVE, DEBUG_TRACE_SERVER_CLIENT_COMMUNICATION, 0) != 0;
  g_bTraceSkillCalls = pApp->GetProfileInt(DEBUG_HIVE, DEBUG_TRACE_SKILL_CALLS, 0) != 0;
  g_bTraceSkills = pApp->GetProfileInt(DEBUG_HIVE, DEBUG_TRACE_SKILLS, 0) != 0;
  g_bTraceClientRequests = pApp->GetProfileInt(DEBUG_HIVE, DEBUG_TRACE_CLIENT_REQUESTS, 0) != 0;
  g_bTraceInterpret = pApp->GetProfileInt(DEBUG_HIVE, DEBUG_TRACE_INTERPRET, 0) != 0;
  g_bTraceInterpretDetails = pApp->GetProfileInt(DEBUG_HIVE, DEBUG_TRACE_INTERPRET_DETAILS, 0) != 0;
  g_bTraceCompiledCode = pApp->GetProfileInt(DEBUG_HIVE, DEBUG_TRACE_COMPILED_CODE, 0) != 0;
  g_bTraceNotifications = pApp->GetProfileInt(DEBUG_HIVE, DEBUG_TRACE_NOTIFICATIONS, 0) != 0;
  g_bTraceNetwork = pApp->GetProfileInt(DEBUG_HIVE, DEBUG_TRACE_NETWORK, 0) != 0;
  g_bTraceLoggedErrors = pApp->GetProfileInt(DEBUG_HIVE, DEBUG_TRACE_ERRORLOGS, 0) != 0;
  g_bDeleteWorkingDirectoryOnCreate = pApp->GetProfileInt(DEBUG_HIVE, DEBUG_DELETE_WORKING_DIRECTORY_ON_CREATE, 1) != 0;
  g_bDeleteWorkingDirectoryOnDelete = pApp->GetProfileInt(DEBUG_HIVE, DEBUG_DELETE_WORKING_DIRECTORY_ON_DELETE, 1) != 0;
  g_bUDPAlive = pApp->GetProfileInt(DEBUG_HIVE, DEBUG_UDP_ALIVE, 0) != 0;
#endif
}

void CGameServerAppDoc::SetStatusText(UINT nID)
{
  CFrameWnd *pWnd = (CFrameWnd *)AfxGetMainWnd();
  if(pWnd == NULL) return;

  if(nID == 0){
    pWnd->SetMessageText(AFX_IDS_IDLEMESSAGE);
  }
  else{
    pWnd->SetMessageText(nID);
  }
}


void CGameServerAppDoc::OnUpdateGamePause(CCmdUI* pCmdUI) 
{
  pCmdUI->Enable(m_eGameStatus == Game_Unpaused);
}
void CGameServerAppDoc::OnGamePause() 
{
  // Pause the game
  PauseGame();
}

void CGameServerAppDoc::OnUpdateGameStop(CCmdUI* pCmdUI) 
{
  pCmdUI->Enable((m_eGameStatus == Game_Unpaused) || (m_eGameStatus == Game_Paused));
}

void CGameServerAppDoc::OnGameStop()
{
  StopGame();

  CGameServerAppApp *pApp = (CGameServerAppApp *)AfxGetApp();
  pApp->OnFileNew();
}

void CGameServerAppDoc::OnUpdateGameUnpause(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(m_eGameStatus == Game_Paused);
}
void CGameServerAppDoc::OnGameUnpause() 
{
  UnpauseGame();
}


void CGameServerAppDoc::FillUsersTree(CTreeCtrl *pTree)
{
  pTree->DeleteAllItems();

  DWORD dwCivilizations = g_cMap.GetCivilizationCount();
  DWORD dwCiv;

  // No civ -> no list
  if(dwCivilizations <= 1) return;

  // Insert civilizations
  HTREEITEM *aCivilizations = new HTREEITEM[dwCivilizations];
  HTREEITEM hItem;
  for(dwCiv = 0; dwCiv < dwCivilizations; dwCiv++){
    hItem = pTree->InsertItem(g_cMap.GetCivilization(dwCiv)->GetCivilizationName(), 3, 3);
    pTree->SetItemData(hItem, (DWORD)g_cMap.GetCivilization(dwCiv));
    aCivilizations[dwCiv] = hItem;
  }

  // Insert users
  DWORD dwUsers = g_cMap.GetUserCount();
  DWORD dwUser, dwID;
  CString strName, strPasswd;
  for(dwUser = 0; dwUser < dwUsers; dwUser++){
    g_cMap.GetUser(dwUser, strName, strPasswd, dwCiv, dwID);
    if(g_cMap.IsUserEnabled(dwID)){
      if(g_cMap.IsUserLoggedOn(dwID))
        hItem = pTree->InsertItem(strName, 0, 0, aCivilizations[dwCiv]);
      else
        hItem = pTree->InsertItem(strName, 1, 1, aCivilizations[dwCiv]);
    }
    else{
      hItem = pTree->InsertItem(strName, 2, 2, aCivilizations[dwCiv]);
    }
    pTree->SetItemData(hItem, dwID);
  }

  for(dwCiv = 1; dwCiv < dwCivilizations; dwCiv++){
    pTree->Expand(aCivilizations[dwCiv], TVE_EXPAND);
  }

  delete aCivilizations;
}

void CGameServerAppDoc::OnUserProperties(DWORD dwUserID)
{
  CUserDialog dlg;
  CString strName, strPasswd;
  DWORD dwCivilization;

  g_cMap.GetUser(dwUserID, strName, strPasswd, dwCivilization);

  dlg.m_bEnableName = FALSE;
  dlg.m_strName = strName;
  dlg.m_strComment.Format(IDS_USER_EXISTSCOMMENT, g_cMap.GetCivilization(dwCivilization)->GetCivilizationName());

  if(dlg.DoModal() == IDOK){
    // Set new password
    g_cMap.EditUser(dwUserID, dlg.m_strName, dlg.m_strPassword, dwCivilization);
  }
}

void CGameServerAppDoc::OnCivilizationProperties(CSCivilization *pCiv)
{
  CCivilizationDialog dlg;

  dlg.m_strName = pCiv->GetCivilizationName();
  DWORD dwColor = pCiv->GetCivilizationColor();
  dlg.m_Color = RGB(dwColor >> 16, (dwColor & 0xFF00) >> 8, dwColor & 0xFF);
  dlg.m_pSCivilization = pCiv;

  if(dlg.DoModal() == IDOK){
  }
}

void CGameServerAppDoc::OnUpdateFileSave(CCmdUI* pCmdUI) 
{
  pCmdUI->Enable(m_eGameStatus != Game_None);
}

void CGameServerAppDoc::OnUpdateViewOptions(CCmdUI* pCmdUI) 
{
  pCmdUI->Enable(TRUE);	
}

void CGameServerAppDoc::OnViewOptions() 
{
  CPropertySheet dlg;
  CString strTitle;
  strTitle.LoadString(IDS_OPTIONS_TITLE);
  dlg.SetTitle(strTitle);
  
  COptionsPathsPage PathsPage(m_eGameStatus);
  COptionsNetworkPage NetworkPage(m_eGameStatus);
  COptionsDebugPage DebugPage(m_eGameStatus);
  COptionsGamePage GamePage(m_eGameStatus);

  PathsPage.m_strArchiveTemp = m_strArchiveTempDir;
  PathsPage.m_strData = m_strGameDataDir;
  PathsPage.m_strGameTemp = m_strGameTempDir;
  dlg.AddPage(&PathsPage);

  GamePage.m_dwTimesliceLength = m_dwTimesliceLength;
  GamePage.m_bShowProfiling = m_bShowProfiling;
  GamePage.m_bShowTrayInfos = m_bShowTrayInfos;
  dlg.AddPage(&GamePage);

  NetworkPage.m_dwPort = m_wServerPort;
  dlg.AddPage(&NetworkPage);

#ifdef _DEBUG
  DebugPage.m_bDeleteWorkingDirectoryOnCreate = g_bDeleteWorkingDirectoryOnCreate;
  DebugPage.m_bDeleteWorkingDirectoryOnDelete = g_bDeleteWorkingDirectoryOnDelete;
  DebugPage.m_bTraceClientRequests = g_bTraceClientRequests;
  DebugPage.m_bTraceInterpret = g_bTraceInterpret;
  DebugPage.m_bTraceInterpretDetails = g_bTraceInterpretDetails;
  DebugPage.m_bTraceCompiledCode = g_bTraceCompiledCode;
  DebugPage.m_bTraceNotifications = g_bTraceNotifications;
  DebugPage.m_bTraceServerClientCommunication = g_bTraceServerClientCommunication;
  DebugPage.m_bTraceServerLoops = g_bTraceServerLoops;
  DebugPage.m_bTraceSkillCalls = g_bTraceSkillCalls;
  DebugPage.m_bTraceSkills = g_bTraceSkills;
  DebugPage.m_bTraceErrorLogs = g_bTraceLoggedErrors;
  DebugPage.m_bTraceNetwork = g_bTraceNetwork;
  DebugPage.m_bUDPAlive = g_bUDPAlive;
  dlg.AddPage(&DebugPage);
#endif

  if(dlg.DoModal() == IDOK){

    m_strArchiveTempDir = PathsPage.m_strArchiveTemp;
    m_strGameDataDir = PathsPage.m_strData;
    m_strGameTempDir = PathsPage.m_strGameTemp;

    m_dwTimesliceLength = GamePage.m_dwTimesliceLength;
    g_cMap.SetTimeSliceLength(m_dwTimesliceLength);

    if ( m_bShowProfiling != (GamePage.m_bShowProfiling ? true : false ) )
    {
      m_bShowProfiling = GamePage.m_bShowProfiling ? true : false;
      UpdateAllViews ( NULL );
    }
    m_bShowTrayInfos = GamePage.m_bShowTrayInfos ? true : false;

    m_wServerPort = (WORD)NetworkPage.m_dwPort;

#ifdef _DEBUG
    g_bDeleteWorkingDirectoryOnCreate = DebugPage.m_bDeleteWorkingDirectoryOnCreate;
    g_bDeleteWorkingDirectoryOnDelete = DebugPage.m_bDeleteWorkingDirectoryOnDelete;
    g_bTraceClientRequests = DebugPage.m_bTraceClientRequests;
    g_bTraceInterpret = DebugPage.m_bTraceInterpret;
    g_bTraceInterpretDetails = DebugPage.m_bTraceInterpretDetails;
	g_bTraceCompiledCode = DebugPage.m_bTraceCompiledCode;
    g_bTraceNotifications = DebugPage.m_bTraceNotifications;
    g_bTraceServerClientCommunication = DebugPage.m_bTraceServerClientCommunication;
    g_bTraceServerLoops = DebugPage.m_bTraceServerLoops;
    g_bTraceSkillCalls = DebugPage.m_bTraceSkillCalls;
    g_bTraceSkills = DebugPage.m_bTraceSkills;
	 g_bTraceLoggedErrors = DebugPage.m_bTraceErrorLogs;
	 g_bTraceNetwork = DebugPage.m_bTraceNetwork;
     g_bUDPAlive = DebugPage.m_bUDPAlive;
#endif

    WriteSettingToRegistry();
  }
}

void CGameServerAppDoc::OnFileSave() 
{
  CMainFrame *pFrame = (CMainFrame *)AfxGetMainWnd();
  pFrame->OnFileSave();
}


void CGameServerAppDoc::OnAddUser(CSCivilization *pCiv)
{
  CUserDialog dlg;

  dlg.m_bEnableName = TRUE;
  dlg.m_strComment.Format(IDS_USER_NEWCOMMENT, pCiv->GetCivilizationName());

Redoit:;
  if(dlg.DoModal() == IDOK){
    DWORD dwUserID;
    if(!g_cMap.AddUser(dlg.m_strName, dlg.m_strPassword, pCiv->GetCivilizationIndex(), dwUserID)){
      AfxMessageBox(IDS_USER_ALREADYEXISTS);
      goto Redoit;
    }
    UpdateAllViews(NULL);
  }
}

void CGameServerAppDoc::OnDeleteUser(DWORD dwUserID)
{
  CString strAsk;
  DWORD dwCiv = 0;
  CString strName, strPassword;
  g_cMap.GetUser(dwUserID, strName, strPassword, dwCiv);
  if(dwCiv == 0) return;
  strAsk.Format(IDS_USER_DELETECONFIRMATION, strName);

  if(AfxMessageBox(strAsk, MB_ICONEXCLAMATION | MB_YESNO) != IDYES)
    return;

  g_cMap.DeleteUser(dwUserID);

  UpdateAllViews(NULL);
}

void CGameServerAppDoc::OnDisableUser(DWORD dwUserID)
{
  CString strAsk;
  DWORD dwCiv = 0;
  CString strName, strPassword;
  g_cMap.GetUser(dwUserID, strName, strPassword, dwCiv);
  if(dwCiv == 0) return;
  strAsk.Format(IDS_USER_DISABLECONFIRMATION, strName);

  if(AfxMessageBox(strAsk, MB_ICONEXCLAMATION | MB_YESNO) != IDYES)
    return;

  // Disable the user
  g_cMap.DisableUser(dwUserID);

  UpdateAllViews(NULL);
}

void CGameServerAppDoc::OnEnableUser(DWORD dwUserID)
{
  g_cMap.EnableUser(dwUserID);

  UpdateAllViews(NULL);
}

BOOL CGameServerAppDoc::IsUserEnabled(DWORD dwUserID)
{
  return g_cMap.IsUserEnabled( dwUserID );
}

BOOL CGameServerAppDoc::IsUserLoggedOn(DWORD dwUserID)
{
  return g_cMap.IsUserLoggedOn( dwUserID );
}

LRESULT CGameServerAppDoc::OnMapObserverEvent(WPARAM wParam, LPARAM lParam)
{
  switch(wParam){
  case Event_UserLoggedOn:
    {
      UpdateAllViews(NULL);
      if ( m_bShowTrayInfos )
      {
        CMainFrame * pFrm = (CMainFrame *)AfxGetMainWnd ();
        // Get the user name
        DWORD dwCiv = 0;
        CString strName, strPassword;
        g_cMap.GetUser( (DWORD)lParam, strName, strPassword, dwCiv);
        CString strInfo;
        strInfo.Format ( IDS_USERLOGGEDON, strName );
        pFrm->DisplayTaskBarNotification ( strInfo );
      }
    }
    break;
  case Event_UserLoggedOff:
    {
      UpdateAllViews(NULL);
      if ( m_bShowTrayInfos )
      {
        CMainFrame * pFrm = (CMainFrame *)AfxGetMainWnd ();
        DWORD dwCiv = 0;
        CString strName, strPassword;
        g_cMap.GetUser((DWORD)lParam, strName, strPassword, dwCiv);
        CString strInfo;
        strInfo.Format ( IDS_USERLOGGEDOFF, strName );
        pFrm->DisplayTaskBarNotification ( strInfo );
      }
    }
    break;
  case Event_Timeslice:
    SetTimeslicePane((DWORD)lParam);
    break;
  }

  return 0;
}

void CGameServerAppDoc::OnScriptSetMaker() 
{
  CScriptSetMakerDlg dlg;

  if(dlg.DoModal() != IDOK) return;

  CScriptSetProgressDlg prog;

  try{
    CDataArchive Destination;

    ::DeleteFile(dlg.m_strDestination);
    Destination.Create(dlg.m_strDestination, CArchiveFile::modeCreate | CArchiveFile::modeReadWrite , CDataArchiveInfo::archiveFile);
    Destination.Open();

    CDataArchive BytecodeDest;
    BytecodeDest = Destination.CreateArchive("Bytecode", TRUE);
    BytecodeDest.Open();

    CDataArchive cGameData;
    cGameData.Create(m_strGameDataDir, CArchiveFile::modeRead | CFile::shareDenyWrite, CDataArchiveInfo::archiveNone);

    CDataArchive cWorking;
    cWorking.Create(m_strGameTempDir, CArchiveFile::modeReadWrite, CDataArchiveInfo::archiveNone);

    // Create the filemanager
    g_cSFileManager.Create( cGameData );

  	CScriptSetMaker ScriptSetMaker;
  	CEditErrorOutput StdErr;

	ScriptSetMaker.Create( dlg.m_strSource, BytecodeDest );

    prog.m_pErrOutput = &StdErr;
    prog.m_pScriptSetMaker = &ScriptSetMaker;
    prog.m_bProgress = TRUE;

    prog.DoModal();

    if(prog.m_bSuccess){
      // Add all sources there
      CDataArchive Source;
      Source.Create(dlg.m_strSource, CArchiveFile::modeRead | CFile::shareDenyWrite, CDataArchiveInfo::archiveDirectory);

      Destination.AppendDir("Source", Source, "", appendRecursive);
    }

    ScriptSetMaker.Delete();
    g_cSFileManager.Delete();
    BytecodeDest.Close();
    Destination.Close();
  }
  catch(CException *e){
    g_cSFileManager.Delete();
    ::DeleteFile(dlg.m_strDestination);
    e->ReportError();
    e->Delete();
  }
  if(!prog.m_bSuccess){
    ::DeleteFile(dlg.m_strDestination);
  }
}

void CGameServerAppDoc::OnUpdateScriptSetMaker(CCmdUI* pCmdUI) 
{
  pCmdUI->Enable(m_eGameStatus == Game_None);
}

// Deletes all file and subdirectories in directory
BOOL CGameServerAppDoc::CleanDirectory ( LPCTSTR lpcszDirectory ) 
{
	// jméno adresáøe
	CString strDirectory ( lpcszDirectory );
	// zkontroluje jméno adresáøe
	if ( strDirectory.IsEmpty () )
	{	// jméno adresáøe je prázdné
		// vrátí pøíznak neúspìchu
		return FALSE;
	}

	// zjistí, je-li jméno adresáøe ukonèeno znakem "\"
	if ( strDirectory.GetAt ( strDirectory.GetLength () - 1 ) != _T('\\') )
	{	// jméno adresáøe není ukonèeno znakem "\"
		// pøidá ke jménu adresáøe znak "\"
		strDirectory += _T("\\");
	}

	// pøipraví jméno hledaného souboru
	CString strFileName = strDirectory + _T("*.*");
	// informace o nalezeném souboru
	WIN32_FIND_DATA sFoundFileInfo;

	// inicializuje první soubor
	HANDLE hFindFile = FindFirstFile ( strFileName, &sFoundFileInfo );
	// zjistí, probìhla-li inicializace hledání souboru v poøádku
	if ( hFindFile == INVALID_HANDLE_VALUE )
	{	// inicializace hledání souborù neuspìla
		// obslouží chybu
        DWORD dwError=GetLastError();

        // V jistych extremnich pripadech proste nehazime vyjimku
        // a povazujeme je za uspesne
        if(dwError==ERROR_ACCESS_DENIED || dwError==ERROR_NO_MORE_FILES)
            return TRUE;
		throw new CFileException ( CFileException::badPath );
	}

	// pøíznak úspìšného smazání souborù adresáøe
	BOOL bSuccess = TRUE;

	// projede soubory v adresáøi
	while ( bSuccess && FindNextFile ( hFindFile, &sFoundFileInfo ) )
	{
		// zjistí jméno souboru
		strFileName = sFoundFileInfo.cFileName;
		// zjistí plné jméno souboru
		CString strFullFileName = strDirectory + strFileName;

		// zjistí, jedná-li se o adresáø
		if ( sFoundFileInfo.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY )
		{	// jedná se o adresáø
			// zjistí, jedná-li se o systémový adresáø
			if ( ( strFileName == _T("..") ) || ( strFileName == _T(".") ) )
			{	// jedná se o systémový adresáø
				// pokraèuje v hledání souborù
				continue;
			}
			// nejedná se o systémový adresáø

			// pokusí se smazat soubory podadresáøe
			if ( bSuccess = CleanDirectory ( strFullFileName + _T("\\") ) )
			{	// podaøilo se smazat soubory adresáøe
				// pokusí se smazat adresáø
				bSuccess = RemoveDirectory ( strFullFileName );
			}
		}
		else
		{	// jedná se o soubor
				// zruší pøípadný read only flag souboru
				SetFileAttributes ( strFullFileName, FILE_ATTRIBUTE_NORMAL );
            // pokusí se smazat soubor
            bSuccess = DeleteFile ( strFullFileName );
		}
	}
	// ukonèí hledání souborù
	FindClose ( hFindFile );

	// vrátí pøíznak úspìchu mazání souborù v adresáøi
	return bSuccess;
}

void CGameServerAppDoc::OnUserSendMessage(DWORD dwUserID)
{
  CSendMessageDlg dlg;

  DWORD dwCiv;
  CString strName, strPassword;
  g_cMap.GetUser(dwUserID, strName, strPassword, dwCiv);

  CSCivilization *pCiv = g_cMap.GetCivilization(dwCiv);
  CZCivilization *pZCiv = pCiv->GetZCivilization();

  pZCiv->LockConnectedClients();
  CZConnectedClient *pClient = pZCiv->GetConnectedClient(strName);
  pZCiv->UnlockConnectedClients();
  if(pClient == NULL){
    return;
  }

  CString strCaption;
  strCaption.Format(IDS_SENDMESSAGE_USER, strName);
  dlg.m_strCaption = strCaption;
  if(dlg.DoModal() != IDOK) return;

  pZCiv->LockConnectedClients();
  pClient = pZCiv->GetConnectedClient(strName);
  if(pClient == NULL){
    pZCiv->UnlockConnectedClients();
    return;
  }
  pClient->SendMessage(dlg.m_strMessage, CZConnectedClient::MsgType_Admin);
  pZCiv->UnlockConnectedClients();
}

void CGameServerAppDoc::OnCivilizationSendMessage(CSCivilization *pCiv)
{
  CSendMessageDlg dlg;

  CString strCaption;
  strCaption.Format(IDS_SENDMESSAGE_CIVILIZATION, pCiv->GetCivilizationName());
  dlg.m_strCaption = strCaption;
  if(dlg.DoModal() != IDOK) return;

  CZCivilization *pZCiv = pCiv->GetZCivilization();

  pZCiv->LockConnectedClients();

  CZConnectedClient *pClient = pZCiv->GetFirstConnectedClient();
  while(pClient != NULL){
    pClient->SendMessage(dlg.m_strMessage, CZConnectedClient::MsgType_Admin);
    pClient = pZCiv->GetNextConnectedClient(pClient);
  }

  pZCiv->UnlockConnectedClients();
}

void CGameServerAppDoc::OnUpdateGameSendMessage(CCmdUI* pCmdUI) 
{
  pCmdUI->Enable(m_eGameStatus == Game_Unpaused);
}

void CGameServerAppDoc::OnGameSendMessage() 
{
	CSendMessageDlg dlg;

  CString strTitle;
  strTitle.LoadString(IDS_SENDMESSAGE_ALL);
  dlg.m_strCaption = strTitle;

  if(dlg.DoModal() != IDOK) return;

  CZCivilization *pZCiv;
  DWORD dwIndex;
  for(dwIndex = 1; dwIndex < g_cMap.GetCivilizationCount(); dwIndex++){
    pZCiv = g_cMap.GetCivilization(dwIndex)->GetZCivilization();

    pZCiv->LockConnectedClients();

    CZConnectedClient *pClient = pZCiv->GetFirstConnectedClient();
    while(pClient != NULL){
      pClient->SendMessage(dlg.m_strMessage, CZConnectedClient::MsgType_Admin);
      pClient = pZCiv->GetNextConnectedClient(pClient);
    }

    pZCiv->UnlockConnectedClients();
  }
}

VOID CGameServerAppDoc::ProfileClearTimerProc ( HWND hwnd, UINT nMsg, UINT_PTR idEvent, DWORD dwTime )
{
  // Clear all profiling counters
  DWORD dwCivilization;
  for ( dwCivilization = 0; dwCivilization < g_cMap.GetCivilizationCount (); dwCivilization++ )
  {
    CZCivilization *pCivilization = g_cMap.GetCivilization ( dwCivilization )->GetZCivilization ();
    pCivilization->GetCodeManager ()->ClearLocalCounters ();
  }

  // Send messages to update the UI
  SProfileObserverNode *pNode = m_pProfileObserverNodes;
  while ( pNode )
  {
    pNode->m_pWindow->PostMessage ( WM_PROFILEUPDATED, 0, 0 );
    pNode = pNode->m_pNext;
  }
}

// register the window to recieve the WM_PROFILEUPDATE message
// when the profiling counters had been updated
void CGameServerAppDoc::RegisterProfileObserver ( CWnd * pWindow )
{
  // Try to find it in the list
  SProfileObserverNode *pNode = m_pProfileObserverNodes;
  while ( pNode != NULL )
  {
    // Allready registered
    if ( pNode->m_pWindow == pWindow ) return;
    pNode = pNode->m_pNext;
  }

  // Create new node
  SProfileObserverNode *pNewNode = new SProfileObserverNode ();
  pNewNode->m_pWindow = pWindow;
  pNewNode->m_pNext = m_pProfileObserverNodes;
  m_pProfileObserverNodes = pNewNode;
}

// unregisters the window from the profiling events
void CGameServerAppDoc::UnregisterProfileObserver ( CWnd * pWindow )
{
  SProfileObserverNode **pNode = &m_pProfileObserverNodes, *pDel;
  while ( (*pNode) != NULL )
  {
    if ( (*pNode)->m_pWindow == pWindow ) break;
    pNode = &((*pNode)->m_pNext);
  }
  if ( (*pNode) == NULL ) return;
  pDel = *pNode;
  *pNode = (*pNode)->m_pNext;
  delete pDel;
}

void CGameServerAppDoc::OnCivilizationResetProfiling() 
{
  if ( m_eGameStatus == Game_None ) return;

  // Clear all profiling counters
  DWORD dwCivilization;
  for ( dwCivilization = 0; dwCivilization < g_cMap.GetCivilizationCount (); dwCivilization++ )
  {
    CZCivilization *pCivilization = g_cMap.GetCivilization ( dwCivilization )->GetZCivilization ();
    pCivilization->GetCodeManager ()->ClearGlobalAndLocalCounters ();
  }

  // Send messages to update the UI
  SProfileObserverNode *pNode = m_pProfileObserverNodes;
  while ( pNode )
  {
    pNode->m_pWindow->PostMessage ( WM_PROFILEUPDATED, 0, 0 );
    pNode = pNode->m_pNext;
  }
}

void CGameServerAppDoc::OnUpdateCivilizationResetProfiling(CCmdUI* pCmdUI) 
{ pCmdUI->Enable ( m_eGameStatus != Game_None ); }
