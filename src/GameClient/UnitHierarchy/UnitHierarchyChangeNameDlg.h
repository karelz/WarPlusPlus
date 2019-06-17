// UnitHierarchyChangeNameDlg.h: interface for the CUnitHierarchyChangeNameDlg class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_UNITHIERARCHYCHANGENAMEDLG_H__34B13451_80F0_11D4_B0FA_004F49068BD6__INCLUDED_)
#define AFX_UNITHIERARCHYCHANGENAMEDLG_H__34B13451_80F0_11D4_B0FA_004F49068BD6__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CUnitHierarchyChangeNameDlg : public CDialogWindow  
{
  DECLARE_OBSERVER_MAP(CUnitHierarchyChangeNameDlg);
  DECLARE_DYNAMIC(CUnitHierarchyChangeNameDlg);
public:
	CUnitHierarchyChangeNameDlg();
	virtual ~CUnitHierarchyChangeNameDlg();

  virtual void InitDialog();
  virtual void Delete();

  CString m_strName;

protected:
  CRect GetDefaultPosition();
  CString GetDefaultCaption();

  void OnOK();

private:
  CStaticText m_wndNameStatic;
  CEditBox m_wndName;

  CTextButton m_wndOK;
  CTextButton m_wndCancel;
};

#endif // !defined(AFX_UNITHIERARCHYCHANGENAMEDLG_H__34B13451_80F0_11D4_B0FA_004F49068BD6__INCLUDED_)
