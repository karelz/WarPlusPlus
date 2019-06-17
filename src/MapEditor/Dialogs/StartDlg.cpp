// StartDlg.cpp : implementation file
//

#include "stdafx.h"
#include "..\MapEditor.h"
#include "StartDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CStartDlg dialog


CStartDlg::CStartDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CStartDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CStartDlg)
	m_strLastOpenedMap = _T("");
	//}}AFX_DATA_INIT
  m_dwWhatToDo = LastOpened;
  m_bDisableLastOpened = FALSE;
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}


void CStartDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CStartDlg)
	DDX_Text(pDX, IDC_LASTOPENEDMAP, m_strLastOpenedMap);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CStartDlg, CDialog)
	//{{AFX_MSG_MAP(CStartDlg)
	ON_BN_CLICKED(IDC_LASTOPENED, OnLastOpened)
	ON_BN_CLICKED(IDC_NEWMAP, OnNewMap)
	ON_BN_CLICKED(IDC_OPEN, OnOpen)
	ON_BN_DOUBLECLICKED(IDC_LASTOPENED, OnDoubleClickedLastOpened)
	ON_BN_DOUBLECLICKED(IDC_NEWMAP, OnDoubleClickedNewMap)
	ON_BN_DOUBLECLICKED(IDC_OPEN, OnDoubleClickedOpen)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CStartDlg message handlers

void CStartDlg::OnLastOpened() 
{
  m_dwWhatToDo = LastOpened;
}

void CStartDlg::OnNewMap() 
{
  m_dwWhatToDo = NewMap;
}

void CStartDlg::OnOpen() 
{
  m_dwWhatToDo = OpenMap;
}

BOOL CStartDlg::OnInitDialog() 
{
  CDialog::OnInitDialog();
	
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

  switch(m_dwWhatToDo){
  case LastOpened:
    ((CButton *)GetDlgItem(IDC_LASTOPENED))->SetCheck(1);
    break;
  case NewMap:
    ((CButton *)GetDlgItem(IDC_NEWMAP))->SetCheck(1);
    break;
  case OpenMap:
    ((CButton *)GetDlgItem(IDC_OPEN))->SetCheck(1);
    break;
  }
  if(m_bDisableLastOpened){
    GetDlgItem(IDC_LASTOPENED)->EnableWindow(FALSE);
  }
  


	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CStartDlg::OnDoubleClickedLastOpened() 
{
  m_dwWhatToDo = LastOpened;
  EndDialog(IDOK);	
}

void CStartDlg::OnDoubleClickedNewMap() 
{
  m_dwWhatToDo = NewMap;
  EndDialog(IDOK);
}

void CStartDlg::OnDoubleClickedOpen() 
{
  m_dwWhatToDo = OpenMap;
  EndDialog(IDOK);
}
