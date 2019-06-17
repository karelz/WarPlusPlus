#if !defined(AFX_ESTORAGESKILLTYPEDLG_H__F3112BD5_A59A_11D4_811B_0000B48431EC__INCLUDED_)
#define AFX_ESTORAGESKILLTYPEDLG_H__F3112BD5_A59A_11D4_811B_0000B48431EC__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// EStorageSkillTypeDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CEStorageSkillTypeDlg dialog

#include "..\UnitEditor\UnitSkillTypeDlg.h"
#include "Common\Map\MStorageSkillType.h"

#include "..\Controls\AppearanceControl.h"
#include "..\Controls\ResourceControl.h"

class CEStorageSkillTypeDlg : public CUnitSkillTypeDlg
{
// Konstrukce
public:
	CEStorageSkillTypeDlg(CWnd* pParent = NULL);   // standard constructor

// Metody
public:
	virtual CString GetName() { return "Storage"; }
	virtual void DeleteSkill(CEUnitSkillType *pSkillType);
	virtual void CloseSkillType(BOOL bApply);
	virtual void InitSkillTypeDlg(CEUnitSkillType *pSkillType);
	virtual void InitNewSkill(CEUnitSkillType *pSKillType);

public:
// Dialog Data
	//{{AFX_DATA(CEStorageSkillTypeDlg)
	enum { IDD = IDD_STORAGESKILL };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CEStorageSkillTypeDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CEStorageSkillTypeDlg)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

// Controly
private:
	CAppearanceControl m_wndUnloadAppearance;
	CResourceControl m_wndResources;

// Data
private:
	SMStorageSkillType m_Data;

};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_ESTORAGESKILLTYPEDLG_H__F3112BD5_A59A_11D4_811B_0000B48431EC__INCLUDED_)
