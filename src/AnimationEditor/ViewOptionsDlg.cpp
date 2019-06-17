// ViewOptionsDlg.cpp : implementation file
//

#include "stdafx.h"
#include "AnimationEditor.h"
#include "ViewOptionsDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CViewOptionsDlg dialog


CViewOptionsDlg::CViewOptionsDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CViewOptionsDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CViewOptionsDlg)
	m_bShowCrossHair = FALSE;
  m_dwBackgroundColor = RGB32 ( 0, 0, 0 );
  m_dwAnimationColor = RGB32 ( 220, 0, 0 );
  m_dwCrossHairColor = RGB32 ( 255, 0, 255 );
	m_strBkgAnimation = _T("");
	m_bDisplayBackgroundAsOverlay = FALSE;
	m_bShowBoundingRectangle = FALSE;
	//}}AFX_DATA_INIT
}


void CViewOptionsDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CViewOptionsDlg)
	DDX_Check(pDX, IDC_SHOWCROSSHAIR, m_bShowCrossHair);
	DDX_Text(pDX, IDC_BACKGROUNDANIMATION, m_strBkgAnimation);
	DDX_Check(pDX, IDC_DISPLAYBACKGROUNDASOVERLAY, m_bDisplayBackgroundAsOverlay);
	DDX_Check(pDX, IDC_SHOWBOUNDINGRECTANGLE, m_bShowBoundingRectangle);
	//}}AFX_DATA_MAP

  if ( pDX->m_bSaveAndValidate )
  {
    m_dwBackgroundColor = m_wndBackgroundColor.GetColor ();
    m_dwAnimationColor = m_wndAnimationColor.GetColor ();
    m_dwCrossHairColor = m_wndCrossHairColor.GetColor ();
  }
  else
  {
    m_wndBackgroundColor.SetColor ( m_dwBackgroundColor );
    m_wndAnimationColor.SetColor ( m_dwAnimationColor );
    m_wndCrossHairColor.SetColor ( m_dwCrossHairColor );
  }
}


BEGIN_MESSAGE_MAP(CViewOptionsDlg, CDialog)
	//{{AFX_MSG_MAP(CViewOptionsDlg)
	ON_BN_CLICKED(IDC_BROWSEBKGANIMATION, OnBrowseBkgAnimation)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CViewOptionsDlg message handlers

BOOL CViewOptionsDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
  m_wndBackgroundColor.Create ( IDC_BACKGROUNDCOLOR, this );
  m_wndAnimationColor.Create ( IDC_ANIMATIONCOLOR, this );
  m_wndCrossHairColor.Create ( IDC_CROSSHAIRCOLOR, this );
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CViewOptionsDlg::OnBrowseBkgAnimation() 
{
  UpdateData ( TRUE );

  CString strExt, strFilter, strTitle;
  strExt.LoadString ( IDS_ANIMATIONEXT );
  strFilter.LoadString ( IDS_ANIMATIONFILTER );
  strTitle.LoadString ( IDS_BKGANIMATIONTITLE );
  CFileDialog dlg ( TRUE, strExt, m_strBkgAnimation, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
    strFilter, this );
  dlg.m_ofn.lpstrTitle = strTitle;

  if ( dlg.DoModal () != IDOK ) return;

  // New animation selected
  m_strBkgAnimation = dlg.GetPathName ();
  UpdateData ( FALSE );
}
