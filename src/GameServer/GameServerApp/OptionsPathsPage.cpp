// OptionsPathsPage.cpp : implementation file
//

#include "stdafx.h"
#include "GameServerApp.h"
#include "OptionsPathsPage.h"

#include "GameServerAppDoc.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// COptionsPathsPage property page

IMPLEMENT_DYNCREATE(COptionsPathsPage, CPropertyPage)

COptionsPathsPage::COptionsPathsPage(int eGameState) : CPropertyPage(COptionsPathsPage::IDD)
{
	//{{AFX_DATA_INIT(COptionsPathsPage)
	m_strData = _T("");
	m_strGameTemp = _T("");
	m_strArchiveTemp = _T("");
	//}}AFX_DATA_INIT
  m_eGameState = eGameState;
}

COptionsPathsPage::~COptionsPathsPage()
{
}

void COptionsPathsPage::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(COptionsPathsPage)
	DDX_Control(pDX, IDC_GAMETEMP, m_wndGameTemp);
	DDX_Control(pDX, IDC_DATA, m_wndData);
	DDX_Control(pDX, IDC_BROWSEGAMETEMP, m_wndBrowseGameTemp);
	DDX_Control(pDX, IDC_BROWSEDATA, m_wndBrowseData);
	DDX_Control(pDX, IDC_BROWSEARCHIVETEMP, m_wndBrowseArchiveTemp);
	DDX_Control(pDX, IDC_ARCHIVETEMP, m_wndArchiveTemp);
	DDX_Text(pDX, IDC_DATA, m_strData);
	DDX_Text(pDX, IDC_GAMETEMP, m_strGameTemp);
	DDX_Text(pDX, IDC_ARCHIVETEMP, m_strArchiveTemp);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(COptionsPathsPage, CPropertyPage)
	//{{AFX_MSG_MAP(COptionsPathsPage)
	ON_BN_CLICKED(IDC_BROWSEARCHIVETEMP, OnBrowseArchiveTemp)
	ON_BN_CLICKED(IDC_BROWSEDATA, OnBrowseData)
	ON_BN_CLICKED(IDC_BROWSEGAMETEMP, OnBrowseGameTemp)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// COptionsPathsPage message handlers

BOOL COptionsPathsPage::OnInitDialog() 
{
	CPropertyPage::OnInitDialog();

  // If the game is already initialized we can't change any paths
  // it's impossible
  if(m_eGameState != CGameServerAppDoc::Game_None){
    m_wndData.EnableWindow(FALSE);
    m_wndBrowseData.EnableWindow(FALSE);
    m_wndGameTemp.EnableWindow(FALSE);
    m_wndBrowseGameTemp.EnableWindow(FALSE);
    m_wndArchiveTemp.EnableWindow(FALSE);
    m_wndBrowseArchiveTemp.EnableWindow(FALSE);
  }
 
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void COptionsPathsPage::OnBrowseArchiveTemp() 
{
  UpdateData(TRUE);

  char pDir[MAX_PATH];
  pDir[0] = 0;
  BROWSEINFO sBrowseInfo;
  CString strTitle;
  strTitle.LoadString(IDS_OPTIONSPATHS_BROWSEARCHIVETEMP_TITLE);

  strcpy(pDir, m_strArchiveTemp);

  // Fill the browse info structure
  sBrowseInfo.hwndOwner = (HWND)GetSafeHwnd();
  sBrowseInfo.pidlRoot = NULL;
  sBrowseInfo.pszDisplayName = pDir;
  sBrowseInfo.lpszTitle = strTitle;
  sBrowseInfo.ulFlags = BIF_RETURNONLYFSDIRS;
  sBrowseInfo.lpfn = NULL;
  sBrowseInfo.lParam = 0;

  LPITEMIDLIST lpItemID;

  // Browse for it
  if((lpItemID = SHBrowseForFolder(&sBrowseInfo)) != NULL){
    // Translate the ListID to path
    SHGetPathFromIDList(lpItemID, pDir);
    // Write the path to the edit box
    m_strArchiveTemp = pDir;
    UpdateData(FALSE);
  }
}

void COptionsPathsPage::OnBrowseData() 
{
  UpdateData(TRUE);

  char pDir[MAX_PATH];
  pDir[0] = 0;
  BROWSEINFO sBrowseInfo;
  CString strTitle;
  strTitle.LoadString(IDS_OPTIONSPATHS_BROWSEDATA_TITLE);

  strcpy(pDir, m_strData);

  // Fill the browse info structure
  sBrowseInfo.hwndOwner = (HWND)GetSafeHwnd();
  sBrowseInfo.pidlRoot = NULL;
  sBrowseInfo.pszDisplayName = pDir;
  sBrowseInfo.lpszTitle = strTitle;
  sBrowseInfo.ulFlags = BIF_RETURNONLYFSDIRS;
  sBrowseInfo.lpfn = NULL;
  sBrowseInfo.lParam = 0;

  LPITEMIDLIST lpItemID;

  // Browse for it
  if((lpItemID = SHBrowseForFolder(&sBrowseInfo)) != NULL){
    // Translate the ListID to path
    SHGetPathFromIDList(lpItemID, pDir);
    // Write the path to the edit box
    m_strData = pDir;
    UpdateData(FALSE);
  }
}

void COptionsPathsPage::OnBrowseGameTemp() 
{
  UpdateData(TRUE);

  char pDir[MAX_PATH];
  pDir[0] = 0;
  BROWSEINFO sBrowseInfo;
  CString strTitle;
  strTitle.LoadString(IDS_OPTIONSPATHS_BROWSEGAMETEMP_TITLE);

  strcpy(pDir, m_strGameTemp);

  // Fill the browse info structure
  sBrowseInfo.hwndOwner = (HWND)GetSafeHwnd();
  sBrowseInfo.pidlRoot = NULL;
  sBrowseInfo.pszDisplayName = pDir;
  sBrowseInfo.lpszTitle = strTitle;
  sBrowseInfo.ulFlags = BIF_RETURNONLYFSDIRS;
  sBrowseInfo.lpfn = NULL;
  sBrowseInfo.lParam = 0;

  LPITEMIDLIST lpItemID;

  // Browse for it
  if((lpItemID = SHBrowseForFolder(&sBrowseInfo)) != NULL){
    // Translate the ListID to path
    SHGetPathFromIDList(lpItemID, pDir);
    // Write the path to the edit box
    m_strGameTemp = pDir;
    UpdateData(FALSE);
  }
}
