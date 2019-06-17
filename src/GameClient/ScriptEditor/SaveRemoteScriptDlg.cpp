// SaveRemoteScriptDlg.cpp: implementation of the CSaveRemoteScriptDlg class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "..\gameclient.h"
#include "SaveRemoteScriptDlg.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

IMPLEMENT_DYNAMIC(CSaveRemoteScriptDlg, CDialogWindow);

BEGIN_OBSERVER_MAP(CSaveRemoteScriptDlg, CDialogWindow)
  BEGIN_NOTIFIER(IDC_ScriptList)
    EVENT(CListControl::E_SELCHANGE)
      OnSelChange(); return FALSE;
    EVENT(CListControl::E_SELECTED)
      OnSelected(); return FALSE;
  END_NOTIFIER()

  BEGIN_NOTIFIER(IDC_OK)
    ON_BUTTONCOMMAND(OnOK)
  END_NOTIFIER()
END_OBSERVER_MAP(CSaveRemoteScriptDlg, CDialogWindow)

// COnstructor
CSaveRemoteScriptDlg::CSaveRemoteScriptDlg()
{

}

// Destructor
CSaveRemoteScriptDlg::~CSaveRemoteScriptDlg()
{

}


// Debug functions
#ifdef _DEBUG

void CSaveRemoteScriptDlg::AssertValid() const
{
  CDialogWindow::AssertValid();
}

void CSaveRemoteScriptDlg::Dump(CDumpContext &dc) const
{
  CDialogWindow::Dump(dc);
}

#endif


// Creation -> Do modal
DWORD CSaveRemoteScriptDlg::DoModal(CPoint pt, CWindow *pParent)
{
  CRect rcWindow(pt.x, pt.y, pt.x + 450, pt.y + 330);

  CString strCaption;
  strCaption.LoadString(IDS_SCRIPTEDITOR_SAVEREMOTESCRIPT);

  return CDialogWindow::DoModal(rcWindow, strCaption, NULL, pParent);
}


void CSaveRemoteScriptDlg::InitDialog()
{
  CDialogWindow::InitDialog();

  CRect rcControl;
  CString strCaption;

  // the edit box - FileName
  rcControl.SetRect(15, 265, 435, 275);
  m_wndFileName.Create(rcControl, NULL, this);
  m_wndFileName.SetText(m_strFileName);

  // the text above the list (List of scripts)
  rcControl.SetRect(15, 30, 250, 35);
  strCaption.LoadString(IDS_SCRIPTEDITOR_SAVEREMOTESCRIPT_LISTTITLE);
  m_wndScriptListTitle.Create(rcControl, NULL, strCaption, this);
  m_wndScriptListTitle.SetAlign(CStaticText::AlignLeft);

  // the list of the scripts
  rcControl.SetRect(15, 50, 435, 255);
  m_wndScriptList.Create(rcControl, NULL, this, 4);
  m_wndScriptList.Connect(this, IDC_ScriptList);
  m_wndScriptList.SetColumnWidth(0, 190);
  m_wndScriptList.SetColumnWidth(1, 50);
  m_wndScriptList.SetColumnWidth(2, 25);

  // Update list itmes
  FillScriptList();

  // open button
  rcControl.SetRect(245, 292, 335, 315);
  strCaption.LoadString(IDS_SCRIPTEDITOR_SAVEREMOTESCRIPT_SAVE);
  m_wndOpen.Create(&rcControl, NULL, strCaption, this);
  m_wndOpen.Connect(this, CDialogWindow::IDC_OK);
  // set it as the default one (enter key)
  m_wndOpen.SetKeyAcc(VK_RETURN, 0);

  // cancel button
  rcControl.SetRect(345, 292, 435, 315);
  strCaption.LoadString(IDS_SCRIPTEDITOR_SAVEREMOTESCRIPT_CANCEL);
  m_wndCancel.Create(&rcControl, NULL, strCaption, this);
  m_wndCancel.Connect(this, CDialogWindow::IDC_CANCEL);

}

// Deletes the window
void CSaveRemoteScriptDlg::Delete()
{
  // delete controls
  m_wndScriptListTitle.Delete();
  m_wndScriptList.Delete();
  m_wndFileName.Delete();
  m_wndOpen.Delete();
  m_wndCancel.Delete();

  CDialogWindow::Delete();
}


// Fills the list of scripts
void CSaveRemoteScriptDlg::FillScriptList()
{
  // first fill in the local files
  CString strFileName, strSize, strTimeDate;
  DWORD dwPos = 0;
  int nItem, nFirstPos = 0;

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
}

BOOL CSaveRemoteScriptDlg::ParseNextLine(CString &strSource, DWORD &dwPos, CString &strFileName, CString &strSize, CString &strTimeDate)
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

void CSaveRemoteScriptDlg::OnSelChange()
{
  int nSelected;

  if(m_wndScriptList.GetSelectedCount() == 0){
    m_wndFileName.SetText(CString());
  }

  nSelected = m_wndScriptList.GetSelectedItem();
  m_wndFileName.SetText(m_wndScriptList.GetItemText(nSelected));
}

void CSaveRemoteScriptDlg::OnOK()
{
  // get the file name from the edit box
  CString strFileName = m_wndFileName.GetText();

  // chech it
  if(strFileName.IsEmpty()){
    CMessageBox::OK(IDS_WRONGFILENAME, IDS_SCRIPTEDITOR_SAVEREMOTESCRIPT, this);
    return;
  }

  // append the .s3 if no extension
  int nPos = strFileName.ReverseFind('.');
  if(nPos == -1)
    strFileName += ".3s";

  // try to find the file in the list
  {
    int nItem;

    for(nItem = 0; nItem < m_wndScriptList.GetItemsCount(); nItem++){
      if(strFileName.CompareNoCase(m_wndScriptList.GetItemText(nItem)) == 0){
        if(CMessageBox::YesNo(IDS_OVERWRITEFILE, IDS_SCRIPTEDITOR_SAVEREMOTESCRIPT, this) != CMessageBox::IDC_Yes)
          return;
        else
          break;
      }
    }
  }

  // everything was OK
  m_strFileName = strFileName;
  EndDialog(IDC_OK);
}

void CSaveRemoteScriptDlg::OnSelected()
{
  int nSelected;

  if(m_wndScriptList.GetSelectedCount() == 0){
    m_wndFileName.SetText(CString());
  }

  nSelected = m_wndScriptList.GetSelectedItem();
  m_wndFileName.SetText(m_wndScriptList.GetItemText(nSelected));

  OnOK();
}