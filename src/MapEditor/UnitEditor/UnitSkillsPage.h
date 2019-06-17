#if !defined(AFX_UNITSKILLSPAGE_H__3F3D2885_949D_11D3_A0D1_FF6546682838__INCLUDED_)
#define AFX_UNITSKILLSPAGE_H__3F3D2885_949D_11D3_A0D1_FF6546682838__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// UnitSkillsPage.h : header file
//

#include "..\DataObjects\EUnitType.h"
/////////////////////////////////////////////////////////////////////////////
// CUnitSkillsPage dialog

class CUnitSkillsPage : public CPropertyPage
{
	DECLARE_DYNCREATE(CUnitSkillsPage)

// Construction
public:
	void Create(CEUnitType *pUnitType);
	CUnitSkillsPage();
	~CUnitSkillsPage();

// Dialog Data
	//{{AFX_DATA(CUnitSkillsPage)
	enum { IDD = IDD_UNITSKILLSPAGE };
	CButton	m_wndUp;
	CButton	m_wndDown;
	CButton	m_wndEnabled;
	CButton	m_wndEdit;
	CButton	m_wndDelete;
	CListCtrl	m_wndModeList;
	CListCtrl	m_wndList;
	//}}AFX_DATA


// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(CUnitSkillsPage)
	public:
	virtual void OnOK();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(CUnitSkillsPage)
	virtual BOOL OnInitDialog();
	afx_msg void OnAdd();
	afx_msg void OnItemChangedList(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDblClkList(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDelete();
	afx_msg void OnEdit();
	afx_msg void OnClickModeList(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDown();
	afx_msg void OnUp();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:
	void WriteSelectedProps();
	CImageList m_CheckModeImageList;
	void UpdateSelectedProps();
	void UpdateSelectedSkill();
	void UpdateSkillsList();
  CEUnitType *m_pUnitType;
  CEUnitSkillType *m_pCurrentSkillType;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_UNITSKILLSPAGE_H__3F3D2885_949D_11D3_A0D1_FF6546682838__INCLUDED_)
