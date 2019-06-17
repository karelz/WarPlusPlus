// MapexEditorDlg.cpp : implementation file
//

#include "stdafx.h"
#include "..\MapEditor.h"
#include "MapexEditorDlg.h"

#include "MapexSizeDlg.h"
#include "MapexEdit.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CMapexEditorDlg dialog


CMapexEditorDlg::CMapexEditorDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CMapexEditorDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CMapexEditorDlg)
	//}}AFX_DATA_INIT

  m_pCurrentMapexLibrary = NULL;
}


void CMapexEditorDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CMapexEditorDlg)
	DDX_Control(pDX, IDC_MAPEXLISTCTRL, m_MapexList);
	DDX_Control(pDX, IDC_EDITMAPEX, m_EditMapex);
	DDX_Control(pDX, IDC_DELETEMAPEX, m_DeleteMapex);
	DDX_Control(pDX, IDC_LIBRARYCOMBO, m_LibraryCombo);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CMapexEditorDlg, CDialog)
	//{{AFX_MSG_MAP(CMapexEditorDlg)
	ON_CBN_SELENDOK(IDC_LIBRARYCOMBO, OnSelEndOkLibraryCombo)
	ON_BN_CLICKED(IDC_ADDMAPEX, OnAddMapex)
	ON_BN_CLICKED(IDC_EDITMAPEX, OnEditMapex)
	ON_BN_CLICKED(IDC_DELETEMAPEX, OnDeleteMapex)
	ON_NOTIFY(NM_DBLCLK, IDC_MAPEXLISTCTRL, OnDblClkMapexList)
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_MAPEXLISTCTRL, OnItemChangedMapexList)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMapexEditorDlg message handlers

void CMapexEditorDlg::Create(CEMap *pMap)
{
  ASSERT_VALID(pMap);

  m_pMap = pMap;
}

BOOL CMapexEditorDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();

  CStatic *pPreview;
  DWORD dwStyle;
  CRect rcPreview;
  pPreview = (CStatic *)GetDlgItem(IDC_MAPEXPREVIEW);
  dwStyle = pPreview->GetStyle();
  pPreview->GetWindowRect(&rcPreview);
  ScreenToClient(&rcPreview);
  pPreview->DestroyWindow();

  m_wndMapexPreview.Create(rcPreview, dwStyle, this, IDC_MAPEXPREVIEW);

  CRect rcList; m_MapexList.GetClientRect(&rcList);
  m_MapexList.SetExtendedStyle(LVS_EX_FULLROWSELECT);
  m_MapexList.InsertColumn(0, "", LVCFMT_LEFT, rcList.Width() - 16);
  m_pMap->FillMapexLibrariesComboBox(&m_LibraryCombo);
  m_LibraryCombo.SetCurSel(0);
  UpdateSelectedLibrary();
  UpdateListItems();
  UpdateSelectedMapex();

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CMapexEditorDlg::UpdateListItems()
{
  if(m_pCurrentMapexLibrary == NULL){
    m_MapexList.DeleteAllItems();
    return;
  }
  
  m_pCurrentMapexLibrary->FillMapexesListCtrl(&m_MapexList);
}

void CMapexEditorDlg::UpdateSelectedLibrary()
{
  int nSel = m_LibraryCombo.GetCurSel();
  if(nSel == -1){
    m_pCurrentMapexLibrary = NULL;
    return;
  }
  m_pCurrentMapexLibrary = (CEMapexLibrary *)m_LibraryCombo.GetItemData(nSel);
}

void CMapexEditorDlg::UpdateSelectedMapex()
{
  if(m_MapexList.GetSelectedCount() == 0){
    m_pCurrentMapex = NULL;
    m_DeleteMapex.EnableWindow(FALSE);
    m_EditMapex.EnableWindow(FALSE);
    m_wndMapexPreview.SetMapex(NULL);
    return;
  }
  int nSel = m_MapexList.GetNextItem(-1, LVNI_SELECTED);
  m_pCurrentMapex = (CEMapex *)m_MapexList.GetItemData(nSel);
  m_DeleteMapex.EnableWindow(TRUE);
  m_EditMapex.EnableWindow(TRUE);
  m_wndMapexPreview.SetMapex(m_pCurrentMapex);
}

void CMapexEditorDlg::OnSelEndOkLibraryCombo() 
{
  UpdateSelectedLibrary();
  UpdateListItems();
  UpdateSelectedMapex();
}

void CMapexEditorDlg::OnAddMapex() 
{
  CEMapex *pMapex;
  CSize sizeMapex;

  if(m_pCurrentMapexLibrary == NULL) return;

  // first create only small mapex
  pMapex = m_pCurrentMapexLibrary->NewMapex(CSize(1, 1));

  // load a graphics for it
  CFileDialog dlg(TRUE, NULL, NULL, OFN_HIDEREADONLY | OFN_PATHMUSTEXIST,
    "Podporované formáty|*.anim;*.tif;*.tga|Animace (*.anim)|*.anim|Targa (*.tga)|*.tga|TIFF (*.tif)|*.tif|Všechny soubory|*.*||",
    this);
  dlg.m_ofn.lpstrTitle = "Nový mapex";

  if(dlg.DoModal() == IDOK){
    pMapex->SetGraphics(dlg.GetPathName(), m_pCurrentMapexLibrary->GetArchive());
  }
  else{
    m_pCurrentMapexLibrary->DeleteMapex(pMapex);
    return;
  }

  CMapexSizeDlg SizeDlg;

  // now get the real size of it
  SizeDlg.m_dwWidth = 10; SizeDlg.m_dwHeight = 10;
  if(pMapex->GetGraphics() != NULL){
    SizeDlg.m_dwWidth = (pMapex->GetGraphics()->GetFrame(0)->GetWidth() - 1) / 16 + 1;
    SizeDlg.m_dwHeight = (pMapex->GetGraphics()->GetFrame(0)->GetHeight() - 1) / 16 + 1;
  }
  if(SizeDlg.DoModal() != IDOK)
    return;
  sizeMapex.cx = SizeDlg.m_dwWidth;
  sizeMapex.cy = SizeDlg.m_dwHeight;
  pMapex->SetSize(sizeMapex);

  CMapexEdit MapexEdit;
  MapexEdit.Create(pMapex, m_pCurrentMapexLibrary);

  MapexEdit.DoModal();

  UpdateListItems();
  UpdateSelectedMapex();
}

void CMapexEditorDlg::OnEditMapex() 
{
  CMapexEdit MapexEdit;
  if(m_pCurrentMapex == NULL) return;

  MapexEdit.Create(m_pCurrentMapex, m_pCurrentMapexLibrary);
  MapexEdit.DoModal();

  UpdateListItems();
  UpdateSelectedMapex();
}

void CMapexEditorDlg::OnDeleteMapex() 
{
	if(m_pCurrentMapex == NULL) return;

  CString str;
  str.Format("Opravdu chcete smazat mapex '%s'?", m_pCurrentMapex->GetName());
  if(AfxMessageBox(str, MB_ICONEXCLAMATION | MB_YESNO) != IDYES){
    return;
  }

  m_pCurrentMapexLibrary->DeleteMapex(m_pCurrentMapex);

  UpdateListItems();
  UpdateSelectedMapex();
}

void CMapexEditorDlg::OnDblClkMapexList(NMHDR* pNMHDR, LRESULT* pResult) 
{
  OnEditMapex();	
	
	*pResult = 0;
}

void CMapexEditorDlg::OnItemChangedMapexList(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;

  if(pNMListView->uChanged & LVIF_STATE)
    UpdateSelectedMapex();

	*pResult = 0;
}
