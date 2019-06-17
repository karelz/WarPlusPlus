#if !defined(AFX_EBULLETATTACKSKILLTYPEDLG_H__0591D42F_E354_4C4A_9036_8EE9F5726B65__INCLUDED_)
#define AFX_EBULLETATTACKSKILLTYPEDLG_H__0591D42F_E354_4C4A_9036_8EE9F5726B65__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "..\UnitEditor\UnitSkillTypeDlg.h"
#include "Common\Map\MBulletAttackSkillType.h"

#include "..\Controls\AppearanceControl.h"
#include "..\Controls\ZPosControl.h"

/////////////////////////////////////////////////////////////////////////////
// CEBulletAttackSkillTypeDlg dialog

class CEBulletAttackSkillTypeDlg : public CUnitSkillTypeDlg
{
// Construction
public:
	CEBulletAttackSkillTypeDlg(CWnd* pParent = NULL);   // standard constructor
    virtual CString GetName() { return "BulletAttack"; }
	virtual void DeleteSkill(CEUnitSkillType *pSkillType);
	virtual void CloseSkillType(BOOL bApply);
	virtual void InitSkillTypeDlg(CEUnitSkillType *pSkillType);
	virtual void InitNewSkill(CEUnitSkillType *pSKillType);


// Dialog Data
	//{{AFX_DATA(CEBulletAttackSkillTypeDlg)
	enum { IDD = IDD_BULLETATTACKSKILL };
	int		m_nIntensity;
	DWORD	m_dwLength;
	DWORD	m_dwLoadTime;
	DWORD	m_dwOffset;
	DWORD	m_dwPeriod;
	DWORD	m_dwAltitudeMax;
	DWORD	m_dwAltitudeMin;
	DWORD	m_dwRadius;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CEBulletAttackSkillTypeDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CEBulletAttackSkillTypeDlg)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:
    // The bullet attack appearance control
    CAppearanceControl m_wndBulletAttackAppearance;
    DWORD m_dwBulletAttackAppearanceID;
    CZPosControl m_wndMinAltitude, m_wndMaxAltitude;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_EBULLETATTACKSKILLTYPEDLG_H__0591D42F_E354_4C4A_9036_8EE9F5726B65__INCLUDED_)
