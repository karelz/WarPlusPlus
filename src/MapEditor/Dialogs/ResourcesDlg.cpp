// ResourcesDlg.cpp : implementation file
//

#include "stdafx.h"
#include "..\MapEditor.h"
#include "ResourcesDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CResourcesDlg dialog


CResourcesDlg::CResourcesDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CResourcesDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CResourcesDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
  m_pMap = NULL;
}


void CResourcesDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CResourcesDlg)
	DDX_Control(pDX, IDC_SETIMAGE, m_wndSetImage);
	DDX_Control(pDX, IDC_RESOURCES, m_wndList);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CResourcesDlg, CDialog)
	//{{AFX_MSG_MAP(CResourcesDlg)
	ON_NOTIFY(LVN_ENDLABELEDIT, IDC_RESOURCES, OnEndLabelEditResources)
	ON_BN_CLICKED(IDC_SETIMAGE, OnSetImage)
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_RESOURCES, OnItemChangedResources)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CResourcesDlg message handlers

void CResourcesDlg::Create(CEMap *pMap)
{
  ASSERT(pMap != NULL);

  m_pMap = pMap;
}

void CResourcesDlg::Delete()
{
  m_pMap = NULL;
}

BOOL CResourcesDlg::OnInitDialog() 
{
  ASSERT(m_pMap != NULL);
	CDialog::OnInitDialog();
	
  CStatic *pPreview;
  DWORD dwStyle;
  CRect rcPreview;
  pPreview = (CStatic *)GetDlgItem(IDC_PREVIEW);
  dwStyle = pPreview->GetStyle();
  pPreview->GetWindowRect(&rcPreview);
  ScreenToClient(&rcPreview);
  pPreview->DestroyWindow();

  m_wndPreview.Create(rcPreview, dwStyle, this, IDC_PREVIEW);

  m_wndList.InsertColumn(0, "Jméno", LVCFMT_LEFT, 200, 0);

  int i, nIndex;
  CEResource *pResource;
  for(i = 0; i < RESOURCE_COUNT; i++){
    pResource = m_pMap->GetResource(i);

    nIndex = m_wndList.InsertItem(i, pResource->GetName(), 0);
    m_wndList.SetItemData(nIndex, (DWORD)pResource);
  }

  m_pSelected = NULL;
  m_wndSetImage.EnableWindow(FALSE);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CResourcesDlg::OnEndLabelEditResources(NMHDR* pNMHDR, LRESULT* pResult) 
{
	LV_DISPINFO* pDispInfo = (LV_DISPINFO*)pNMHDR;

	*pResult = TRUE;
}

void CResourcesDlg::OnSetImage() 
{
  if(m_pSelected == NULL) return;

  CFileDialog dlg(TRUE, NULL, NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
    "Truevision Targa (*.tga)|*.tga|TIFF (*.tif)|*.tif|Všechny podporované formáty (*.tga;*.tif)|*.tga;*.tif||",
    this);

  if(dlg.DoModal() != IDOK) return;

  CImageSurface icon;
  VERIFY(icon.Create(CDataArchive::GetRootArchive()->CreateFile(dlg.GetPathName())));
  m_pSelected->SetIcon(&icon);
  icon.Delete();

  UpdateSelectedResource();
}

void CResourcesDlg::OnItemChangedResources(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;
	
  if(pNMListView->uChanged & LVIF_STATE)
    UpdateSelectedResource();

	*pResult = 0;
}

void CResourcesDlg::UpdateSelectedResource()
{
  if(m_wndList.GetSelectedCount() == 0){
    m_pSelected = NULL;
  }
  else{
    int nSel = m_wndList.GetNextItem(-1, LVNI_SELECTED);
    if(nSel != -1){
      m_pSelected = (CEResource *)m_wndList.GetItemData(nSel);
    }
    else{
      m_pSelected = NULL;
    }
  }

  if(m_pSelected != NULL){
    m_wndSetImage.EnableWindow(TRUE);
  }
  else{
    m_wndSetImage.EnableWindow(FALSE);
  }
  m_wndPreview.SetResource(m_pSelected);
}

void CResourcesDlg::OnOK() 
{
  int i;
  CEResource *pResource;
  for(i = 0; i < RESOURCE_COUNT; i++){
    pResource = (CEResource *)m_wndList.GetItemData(i);
    ASSERT(pResource == m_pMap->GetResource(i));
    pResource->SetName(m_wndList.GetItemText(i, 0));
  }
  
	CDialog::OnOK();
}
