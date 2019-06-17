// OpenLocalScriptDlg.cpp: implementation of the COpenLocalScriptDlg class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "..\gameclient.h"
#include "OpenLocalScriptDlg.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

IMPLEMENT_DYNAMIC(COpenLocalScriptDlg, CDialogWindow)

BEGIN_OBSERVER_MAP(COpenLocalScriptDlg, CDialogWindow)
  BEGIN_NOTIFIER(CDialogWindow::IDC_OK)
    ON_BUTTONCOMMAND(OnOK)
  END_NOTIFIER()
END_OBSERVER_MAP(COpenLocalScriptDlg, CDialogWindow)

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

COpenLocalScriptDlg::COpenLocalScriptDlg()
{

}

COpenLocalScriptDlg::~COpenLocalScriptDlg()
{

}

void COpenLocalScriptDlg::InitDialog()
{
  CDialogWindow::InitDialog();

  CString strText;
  strText.LoadString(IDS_SCRIPTEDITOR_OPENLOCALSCRIPT_FILENAME);
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

void COpenLocalScriptDlg::Delete()
{
  m_wndOK.Delete(); m_wndCancel.Delete();
  m_wndName.Delete(); m_wndNameStatic.Delete();

  CDialogWindow::Delete();
}

CRect COpenLocalScriptDlg::GetDefaultPosition()
{
  return GetCenterPosition(CSize(320, 125));
}

CString COpenLocalScriptDlg::GetDefaultCaption()
{
  CString strText;
  strText.LoadString(IDS_SCRIPTEDITOR_OPENLOCALSCRIPT_CAPTION);
  return strText;
}

void COpenLocalScriptDlg::OnOK()
{
  m_strName = m_wndName.GetText();
  EndDialog(CDialogWindow::IDC_OK);
}