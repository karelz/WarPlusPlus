// MapEditorDoc.cpp : implementation of the CMapEditorDoc class
//

#include "stdafx.h"
#include "MapEditor.h"

#include "MapEditorDoc.h"
#include "MapEditorView.h"
#include "MainFrm.h"

#include "Dialogs\MapPropertiesDlg.h"
#include "Dialogs\NewMapDlg.h"
#include "Dialogs\ResourcesDlg.h"
#include "Dialogs\InvisibilityDlg.h"
#include "Dialogs\ScriptSetEditDlg.h"
#include "Dialogs\FindPathGraphsDlg.h"
#include "Dialogs\SetMapSizeDlg.h"

#include "CivilizationEditor\CivilizationEditorDlg.h"

#include "MapexEditor\MapexEditorDlg.h"
#include "MapexEditor\MapexLibraryEditDlg.h"
#include "MapexEditor\EditMapexLibraryDlg.h"
#include "MapexEditor\DeleteMapexLibraryDlg.h"

#include "UnitEditor\DeleteUnitLibraryDlg.h"
#include "UnitEditor\EditUnitLibraryDlg.h"
#include "UnitEditor\UnitLibraryEditorDlg.h"
#include "UnitEditor\UnitLibrariesDlg.h"

#include "Controls\FinishDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define MAPEX_DEFAULT_ZPOSITION 100

/////////////////////////////////////////////////////////////////////////////
// CMapEditorDoc

const int g_nMiniMapZooms = 7;
const DWORD g_aMiniMapZooms[g_nMiniMapZooms] = {1, 2, 4, 8, 16, 32, 50};

IMPLEMENT_DYNCREATE(CMapEditorDoc, CDocument)

BEGIN_MESSAGE_MAP(CMapEditorDoc, CDocument)
	//{{AFX_MSG_MAP(CMapEditorDoc)
	ON_COMMAND(ID_MAPEXEDITOR, OnMapexEditor)
	ON_COMMAND(ID_FILE_PROPERTIES, OnFileProperties)
	ON_COMMAND(ID_MAPEX_NEWLIBRARY, OnMapexNewLibrary)
	ON_COMMAND(ID_MAPEX_OPENLIBRARY, OnMapexOpenLibrary)
	ON_COMMAND(ID_MAPEX_EDITLIBRARY, OnMapexEditLibrary)
	ON_COMMAND(ID_MAPEX_DELETELIBRARY, OnMapexDeleteLibrary)
	ON_COMMAND(ID_FILE_SETLANDTYPES, OnFileSetLandTypes)
	ON_COMMAND(ID_VIEW_BL1, OnViewBL1)
	ON_COMMAND(ID_VIEW_BL2, OnViewBL2)
	ON_COMMAND(ID_VIEW_BL3, OnViewBL3)
	ON_COMMAND(ID_VIEW_UL, OnViewUL)
	ON_UPDATE_COMMAND_UI(ID_VIEW_BL1, OnUpdateViewBL1)
	ON_UPDATE_COMMAND_UI(ID_VIEW_BL2, OnUpdateViewBL2)
	ON_UPDATE_COMMAND_UI(ID_VIEW_BL3, OnUpdateViewBL3)
	ON_UPDATE_COMMAND_UI(ID_VIEW_UL, OnUpdateViewUL)
	ON_COMMAND(ID_SELECT_BL1, OnSelectBL1)
	ON_COMMAND(ID_SELECT_BL2, OnSelectBL2)
	ON_COMMAND(ID_SELECT_BL3, OnSelectBL3)
	ON_COMMAND(ID_SELECT_UL, OnSelectUL)
	ON_UPDATE_COMMAND_UI(ID_SELECT_BL1, OnUpdateSelectBL1)
	ON_UPDATE_COMMAND_UI(ID_SELECT_BL2, OnUpdateSelectBL2)
	ON_UPDATE_COMMAND_UI(ID_SELECT_BL3, OnUpdateSelectBL3)
	ON_UPDATE_COMMAND_UI(ID_SELECT_UL, OnUpdateSelectUL)
	ON_COMMAND(ID_MAPEX_DELETE, OnMapexDelete)
	ON_UPDATE_COMMAND_UI(ID_MAPEX_DELETE, OnUpdateMapexDelete)
	ON_COMMAND(ID_MAP_GRID, OnMapGrid)
	ON_UPDATE_COMMAND_UI(ID_MAP_GRID, OnUpdateMapGrid)
	ON_UPDATE_COMMAND_UI(ID_GRIDSIZE_1, OnUpdateGridSize1)
	ON_UPDATE_COMMAND_UI(ID_GRIDSIZE_2, OnUpdateGridSize2)
	ON_UPDATE_COMMAND_UI(ID_GRIDSIZE_4, OnUpdateGridSize4)
	ON_UPDATE_COMMAND_UI(ID_GRIDSIZE_8, OnUpdateGridSize8)
	ON_UPDATE_COMMAND_UI(ID_GRIDSIZE_16, OnUpdateGridSize16)
	ON_COMMAND(ID_GRIDSIZE_1, OnGridSize1)
	ON_COMMAND(ID_GRIDSIZE_2, OnGridSize2)
	ON_COMMAND(ID_GRIDSIZE_4, OnGridSize4)
	ON_COMMAND(ID_GRIDSIZE_8, OnGridSize8)
	ON_COMMAND(ID_GRIDSIZE_16, OnGridSize16)
	ON_COMMAND(ID_MAP_LANDTYPES, OnMapLandTypes)
	ON_UPDATE_COMMAND_UI(ID_MAP_LANDTYPES, OnUpdateMapLandTypes)
	ON_COMMAND(ID_MINIMAP_ZOOMIN, OnMinimapZoomin)
	ON_UPDATE_COMMAND_UI(ID_MINIMAP_ZOOMIN, OnUpdateMinimapZoomin)
	ON_COMMAND(ID_MINIMAP_ZOOMOUT, OnMinimapZoomout)
	ON_UPDATE_COMMAND_UI(ID_MINIMAP_ZOOMOUT, OnUpdateMinimapZoomout)
	ON_UPDATE_COMMAND_UI(ID_EDIT_COPY, OnUpdateEditCopy)
	ON_COMMAND(ID_EDIT_COPY, OnEditCopy)
	ON_UPDATE_COMMAND_UI(ID_EDIT_CUT, OnUpdateEditCut)
	ON_COMMAND(ID_EDIT_CUT, OnEditCut)
	ON_UPDATE_COMMAND_UI(ID_EDIT_PASTE, OnUpdateEditPaste)
	ON_COMMAND(ID_EDIT_PASTE, OnEditPaste)
	ON_UPDATE_COMMAND_UI(ID_MAPEX_ZPOS, OnUpdateMapexZPos)
	ON_COMMAND(ID_MAPEX_ZPOS, OnMapexZPos)
	ON_COMMAND(ID_UNIT_CIVILIZATIONS, OnUnitCivilizations)
	ON_COMMAND(ID_UNIT_DELETELIBRARY, OnUnitDeleteLibrary)
	ON_COMMAND(ID_UNIT_EDITLIBRARY, OnUnitEditLibrary)
	ON_COMMAND(ID_UNIT_NEWLIBRARY, OnUnitNewLibrary)
	ON_COMMAND(ID_UNIT_OPENLIBRARY, OnUnitOpenLibrary)
	ON_COMMAND(ID_UNIT_EDITOR, OnUnitEditor)
	ON_COMMAND(ID_UNIT_DIRECTION_LEFT, OnUnitDirectionLeft)
	ON_UPDATE_COMMAND_UI(ID_UNIT_DIRECTION_LEFT, OnUpdateUnitDirectionLeft)
	ON_COMMAND(ID_UNIT_DIRECTION_RANDOM, OnUnitDirectionRandom)
	ON_UPDATE_COMMAND_UI(ID_UNIT_DIRECTION_RANDOM, OnUpdateUnitDirectionRandom)
	ON_COMMAND(ID_UNIT_DIRECTION_RIGHT, OnUnitDirectionRight)
	ON_UPDATE_COMMAND_UI(ID_UNIT_DIRECTION_RIGHT, OnUpdateUnitDirectionRight)
	ON_COMMAND(ID_FILE_RESOURCES, OnFileResources)
	ON_UPDATE_COMMAND_UI(ID_MAPEX_UP, OnUpdateMapexUp)
	ON_COMMAND(ID_MAPEX_UP, OnMapexUp)
	ON_UPDATE_COMMAND_UI(ID_MAPEX_DOWN, OnUpdateMapexDown)
	ON_COMMAND(ID_MAPEX_DOWN, OnMapexDown)
	ON_COMMAND(ID_FILE_INVISIBILITY, OnFileInvisibility)
	ON_COMMAND(ID_FILE_SCRIPTSET, OnFileScriptSet)
	ON_COMMAND(ID_FILE_FINDPATHGRAPHS, OnFileFindPathGraphs)
	ON_COMMAND(ID_FILE_SETMAPSIZE, OnFileSetMapSize)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMapEditorDoc construction/destruction

CMapEditorDoc::CMapEditorDoc()
{
  m_pSelectedMapex = NULL;
  m_pSelectedULInstance = NULL;
  m_pSelectedUnit = NULL;
  m_nMiniMapZoom = 0;
  m_nSelectionType = Sel_None;

  m_MiniMap.Create();
}

CMapEditorDoc::~CMapEditorDoc()
{
  m_MiniMap.Delete();
}

BOOL CMapEditorDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;

  // lock whole map
  m_Map.GetExclusiveAccess();
  try{
  CNewMapDlg dlg;
  dlg.m_dwWidth = 1024;
  dlg.m_dwHeight = 1024;
  dlg.m_strName = "Moje mapa";

  if(dlg.DoModal() != IDOK){
    return FALSE;
  }

  if(bMainFrameCreated)
    ((CMainFrame *)AfxGetMainWnd())->CloseToolbars();
  m_Map.Delete();
  m_Map.SetMiniMap(&m_MiniMap);

  AppSetActiveMap(&m_Map);
  m_Map.Create(dlg.m_dwWidth, dlg.m_dwHeight);
  m_Map.SetName(dlg.m_strName);
  m_Map.SetDescription(dlg.m_strDescription);

  m_pSelectedMapex = NULL;
  m_Map.SetSelection(&m_Selection);

  CMainFrame *pFrame = (CMainFrame *)AfxGetMainWnd();
  if((pFrame != NULL) && (pFrame->IsKindOf(RUNTIME_CLASS(CMainFrame)))){
    pFrame->OnNewMap(&m_Map);
  }

  m_nSelectionType = Sel_None;

  if(m_Map.IsModified()){
    SetModifiedFlag();
    m_Map.SetModified(FALSE);
  }

  CRect rcMap;
  rcMap.left = 0; rcMap.top = 0;
  rcMap.right = 200 * m_MiniMap.GetZoom();
  rcMap.bottom = 200 * m_MiniMap.GetZoom();
  m_Map.SetMiniMapRect(rcMap);
  m_MiniMap.SetMap(&m_Map);
  m_MiniMap.SetZoom(1);

  // unlock the map
  m_Map.ReleaseExclusiveAccess();
  }
  catch(...){
    m_Map.ReleaseExclusiveAccess();
    if(bMainFrameCreated)
      ((CMainFrame *)AfxGetMainWnd())->OpenToolbars();
    throw;
  }

  if(bMainFrameCreated)
    ((CMainFrame *)AfxGetMainWnd())->OpenToolbars();
	return TRUE;
}

BOOL CMapEditorDoc::OnOpenDocument(LPCTSTR lpszPathName) 
{
  CArchiveFile MapFile;
  
  // lock the map
  m_Map.GetExclusiveAccess();

  CFinishDlg progdlg;
  progdlg.Create(IDD_FINISH_PROGRESS, NULL);
  progdlg.ShowWindow(SW_SHOW);
  progdlg.UpdateWindow();
  progdlg.SetWindowText("Otevírání mapy");

  try{
  progdlg.SetText("Zavírání mapy...");

  // close toolbars
  if(bMainFrameCreated)
    ((CMainFrame *)AfxGetMainWnd())->CloseToolbars();

  m_Map.Delete(&progdlg.m_wndProgress);
  m_Map.SetMiniMap(&m_MiniMap);
  AppSetActiveMap(&m_Map);
  MapFile = CDataArchive::GetRootArchive()->CreateFile(lpszPathName);
  progdlg.SetText("Otevírání mapy...");
  if(!m_Map.Create(MapFile, &progdlg.m_wndProgress)) return FALSE;
  progdlg.DestroyWindow();
  m_Map.SetSelection(&m_Selection);
  
  CMainFrame *pFrame = (CMainFrame *)AfxGetMainWnd();
  if(pFrame->IsKindOf(RUNTIME_CLASS(CMainFrame))){
    pFrame->OnNewMap(&m_Map);
  }

  m_pSelectedMapex = NULL;
  m_nSelectionType = Sel_None;

  if(m_Map.IsModified()){
    SetModifiedFlag();
    m_Map.SetModified(FALSE);
  }

  CRect rcMap;
  rcMap.left = 0; rcMap.top = 0;
  rcMap.right = 200 * m_MiniMap.GetZoom();
  rcMap.bottom = 200 * m_MiniMap.GetZoom();
  m_Map.SetMiniMapRect(rcMap);
  m_MiniMap.SetMap(&m_Map);
  m_MiniMap.SetZoom(1);

  // release the map
  m_Map.ReleaseExclusiveAccess();
  }
  catch(...){
    m_Map.ReleaseExclusiveAccess();
    if(bMainFrameCreated)
      ((CMainFrame *)AfxGetMainWnd())->OpenToolbars();
    throw;
  }

  if(bMainFrameCreated)
    ((CMainFrame *)AfxGetMainWnd())->OpenToolbars();
	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// CMapEditorDoc diagnostics

#ifdef _DEBUG
void CMapEditorDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void CMapEditorDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CMapEditorDoc commands

void CMapEditorDoc::OnMapexEditor() 
{
  CMapexEditorDlg MapexEditorDlg;

  m_Map.GetExclusiveAccess();
  try{
  MapexEditorDlg.Create(&m_Map);

  MapexEditorDlg.DoModal();

  // we have to save libraries (mapex graphics could changed)
  // so we have to use the new one
  m_Map.SaveLibraries();

  m_Map.ReleaseExclusiveAccess();
  }
  catch(...){
    m_Map.ReleaseExclusiveAccess();
    throw;
  }

  UpdateAllViews(NULL);
}

BOOL CMapEditorDoc::OnSaveDocument(LPCTSTR lpszPathName) 
{
  CArchiveFile MapFile;

  // Check the map
  if(!m_Map.CheckValid()){
    AfxMessageBox("Mapa není konzistentní.\nOpravte pøedchozí chybu a uložte mapu znovu");
    return FALSE;
  }

  MapFile = CDataArchive::GetRootArchive()->CreateFile(lpszPathName, CArchiveFile::modeWrite | CArchiveFile::modeCreate | CArchiveFile::modeUncompressed);
  m_Map.SaveMap(MapFile);

  SetModifiedFlag(FALSE);
  m_Map.SetModified(FALSE);

  return TRUE;
}

void CMapEditorDoc::OnFileProperties() 
{
  CMapPropertiesDlg dlg;
  
  dlg.m_strFileName = GetPathName();
  dlg.m_dwHeight = m_Map.GetHeight();
  dlg.m_dwWidth = m_Map.GetWidth();
  dlg.m_strMapName = m_Map.GetName();
  dlg.m_strMapDescription = m_Map.GetDescription();

  if(dlg.DoModal() == IDOK){
    m_Map.SetName(dlg.m_strMapName);
    m_Map.SetDescription(dlg.m_strMapDescription);

    SetModifiedFlag();
  }
}

void CMapEditorDoc::OnMapexNewLibrary() 
{
	CFileDialog dlg(FALSE, "*.mlb", NULL, OFN_HIDEREADONLY |
    OFN_OVERWRITEPROMPT, "Knihovny mapexù (*.mlb)|*.mlb|Všechny soubory|*.*||",
    AfxGetMainWnd());

  // ask for the library file
  if(dlg.DoModal() != IDOK)
    return;

  CString strLibFile = dlg.GetPathName();

  m_Map.GetExclusiveAccess();

  try{
    // create new library
    CEMapexLibrary *pLibrary;
    pLibrary = m_Map.NewMapexLibrary(strLibFile);

    // edit it (name ...)
    CMapexLibraryEditDlg libdlg;
    libdlg.Create(pLibrary);

    libdlg.DoModal();

    SetModifiedFlag();

    m_Map.ReleaseExclusiveAccess();
  }
  catch(...){
    m_Map.ReleaseExclusiveAccess();
    throw;
  }
}

void CMapEditorDoc::OnMapexOpenLibrary() 
{
	CFileDialog dlg(TRUE, "*.mlb", NULL, OFN_HIDEREADONLY | OFN_PATHMUSTEXIST,
    "Knihovny mapxù (*.mlb)|*.mlb|Všechny soubory|*.*||", AfxGetMainWnd());

  if(dlg.DoModal() != IDOK){
    return;
  }

  CString strLibFile = dlg.GetPathName();

  m_Map.GetExclusiveAccess();
  try{

  POSITION pos;
  CEMapexLibrary *pMapexLibrary;
  pos = m_Map.GetFirstMapexLibraryPosition();
  while(pos != NULL){
    pMapexLibrary = m_Map.GetNextMapexLibrary(pos);
    if(strLibFile.CompareNoCase(pMapexLibrary->GetFullPath()) == 0){
      // library allready loaded
      AfxMessageBox("Tato knohivna je již otevøená.");
      return;
    }
  }
  m_Map.OpenMapexLibrary(strLibFile);
  m_Map.ReleaseExclusiveAccess();
  }
  catch(...){
    m_Map.ReleaseExclusiveAccess();
    throw;
  }

  SetModifiedFlag();
}

void CMapEditorDoc::OnMapexEditLibrary() 
{
  CEditMapexLibraryDlg dlg;
  
  dlg.Create(&m_Map);

  dlg.DoModal();
}

void CMapEditorDoc::OnMapexDeleteLibrary() 
{
	CDeleteMapexLibraryDlg dlg;

  m_Map.GetExclusiveAccess();
  try{
  dlg.Create(&m_Map);

  dlg.DoModal();
  m_Map.ReleaseExclusiveAccess();
  }
  catch(...){
    m_Map.ReleaseExclusiveAccess();
  }
}

void CMapEditorDoc::OnFileSetLandTypes() 
{
	CFileDialog dlg(TRUE, "*.landtypes", NULL, OFN_HIDEREADONLY | OFN_PATHMUSTEXIST,
    "Typy povrchù (*.landtypes)|*.landtypes|Všechny soubory|*.*||", AfxGetMainWnd());

  if(dlg.DoModal() != IDOK)
    return;

  CArchiveFile file;
  file = CDataArchive::GetRootArchive()->CreateFile(dlg.GetPathName());
  m_Map.LoadLandTypes(file);

  SetModifiedFlag();
}

CEMap * CMapEditorDoc::GetMap()
{
  return &m_Map;
}

void CMapEditorDoc::SetSelectedMapex(CEMapex *pMapex)
{
  m_pSelectedMapex = pMapex;

  // set the selected unit to NULL
  m_pSelectedUnitType = NULL;
  m_pSelectedCivilization = NULL;
  
  // set the drawing description text
  CMainFrame *pFrame = (CMainFrame *)AfxGetMainWnd();
  pFrame->ResetUnitDrawingSelection();
  if(pMapex == NULL){
    pFrame->SetDrawingMapexPaneText("");
  }
  else{
    pFrame->SetDrawingMapexPaneText(pMapex->GetName());
  }
}

CEMapex * CMapEditorDoc::GetSelectedMapex()
{
  return m_pSelectedMapex;
}

void CMapEditorDoc::SetSelectedUnitType(CEUnitType *pUnitType, CECivilization *pCiv)
{
  m_pSelectedUnitType = pUnitType;
  m_pSelectedCivilization = pCiv;
  
  // set selected mapex to NULL
  m_pSelectedMapex = NULL;

  // set new drawing description
  CMainFrame *pFrame = (CMainFrame *)AfxGetMainWnd();
  pFrame->ResetMapexDrawingSelection();
  if(pUnitType == NULL){
    pFrame->SetDrawingMapexPaneText("");
  }
  else{
    pFrame->SetDrawingMapexPaneText(pUnitType->GetName());
  }
}

CEMapexInstance * CMapEditorDoc::AddMapexInstance(CPoint ptPos, CEMapex *pMapex, int nLevel)
{
  if(pMapex == NULL) return NULL;
  CEMapSquare *pSquare = m_Map.GetMapSquareFromCell(ptPos.x, ptPos.y);
  CEMapexInstance *pMapexInstance = new CEMapexInstance();
  pMapexInstance->Create(pMapex, ptPos.x, ptPos.y);
  m_Map.LockSquare(pSquare);
  pSquare->AddBLMapexInstance(nLevel, pMapexInstance);
  m_Map.ReleaseSquare(pSquare);

  SetModifiedFlag();
  UpdateAllViews(NULL, Update_MapexInstance, (CObject *)(void *)pMapexInstance);
  CRect rcMapex;
  GetMapexInstanceRect(pMapexInstance, &rcMapex);
  m_MiniMap.UpdateRect(rcMapex);

  return pMapexInstance;
}

CEULMapexInstance * CMapEditorDoc::AddULMapexInstance(CPoint ptPos, CEMapex *pMapex)
{
  if(pMapex == NULL) return NULL;
  CEMapSquare *pSquare = m_Map.GetMapSquareFromCell(ptPos.x, ptPos.y);
  CEULMapexInstance *pMapexInstance = new CEULMapexInstance();
  pMapexInstance->Create(pMapex, ptPos.x, ptPos.y, GetSelectedZPos());
  m_Map.LockSquare(pSquare);
  pSquare->AddULMapexInstance(pMapexInstance);
  m_Map.ReleaseSquare(pSquare);

  SetModifiedFlag();
  UpdateAllViews(NULL, Update_ULMapexInstance, (CObject *)(void *)pMapexInstance);
  CRect rcMapex;
  rcMapex = GetULInstanceRect(pMapexInstance);
  m_MiniMap.UpdateRect(rcMapex);

  return pMapexInstance;
}

CEUnit * CMapEditorDoc::AddUnitInstance(CPoint ptPos, CEUnitType *pUnitType, CECivilization *pCivilization)
{
  if(pUnitType == NULL) return NULL;
  if(pCivilization == NULL) return NULL;

  // create new unit object
  CEUnit *pUnit;
  pUnit = new CEUnit();

  // create it and set its position
  pUnit->Create(pUnitType, pCivilization);
  pUnit->SetPos(ptPos.x, ptPos.y, pUnitType->GetZPos());
  

  // find the map square and lock it
  CEMapSquare *pSquare = m_Map.GetMapSquareFromCell(ptPos.x, ptPos.y);
  m_Map.LockSquare(pSquare);
  
  // insert unit instance to the map
  // this will also add it to its mapsquare (the one locked)
  m_Map.InsertUnit(pUnit);

  // release the square
  m_Map.ReleaseSquare(pSquare);


  // update flags and views
  SetModifiedFlag();
  UpdateAllViews(NULL, Update_Unit, (CObject *)(void *)pUnit);

  // we wan't need to update the minimap (no units on minimap (probably))

  return pUnit;
}

void CMapEditorDoc::OnViewBL1() 
{ m_Map.m_bBL1Visible = m_Map.m_bBL1Visible ? FALSE : TRUE; UpdateAllViews(NULL); }

void CMapEditorDoc::OnViewBL2() 
{ m_Map.m_bBL2Visible = m_Map.m_bBL2Visible ? FALSE : TRUE; UpdateAllViews(NULL); }

void CMapEditorDoc::OnViewBL3() 
{ m_Map.m_bBL3Visible = m_Map.m_bBL3Visible ? FALSE : TRUE; UpdateAllViews(NULL); }

void CMapEditorDoc::OnViewUL() 
{ m_Map.m_bULVisible = m_Map.m_bULVisible ? FALSE : TRUE; UpdateAllViews(NULL); }


void CMapEditorDoc::OnUpdateViewBL1(CCmdUI* pCmdUI) 
{ pCmdUI->SetCheck(m_Map.m_bBL1Visible ? 1 : 0);
  pCmdUI->Enable(m_Map.GetSelectedLevel() != 1);}

void CMapEditorDoc::OnUpdateViewBL2(CCmdUI* pCmdUI) 
{ pCmdUI->SetCheck(m_Map.m_bBL2Visible ? 1 : 0);
  pCmdUI->Enable(m_Map.GetSelectedLevel() != 2);}

void CMapEditorDoc::OnUpdateViewBL3(CCmdUI* pCmdUI) 
{ pCmdUI->SetCheck(m_Map.m_bBL3Visible ? 1 : 0);
  pCmdUI->Enable(m_Map.GetSelectedLevel() != 3);}

void CMapEditorDoc::OnUpdateViewUL(CCmdUI* pCmdUI) 
{ pCmdUI->SetCheck(m_Map.m_bULVisible ? 1 : 0);
  pCmdUI->Enable(m_Map.GetSelectedLevel() != 4);}


void CMapEditorDoc::OnSelectBL1() 
{ m_Map.m_bBL1Visible = TRUE; m_Map.SetSelectedLevel(1); UpdateAllViews(NULL);	}

void CMapEditorDoc::OnSelectBL2() 
{ m_Map.m_bBL2Visible = TRUE; m_Map.SetSelectedLevel(2); UpdateAllViews(NULL);	}

void CMapEditorDoc::OnSelectBL3() 
{ m_Map.m_bBL3Visible = TRUE; m_Map.SetSelectedLevel(3); UpdateAllViews(NULL);	}

void CMapEditorDoc::OnSelectUL() 
{ m_Map.m_bULVisible = TRUE; m_Map.SetSelectedLevel(4); UpdateAllViews(NULL);	}


void CMapEditorDoc::OnUpdateSelectBL1(CCmdUI* pCmdUI) 
{ pCmdUI->SetCheck((m_Map.GetSelectedLevel() == 1) ? 1 : 0); }

void CMapEditorDoc::OnUpdateSelectBL2(CCmdUI* pCmdUI) 
{ pCmdUI->SetCheck((m_Map.GetSelectedLevel() == 2) ? 1 : 0); }

void CMapEditorDoc::OnUpdateSelectBL3(CCmdUI* pCmdUI) 
{ pCmdUI->SetCheck((m_Map.GetSelectedLevel() == 3) ? 1 : 0); }

void CMapEditorDoc::OnUpdateSelectUL(CCmdUI* pCmdUI) 
{ pCmdUI->SetCheck((m_Map.GetSelectedLevel() == 4) ? 1 : 0); }

void CMapEditorDoc::AddSelectedMapexInstance(CEMapexInstance *pMapexInstance, BOOL bRevert)
{
  UpdateSelection();
  if(m_nSelectionType != Sel_BLMapexInstance){
    ClearSelection();
  }
  m_nSelectionType = Sel_BLMapexInstance;

  if(bRevert){
    if(m_Selection.Includes(pMapexInstance)) m_Selection.RemoveInstance(pMapexInstance);
    else m_Selection.AddInstance(pMapexInstance);
  }
  else{
    m_Selection.AddInstance(pMapexInstance);
  }
  CMainFrame *pFrame = (CMainFrame *)AfxGetMainWnd();
  pFrame->SetSelectedMapexPaneText(m_Selection.GetStatusBarText(&m_Map));
  UpdateAllViews(NULL, Update_Window, NULL); // update now -> no whole screen painting (the prev and the new one -> rect around == whole screen)
  UpdateSelection();
}

void CMapEditorDoc::SetSelectedULMapexInstance(CEULMapexInstance *pInstance)
{
  UpdateSelection();

  ClearSelection();
  m_nSelectionType = Sel_ULMapexInstance;

  m_pSelectedULInstance = pInstance;
  CString str;
  if(pInstance != NULL){
    str.Format("4 - %s", pInstance->GetMapex()->GetName());
  }
  else{
    str.Empty();
  }
  CMainFrame *pFrame = (CMainFrame *)AfxGetMainWnd();
  pFrame->SetSelectedMapexPaneText(str);
  if(pInstance != NULL) pFrame->SetZPosComboValue(pInstance->GetZPosition());
  UpdateAllViews(NULL, Update_Window, NULL); // update now -> no whole screen painting (the prev and the new one -> rect around == whole screen)
  m_Map.SetULSelection(m_pSelectedULInstance);
  UpdateSelection();
}

void CMapEditorDoc::SetSelectedUnit(CEUnit *pUnit)
{
  // update all old selection
  UpdateSelection();

  // clear the selection and set it for units
  ClearSelection();
  m_nSelectionType = Sel_Unit;

  // set the selected unit
  m_pSelectedUnit = pUnit;

  // construct the unit string
  CString str;
  if(pUnit != NULL){
    str.Format("U - %s", pUnit->GetUnitType()->GetName());
  }
  else{
    str.Empty();
  }

  // update frame status bar and toolbar
  CMainFrame *pFrame = (CMainFrame *)AfxGetMainWnd();
  pFrame->SetSelectedMapexPaneText(str);
  if(pUnit != NULL) pFrame->SetZPosComboValue(pUnit->GetZPos());

  // update views and selections
  UpdateAllViews(NULL, Update_Window, NULL); // update now -> no whole screen painting
  m_Map.SetUnitSelection(m_pSelectedUnit);
  UpdateSelection();
}

void CMapEditorDoc::ClearSelection()
{
  switch(m_nSelectionType){
  case Sel_None:
    break;
  case Sel_BLMapexInstance:
    {
      // update now -> the drawing will occure after rturn from this function
      // so everything will be OK
      UpdateSelection();
      m_Selection.Clear();
      CMainFrame *pFrame = (CMainFrame *)AfxGetMainWnd();
      pFrame->SetSelectedMapexPaneText(m_Selection.GetStatusBarText(&m_Map));
    }
    break;
  case Sel_ULMapexInstance:
    {
      UpdateSelection();
      m_pSelectedULInstance = NULL;
      m_Map.SetULSelection(NULL);
    }
    break;
  case Sel_Unit:
    {
      UpdateSelection();
      m_pSelectedUnit = NULL;
      m_Map.SetUnitSelection(NULL);
    }
    break;
  }
  m_nSelectionType = Sel_None;
}

void CMapEditorDoc::OnMapexDelete() 
{
  switch(m_nSelectionType){
  case Sel_None:
    return;
  case Sel_BLMapexInstance:
    {
      CRect rcUpdate = m_Selection.GetBoundingRect();

      UpdateSelection();
      m_Selection.DeleteSelection(&m_Map);
      CMainFrame *pFrame = (CMainFrame *)AfxGetMainWnd();
      pFrame->SetSelectedMapexPaneText(m_Selection.GetStatusBarText(&m_Map));

      m_MiniMap.UpdateRect(rcUpdate);
      SetModifiedFlag();
    }
    break;
  case Sel_ULMapexInstance:
    {
      UpdateSelection();
      UpdateMiniMapSelection();

      CEMapSquare *pSquare;
      CEULMapexInstance *pInst = m_pSelectedULInstance;
      pSquare = m_Map.GetMapSquareFromCell(m_pSelectedULInstance->GetXPosition(), m_pSelectedULInstance->GetYPosition());
      m_Map.LockSquare(pSquare);
      pSquare->DeleteULMapexInstance(m_pSelectedULInstance);
      m_Map.ReleaseSquare(pSquare);
      ClearSelection();
      delete pInst;
    }
    break;
  case Sel_Unit:
    {
      UpdateSelection();

      CEMapSquare *pSquare;
      CEUnit *pUnit = m_pSelectedUnit;
      pSquare = m_Map.GetMapSquareFromCell(m_pSelectedUnit->GetXPos(), m_pSelectedUnit->GetYPos());
      m_Map.LockSquare(pSquare);
      m_Map.DeleteUnit(pUnit);
      m_Map.ReleaseSquare(pSquare);
      ClearSelection();

      pUnit->Delete();
      delete pUnit;
    }
    break;
  }
}

void CMapEditorDoc::OnUpdateMapexDelete(CCmdUI* pCmdUI) 
{
  switch(m_nSelectionType){
  case Sel_None: pCmdUI->Enable(FALSE); break;
  case Sel_BLMapexInstance: pCmdUI->Enable(!m_Selection.IsEmpty()); break;
  case Sel_ULMapexInstance: pCmdUI->Enable(m_pSelectedULInstance != NULL); break;
  case Sel_Unit: break;
  }
}

CEMapexInstance * CMapEditorDoc::GetMapexFromCell(DWORD dwX, DWORD dwY, CEMapexInstance *pPrevSel)
{
  return m_Map.GetMapexFromCell(dwX, dwY, pPrevSel);
}

CEULMapexInstance * CMapEditorDoc::GetULMapexFromCell(DWORD dwX, DWORD dwY)
{
  return m_Map.GetULMapexFromCell(dwX, dwY);
}

CEUnit * CMapEditorDoc::GetUnitFromCell(DWORD dwX, DWORD dwY)
{
  return m_Map.GetUnitFromCell(dwX, dwY);
}

void CMapEditorDoc::OnMapGrid() 
{
  if(m_Map.GetDrawGrid()){
    m_Map.SetDrawGrid(FALSE);
  }
  else{
    m_Map.SetDrawGrid(TRUE);
  }
  UpdateAllViews(NULL);
}

void CMapEditorDoc::OnUpdateMapGrid(CCmdUI* pCmdUI) 
{
  pCmdUI->SetCheck(m_Map.GetDrawGrid());	
}

void CMapEditorDoc::OnUpdateGridSize1(CCmdUI* pCmdUI) 
{ pCmdUI->SetRadio(m_Map.GetGridSize() == 1); }

void CMapEditorDoc::OnUpdateGridSize2(CCmdUI* pCmdUI) 
{ pCmdUI->SetRadio(m_Map.GetGridSize() == 2); }

void CMapEditorDoc::OnUpdateGridSize4(CCmdUI* pCmdUI) 
{ pCmdUI->SetRadio(m_Map.GetGridSize() == 4); }

void CMapEditorDoc::OnUpdateGridSize8(CCmdUI* pCmdUI) 
{ pCmdUI->SetRadio(m_Map.GetGridSize() == 8); }

void CMapEditorDoc::OnUpdateGridSize16(CCmdUI* pCmdUI) 
{ pCmdUI->SetRadio(m_Map.GetGridSize() == 16); }


void CMapEditorDoc::OnGridSize1() 
{ m_Map.SetGridSize(1); UpdateAllViews(NULL); }

void CMapEditorDoc::OnGridSize2() 
{ m_Map.SetGridSize(2); UpdateAllViews(NULL); }

void CMapEditorDoc::OnGridSize4() 
{ m_Map.SetGridSize(4); UpdateAllViews(NULL); }

void CMapEditorDoc::OnGridSize8() 
{ m_Map.SetGridSize(8); UpdateAllViews(NULL); }

void CMapEditorDoc::OnGridSize16() 
{ m_Map.SetGridSize(16); UpdateAllViews(NULL); }


void CMapEditorDoc::OnMapLandTypes() 
{ if(m_Map.GetDrawLandTypes()) m_Map.SetDrawLandTypes(FALSE); else m_Map.SetDrawLandTypes(TRUE);
  UpdateAllViews(NULL); }

void CMapEditorDoc::OnUpdateMapLandTypes(CCmdUI* pCmdUI) 
{ pCmdUI->SetCheck(m_Map.GetDrawLandTypes() ? 1 : 0); }

CMiniMap * CMapEditorDoc::GetMiniMap()
{
  return &m_MiniMap;
}

void CMapEditorDoc::GetMapexInstanceRect(CEMapexInstance *pMapexInstance, CRect *pRect)
{
  pRect->left = pMapexInstance->GetXPosition();
  pRect->top = pMapexInstance->GetYPosition();
  CEMapex *pMapex = pMapexInstance->GetMapex();
  pRect->right = pRect->left + pMapex->GetSize().cx;
  pRect->bottom = pRect->top + pMapex->GetSize().cy;
}

CRect CMapEditorDoc::GetULInstanceRect(CEULMapexInstance *pInstance)
{
  CRect rcMapex;
  if(pInstance == NULL){ rcMapex.SetRectEmpty(); return rcMapex; }
  rcMapex.left = pInstance->GetXPosition();
  rcMapex.top = pInstance->GetYPosition();
  CEMapex *pMapex = pInstance->GetMapex();
  rcMapex.right = rcMapex.left + pMapex->GetSize().cx;
  rcMapex.bottom = rcMapex.top + pMapex->GetSize().cy;
  return rcMapex;
}

CRect CMapEditorDoc::GetUnitInstanceRect(CEUnit *pInstance)
{
  CRect rcUnit;
  if(pInstance == NULL){ rcUnit.SetRectEmpty(); return rcUnit; }
  DWORD dwHlp = pInstance->GetUnitType()->GetMoveWidth(0);

  rcUnit.left = pInstance->GetXPos() - (dwHlp + 1) / 2;
  rcUnit.top = pInstance->GetYPos() - (dwHlp + 1) / 2;
  rcUnit.right = rcUnit.left + dwHlp;
  rcUnit.bottom = rcUnit.top + dwHlp;

  return rcUnit;
}

void CMapEditorDoc::OnMinimapZoomin() 
{
  CRect rcMap = m_Map.GetMiniMapRect();
  m_nMiniMapZoom--;
  if(m_nMiniMapZoom < 0) m_nMiniMapZoom = 0;
  DWORD dwZoom = g_aMiniMapZooms[m_nMiniMapZoom];

  CPoint ptCenter((rcMap.left + rcMap.right)/2, (rcMap.top + rcMap.bottom)/2);
  rcMap.left = ptCenter.x - 100 * dwZoom;
  rcMap.right = ptCenter.x + 100 * dwZoom;
  rcMap.top = ptCenter.y - 100 * dwZoom;
  rcMap.bottom = ptCenter.y + 100 * dwZoom;

//  m_Map.GetExclusiveAccess();
  m_Map.SetMiniMapRect(rcMap);
  m_MiniMap.SetZoom(dwZoom);
//  m_Map.ReleaseExclusiveAccess();
}

void CMapEditorDoc::OnUpdateMinimapZoomin(CCmdUI* pCmdUI) 
{ pCmdUI->Enable((m_nMiniMapZoom > 0) ? 1 : 0); }

void CMapEditorDoc::OnMinimapZoomout() 
{
  CRect rcMap = m_Map.GetMiniMapRect();
  m_nMiniMapZoom++;
  if(m_nMiniMapZoom >= (g_nMiniMapZooms - 1)) m_nMiniMapZoom = g_nMiniMapZooms - 1;
  DWORD dwZoom = g_aMiniMapZooms[m_nMiniMapZoom];

  CPoint ptCenter((rcMap.left + rcMap.right)/2, (rcMap.top + rcMap.bottom)/2);
  rcMap.left = ptCenter.x - 100 * dwZoom;
  rcMap.right = ptCenter.x + 100 * dwZoom;
  rcMap.top = ptCenter.y - 100 * dwZoom;
  rcMap.bottom = ptCenter.y + 100 * dwZoom;

  m_Map.SetMiniMapRect(rcMap);
  m_MiniMap.SetZoom(dwZoom);
}

void CMapEditorDoc::OnUpdateMinimapZoomout(CCmdUI* pCmdUI) 
{ pCmdUI->Enable((m_nMiniMapZoom < (g_nMiniMapZooms - 1)) ? 1 : 0); }

void CMapEditorDoc::SetVisibleRect(CRect rcRect)
{
  m_Map.SetVisibleRect(rcRect);
  rcRect = m_Map.GetVisibleRect();

  POSITION pos;
  pos = GetFirstViewPosition();
  CMapEditorView *pView = (CMapEditorView *)GetNextView(pos);

  pView->SetVisibleRect(rcRect);
}

void CMapEditorDoc::SetSelectionPosition(DWORD dwX, DWORD dwY)
{
  switch(m_nSelectionType){
  case Sel_None: return;
  case Sel_BLMapexInstance:
    {
      if(m_Selection.IsEmpty()) return;
      
      if(dwX >= m_Map.GetWidth()) dwX = m_Map.GetWidth() - 1;
      if(dwY >= m_Map.GetHeight()) dwY = m_Map.GetHeight() - 1;
      
      CRect rcSel = m_Selection.GetBoundingRect();
      // if no position change -> do nothing
      if((rcSel.left == (int)dwX) && (rcSel.top == (int)dwY)) return;
      
      // update previous position (this will erase the old parts)
      UpdateAllViews(NULL, Update_Selection, (CObject *)(void *)&m_Selection);
      m_MiniMap.UpdateRect(rcSel);
      
      // set new position
      m_Selection.SetPosition(dwX, dwY, &m_Map);
      
      // update the new position
      UpdateAllViews(NULL, Update_Selection, (CObject *)(void *)&m_Selection);
      rcSel = m_Selection.GetBoundingRect();
      m_MiniMap.UpdateRect(rcSel);
      SetModifiedFlag(TRUE);
    }
    break;
  case Sel_ULMapexInstance:
    {
      if(m_pSelectedULInstance == NULL) return;

      if(dwX >= m_Map.GetWidth()) dwX = m_Map.GetWidth() - 1;
      if(dwY >= m_Map.GetHeight()) dwY = m_Map.GetHeight() - 1;

      CRect rcSel = GetULInstanceRect(m_pSelectedULInstance);
      // if no position change -> do nothing
      if((rcSel.left == (int)dwX) && (rcSel.top == (int)dwY)) return;

      UpdateSelection();
      m_MiniMap.UpdateRect(rcSel);

      CEULMapexInstance *pInstance = m_pSelectedULInstance;
      CEMapSquare *pOldSquare, *pNewSquare;
      
      pOldSquare = m_Map.GetMapSquareFromCell(pInstance->GetXPosition(), pInstance->GetYPosition());
      pNewSquare = m_Map.GetMapSquareFromCell(dwX, dwY);

      m_Map.LockSquare(pOldSquare);

      if(pNewSquare != pOldSquare){
        m_Map.LockSquare(pNewSquare);

        pOldSquare->DeleteULMapexInstance(pInstance);
        pNewSquare->AddULMapexInstance(pInstance);
        pInstance->SetPosition(dwX, dwY);
        m_Map.ReleaseSquare(pNewSquare);
      }
      else{
        pInstance->SetPosition(dwX, dwY);
      }

      m_Map.ReleaseSquare(pOldSquare);

      UpdateSelection();
      UpdateMiniMapSelection();
      SetModifiedFlag(TRUE);
    }
    break;
  case Sel_Unit:
    {
      if(m_pSelectedUnit == NULL) return;

      DWORD dwHlp = m_pSelectedUnit->GetUnitType()->GetMoveWidth(0);
      dwX = dwX + (dwHlp + 1) / 2;
      dwY = dwY + (dwHlp + 1) / 2;

      if(dwX >= m_Map.GetWidth()) dwX = m_Map.GetWidth() - 1;
      if(dwY >= m_Map.GetHeight()) dwY = m_Map.GetHeight() - 1;

      CRect rcSel = GetUnitInstanceRect(m_pSelectedUnit);
      // if no position change -> do nothing
      if((rcSel.left == (int)dwX) && (rcSel.top == (int)dwY)) return;

      UpdateSelection();
      m_MiniMap.UpdateRect(rcSel);

      CEUnit *pInstance = m_pSelectedUnit;
      CEMapSquare *pOldSquare, *pNewSquare;
      
      pOldSquare = m_Map.GetMapSquareFromCell(pInstance->GetXPos(), pInstance->GetYPos());
      pNewSquare = m_Map.GetMapSquareFromCell(dwX, dwY);

      m_Map.LockSquare(pOldSquare);

      if(pNewSquare != pOldSquare){
        m_Map.LockSquare(pNewSquare);

        pOldSquare->DeleteUnit(pInstance);
        pNewSquare->AddUnit(pInstance);
        pInstance->SetPos(dwX, dwY, pInstance->GetZPos());
        m_Map.ReleaseSquare(pNewSquare);
      }
      else{
        pInstance->SetPos(dwX, dwY, pInstance->GetZPos());
      }

      m_Map.ReleaseSquare(pOldSquare);

      UpdateSelection();
      SetModifiedFlag(TRUE);    
    }
    break;
  }
}

void CMapEditorDoc::SetRectSelection(CRect rcSelection)
{
  CRect rcSel = m_Map.GetSelectionRectangle();
  rcSel.UnionRect(&rcSel, &rcSelection);

  if(m_nSelectionType != Sel_BLMapexInstance){
    UpdateSelection();
    ClearSelection();
    m_nSelectionType = Sel_BLMapexInstance;
  }

  // update the selection rectangle
  if(m_Map.SetSelectionRectangle(&rcSelection)){
    UpdateAllViews(NULL, Update_SelectionRect, (CObject *)(void *)&rcSel);
  }

  rcSel = m_Selection.GetBoundingRect();
  if(m_Selection.SetRect(rcSelection, &m_Map)){
    UpdateAllViews(NULL, Update_SelectionRect, (CObject *)(void *)&rcSel);
    UpdateAllViews(NULL, Update_Selection, (CObject *)(void *)&m_Selection);
  }
}

void CMapEditorDoc::DoneRectSelection()
{
  CRect rcSel = m_Map.GetSelectionRectangle();

  m_Map.SetSelectionRectangle(NULL);
  UpdateAllViews(NULL, Update_SelectionRect, (CObject *)(void *)&rcSel);
}

void CMapEditorDoc::OnUpdateEditCopy(CCmdUI* pCmdUI) 
{
  switch(m_nSelectionType){
  case Sel_None: pCmdUI->Enable(FALSE); break;
  case Sel_BLMapexInstance: pCmdUI->Enable(!m_Selection.IsEmpty()); break;
  case Sel_ULMapexInstance: pCmdUI->Enable(FALSE); break;
  case Sel_Unit: pCmdUI->Enable(FALSE); break;
  }
}

void CMapEditorDoc::OnEditCopy() 
{
  switch(m_nSelectionType){
  case Sel_None: return;
  case Sel_BLMapexInstance:
    m_Clipboard.SetMapexInstanceSelection(&m_Selection, &m_Map);
    break;
  case Sel_ULMapexInstance:
    break;
  case Sel_Unit:
    break;
  }
}

void CMapEditorDoc::OnUpdateEditCut(CCmdUI* pCmdUI) 
{
  switch(m_nSelectionType){
  case Sel_None: pCmdUI->Enable(FALSE); break;
  case Sel_BLMapexInstance: pCmdUI->Enable(!m_Selection.IsEmpty()); break;
  case Sel_ULMapexInstance: pCmdUI->Enable(FALSE); break;
  case Sel_Unit: pCmdUI->Enable(FALSE); break;
  }
}

void CMapEditorDoc::OnEditCut() 
{
  switch(m_nSelectionType){
  case Sel_None: return;
  case Sel_BLMapexInstance:
    m_Clipboard.SetMapexInstanceSelection(&m_Selection, &m_Map);
    OnMapexDelete();  
    break;
  case Sel_ULMapexInstance:
    break;
  case Sel_Unit:
    break;
  }
}

void CMapEditorDoc::OnUpdateEditPaste(CCmdUI* pCmdUI) 
{
  pCmdUI->Enable(!m_Clipboard.IsEmpty());	
}

void CMapEditorDoc::OnEditPaste() 
{
	POSITION pos;

  CClipboard::SNode *pNode;
  CEMapexInstance *pInstance;
  CEMapSquare *pSquare;
  CEMapex *pMapex;
  CMapEditorView *pView;
  pos = GetFirstViewPosition();
  pView = (CMapEditorView *)GetNextView(pos);
  CPoint ptPos = pView->GetScrollPosition();
  ptPos.x /= 16; ptPos.y /= 16;

  UpdateSelection();
  ClearSelection();

  pos = m_Clipboard.GetFirstPosition();
  while(pos != NULL){
    pNode = m_Clipboard.GetNextPosition(pos);

    pMapex = m_Map.GetMapex(pNode->m_dwMapexID);
    if(pMapex == NULL) continue;
    pSquare = m_Map.GetMapSquareFromCell(ptPos.x + pNode->m_dwX, ptPos.y + pNode->m_dwY);
    pInstance = new CEMapexInstance();
    pInstance->Create(pMapex, ptPos.x + pNode->m_dwX, ptPos.y + pNode->m_dwY);
    m_Map.LockSquare(pSquare);
    pSquare->AddBLMapexInstance(pNode->m_dwLevel, pInstance);
    m_Map.ReleaseSquare(pSquare);

//    m_Selection.AddInstance(pInstance);
    AddSelectedMapexInstance(pInstance, FALSE);
  }

  UpdateAllViews(NULL);
  UpdateMiniMapSelection();
}

void CMapEditorDoc::OnUpdateMapexZPos(CCmdUI* pCmdUI) 
{
  switch(m_nSelectionType){
  case Sel_ULMapexInstance:
    if(m_pSelectedULInstance != NULL){
      pCmdUI->Enable(TRUE);
      break;
    }
    pCmdUI->Enable(FALSE);
    break;
  case Sel_Unit:
    if(m_pSelectedUnit != NULL){
      pCmdUI->Enable(TRUE);
      break;
    }
    pCmdUI->Enable(FALSE);
    break;
  default:
    pCmdUI->Enable(FALSE);
    break;
  }
}

void CMapEditorDoc::UpdateSelection()
{
  switch(m_nSelectionType){
  case Sel_None: break;
  case Sel_BLMapexInstance:
    UpdateAllViews(NULL, Update_Selection, (CObject *)(void *)&m_Selection);
    break;
  case Sel_ULMapexInstance:
    UpdateAllViews(NULL, Update_ULMapexInstance, (CObject *)(void *)m_pSelectedULInstance);
    break;
  case Sel_Unit:
    UpdateAllViews(NULL, Update_Unit, (CObject *)(void *)m_pSelectedUnit);
    break;
  }
}

void CMapEditorDoc::UpdateMiniMapSelection()
{
  CRect rcSel;
  switch(m_nSelectionType){
  case Sel_None: break;
  case Sel_BLMapexInstance:
    rcSel = m_Selection.GetBoundingRect();
    m_MiniMap.UpdateRect(rcSel);
    break;
  case Sel_ULMapexInstance:
    rcSel = GetULInstanceRect(m_pSelectedULInstance);
    m_MiniMap.UpdateRect(rcSel);
    break;
  case Sel_Unit:
    break;
  }
}

DWORD CMapEditorDoc::GetSelectedZPos()
{
  CMainFrame *pFrame = (CMainFrame *)AfxGetMainWnd();
  DWORD dwRet = pFrame->GetZPosComboValue();
  if(dwRet == 0) dwRet = MAPEX_DEFAULT_ZPOSITION;
  return dwRet;
}

CRect CMapEditorDoc::GetSelectionBoundingRect()
{
  CRect rc;
  rc.SetRectEmpty();
  switch(m_nSelectionType){
  case Sel_None: break;
  case Sel_BLMapexInstance:
    return m_Selection.GetBoundingRect();
    break;
  case Sel_ULMapexInstance:
    return GetULInstanceRect(m_pSelectedULInstance);
    break;
  case Sel_Unit:
    return GetUnitInstanceRect(m_pSelectedUnit);
    break;
  }
  return rc;
}

BOOL CMapEditorDoc::PtInSelection(DWORD dwX, DWORD dwY)
{
  switch(m_nSelectionType){
  case Sel_None: return FALSE;
  case Sel_BLMapexInstance:
    return m_Selection.PtInSelection(dwX, dwY);
  case Sel_ULMapexInstance:
    {
      CPoint pt(dwX, dwY);
      CRect rc = GetULInstanceRect(m_pSelectedULInstance);
      if(rc.PtInRect(pt)) return TRUE;
      else return FALSE;
    }
    break;
  case Sel_Unit:
    {
      CPoint pt(dwX, dwY);
      CRect rc = GetUnitInstanceRect(m_pSelectedUnit);
      if(rc.PtInRect(pt)) return TRUE;
      else return FALSE;
    }
    return FALSE;
  }
  return FALSE;
}

int CMapEditorDoc::GetSelectionType()
{
  return m_nSelectionType;
}

void CMapEditorDoc::OnMapexZPos() 
{
}

void CMapEditorDoc::OnChangedMapexZPos(DWORD dwZPos)
{
  switch(m_nSelectionType){
  case Sel_None:
  case Sel_BLMapexInstance:
    return;
  case Sel_ULMapexInstance:
    {
      if(m_pSelectedULInstance == NULL) break;
      
      if(dwZPos != 0){
        m_pSelectedULInstance->SetZPos(dwZPos);
      }
      UpdateSelection();
    }
    break;
  case Sel_Unit:
    {
      if(m_pSelectedUnit == NULL) break;

      if(dwZPos != 0){
        m_pSelectedUnit->SetPos(m_pSelectedUnit->GetXPos(), m_pSelectedUnit->GetYPos(), dwZPos);
      }
      UpdateSelection();
    }
    break;
  }
}

void CMapEditorDoc::OnUnitCivilizations() 
{
  CCivilizationEditorDlg dlg;
  
  dlg.Create(&m_Map);
  dlg.DoModal();

  SetModifiedFlag();
  UpdateAllViews(NULL);
}

void CMapEditorDoc::OnUnitDeleteLibrary() 
{
	CDeleteUnitLibraryDlg dlg;

  m_Map.GetExclusiveAccess();
  try{
  dlg.Create(&m_Map);

  dlg.DoModal();
  m_Map.ReleaseExclusiveAccess();
  }
  catch(...){
    m_Map.ReleaseExclusiveAccess();
  }	
}

void CMapEditorDoc::OnUnitEditLibrary() 
{
  CEditUnitLibraryDlg dlg;
  
  dlg.Create(&m_Map);

  dlg.DoModal();	
}

void CMapEditorDoc::OnUnitNewLibrary() 
{
	CFileDialog dlg(FALSE, "*.ulb", NULL, OFN_HIDEREADONLY |
    OFN_OVERWRITEPROMPT, "Knihovny jednotek (*.ulb)|*.ulb|Všechny soubory|*.*||",
    AfxGetMainWnd());

  // ask for the library file
  if(dlg.DoModal() != IDOK)
    return;

// !!!!!!!!!!!!!!!!!!
// only temporary
// !!!!!!!!!!!!!!!!!!
  // cur the extension of -> use only the name
  // cause we have to open directory

/*  CString strLibFile, strPath = dlg.GetPathName();
  int nPos;
  nPos = strPath.ReverseFind('.');
  if(nPos > -1){
    strLibFile = strPath.Left(nPos);
  }
  else{
    strLibFile = strPath;
  }*/

// !!!!!!!!!!!!!!!!!!!!!!

  CString strLibFile = dlg.GetPathName();

  m_Map.GetExclusiveAccess();

  try{
    // create new library
    CEUnitLibrary *pLibrary;
    pLibrary = m_Map.NewUnitLibrary(strLibFile);

    // edit it (name ...)
    CUnitLibraryEditorDlg libdlg;
    libdlg.Create(pLibrary);

    libdlg.DoModal();

    SetModifiedFlag();

    m_Map.ReleaseExclusiveAccess();
  }
  catch(...){
    m_Map.ReleaseExclusiveAccess();
    throw;
  }	
}

void CMapEditorDoc::OnUnitOpenLibrary() 
{
	CFileDialog dlg(TRUE, "*.ulb", NULL, OFN_HIDEREADONLY | OFN_PATHMUSTEXIST,
    "Knihovny jednotek (*.ulb)|*.ulb|Všechny soubory|*.*||", AfxGetMainWnd());

  if(dlg.DoModal() != IDOK){
    return;
  }

// !!!!!!!!!!!!!!!!!!
// only temporary
// !!!!!!!!!!!!!!!!!!
  // cur the extension of -> use only the name
  // cause we have to open directory

/*  CString strLibFile, strPath = dlg.GetPathName();
  int nPos;
  nPos = strPath.ReverseFind('.');
  if(nPos > -1){
    strLibFile = strPath.Left(nPos);
  }
  else{
    strLibFile = strPath;
  }*/

// !!!!!!!!!!!!!!!!!!!!!!
  CString strLibFile = dlg.GetPathName();

  m_Map.GetExclusiveAccess();
  try{

  POSITION pos;
  CEUnitLibrary *pUnitLibrary;
  pos = m_Map.GetFirstUnitLibraryPosition();
  while(pos != NULL){
    pUnitLibrary = m_Map.GetNextUnitLibrary(pos);
    if(strLibFile.CompareNoCase(pUnitLibrary->GetFullPath()) == 0){
      // library allready loaded
      AfxMessageBox("Tato knohivna je již otevøená.");
      return;
    }
  }
  m_Map.OpenUnitLibrary(strLibFile);
  m_Map.ReleaseExclusiveAccess();
  }
  catch(...){
    m_Map.ReleaseExclusiveAccess();
    throw;
  }

  SetModifiedFlag();	
}

void CMapEditorDoc::OnUnitEditor() 
{
  CUnitLibrariesDlg UnitLibrariesDlg;

  m_Map.GetExclusiveAccess();
  try{
  UnitLibrariesDlg.Create(&m_Map);

  UnitLibrariesDlg.DoModal();

  // we have to save libraries (mapex graphics could changed)
  // so we have to use the new one
  m_Map.SaveUnitLibraries();

  m_Map.ReleaseExclusiveAccess();
  }
  catch(CException *){
    m_Map.ReleaseExclusiveAccess();
    throw;
  }

  UpdateAllViews(NULL);
}

CEMapexInstance * CMapEditorDoc::GetSelectedMapexFromCell(DWORD dwX, DWORD dwY)
{
  if(m_nSelectionType != Sel_BLMapexInstance) return NULL;
  return m_Selection.GetMapexFromCell(dwX, dwY);
}


void CMapEditorDoc::OnUnitDirectionLeft() 
{
  ASSERT(m_nSelectionType == Sel_Unit);
  if(m_pSelectedUnit == NULL) return;

  UpdateSelection();
  BYTE nDirection = m_pSelectedUnit->GetDirection();
  if(nDirection == 8) nDirection = 1; // Hack -> the result will be 0
  if(nDirection == 0) nDirection = 7;
  else nDirection --;
  m_pSelectedUnit->SetDirection(nDirection);
  UpdateAllViews(NULL, Update_Window, NULL); // update now -> no whole screen painting
  UpdateSelection();
  SetModifiedFlag();
}
void CMapEditorDoc::OnUpdateUnitDirectionLeft(CCmdUI* pCmdUI){ pCmdUI->Enable(m_nSelectionType == Sel_Unit); }
void CMapEditorDoc::OnUnitDirectionRandom() 
{
  ASSERT(m_nSelectionType == Sel_Unit);
  if(m_pSelectedUnit == NULL) return;

  UpdateSelection();
  if(m_pSelectedUnit->GetDirection() == 8){
    m_pSelectedUnit->SetDirection(0);
  }
  else{
    m_pSelectedUnit->SetDirection(8);
  }
  UpdateAllViews(NULL, Update_Window, NULL); // update now -> no whole screen painting
  UpdateSelection();
  SetModifiedFlag();
}

void CMapEditorDoc::OnUpdateUnitDirectionRandom(CCmdUI* pCmdUI)
{
  if(m_nSelectionType != Sel_Unit){
    pCmdUI->SetCheck(0);
    pCmdUI->Enable(FALSE);
  }
  else{
    if(m_pSelectedUnit != NULL){
      if(m_pSelectedUnit->GetDirection() == 8) pCmdUI->SetCheck(1);
      else pCmdUI->SetCheck(0);
    }
    else pCmdUI->SetCheck(0);
    pCmdUI->Enable(TRUE);
  }
}

void CMapEditorDoc::OnUnitDirectionRight() 
{
  ASSERT(m_nSelectionType == Sel_Unit);
  if(m_pSelectedUnit == NULL) return;

  UpdateSelection();
  BYTE nDirection = m_pSelectedUnit->GetDirection();
  if(nDirection == 8) nDirection = 7; // Hack -> the result will be 0
  nDirection++; if(nDirection > 7) nDirection = 0;
  m_pSelectedUnit->SetDirection(nDirection);
  UpdateAllViews(NULL, Update_Window, NULL); // update now -> no whole screen painting
  UpdateSelection();
}
void CMapEditorDoc::OnUpdateUnitDirectionRight(CCmdUI* pCmdUI) { pCmdUI->Enable(m_nSelectionType == Sel_Unit); }

void CMapEditorDoc::OnFileResources() 
{
  CResourcesDlg dlg;	

  dlg.Create(&m_Map);
  dlg.DoModal();
}

void CMapEditorDoc::OnUpdateMapexUp(CCmdUI* pCmdUI) 
{
  if(m_nSelectionType != Sel_BLMapexInstance) pCmdUI->Enable(FALSE);
  else{
    if(m_Selection.GetHighestLevel(&m_Map) < 3)
      pCmdUI->Enable(TRUE);
    else pCmdUI->Enable(FALSE);
  }
}

void CMapEditorDoc::OnMapexUp() 
{
  if(m_nSelectionType != Sel_BLMapexInstance) return;
  m_Selection.MoveSelectionUp(&m_Map);
  SetModifiedFlag();
  CMainFrame *pFrame = (CMainFrame *)AfxGetMainWnd();
  pFrame->SetSelectedMapexPaneText("");
}

void CMapEditorDoc::OnUpdateMapexDown(CCmdUI* pCmdUI) 
{
  if(m_nSelectionType != Sel_BLMapexInstance) pCmdUI->Enable(FALSE);
  else{
    if(m_Selection.GetLowestLevel(&m_Map) > 1)
      pCmdUI->Enable(TRUE);
    else pCmdUI->Enable(FALSE);
  }
}

void CMapEditorDoc::OnMapexDown() 
{
  if(m_nSelectionType != Sel_BLMapexInstance) return;
  m_Selection.MoveSelectionDown(&m_Map);
  SetModifiedFlag();
  CMainFrame *pFrame = (CMainFrame *)AfxGetMainWnd();
  pFrame->SetSelectedMapexPaneText("");
}


void CMapEditorDoc::OnFileInvisibility() 
{
  CInvisibilityDlg dlg;

  dlg.DoModal();
}

void CMapEditorDoc::OnFileScriptSet() 
{
  CScriptSetEditDlg dlg;
  
  dlg.Create(&m_Map);

  dlg.DoModal();
}

void CMapEditorDoc::OnFileFindPathGraphs() 
{
  CFindPathGraphsDlg dlg;
  
  dlg.Create(&m_Map);

  dlg.DoModal();
}

void CMapEditorDoc::OnFileSetMapSize() 
{
  // First we have to lock the map
  m_Map.GetExclusiveAccess ();

  // Run the dialog
  CSetMapSizeDlg dlg;
  dlg.m_dwWidth = m_Map.GetWidth ();
  dlg.m_dwHeight = m_Map.GetHeight ();
  if ( dlg.DoModal () == IDOK )
  {
    // If the new size is smaller then the old one -> ask the user again
    if ( (dlg.m_dwWidth < (DWORD)(m_Map.GetWidth ())) || (dlg.m_dwHeight < (DWORD)(m_Map.GetHeight ())) )
    {
      if ( AfxMessageBox ( IDS_REALLYRESIZEMAP, MB_YESNO ) == IDYES )
      {
        // Round up the values
        dlg.m_dwWidth = (dlg.m_dwWidth + MAPSQUARE_WIDTH - 1) / MAPSQUARE_WIDTH;
        dlg.m_dwHeight = (dlg.m_dwHeight + MAPSQUARE_HEIGHT - 1) / MAPSQUARE_HEIGHT;
        if ( dlg.m_dwWidth < 2 ) dlg.m_dwWidth = 2;
        if ( dlg.m_dwHeight < 2 ) dlg.m_dwHeight = 2;
        dlg.m_dwWidth *= MAPSQUARE_WIDTH;
        dlg.m_dwHeight *= MAPSQUARE_HEIGHT;

        // OK, resize the map
        m_Map.SetSize ( dlg.m_dwWidth, dlg.m_dwHeight );
      }
    }
  }

  m_Map.ReleaseExclusiveAccess ();
  UpdateAllViews(NULL);
  // Small hack, this will update whole minimap
  m_MiniMap.SetZoom ( m_MiniMap.GetZoom () );  
}
