// UnitDlg.cpp : implementation file
//

#include "stdafx.h"
#include "..\MapEditor.h"
#include "UnitDlg.h"

#include "..\DataObjects\EMap.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CUnitDlg

IMPLEMENT_DYNAMIC(CUnitDlg, CPropertySheet)

CUnitDlg::CUnitDlg(UINT nIDCaption, CWnd* pParentWnd, UINT iSelectPage)
	:CPropertySheet(nIDCaption, pParentWnd, iSelectPage)
{
}

CUnitDlg::CUnitDlg(LPCTSTR pszCaption, CWnd* pParentWnd, UINT iSelectPage)
	:CPropertySheet(pszCaption, pParentWnd, iSelectPage)
{
}

CUnitDlg::~CUnitDlg()
{
}


BEGIN_MESSAGE_MAP(CUnitDlg, CPropertySheet)
	//{{AFX_MSG_MAP(CUnitDlg)
		// NOTE - the ClassWizard will add and remove mapping macros here.
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CUnitDlg message handlers

void CUnitDlg::Create(CEUnitType *pUnitType, CEMap *pMap)
{
  ASSERT_VALID(pUnitType);
  ASSERT_VALID(pMap);

  m_pUnitType = pUnitType;
  m_pMap = pMap;

  // we have to load all appearances into the memory
  // so do it
  m_pUnitType->LoadGraphics();

  m_MainPage.Create(m_pUnitType);
  AddPage(&m_MainPage);
  m_ModesPage.Create(m_pUnitType);
  AddPage(&m_ModesPage);
  m_AppearancePage.Create(m_pUnitType);
  AddPage(&m_AppearancePage);
  m_LandTypesPage.Create(m_pUnitType, m_pMap);
  AddPage(&m_LandTypesPage);
  m_SkillsPage.Create(m_pUnitType);
  AddPage(&m_SkillsPage);
}

void CUnitDlg::Delete()
{
  // clear the unit appearances
  m_pUnitType->ReleaseGraphics();
}
