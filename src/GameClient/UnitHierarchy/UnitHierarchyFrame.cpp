// UnitHierarchyFrame.cpp: implementation of the CUnitHierarchyFrame class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "..\GameClient.h"
#include "UnitHierarchyFrame.h"

#include "..\GameClientGlobal.h"

#include "UnitHierarchyChangeScriptDlg.h"
#include "UnitHierarchyChangeNameDlg.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

IMPLEMENT_DYNAMIC(CUnitHierarchyFrame, CCaptionWindow)

BEGIN_OBSERVER_MAP(CUnitHierarchyFrame, CCaptionWindow)
  BEGIN_NOTIFIER(IDC_CLOSE)
    ON_BUTTONCOMMAND(OnClose)
  END_NOTIFIER()

  BEGIN_NOTIFIER(IDC_TREE)
    EVENT(CTreeControl::E_ItemSelected)
      OnItemSelected((int)dwParam); return FALSE;
    EVENT(CTreeControl::E_ItemDeselected)
      OnItemDeselected((int)dwParam); return FALSE;
    EVENT(CTreeControl::E_ItemCollapsed)
      OnItemCollapsed((int)dwParam); return FALSE;
    EVENT(CTreeControl::E_ItemExpanded)
      OnItemExpanded((int)dwParam); return FALSE;
  END_NOTIFIER()

  BEGIN_NOTIFIER(IDC_REFRESH)
    ON_BUTTONCOMMAND(OnRefresh)
  END_NOTIFIER()

  BEGIN_NOTIFIER(IDC_CHANGESCRIPT)
    ON_BUTTONCOMMAND(OnChangeScript)
  END_NOTIFIER()

  BEGIN_NOTIFIER(IDC_CHANGENAME)
    ON_BUTTONCOMMAND(OnChangeName)
  END_NOTIFIER()

  BEGIN_NOTIFIER(IDC_HIERARCHY)
    EVENT(CUnitHierarchy::E_HierarchyUpdated)
      OnHierarchyUpdated((int)dwParam); return FALSE;
    EVENT(CUnitHierarchy::E_ChangedScriptName)
      OnChangedScriptName(dwParam); return FALSE;
    EVENT(CUnitHierarchy::E_ChangedName)
      OnChangedName(dwParam); return FALSE;
  END_NOTIFIER()
END_OBSERVER_MAP(CUnitHierarchyFrame, CCaptionWindow)

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CUnitHierarchyFrame::CUnitHierarchyFrame()
{
  m_pUnitHierarchy = NULL;
  m_eState = State_None;
}

CUnitHierarchyFrame::~CUnitHierarchyFrame()
{
  ASSERT(m_pUnitHierarchy == NULL);
}

#ifdef _DEBUG

void CUnitHierarchyFrame::AssertValid() const
{
  CCaptionWindow::AssertValid();
  ASSERT(m_pUnitHierarchy != NULL);
}

void CUnitHierarchyFrame::Dump(CDumpContext &dc) const
{
  CCaptionWindow::Dump(dc);
}

#endif

void CUnitHierarchyFrame::Create(CRect rcWindow, CWindow *pParent, CCaptionWindowLayout *pLayout, CTreeControlLayout *pTreeLayout,
                                 CUnitHierarchy *pUnitHierarchy)
{
  m_pUnitHierarchy = pUnitHierarchy;

  CString strCaption;
  strCaption.LoadString(IDS_UNITHIERARCHY_CAPTION);
  CCaptionWindow::Create(rcWindow, strCaption, pLayout, pParent, TRUE);

  // first create the close button
  {
    CDialogWindowLayout *pLayout = CLayouts::m_pDefaults->GetDialogWindowLayout();

    // create the point and button itself there
    CPoint pt(rcWindow.Width() - pLayout->m_dwCloseRightMargin,
      pLayout->m_dwCloseTopMargin);
    m_CloseButton.Create(pt, &(pLayout->m_CloseButtonLayout), this, FALSE);

    // connect it
    m_CloseButton.Connect(this, IDC_CLOSE);
  }

  DWORD dwYPos = rcWindow.Height() - 35;
  CString strText;
  // The refresh button
  strText.LoadString(IDS_UNITHIERARCHY_REFRESH);
  m_Refresh.Create(CPoint(7, dwYPos), NULL, strText, this);
  m_Refresh.Connect(this, IDC_REFRESH);
  strText.LoadString(IDS_UNITHIERARCHY_CHANGESCRIPT);
  m_ChangeScript.Create(CPoint(92, dwYPos), NULL, strText, this);
  m_ChangeScript.Connect(this, IDC_CHANGESCRIPT);
  strText.LoadString(IDS_UNITHIERARCHY_CHANGENAME);
  m_ChangeName.Create(CPoint(227, dwYPos), NULL, strText, this);
  m_ChangeName.Connect(this, IDC_CHANGENAME);

  CRect rcTree;
  GetTreeRect(&rcTree);
  m_wndTree.Create(rcTree, pTreeLayout, this, 3, TRUE);
  m_wndTree.SetColumnWidth(0, 250);
  m_wndTree.SetColumnWidth(1, 50);
  m_wndTree.SetColumnWidth(2, 200);

  m_bAvailable = TRUE;
  m_pUnitHierarchy->Connect(this, IDC_HIERARCHY);
  OnRefresh();
}

void CUnitHierarchyFrame::Delete()
{
  m_wndTree.Delete();
  m_CloseButton.Delete();

  m_Refresh.Delete(); m_ChangeScript.Delete(); m_ChangeName.Delete();
  CCaptionWindow::Delete();

  if(m_pUnitHierarchy != NULL){
    m_pUnitHierarchy->Disconnect(this);
    m_pUnitHierarchy = NULL;
  }
}

void CUnitHierarchyFrame::OnClose()
{
  if(IsVisible()) HideWindow();
  else ShowWindow();
}

void CUnitHierarchyFrame::GetTreeRect(CRect *pRect)
{
  CRect rcBound(GetWindowPosition());

  pRect->left = 7;
  pRect->top = 26;
  pRect->right = rcBound.Width() - 7;
  pRect->bottom = rcBound.Height() - 37;
}

void CUnitHierarchyFrame::OnSize(CSize size)
{
  CRect rcTree;
  GetTreeRect(&rcTree);
  m_wndTree.SetWindowPos(&rcTree);

  {
    CDialogWindowLayout *pLayout = CLayouts::m_pDefaults->GetDialogWindowLayout();
    CPoint pt(size.cx - pLayout->m_dwCloseRightMargin,
      pLayout->m_dwCloseTopMargin);
    m_CloseButton.SetWindowPosition(&pt);
  }

  {
    CPoint pt(7, size.cy - 35);
    m_Refresh.SetWindowPosition(&pt);
    pt.x = 92;
    m_ChangeScript.SetWindowPosition(&pt);
    pt.x = 227;
    m_ChangeName.SetWindowPosition(&pt);
  }

  CCaptionWindow::OnSize(size);
}

void CUnitHierarchyFrame::OnRefresh()
{
  if(!m_pUnitHierarchy->Refresh()){
    OnServerError();
    return;
  }

  m_pUnitHierarchy->Lock();
  if(!m_pUnitHierarchy->IsAvailable()){
    SetAvailable(FALSE);
  }
  else{
    SetAvailable(TRUE);
  }
  m_pUnitHierarchy->Unlock();
}

void CUnitHierarchyFrame::SetAvailable(BOOL bAvailable)
{
  if(m_bAvailable == bAvailable) return;
  m_bAvailable = bAvailable;

  if(bAvailable){
    UpdateTree();
  }
  else{
    m_wndTree.RemoveAll();
    CString strItem;    
    strItem.LoadString(IDS_UNITHIERARCHY_NOITEM);
    int nItem = m_wndTree.InsertItem(-1, -1, strItem);
    m_wndTree.SetItemData(nItem, 0);
    m_wndTree.ExpandItem(-1);
  }
}

void CUnitHierarchyFrame::UpdateTree()
{
  m_wndTree.RemoveAll();
  if(m_pUnitHierarchy->GetGeneralCommander() != NULL){
    FillTree(m_pUnitHierarchy->GetGeneralCommander(), -1);
  }
  m_wndTree.ExpandItem(-1);
}

void CUnitHierarchyFrame::FillTree(CUnitHierarchyUnit *pUnit, int nParent)
{
  // Get the unit name
  CString strName;
  strName = pUnit->GetName();

  // Get the unit type name
  CString strUnitTypeName;
  if(pUnit->GetSUnitID() == 0x0FFFFFFFF){
    strUnitTypeName.LoadString(IDS_UNITHIERARCHY_COMMANDERUNIT);
  }
  else{
    strUnitTypeName = g_pMap->GetGeneralUnitType(pUnit->GetSUnitTypeID())->GetName();
  }

  // Format the name string
  CString strItem;
  strItem.Format("%s (%s)", strName, strUnitTypeName);

  // Insert the item
  int nItem = m_wndTree.InsertItem(nParent, -1, strItem);
  m_wndTree.SetItemData(nItem, (DWORD)pUnit);

  // set the script name
  m_wndTree.SetItemText(nItem, 1, pUnit->GetScriptName());

  // set the unit ID
  CString strID;
  strID.Format("%d", pUnit->GetZUnitID());
  m_wndTree.SetItemText(nItem, 0, strID);

  // Insert all child items
  CUnitHierarchyUnit *pChildUnit = pUnit->GetFirstInferior();
  while(pChildUnit != NULL){
    FillTree(pChildUnit, nItem);

    pChildUnit = pChildUnit->GetNextSibbling();
  }

  if(pUnit->IsExpanded()){
    m_wndTree.ExpandItem(nItem);
  }
  else{
    m_wndTree.ExpandItem(nItem);
  }
}

void CUnitHierarchyFrame::OnHierarchyUpdated(int nStatus)
{
  if(nStatus == -1){
    SetWindowCursor(NULL);
    EnableWindow(TRUE);
    m_eState = State_None;
    OnServerError();
    return;
  }
  m_pUnitHierarchy->Lock();
  SetAvailable(TRUE);
  m_pUnitHierarchy->Unlock();
}

void CUnitHierarchyFrame::OnServerError()
{
  m_wndTree.RemoveAll();
  CString str;
  str.LoadString(IDS_UNITHIERARCHY_SERVERERROR);
  int nItem = m_wndTree.AddItem(-1, str);
  m_wndTree.SetItemData(nItem, 0);
}

void CUnitHierarchyFrame::OnChangeScript()
{
  CUnitHierarchyChangeScriptDlg dlg;

  if(m_wndTree.GetSelectedCount() == 0) return;
  if(m_eState != State_None) return;

  if(!m_bAvailable) return;
  int nItem = m_wndTree.GetSelectedItem();
  ASSERT(nItem != -1);
  CUnitHierarchyUnit *pUnit = (CUnitHierarchyUnit *)m_wndTree.GetItemData(nItem);
  if(pUnit == NULL){
    return;
  }
  dlg.m_strScriptName = m_wndTree.GetItemText(nItem, 1);

  if ( dlg.DoModal(this) != CDialogWindow::IDC_OK )
  {
	  return;
  }

  m_pUnitHierarchy->Lock();

  DWORD *pZUnitIDs = new DWORD[m_wndTree.GetSelectedCount()];

  int nCount = 0;
  nItem = m_wndTree.GetSelectedItem();
  while(nItem != -1){
    pUnit = (CUnitHierarchyUnit *)m_wndTree.GetItemData(nItem);
    pZUnitIDs[nCount] = pUnit->GetZUnitID();

    nItem = m_wndTree.GetNextSelectedItem(nItem);
    nCount++;
  }
  ASSERT(nCount == m_wndTree.GetSelectedCount());

  if(!m_pUnitHierarchy->ChangeScript(dlg.m_strScriptName, pZUnitIDs, nCount))
  {
    OnServerError();
    m_pUnitHierarchy->Unlock();
    delete pZUnitIDs;
    return;
  }

  SetWindowCursor(g_pMouse->GetWaitingCursor());
  EnableWindow(FALSE);
  m_eState = State_ScriptChange;

  delete pZUnitIDs;

  m_pUnitHierarchy->Unlock();
}

void CUnitHierarchyFrame::OnChangeName()
{
  if(m_wndTree.GetSelectedCount() == 0) return;
  if(m_eState != State_None) return;
  if(!m_bAvailable) return;

  CUnitHierarchyChangeNameDlg dlg;

  m_pUnitHierarchy->Lock();
  int nItem = m_wndTree.GetSelectedItem();
  ASSERT(nItem != -1);
  CUnitHierarchyUnit *pUnit;
  pUnit = (CUnitHierarchyUnit *)m_wndTree.GetItemData(nItem);
  if(pUnit == NULL){
    m_pUnitHierarchy->Unlock();
    return;
  }
  dlg.m_strName = pUnit->GetName();
  m_pUnitHierarchy->Unlock();

  dlg.DoModal(this);

  m_pUnitHierarchy->Lock();

  DWORD *pZUnitIDs = new DWORD[m_wndTree.GetSelectedCount()];

  int nCount = 0;
  nItem = m_wndTree.GetSelectedItem();
  while(nItem != -1){
    pUnit = (CUnitHierarchyUnit *)m_wndTree.GetItemData(nItem);
    pZUnitIDs[nCount] = pUnit->GetZUnitID();

    nItem = m_wndTree.GetNextSelectedItem(nItem);
    nCount++;
  }
  ASSERT(nCount == m_wndTree.GetSelectedCount());

  if(!m_pUnitHierarchy->ChangeName(dlg.m_strName, pZUnitIDs, nCount)){
    OnServerError();
    m_pUnitHierarchy->Unlock();
    delete pZUnitIDs;
    return;
  }
  SetWindowCursor(g_pMouse->GetWaitingCursor());
  EnableWindow(FALSE);
  m_eState = State_NameChange;

  delete pZUnitIDs;

  m_pUnitHierarchy->Unlock();
} 

void CUnitHierarchyFrame::OnItemCollapsed(int nItem)
{
  m_pUnitHierarchy->Lock();
  CUnitHierarchyUnit *pUnit = (CUnitHierarchyUnit *)m_wndTree.GetItemData(nItem);
  pUnit->SetExpanded(FALSE);
  m_pUnitHierarchy->Unlock();
}

void CUnitHierarchyFrame::OnItemExpanded(int nItem)
{
  m_pUnitHierarchy->Lock();
  CUnitHierarchyUnit *pUnit = (CUnitHierarchyUnit *)m_wndTree.GetItemData(nItem);
  pUnit->SetExpanded(TRUE);
  m_pUnitHierarchy->Unlock();
}

void CUnitHierarchyFrame::OnItemSelected(int nItem)
{
  m_pUnitHierarchy->Lock();
  CUnitHierarchyUnit *pUnit = (CUnitHierarchyUnit *)m_wndTree.GetItemData(nItem);
  pUnit->SetSelected(TRUE);
  m_pUnitHierarchy->Unlock();
}

void CUnitHierarchyFrame::OnItemDeselected(int nItem)
{
  m_pUnitHierarchy->Lock();
  CUnitHierarchyUnit *pUnit = (CUnitHierarchyUnit *)m_wndTree.GetItemData(nItem);
  pUnit->SetSelected(FALSE);
  m_pUnitHierarchy->Unlock();
}

void CUnitHierarchyFrame::OnChangedScriptName(DWORD dwErrors)
{
  ASSERT(m_eState == State_ScriptChange);

  m_eState = State_None;
  SetWindowCursor(NULL);
  EnableWindow(TRUE);

  CString strMsg;
  if(dwErrors > 0){
    strMsg.Format(IDS_UNITHIERARCHY_CHANGEERROR, dwErrors);
  }
  else{
    strMsg.LoadString(IDS_UNITHIERARCHY_CHANGESUCCESS);
  }
  CString strCaption;
  strCaption.LoadString(IDS_UNITHIERARCHY_CHANGESCRIPT_CAPTION);
  CMessageBox::OK(strMsg, strCaption, this);

  UpdateTree();
}

void CUnitHierarchyFrame::OnChangedName(DWORD dwErrors)
{
  ASSERT(m_eState == State_NameChange);

  m_eState = State_None;
  SetWindowCursor(NULL);
  EnableWindow(TRUE);

  CString strMsg;
  if(dwErrors > 0){
    strMsg.Format(IDS_UNITHIERARCHY_CHANGEERROR, dwErrors);
  }
  else{
    strMsg.LoadString(IDS_UNITHIERARCHY_CHANGESUCCESS);
  }
  CString strCaption;
  strCaption.LoadString(IDS_UNITHIERARCHY_CHANGENAME_CAPTION);
  CMessageBox::OK(strMsg, strCaption, this);

  UpdateTree();
}