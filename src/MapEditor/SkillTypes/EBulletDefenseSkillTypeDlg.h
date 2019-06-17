#if !defined(AFX_EBULLETDEFENSESKILLTYPEDLG_H__8BD08784_7EF3_4C2D_A4CC_F8D0DB4F9721__INCLUDED_)
#define AFX_EBULLETDEFENSESKILLTYPEDLG_H__8BD08784_7EF3_4C2D_A4CC_F8D0DB4F9721__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// EBulletDefenseSkillTypeDlg.h : header file
//

#include "..\UnitEditor\UnitSkillTypeDlg.h"

/////////////////////////////////////////////////////////////////////////////
// CEBulletDefenseSkillTypeDlg dialog

class CEBulletDefenseSkillTypeDlg : public CUnitSkillTypeDlg
{
// Construction
public:
	CEBulletDefenseSkillTypeDlg(CWnd* pParent = NULL);   // standard constructor
    virtual CString GetName() { return "BulletDefense"; }
	virtual void DeleteSkill(CEUnitSkillType *pSkillType);
	virtual void CloseSkillType(BOOL bApply);
	virtual void InitSkillTypeDlg(CEUnitSkillType *pSkillType);
	virtual void InitNewSkill(CEUnitSkillType *pSKillType);

// Dialog Data
	//{{AFX_DATA(CEBulletDefenseSkillTypeDlg)
	enum { IDD = IDD_BULLETDEFENSESKILL };
	DWORD	m_dwAbsolute;
	float	m_fLinear;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CEBulletDefenseSkillTypeDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CEBulletDefenseSkillTypeDlg)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_EBULLETDEFENSESKILLTYPEDLG_H__8BD08784_7EF3_4C2D_A4CC_F8D0DB4F9721__INCLUDED_)
