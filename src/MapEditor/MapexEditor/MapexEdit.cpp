// MapexEdit.cpp : implementation file
//

#include "stdafx.h"
#include "..\MapEditor.h"
#include "MapexEdit.h"

#include "..\Controls\ImageTBButton.h"

#include "LandTypeSelectDlg.h"
#include "..\DataObjects\EMap.h"

#include "MapexSizeDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define ID_LANDTYPEBUTTONS 20000
#define ID_SETGRAPHICS 20100
#define ID_SETSIZE 20101
#define ID_VIEWGRAPHICS 20102

/////////////////////////////////////////////////////////////////////////////
// CMapexEdit dialog


CMapexEdit::CMapexEdit(CWnd* pParent /*=NULL*/)
	: CDialog(CMapexEdit::IDD, pParent)
{
	//{{AFX_DATA_INIT(CMapexEdit)
	m_strName = _T("");
	//}}AFX_DATA_INIT

  m_pCurrentLandType= NULL;
}


void CMapexEdit::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CMapexEdit)
	DDX_Text(pDX, IDC_NAME, m_strName);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CMapexEdit, CDialog)
	//{{AFX_MSG_MAP(CMapexEdit)
	ON_WM_DESTROY()
	ON_WM_SIZE()
	ON_COMMAND(ID_SETGRAPHICS, OnSetGraphics)
  ON_COMMAND(ID_SETSIZE, OnSetSize)
  ON_COMMAND(ID_VIEWGRAPHICS, OnViewType)
	ON_COMMAND(IDOK, OnOk)
	//}}AFX_MSG_MAP
  ON_COMMAND_RANGE(ID_LANDTYPEBUTTONS, ID_LANDTYPEBUTTONS + 9, OnLandTypeButton)
  ON_NOTIFY_RANGE(CImageTBButton::N_DOUBLECLICKED, ID_LANDTYPEBUTTONS, ID_LANDTYPEBUTTONS + 9, OnLandTypeButtonDblClk)
  ON_NOTIFY(TBN_DROPDOWN, 1, OnDropDownViewGraphics)
  ON_COMMAND(ID_MAPEXGRP_GRAPHICS, OnViewGraphics)
  ON_COMMAND(ID_MAPEXGRP_GRID, OnViewGrid)
  ON_COMMAND(ID_MAPEXGRP_BOTH, OnViewBoth)
END_MESSAGE_MAP()

BYTE CMapexEdit::m_aLandTypesSelected[10] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
int CMapexEdit::m_nSelectedLandType = 0;
RECT CMapexEdit::m_rectWindow = {-1, -1, -1, -1};

/////////////////////////////////////////////////////////////////////////////
// CMapexEdit message handlers

void CMapexEdit::Create(CEMapex *pMapex, CEMapexLibrary *pMapexLibrary)
{
  ASSERT(pMapex != NULL);
  ASSERT_VALID(pMapexLibrary);
  m_pMapex = pMapex;
  m_pMapexLibrary = pMapexLibrary;

  m_hIcon = AfxGetApp()->LoadIcon(IDI_MAPEX);
}

BOOL CMapexEdit::OnInitDialog() 
{
	CDialog::OnInitDialog();

  SetIcon(m_hIcon, TRUE);

  m_strName = m_pMapex->GetName();

  if(m_rectWindow.left != -1){
    CRect rc(&m_rectWindow);
    SetWindowPos(NULL, rc.left, rc.top,rc.Width(), rc.Height(), SWP_NOZORDER | SWP_NOREDRAW);
  }
	
  UpdateData(FALSE);

  m_wndStatusBar.Create(WS_CHILD | WS_VISIBLE | CCS_BOTTOM | SBARS_SIZEGRIP, CRect(0, 0, 0, 0), this, 2);
  int nWidths[1];
  nWidths[0] = -1;
  m_wndStatusBar.SetParts(1, nWidths);

  CWnd *pView;
  pView = GetDlgItem(IDC_VIEW);
  DWORD dwStyle = pView->GetStyle();
  CRect rcView;
  pView->GetWindowRect(&rcView);
  ScreenToClient(&rcView);
  pView->DestroyWindow();
  m_wndView.Create(m_pMapex, rcView, dwStyle, this, IDC_VIEW);

  CWnd *pToolBar = GetDlgItem(IDC_TOOLBAR);
  CRect rcToolBar;
  pToolBar->GetWindowRect(&rcToolBar);
  pToolBar->DestroyWindow();
  ScreenToClient(&rcToolBar);

  m_wndToolBar.Create(TBSTYLE_FLAT | TBSTYLE_LIST | CCS_NOMOVEY | CCS_NOPARENTALIGN | CCS_NODIVIDER |
    WS_VISIBLE | WS_CHILD, rcToolBar, this, 1);

  m_wndToolBar.SetIndent(2);
  m_wndToolBar.SetButtonSize(CSize(17, 15));
  m_wndToolBar.SetExtendedStyle(TBSTYLE_EX_DRAWDDARROWS);
  
  m_wndToolBar.ShowWindow(SW_SHOW);
  m_wndToolBar.UpdateWindow();

  AutoSize();

  int i;
  m_aLandTypeButtons.SetSize(10);
  CLandTypeTBButton *pBut;
  for(i = 0; i < m_aLandTypeButtons.GetSize(); i++){
    pBut = new CLandTypeTBButton();
    pBut->Insert(&m_wndToolBar, i, ID_LANDTYPEBUTTONS + i);
    m_aLandTypeButtons[i] = pBut;
  }

  m_wndToolBar.AddStrings("Grafika\0Zavøít\0Velikost\0Zobrazit\0");

  m_wndToolBar.AddBitmap(4, IDB_MAPEXEDITTB);

  TBBUTTON but;
  but.fsState = TBSTATE_ENABLED;
  but.fsStyle = TBSTYLE_SEP;
  but.iBitmap = 0;
  but.iString = 0;
  but.idCommand = 0;
  m_wndToolBar.InsertButton(10, &but);

  but.fsState = TBSTATE_ENABLED;
  but.fsStyle = TBSTYLE_DROPDOWN;
  but.iBitmap = 3;
  but.iString = 3;
  but.idCommand = ID_VIEWGRAPHICS;
  m_wndToolBar.InsertButton(11, &but);

  but.fsState = TBSTATE_ENABLED;
  but.fsStyle = TBSTYLE_BUTTON;
  but.iBitmap = 0;
  but.iString = 0;
  but.idCommand = ID_SETGRAPHICS;
  m_wndToolBar.InsertButton(12, &but);

  but.iBitmap = 2;
  but.iString = 2;
  but.idCommand = ID_SETSIZE;
  m_wndToolBar.InsertButton(13, &but);

  but.iBitmap = 1;
  but.iString = 1;
  but.idCommand = IDOK;
  m_wndToolBar.InsertButton(14, &but);

  for(i = 0; i < 10; i++){
    CLandType *pLandType = AppGetActiveMap()->GetLandType(m_aLandTypesSelected[i]);
    if(pLandType == NULL)
      pLandType = AppGetActiveMap()->GetLandType(0); // get the transparent one
    m_aLandTypeButtons[i]->SetLandType(pLandType);
  }
  m_aLandTypeButtons[m_nSelectedLandType]->SetSelected(TRUE);
  m_pCurrentLandType = m_aLandTypeButtons[m_nSelectedLandType]->GetLandType();
  m_wndView.SetCurrentLandType(m_pCurrentLandType);

  m_wndView.SetViewType(CMapexEditView::VIEW_BOTH);

  UpdateStatusBar();

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CMapexEdit::OnDestroy()
{
  GetWindowRect(&m_rectWindow);

  int i;
  for(i = 0; i < m_aLandTypeButtons.GetSize(); i++){
    if(m_aLandTypeButtons[i]->GetSelected()){
      m_nSelectedLandType = i;
    }
    m_aLandTypesSelected[i] = m_aLandTypeButtons[i]->GetLandType()->GetID();
    delete m_aLandTypeButtons[i];
  }
  m_aLandTypeButtons.RemoveAll();

  CDialog::OnDestroy();
}

void CMapexEdit::OnSize(UINT nType, int cx, int cy) 
{
	CDialog::OnSize(nType, cx, cy);

  AutoSize();
}

void CMapexEdit::AutoSize()
{
  if(!::IsWindow(m_wndToolBar.GetSafeHwnd()))
    return;

  CRect rcToolBar;
  m_wndToolBar.AutoSize();	
  m_wndToolBar.GetWindowRect(&rcToolBar);
  ScreenToClient(&rcToolBar);

  CRect rcStatusBar;
  m_wndStatusBar.GetWindowRect(&rcStatusBar);
  ScreenToClient(&rcStatusBar);

  CRect rcView;
  m_wndView.GetWindowRect(&rcView);
  ScreenToClient(&rcView);

  CRect rcClient;
  GetClientRect(&rcClient);

  m_wndStatusBar.SetWindowPos(NULL, rcClient.left, rcClient.bottom - rcStatusBar.Height(),
    rcClient.Width(), rcStatusBar.Height(), SWP_NOZORDER);

  m_wndView.SetWindowPos(NULL, rcView.left, rcToolBar.bottom,
    rcClient.Width(), rcClient.bottom - rcToolBar.bottom - rcStatusBar.Height(),
    SWP_NOZORDER | SWP_NOREDRAW);
  m_wndView.Invalidate(FALSE);
}

void CMapexEdit::OnSetGraphics()
{
  CFileDialog dlg(TRUE, NULL, NULL, OFN_HIDEREADONLY | OFN_PATHMUSTEXIST,
    "Podporované formáty|*.anim;*.tif;*.tga|Animace (*.anim)|*.anim|Targar (*.tga)|*.tga|TIFF (*.tif)|*.tif|Všechny soubory|*.*||",
    this);
  dlg.m_ofn.lpstrTitle = "Nastavit grafiku mapexu";

  if(dlg.DoModal() == IDOK){
    m_pMapex->SetGraphics(dlg.GetPathName(), m_pMapexLibrary->GetArchive());
    m_wndView.Invalidate(FALSE);
  }
}

void CMapexEdit::OnSetSize()
{
  CMapexSizeDlg dlg;

  dlg.m_dwWidth = m_pMapex->GetSize().cx;
  dlg.m_dwHeight = m_pMapex->GetSize().cy;
  if(dlg.DoModal() == IDOK){
    m_pMapex->SetSize(CSize(dlg.m_dwWidth, dlg.m_dwHeight));
    m_wndView.OnMapexSizeChanged();
  }
}

void CMapexEdit::OnLandTypeButton(UINT nID)
{
  int nSelected = nID - ID_LANDTYPEBUTTONS;
  int i;

  for(i = 0; i < 10; i++){
    if(nSelected == i){
      m_aLandTypeButtons[i]->SetSelected(TRUE);
      m_pCurrentLandType = m_aLandTypeButtons[i]->GetLandType();
      m_wndView.SetCurrentLandType(m_pCurrentLandType);
    }
    else{
      m_aLandTypeButtons[i]->SetSelected(FALSE);
    }
  }

  UpdateStatusBar();
}

void CMapexEdit::OnLandTypeButtonDblClk(UINT nID, NMHDR *pNotifyStruct, LRESULT *pResult)
{
  CLandTypeTBButton *pButton;
  pButton = m_aLandTypeButtons[nID - ID_LANDTYPEBUTTONS];

  CLandTypeSelectDlg dlg;
  dlg.Create(AppGetActiveMap(), pButton->GetLandType());

  if(dlg.DoModal() == IDOK){
    pButton->SetLandType(dlg.GetSelected());
    m_pCurrentLandType = pButton->GetLandType();
    m_wndView.SetCurrentLandType(m_pCurrentLandType);
    UpdateStatusBar();
  }

  *pResult = 0;
}

void CMapexEdit::UpdateStatusBar()
{
  if(m_pCurrentLandType != NULL){
    m_wndStatusBar.SetText(m_pCurrentLandType->GetName(), 0, 0);
  }
  else{
    m_wndStatusBar.SetText("", 0, 0);
  }
}

void CMapexEdit::SaveState(CString strKey)
{
  CString strK;
  int i;
  CWinApp *pApp = AfxGetApp();

  for(i = 0; i < 10; i++){
    strK.Format("LandType%d", i);
    pApp->WriteProfileInt(strKey, strK, m_aLandTypesSelected[i]);
  }
  pApp->WriteProfileInt(strKey, "SelectedLandType", m_nSelectedLandType);
}

void CMapexEdit::LoadState(CString strKey)
{
  CString strK;
  int i;
  CWinApp *pApp = AfxGetApp();

  for(i = 0; i < 10; i++){
    strK.Format("LandType%d", i);
    m_aLandTypesSelected[i] = pApp->GetProfileInt(strKey, strK, 0);
  }
  m_nSelectedLandType = pApp->GetProfileInt(strKey, "SelectedLandType", 0);
}

void CMapexEdit::OnViewType()
{
  switch(m_wndView.GetViewType()){
  case CMapexEditView::VIEW_BOTH:
    m_wndView.SetViewType(CMapexEditView::VIEW_GRAPHICS);
    break;
  case CMapexEditView::VIEW_GRAPHICS:
    m_wndView.SetViewType(CMapexEditView::VIEW_GRID);
    break;
  case CMapexEditView::VIEW_GRID:
    m_wndView.SetViewType(CMapexEditView::VIEW_BOTH);
    break;
  }
}

void CMapexEdit::OnDropDownViewGraphics(NMHDR *pNotifyStruct, LRESULT *pResult)
{
  LPNMTOOLBAR pNMToolBar = (LPNMTOOLBAR)pNotifyStruct;

  switch(pNMToolBar->iItem){
  case ID_VIEWGRAPHICS:
    CMenu menu, *pPopupMenu;
    menu.LoadMenu(IDM_VIEWGRAPHICSMENU);
    pPopupMenu = menu.GetSubMenu(0);

    switch(m_wndView.GetViewType()){
    case CMapexEditView::VIEW_BOTH:
      pPopupMenu->CheckMenuItem(ID_MAPEXGRP_BOTH, MF_BYCOMMAND | MF_CHECKED);
      break;
    case CMapexEditView::VIEW_GRAPHICS:
      pPopupMenu->CheckMenuItem(ID_MAPEXGRP_GRAPHICS, MF_BYCOMMAND | MF_CHECKED);
      break;
    case CMapexEditView::VIEW_GRID:
      pPopupMenu->CheckMenuItem(ID_MAPEXGRP_GRID, MF_BYCOMMAND | MF_CHECKED);
      break;
    }

    CRect rcButton;
    m_wndToolBar.GetRect(ID_VIEWGRAPHICS, &rcButton);
    m_wndToolBar.ClientToScreen(&rcButton);
    pPopupMenu->TrackPopupMenu(TPM_LEFTALIGN | TPM_LEFTBUTTON, rcButton.left, rcButton.bottom, this);
    break;
  }
  *pResult = TBDDRET_DEFAULT;
}

void CMapexEdit::OnViewGraphics()
{
  m_wndView.SetViewType(CMapexEditView::VIEW_GRAPHICS);
}

void CMapexEdit::OnViewGrid()
{
  m_wndView.SetViewType(CMapexEditView::VIEW_GRID);
}

void CMapexEdit::OnViewBoth()
{
  m_wndView.SetViewType(CMapexEditView::VIEW_BOTH);
}

void CMapexEdit::OnOk() 
{
  UpdateData(TRUE);

  if(m_strName.IsEmpty()){
    AfxMessageBox("Musíte zadat jméno mapexu.");
    return;
  }
  m_pMapex->SetName(m_strName);

  CDialog::OnOK();
}
