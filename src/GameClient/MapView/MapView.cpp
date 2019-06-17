// MapView.cpp: implementation of the CMapView class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "..\GameClient.h"
#include "MapView.h"

#include "..\GameClientNetwork.h"
#include "..\GameClientGlobal.h"

#include "Common\ServerClient\MapViewConnection.h"

#include "..\UnitHierarchy\UnitHierarchy.h"

#include "..\GameClientHelperFunctions.h"

CCMap *g_pMap = NULL;
CCCivilization *g_pCivilization = NULL;

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

#define MAPVIEW_SCROLLDELAY 10
#define MAPVIEW_CORNERSIZE 30
#define MAPVIEW_SCROLLAMOUNT 10

#define MAPVIEW_MSGBAR_TOP 40
#define MAPVIEW_MSGBAR_HEIGHT 60

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

IMPLEMENT_DYNAMIC(CMapView, CWindow);

BEGIN_OBSERVER_MAP(CMapView, CWindow)
  BEGIN_NOTIFIER(IDC_MSHTop)
    EVENT(CMapScrollHelper::E_SCROLLEVENT)
      OnMapScroll(0, -MAPVIEW_SCROLLAMOUNT); return FALSE;
  END_NOTIFIER()
  BEGIN_NOTIFIER(IDC_MSHTopLeft)
    EVENT(CMapScrollHelper::E_SCROLLEVENT)
      OnMapScroll(-MAPVIEW_SCROLLAMOUNT, -MAPVIEW_SCROLLAMOUNT); return FALSE;
  END_NOTIFIER()
  BEGIN_NOTIFIER(IDC_MSHTopRight)
    EVENT(CMapScrollHelper::E_SCROLLEVENT)
      OnMapScroll(MAPVIEW_SCROLLAMOUNT, -MAPVIEW_SCROLLAMOUNT); return FALSE;
  END_NOTIFIER()
  BEGIN_NOTIFIER(IDC_MSHRight)
    EVENT(CMapScrollHelper::E_SCROLLEVENT)
      OnMapScroll(MAPVIEW_SCROLLAMOUNT, 0); return FALSE;
  END_NOTIFIER()
  BEGIN_NOTIFIER(IDC_MSHBottomRight)
    EVENT(CMapScrollHelper::E_SCROLLEVENT)
      OnMapScroll(MAPVIEW_SCROLLAMOUNT, MAPVIEW_SCROLLAMOUNT); return FALSE;
  END_NOTIFIER()
  BEGIN_NOTIFIER(IDC_MSHBottom)
    EVENT(CMapScrollHelper::E_SCROLLEVENT)
      OnMapScroll(0, MAPVIEW_SCROLLAMOUNT); return FALSE;
  END_NOTIFIER()
  BEGIN_NOTIFIER(IDC_MSHBottomLeft)
    EVENT(CMapScrollHelper::E_SCROLLEVENT)
      OnMapScroll(-MAPVIEW_SCROLLAMOUNT, MAPVIEW_SCROLLAMOUNT); return FALSE;
  END_NOTIFIER()
  BEGIN_NOTIFIER(IDC_MSHLeft)
    EVENT(CMapScrollHelper::E_SCROLLEVENT)
      OnMapScroll(-MAPVIEW_SCROLLAMOUNT, 0); return FALSE;
  END_NOTIFIER()

  BEGIN_NOTIFIER(CMapViewToolbar::IDC_SCRIPTEDITOR)
    ON_BUTTONCOMMAND(OnScriptEditor);
  END_NOTIFIER()
  BEGIN_NOTIFIER(CMapViewToolbar::IDC_MINIMAP)
    ON_BUTTONCOMMAND(OnToggleMinimap);
  END_NOTIFIER()
  BEGIN_NOTIFIER(CMapViewToolbar::IDC_SELECTION)
    ON_BUTTONCOMMAND(OnToggleSelection);
  END_NOTIFIER()

  BEGIN_NOTIFIER(IDC_MINIMAP)
    EVENT(CMinimapFrame::E_CLOSEWINDOW)
      OnToggleMinimap(); return FALSE;
  END_NOTIFIER()

  BEGIN_NOTIFIER(IDC_SELECTION)
    EVENT(CSelectionWindow::E_Close)
      OnToggleSelection(); return FALSE;
  END_NOTIFIER()

  BEGIN_NOTIFIER(ID_UnitSelection)
    EVENT(CUnitSelection::E_SelectionChanged)
      OnUnitSelectionChanged(); return FALSE;
  END_NOTIFIER()

  BEGIN_NOTIFIER(IDC_UNITTOOLBAR)
    EVENT(CUnitToolbar::E_Command)
      OnUnitToolbarCommand((CToolbarButtonInfo *)dwParam); return FALSE;
  END_NOTIFIER()

  BEGIN_NOTIFIER(IDC_VIEWPORT)
    EVENT(CInteractiveViewport::E_Cancel)
      OnViewportCancel(); return FALSE;
    EVENT(CInteractiveViewport::E_Position)
      OnViewportPosition(dwParam >> 16, dwParam & 0x0FFFF); return FALSE;
    EVENT(CInteractiveViewport::E_Unit)
      OnViewportUnit((CCUnit *)dwParam); return FALSE;
    EVENT ( CInteractiveViewport::E_WatchUnit )
      OnViewportWatchUnit ( (CCUnit *)dwParam ); return FALSE;
    EVENT ( CInteractiveViewport::E_RightClickPosition )
      OnViewportRightClickPosition ( dwParam >> 16, dwParam & 0x0FFFF ); return FALSE;
    EVENT ( CInteractiveViewport::E_RightClickUnit )
      OnViewportRightClickUnit ( (CCUnit *)dwParam ); return FALSE;
    EVENT ( CInteractiveViewport::E_ShiftRightClickPosition )
      OnViewportShiftRightClickPosition ( dwParam >> 16, dwParam & 0x0FFFF ); return FALSE;
    EVENT ( CInteractiveViewport::E_ShiftRightClickUnit )
      OnViewportShiftRightClickUnit ( (CCUnit *)dwParam ); return FALSE;
  END_NOTIFIER()

  BEGIN_NOTIFIER ( IDC_WatchUnitFrame )
    EVENT ( CWatchUnitFrame::E_Close )
      OnWatchUnitFrameClose ( (CWatchUnitFrame *)dwParam ); return FALSE;
  END_NOTIFIER ()

  BEGIN_NOTIFIER(ID_MapViewConnection)
    EVENT(E_PACKETAVAILABLE)
      OnMapViewConnectionPacketAvailable(); return FALSE;
    EVENT(E_NETWORKERROR)
      OnMapViewConnectionNetworkError(); return FALSE;
  END_NOTIFIER()

  BEGIN_KEYBOARD()
    ON_KEYACCELERATOR()
  END_KEYBOARD()

END_OBSERVER_MAP(CMapView, CWindow)

// Constructor
CMapView::CMapView() :
  m_eventPauseFrameLoop(FALSE, TRUE),
  m_eventExitFrameLoop(FALSE, TRUE),
  m_eventCanContinueFrameLoop(FALSE, TRUE),
  m_eventFrameLoopPaused(TRUE, TRUE)
{
  m_pFrameLoopThread = NULL;

  m_eMapViewState = MapViewState_Normal;
  m_bNoServerAction = FALSE;
  m_eMapViewQuestion = MapViewQuestion_None;
}

// Destructor
CMapView::~CMapView()
{
  ASSERT(m_pFrameLoopThread == NULL);
}


// Debug functions
#ifdef _DEBUG

void CMapView::AssertValid() const
{
  CWindow::AssertValid();
}

void CMapView::Dump(CDumpContext &dc) const
{
  CWindow::Dump(dc);
}

#endif



// Creation
void CMapView::Create(CWindow *pParent, CString &strMapPath, CDataArchive *pGraphicsArchive,
                      DWORD dwCivilizationID, DWORD dwTimeslice, DWORD dwTimesliceInterval,
                      CUnitHierarchy *pUnitHierarchy)
{
  ASSERT_VALID(pParent);
  ASSERT(pGraphicsArchive);

  m_cGraphicsArchive = *pGraphicsArchive;

  // Create some fonts
  m_MiniFont.Create(m_cGraphicsArchive.CreateFile("Fonts\\Mini.fnt"));

  // Create the MapView connection
  {
    DWORD dwUserData = VirtualConnection_MapView;
    m_MapViewConnection = g_pNetworkConnection->CreateVirtualConnectionEx( PACKET_TYPE1, this,
      ID_MapViewConnection, 0, &dwUserData, sizeof(dwUserData));
    // This is unusual - we'll get these events in normal thread
    // not in the network event thread (no need for locking)
  }

  // Update the toolbars from the server
  {
    // Create the directory for toolbars
    try{
      CDataArchive::GetRootArchive ()->MakeDirectory ( g_strStartDirectory + "\\Toolbars" );
    }
    catch ( CException * e)
    { e->Delete (); }

    m_eventToolbarsUpdateDone.ResetEvent ();

    // Just send the request
    DWORD dwRequest = MapViewRequest_ToolbarFileUpdate;
    VERIFY ( m_MapViewConnection.BeginSendCompoundBlock () );
    VERIFY ( m_MapViewConnection.SendBlock ( &dwRequest, sizeof ( dwRequest ) ) );
    VERIFY ( m_MapViewConnection.EndSendCompoundBlock () );

    // And wait
    // We can do it this way, cause we know that the network events will arrive to the main event thread
    // (it's connected that way) and we are in another (client creation) thread
    VERIFY ( m_eventToolbarsUpdateDone.Lock () );
  }

  // Load toolbars configs
  {
    CDataArchive Archive;
    Archive.Create(g_strStartDirectory + "\\Toolbars");
    m_ToolbarsConfig.Create(Archive);
  }

  // we are transparent
  m_bTransparent = TRUE;

  // create the window itself
  CWindow::Create(g_pDDPrimarySurface->GetScreenRect(), pParent);

  // Create the video buffer
  // We won't take care if it's really in the video memory
  m_VideoBuffer.SetVideoMemory();
  m_VideoBuffer.SetWidth(g_pDDPrimarySurface->GetScreenRect()->Width());
  m_VideoBuffer.SetHeight(g_pDDPrimarySurface->GetScreenRect()->Height());
  m_VideoBuffer.Create();

  // load the map
  {
    CDataArchive TheMapArchive;
    TheMapArchive.Create(strMapPath, CArchiveFile::modeRead, 0);
    m_Map.Create(TheMapArchive);
    g_pMap = &m_Map;
  }

  // get our civilization
  m_pCivilization = m_Map.GetCivilization(dwCivilizationID);
  g_pCivilization = m_pCivilization;

  // Initialize the timeslice estimation
  m_Map.InitTimesliceIntervalEstimation(dwTimeslice, dwTimesliceInterval);

  // create server info receiver structures
  // We can pass the resources bar here, because the newtwork notification
  // will arrive after this function returns (CMapView::Create)
  m_ServerUnitInfoReceiver.Create(g_pNetworkConnection, &m_Map, &m_wndResourcesBar);

  // start unit interpolations
  m_UnitInterpolations.Create(&m_Map, m_ServerUnitInfoReceiver.GetUnitCache());

  // Create the selection object
  m_UnitSelection.Create( m_ServerUnitInfoReceiver.GetUnitCache () );
  m_UnitSelection.Connect(this, ID_UnitSelection);

  // create the main viewport
  m_MainViewport.Create(*(g_pDDPrimarySurface->GetScreenRect()), &m_Map, &m_ServerUnitInfoReceiver, this,
    &m_UnitSelection, &m_cGraphicsArchive, &m_VideoBuffer);
  m_MainViewport.Connect(this, IDC_VIEWPORT);
  m_MainViewport.EnableRectangleSelection ();

  // Create the scroll helper windows
  {
    CRect rcHelperWindow;
    CRect *pAll = g_pDDPrimarySurface->GetScreenRect();
    m_NWCur.Create(m_cGraphicsArchive.CreateFile("Mouse\\Scroll\\ScrollNW.cursor"));
    rcHelperWindow.SetRect(0, 0, MAPVIEW_CORNERSIZE, 1);
    m_MSHTL1.Create(rcHelperWindow, this, MAPVIEW_SCROLLDELAY);
    m_MSHTL1.Connect(this, IDC_MSHTopLeft);
    m_MSHTL1.SetWindowCursor(&m_NWCur);
    m_NCur.Create(m_cGraphicsArchive.CreateFile("Mouse\\Scroll\\ScrollN.cursor"));
    rcHelperWindow.SetRect(MAPVIEW_CORNERSIZE, 0, pAll->Width() - MAPVIEW_CORNERSIZE, 1);
    m_MSHTop.Create(rcHelperWindow, this, MAPVIEW_SCROLLDELAY);
    m_MSHTop.Connect(this, IDC_MSHTop);
    m_MSHTop.SetWindowCursor(&m_NCur);
    m_NECur.Create(m_cGraphicsArchive.CreateFile("Mouse\\Scroll\\ScrollNE.cursor"));
    rcHelperWindow.SetRect(pAll->Width() - MAPVIEW_CORNERSIZE, 0, pAll->Width(), 1);
    m_MSHTR1.Create(rcHelperWindow, this, MAPVIEW_SCROLLDELAY);
    m_MSHTR1.Connect(this, IDC_MSHTopRight);
    m_MSHTR1.SetWindowCursor(&m_NECur);
    rcHelperWindow.SetRect(pAll->Width() - 1, 1, pAll->Width(), MAPVIEW_CORNERSIZE);
    m_MSHTR2.Create(rcHelperWindow, this, MAPVIEW_SCROLLDELAY);
    m_MSHTR2.Connect(this, IDC_MSHTopRight);
    m_MSHTR2.SetWindowCursor(&m_NECur);
    m_ECur.Create(m_cGraphicsArchive.CreateFile("Mouse\\Scroll\\ScrollE.cursor"));
    rcHelperWindow.SetRect(pAll->Width() - 1, MAPVIEW_CORNERSIZE, pAll->Width(), pAll->Height() - MAPVIEW_CORNERSIZE);
    m_MSHRight.Create(rcHelperWindow, this, MAPVIEW_SCROLLDELAY);
    m_MSHRight.Connect(this, IDC_MSHRight);
    m_MSHRight.SetWindowCursor(&m_ECur);
    m_SECur.Create(m_cGraphicsArchive.CreateFile("Mouse\\Scroll\\ScrollSE.cursor"));
    rcHelperWindow.SetRect(pAll->Width() - 1, pAll->Height() - MAPVIEW_CORNERSIZE, pAll->Width(), pAll->Height());
    m_MSHBR1.Create(rcHelperWindow, this, MAPVIEW_SCROLLDELAY);
    m_MSHBR1.Connect(this, IDC_MSHBottomRight);
    m_MSHBR1.SetWindowCursor(&m_SECur);
    rcHelperWindow.SetRect(pAll->Width() - MAPVIEW_CORNERSIZE, pAll->Height() - 1, pAll->Width(), pAll->Height());
    m_MSHBR2.Create(rcHelperWindow, this, MAPVIEW_SCROLLDELAY);
    m_MSHBR2.Connect(this, IDC_MSHBottomRight);
    m_MSHBR2.SetWindowCursor(&m_SECur);
    m_SCur.Create(m_cGraphicsArchive.CreateFile("Mouse\\Scroll\\ScrollS.cursor"));
    rcHelperWindow.SetRect(MAPVIEW_CORNERSIZE, pAll->Height() - 1, pAll->Width() - MAPVIEW_CORNERSIZE, pAll->Height());
    m_MSHBottom.Create(rcHelperWindow, this, MAPVIEW_SCROLLDELAY);
    m_MSHBottom.Connect(this, IDC_MSHBottom);
    m_MSHBottom.SetWindowCursor(&m_SCur);
    m_SWCur.Create(m_cGraphicsArchive.CreateFile("Mouse\\Scroll\\ScrollSW.cursor"));
    rcHelperWindow.SetRect(0, pAll->Height() - 1, MAPVIEW_CORNERSIZE, pAll->Height());
    m_MSHBL1.Create(rcHelperWindow, this, MAPVIEW_SCROLLDELAY);
    m_MSHBL1.Connect(this, IDC_MSHBottomLeft);
    m_MSHBL1.SetWindowCursor(&m_SWCur);
    rcHelperWindow.SetRect(0, pAll->Height() - MAPVIEW_CORNERSIZE, 1, pAll->Height() - 1);
    m_MSHBL2.Create(rcHelperWindow, this, MAPVIEW_SCROLLDELAY);
    m_MSHBL2.Connect(this, IDC_MSHBottomLeft);
    m_MSHBL2.SetWindowCursor(&m_SWCur);
    m_WCur.Create(m_cGraphicsArchive.CreateFile("Mouse\\Scroll\\ScrollW.cursor"));
    rcHelperWindow.SetRect(0, MAPVIEW_CORNERSIZE, 1, pAll->Height() - MAPVIEW_CORNERSIZE);
    m_MSHLeft.Create(rcHelperWindow, this, MAPVIEW_SCROLLDELAY);
    m_MSHLeft.Connect(this, IDC_MSHLeft);
    m_MSHLeft.SetWindowCursor(&m_WCur);
    rcHelperWindow.SetRect(0, 0, 1, MAPVIEW_CORNERSIZE);
    m_MSHTL2.Create(rcHelperWindow, this, MAPVIEW_SCROLLDELAY);
    m_MSHTL2.Connect(this, IDC_MSHTopLeft);
    m_MSHTL2.SetWindowCursor(&m_NWCur);
  }

  // Create the status bar
  m_wndStatusBar.SetTopMost();
  m_wndStatusBar.Create(m_cGraphicsArchive.CreateArchive("MapView", FALSE), &m_MainViewport);

  {
    CRect rc(0, MAPVIEW_MSGBAR_TOP, m_MainViewport.GetWindowPosition().Width(), MAPVIEW_MSGBAR_TOP + MAPVIEW_MSGBAR_HEIGHT);
    // Create the message bar
    m_wndMessageBar.Create(rc, &m_MainViewport, m_cGraphicsArchive.CreateArchive("MapView", FALSE));
  }

  // Create the mini toolbar at bottom right
  m_wndToolbar.SetTopMost();
  m_wndToolbar.Create(&m_MainViewport, m_cGraphicsArchive.CreateArchive("MapView", FALSE), this);
  // Create the resources bar
  m_wndResourcesBar.Create(&m_MainViewport, &m_MiniFont);
  
  // Create the unit toolbar
  {
    CRect rcRect(600, 500, 750, 540);
    m_wndUnitToolbar.Create(rcRect, &m_MainViewport, m_cGraphicsArchive.CreateArchive("MapView", FALSE), &m_MainViewport);
    m_wndUnitToolbar.Connect(this, IDC_UNITTOOLBAR);
    ReadWindowPositionFromRegistry ( "Layout\\MapView\\UnitToolBar", &m_wndUnitToolbar, &rcRect, true  );
    m_wndUnitToolbar.SetToolbarInfo ( m_ToolbarsConfig.GetNoSelectionToolbar () );
  }

  // Create the watch unit frames
  {
    // Just load the layout
    m_cWatchUnitFrameLayout.Create ( m_cGraphicsArchive.CreateFile ( "Windows\\CaptionNonTransparent.window" ) );
  }

  // Create the selection window
  {
    CRect rcRect(20, 20, 150, 100);
    m_wndSelection.Create(rcRect, &m_MainViewport, &m_UnitSelection, m_cGraphicsArchive.CreateArchive("MapView", FALSE), &m_MainViewport);
    m_wndSelection.Connect(this, IDC_SELECTION);

    ReadWindowPositionFromRegistry ( "Layout\\MapView\\Selection", &m_wndSelection, &rcRect, true );
  }

  // Create the minimap frame
  {
    CRect rcRect(20, 400, 200, 580);
    m_wndMinimapFrame.Create(rcRect, &m_MainViewport, &m_Map, &m_cGraphicsArchive, &m_MainViewport,
      m_ServerUnitInfoReceiver.GetMiniMapClip());
    m_wndMinimapFrame.Connect(this, IDC_MINIMAP);

    ReadWindowPositionFromRegistry ( "Layout\\MapView\\Minimap", &m_wndMinimapFrame, &rcRect, true );
  }

  // Connect our accelerators
  {
    // Escape -> for canceling server actions
    g_pKeyAccelerators->Connect ( this, VK_ESCAPE, 0 );
    // Tabelator -> life bar marking toggle
    g_pKeyAccelerators->Connect ( this, VK_TAB, 0 );
  }

  // start the frame loop thread (paused)
  m_eventPauseFrameLoop.SetEvent();
  m_eventCanContinueFrameLoop.ResetEvent();
  m_eventExitFrameLoop.ResetEvent();
  m_pFrameLoopThread = AfxBeginThread(_FrameLoopThread, this, THREAD_PRIORITY_BELOW_NORMAL,
    0, CREATE_SUSPENDED);
  m_pFrameLoopThread->m_bAutoDelete = FALSE;
  // Attach our event manager to that thread
  CEventManager::FindEventManager ( GetCurrentThreadId () )->AttachToThread ( m_pFrameLoopThread->m_nThreadID );
  m_pFrameLoopThread->ResumeThread();

  // Set position of the main viewport
  m_MainViewport.SetPosition(0, 0);
}

// Deletion
void CMapView::Delete()
{
  // stop the frame loop thread
  if(m_pFrameLoopThread != NULL){
    m_eventExitFrameLoop.SetEvent();
    m_eventPauseFrameLoop.ResetEvent();
    m_eventCanContinueFrameLoop.SetEvent();
    if(WaitForSingleObject((HANDLE)*m_pFrameLoopThread, INFINITE) != WAIT_OBJECT_0)
      TerminateThread((HANDLE)*m_pFrameLoopThread, -1);
    delete m_pFrameLoopThread;
    m_pFrameLoopThread = NULL;
  }

  // Disconnect accelerators
  {
    // Escape disconnect
    g_pKeyAccelerators->Disconnect ( this );
    // Tabelator disconnect
    g_pKeyAccelerators->Disconnect ( this );
  }

  WriteWindowPositionToRegistry ( "Layout\\MapView\\Minimap", &m_wndMinimapFrame );
  // Delete the minimap frame
  m_wndMinimapFrame.Delete();

  WriteWindowPositionToRegistry ( "Layout\\MapView\\Selection", &m_wndSelection );
  // Delete the selection window
  m_wndSelection.Delete();

  WriteWindowPositionToRegistry ( "Layout\\MapView\\UnitToolbar", &m_wndUnitToolbar );
  // Delete the unit toolbar
  m_wndUnitToolbar.Delete();
  // Delete resources bar
  m_wndResourcesBar.Delete();
  // Delete message bar
  m_wndMessageBar.Delete();
  // Delete the status bar
  m_wndStatusBar.Delete();
  // Delete the mini toolbar
  m_wndToolbar.Delete();

  // Delete the virtual connection
  m_MapViewConnection.DeleteVirtualConnection();

  // Delete scroll helper windows
  {
    m_MSHTop.Delete(); m_MSHRight.Delete(); m_MSHLeft.Delete(); m_MSHBottom.Delete();
    m_MSHTL1.Delete(); m_MSHTL2.Delete();
    m_MSHTR1.Delete(); m_MSHTR2.Delete();
    m_MSHBL1.Delete(); m_MSHBL2.Delete();
    m_MSHBR1.Delete(); m_MSHBR2.Delete();

    m_NCur.Delete(); m_NECur.Delete(); m_ECur.Delete(); m_SECur.Delete();
    m_SCur.Delete(); m_SWCur.Delete(); m_WCur.Delete(); m_NWCur.Delete();
  }

  // Delete any watch unit frames
  {
    POSITION pos = m_listWatchUnitFrames.GetHeadPosition ();
    CWatchUnitFrame *pFrame;

    while ( pos != NULL )
    {
      pFrame = m_listWatchUnitFrames.GetNext ( pos );
      pFrame->Delete ();
      delete pFrame;
    }
    m_listWatchUnitFrames.RemoveAll ();

    // Delete the layout
    m_cWatchUnitFrameLayout.Delete ();
  }

  // delete the main viewport
  m_MainViewport.Delete();

  // Delete the selection
  m_UnitSelection.Delete();

  // stop unit interpolations
  m_UnitInterpolations.Delete();
  // delete the server unit info receiver
  m_ServerUnitInfoReceiver.Delete();

  // close the map
  g_pCivilization = NULL;
  g_pMap = NULL;
  m_Map.Delete();

  // Delete the video buffer
  m_VideoBuffer.Delete();

  // delete the window
  CWindow::Delete();

  // Delete the mini font
  m_MiniFont.Delete();

  // Delete toolbars config
  m_ToolbarsConfig.Delete();
}

void CMapView::Start()
{
  // if we're visible -> do it
  if(!CWindow::IsVisible()) return;

  // disable anny windows updating
  CWindow::EnableWindowsUpdating(FALSE);

  // start the frame loop thread
  m_eventPauseFrameLoop.ResetEvent();
  m_eventCanContinueFrameLoop.SetEvent();

  m_MainViewport.UpdateRect(NULL);
}

void CMapView::Stop()
{
  // if we're not visible -> do nothing
  if(!CWindow::IsVisible()) return;

  // stop the frame loop thread
  m_eventCanContinueFrameLoop.ResetEvent();
  m_eventPauseFrameLoop.SetEvent();
  WaitForSingleObject(m_eventFrameLoopPaused.m_hObject, INFINITE);

  // enable windows updating
  CWindow::EnableWindowsUpdating(TRUE);
}

// windows drawing routine
void CMapView::Draw(CDDrawSurface *pDDSurface, CRect *pBoundRect)
{
  // a little hack
  // we wan't draw anything here
  // cause all drawing is done before by the frameloop thread
}

// The frame loop thread
UINT CMapView::_FrameLoopThread(LPVOID pParam)
{
  CMapView *pMapView = (CMapView *)pParam;
  return pMapView->FrameLoopThread();
}

UINT CMapView::FrameLoopThread()
{
  HANDLE aLoopWaitHandles[1];
  aLoopWaitHandles[0] = g_pMainWindow->GetWaitableForActive();

  m_eventFrameLoopPaused.ResetEvent();

  try{
    while(1){
      // We must wait for the pause event to be set and also for the active event to be set
      // Cause our loop can be executed only if the window is active
      WaitForMultipleObjects(1, aLoopWaitHandles, TRUE, INFINITE);

      if(WaitForSingleObject(m_eventPauseFrameLoop.m_hObject, 0) == WAIT_OBJECT_0){
        m_eventFrameLoopPaused.SetEvent();
        WaitForSingleObject(m_eventCanContinueFrameLoop.m_hObject, INFINITE);
      }

      WaitForMultipleObjects(1, aLoopWaitHandles, TRUE, INFINITE);

      // check the exit flag
      if(m_eventExitFrameLoop.Lock(0)) break;

      if(!g_pMainWindow->IsActive()){
        continue;
      }

      CWindow::FreezeWindows();

      try{

        // Draw all updates to the video buffer (but carefully)
//        CWindow::UpdateNowBuffered(&m_VideoBuffer, &m_MainViewport);

        // Go through all viewports and do the job
        CViewport::DoFrameLoop();

        // Draw all updates to the video buffer (but carefully)
        CWindow::UpdateNowBuffered(&m_VideoBuffer, &m_MainViewport);

        // Now paste the video buffer to the screen
        g_pDDPrimarySurface->Paste(0, 0, &m_VideoBuffer);

      }
      catch(CDirectDrawException *e){
        e->Delete();
      }

      CWindow::UnfreezeWindows();
    }

    m_eventFrameLoopPaused.SetEvent();
    return 0;
  }
  catch(CException *){
    TRACE("Exception in the frame loop thread... exiting the thread.\n");
    m_eventFrameLoopPaused.SetEvent();
    return -1;
  }
}

void CMapView::OnMapScroll(int nXAmount, int nYAmount)
{
  DWORD dwXPos, dwYPos;
  m_MainViewport.GetPosition(dwXPos, dwYPos);

  dwXPos += nXAmount;
  dwYPos += nYAmount;

  m_MainViewport.SetPosition(dwXPos, dwYPos);
}


void CMapView::OnScriptEditor()
{
  InlayEvent(E_SCRIPTEDITOR, 0);
}


// Toggles the minimap window (show/hide)
void CMapView::OnToggleMinimap ()
{
  if ( m_wndMinimapFrame.IsVisible () )
  {
    m_wndMinimapFrame.HideWindow ();
  }
  else
  {
    m_wndMinimapFrame.ShowWindow ();
    m_wndMinimapFrame.Activate ();
  }
}

// Toggles the selection window (show/hide)
void CMapView::OnToggleSelection ()
{
  if ( m_wndSelection.IsVisible () )
  {
    m_wndSelection.HideWindow ();
  }
  else
  {
    m_wndSelection.ShowWindow ();
    m_wndSelection.Activate ();
  }
}

// Called when the unit selection changes
void CMapView::OnUnitSelectionChanged()
{
  // Determine the type of the selection

  if(m_UnitSelection.IsEmpty()){
    // Empty selection -> no selection toolbar
    m_wndUnitToolbar.SetToolbarInfo( m_ToolbarsConfig.GetNoSelectionToolbar () );
  }
  else if(m_UnitSelection.IsEnemyCivilization()){
    // Enemy selection
    m_wndUnitToolbar.SetToolbarInfo(m_ToolbarsConfig.GetEnemyToolbar());
  }
  else{
    // Our civilization
    // ? are they of the same type
    BOOL bSame = TRUE;
    DWORD dwUnitTypeID = 0;
    {
      CUnitSelection::SUnitNode *pNode;

      pNode = m_UnitSelection.GetFirstNode();
      while(pNode != NULL){
        if(dwUnitTypeID == 0) dwUnitTypeID = pNode->m_dwUnitTypeID;
        if(dwUnitTypeID != pNode->m_dwUnitTypeID){
          bSame = FALSE;
          break;
        }

        pNode = m_UnitSelection.GetNext(pNode);
      }
    }

    if(bSame){
      // Same unit type
      m_wndUnitToolbar.SetToolbarInfo(m_ToolbarsConfig.GetUnitToolbar(m_Map.GetGeneralUnitType(dwUnitTypeID)->GetName()));
    }
    else{
      // Different unit types
      m_wndUnitToolbar.SetToolbarInfo(m_ToolbarsConfig.GetDefaultToolbar());
    }
  }
}

// Called when the unit toolbar button is pressed
void CMapView::OnUnitToolbarCommand(CToolbarButtonInfo *pButton)
{
  ASSERT(pButton != NULL);

  RunFunction(pButton);
}

// Sets map view state
void CMapView::SetMapViewState(EMapViewState eState)
{
  switch(eState){
  case MapViewState_Normal:
    m_eMapViewState = eState;
    m_wndSelection.EnableAll();
    m_wndUnitToolbar.EnableAll();
    m_wndToolbar.EnableAll();
    {
      POSITION pos = m_listWatchUnitFrames.GetHeadPosition ();
      CWatchUnitFrame *pFrame;
      while ( pos != NULL )
      {
        pFrame = m_listWatchUnitFrames.GetNext ( pos );
        pFrame->EnableAll ();
      }
    }
    m_wndToolbar.SetState(CMapViewToolbar::State_None);
    m_MainViewport.SetViewportState(CInteractiveViewport::ViewportState_Normal);
    break;
  case MapViewState_Disabled:
    m_wndSelection.DisablePartial();
    m_wndUnitToolbar.DisablePartial();
    m_wndToolbar.DisablePartial();
    {
      POSITION pos = m_listWatchUnitFrames.GetHeadPosition ();
      CWatchUnitFrame *pFrame;
      while ( pos != NULL )
      {
        pFrame = m_listWatchUnitFrames.GetNext ( pos );
        pFrame->DisablePartial ();
      }
    }
    m_wndToolbar.SetState(CMapViewToolbar::State_HourGlass);
    m_MainViewport.SetViewportState(CInteractiveViewport::ViewportState_Disabled);
    m_eMapViewState = eState;
    break;
  case MapViewState_Asking:
    m_wndSelection.DisablePartial();
    m_wndUnitToolbar.DisablePartial();
    m_wndToolbar.DisablePartial();
    {
      POSITION pos = m_listWatchUnitFrames.GetHeadPosition ();
      CWatchUnitFrame *pFrame;
      while ( pos != NULL )
      {
        pFrame = m_listWatchUnitFrames.GetNext ( pos );
        pFrame->DisablePartial ();
      }
    }
    m_wndToolbar.SetState(CMapViewToolbar::State_Question);
    m_MainViewport.SetViewportState(CInteractiveViewport::ViewportState_Asking);
    m_eMapViewState = eState;
    break;
  }
}

// Runs function on the server
void CMapView::RunFunction(CToolbarButtonInfo *pButtonInfo, SMapViewRequest_RunFunctionParam *pAdditionalParam, BYTE *pAdditionalParamData)
{
  // If not in normal state -> something is little wrong
  // (For example we've disabled window, but there was already a message in a queue
  // for that window, so we will recieve it after the widnow is disabled)
  if ( m_eMapViewState != MapViewState_Normal ) return;

  // Send the request to the server
  {
    SMapViewRequest_RunFunction sRequest;

    m_UnitSelection.Lock();

    VERIFY(m_MapViewConnection.BeginSendCompoundBlock());

    DWORD dwRequest = MapViewRequest_RunFunction;

    VERIFY(m_MapViewConnection.SendBlock(&dwRequest, sizeof(dwRequest)));

    // Fill the header
    sRequest.m_dwFunctionNameLength = pButtonInfo->GetFunctionName().GetLength() + 1;
    sRequest.m_dwParamsCount = pButtonInfo->GetParamCount();
    if ( pAdditionalParam != NULL )
    {
      sRequest.m_dwParamsCount ++;
    }
    sRequest.m_dwSelectedUnitsCount = m_UnitSelection.GetCount();
    // send it to the network
    VERIFY(m_MapViewConnection.SendBlock(&sRequest, sizeof(sRequest)));

    // send the function name
    VERIFY(m_MapViewConnection.SendBlock((LPCSTR)(pButtonInfo->GetFunctionName()), sRequest.m_dwFunctionNameLength));

    // send unit IDs
    {
      CUnitSelection::SUnitNode *pNode;
      pNode = m_UnitSelection.GetFirstNode();
      while(pNode != NULL){
        VERIFY(m_MapViewConnection.SendBlock(&(pNode->m_dwUnitID), sizeof(DWORD)));
        pNode = m_UnitSelection.GetNext(pNode);
      }
    }

    // send params
    {
      CToolbarButtonInfo::SParameter *pParam;
      DWORD dwParam;
      SMapViewRequest_RunFunctionParam sParam;

      if ( pAdditionalParam != NULL )
      {
        // Send the param structure
        VERIFY ( m_MapViewConnection.SendBlock ( pAdditionalParam, sizeof ( *pAdditionalParam ) ) );

        if ( pAdditionalParam->m_eParamType == SMapViewRequest_RunFunctionParam::ParamType_String )
        {
          VERIFY ( m_MapViewConnection.SendBlock ( pAdditionalParamData, pAdditionalParam->m_dwStringLength ) );
        }
      }

      for(dwParam = 0; dwParam < pButtonInfo->GetParamCount(); dwParam++){
        pParam = pButtonInfo->GetParameter(dwParam);

        switch(pParam->m_eParamType){
        case CToolbarButtonInfo::ParamType_Integer:
          sParam.m_eParamType = SMapViewRequest_RunFunctionParam::ParamType_Integer;
          sParam.m_nValue = pParam->m_nValue;
          break;
        case CToolbarButtonInfo::ParamType_Real:
          sParam.m_eParamType = SMapViewRequest_RunFunctionParam::ParamType_Real;
          sParam.m_dbValue = pParam->m_dbValue;
          break;
        case CToolbarButtonInfo::ParamType_String:
          sParam.m_eParamType = SMapViewRequest_RunFunctionParam::ParamType_String;
          sParam.m_dwStringLength = strlen(pParam->m_lpszValue) + 1;
          break;
        case CToolbarButtonInfo::ParamType_Bool:
          sParam.m_eParamType = SMapViewRequest_RunFunctionParam::ParamType_Bool;
          sParam.m_nValue = pParam->m_bValue;
          break;
        }

        VERIFY(m_MapViewConnection.SendBlock(&sParam, sizeof(sParam)));

        if(sParam.m_eParamType == SMapViewRequest_RunFunctionParam::ParamType_String){
          VERIFY(m_MapViewConnection.SendBlock(pParam->m_lpszValue, sParam.m_dwStringLength));
        }
      }
    }

    VERIFY(m_MapViewConnection.EndSendCompoundBlock());

    m_UnitSelection.Unlock();
  }

  // Switch our state to disabled - we're waiting for server to reply
  SetMapViewState(MapViewState_Disabled);
}

void CMapView::KillFunction()
{
  // Kill any running function on server
  VERIFY(m_MapViewConnection.BeginSendCompoundBlock());
  DWORD dwRequest = MapViewRequest_KillFunction;
  VERIFY(m_MapViewConnection.SendBlock(&dwRequest, sizeof(dwRequest)));
  VERIFY(m_MapViewConnection.EndSendCompoundBlock());

  // Set new icon in our state bar
  m_wndToolbar.SetState(CMapViewToolbar::State_HourGlassWarning);

  // And wait for the server to reply
  m_bNoServerAction = TRUE;
}

void CMapView::OnMapViewConnectionPacketAvailable()
{
  DWORD dwAnswer, dwSize;

  // Recieve the answer ID
  VERIFY(m_MapViewConnection.BeginReceiveCompoundBlock());
  dwSize = sizeof(dwAnswer);
  VERIFY(m_MapViewConnection.ReceiveBlock(&dwAnswer, dwSize));
  ASSERT(dwSize == sizeof(dwAnswer));

  try
  {

  switch(dwAnswer){
  case MapViewAnswer_RunFunctionError:
    {
      m_bNoServerAction = FALSE;
      // Ooops - some error running the function on the server
      // so just leave it and reset our state
      SetMapViewState(MapViewState_Normal);

      CString strMessage;
      strMessage.LoadString(IDS_MAPVIEW_RUNFUNCTIONERROR);
      m_wndMessageBar.Message(strMessage, CMapViewMessageBar::MsgType_Error);
    }
    break;

  case MapViewAnswer_FunctionActionDone:
    
    // The function has done all its work with client
    m_bNoServerAction = FALSE;
    SetMapViewQuestion(MapViewQuestion_None);
    SetMapViewState(MapViewState_Normal);
    break;

  case MapViewAnswer_Message:
    // Display some message
    {
      SMapViewAnswer_Message msg;

      // Receive the message header
      dwSize = sizeof(msg);
      VERIFY(m_MapViewConnection.ReceiveBlock(&msg, dwSize));
      ASSERT(dwSize == sizeof(msg));

      // Receive the message text
      CString strText;
      dwSize = msg.m_dwTextLength;
      VERIFY(m_MapViewConnection.ReceiveBlock(strText.GetBuffer(dwSize), dwSize));
      ASSERT(dwSize == msg.m_dwTextLength);
      strText.ReleaseBuffer();

      // Translate the type
      CMapViewMessageBar::EMessageType eType;
      switch(msg.m_eMessageType){
      case SMapViewAnswer_Message::MsgType_User:
        eType = CMapViewMessageBar::MsgType_User;
        break;
      case SMapViewAnswer_Message::MsgType_Info:
        eType = CMapViewMessageBar::MsgType_Info;
        break;
      case SMapViewAnswer_Message::MsgType_Warning:
        eType = CMapViewMessageBar::MsgType_Warning;
        break;
      case SMapViewAnswer_Message::MsgType_Error:
        eType = CMapViewMessageBar::MsgType_Error;
        break;
      case SMapViewAnswer_Message::MsgType_Admin:
        eType = CMapViewMessageBar::MsgType_Admin;
        break;
      default:
        eType = CMapViewMessageBar::MsgType_Info;
        break;
      }

      // now display the message
      if(msg.m_dwUnitID != 0x0FFFFFFFF){
        m_wndMessageBar.Message(strText, eType, msg.m_dwUnitID);
      }
      else if((msg.m_dwXPos != 0x0F0000000) && (msg.m_dwYPos != 0x0F0000000)){
        m_wndMessageBar.Message(strText, eType, msg.m_dwXPos, msg.m_dwYPos);
      }
      else{
        m_wndMessageBar.Message(strText, eType);
      }
    }
    break;

  case MapViewAnswer_AskForPosition:
    {
      ASSERT(m_eMapViewQuestion == MapViewQuestion_None);

      SMapViewAnswer_AskForPosition header;
      dwSize = sizeof(header);
      VERIFY(m_MapViewConnection.ReceiveBlock(&header, dwSize));
      ASSERT(dwSize == sizeof(header));

      CString strQuestion;
      dwSize = header.m_dwQuestionLength;
      VERIFY(m_MapViewConnection.ReceiveBlock(strQuestion.GetBuffer(header.m_dwQuestionLength), dwSize));
      ASSERT(dwSize == header.m_dwQuestionLength);
      strQuestion.ReleaseBuffer();

      // Set the text to status bar
      m_wndStatusBar.Show(strQuestion);
      // Set our state to asking
      SetMapViewState(MapViewState_Asking);
      // Set the question
      SetMapViewQuestion(MapViewQuestion_Position);
    }
    break;

  case MapViewAnswer_AskForUnit:
    {
      ASSERT(m_eMapViewQuestion == MapViewQuestion_None);

      SMapViewAnswer_AskForUnit header;
      dwSize = sizeof(header);
      VERIFY(m_MapViewConnection.ReceiveBlock(&header, dwSize));
      ASSERT(dwSize == sizeof(header));

      CString strQuestion;
      dwSize = header.m_dwQuestionLength;
      VERIFY(m_MapViewConnection.ReceiveBlock(strQuestion.GetBuffer(header.m_dwQuestionLength), dwSize));
      ASSERT(dwSize == header.m_dwQuestionLength);
      strQuestion.ReleaseBuffer();

      // Set the text to status bar
      m_wndStatusBar.Show(strQuestion);
      // Set our state to asking
      SetMapViewState(MapViewState_Asking);
      // Set the question
      switch(header.m_eUnitType){
      case SMapViewAnswer_AskForUnit::UnitType_My:
        SetMapViewQuestion(MapViewQuestion_MyUnit);
        break;
      case SMapViewAnswer_AskForUnit::UnitType_Enemy:
        SetMapViewQuestion(MapViewQuestion_EnemyUnit);
        break;
      case SMapViewAnswer_AskForUnit::UnitType_Any:
        SetMapViewQuestion(MapViewQuestion_AnyUnit);
        break;
      }
    }
    break;

  case MapViewAnswer_ToolbarFileUpdate:
    {
      CString strToolbarsDirectory = g_strStartDirectory + "\\Toolbars\\";

      // Read the file header
      SMapViewAnswer_ToolbarFileUpdate sHeader;
      dwSize = sizeof ( sHeader );
      VERIFY ( m_MapViewConnection.ReceiveBlock ( &sHeader, dwSize ) );
      ASSERT ( dwSize == sizeof ( sHeader ) );

      // Receive the path
      CString strPath;
      dwSize = sHeader.m_dwPathLength;
      VERIFY ( m_MapViewConnection.ReceiveBlock ( strPath.GetBuffer ( dwSize ), dwSize ) );
      ASSERT ( dwSize == sHeader.m_dwPathLength );
      strPath.ReleaseBuffer ();

      // What type is it
      if ( sHeader.m_dwFlags == 1 )
      {
        // Directory -> create it
        // no more data will arrive
        try
        {
          CDataArchive::GetRootArchive ()->MakeDirectory ( strToolbarsDirectory + strPath );
        }
        catch ( CException * e)
        {
          e->Delete ();
        }
      }
      else
      {
        // File -> receive it
        BYTE *pBuffer = new BYTE [ sHeader.m_dwFileLength ];
        dwSize = sHeader.m_dwFileLength;
        VERIFY ( m_MapViewConnection.ReceiveBlock ( pBuffer, dwSize ) );
        ASSERT ( dwSize == sHeader.m_dwFileLength );

        // Create the file
        CArchiveFile cFile;
        cFile = CDataArchive::GetRootArchive ()->CreateFile ( strToolbarsDirectory + strPath,
          CArchiveFile::modeWrite | CArchiveFile::modeCreate | CArchiveFile::modeUncompressed );
        // And write the data
        cFile.Write ( pBuffer, sHeader.m_dwFileLength );

        delete pBuffer;
      }
    }
    break;

  case MapViewAnswer_ToolbarUpdateComplete:
    {
      // no more data

      // Just end the toolbar update
      m_eventToolbarsUpdateDone.SetEvent ();
    }
    break;

  default:
    // Shit - unknown packet arrived
    TRACE("MapView - MapView connection - unknown packet arrived\n");
    ASSERT(FALSE);
    break;
  }

  }
  catch ( CException * )
  {
    VERIFY(m_MapViewConnection.EndReceiveCompoundBlock());
    throw;
  }

  VERIFY(m_MapViewConnection.EndReceiveCompoundBlock());
}

void CMapView::OnMapViewConnectionNetworkError()
{
  m_bNoServerAction = FALSE;
  // Some network error -> restore the mapview state to normal
  SetMapViewState(MapViewState_Normal);
}

BOOL CMapView::OnKeyAccelerator(int nChar, DWORD dwFlags)
{
  switch(nChar){
  
  // The escape was pressed
  case VK_ESCAPE:
    // If in normal state -> nothing
    if(m_eMapViewState == MapViewState_Normal) return FALSE;

    // If in some action -> cancel it
    CancelAction();

    break;

  // The tabelator was pressed
  case VK_TAB:
    {
      bool bLifeBarMarking;
      // Read it and toggle it
      bLifeBarMarking = !m_MainViewport.GetLifeBarMarking ();
      // Toggle the life bar marking in the main viewport
      m_MainViewport.SetLifeBarMarking ( bLifeBarMarking );
      // Also go through all watched unit view and set the flag
      CWatchUnitFrame * pFrame; POSITION pos = m_listWatchUnitFrames.GetHeadPosition ();
      while ( pos != NULL )
      {
        pFrame = m_listWatchUnitFrames.GetNext ( pos );
        pFrame->GetViewport ()->SetLifeBarMarking ( bLifeBarMarking );
      }
    }

    break;
  }

  return TRUE;
}

void CMapView::SetMapViewQuestion ( EMapViewQuestion eQuestion )
{
  switch(eQuestion)
  {
  case MapViewQuestion_None:
    m_MainViewport.SetViewportQuestion ( CInteractiveViewport::ViewportQuestion_None);
    m_wndStatusBar.Hide();
    m_eMapViewQuestion = eQuestion;
    break;
  case MapViewQuestion_Position:
    m_eMapViewQuestion = eQuestion;
    m_MainViewport.SetViewportQuestion ( CInteractiveViewport::ViewportQuestion_Position );
    break;
  case MapViewQuestion_MyUnit:
    m_eMapViewQuestion = eQuestion;
    m_MainViewport.SetViewportQuestion ( CInteractiveViewport::ViewportQuestion_MyUnit );
    break;
  case MapViewQuestion_EnemyUnit:
    m_eMapViewQuestion = eQuestion;
    m_MainViewport.SetViewportQuestion ( CInteractiveViewport::ViewportQuestion_EnemyUnit );
    break;
  case MapViewQuestion_AnyUnit:
    m_eMapViewQuestion = eQuestion;
    m_MainViewport.SetViewportQuestion ( CInteractiveViewport::ViewportQuestion_AnyUnit );
    break;
  }
}


void CMapView::CancelAction()
{
  switch(m_eMapViewState){
  case MapViewState_Normal:
    return;
  case MapViewState_Disabled:
    KillFunction();
    break;
  case MapViewState_Asking:
    switch(m_eMapViewQuestion){
    case MapViewQuestion_None:
      break;
    case MapViewQuestion_Position:
      {
        VERIFY(m_MapViewConnection.BeginSendCompoundBlock());
        DWORD dwRequest = MapViewRequest_AskForPositionCanceled;
        VERIFY(m_MapViewConnection.SendBlock(&dwRequest, sizeof(dwRequest)));
        VERIFY(m_MapViewConnection.EndSendCompoundBlock());
      }
      break;
    case MapViewQuestion_AnyUnit:
    case MapViewQuestion_MyUnit:
    case MapViewQuestion_EnemyUnit:
      {
        VERIFY(m_MapViewConnection.BeginSendCompoundBlock());
        DWORD dwRequest = MapViewRequest_AskForUnitCanceled;
        VERIFY(m_MapViewConnection.SendBlock(&dwRequest, sizeof(dwRequest)));
        VERIFY(m_MapViewConnection.EndSendCompoundBlock());
      }
      break;
    }
    SetMapViewQuestion(MapViewQuestion_None);
    if(m_bNoServerAction)
      SetMapViewState(MapViewState_Normal);
    else
      SetMapViewState(MapViewState_Disabled);
    break;
  }
}

void CMapView::OnViewportCancel()
{
  CancelAction();
}

void CMapView::OnViewportPosition( DWORD dwX, DWORD dwY )
{
  ASSERT(m_eMapViewState == MapViewState_Asking);
  if(m_eMapViewState != MapViewState_Asking) return;
  ASSERT(m_eMapViewQuestion == MapViewQuestion_Position);
  if(m_eMapViewQuestion != MapViewQuestion_Position) return;

  // answer the position to the server
  VERIFY(m_MapViewConnection.BeginSendCompoundBlock());

  DWORD dwRequest = MapViewRequest_AskForPosition;
  VERIFY(m_MapViewConnection.SendBlock(&dwRequest, sizeof(dwRequest)));

  SMapViewRequest_AskForPosition header;
  header.m_dwPositionX = dwX;
  header.m_dwPositionY = dwY;
  VERIFY(m_MapViewConnection.SendBlock(&header, sizeof(header)));

  VERIFY(m_MapViewConnection.EndSendCompoundBlock());

  // End our question state
  SetMapViewQuestion(MapViewQuestion_None);
  if(m_bNoServerAction)
    SetMapViewState(MapViewState_Normal);
  else
    SetMapViewState(MapViewState_Disabled);
}

void CMapView::OnViewportUnit( CCUnit *pUnit )
{
  ASSERT(pUnit != NULL);
  ASSERT(m_eMapViewState == MapViewState_Asking);
  if(m_eMapViewState != MapViewState_Asking)
  {
    pUnit->Release ();
	  return;
  }

  ASSERT((m_eMapViewQuestion == MapViewQuestion_MyUnit) || (m_eMapViewQuestion == MapViewQuestion_EnemyUnit) ||
    (m_eMapViewQuestion == MapViewQuestion_AnyUnit));
  if(!((m_eMapViewQuestion == MapViewQuestion_MyUnit) || (m_eMapViewQuestion == MapViewQuestion_EnemyUnit) ||
    (m_eMapViewQuestion == MapViewQuestion_AnyUnit)))
  {
    pUnit->Release ();
    return;
  }

  // answer the position to the server
  VERIFY(m_MapViewConnection.BeginSendCompoundBlock());

  DWORD dwRequest = MapViewRequest_AskForUnit;
  VERIFY(m_MapViewConnection.SendBlock(&dwRequest, sizeof(dwRequest)));

  SMapViewRequest_AskForUnit header;
  header.m_dwUnitID = pUnit->GetID();
  VERIFY(m_MapViewConnection.SendBlock(&header, sizeof(header)));

  VERIFY(m_MapViewConnection.EndSendCompoundBlock());

  // End our question state
  SetMapViewQuestion(MapViewQuestion_None);
  if(m_bNoServerAction)
    SetMapViewState(MapViewState_Normal);
  else
    SetMapViewState(MapViewState_Disabled);

  pUnit->Release();
}

// If in normal state
// reaction on viewport unit watch selection
// !!!! the unit has addref called on it -> call Release
void CMapView::OnViewportWatchUnit ( CCUnit *pUnit )
{
  // Try to find the unit in allready watched units
  CWatchUnitFrame *pWatchUnitFrame;
  {
    POSITION pos = m_listWatchUnitFrames.GetHeadPosition ();
    while ( pos != NULL )
    {
      pWatchUnitFrame = m_listWatchUnitFrames.GetAt ( pos );
      if ( pWatchUnitFrame->GetWatchedUnit () == pUnit )
      {
        // We've found it
        break;
      }

      m_listWatchUnitFrames.GetNext ( pos );
    }

    if ( pos == NULL )
    {
      pWatchUnitFrame = NULL;
    }
  }

  // If there already is some watch frame for the unit -> activate it
  if ( pWatchUnitFrame != NULL )
  {
    ASSERT_VALID ( pWatchUnitFrame );

    pWatchUnitFrame->Activate ();

    // And that's all
  }
  else
  {
    // No Watch frame for the unit -> create a new one
    pWatchUnitFrame = new CWatchUnitFrame ();

    CPoint ptPosition ( 50, 50 );
    pWatchUnitFrame->Create ( ptPosition, pUnit, &m_cWatchUnitFrameLayout, &m_ServerUnitInfoReceiver,
      &m_UnitSelection, &m_cGraphicsArchive, &m_MainViewport );
	pWatchUnitFrame->SetBuffering ();

    // Add it to our list
    m_listWatchUnitFrames.AddHead ( pWatchUnitFrame );

    // Connect us to the window
    pWatchUnitFrame->Connect ( this, IDC_WatchUnitFrame );

    // Activate it
    pWatchUnitFrame->Activate ();
  }

  // Now release the unit
  pUnit->Release ();
}

// The watch unit frame want to be closed
void CMapView::OnWatchUnitFrameClose ( CWatchUnitFrame * pWatchUnitFrame )
{
  ASSERT_VALID ( pWatchUnitFrame );

  // Close the window
  pWatchUnitFrame->Delete ();

  // Remove it from our list
  POSITION pos = m_listWatchUnitFrames.Find ( pWatchUnitFrame );
  ASSERT ( pos != NULL );
  m_listWatchUnitFrames.RemoveAt ( pos );

  delete pWatchUnitFrame;
}

void CMapView::SetViewportPosition(DWORD dwXPos, DWORD dwYPos)
{
  m_MainViewport.SetPosition((dwXPos - (m_MainViewport.GetSize().cx / 2)) * MAPCELL_WIDTH,
    (dwYPos - (m_MainViewport.GetSize().cy / 2)) * MAPCELL_HEIGHT);
}


// If in normal state
// reaction on viewport right click position
void CMapView::OnViewportRightClickPosition ( DWORD dwX, DWORD dwY )
{
  ASSERT_VALID ( this );

  // From current toolbar, get the right click position button info
  if ( m_wndUnitToolbar.GetToolbarInfo () == NULL ) return;
  CToolbarButtonInfo * pButtonInfo = m_wndUnitToolbar.GetToolbarInfo ()->GetRightClickPositionButton ();
  if ( pButtonInfo == NULL ) return;

  // Prepare the additional parameter
  SMapViewRequest_RunFunctionParam sParam;
  sParam.m_eParamType = SMapViewRequest_RunFunctionParam::ParamType_Position;
  sParam.m_sPosition.m_dwX = dwX;
  sParam.m_sPosition.m_dwY = dwY;
  sParam.m_sPosition.m_dwZ = 0; // Nonsense value, cause we don't know it

  // Run the function for that button
  RunFunction ( pButtonInfo, &sParam, NULL );
}

// If in normal state
// reaction on viewport right click unit
void CMapView::OnViewportRightClickUnit ( CCUnit *pUnit )
{
  ASSERT_VALID ( this );

  if ( pUnit->GetUnitType ()->GetCivilization () == g_pCivilization )
  {
    // My unit was clicked

    // From current toolbar, get the right click my unit button info
    if ( m_wndUnitToolbar.GetToolbarInfo () == NULL )
    {
      pUnit->Release ();
      return;
    }
    CToolbarButtonInfo * pButtonInfo = m_wndUnitToolbar.GetToolbarInfo ()->GetRightClickMyUnitButton ();
    if ( pButtonInfo == NULL )
    {
      pUnit->Release ();
      return;
    }

    // Prepare the additional parameter
    SMapViewRequest_RunFunctionParam sParam;
    sParam.m_eParamType = SMapViewRequest_RunFunctionParam::ParamType_Unit;
    sParam.m_dwSUnitID = pUnit->GetID ();

    // Run the function
    RunFunction ( pButtonInfo, &sParam, NULL );
  }
  else
  {
    // Enemy unit was clicked

    // From current toolbar, get the right click my unit button info
    if ( m_wndUnitToolbar.GetToolbarInfo () == NULL )
    {
      pUnit->Release ();
      return;
    }
    CToolbarButtonInfo * pButtonInfo = m_wndUnitToolbar.GetToolbarInfo ()->GetRightClickEnemyUnitButton ();
    if ( pButtonInfo == NULL )
    {
      pUnit->Release ();
      return;
    }

    // Prepare the additional parameter
    SMapViewRequest_RunFunctionParam sParam;
    sParam.m_eParamType = SMapViewRequest_RunFunctionParam::ParamType_Unit;
    sParam.m_dwSUnitID = pUnit->GetID ();

    // Run the function
    RunFunction ( pButtonInfo, &sParam, NULL );
  }
  pUnit->Release ();
}

// If in normal state
// reaction on viewport shift right click position
void CMapView::OnViewportShiftRightClickPosition ( DWORD dwX, DWORD dwY )
{
  ASSERT_VALID ( this );

  // From current toolbar, get the right click position button info
  if ( m_wndUnitToolbar.GetToolbarInfo () == NULL ) return;
  CToolbarButtonInfo * pButtonInfo = m_wndUnitToolbar.GetToolbarInfo ()->GetShiftRightClickPositionButton ();
  if ( pButtonInfo == NULL ) return;

  // Prepare the additional parameter
  SMapViewRequest_RunFunctionParam sParam;
  sParam.m_eParamType = SMapViewRequest_RunFunctionParam::ParamType_Position;
  sParam.m_sPosition.m_dwX = dwX;
  sParam.m_sPosition.m_dwY = dwY;
  sParam.m_sPosition.m_dwZ = 0; // Nonsense value, cause we don't know it

  // Run the function for that button
  RunFunction ( pButtonInfo, &sParam, NULL );
}

// If in normal state
// reaction on viewport shift right click unit
void CMapView::OnViewportShiftRightClickUnit ( CCUnit *pUnit )
{
  ASSERT_VALID ( this );

  if ( pUnit->GetUnitType ()->GetCivilization () == g_pCivilization )
  {
    // My unit was clicked

    // From current toolbar, get the right click my unit button info
    if ( m_wndUnitToolbar.GetToolbarInfo () == NULL )
    {
      pUnit->Release ();
      return;
    }

    CToolbarButtonInfo * pButtonInfo = m_wndUnitToolbar.GetToolbarInfo ()->GetShiftRightClickMyUnitButton ();
    if ( pButtonInfo == NULL )
    {
      pUnit->Release ();
      return;
    }

    // Prepare the additional parameter
    SMapViewRequest_RunFunctionParam sParam;
    sParam.m_eParamType = SMapViewRequest_RunFunctionParam::ParamType_Unit;
    sParam.m_dwSUnitID = pUnit->GetID ();

    // Run the function
    RunFunction ( pButtonInfo, &sParam, NULL );
  }
  else
  {
    // Enemy unit was clicked

    // From current toolbar, get the right click my unit button info
    if ( m_wndUnitToolbar.GetToolbarInfo () == NULL )
    {
      pUnit->Release ();
      return;
    }

    CToolbarButtonInfo * pButtonInfo = m_wndUnitToolbar.GetToolbarInfo ()->GetShiftRightClickEnemyUnitButton ();
    if ( pButtonInfo == NULL )
    {
      pUnit->Release ();
      return;
    }

    // Prepare the additional parameter
    SMapViewRequest_RunFunctionParam sParam;
    sParam.m_eParamType = SMapViewRequest_RunFunctionParam::ParamType_Unit;
    sParam.m_dwSUnitID = pUnit->GetID ();

    // Run the function
    RunFunction ( pButtonInfo, &sParam, NULL );
  }
  pUnit->Release ();
}
