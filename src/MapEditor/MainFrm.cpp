// MainFrm.cpp : implementation of the CMainFrame class
//

#include "stdafx.h"
#include "MapEditor.h"

#include "MainFrm.h"
#include "MapEditorDoc.h"

#include "Controls\FinishDlg.h"

#include "FindPathGraphCreation\FindPathGraphCreation.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define ID_MAPEXTOOLBAR 20100
#define ID_SELECTEDMAPEX 20101
#define ID_DRAWINGMAPEX 20102
#define ID_MINIMAPBAR 20103
#define ID_UNITTOOLBAR 20104

#define MINIMAP_DARKMULTIPLY 3
#define MINIMAP_DARKDIVISION 4

/////////////////////////////////////////////////////////////////////////////
// CMainFrame

IMPLEMENT_DYNCREATE(CMainFrame, CFrameWnd)

BEGIN_MESSAGE_MAP(CMainFrame, CFrameWnd)
	//{{AFX_MSG_MAP(CMainFrame)
	ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_UPDATE_COMMAND_UI(ID_VIEW_MINIMAP, OnUpdateViewMinimap)
	ON_COMMAND(ID_VIEW_MINIMAP, OnViewMinimap)
	ON_WM_SHOWWINDOW()
	ON_UPDATE_COMMAND_UI(ID_VIEW_MAPEXBAR, OnUpdateViewMapexbar)
	ON_COMMAND(ID_VIEW_MAPEXBAR, OnViewMapexbar)
  ON_CBN_SELENDOK(ID_MAPEX_ZPOS, OnSelEndMapexZPos)
	ON_COMMAND(IDOK, OnOk)
	ON_COMMAND(ID_FILE_REBUILDLIBRARIES, OnFileRebuildLibraries)
	ON_COMMAND(ID_FILE_FINISH, OnFileFinish)
	ON_COMMAND(ID_VIEW_UNITBAR, OnViewUnitbar)
	ON_UPDATE_COMMAND_UI(ID_VIEW_UNITBAR, OnUpdateViewUnitbar)
	ON_COMMAND(ID_FILE_FINISHEXIT, OnFileFinishExit)
	//}}AFX_MSG_MAP
  ON_NOTIFY(TBN_DROPDOWN, AFX_IDW_TOOLBAR, OnDropDownGrid)
END_MESSAGE_MAP()

static UINT indicators[] =
{
	ID_SEPARATOR,           // status line indicator
  ID_SEPARATOR,           // selected mapex
  ID_SEPARATOR            // drawing mapex
};

/////////////////////////////////////////////////////////////////////////////
// CMainFrame construction/destruction

CMainFrame::CMainFrame()
{
  m_pMap = NULL;	
  m_bMaximized = FALSE;
}

CMainFrame::~CMainFrame()
{
}

int CMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
  CWinApp *pApp = AfxGetApp();

	if (CFrameWnd::OnCreate(lpCreateStruct) == -1)
		return -1;
	
  m_wndMapexBar.LoadState("MapexBar");
  m_wndUnitBar.LoadState("UnitBar");

	if (!m_wndToolBar.CreateEx(this, TBSTYLE_FLAT, WS_CHILD | WS_VISIBLE | CBRS_TOP
		| CBRS_GRIPPER | CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC) ||
		!m_wndToolBar.LoadToolBar(IDR_MAINFRAME))
	{
		TRACE0("Failed to create toolbar\n");
		return -1;      // fail to create
	}
  {
    CBitmap bmpToolBar;
    bmpToolBar.LoadBitmap(IDB_TOOLBAR_COLOR);
    m_imglsToolBarColor.Create(16, 15, ILC_COLOR24 | ILC_MASK, 0, 1);
    m_imglsToolBarColor.Add(&bmpToolBar, RGB(192, 192, 192));
    bmpToolBar.DeleteObject();
    bmpToolBar.LoadBitmap(IDB_TOOLBAR_GRAY);
    m_imglsToolBarGray.Create(16, 15, ILC_COLOR24 | ILC_MASK, 0, 1);
    m_imglsToolBarGray.Add(&bmpToolBar, RGB(192, 192, 192));
    bmpToolBar.DeleteObject();
    bmpToolBar.LoadBitmap(IDB_TOOLBAR_DISABLE);
    m_imglsToolBarDisabled.Create(16, 15, ILC_COLOR24 | ILC_MASK, 0, 1);
    m_imglsToolBarDisabled.Add(&bmpToolBar, RGB(192, 192, 192));
    bmpToolBar.DeleteObject();
    m_wndToolBar.GetToolBarCtrl().SetHotImageList(&m_imglsToolBarColor);
    m_wndToolBar.GetToolBarCtrl().SetImageList(&m_imglsToolBarGray);
    m_wndToolBar.GetToolBarCtrl().SetDisabledImageList(&m_imglsToolBarDisabled);
  }

  m_wndToolBar.GetToolBarCtrl().SetExtendedStyle(TBSTYLE_EX_DRAWDDARROWS);
  TBBUTTONINFO butinfo;
  butinfo.cbSize = sizeof(butinfo);
  butinfo.dwMask = TBIF_STYLE;
  butinfo.fsStyle = TBSTYLE_DROPDOWN | TBSTYLE_CHECK;
  m_wndToolBar.GetToolBarCtrl().SetButtonInfo(ID_MAP_GRID, &butinfo);

  m_wndToolBar.SetButtonInfo(m_wndToolBar.CommandToIndex(ID_MAPEX_ZPOS), ID_MAPEX_ZPOS, TBBS_SEPARATOR, 150);

  m_ComboFont.CreatePointFont(80, "MS Sans Serif", NULL);

  CRect rcZPos;
  m_wndToolBar.GetItemRect(m_wndToolBar.CommandToIndex(ID_MAPEX_ZPOS), &rcZPos);
  rcZPos.bottom = rcZPos.top + 150;
  m_ZPosCombo.Create(CBS_DROPDOWN | CBS_AUTOHSCROLL, rcZPos, &m_wndToolBar, ID_MAPEX_ZPOS);
  m_ZPosCombo.ShowWindow(SW_SHOW);
  m_wndToolBar.GetDlgItem(ID_MAPEX_ZPOS)->SetFont(&m_ComboFont);

  m_ZPosCombo.SetItemData(m_ZPosCombo.AddString("200000 Vysoká letadla"), 200000);
  m_ZPosCombo.SetItemData(m_ZPosCombo.AddString("150000 Nízká letadla"), 150000);
  m_ZPosCombo.SetItemData(m_ZPosCombo.AddString("45000 Vznášedla"), 45000);
  m_ZPosCombo.SetItemData(m_ZPosCombo.AddString("35000 Pozemní jednotky"), 35000);
  m_ZPosCombo.SetItemData(m_ZPosCombo.AddString("25000 Podzemí"), 25000);
  m_ZPosCombo.SetItemData(m_ZPosCombo.AddString("15000 Voda"), 15000);
  m_ZPosCombo.SetItemData(m_ZPosCombo.AddString("5000 Pod vodou"), 5000);
  m_ZPosCombo.SetWindowText("35000");

  if (!m_wndMapexBar.CreateEx(this, TBSTYLE_FLAT, WS_CHILD | WS_VISIBLE | CBRS_RIGHT
    | CBRS_GRIPPER | CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC,
    CRect(0, 0, 0, 0), ID_MAPEXTOOLBAR))
  {
    TRACE0("Failed to create toolbar\n");
    return -1;      // fail to create
  }
  m_wndMapexBar.LoadToolBar(IDR_MAPEXBAR);
  m_wndMapexBar.CreateButtons();
  m_wndMapexBar.SetWindowText("Mapexy");

  if (!m_wndUnitBar.CreateEx(this, TBSTYLE_FLAT, WS_CHILD | WS_VISIBLE | CBRS_BOTTOM
    | CBRS_GRIPPER | CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC,
    CRect(0, 0, 0, 0), ID_UNITTOOLBAR))
  {
    TRACE0("Failed to create toolbar\n");
    return -1;      // fail to create
  }
  m_wndUnitBar.LoadToolBar(IDR_UNITBAR);
  m_wndUnitBar.CreateButtons();
  m_wndUnitBar.SetWindowText("Jednotky");

  CRect rcMiniMap;
  rcMiniMap.left = pApp->GetProfileInt("Layout", "MiniMapX", 0);
  rcMiniMap.top = pApp->GetProfileInt("Layout", "MiniMapY", 0);
  rcMiniMap.right = rcMiniMap.left + 206;
  rcMiniMap.bottom = rcMiniMap.top + 222;
  m_pMiniMapFrame = (CMiniMapFrame *)RUNTIME_CLASS(CMiniMapFrame)->CreateObject();
  m_pMiniMapFrame->Create(NULL, "MiniMap", WS_POPUP | WS_CAPTION | WS_SYSMENU | MFS_SYNCACTIVE,
    rcMiniMap, this, 0);

	if (!m_wndStatusBar.Create(this) ||
		!m_wndStatusBar.SetIndicators(indicators,
		  sizeof(indicators)/sizeof(UINT)))
	{
		TRACE0("Failed to create status bar\n");
		return -1;      // fail to create
	}
  m_wndStatusBar.SetPaneInfo(1, ID_SELECTEDMAPEX, SBPS_NORMAL, 150);
  m_wndStatusBar.GetStatusBarCtrl().SetIcon(1, (HICON)::LoadImage(AfxGetInstanceHandle(),
    MAKEINTRESOURCE(IDI_SELECTIONICON), IMAGE_ICON, 0, 0, 0));
  m_wndStatusBar.SetPaneInfo(2, ID_DRAWINGMAPEX, SBPS_NORMAL, 150);
  m_wndStatusBar.GetStatusBarCtrl().SetIcon(2, (HICON)::LoadImage(AfxGetInstanceHandle(),
    MAKEINTRESOURCE(IDI_DRAWICON), IMAGE_ICON, 0, 0, 0));

	m_wndToolBar.EnableDocking(CBRS_ALIGN_ANY);
  m_wndMapexBar.EnableDocking(CBRS_ALIGN_ANY);
  m_wndUnitBar.EnableDocking(CBRS_ALIGN_ANY);
	EnableDocking(CBRS_ALIGN_ANY);
  DockControlBar(&m_wndUnitBar);
  DockControlBar(&m_wndMapexBar);
	DockControlBar(&m_wndToolBar);

  LoadBarState("Layout\\MainFrame");
  if(pApp->GetProfileInt("Layout", "Maximized", 0) != 0){
    m_bMaximized = TRUE;
  }

	return 0;
}

void CMainFrame::OnDestroy() 
{
  CWinApp *pApp = AfxGetApp();

  m_wndMapexBar.SaveState("MapexBar");
  m_wndUnitBar.SaveState("UnitBar");
  SaveBarState("Layout\\MainFrame");

  if(IsZoomed()){
    pApp->WriteProfileInt("Layout", "Maximized", 1);
  }
  else{
    pApp->WriteProfileInt("Layout", "Maximized", 0);
  }

  CloseToolbars();

	CFrameWnd::OnDestroy();	
}


BOOL CMainFrame::PreCreateWindow(CREATESTRUCT& cs)
{
	if( !CFrameWnd::PreCreateWindow(cs) )
		return FALSE;

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// CMainFrame diagnostics

#ifdef _DEBUG
void CMainFrame::AssertValid() const
{
	CFrameWnd::AssertValid();
}

void CMainFrame::Dump(CDumpContext& dc) const
{
	CFrameWnd::Dump(dc);
}

#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CMainFrame message handlers

void CMainFrame::OnNewMap(CEMap *pMap)
{
  m_pMap = pMap;
  if(!IsWindow(GetSafeHwnd())){
    return;
  }
  m_wndMapexBar.SetSelectedMapexes(pMap);
  m_wndUnitBar.SetSelectedUnit(pMap);
  AppGetActiveDocument()->GetMiniMap()->SetDrawWnd(m_pMiniMapFrame);
}

void CMainFrame::SetSelectedMapexPaneText(CString strText)
{
  m_wndStatusBar.SetPaneText(1, strText, TRUE);
}

void CMainFrame::SetDrawingMapexPaneText(CString strText)
{
  m_wndStatusBar.SetPaneText(2, strText, TRUE);
}

void CMainFrame::OnDropDownGrid(NMHDR *pNotifyStruct, LRESULT *pResult)
{
  LPNMTOOLBAR pNMToolBar = (LPNMTOOLBAR)pNotifyStruct;

  switch(pNMToolBar->iItem){
  case ID_MAP_GRID:
    CMenu menu, *pPopupMenu;
    menu.LoadMenu(IDM_GRIDSIZEMENU);
    pPopupMenu = menu.GetSubMenu(0);

    CRect rcButton;
    m_wndToolBar.GetToolBarCtrl().GetRect(ID_MAP_GRID, &rcButton);
    m_wndToolBar.ClientToScreen(&rcButton);
    pPopupMenu->TrackPopupMenu(TPM_LEFTALIGN | TPM_LEFTBUTTON, rcButton.left, rcButton.bottom, this);
    break;
  }
  *pResult = TBDDRET_DEFAULT;
}

void CMainFrame::OnUpdateViewMinimap(CCmdUI* pCmdUI) 
{
  if(m_pMiniMapFrame->GetStyle() & WS_VISIBLE){
    pCmdUI->SetCheck(1);
  }
  else{
    pCmdUI->SetCheck(0);
  }
}

void CMainFrame::OnViewMinimap() 
{
  if(m_pMiniMapFrame->GetStyle() & WS_VISIBLE){
    m_pMiniMapFrame->ShowWindow(SW_HIDE);
  }
  else{
    m_pMiniMapFrame->ShowWindow(SW_SHOW);
  }
}

void CMainFrame::OnShowWindow(BOOL bShow, UINT nStatus) 
{
	CFrameWnd::OnShowWindow(bShow, nStatus);
	
  if(bShow)
    m_pMiniMapFrame->ShowWindow(SW_SHOW);
  else
    m_pMiniMapFrame->ShowWindow(SW_HIDE);
}

void CMainFrame::OnUpdateViewMapexbar(CCmdUI* pCmdUI) 
{
  if(m_wndMapexBar.GetStyle() & WS_VISIBLE) pCmdUI->SetCheck(1);
  else pCmdUI->SetCheck(0);
}

void CMainFrame::OnViewMapexbar() 
{
	if(m_wndMapexBar.GetStyle() & WS_VISIBLE) ShowControlBar(&m_wndMapexBar, FALSE, TRUE);
  else ShowControlBar(&m_wndMapexBar, TRUE, TRUE);
}

void CMainFrame::SetZPosComboValue(DWORD dwZPos)
{
  CString str;
  str.Format("%d", dwZPos);
  m_ZPosCombo.SetWindowText(str);
}

DWORD CMainFrame::GetZPosComboValue()
{
  CString str;
  m_ZPosCombo.GetWindowText(str);
  long l = atol(str);
  if(l <= 0){
    AfxMessageBox("Výška mapexu musí být èíslo. ( >= 1 )");
    return 0;
  }
  return l;
}

void CMainFrame::EnableZPosCombo(BOOL bEnable)
{
  m_ZPosCombo.EnableWindow(bEnable);
}

void CMainFrame::OnSelEndMapexZPos()
{
  int nSel = m_ZPosCombo.GetCurSel();
  AppGetActiveDocument()->OnChangedMapexZPos(m_ZPosCombo.GetItemData(nSel));
}

void CMainFrame::OnOk() 
{
  AppGetActiveDocument()->OnChangedMapexZPos(GetZPosComboValue());
}

void CMainFrame::ResetMapexDrawingSelection()
{
  m_wndMapexBar.ResetSelection();
}

void CMainFrame::ResetUnitDrawingSelection()
{
  m_wndUnitBar.ResetSelection();
}

void CMainFrame::OnFileRebuildLibraries() 
{
  // ask the user if he's sure
  if(AfxMessageBox("Optimalizace knihoven se provede az pøi zavøení mapy.\nChcete ji takto naplánovat?", MB_YESNO) != IDYES)
    return;

  SetMessageText("Optimalizace knihoven...");
  AppGetActiveMap()->RebuildLibraries();
  SetMessageText((LPCSTR)NULL);
}

void CMainFrame::OnFileFinish() 
{
  FinishMap(FALSE);
}

void CMainFrame::FinishMap(BOOL bExit)
{
  // First save the map (it must be saved)
  CMapEditorDoc *pDoc = AppGetActiveDocument();
  if(pDoc == NULL) return;
  pDoc->SetModifiedFlag();
  if(!pDoc->SaveModified()) return;

  // Then ask for the export file name
  CString strExportName = AppGetActiveDocument()->GetPathName();
  {
    int nPos = strExportName.ReverseFind('.');
    if(nPos != -1){
      strExportName = strExportName.Left(nPos) + ".TheMap";
    }
    else{
      strExportName += ".TheMap";
    }
  }
  CFileDialog filedlg(FALSE, "*.TheMap", strExportName, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
    "Hotové mapy (*.TheMap)|*.TheMap|Všechny soubory (*.*)|*.*||", this);
  filedlg.m_ofn.lpstrTitle = "Exportovat hotovou mapu";
  if(filedlg.DoModal() != IDOK){
    return;
  }
  strExportName = filedlg.GetPathName();
  UpdateWindow();


  // remeber paths to libraries
  CStringArray aMapexLibrariesFull, aMapexLibrariesName;
  CStringArray aUnitTypeLibrariesFull, aUnitTypeLibrariesName;
  {
    CEMap *pMap = AppGetActiveMap();
    CEMapexLibrary *pMapexLibrary;
    POSITION pos;

    pos = pMap->GetFirstMapexLibraryPosition();
    while(pos != NULL){
      pMapexLibrary = pMap->GetNextMapexLibrary(pos);
      aMapexLibrariesFull.Add(pMapexLibrary->GetFullPath());
      aMapexLibrariesName.Add(pMapexLibrary->GetFileName());
    }

    CEUnitLibrary *pUnitLibrary;
    pos = pMap->GetFirstUnitLibraryPosition();
    while(pos != NULL){
      pUnitLibrary = pMap->GetNextUnitLibrary(pos);
      aUnitTypeLibrariesFull.Add(pUnitLibrary->GetFullPath());
      aUnitTypeLibrariesName.Add(pUnitLibrary->GetFileName());
    }
  }

  // Start progress dialog
  CWaitCursor wait;
  CFinishDlg m_wndFinishDlg;
  m_wndFinishDlg.Create(IDD_FINISH_PROGRESS);
  m_wndFinishDlg.ShowWindow(SW_SHOW);
  m_wndFinishDlg.UpdateWindow();

  // Open the export archive object
  CDataArchive ExportArchive;

  try{
    // open the archive file -> the exported map
    ::DeleteFile(strExportName);
    VERIFY(ExportArchive.Create(strExportName, CArchiveFile::modeReadWrite | CArchiveFile::modeCreate, CDataArchiveInfo::archiveFile));
  
    // compute minimaps
    m_wndFinishDlg.SetText("Generování minimap...");
    ExportMinimaps(AppGetActiveMap(), ExportArchive, &m_wndFinishDlg);
  }
  catch(CException *e){
    e->ReportError();
    // Before end just reload the map again
    m_wndFinishDlg.DestroyWindow();
  }

  CString strMapFileName;

  try{

    // Compute the graph landtypes
    CEMap *pMap = AppGetActiveMap();

    // Export the script sets
    m_wndFinishDlg.SetText("Exportování mapy - SkriptSety...");
    m_wndFinishDlg.SetProgress(0);
    {
      CEMap *pMap = AppGetActiveMap();
      CDataArchive ScriptSetArchive;
      int nPos, nCount = pMap->m_listScriptSet.GetCount();

      ScriptSetArchive = ExportArchive.CreateArchive("ScriptSets", TRUE);

      POSITION pos = pMap->m_listScriptSet.GetHeadPosition();
      CEScriptSet *pScriptSet;
      while(pos != NULL){
        pScriptSet = pMap->m_listScriptSet.GetNext(pos);

        pScriptSet->Export(ScriptSetArchive);

        nPos++;
        m_wndFinishDlg.SetProgress(nPos * 100 / nCount);
      }
    }

    // close the map
    // First I must remember the file from which to load it again
    m_wndFinishDlg.SetText("Zavírání mapy...");

    // delete all toolbar buttons
    CloseToolbars();
  
    strMapFileName = AppGetActiveDocument()->GetPathName();
    AppGetActiveMap()->Delete(&m_wndFinishDlg.m_wndProgress);
    // some computation

    m_wndFinishDlg.SetText("Exportování mapy...");
    // export the map

    // insert the map file
    ExportArchive.AppendFile(strMapFileName, "Map", appendCompressed);

    m_wndFinishDlg.SetText("Exportování mapy - mapexy...");
    m_wndFinishDlg.SetProgress(0);
    // insert libraries
    {
      int nLib;
      for(nLib = 0; nLib < aMapexLibrariesFull.GetSize(); nLib++){
        CDataArchive MapexLibrary;
        MapexLibrary.Create(aMapexLibrariesFull[nLib], CArchiveFile::modeRead | CFile::shareDenyWrite, CDataArchiveInfo::archiveFile);

        try{
          ExportArchive.MakeDirectory(aMapexLibrariesName[nLib]);
        }
        catch(CException *e){
          e->Delete();
        }
        ExportArchive.AppendDir(aMapexLibrariesName[nLib], MapexLibrary, "");

        m_wndFinishDlg.SetProgress((nLib + 1) * 100 / aMapexLibrariesFull.GetSize());
      }
    }

    m_wndFinishDlg.SetText("Exportování mapy - jednotky...");
    m_wndFinishDlg.SetProgress(0);
    {
      int nLib;
      for(nLib = 0; nLib < aUnitTypeLibrariesFull.GetSize(); nLib++){
        CDataArchive UnitLibrary;
        UnitLibrary.Create(aUnitTypeLibrariesFull[nLib], CArchiveFile::modeRead | CFile::shareDenyWrite, CDataArchiveInfo::archiveFile);

        try{
          ExportArchive.MakeDirectory(aUnitTypeLibrariesName[nLib]);
        }
        catch(CException *e){
          e->Delete();
        }
        ExportArchive.AppendDir(aUnitTypeLibrariesName[nLib], UnitLibrary, "");

        m_wndFinishDlg.SetProgress((nLib + 1) * 100 / aUnitTypeLibrariesFull.GetSize());
      }
    }

    // Export the script sets
    m_wndFinishDlg.SetText("Vytváøení grafù pro hledání cest...");
    m_wndFinishDlg.SetProgress(0);

    {

      switch(CreateFindPathGraphs(ExportArchive))
      {
      case ECFPGE_IncompleteMapLand:
        AfxMessageBox("Mapa obsahuje mapcelly, které nejsou pokryté žádným typem povrchu!\nPøi použití se mohou nastat chyby v grafice.");
      case ECFPGE_OK:
        break;

      case ECFPGE_NotEnoughMemory:
        AfxThrowMemoryException();

      case ECFPGE_UnknownError:
      case ECFPGE_BadMapFile:
      default:
        throw new CStringException("Interní chyba pøi vytváøení grafù pro hledání cest.");
      }
    }
  }
  catch(CException *e){
    e->ReportError();
  }

  // Before end just reload the map again
  m_wndFinishDlg.DestroyWindow();

  if(bExit){
    // If we're about to exit -> do it
    SendMessage(WM_CLOSE);
    return;
  }

  // reopen toolbars
  OpenToolbars();

  AppGetActiveDocument()->OnOpenDocument(strMapFileName);
}

void CMainFrame::OnViewUnitbar() 
{
	if(m_wndUnitBar.GetStyle() & WS_VISIBLE) ShowControlBar(&m_wndUnitBar, FALSE, TRUE);
  else ShowControlBar(&m_wndUnitBar, TRUE, TRUE);
}

void CMainFrame::OnUpdateViewUnitbar(CCmdUI* pCmdUI) 
{
  if(m_wndUnitBar.GetStyle() & WS_VISIBLE) pCmdUI->SetCheck(1);
  else pCmdUI->SetCheck(0);
}

void CMainFrame::CloseToolbars()
{
  m_wndMapexBar.DeleteButtons();
  m_wndMapexBar.SaveState("MapexBar");
  m_wndUnitBar.DeleteButtons();
  m_wndUnitBar.SaveState("UnitBar");
}

void CMainFrame::OpenToolbars()
{
  m_wndUnitBar.LoadState("UnitBar");
  m_wndUnitBar.CreateButtons();
  m_wndMapexBar.LoadState("MapexBar");
  m_wndMapexBar.CreateButtons();
}

void CMainFrame::ExportMinimaps(CEMap *pMap, CDataArchive Archive, CFinishDlg *pFinishDlg)
{
  // Here we will compute all minimaps background as TrueColor bitmaps (24bit)
  // We'll do it also for all zooms - it means for 2, 4, 8 and 16
  // We'll save the result to the disk as small files each of size 256x256 pixels
  //  the files near right-bottom edges are saved with same size, but the empty space
  //  will be filled with black color

  // Files will be saved to the export archive to directory MiniMaps
  // Each zoom under separate directory of names '2', '4', '8', '16'
  // The filenames will be MinimapZZXXXXYYYY.raw
  //  where: ZZ is the zoom factor (02, 04, 08, 16)
  //         XXXX is the index of the file in x coord (starting from 0)
  //         YYYY is the index of the file in y coord (starting from 0)
  //  Maximum values of XXXX and YYYY can be computed as (MapWidthInMapCells - 1) / (256 * Zoom) + 1
  //  (and Height as well)


  // Some variables

  // Buffer for the result bitmap (this will be saved to disk)
  BYTE *pResult = NULL, *pSource = NULL;
  pResult = new BYTE[256 * 256 * 3];
  pSource = new BYTE[256 * 256 * 3];
  // Buffer for landtypes drawned from the map
  BYTE *pLandTypesBuffer = NULL;
  pLandTypesBuffer = new BYTE[512 * 512];

  try{
    // Make directory for minimaps
    try{
      Archive.MakeDirectory("MiniMaps");
    }
    catch(CException *e){
      e->Delete();
    }

    // So first compute the 2x minimaps
    {
      // Make the directory
      try{
        Archive.MakeDirectory("MiniMaps\\2");
      }
      catch(CException *e){
        e->Delete();
      }

      // Set some progress bars
      pFinishDlg->SetText("Generování minimap - 2x...");
      DWORD dwWhole, dwCurrent = 0;
      pFinishDlg->SetProgress(0);

      // Coordinates in pieces where we're
      DWORD dwXPiecePos, dwYPiecePos;
      // Number of pieces in x and y directions
      DWORD dwPieceWidth, dwPieceHeight;

      dwPieceWidth = (pMap->GetWidth() - 1) / 512 + 1;
      dwPieceHeight = (pMap->GetHeight() - 1) / 512 + 1;

      dwWhole = dwPieceWidth * dwPieceHeight;

      // Position in mapcells of the current piece we're computing
      CRect rcPos;
      // List of mapsquares we're interested in
      CTypedPtrList<CPtrList, CEMapSquare *> listSquares;
      CTypedPtrList<CPtrList, CEMapexInstance *> listMapexes;

      for(dwYPiecePos = 0; dwYPiecePos < dwPieceHeight; dwYPiecePos++){
        for(dwXPiecePos = 0; dwXPiecePos < dwPieceWidth; dwXPiecePos++){
          // Compute the position in mapcells
          rcPos.left = dwXPiecePos * 512; rcPos.top = dwYPiecePos * 512;
          rcPos.right = rcPos.left + 512; rcPos.bottom = rcPos.top + 512;

          // Now we'll build a list of mapsquares wich we're interested in
          {
            listSquares.RemoveAll();

            int x, y;
            for(y = rcPos.top - MAPSQUARE_HEIGHT; y < rcPos.bottom + MAPSQUARE_HEIGHT; y += MAPSQUARE_HEIGHT){
              if(y < 0) continue;
              if(y >= (int)pMap->GetHeight()) continue;
              for(x = rcPos.left - MAPSQUARE_WIDTH; x < rcPos.right + MAPSQUARE_WIDTH; x += MAPSQUARE_WIDTH){
                if((x < 0) || (x >= (int)pMap->GetWidth())) continue;
                listSquares.AddHead(pMap->GetMapSquareFromCell(x, y));
              }
            }
          }

          memset(pLandTypesBuffer, 0, 512 * 512);

          // Now for all background levels
          DWORD dwLevel;
          for(dwLevel = 1; dwLevel <= 3; dwLevel++){
            // Build a list of all interesting mapexes
            {
              CEMapSquare *pSquare;
              POSITION pos;

              listMapexes.RemoveAll();
              // Go through all mapsquares
              pos = listSquares.GetHeadPosition();
              while(pos != NULL){
                pSquare = listSquares.GetNext(pos);

                pSquare->AppendBLMapexes(dwLevel, &listMapexes);
              }
            }

            // Draw them
            {
              POSITION pos;
              CEMapexInstance *pMapexInst;
              CEMapex *pMapex;
              CSize sizeMapex;
              int x, y, xh, yh;
              BYTE bLandType;


              // Go through all mapexes
              pos = listMapexes.GetHeadPosition();
              while(pos != NULL){
                pMapexInst = listMapexes.GetNext(pos);
                pMapex = pMapexInst->GetMapex();
                if(pMapex != NULL){
                  sizeMapex = pMapex->GetSize();
                }
                else{
                  sizeMapex.cx = 0; sizeMapex.cy = 0;
                }

                yh = pMapexInst->GetYPosition() - rcPos.top;
                for(y = 0; y < sizeMapex.cy; y++, yh++){
                  if((yh < 0) || (yh >= rcPos.Height())) continue;
        
                  xh = pMapexInst->GetXPosition() - rcPos.left;
                  for(x = 0; x < sizeMapex.cx; x++, xh++){
                    if((xh < 0) || (xh >= rcPos.Width())) continue;

                    bLandType = pMapex->GetLandType(x, y);
                    if(bLandType != 0){
                      pLandTypesBuffer[xh + yh * 512] = bLandType;
                    }
                  }
                }
              } // while( mapexes )

            } // Drawing

          } // for( levels )

          // Now we have in pLandTypesBuffer the land types
          // We'll zoom them and convert to RGB colors
          {
            int x, y;
            int nR, nG, nB;
            DWORD dwColor;
            BYTE bLandType;

            // Clear the result
            memset(pResult, 0, 256 * 256 * 3);

            for(y = 0; y < 256; y++){
              for(x = 0; x < 256; x++){
                nR = 0; nG = 0; nB = 0;

                bLandType = pLandTypesBuffer[x * 2 + (y * 2 * 512)];
                if(pMap->GetLandType(bLandType) == NULL) continue;
                if(bLandType != 0){
                  dwColor = pMap->GetLandType(bLandType)->GetColor();
                  nR += R32(dwColor); nG += G32(dwColor); nB += B32(dwColor);
                }
                bLandType = pLandTypesBuffer[x * 2 + 1 + (y * 2 * 512)];
                if(bLandType != 0){
                  dwColor = pMap->GetLandType(bLandType)->GetColor();
                  nR += R32(dwColor); nG += G32(dwColor); nB += B32(dwColor);
                }
                bLandType = pLandTypesBuffer[x * 2 + ((y * 2 + 1) * 512)];
                if(bLandType != 0){
                  dwColor = pMap->GetLandType(bLandType)->GetColor();
                  nR += R32(dwColor); nG += G32(dwColor); nB += B32(dwColor);
                }
                bLandType = pLandTypesBuffer[x * 2 + 1 + ((y * 2 + 1) * 512)];
                if(bLandType != 0){
                  dwColor = pMap->GetLandType(bLandType)->GetColor();
                  nR += R32(dwColor); nG += G32(dwColor); nB += B32(dwColor);
                }

                nR = nR * MINIMAP_DARKMULTIPLY / (4 * MINIMAP_DARKDIVISION);
                nG = nG * MINIMAP_DARKMULTIPLY / (4 * MINIMAP_DARKDIVISION);
                nB = nB * MINIMAP_DARKMULTIPLY / (4 * MINIMAP_DARKDIVISION);

                // Write it to the result
                pResult[(x + y * 256) * 3] = (BYTE)nR;
                pResult[(x + y * 256) * 3 + 1] = (BYTE)nG;
                pResult[(x + y * 256) * 3 + 2] = (BYTE)nB;
              }
            }
          }

          // And now write the result to the disk
          {
            CArchiveFile file;
            CString strFileName;
            strFileName.Format("MiniMaps\\2\\MiniMap02%04d%04d.raw", dwXPiecePos, dwYPiecePos);

            file = Archive.CreateFile(strFileName, CArchiveFile::modeReadWrite | CArchiveFile::modeCreate);
            file.Write(pResult, 256 * 256 * 3);
            file.Close();
          }

          // Update progress bar
          dwCurrent++;
          pFinishDlg->SetProgress(dwCurrent * 100 / dwWhole);
        }
      } // for( x, y - pieces )

    }

    // Now compute all other minimaps
    // It will be done in more simple way
    // We'll just zoom out already computed pictures
    {
      DWORD dwZoom, dwPrevZoom, dwZoomExp;
      CString strPrevDir, strDir;
      DWORD dwPieceWidth, dwPieceHeight;
      DWORD dwPrevPieceWidth, dwPrevPieceHeight;
      DWORD dwXPiecePos, dwYPiecePos;
      DWORD dwWhole, dwCurrent;

      // Loop for zooms
      for(dwZoomExp = 2; dwZoomExp <= 4; dwZoomExp++){
        dwZoom = 1 << dwZoomExp;
        dwPrevZoom = dwZoom >> 1;

        strDir.Format("MiniMaps\\%d", dwZoom);
        strPrevDir.Format("MiniMaps\\%d", dwPrevZoom);

        {
          CString strHlp;
          strHlp.Format("Generating minimaps - %dx...", dwZoom);
          pFinishDlg->SetText(strHlp);
          pFinishDlg->SetProgress(0);
          dwCurrent = 0;
        }

        // Create the directory
        try{
          Archive.MakeDirectory(strDir);
        }
        catch(CException *e){
          e->Delete();
        }

        // Compute sizes in pieces
        dwPieceWidth = (pMap->GetWidth() - 1) / (256 * dwZoom) + 1;
        dwPieceHeight = (pMap->GetHeight() - 1) / (256 * dwZoom) + 1;
        dwPrevPieceWidth = (pMap->GetWidth() - 1) / (256 * dwPrevZoom) + 1;
        dwPrevPieceHeight = (pMap->GetHeight() - 1) / (256 * dwPrevZoom) + 1;

        dwWhole = dwPieceWidth * dwPieceHeight;

        for(dwYPiecePos = 0; dwYPiecePos < dwPieceHeight; dwYPiecePos++){
          for(dwXPiecePos = 0; dwXPiecePos < dwPieceWidth; dwXPiecePos++){

            // Go through the previous buffers (4 ones) and zoom them to the result
            {
              // Clear the result
              memset(pResult, 0, 256 * 256 * 3);

              int x, y;
              DWORD dwXPrevPiecePos, dwYPrevPiecePos;
              for(y = 0; y < 2; y++){
                dwYPrevPiecePos = dwYPiecePos * 2 + y;
                if(dwYPrevPiecePos >= dwPrevPieceHeight) continue;
                for(x = 0; x < 2; x++){
                  dwXPrevPiecePos = dwXPiecePos * 2 + x;
                  if(dwXPrevPiecePos >= dwPrevPieceWidth) continue;

                  // get the file name
                  CString strFileName;
                  strFileName.Format("%s\\MiniMap%02d%04d%04d.raw", strPrevDir, dwPrevZoom, dwXPrevPiecePos, dwYPrevPiecePos);

                  CArchiveFile file;
                  file = Archive.CreateFile(strFileName, CArchiveFile::modeRead | CFile::shareDenyWrite);
                  file.Read(pSource, 256 * 256 * 3);
                  file.Close();

                  // Go through the image and zoom it
                  int xh, yh;
                  int nR, nG, nB;
                  DWORD dwOff;
                  for(yh = 0; yh < 128; yh++){
                    for(xh = 0; xh < 128; xh++){
                      nR = nG = nB = 0;

                      dwOff = (yh * 2) * 256 + (xh * 2);
                      dwOff *= 3;
                      nR += pSource[dwOff];
                      nG += pSource[dwOff + 1];
                      nB += pSource[dwOff + 2];
                      dwOff = (yh * 2) * 256 + (xh * 2 + 1);
                      dwOff *= 3;
                      nR += pSource[dwOff];
                      nG += pSource[dwOff + 1];
                      nB += pSource[dwOff + 2];
                      dwOff = (yh * 2 + 1) * 256 + (xh * 2);
                      dwOff *= 3;
                      nR += pSource[dwOff];
                      nG += pSource[dwOff + 1];
                      nB += pSource[dwOff + 2];
                      dwOff = (yh * 2 + 1) * 256 + (xh * 2 + 1);
                      dwOff *= 3;
                      nR += pSource[dwOff];
                      nG += pSource[dwOff + 1];
                      nB += pSource[dwOff + 2];

                      nR /= 4; nG /= 4; nB /= 4;

                      dwOff = (y * 128 + yh) * 256 + (x * 128 + xh);
                      dwOff *= 3;
                      pResult[dwOff] = (BYTE)nR;
                      pResult[dwOff + 1] = (BYTE)nG;
                      pResult[dwOff + 2] = (BYTE)nB;
                    }
                  } // for( x, y = source image pixel )

                }
              } // for( x,y - prev pieces )
            }

            // Write the piece to the disk
            {
              CString strFileName;
              strFileName.Format("%s\\MiniMap%02d%04d%04d.raw", strDir, dwZoom, dwXPiecePos, dwYPiecePos);

              CArchiveFile file;
              file = Archive.CreateFile(strFileName, CArchiveFile::modeWrite | CArchiveFile::modeCreate);
              file.Write(pResult, 256 * 256 * 3);
              file.Close();
            }

            dwCurrent++;
            pFinishDlg->SetProgress(dwCurrent * 100 / dwWhole);
          }
        }  // for( x,y - pieces )

      } // Zooms
    }

  }
  catch(CException *){
    if(pResult != NULL) delete pResult;
    if(pSource != NULL) delete pSource;
    if(pLandTypesBuffer != NULL) delete pLandTypesBuffer;
    throw;
  }

  // Free buffers
  if(pResult != NULL) delete pResult;
  if(pSource != NULL) delete pSource;
  if(pLandTypesBuffer != NULL) delete pLandTypesBuffer;
}

void CMainFrame::OnFileFinishExit() 
{
  FinishMap(TRUE);	
}
