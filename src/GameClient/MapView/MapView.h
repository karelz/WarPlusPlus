// MapView.h: interface for the CMapView class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MAPVIEW_H__102F2AD3_10B0_11D4_849D_004F4E0004AA__INCLUDED_)
#define AFX_MAPVIEW_H__102F2AD3_10B0_11D4_849D_004F4E0004AA__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "..\DataObjects\CMap.h"
#include "InteractiveViewport.h"
#include "UnitInterpolations.h"
#include "..\ToolbarsSrc\ToolbarsConfig.h"
#include "MapScrollHelper.h"
#include "MinimapFrame.h"
#include "MapViewToolbar.h"
#include "MapViewStatusBar.h"
#include "MapViewMessageBar.h"
#include "ResourcesBar.h"
#include "UnitSelection.h"
#include "SelectionWindow.h"
#include "..\ToolbarsSrc\UnitToolbar.h"

#include "WatchUnitFrame.h"

#include "..\ServerCommunication\CServerUnitInfoReceiver.h"

#include "Common\ServerClient\MapViewConnection.h"

class CUnitHierarchy;

// the underlay main window for the map view
class CMapView : public CWindow  
{
  DECLARE_DYNAMIC(CMapView);
  DECLARE_OBSERVER_MAP(CMapView);

public:

  // start the map view (show it)
  void Start();
  // stop the map view (hide it)
  void Stop();

  // windows drawing
  virtual void Draw(CDDrawSurface *pDDSurface, CRect *pBoundRect);

  // creation
  void Create(CWindow *pParent, CString &strMapPath, CDataArchive *pGraphicsArchive,
    DWORD dwCivilizationID, DWORD dwTimeslice, DWORD dwTimesliceInterval,
    CUnitHierarchy *pUnitHierarchy);
  virtual void Delete();

  // Sets viewport position in mapcells
  // This position will be in the center of the viewport
  void SetViewportPosition(DWORD dwXPos, DWORD dwYPos);

  // constructor & destructor
	CMapView();
	virtual ~CMapView();

  // Events sent by this object
  enum{
    E_SCRIPTEDITOR = 0x01000,
  };

  // Runs global function on the server
  // specified by the toolbar button info
  // If the additional param is specified (!=NULL) then this param is passed
  // as the first custom param (right before any aprams from the button info)
  void RunFunction(CToolbarButtonInfo *pButtonInfo, SMapViewRequest_RunFunctionParam *pAditionalParam = NULL, BYTE * pAdditionalParamData = NULL);
  // Kills currently running function on the server
  void KillFunction();

  // debug functions
#ifdef _DEBUG
  virtual void AssertValid() const;
  virtual void Dump(CDumpContext &dc) const;
#endif

protected:
  // Reaction on the toolbar buttons
  // ScriptEditor
  void OnScriptEditor();

  // Reaction on the Minimap events
  void OnToggleMinimap();

  // Reaction on the Selection events
  void OnToggleSelection();

  // Reaction on the events from unit selection
  void OnUnitSelectionChanged();

  // Reaction on unit toolbar command (button was pressed)
  void OnUnitToolbarCommand(CToolbarButtonInfo *pButtonInfo);

  // Reaction on new packet arriving on our connection
  void OnMapViewConnectionPacketAvailable();
  // Reaction on network error on our connection
  void OnMapViewConnectionNetworkError();

  // If in asking state
  // reaction on viewports cancel event (right click)
  void OnViewportCancel();
  // If in AskForPosition state
  // reaction on viewport position selection
  void OnViewportPosition(DWORD dwX, DWORD dwY);
  // If in AskFor unit state
  // reaction on viewport unit selection
  // !!!! the unit has addref called on it -> call Release
  void OnViewportUnit(CCUnit *pUnit);
  // If in normal state
  // reaction on viewport unit watch selection
  // !!!! the unit has addref called on it -> call Release
  void OnViewportWatchUnit ( CCUnit *pUnit );
  // If in normal state
  // reaction on viewport right click position
  void OnViewportRightClickPosition ( DWORD dwX, DWORD dwY );
  // If in normal state
  // reaction on viewport right click unit
  void OnViewportRightClickUnit ( CCUnit *pUnit );
  // If in normal state
  // reaction on viewport ShiftRight click position
  void OnViewportShiftRightClickPosition ( DWORD dwX, DWORD dwY );
  // If in normal state
  // reaction on viewport ShiftRight click unit
  void OnViewportShiftRightClickUnit ( CCUnit *pUnit );


  // The watch unit frame want to be closed
  void OnWatchUnitFrameClose ( CWatchUnitFrame * pWatchUnitFrame );

  // reaction on the key accelerators
  BOOL OnKeyAccelerator(int nChar, DWORD dwFlags);

private:
  // the MAP object
  CCMap m_Map;

//-------------------------------------------------------------------------------
// Basic informations
  // The civilization we're logged on
  CCCivilization *m_pCivilization;

  // Graphics archive to load graphics from
  CDataArchive m_cGraphicsArchive;

//-------------------------------------------------------------------------------
// Network (or server) things
  // server info receiver object (to obtain data from server and all notifications)
  CCServerUnitInfoReceiver m_ServerUnitInfoReceiver;

//-------------------------------------------------------------------------------
// Interpolations
  CUnitInterpolations m_UnitInterpolations;

//-------------------------------------------------------------------------------
// Main fullscreen viewport
  CInteractiveViewport m_MainViewport;

  enum{ IDC_VIEWPORT = 0x0104, };

  // Scroll helper windows
  CMapScrollHelper m_MSHTop, m_MSHLeft, m_MSHBottom, m_MSHRight;
  CMapScrollHelper m_MSHTL1, m_MSHTL2;
  CMapScrollHelper m_MSHTR1, m_MSHTR2;
  CMapScrollHelper m_MSHBL1, m_MSHBL2;
  CMapScrollHelper m_MSHBR1, m_MSHBR2;

  CCursor m_NCur, m_NECur, m_ECur, m_SECur, m_SCur, m_SWCur, m_WCur, m_NWCur;

  enum{
    IDC_MSHTop = 0x0300,
    IDC_MSHLeft = 0x0301,
    IDC_MSHRight = 0x0302,
    IDC_MSHBottom = 0x0303,
    IDC_MSHTopLeft = 0x0304,
    IDC_MSHTopRight = 0x0305,
    IDC_MSHBottomLeft = 0x0306,
    IDC_MSHBottomRight = 0x0307,
  };

  void OnMapScroll(int nXAmount, int nYAmount);

//-------------------------------------------------------------------------------
// Frame loop
  // The frame loop thread
  CWinThread *m_pFrameLoopThread;
  static UINT _FrameLoopThread(LPVOID pParam);
  UINT FrameLoopThread();

  // pause event for the frame loop thread
  CEvent m_eventPauseFrameLoop;
  CEvent m_eventFrameLoopPaused;
  CEvent m_eventCanContinueFrameLoop;
  // If the event is set -> exit the frame loop thread
  CEvent m_eventExitFrameLoop;

//-------------------------------------------------------------------------------
// Drawing
  // Helper surface for fullscreen map drawing
  // We want this surface to be in the video memory
  // If it's not in the video memory (well) we can stand it but it will be slower
  CScratchSurface m_VideoBuffer;

//-------------------------------------------------------------------------------
// Unit Toolbars
  // The configuration of toolbars
  CToolbarsConfig m_ToolbarsConfig;

  CFrameWindow m_TestWindow;

  // Event which is set when the toolbars update is done
  CEvent m_eventToolbarsUpdateDone;

//-------------------------------------------------------------------------------
// Toolbar
  // The standard mini toolbar (at bottom right)
  CMapViewToolbar m_wndToolbar;
  // The resource bar (at top)
  CResourcesBar m_wndResourcesBar;

  // Mini font for all small pieces
  CGraphicFont m_MiniFont;

//-------------------------------------------------------------------------------
// StatusBar
  // The status bar window
  CMapViewStatusBar m_wndStatusBar;

//-------------------------------------------------------------------------------
// MessageBar
  // The message bar window
  CMapViewMessageBar m_wndMessageBar;

  enum{
    IDC_MESSAGEBAR = 0x0105,
  };

//-------------------------------------------------------------------------------
// Minimap
  // The minimap frame window
  CMinimapFrame m_wndMinimapFrame;

  enum{
    IDC_MINIMAP = 0x0100,
  };

//-------------------------------------------------------------------------------
// Selection
  // The selection object
  CUnitSelection m_UnitSelection;
  // The window showing the selection
  CSelectionWindow m_wndSelection;

  enum{
    IDC_SELECTION = 0x0101,
    ID_UnitSelection = 0x0102, 
  };

//-------------------------------------------------------------------------------
// Watched units
  // List of watched units frames
  CTypedPtrList < CPtrList, CWatchUnitFrame * > m_listWatchUnitFrames;
  // Layout of the watch frame
  CCaptionWindowLayout m_cWatchUnitFrameLayout;

  enum{
    IDC_WatchUnitFrame = 0x0200,
  };

//-------------------------------------------------------------------------------
// Unit toolbar
  // The unit toolbar
  CUnitToolbar m_wndUnitToolbar;

  enum{
    IDC_UNITTOOLBAR = 0x0103,
  };

//-------------------------------------------------------------------------------
// MapView connection
  // The virtual connection
  CVirtualConnection m_MapViewConnection;

  enum{
    ID_MapViewConnection = 0x02000,
  };

//-------------------------------------------------------------------------------
// States

  // If the state is normal - it means, that no server interactive action is in progress
  // It means, that all server request to perform interactive action will end with error
  // If the state is disabled - we've run some action on the server and now we're
  //  waiting for server to do some interactive action or just close this
  //  server action call
  // If the state is asking - some server interactive action is in progress

  typedef enum{
    MapViewState_Normal = 0,   // Normal state
    MapViewState_Disabled = 1, // Disabled - some server action is running
    MapViewState_Asking = 2,   // Asking - server question in progress
  } EMapViewState;

  // State of the mapview
  EMapViewState m_eMapViewState;
  // If TRUE -> no server interactive action is allowed -> error
  BOOL m_bNoServerAction;
  // Sets the mapview state
  void SetMapViewState(EMapViewState eState);

  typedef enum{
    MapViewQuestion_None = 0,
    MapViewQuestion_Position = 1, // Ask for the position
    MapViewQuestion_AnyUnit = 2,  // Ask for any unit
    MapViewQuestion_EnemyUnit = 3, // Ask for enemy unit
    MapViewQuestion_MyUnit = 4,   // Ask for my unit
  } EMapViewQuestion;

  // Current question
  EMapViewQuestion m_eMapViewQuestion;
  // Sets question
  void SetMapViewQuestion(EMapViewQuestion eQuestion);

  // Cancel any server action
  // Called if right clicked or VK_ESCAPE
  void CancelAction();
};

#endif // !defined(AFX_MAPVIEW_H__102F2AD3_10B0_11D4_849D_004F4E0004AA__INCLUDED_)
