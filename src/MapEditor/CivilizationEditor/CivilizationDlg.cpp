// CivilizationDlg.cpp : implementation file
//

#include "stdafx.h"
#include "..\MapEditor.h"
#include "CivilizationDlg.h"

#include "..\DataObjects\EMap.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CCivilizationDlg dialog


CCivilizationDlg::CCivilizationDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CCivilizationDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CCivilizationDlg)
	m_strName = _T("");
	//}}AFX_DATA_INIT
  m_dwColor = 0;
}


void CCivilizationDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CCivilizationDlg)
	DDX_Control(pDX, IDC_STARTPOS, m_wndStartPos);
	DDX_Control(pDX, IDC_SCRIPTSET, m_wndScriptSet);
	DDX_Text(pDX, IDC_NAME, m_strName);
	//}}AFX_DATA_MAP
  if(pDX->m_bSaveAndValidate){
    m_dwColor = m_wndColor.GetColor();
  }
  else{
    m_wndColor.SetColor(m_dwColor);
  }
}


BEGIN_MESSAGE_MAP(CCivilizationDlg, CDialog)
	//{{AFX_MSG_MAP(CCivilizationDlg)
	ON_CBN_SELENDOK(IDC_SCRIPTSET, OnSelEndOkScriptSet)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CCivilizationDlg message handlers

BOOL CCivilizationDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
  CWnd *pColor = GetDlgItem(IDC_COLOR);
  CRect rcColor;
  DWORD dwStyle = pColor->GetStyle();
  pColor->GetWindowRect(&rcColor);
  ScreenToClient(&rcColor);
  pColor->DestroyWindow();
  m_wndColor.Create("", dwStyle, rcColor, this, IDC_COLOR);
  m_wndColor.ShowWindow(SW_SHOW);
  
  m_wndResource.Create(this, IDC_RESOURCES, m_pResources);

  m_pMap->FillScriptSetComboBox(&m_wndScriptSet);

  {
    int i;
    for(i = 0; i < m_wndScriptSet.GetCount(); i++){
      if(m_pScriptSet == (CEScriptSet *)m_wndScriptSet.GetItemData(i)){
        m_wndScriptSet.SetCurSel(i);
      }
    }
  }

  m_wndStartPos.ResetContent();
  POSITION pos = m_pMap->GetFirstUnitPosition();
  CEUnit *pUnit;
  int nSel = -1;
  while(pos != NULL){
    pUnit = m_pMap->GetNextUnit(pos);

    if(pUnit->GetCivilization() == m_pCivilization){
      CString str;
      int i;
      str.Format("%s (%d, %d)", (LPCSTR)(pUnit->GetUnitType()->GetName()), pUnit->GetXPos(), pUnit->GetYPos());
      i = m_wndStartPos.AddString(str);
      m_wndStartPos.SetItemData(i, (DWORD)pUnit);

      if((pUnit->GetXPos() == m_pCivilization->GetXStartPosition()) && (pUnit->GetYPos() == m_pCivilization->GetYStartPosition())){
        nSel = i;
      }
    }
  }

  if(nSel != -1){
    m_wndStartPos.SetCurSel(nSel);
  }

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CCivilizationDlg::OnSelEndOkScriptSet() 
{
  int nSel = m_wndScriptSet.GetCurSel();
  if(nSel == -1){
    m_pScriptSet = NULL;
    return;
  }
  m_pScriptSet = (CEScriptSet *)m_wndScriptSet.GetItemData(nSel);
}

void CCivilizationDlg::OnOK() 
{
  int nSel = m_wndStartPos.GetCurSel();
  if(nSel == -1){
    m_pCivilization->SetStartPosition(0, 0);
  }
  else{
    CEUnit *pUnit = (CEUnit *)m_wndStartPos.GetItemData(nSel);
    m_pCivilization->SetStartPosition(pUnit->GetXPos(), pUnit->GetYPos());
  }
  
	CDialog::OnOK();
}
