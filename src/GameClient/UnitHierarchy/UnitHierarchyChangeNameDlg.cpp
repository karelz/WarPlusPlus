// UnitHierarchyChangeNameDlg.cpp: implementation of the CUnitHierarchyChangeNameDlg class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "..\gameclient.h"
#include "UnitHierarchyChangeNameDlg.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

IMPLEMENT_DYNAMIC(CUnitHierarchyChangeNameDlg, CDialogWindow)

BEGIN_OBSERVER_MAP(CUnitHierarchyChangeNameDlg, CDialogWindow)
  BEGIN_NOTIFIER(CDialogWindow::IDC_OK)
    ON_BUTTONCOMMAND(OnOK)
  END_NOTIFIER()
END_OBSERVER_MAP(CUnitHierarchyChangeNameDlg, CDialogWindow)

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CUnitHierarchyChangeNameDlg::CUnitHierarchyChangeNameDlg()
{
}

CUnitHierarchyChangeNameDlg::~CUnitHierarchyChangeNameDlg()
{
}

void CUnitHierarchyChangeNameDlg::InitDialog()
{
  CDialogWindow::InitDialog();

  CString strText;
  strText.LoadString(IDS_UNITHIERARCHY_CHANGENAME_NAME);
  CRect rcBound(20, 37, 290, 57);
  VERIFY(m_wndNameStatic.Create(rcBound, NULL, strText, this));

  rcBound.SetRect(20, 57, 290, 77);
  VERIFY(m_wndName.Create(rcBound, NULL, this));
  m_wndName.SetText(m_strName);

  strText.LoadString(IDS_OK);
  m_wndOK.Create(CPoint(140, 86), NULL, strText, this);
  m_wndOK.Connect(this, CDialogWindow::IDC_OK);
  m_wndOK.SetKeyAcc(VK_RETURN, 0);

  strText.LoadString(IDS_CANCEL);
  m_wndCancel.Create(CPoint(210, 86), NULL, strText, this);
  m_wndCancel.Connect(this, CDialogWindow::IDC_CANCEL);
  m_wndCancel.SetKeyAcc(VK_ESCAPE, 0);
}

void CUnitHierarchyChangeNameDlg::Delete()
{
  m_wndOK.Delete(); m_wndCancel.Delete();
  m_wndName.Delete(); m_wndNameStatic.Delete();

  CDialogWindow::Delete();
}

CRect CUnitHierarchyChangeNameDlg::GetDefaultPosition()
{
  return GetCenterPosition(CSize(320, 125));
}

CString CUnitHierarchyChangeNameDlg::GetDefaultCaption()
{
  CString strText;
  strText.LoadString(IDS_UNITHIERARCHY_CHANGENAME_CAPTION);
  return strText;
}

void CUnitHierarchyChangeNameDlg::OnOK()
{
  m_strName = m_wndName.GetText();
  EndDialog(CDialogWindow::IDC_OK);
}