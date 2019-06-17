#if !defined(AFX_EMAKESKILLTYPERECORDDLG_H__22AA2026_8009_11D4_80DB_0000B4A08F9A__INCLUDED_)
#define AFX_EMAKESKILLTYPERECORDDLG_H__22AA2026_8009_11D4_80DB_0000B4A08F9A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// EMakeSkillTypeRecordDlg.h : header file
//

#include "..\UnitEditor\UnitSkillTypeDlg.h"
#include "Common\Map\MMakeSkillType.h"

#include "..\Controls\AppearanceControl.h"
#include "..\Controls\ResourceControl.h"
#include "..\Controls\UnitTypeControl.h"

/////////////////////////////////////////////////////////////////////////////
// CEMakeSkillTypeRecordDlg dialog

class CEMakeSkillTypeRecordDlg : public CDialog
{
// Construction
public:
	CEMakeSkillTypeRecordDlg(SMMakeSkillTypeRecord *pRecord, CEUnitSkillType * pSkillType, CWnd* pParent = NULL);

// Dialog Data
	//{{AFX_DATA(CEMakeSkillTypeRecordDlg)
	enum { IDD = IDD_MAKESKILLTYPE };
	int		m_nTimeToMake;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CEMakeSkillTypeRecordDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CEMakeSkillTypeRecordDlg)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:
    // Data reprezentujici controly
    CAppearanceControl m_wndMakeAppearance;
    CResourceControl m_wndResources;
    CUnitTypeControl m_wndUnitType;

    // Vnitrni data
    DWORD m_dwMakeAppearanceID;
    int m_ResourcesNeeded[RESOURCE_COUNT];
    
    // Data predana pri inicializaci
    CEUnitSkillType * m_pSkillType;
    SMMakeSkillTypeRecord *m_pRecord;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_EMAKESKILLTYPERECORDDLG_H__22AA2026_8009_11D4_80DB_0000B4A08F9A__INCLUDED_)
