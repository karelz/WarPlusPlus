#if !defined(AFX_EMAKESKILLTYPEDLG_H__B190C013_8001_11D4_80DB_0000B4A08F9A__INCLUDED_)
#define AFX_EMAKESKILLTYPEDLG_H__B190C013_8001_11D4_80DB_0000B4A08F9A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// emakeskilltypedlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CEMakeSkillTypeDlg dialog

#include "..\UnitEditor\UnitSkillTypeDlg.h"
#include "Common\Map\MMakeSkillType.h"

#include "..\Controls\AppearanceControl.h"

class CEMakeSkillTypeDlg : public CUnitSkillTypeDlg
{
public:
    // Konstruktor
	CEMakeSkillTypeDlg(CWnd* pParent = NULL);   // standard constructor

    // Destruktor
    ~CEMakeSkillTypeDlg();

// Metody
public:
    virtual CString GetName() { return "Make"; }
	virtual void DeleteSkill(CEUnitSkillType *pSkillType);
	virtual void CloseSkillType(BOOL bApply);
	virtual void InitSkillTypeDlg(CEUnitSkillType *pSkillType);
	virtual void InitNewSkill(CEUnitSkillType *pSKillType);

// Dialog Data
	//{{AFX_DATA(CEMakeSkillTypeDlg)
	enum { IDD = IDD_MAKESKILL };
	CButton	m_wndRemoveUnitType;
	CButton	m_wndEditUnitType;
	CButton	m_wndAddUnitType;
	CListBox	m_wndUnitTypeList;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CEMakeSkillTypeDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CEMakeSkillTypeDlg)
	afx_msg void OnAddUnitType();
	afx_msg void OnEditUnitType();
	afx_msg void OnRemoveUnitType();
	virtual void OnOK();
	virtual BOOL OnInitDialog();
	afx_msg void OnSelchangeUnitTypeList();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

// Pomocne metody
private:
    // Obnovi seznam vyrabenych typu
    void RefreshList();

    // Naformatuje retezec popisujici vyrobu
    CString FormatItem(SMMakeSkillTypeRecord *pRecord);

    // Nastavi enable/disable na buttonech
    void UpdateButtons();

// Data
private:
    SMMakeSkillType *m_pData;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_EMAKESKILLTYPEDLG_H__B190C013_8001_11D4_80DB_0000B4A08F9A__INCLUDED_)
