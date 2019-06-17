// OpenRemoteScriptDlg.cpp: implementation of the COpenRemoteScriptDlg class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "..\gameclient.h"
#include "OpenRemoteScriptDlg.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

IMPLEMENT_DYNAMIC(COpenRemoteScriptDlg, CDialogWindow)

BEGIN_OBSERVER_MAP(COpenRemoteScriptDlg, CDialogWindow)
  BEGIN_NOTIFIER(IDC_ScriptList)
    EVENT(CListControl::E_SELCHANGE)
      OnSelChange(); return FALSE;
	EVENT(CListControl::E_SELECTED)
      OnSelected(); return FALSE;
  END_NOTIFIER()

  BEGIN_NOTIFIER(IDC_ToggleGlobals)
    ON_BUTTONCOMMAND(OnToggleGlobals);
  END_NOTIFIER()

  BEGIN_NOTIFIER(IDC_OK)
    ON_BUTTONCOMMAND(OnOK);
  END_NOTIFIER()
END_OBSERVER_MAP(COpenRemoteScriptDlg, CDialogWindow)

BOOL COpenRemoteScriptDlg::m_bShowGlobals = FALSE;

COpenRemoteScriptDlg::COpenRemoteScriptDlg()
{
}

COpenRemoteScriptDlg::~COpenRemoteScriptDlg()
{

}

#ifdef _DEBUG

void COpenRemoteScriptDlg::AssertValid() const
{
  CDialogWindow::AssertValid();
}

void COpenRemoteScriptDlg::Dump(CDumpContext &dc) const
{
  CDialogWindow::Dump(dc);
}

#endif


DWORD COpenRemoteScriptDlg::DoModal(CPoint pt, CWindow *pParent)
{
  CRect rcWindow(pt.x, pt.y, pt.x + 450, pt.y + 330);

  CString strCaption;
  strCaption.LoadString(IDS_SCRIPTEDITOR_OPENREMOTESCRIPT);

  return CDialogWindow::DoModal(rcWindow, strCaption, NULL, pParent);
}

void COpenRemoteScriptDlg::InitDialog()
{
  CDialogWindow::InitDialog();

  CRect rcControl;
  CString strCaption;

  // the edit box - FileName
  rcControl.SetRect(15, 265, 435, 275);
  m_wndFileName.Create(rcControl, NULL, this);

  // the text above the list (List of scripts)
  rcControl.SetRect(15, 30, 250, 35);
  strCaption.LoadString(IDS_SCRIPTEDITOR_OPENREMOTESCRIPT_LISTTITLE);
  m_wndScriptListTitle.Create(rcControl, NULL, strCaption, this);
  m_wndScriptListTitle.SetAlign(CStaticText::AlignLeft);

  // the list of the scripts
  rcControl.SetRect(15, 50, 435, 255);
  m_wndScriptList.Create(rcControl, NULL, this, 4);
  m_wndScriptList.Connect(this, IDC_ScriptList);
  m_wndScriptList.SetColumnWidth(0, 190);
  m_wndScriptList.SetColumnWidth(1, 50);
  m_wndScriptList.SetColumnWidth(2, 25);

  // the toggle globals button
  rcControl.SetRect(250, 25, 435, 48);
  m_wndToggleGlobals.Create(&rcControl, NULL, CString(), this);
  m_wndToggleGlobals.Connect(this, IDC_ToggleGlobals);
  SetToggleGlobalsText();

  // Update list itmes
  FillScriptList();

  // open button
  rcControl.SetRect(245, 292, 335, 315);
  strCaption.LoadString(IDS_SCRIPTEDITOR_OPENREMOTESCRIPT_OPEN);
  m_wndOpen.Create(&rcControl, NULL, strCaption, this);
  m_wndOpen.Connect(this, CDialogWindow::IDC_OK);
  // set it as the default one (enter key)
  m_wndOpen.SetKeyAcc(VK_RETURN, 0);

  // cancel button
  rcControl.SetRect(345, 292, 435, 315);
  strCaption.LoadString(IDS_SCRIPTEDITOR_OPENREMOTESCRIPT_CANCEL);
  m_wndCancel.Create(&rcControl, NULL, strCaption, this);
  m_wndCancel.Connect(this, CDialogWindow::IDC_CANCEL);
}

void COpenRemoteScriptDlg::Delete()
{
  // delete controls
  m_wndScriptListTitle.Delete();
  m_wndScriptList.Delete();
  m_wndFileName.Delete();
  m_wndOpen.Delete();
  m_wndCancel.Delete();
  m_wndToggleGlobals.Delete();

  CDialogWindow::Delete();
}

void COpenRemoteScriptDlg::FillScriptList()
{
  // first fill in the local files
  CString strFileName, strSize, strTimeDate;
  DWORD dwPos = 0;
  int nItem, nFirstPos = 0;

  // remove all items
  m_wndScriptList.RemoveAll();

  // fill in locals
  while(ParseNextLine(m_strLocalList, dwPos, strFileName, strSize, strTimeDate)){
    // find first item with greater text
    {
      int nI;

      nItem = m_wndScriptList.GetItemsCount();;
      for(nI = nFirstPos; nI < m_wndScriptList.GetItemsCount(); nI++){
        if(strFileName.CompareNoCase(m_wndScriptList.GetItemText(nI)) <= 0){
          nItem = nI;
          break;
        }
      }
    }

    // insert the item
    nItem = m_wndScriptList.InsertItem(nItem, strFileName);
    m_wndScriptList.SetItemText(nItem, 0, strSize);
    m_wndScriptList.SetItemText(nItem, 1, "L");
    m_wndScriptList.SetItemText(nItem, 2, strTimeDate);
  }

  // now fill in globals if they're visible
  if(m_bShowGlobals){
    dwPos = 0; nFirstPos = m_wndScriptList.GetItemsCount();
    while(ParseNextLine(m_strGlobalList, dwPos, strFileName, strSize, strTimeDate)){
      // find first item with greater text
      {
        int nI;
        
        nItem = m_wndScriptList.GetItemsCount();;
        for(nI = nFirstPos; nI < m_wndScriptList.GetItemsCount(); nI++){
          if(strFileName.CompareNoCase(m_wndScriptList.GetItemText(nI)) <= 0){
            nItem = nI;
            break;
          }
        }
      }
      
      // insert the item
      nItem = m_wndScriptList.InsertItem(nItem, strFileName);
      m_wndScriptList.SetItemText(nItem, 0, strSize);
      m_wndScriptList.SetItemText(nItem, 1, "G");
      m_wndScriptList.SetItemText(nItem, 2, strTimeDate);
    }
  }
}

BOOL COpenRemoteScriptDlg::ParseNextLine(CString &strSource, DWORD &dwPos, CString &strFileName, CString &strSize, CString &strTimeDate)
{
  // get the line first
  int nLineEnd;
  nLineEnd = strSource.Find('|', dwPos);

  CString strLine;
  if(nLineEnd >= 0){
    strLine = strSource.Mid(dwPos, nLineEnd - dwPos);
    dwPos = (DWORD)nLineEnd + 1;
  }
  else{
    strLine = strSource.Mid(dwPos);
    dwPos = strSource.GetLength();
  }

  // parse the line -> find backslashes
  int nPos, nCurPos = 0;

  nPos = strLine.Find('\\', nCurPos);
  if(nPos == -1) return FALSE;
  strFileName = strLine.Left(nPos);
  nCurPos = nPos + 1;

  nPos = strLine.Find('\\', nCurPos);
  if(nPos == -1) return FALSE;
  strSize = strLine.Mid(nCurPos, nPos - nCurPos);
  nCurPos = nPos + 1;

  nPos = strLine.Find('\\', nCurPos);
  if(nPos == -1) return FALSE;
  strTimeDate = strLine.Mid(nCurPos, nPos - nCurPos);
  nCurPos = nPos + 1;
  strTimeDate += " ";
  strTimeDate += strLine.Mid(nCurPos);

  return TRUE;
}

void COpenRemoteScriptDlg::OnSelChange()
{
  int nSelected;

  nSelected = m_wndScriptList.GetSelectedItem();
  if(nSelected == -1){
    m_wndFileName.SetText(CString());
    return;
  }
  m_wndFileName.SetText(m_wndScriptList.GetItemText(nSelected));
}

void COpenRemoteScriptDlg::OnSelected()
{
  int nSelected;

  nSelected = m_wndScriptList.GetSelectedItem();
  if(nSelected == -1){
    m_wndFileName.SetText(CString());
    return;
  }
  if(m_wndScriptList.GetItemText(nSelected, 1) == "L"){
    OnOK(1);
  }
  else{
    OnOK(2);
  }
}

void COpenRemoteScriptDlg::SetToggleGlobalsText()
{
  CString strCaption;
  if(m_bShowGlobals){
    strCaption.LoadString(IDS_SCRIPTEDITOR_OPENREMOTESCRIPT_HIDEGLOBALS);
  }
  else{
    strCaption.LoadString(IDS_SCRIPTEDITOR_OPENREMOTESCRIPT_SHOWGLOBALS);
  }
  m_wndToggleGlobals.SetText(strCaption);
}

void COpenRemoteScriptDlg::OnToggleGlobals()
{
  // first toggle the Boolean
  m_bShowGlobals = ~m_bShowGlobals;

  // now refill the list
  FillScriptList();

  // now set the new button text
  SetToggleGlobalsText();
}

void COpenRemoteScriptDlg::OnOK(int nLocalAdvise)
{
  // get the file name from the edit box
  CString strFileName = m_wndFileName.GetText();

  // chech it
  if(strFileName.IsEmpty()){
    CMessageBox::OK(IDS_WRONGFILENAME, IDS_SCRIPTEDITOR_OPENREMOTESCRIPT, this);
    return;
  }

  // append the .s3 if no extension
  int nPos = strFileName.ReverseFind('.');
  if(nPos == -1)
    strFileName += ".3s";

  m_wndFileName.SetText(strFileName);

  // refill the list with globals
  BOOL bG = m_bShowGlobals;
  m_bShowGlobals = TRUE;

  FillScriptList();

  m_bShowGlobals = bG;

  // try to find the item in the list
  {
    int nItem;
    
    for(nItem = 0; nItem < m_wndScriptList.GetItemsCount(); nItem++){
      if(strFileName.CompareNoCase(m_wndScriptList.GetItemText(nItem)) == 0){
        m_strFileName = strFileName;
        if(m_wndScriptList.GetItemText(nItem, 1) == "L"){
          if(nLocalAdvise == 2) // it means global is required
            continue;
          m_bLocalScript = TRUE;
        }
        else{
          if(nLocalAdvise == 1) // it means local is required
            continue;
          m_bLocalScript = FALSE;
        }
        
        // everything was OK
        m_strFileName = strFileName;
        EndDialog(IDC_OK);
        return;
      }
    }
  }

  FillScriptList();

  CMessageBox::OK(IDS_WRONGFILENAME, IDS_SCRIPTEDITOR_OPENREMOTESCRIPT, this);
}