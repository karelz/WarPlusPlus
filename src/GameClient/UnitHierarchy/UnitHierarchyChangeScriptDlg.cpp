// UnitHierarchyChangeScriptDlg.cpp: implementation of the CUnitHierarchyChangeScriptDlg class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "..\gameclient.h"
#include "UnitHierarchyChangeScriptDlg.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

IMPLEMENT_DYNAMIC(CUnitHierarchyChangeScriptDlg, CDialogWindow)

BEGIN_OBSERVER_MAP(CUnitHierarchyChangeScriptDlg, CDialogWindow)
  BEGIN_NOTIFIER(CDialogWindow::IDC_OK)
    ON_BUTTONCOMMAND(OnOK)
  END_NOTIFIER()
END_OBSERVER_MAP(CUnitHierarchyChangeScriptDlg, CDialogWindow)

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CUnitHierarchyChangeScriptDlg::CUnitHierarchyChangeScriptDlg()
{
}

CUnitHierarchyChangeScriptDlg::~CUnitHierarchyChangeScriptDlg()
{
}

void CUnitHierarchyChangeScriptDlg::InitDialog()
{
  CString strText;
  strText.LoadString(IDS_UNITHIERARCHY_CHANGESCRIPT_SCRIPTNAME);
  CRect rcBound(20, 37, 290, 57);
  VERIFY(m_wndScriptNameStatic.Create(rcBound, NULL, strText, this));

  rcBound.SetRect(20, 57, 290, 77);
  VERIFY(m_wndScriptName.Create(rcBound, NULL, this));
  m_wndScriptName.SetText(m_strScriptName);

  strText.LoadString(IDS_OK);
  m_wndOK.Create(CPoint(140, 86), NULL, strText, this);
  m_wndOK.Connect(this, CDialogWindow::IDC_OK);
  m_wndOK.SetKeyAcc(VK_RETURN, 0);

  strText.LoadString(IDS_CANCEL);
  m_wndCancel.Create(CPoint(210, 86), NULL, strText, this);
  m_wndCancel.Connect(this, CDialogWindow::IDC_CANCEL);
  m_wndCancel.SetKeyAcc(VK_ESCAPE, 0);
}

void CUnitHierarchyChangeScriptDlg::Delete()
{
  m_wndOK.Delete(); m_wndCancel.Delete();
  m_wndScriptName.Delete(); m_wndScriptNameStatic.Delete();

  CDialogWindow::Delete();
}

CRect CUnitHierarchyChangeScriptDlg::GetDefaultPosition()
{
  return GetCenterPosition(CSize(320, 125));
}

CString CUnitHierarchyChangeScriptDlg::GetDefaultCaption()
{
  CString strText;
  strText.LoadString(IDS_UNITHIERARCHY_CHANGESCRIPT_CAPTION);
  return strText;
}

void CUnitHierarchyChangeScriptDlg::OnOK()
{
  m_strScriptName = m_wndScriptName.GetText();
  EndDialog(CDialogWindow::IDC_OK);
}