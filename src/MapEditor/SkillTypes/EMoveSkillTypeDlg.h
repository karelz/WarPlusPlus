#if !defined(AFX_EMOVESKILLTYPEDLG_H__A650B844_5402_11D4_B0AA_004F49068BD6__INCLUDED_)
#define AFX_EMOVESKILLTYPEDLG_H__A650B844_5402_11D4_B0AA_004F49068BD6__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// EMoveSkillTypeDlg.h : header file
//

#include "..\UnitEditor\UnitSkillTypeDlg.h"
#include "Common\Map\MMoveSkillType.h"

#include "..\Controls\AppearanceControl.h"

/////////////////////////////////////////////////////////////////////////////
// CEMoveSkillTypeDlg dialog

class CEMoveSkillTypeDlg : public CUnitSkillTypeDlg
{
// Construction
public:
  virtual CString GetName(){ return "Move"; }
	virtual void DeleteSkill(CEUnitSkillType *pSkillType);
	virtual void CloseSkillType(BOOL bApply);
	virtual void InitSkillTypeDlg(CEUnitSkillType *pSkillType);
	virtual void InitNewSkill(CEUnitSkillType *pSKillType);
	CEMoveSkillTypeDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CEMoveSkillTypeDlg)
	enum { IDD = IDD_MOVESKILL };
	DWORD	m_dwSpeed;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CEMoveSkillTypeDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CEMoveSkillTypeDlg)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:
  // The move appearance control
  CAppearanceControl m_wndMoveAppearance;
  DWORD m_dwMoveAppearanceID;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_EMOVESKILLTYPEDLG_H__A650B844_5402_11D4_B0AA_004F49068BD6__INCLUDED_)
