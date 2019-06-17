#if !defined(AFX_EXAMPLESKILLTYPEDLG_H__14C5FA77_99B3_11D3_A0DD_B66FF94DE631__INCLUDED_)
#define AFX_EXAMPLESKILLTYPEDLG_H__14C5FA77_99B3_11D3_A0DD_B66FF94DE631__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ExampleSkillTypeDlg.h : header file
//

#include "..\UnitEditor\UnitSkillTypeDlg.h"

/////////////////////////////////////////////////////////////////////////////
// CExampleSkillTypeDlg dialog

class CExampleSkillTypeDlg : public CUnitSkillTypeDlg
{
// Construction
public:
  virtual CString GetName(){ return "Example"; }
	virtual void DeleteSkill(CEUnitSkillType *pSkillType);
	virtual void CloseSkillType(BOOL bApply);
	virtual void InitSkillTypeDlg(CEUnitSkillType *pSkillType);
	virtual void InitNewSkill(CEUnitSkillType *pSKillType);
	CExampleSkillTypeDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CExampleSkillTypeDlg)
	enum { IDD = IDD_EXAMPLESKILLTYPE };
	CString	m_strDescription;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CExampleSkillTypeDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

  struct tagSSkillData{
    char m_pDescription[31]; // the description
  };
  typedef struct tagSSkillData SSkillData;

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CExampleSkillTypeDlg)
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_EXAMPLESKILLTYPEDLG_H__14C5FA77_99B3_11D3_A0DD_B66FF94DE631__INCLUDED_)
