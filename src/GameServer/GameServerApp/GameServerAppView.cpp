// GameServerAppView.cpp : implementation of the CGameServerAppView class
//

#include "stdafx.h"
#include "GameServerApp.h"

#include "GameServerAppDoc.h"
#include "GameServerAppView.h"
#include "MainFrm.h"

#include "..\GameServer\SMap.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif
	
/////////////////////////////////////////////////////////////////////////////
// CGameServerAppView

IMPLEMENT_DYNCREATE(CGameServerAppView, CTreeView)

BEGIN_MESSAGE_MAP(CGameServerAppView, CTreeView)
	//{{AFX_MSG_MAP(CGameServerAppView)
	ON_WM_CREATE()
	ON_UPDATE_COMMAND_UI(ID_PROPERTIES, OnUpdateProperties)
	ON_COMMAND(ID_PROPERTIES, OnProperties)
	ON_NOTIFY_REFLECT(NM_DBLCLK, OnDblclk)
	ON_COMMAND(ID_CIVILIZATION_ADDUSER, OnCivilizationAddUser)
	ON_UPDATE_COMMAND_UI(ID_CIVILIZATION_ADDUSER, OnUpdateCivilizationAddUser)
	ON_NOTIFY_REFLECT(NM_RCLICK, OnRClick)
    ON_COMMAND(ID_USER_DELETEUSER, OnCivilizationDeleteUser)
    ON_UPDATE_COMMAND_UI(ID_USER_DELETEUSER, OnUpdateCivilizationDeleteUser)
	ON_UPDATE_COMMAND_UI(ID_USER_DISABLE, OnUpdateUserDisable)
	ON_COMMAND(ID_USER_DISABLE, OnUserDisable)
	ON_UPDATE_COMMAND_UI(ID_USER_ENABLE, OnUpdateUserEnable)
	ON_COMMAND(ID_USER_ENABLE, OnUserEnable)
	ON_COMMAND(ID_PROPERTIES_CIVILIZATION, OnPropertiesCivilization)
	ON_COMMAND(ID_PROPERTIES_USER, OnPropertiesUser)
	ON_UPDATE_COMMAND_UI(ID_PROPERTIES_CIVILIZATION, OnUpdatePropertiesCivilization)
	ON_UPDATE_COMMAND_UI(ID_PROPERTIES_USER, OnUpdatePropertiesUser)
	ON_COMMAND(ID_USER_SENDMESSAGE, OnUserSendMessage)
    ON_UPDATE_COMMAND_UI(ID_USER_SENDMESSAGE, OnUpdateUserSendMessage)
    ON_COMMAND(ID_CIVILIZATION_SENDMESSAGE, OnCivilizationSendMessage)
    ON_UPDATE_COMMAND_UI(ID_CIVILIZATION_SENDMESSAGE, OnUpdateCivilizationSendMessage)
	ON_UPDATE_COMMAND_UI(ID_SENDMESSAGE, OnUpdateSendMessage)
	ON_COMMAND(ID_SENDMESSAGE, OnSendMessage)
  ON_MESSAGE ( WM_PROFILEUPDATED, OnProfileUpdated )
	ON_WM_DESTROY()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CGameServerAppView construction/destruction

CGameServerAppView::CGameServerAppView()
{
	// TODO: add construction code here

}

CGameServerAppView::~CGameServerAppView()
{
}

BOOL CGameServerAppView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	return CTreeView::PreCreateWindow(cs);
}

/////////////////////////////////////////////////////////////////////////////
// CGameServerAppView drawing

void CGameServerAppView::OnDraw(CDC* pDC)
{
	CGameServerAppDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	// TODO: add draw code for native data here
}

void CGameServerAppView::OnInitialUpdate()
{
	CTreeView::OnInitialUpdate();

  // Get the document
  CGameServerAppDoc *pDoc = GetDocument();

  // If there is no game -> disable us
  if(pDoc->IsEmpty()){
    // clear
    GetTreeCtrl().DeleteAllItems();
    EnableWindow(FALSE);
    pDoc->RefreshFrameState();
    return;
  }
  EnableWindow(TRUE);
  pDoc->RefreshFrameState();
}

/////////////////////////////////////////////////////////////////////////////
// CGameServerAppView diagnostics

#ifdef _DEBUG
void CGameServerAppView::AssertValid() const
{
	CTreeView::AssertValid();
}

void CGameServerAppView::Dump(CDumpContext& dc) const
{
	CTreeView::Dump(dc);
}

CGameServerAppDoc* CGameServerAppView::GetDocument() // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CGameServerAppDoc)));
	return (CGameServerAppDoc*)m_pDocument;
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CGameServerAppView message handlers

void CGameServerAppView::OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint) 
{
  // Get the document
  CGameServerAppDoc *pDoc = GetDocument();

  // If there is no game -> disable us
  if(pDoc->IsEmpty()){
    // clear
    GetTreeCtrl().DeleteAllItems();
    EnableWindow(FALSE);
    pDoc->RefreshFrameState();
    return;
  }

  EnableWindow(TRUE);
  pDoc->RefreshFrameState();

  pDoc->FillUsersTree(&GetTreeCtrl());
}

int CGameServerAppView::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CTreeView::OnCreate(lpCreateStruct) == -1)
		return -1;
	
  GetDocument ()->RegisterProfileObserver ( this );

  CBitmap bmp;
  m_ImageList.Create(16, 16, ILC_COLOR24 | ILC_MASK, 0, 1);
  bmp.LoadBitmap(IDB_USERLIST);
  m_ImageList.Add(&bmp, RGB(255, 0, 255));
  GetTreeCtrl().SetImageList(&m_ImageList, TVSIL_NORMAL);
  GetTreeCtrl().ModifyStyle(0, TVS_HASLINES | TVS_HASBUTTONS | TVS_LINESATROOT | TVS_SHOWSELALWAYS | TVS_TRACKSELECT);
  return 0;
}

void CGameServerAppView::OnUpdateProperties(CCmdUI* pCmdUI) 
{
  HTREEITEM hItem = GetTreeCtrl().GetNextItem(NULL, TVGN_CARET);
  if(hItem != NULL){
    pCmdUI->Enable(TRUE);
  }
  else{
    pCmdUI->Enable(FALSE);
  }
}


void CGameServerAppView::OnUpdatePropertiesCivilization(CCmdUI* pCmdUI) 
{
  HTREEITEM hItem = GetTreeCtrl().GetNextItem(NULL, TVGN_CARET);
  if(hItem == NULL){
    pCmdUI->Enable(FALSE);
	return;
  }
  if(GetTreeCtrl().GetParentItem(hItem) == NULL){
    pCmdUI->Enable(TRUE);
  }
  else{
    pCmdUI->Enable(FALSE);
  }
}

void CGameServerAppView::OnUpdatePropertiesUser(CCmdUI* pCmdUI) 
{
  HTREEITEM hItem = GetTreeCtrl().GetNextItem(NULL, TVGN_CARET);
  if(hItem == NULL){
    pCmdUI->Enable(FALSE);
	return;
  }
  if(GetTreeCtrl().GetParentItem(hItem) == NULL){
    pCmdUI->Enable(FALSE);
  }
  else{
    pCmdUI->Enable(TRUE);
  }
}

void CGameServerAppView::OnProperties() 
{
  HTREEITEM hItem = GetTreeCtrl().GetNextItem(NULL, TVGN_CARET);
  if(hItem == NULL) return;
  if(GetTreeCtrl().GetParentItem(hItem) == NULL){
    // Civilization
    GetDocument()->OnCivilizationProperties((CSCivilization *)(GetTreeCtrl().GetItemData(hItem)));
  }
  else{
    // User
    GetDocument()->OnUserProperties(GetTreeCtrl().GetItemData(hItem));
  }
}

void CGameServerAppView::OnPropertiesCivilization() 
{
  HTREEITEM hItem = GetTreeCtrl().GetNextItem(NULL, TVGN_CARET);
  if(hItem == NULL) return;

  GetDocument()->OnCivilizationProperties((CSCivilization *)(GetTreeCtrl().GetItemData(hItem)));
}

void CGameServerAppView::OnPropertiesUser() 
{
  HTREEITEM hItem = GetTreeCtrl().GetNextItem(NULL, TVGN_CARET);
  if(hItem == NULL) return;

  GetDocument()->OnUserProperties(GetTreeCtrl().GetItemData(hItem));
}

void CGameServerAppView::OnDblclk(NMHDR* pNMHDR, LRESULT* pResult) 
{
  HTREEITEM hItem = GetTreeCtrl().GetNextItem(NULL, TVGN_CARET);
  if(hItem == NULL) return;
  if(GetTreeCtrl().GetParentItem(hItem) == NULL){
    // Civilization
    GetDocument()->OnCivilizationSendMessage((CSCivilization *)(GetTreeCtrl().GetItemData(hItem)));
  }
  else{
    // User
    GetDocument()->OnUserSendMessage(GetTreeCtrl().GetItemData(hItem));
  }
  
	*pResult = 0;
}

void CGameServerAppView::OnCivilizationAddUser() 
{
  HTREEITEM hItem = GetTreeCtrl().GetNextItem(NULL, TVGN_CARET);
  if(hItem == NULL) return;
  if(GetTreeCtrl().GetParentItem(hItem) != NULL) return;

  // In item data is pointer to CSCivilization
  GetDocument()->OnAddUser((CSCivilization *)GetTreeCtrl().GetItemData(hItem));
}

void CGameServerAppView::OnCivilizationDeleteUser() 
{
  HTREEITEM hItem = GetTreeCtrl().GetNextItem(NULL, TVGN_CARET);
  if(hItem == NULL) return;
  if(GetTreeCtrl().GetParentItem(hItem) == NULL) return;

  // In item data is ID of the user
  GetDocument()->OnDeleteUser((DWORD)GetTreeCtrl().GetItemData(hItem));
}

void CGameServerAppView::OnUpdateCivilizationAddUser(CCmdUI* pCmdUI) 
{
  HTREEITEM hItem = GetTreeCtrl().GetNextItem(NULL, TVGN_CARET);
  if(hItem == NULL){
    pCmdUI->Enable(FALSE);
    return;
  }
  if(GetTreeCtrl().GetParentItem(hItem) == NULL){
    pCmdUI->Enable(TRUE);
  }
  else{
    pCmdUI->Enable(FALSE);
  }
}

void CGameServerAppView::OnUpdateCivilizationDeleteUser(CCmdUI* pCmdUI) 
{
  HTREEITEM hItem = GetTreeCtrl().GetNextItem(NULL, TVGN_CARET);
  if(hItem == NULL){
    pCmdUI->Enable(FALSE);
    return;
  }
  if(GetTreeCtrl().GetParentItem(hItem) == NULL){
    pCmdUI->Enable(FALSE);
  }
  else{
    pCmdUI->Enable(TRUE);
  }
}

void CGameServerAppView::OnRClick(NMHDR* pNMHDR, LRESULT* pResult) 
{
  CPoint ptScr, ptClient;
  GetCursorPos(&ptScr);
  ptClient = ptScr;
  ScreenToClient(&ptClient);

  UINT nFlags;
  HTREEITEM hItem = GetTreeCtrl().HitTest(ptClient, &nFlags);
  if(!(nFlags & TVHT_ONITEM)) return;
  if(hItem == NULL) return;
  GetTreeCtrl().SelectItem(hItem);
  if(GetTreeCtrl().GetParentItem(hItem) == NULL){
    // Civilization
    CMenu menu;
    menu.LoadMenu(IDM_CIVILIZATION);
    menu.GetSubMenu(0)->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, ptScr.x, ptScr.y, this);
  }
  else{
    // User
    CMenu menu;
    menu.LoadMenu(IDM_USER);
    menu.GetSubMenu(0)->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, ptScr.x, ptScr.y, this);
  }
  
	*pResult = 0;
}

void CGameServerAppView::OnUpdateUserDisable(CCmdUI* pCmdUI) 
{
  HTREEITEM hItem = GetTreeCtrl().GetNextItem(NULL, TVGN_CARET);
  if(hItem == NULL){
    pCmdUI->Enable(FALSE);
    return;
  }
  if(GetTreeCtrl().GetParentItem(hItem) == NULL){
    pCmdUI->Enable(FALSE);
  }
  else{
    if(GetDocument()->IsUserEnabled(GetTreeCtrl().GetItemData(hItem)))
      pCmdUI->Enable(TRUE);
    else
      pCmdUI->Enable(FALSE);
  }
}

void CGameServerAppView::OnUserDisable() 
{
  HTREEITEM hItem = GetTreeCtrl().GetNextItem(NULL, TVGN_CARET);
  if(hItem == NULL) return;
  if(GetTreeCtrl().GetParentItem(hItem) == NULL) return;

  GetDocument()->OnDisableUser(GetTreeCtrl().GetItemData(hItem));
}

void CGameServerAppView::OnUpdateUserEnable(CCmdUI* pCmdUI) 
{
  HTREEITEM hItem = GetTreeCtrl().GetNextItem(NULL, TVGN_CARET);
  if(hItem == NULL){
    pCmdUI->Enable(FALSE);
    return;
  }
  if(GetTreeCtrl().GetParentItem(hItem) == NULL){
    pCmdUI->Enable(FALSE);
  }
  else{
    if(GetDocument()->IsUserEnabled(GetTreeCtrl().GetItemData(hItem)))
      pCmdUI->Enable(FALSE);
    else
      pCmdUI->Enable(TRUE);
  }
}

void CGameServerAppView::OnUserEnable() 
{
  HTREEITEM hItem = GetTreeCtrl().GetNextItem(NULL, TVGN_CARET);
  if(hItem == NULL) return;
  if(GetTreeCtrl().GetParentItem(hItem) == NULL) return;

  GetDocument()->OnEnableUser(GetTreeCtrl().GetItemData(hItem));
}

void CGameServerAppView::OnUpdateUserSendMessage(CCmdUI *pCmdUI)
{
  if ( GetDocument()->m_eGameStatus != CGameServerAppDoc::Game_Unpaused){ 
    pCmdUI->Enable(FALSE);
	return;
  }

  HTREEITEM hItem = GetTreeCtrl().GetNextItem(NULL, TVGN_CARET);
  if(hItem == NULL){
    pCmdUI->Enable(FALSE);
    return;
  }
  if(GetTreeCtrl().GetParentItem(hItem) == NULL){
    pCmdUI->Enable(FALSE);
  }
  else
  {
	DWORD dwUserID = GetTreeCtrl().GetItemData(hItem);
	if(GetDocument()->IsUserEnabled( dwUserID) &&
	   GetDocument()->IsUserLoggedOn( dwUserID) )
	{
		pCmdUI->Enable(TRUE);
	}
    else pCmdUI->Enable(FALSE);
  }
}

void CGameServerAppView::OnUserSendMessage()
{
  HTREEITEM hItem = GetTreeCtrl().GetNextItem(NULL, TVGN_CARET);
  if(hItem == NULL) return;
  if(GetTreeCtrl().GetParentItem(hItem) == NULL) return;

  GetDocument()->OnUserSendMessage(GetTreeCtrl().GetItemData(hItem));
}

void CGameServerAppView::OnUpdateCivilizationSendMessage(CCmdUI *pCmdUI)
{
  if ( GetDocument()->m_eGameStatus != CGameServerAppDoc::Game_Unpaused){ 
    pCmdUI->Enable(FALSE);
	return;
  }

  HTREEITEM hItem = GetTreeCtrl().GetNextItem(NULL, TVGN_CARET);
  if(hItem == NULL){
    pCmdUI->Enable(FALSE);
    return;
  }
  if(GetTreeCtrl().GetParentItem(hItem) == NULL){
    pCmdUI->Enable(TRUE);
  }
  else{
    pCmdUI->Enable(FALSE);
  }
}

void CGameServerAppView::OnCivilizationSendMessage()
{
  HTREEITEM hItem = GetTreeCtrl().GetNextItem(NULL, TVGN_CARET);
  if(hItem == NULL) return;
  if(GetTreeCtrl().GetParentItem(hItem) != NULL) return;

  // In item data is pointer to CSCivilization
  GetDocument()->OnCivilizationSendMessage((CSCivilization *)GetTreeCtrl().GetItemData(hItem));
}
void CGameServerAppView::OnUpdateSendMessage(CCmdUI* pCmdUI) 
{
  if ( GetDocument()->m_eGameStatus != CGameServerAppDoc::Game_Unpaused){ 
    pCmdUI->Enable(FALSE);
	return;
  }

  HTREEITEM hItem = GetTreeCtrl().GetNextItem(NULL, TVGN_CARET);
  if(hItem == NULL){
    pCmdUI->Enable(FALSE);
    return;
  }
  
  if(GetTreeCtrl().GetParentItem(hItem) != NULL)
  {
	// user selected
  	DWORD dwUserID = GetTreeCtrl().GetItemData(hItem);
	if(GetDocument()->IsUserEnabled( dwUserID) &&
	   GetDocument()->IsUserLoggedOn( dwUserID) )
	{
		pCmdUI->Enable(TRUE);
	}
    else pCmdUI->Enable(FALSE);
  }
  else 
  {
	// civilization selected
	pCmdUI->Enable(TRUE);
  }
}

void CGameServerAppView::OnSendMessage() 
{
  HTREEITEM hItem = GetTreeCtrl().GetNextItem(NULL, TVGN_CARET);
  if(hItem == NULL) return;
  if(GetTreeCtrl().GetParentItem(hItem) != NULL){
    GetDocument()->OnUserSendMessage(GetTreeCtrl().GetItemData(hItem));
  }
  else{
    GetDocument()->OnCivilizationSendMessage((CSCivilization *)GetTreeCtrl().GetItemData(hItem));
  }
}

CString PrintInt64ByLocale ( __int64 nNumber );

LRESULT CGameServerAppView::OnProfileUpdated ( WPARAM sParam, LPARAM lParam )
{
  if ( !GetDocument ()->ShowProfilingInTheMainView () ) return 0;

  // Go through the tree and update the civilizations
  HTREEITEM hItem = GetTreeCtrl().GetNextItem ( NULL, TVGN_CHILD );

  __int64 nGlobalAll = 0, nLocalAll = 0;
  DWORD dwCivilization;
  for ( dwCivilization = 0; dwCivilization < g_cMap.GetCivilizationCount (); dwCivilization++ )
  {
    CZCivilization *pCiv = g_cMap.GetCivilization ( dwCivilization )->GetZCivilization ();
    nGlobalAll += pCiv->GetInterpret ()->GetGlobalProfileCounter ();
    nLocalAll += pCiv->GetInterpret ()->GetLocalProfileCounter ();
  }

  ((CMainFrame *)AfxGetMainWnd () )->SetInstructionCountersPane ( nLocalAll, nGlobalAll );

  while ( hItem != NULL )
  {
    // Get the profile counters
    CZCivilization * pCiv = ((CSCivilization *)GetTreeCtrl ().GetItemData ( hItem ))->GetZCivilization ();
    
    CString strText, strProfile;
    double dbPercent;
	  if ( nLocalAll == 0 )
    {
      strProfile.Format ( "n/a (%d)", nLocalAll );
    }
    else
    {
      dbPercent = ((double)(pCiv->GetInterpret ()->GetLocalProfileCounter ()) * 100) / ((double)nLocalAll);
      strProfile.Format ( "%.02f%%", dbPercent );
    }

    if ( nGlobalAll == 0 )
    {
      strText.Format ( "(n/a (%d))", nGlobalAll );
    }
    else
    {
      dbPercent = ((double)(pCiv->GetInterpret ()->GetGlobalProfileCounter () ) * 100) / ((double)nGlobalAll );
      strText.Format ( " (%.02f%%)", dbPercent );
    }
    strProfile += strText;
    strText.Format ( "  (%s/%s)", PrintInt64ByLocale ( pCiv->GetInterpret ()->GetLocalProfileCounter () ),
      PrintInt64ByLocale ( pCiv->GetInterpret ()->GetGlobalProfileCounter () ) );
    strProfile += strText;

    CString strItem = GetTreeCtrl ().GetItemText ( hItem );
    int nPos = strItem.ReverseFind ( '-' );
    if ( nPos == -1 )
    {
      strItem = strItem + " - " + strProfile;
    }
    else
    {
      strItem = strItem.Left ( nPos ) + "- " + strProfile;
    }
    GetTreeCtrl().SetItemText ( hItem, strItem );

    hItem = GetTreeCtrl().GetNextItem ( hItem, TVGN_NEXT );
  }

  return 0;
}

void CGameServerAppView::OnDestroy() 
{
	CTreeView::OnDestroy();
	
  GetDocument ()->UnregisterProfileObserver ( this );	
}

// A small helper function to nicely print large numbers
CString PrintInt64ByLocale ( __int64 nNumber )
{
  CString strNumber;
  strNumber.Format ( "%I64d", nNumber );

  char pTxt [1024];
  NUMBERFMT fmt;
  fmt.NumDigits = 0;
  fmt.LeadingZero = 0;
  fmt.Grouping = 3;
  fmt.lpDecimalSep = "";
  fmt.lpThousandSep = " ";
  fmt.NegativeOrder = 0;
  GetNumberFormat ( LOCALE_USER_DEFAULT, 0, (LPCSTR)strNumber,
    &fmt, pTxt, 1023 );
  return CString ( pTxt );
}
