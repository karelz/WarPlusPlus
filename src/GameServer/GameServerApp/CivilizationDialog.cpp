// CivilizationDialog.cpp : implementation file
//

#include "stdafx.h"
#include "GameServerApp.h"
#include "CivilizationDialog.h"

#include "GameServerAppDoc.h"

#include "..\GameServer\SMap.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CCivilizationDialog dialog


CCivilizationDialog::CCivilizationDialog(CWnd* pParent /*=NULL*/)
	: CDialog(CCivilizationDialog::IDD, pParent)
{
	//{{AFX_DATA_INIT(CCivilizationDialog)
	m_strName = _T("");
	m_strGlobalProfile = _T("");
	m_strLocalProfile = _T("");
	m_bFreeze = FALSE;
	m_strGlobalCount = _T("");
	m_strLocalCount = _T("");
	//}}AFX_DATA_INIT
  m_nSortColumn = 0;
}


void CCivilizationDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CCivilizationDialog)
	DDX_Control(pDX, IDC_PROFILELIST, m_wndProfileList);
	DDX_Text(pDX, IDC_NAME, m_strName);
	DDX_Text(pDX, IDC_GLOBALPROFILE, m_strGlobalProfile);
	DDX_Text(pDX, IDC_LOCALPROFILE, m_strLocalProfile);
	DDX_Check(pDX, IDC_FREEZE, m_bFreeze);
	DDX_Text(pDX, IDC_GLOBALCOUNT, m_strGlobalCount);
	DDX_Text(pDX, IDC_LOCALCOUNT, m_strLocalCount);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CCivilizationDialog, CDialog)
	//{{AFX_MSG_MAP(CCivilizationDialog)
	ON_WM_DESTROY()
  ON_MESSAGE ( WM_PROFILEUPDATED, OnProfileUpdated )
	ON_BN_CLICKED(IDC_RESET, OnReset)
	ON_NOTIFY(HDN_ITEMCLICK, IDC_PROFILELIST, OnColumnItemClickProfileList)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CCivilizationDialog message handlers

BOOL CCivilizationDialog::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
  // Create the color control
  {
    CWnd *pStatic = GetDlgItem(IDC_COLOR);
    DWORD dwStyle = pStatic->GetStyle();
    CRect rcStatic;
    pStatic->GetWindowRect(&rcStatic);
    ScreenToClient(&rcStatic);
    pStatic->DestroyWindow();

    m_wndColor.Create(rcStatic, dwStyle, this, m_Color, IDC_COLOR);
  }

  m_wndProfileList.InsertColumn ( 0, "Funkce", LVCFMT_LEFT, 260, 0 );
  m_wndProfileList.InsertColumn ( 1, "5 sec", LVCFMT_RIGHT, 50, 0 );
  m_wndProfileList.InsertColumn ( 2, "Celk.", LVCFMT_RIGHT, 50, 0 );

  CGameServerAppDoc * pDoc = (CGameServerAppDoc * )(((CFrameWnd *)AfxGetMainWnd ())->GetActiveDocument ());
  pDoc->RegisterProfileObserver ( this );

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CCivilizationDialog::OnDestroy() 
{
	CDialog::OnDestroy();
	
  CGameServerAppDoc * pDoc = (CGameServerAppDoc * )(((CFrameWnd *)AfxGetMainWnd ())->GetActiveDocument ());
  pDoc->UnregisterProfileObserver ( this );	
}

CString PrintInt64ByLocale ( __int64 nNumber );

LRESULT CCivilizationDialog::OnProfileUpdated ( WPARAM, LPARAM )
{
  // Update our profiling info

  UpdateData ( TRUE );
  if ( m_bFreeze ) return 0;

  __int64 nGlobalAll = 0, nLocalAll = 0;
  DWORD dwCivilization;
  for ( dwCivilization = 0; dwCivilization < g_cMap.GetCivilizationCount (); dwCivilization++ )
  {
    CZCivilization *pCiv = g_cMap.GetCivilization ( dwCivilization )->GetZCivilization ();
    nGlobalAll += pCiv->GetInterpret ()->GetGlobalProfileCounter ();
    nLocalAll += pCiv->GetInterpret ()->GetLocalProfileCounter ();
  }

  m_strGlobalCount = PrintInt64ByLocale ( nGlobalAll );
  m_strLocalCount = PrintInt64ByLocale ( nLocalAll );

  CZCivilization * pCiv = m_pSCivilization->GetZCivilization ();
  CString str;
  double dbPercent;
  if ( nLocalAll == 0 )
  {
    m_strLocalProfile = "n/a";
  }
  else
  {
    dbPercent = ((double)(pCiv->GetInterpret ()->GetLocalProfileCounter ()) * 100) / ((double)nLocalAll);
    m_strLocalProfile.Format ( "%.02f%%", dbPercent );
  }

  if ( nGlobalAll == 0 )
  {
    m_strGlobalProfile = "n/a";
  }
  else
  {
    dbPercent = ((double)(pCiv->GetInterpret ()->GetGlobalProfileCounter ()) * 100) / ((double)nGlobalAll);
    m_strGlobalProfile.Format ( "%.02f%%", dbPercent );  
  }

  CString strDetail;
  pCiv->GetCodeManager ()->GetCounters ( strDetail );

  m_wndProfileList.DeleteAllItems ();

  CString strLine;
  int nPos, nLinePos = 0;
  int nItem = 0;
  while ( true )
  {
    nPos = strDetail.Find ( '|', nLinePos );
    if ( nPos == -1 )
    {
      InsertProfileLine ( strDetail.Mid ( nLinePos ), nItem );
      break;
    }
    InsertProfileLine ( strDetail.Mid ( nLinePos, nPos - nLinePos ), nItem );
    nPos++;
    nLinePos = nPos;
  }

  m_wndProfileList.SortItems ( (PFNLVCOMPARE)SortFunction, (DWORD)this );

  UpdateData ( FALSE );
  return 0;
}

void CCivilizationDialog::InsertProfileLine ( CString &strLine, int &nItem )
{
  CString strFunction, strShort, strLong;
  int nPos, nStartPos = 0;

  nPos = strLine.Find ( '/', nStartPos );
  if ( nPos == -1 ) return;
  strFunction = strLine.Mid ( nStartPos, nPos - nStartPos );
  nStartPos = nPos + 1;
  nPos = strLine.Find ( '/', nStartPos );
  if ( nPos == -1 ) return;
  strShort = strLine.Mid ( nStartPos, nPos - nStartPos );
  nStartPos = nPos + 1;
  nPos = strLine.Find ( '/', nStartPos );
  if ( nPos == -1 )
    strLong = strLine.Mid ( nStartPos );
  else
    strLong = strLine.Mid ( nStartPos, nPos - nStartPos );

  int nI = m_wndProfileList.InsertItem ( nItem++, strFunction, 0 );
  m_wndProfileList.SetItemText ( nI, 1, strShort );
  m_wndProfileList.SetItemText ( nI, 2, strLong );
}

void CCivilizationDialog::OnReset() 
{
  CZCivilization * pCiv = m_pSCivilization->GetZCivilization ();
  pCiv->GetCodeManager ()->ClearGlobalAndLocalCounters ();
}

void CCivilizationDialog::OnColumnItemClickProfileList(NMHDR* pNMHDR, LRESULT* pResult) 
{
	HD_NOTIFY *phdn = (HD_NOTIFY *) pNMHDR;

  LPNMHEADER pH = (NMHEADER *)pNMHDR;
  
  m_nSortColumn = pH->iItem;
  m_wndProfileList.SortItems ( (PFNLVCOMPARE)SortFunction, (DWORD)this );
  
	*pResult = 0;
}

int CCivilizationDialog::SortFunction (LPARAM p1, LPARAM p2, LPARAM pT)
{
  CCivilizationDialog * pThis = (CCivilizationDialog * )pT;

  CString str1 = pThis->m_wndProfileList.GetItemText ( p1, pThis->m_nSortColumn );
  CString str2 = pThis->m_wndProfileList.GetItemText ( p2, pThis->m_nSortColumn );

  return str1.CompareNoCase ( str2 );
}
