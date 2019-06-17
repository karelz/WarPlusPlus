#if !defined(AFX_EMINESKILLTYPEDLG_H__F3112BD3_A59A_11D4_811B_0000B48431EC__INCLUDED_)
#define AFX_EMINESKILLTYPEDLG_H__F3112BD3_A59A_11D4_811B_0000B48431EC__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// EMineSkillTypeDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CEMineSkillTypeDlg dialog

#include "..\UnitEditor\UnitSkillTypeDlg.h"
#include "Common\Map\MMineSkillType.h"

#include "..\Controls\AppearanceControl.h"
#include "..\Controls\ResourceControl.h"

class CEMineSkillTypeDlg : public CUnitSkillTypeDlg
{
// Konstrukce
public:
	CEMineSkillTypeDlg(CWnd* pParent = NULL);   // standard constructor

// Metody
public:
	virtual CString GetName() { return "Mine"; }
	virtual void DeleteSkill(CEUnitSkillType *pSkillType);
	virtual void CloseSkillType(BOOL bApply);
	virtual void InitSkillTypeDlg(CEUnitSkillType *pSkillType);
	virtual void InitNewSkill(CEUnitSkillType *pSKillType);
	virtual void OnLoadSkill(CEUnitSkillType *pSkillType);

// Controly
private:
	CAppearanceControl m_wndBeforeMineAppearance;
	CAppearanceControl m_wndMineAppearance;
	CAppearanceControl m_wndAfterMineAppearance;

	CAppearanceControl m_wndBeforeUnloadAppearance;
	CAppearanceControl m_wndUnloadAppearance;
	CAppearanceControl m_wndAfterUnloadAppearance;

	CAppearanceControl m_wndBeforeMineFullAppearance;
	CAppearanceControl m_wndMineFullAppearance;
	CAppearanceControl m_wndAfterUnloadFullAppearance;

	CResourceControl m_wndStorageResources;
	CResourceControl m_wndMineResources;
	CResourceControl m_wndUnloadResources;

// Data
private:
	SMMineSkillType m_Data;

public:
// Dialog Data
	//{{AFX_DATA(CEMineSkillTypeDlg)
	enum { IDD = IDD_MINESKILL };
	CComboBox	m_wndMode;
	DWORD	m_dwUnloadDistance;
	DWORD	m_dwMineDistance;
	DWORD	m_dwTimeslicesAfterMine;
	DWORD	m_dwTimeslicesAfterUnload;
	DWORD	m_dwTimeslicesBeforeMine;
	DWORD	m_dwTimeslicesBeforeUnload;
	DWORD	m_dwTimeslicesAfterUnloadFull;
	DWORD	m_dwTimeslicesBeforeMineFull;
	//}}AFX_DATA

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CEMineSkillTypeDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CEMineSkillTypeDlg)
	virtual void OnOK();
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_EMINESKILLTYPEDLG_H__F3112BD3_A59A_11D4_811B_0000B48431EC__INCLUDED_)
