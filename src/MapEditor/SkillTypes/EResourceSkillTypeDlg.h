#if !defined(AFX_ERESOURCESKILLTYPEDLG_H__F3112BD4_A59A_11D4_811B_0000B48431EC__INCLUDED_)
#define AFX_ERESOURCESKILLTYPEDLG_H__F3112BD4_A59A_11D4_811B_0000B48431EC__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// EResourceSkillTypeDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CEResourceSkillTypeDlg dialog

#include "..\UnitEditor\UnitSkillTypeDlg.h"
#include "Common\Map\MResourceSkillType.h"

#include "..\Controls\AppearanceControl.h"
#include "..\Controls\ResourceControl.h"

class CEResourceSkillTypeDlg : public CUnitSkillTypeDlg
{
// Construction
public:
	CEResourceSkillTypeDlg(CWnd* pParent = NULL);   // standard constructor

// Metody
public:
	virtual CString GetName() { return "Resource"; }
	virtual void DeleteSkill(CEUnitSkillType *pSkillType);
	virtual void CloseSkillType(BOOL bApply);
	virtual void InitSkillTypeDlg(CEUnitSkillType *pSkillType);
	virtual void InitNewSkill(CEUnitSkillType *pSKillType);

// Controly
private:	
	CResourceControl m_wndInitResources;
	CResourceControl m_wndTimeSliceResources;

	CResourceControl m_wndResourcesLimit1;
	CResourceControl m_wndResourcesLimit2;
	CResourceControl m_wndResourcesLimit3;

	// Vzhledy normalni
	CAppearanceControl m_wndAppearance1;
	CAppearanceControl m_wndAppearance2;
	CAppearanceControl m_wndAppearance3;

	// Vzhledy behem tezeni
	CAppearanceControl m_wndAppearanceMine1;
	CAppearanceControl m_wndAppearanceMine2;
	CAppearanceControl m_wndAppearanceMine3;

	// Vzhled po dotezeni
	CAppearanceControl m_wndAppearanceAfterMining;

// Data
private:
	SMResourceSkillType m_Data;

public:
// Dialog Data
	//{{AFX_DATA(CEResourceSkillTypeDlg)
	enum { IDD = IDD_RESOURCESKILL };
	DWORD	m_dwTimeslicesAfterMine;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CEResourceSkillTypeDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CEResourceSkillTypeDlg)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_ERESOURCESKILLTYPEDLG_H__F3112BD4_A59A_11D4_811B_0000B48431EC__INCLUDED_)
