// NewMapDlg.cpp : implementation file
//

#include "stdafx.h"
#include "..\MapEditor.h"
#include "NewMapDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CNewMapDlg dialog


CNewMapDlg::CNewMapDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CNewMapDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CNewMapDlg)
	m_strDescription = _T("");
	m_strName = _T("");
	m_dwWidth = 0;
	m_dwHeight = 0;
	//}}AFX_DATA_INIT
}


void CNewMapDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CNewMapDlg)
	DDX_Text(pDX, IDC_DESCRIPTION, m_strDescription);
	DDX_Text(pDX, IDC_NAME, m_strName);
	DDX_Text(pDX, IDC_WIDTH, m_dwWidth);
	DDV_MinMaxDWord(pDX, m_dwWidth, 64, 100032);
	DDX_Text(pDX, IDC_HEIGHT, m_dwHeight);
	DDV_MinMaxDWord(pDX, m_dwHeight, 64, 100032);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CNewMapDlg, CDialog)
	//{{AFX_MSG_MAP(CNewMapDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CNewMapDlg message handlers

void CNewMapDlg::OnOK() 
{
  UpdateData(TRUE);

  BOOL bSizesChanged = FALSE;

  if((m_dwWidth % MAPSQUARE_WIDTH) != 0){
    m_dwWidth = m_dwWidth - (m_dwWidth % MAPSQUARE_WIDTH);
    bSizesChanged = TRUE;
  }
  if((m_dwHeight % MAPSQUARE_HEIGHT) != 0){
    m_dwHeight = m_dwHeight - (m_dwHeight % MAPSQUARE_HEIGHT);
    bSizesChanged = TRUE;
  }

  if(bSizesChanged){
    CString str;
    str.Format("Šíøka mapy musí být dìlitelná %d a výška mapy musí být dìlitelná %d.\n"
      "(Zadané velikosti byly zaokrouhleny dolù)", MAPSQUARE_WIDTH, MAPSQUARE_HEIGHT);
    AfxMessageBox(str);
    return;
  }
  
  if ( (m_dwHeight < MAPSQUARE_WIDTH * 2) && (m_dwWidth < MAPSQUARE_HEIGHT * 2 ) )
  {
    CString str;
    str.Format ( "Velikost mapy musí být alespoò %dx%d. Mapa bude zvìtšena.", MAPSQUARE_WIDTH * 2, MAPSQUARE_HEIGHT * 2 );
    AfxMessageBox ( str );
    if ( m_dwHeight < MAPSQUARE_WIDTH * 2 ) m_dwHeight = MAPSQUARE_WIDTH * 2;
    if ( m_dwWidth < MAPSQUARE_HEIGHT * 2) m_dwHeight = MAPSQUARE_HEIGHT * 2;
  }

	CDialog::OnOK();
}
