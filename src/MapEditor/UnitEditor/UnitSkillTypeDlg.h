#if !defined(AFX_UNITSKILLTYPEDLG_H__14C5FA76_99B3_11D3_A0DD_B66FF94DE631__INCLUDED_)
#define AFX_UNITSKILLTYPEDLG_H__14C5FA76_99B3_11D3_A0DD_B66FF94DE631__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// UnitSkillTypeDlg.h : header file
//

#include "..\DataObjects\EUnitSkillType.h"

/////////////////////////////////////////////////////////////////////////////
// CUnitSkillTypeDlg dialog

class CUnitSkillTypeDlg : public CDialog
{
// Construction
public:
  // called when the map is saving
  // Return TRUE if all data in the skilltype is valid
  virtual BOOL CheckValid(CEUnitSkillType *pSkillType);

  // called when the skill is remvoed from the unit's list
  // do some clean up
  // the skill data will be deleted automaticaly
	virtual void DeleteSkill(CEUnitSkillType *pSkillType);

  // this is called when the new skill is added to the unit's list
  // you should initialize the skills data by calling
  // CEUnitSkillType::AllocateData() (even if you pass the 0 size)
	virtual void InitNewSkill(CEUnitSkillType *pSkillType);

  // this is called when the skill is loaded from the saved map
  // you can convert data from older versions (using ReallocateData or so...)
  // the version of the file you were given the data from is in
  // g_dwUnitTypeFileVersion variable in MapFormats.h
  virtual void OnLoadSkill(CEUnitSkillType *pSkillType);

  // called after the dlg was closed
  // you can acces the skill by the m_pSkillType
  // after your cleanup, you should call the CUnitSkillTypeDlg::CloseSkillType()
  // you should apply changes only if the bApply flag is set
	virtual void CloseSkillType(BOOL bApply);

  // called before the dlg is opened
  // sets new skill type
  // the default implementation copies the pointer into the m_pSkillType;
  // you should call the CUnitSkillTypeDlg::InitSkillTypeDlg() before
  // you do some initialization for the skill
	virtual void InitSkillTypeDlg(CEUnitSkillType *pSkillType);

  // return the name of this skill type
  // (should be unique)
	virtual CString GetName();

  // deletes the object
  // here do some clean up
  // its called when the unit editor is closed
	virtual void Delete();

  // creates the object
  // here do some initialization
  // its called when the unit editor is started
	virtual void Create();

  // constructor
	CUnitSkillTypeDlg(int nID, CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CUnitSkillTypeDlg)
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CUnitSkillTypeDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

  // the skill type for current dialog
  // it will surely contain the skill type while the dialog exists
  // when the dialog is not active this will be NULL
	CEUnitSkillType * m_pSkillType;

	// Generated message map functions
	//{{AFX_MSG(CUnitSkillTypeDlg)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_UNITSKILLTYPEDLG_H__14C5FA76_99B3_11D3_A0DD_B66FF94DE631__INCLUDED_)
