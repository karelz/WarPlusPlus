#if !defined(AFX_UNITDLG_H__CB5410F6_8E0D_11D3_A876_00105ACA8325__INCLUDED_)
#define AFX_UNITDLG_H__CB5410F6_8E0D_11D3_A876_00105ACA8325__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// UnitDlg.h : header file
//

#include "..\DataObjects\EUnitType.h"
#include "UnitMainPage.h"	// Added by ClassView
#include "UnitAppearancePage.h"	// Added by ClassView
#include "UnitModesPage.h"
#include "UnitLandTypesPage.h"
#include "UnitSkillsPage.h"
class CEMap;

/////////////////////////////////////////////////////////////////////////////
// CUnitDlg

class CUnitDlg : public CPropertySheet
{
	DECLARE_DYNAMIC(CUnitDlg)

// Construction
public:
	CUnitDlg(UINT nIDCaption, CWnd* pParentWnd = NULL, UINT iSelectPage = 0);
	CUnitDlg(LPCTSTR pszCaption, CWnd* pParentWnd = NULL, UINT iSelectPage = 0);

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CUnitDlg)
	//}}AFX_VIRTUAL

// Implementation
public:
	void Delete();
	void Create(CEUnitType *pUnitType, CEMap *pMap);
	virtual ~CUnitDlg();

	// Generated message map functions
protected:
	//{{AFX_MSG(CUnitDlg)
		// NOTE - the ClassWizard will add and remove member functions here.
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:
  CUnitModesPage m_ModesPage;
  CUnitAppearancePage m_AppearancePage;
	CUnitMainPage m_MainPage;
  CUnitLandTypesPage m_LandTypesPage;
  CUnitSkillsPage m_SkillsPage;

  CEUnitType *m_pUnitType;
  CEMap *m_pMap;
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_UNITDLG_H__CB5410F6_8E0D_11D3_A876_00105ACA8325__INCLUDED_)
