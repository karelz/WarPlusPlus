// ListControl.cpp: implementation of the CListControl class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ListControl.h"
#include "..\..\Keyboard.h"
#include "..\..\Mouse.h"
#include "..\Layouts.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

IMPLEMENT_DYNAMIC(CListControl, CFrameWindow)

BEGIN_OBSERVER_MAP(CListControl, CFrameWindow)
  BEGIN_NOTIFIER(ScrollID)
    EVENT(CListControlScroll::E_ACTIVATED)
      OnScrollActivated();
      return FALSE;
    ON_STEPUP(OnStepUp)
    ON_STEPDOWN(OnStepDown)
    ON_PAGEUP(OnPageUp)
    ON_PAGEDOWN(OnPageDown)
    ON_POSITION(OnPosition)
  END_NOTIFIER()

  BEGIN_ANIMS()
    ON_ANIMSREPAINT()
  END_ANIMS()

  BEGIN_KEYBOARD()
    ON_KEYDOWN()
    ON_KEYUP()
  END_KEYBOARD()

  BEGIN_MOUSE()
    ON_LBUTTONDOWN()
    ON_LBUTTONUP()
    ON_LBUTTONDBLCLK()
    ON_MOUSEMOVE()
  END_MOUSE()
END_OBSERVER_MAP(CListControl, CFrameWindow)

CListControl::CListControl()
{
  m_pLayout = NULL;
  m_aColumnWidths = NULL;
}

CListControl::~CListControl()
{

}

#ifdef _DEBUG

void CListControl::AssertValid() const
{
  CFrameWindow::AssertValid();

  ASSERT_VALID(m_pLayout);
}

void CListControl::Dump(CDumpContext &dc) const
{
  CFrameWindow::Dump(dc);
}

#endif

BOOL CListControl::Create(CRect &rcBound, CListControlLayout *pLayout, CWindow *pParent, DWORD dwColumns, BOOL bMultiSelect)
{
  if(pLayout == NULL){
    m_pLayout = CLayouts::m_pDefaults->GetListControlLayout();
  }
  else{
    m_pLayout = pLayout;
  }
  ASSERT_VALID(m_pLayout);

  ASSERT(dwColumns >= 1);

  CRect rcScroll;

  m_dwColumns = dwColumns;
  m_aColumnWidths = new DWORD[dwColumns];

  m_bMultiSelect = bMultiSelect;
  m_bMouseDrag = FALSE;

  m_nFirstVisible = 0;
  m_nLastVisible = 0;
  m_nCaretPosition = 0;
  m_nSelectedItems = 0;

  // we have the tabstop -> we want the keyboard input
  m_bTabStop = TRUE;
  // also we want doubleclicks
  m_bDoubleClk = TRUE;

  m_Selection.Create(m_pLayout->m_pSelection, this);
  m_Selection.Play();
  m_Focus.Create(m_pLayout->m_pFocus, this);
  m_SelectionFocus.Create(m_pLayout->m_pSelectionFocus, this);

  // small down the window -> some scroll bar
  rcBound.right -= m_pLayout->m_ScrollControlLayout.GetWidth();

  if(!CFrameWindow::Create(rcBound, m_pLayout, pParent, FALSE))
    return FALSE;

  rcScroll.top = rcBound.top; rcScroll.bottom = rcBound.bottom;
  rcScroll.left = rcBound.right; rcScroll.right = rcBound.right;

  if(!m_Scroll.Create(rcScroll, &m_pLayout->m_ScrollControlLayout, pParent))
    return FALSE;

  m_Scroll.Connect(this, ScrollID);

  return TRUE;
}

void CListControl::Delete()
{
  m_Scroll.Delete();
  CFrameWindow::Delete();

  m_pLayout = NULL;

  int i;
  for(i = 0; i < m_aItems.GetSize(); i++){
    m_aItems[i]->Delete();
    delete m_aItems[i];
  }
  m_aItems.RemoveAll();

  m_Selection.Stop();

  if(m_aColumnWidths != NULL){
    delete m_aColumnWidths;
    m_aColumnWidths = NULL;
  }
}

CListControl::CItem::CItem()
{
  m_pControl = NULL;
  m_dwData = 0;
  m_eState = NonSelected;
  m_dwHeight = 0;
}

CListControl::CItem::~CItem()
{
  Delete();
}

void CListControl::CItem::Create(CListControl *pControl)
{
  m_pControl = pControl;
  m_dwHeight = m_pControl->m_pLayout->GetFont()->GetCharSize('A').cy;
  m_aSubItems.SetSize(m_pControl->m_dwColumns);
}

void CListControl::CItem::Delete()
{
  m_pControl = NULL;
  m_strText.Empty();
}

CListControl::CItem::EState CListControl::CItem::GetState()
{
  return m_eState;
}

void CListControl::CItem::SetState(EState eState)
{
  m_eState = eState;
}

void CListControl::CItem::SetText(CString strText)
{
  m_strText = strText;
}

void CListControl::CItem::SetText(DWORD dwSubItem, CString strText)
{
  ASSERT(dwSubItem < m_pControl->m_dwColumns - 1);
  m_aSubItems[dwSubItem] = strText;
}

DWORD CListControl::CItem::GetTextColor(CDDrawSurface **pBkg)
{
  DWORD dwColor;
 
  switch(m_eState){
  case Selected:
    dwColor = m_pControl->m_pLayout->GetSelectedTextColor();
    *pBkg = m_pControl->m_Selection.Frame();
    break;
  case Focus:
    dwColor = m_pControl->m_pLayout->GetFocusTextColor();
    *pBkg = m_pControl->m_Focus.Frame();
    break;
  case SelectedFocus:
    dwColor = m_pControl->m_pLayout->GetFocusTextColor();
    *pBkg = m_pControl->m_SelectionFocus.Frame();
    break;
  case NonSelected:
  default:
    dwColor = m_pControl->m_pLayout->GetNormalTextColor();
    break;
  }
  return dwColor;
}

void CListControl::CItem::Draw(CDDrawSurface *pSurface, CRect *pRectBound)
{
  CDDrawSurface *pBkg = NULL;
  DWORD dwColor = GetTextColor(&pBkg);

  DrawBackground(pSurface, pBkg, pRectBound);

  // first draw the item text
  m_pControl->m_pLayout->GetFont()->PaintText(0, 0, m_strText, pSurface,
    dwColor);

  // then draw the subitems text
  {
    DWORD dwSubItem;
    DWORD dwXPos = m_pControl->m_aColumnWidths[0];

    for(dwSubItem = 0; dwSubItem < m_pControl->m_dwColumns - 1; dwSubItem++){
      m_pControl->m_pLayout->GetFont()->PaintText(dwXPos, 0, m_aSubItems[dwSubItem], pSurface, dwColor);
      dwXPos += m_pControl->m_aColumnWidths[dwSubItem + 1];
    }
  }
}

void CListControl::CItem::DrawBackground(CDDrawSurface *pSurface, CDDrawSurface *pBkg, CRect *pRectBound)
{
  if(pBkg != NULL){
    int nXTile, nYTile, x, y;
    CSize sizeBkg(pBkg->GetWidth(), pBkg->GetHeight());
    nXTile = pRectBound->Width() / sizeBkg.cx + 1;
    nYTile = pRectBound->Height() / sizeBkg.cy + 1;

    for(y = 0; y < nYTile; y++){
      for(x = 0; x < nXTile; x++){
        pSurface->Paste(x * sizeBkg.cx, y * sizeBkg.cy, pBkg);
      }
    }
  }
}

void CListControl::CItem::SetFocus(BOOL bFocus)
{
  if(bFocus){
    if(m_eState == Selected) m_eState = SelectedFocus;
    if(m_eState == NonSelected) m_eState = Focus;
  }
  else{
    if(m_eState == Focus) m_eState = NonSelected;
    if(m_eState == SelectedFocus) m_eState = Selected;
  }
}

BOOL CListControl::CItem::HasFocus()
{
  if(m_eState == Focus) return TRUE;
  if(m_eState == SelectedFocus) return TRUE;
  return FALSE;
}

void CListControl::CItem::SetSelected(BOOL bSelected)
{
  if(bSelected){
    if(m_eState == NonSelected) m_eState = Selected;
    if(m_eState == Focus) m_eState = SelectedFocus;
  }
  else{
    if(m_eState == Selected) m_eState = NonSelected;
    if(m_eState == SelectedFocus) m_eState = Focus;
  }
}

BOOL CListControl::CItem::IsSelected()
{
  if(m_eState == Selected) return TRUE;
  if(m_eState == SelectedFocus) return TRUE;
  return FALSE;
}

void CListControl::Draw(CDDrawSurface *pSurface, CRect *pRectBound)
{
  CFrameWindow::Draw(pSurface, pRectBound);

  int i;
  int nYPos;
  CRect rcItem, rcOldClip, rcInner, rcHlp;
  CPoint ptPrevTrans;

  rcInner.SetRect(m_pLayout->m_dwLeftMargin,
    m_pLayout->m_dwTopMargin,
    GetClientRect()->right - m_pLayout->m_dwRightMargin,
    GetClientRect()->bottom - m_pLayout->m_dwBottomMargin);
  nYPos = rcInner.top;

  if(m_aItems.GetSize() > 0){
    ptPrevTrans = pSurface->GetTransformation();
    pSurface->GetClipRect(&rcOldClip);
    for(i = m_nFirstVisible; i <= m_nLastVisible; i++){
      rcItem.SetRect(rcInner.left, nYPos, rcInner.right, nYPos + m_aItems[i]->GetHeight());
      if(rcHlp.IntersectRect(&rcItem, pRectBound)){
        rcItem.IntersectRect(&rcInner, &rcItem);
        rcItem.OffsetRect(-rcInner.left, -nYPos);
        pSurface->SetTransformation(CPoint(rcInner.left + ptPrevTrans.x,
          nYPos + ptPrevTrans.y));
        pSurface->SetClipRect(&rcItem, NULL);
        m_aItems[i]->Draw(pSurface, &rcItem);
      }
      nYPos += m_aItems[i]->GetHeight();
    }
    pSurface->SetClipRect(&rcOldClip, NULL);
    pSurface->SetTransformation(ptPrevTrans);
  }
}

void CListControl::UpdateScroll()
{
  DWORD dwSizeAll, dwSizeVisible, dwPrevItemsSize;
  int i, nItemsCount = m_aItems.GetSize();

  dwSizeVisible = GetClientRect()->Height() - m_pLayout->m_dwTopMargin -
    m_pLayout->m_dwBottomMargin;

  for(dwSizeAll = 0, i = 0; i < nItemsCount; i++){
    dwSizeAll += m_aItems[i]->GetHeight();
  }

  m_Scroll.SetRange(0, dwSizeAll);
  m_Scroll.SetPageSize(dwSizeVisible);
  for(dwPrevItemsSize = 0, i = 0; i < m_nFirstVisible; i++){
    dwPrevItemsSize += m_aItems[i]->GetHeight();
  }

  m_Scroll.SetPosition(dwPrevItemsSize);
}

void CListControl::UpdateVisibility()
{
  DWORD dwSizeVisible, dwCurSize;
  int i;
  
  dwSizeVisible = GetClientRect()->Height() - m_pLayout->m_dwTopMargin - 
    m_pLayout->m_dwBottomMargin;

  for(dwCurSize = 0, i = m_nFirstVisible; i < m_aItems.GetSize(); i++){
    dwCurSize += m_aItems[i]->GetHeight();
    if(dwCurSize > dwSizeVisible) break;
  }
  if(i >= m_aItems.GetSize()) i = m_aItems.GetSize() - 1;
  if(i < 0) i = 0;
  m_nLastVisible = i;
  if ( m_nFirstVisible > m_nLastVisible ) m_nFirstVisible = m_nLastVisible;
}

void CListControl::UpdateItem(int nIndex)
{

}

int CListControl::AddItem(CString strText)
{
  ASSERT_VALID(this);

  CItem *pItem;
  int nIndex;

  pItem = NewItem();
  pItem->SetText(strText);
  nIndex = m_aItems.Add(pItem);

  UpdateVisibility();
  UpdateScroll();
  UpdateRect();

  return nIndex;
}

void CListControl::DeleteItem(int nIndex)
{
  ASSERT_VALID(this);
  ASSERT(nIndex >= 0);
  
  if(nIndex > m_aItems.GetSize()) return;

  CItem *pItem;
  pItem = m_aItems[nIndex];
  m_aItems.RemoveAt(nIndex);
  pItem->Delete();
  delete pItem;

  UpdateVisibility();
  UpdateScroll();
  UpdateRect();
}

void CListControl::RemoveAll()
{
  ASSERT_VALID(this);

  {
	  int nIndex;
	  CItem *pItem;
	  // First we have to copy the array
	  CTypedPtrArray < CPtrArray, CItem * > aItems;
	  aItems.Copy ( m_aItems );
	  // Now clear the member array -> no one will draw any of these items
	  m_aItems.RemoveAll ();

	  // And now go through the stored array and delete items
	  for ( nIndex = 0; nIndex < aItems.GetSize (); nIndex++ )
	  {
		  pItem = aItems [ nIndex ];
		  pItem->Delete();
		  delete pItem;
	  }
  }

  m_nCaretPosition = 0;
  m_nSelectedItems = 0;

  UpdateVisibility();
  UpdateScroll();
  UpdateRect();
}

int CListControl::InsertItem(int nIndex, CString strText)
{
  ASSERT_VALID(this);
  CItem *pItem;

  pItem = NewItem();
  pItem->SetText(strText);
  m_aItems.InsertAt(nIndex, pItem);

  UpdateVisibility();
  UpdateScroll();
  UpdateRect();

  return nIndex;
}

void CListControl::OnAnimsRepaint(CAnimationInstance *pAnim)
{
  if(pAnim == &m_Selection){
    if(m_nSelectedItems > 0){
      UpdateRect();
    }
  } else
  if((pAnim == &m_Focus) || (pAnim == &m_SelectionFocus)){
    UpdateRect();
  } else{
    CFrameWindow::OnAnimsRepaint(pAnim);
  }
}

BOOL CListControl::OnActivate()
{
  CFrameWindow::OnActivate();

  // we have to deactivate the frame if we are active
  // but don't have the focus
  if(!HasFocus()){
    m_bActiveFrame = FALSE;
    ChooseLayout();
  }
  return TRUE; // we want the event
}

void CListControl::OnSetFocus()
{
  CFrameWindow::OnSetFocus();

  // activate the frame -> set the focus
  if(!m_bActiveFrame){
    m_bActiveFrame = TRUE;
    ChooseLayout();
  }

  if(m_aItems.GetSize() > 0){
    m_aItems[m_nCaretPosition]->SetFocus(TRUE);
  }

  UpdateRect();

  // start the focus animation
  m_Focus.Play(); m_SelectionFocus.Play();
}

void CListControl::OnLoseFocus()
{
  CFrameWindow::OnLoseFocus();

  if(m_bMouseDrag){
    m_bMouseDrag = FALSE;
    ReleaseCapture();
  }

  // deactivate the frame -> lose the focus
  if(m_bActiveFrame){
    m_bActiveFrame = FALSE;
    ChooseLayout();
  }

  if(m_aItems.GetSize() > 0){
    m_aItems[m_nCaretPosition]->SetFocus(FALSE);
  }

  UpdateRect();

  // stop the focus animation
  m_Focus.Stop(); m_SelectionFocus.Stop();
}

void CListControl::OnScrollActivated()
{
  this->Activate();
  this->SetFocus();
}

void CListControl::UpdateItem(CItem *pItem)
{
  UpdateItem(FindItem(pItem));
}

int CListControl::FindItem(CItem *pItem)
{
  int i;
  for(i = 0; i < m_aItems.GetSize(); i++){
    if(m_aItems[i] == pItem) return i;
  }
  ASSERT(FALSE);
  return 0;
}

void CListControl::OnStepUp()
{
  if(m_nFirstVisible <= 0) return;
  m_nFirstVisible--;

  UpdateVisibility();
  UpdateScroll();
  UpdateRect();
}

void CListControl::OnStepDown()
{
  if(m_nFirstVisible >= m_aItems.GetSize()) return;

  DWORD dwSizeVisible, dwPos;
  int i;
  dwSizeVisible = GetClientRect()->Height() - m_pLayout->m_dwTopMargin - 
    m_pLayout->m_dwBottomMargin;
  for(dwPos = 0, i = m_nFirstVisible; i <= m_nLastVisible; i++){
    dwPos += m_aItems[i]->GetHeight();
  }
  if(dwPos < dwSizeVisible) return;

  m_nFirstVisible++;

  UpdateVisibility();
  UpdateScroll();
  UpdateRect();
}

void CListControl::OnPageUp()
{
  if(m_nFirstVisible <= 0) return;

  DWORD dwSizeVisible, dwPos;
  int i;
  dwSizeVisible = GetClientRect()->Height() - m_pLayout->m_dwTopMargin - 
    m_pLayout->m_dwBottomMargin;
  for(dwPos = 0, i = m_nFirstVisible; i >= 0; i--){
    dwPos += m_aItems[i]->GetHeight();
    if(dwPos > dwSizeVisible) break;
  }
  if(i < 0) i = 0;
  m_nFirstVisible = i;

  UpdateVisibility();
  UpdateScroll();
  UpdateRect();
}

void CListControl::OnPageDown()
{
  DWORD dwSizeVisible, dwPos;
  int i, nNewLastVisible;
  dwSizeVisible = GetClientRect()->Height() - m_pLayout->m_dwTopMargin - 
    m_pLayout->m_dwBottomMargin;
  for(dwPos = 0, i = m_nLastVisible; i < m_aItems.GetSize(); i++){
    dwPos += m_aItems[i]->GetHeight();
    if(dwPos > dwSizeVisible) break;
  }
  if(i >= m_aItems.GetSize()) i = m_aItems.GetSize() - 1;
  if(i < 0) i = 0;
  nNewLastVisible = i;
  for(dwPos = 0, i = nNewLastVisible; i >= m_nFirstVisible; i--){
    dwPos += m_aItems[i]->GetHeight();
    if(dwPos > dwSizeVisible){
      i++;
      break;
    }
  }
  if(i >= m_aItems.GetSize()) i = m_aItems.GetSize() - 1;
  if(i < m_nFirstVisible) i = m_nFirstVisible;
  m_nFirstVisible = i;

  UpdateVisibility();
  UpdateScroll();
  UpdateRect();
}

void CListControl::OnSize ( CSize size )
{
  // Call the base class
  CFrameWindow::OnSize ( size );
  // Update visibility
  UpdateVisibility ();
  // Update the scroll bar
  UpdateScroll ();
}

void CListControl::OnPosition(int nNewPosition)
{
  DWORD dwPos, dwNewPos;
  int i;

  if(m_aItems.GetSize() == 0) return;

  if(nNewPosition == 0){
    m_nFirstVisible = 0;
  } else
  if(nNewPosition + m_Scroll.GetPageSize() == m_Scroll.GetRangeMax()){
    DWORD dwSizeVisible, dwPos;
    int i;
    dwSizeVisible = GetClientRect()->Height() - m_pLayout->m_dwTopMargin - 
      m_pLayout->m_dwBottomMargin;
    for(dwPos = 0, i = m_aItems.GetSize() - 1; i >= 0; i--){
      dwPos += m_aItems[i]->GetHeight();
      if(dwPos > dwSizeVisible){
        i++; break;
      }
    }
    if(i >= m_aItems.GetSize()) i = m_aItems.GetSize() - 1;
    if(i < 0) i = 0;
    m_nFirstVisible = i;
  } else{
    for(dwPos = 0, dwNewPos = 0, i = 0; i < m_aItems.GetSize(); i++){
      dwNewPos += m_aItems[i]->GetHeight();
      if(dwNewPos > (DWORD)nNewPosition){
        break;
      }
      dwPos = dwNewPos;
    }
    if(i < 0) i = 0;
    if(i > m_aItems.GetSize()) i = m_aItems.GetSize() - 1;
    if(nNewPosition > (int)(dwPos + (m_aItems[i]->GetHeight() / 2)))
      i++;
    if(i > m_aItems.GetSize()) i = m_aItems.GetSize() - 1;
    m_nFirstVisible = i;
  }

  UpdateVisibility();
//  UpdateScroll();
  m_Scroll.SetPosition(nNewPosition);
  UpdateRect();
}

void CListControl::SetCaretPosition(int nPosition)
{
  if(m_aItems.GetSize() <= 0) return;
  if(nPosition < 0) nPosition = 0;
  if(nPosition >= m_aItems.GetSize()) nPosition = m_aItems.GetSize() - 1;

  if(m_nCaretPosition < nPosition){
    int nNew = NextSelectable(nPosition);
    if(nNew == -1) nNew = PrevSelectable(nPosition);
    nPosition = nNew;
  }
  else{
    int nNew = PrevSelectable(nPosition);
    if(nNew == -1) nNew = NextSelectable(nPosition);
    nPosition = nNew;
  }

  if(m_nCaretPosition == nPosition) return;
  m_aItems[m_nCaretPosition]->SetFocus(FALSE);
  m_nCaretPosition = nPosition;
  m_aItems[m_nCaretPosition]->SetFocus(TRUE);

  UpdateRect();
}

BOOL CListControl::OnKeyDown(UINT nChar, DWORD dwFlags)
{
  int nPrevPos;

  switch(nChar){
  case VK_DOWN:
    if(m_aItems.GetSize() <= 0) return TRUE;

    nPrevPos = m_nCaretPosition;
    // if the focus is set -> move caret
    // else set the focus to the caret one
    if(!m_aItems[m_nCaretPosition]->HasFocus()){
      m_aItems[m_nCaretPosition]->SetFocus(TRUE);
    }
    else{
      SetCaretPosition(m_nCaretPosition + 1);
    }
    if(!(dwFlags & CKeyboard::CtrlDown)){
      if(!(dwFlags & CKeyboard::ShiftDown)) ClearSelection();
      SelectItem(m_nCaretPosition);
      InlayEvent(E_SELCHANGE, m_nCaretPosition);
    }
    if(dwFlags & CKeyboard::ShiftDown){
      ShiftSelection(nPrevPos, m_nCaretPosition);
      InlayEvent(E_SELCHANGE, m_nCaretPosition);
    }
    ScrollToPosition(m_nCaretPosition);
    UpdateRect();
    return TRUE;
  case VK_UP:
    if(m_aItems.GetSize() <= 0) return TRUE;

    nPrevPos = m_nCaretPosition;
    // if the focus is set -> move caret
    // else set the focus to the caret one
    if(!m_aItems[m_nCaretPosition]->HasFocus()){
      m_aItems[m_nCaretPosition]->SetFocus(TRUE);
    }
    else{
      SetCaretPosition(m_nCaretPosition - 1);
    }
    if(!(dwFlags & CKeyboard::CtrlDown)){
      if(!(dwFlags & CKeyboard::ShiftDown)) ClearSelection();
      SelectItem(m_nCaretPosition);
      InlayEvent(E_SELCHANGE, m_nCaretPosition);
    }
    if(dwFlags & CKeyboard::ShiftDown){
      ShiftSelection(nPrevPos, m_nCaretPosition);
      InlayEvent(E_SELCHANGE, m_nCaretPosition);
    }
    ScrollToPosition(m_nCaretPosition);
    UpdateRect();
    return TRUE;
  case VK_NEXT:  // Page down
    {
      DWORD dwSizeVisible, dwPos;
      int i;
      
      if(m_aItems.GetSize() <= 0) return TRUE;
      nPrevPos = m_nCaretPosition;

      dwSizeVisible = GetClientRect()->Height() - m_pLayout->m_dwTopMargin - 
        m_pLayout->m_dwBottomMargin;
      for(dwPos = 0, i = m_nCaretPosition; i < m_aItems.GetSize(); i++){
        dwPos += m_aItems[i]->GetHeight();
        if(dwPos > dwSizeVisible){
          i--; break;
        }
      }
      if(i >= m_aItems.GetSize()) i = m_aItems.GetSize() - 1;
      if(i < 0) i = 0;
      SetCaretPosition(i);
      if(!(dwFlags & CKeyboard::CtrlDown)){
        if(!(dwFlags & CKeyboard::ShiftDown)) ClearSelection();
        SelectItem(m_nCaretPosition);
        InlayEvent(E_SELCHANGE, m_nCaretPosition);
      }
      if(dwFlags & CKeyboard::ShiftDown){
        ShiftSelection(nPrevPos, m_nCaretPosition);
        InlayEvent(E_SELCHANGE, m_nCaretPosition);
      }
      ScrollToPosition(m_nCaretPosition);
      UpdateRect();
    }
    return TRUE;
  case VK_PRIOR:  // Page up
    {
      DWORD dwSizeVisible, dwPos;
      int i;
      
      if(m_aItems.GetSize() <= 0) return TRUE;
      nPrevPos = m_nCaretPosition;

      dwSizeVisible = GetClientRect()->Height() - m_pLayout->m_dwTopMargin - 
        m_pLayout->m_dwBottomMargin;
      for(dwPos = 0, i = m_nCaretPosition; i >= 0; i--){
        dwPos += m_aItems[i]->GetHeight();
        if(dwPos > dwSizeVisible){
          i++; break;
        }
      }
      if(i >= m_aItems.GetSize()) i = m_aItems.GetSize() - 1;
      if(i < 0) i = 0;
      SetCaretPosition(i);
      if(!(dwFlags & CKeyboard::CtrlDown)){
        if(!(dwFlags & CKeyboard::ShiftDown)) ClearSelection();
        SelectItem(m_nCaretPosition);
        InlayEvent(E_SELCHANGE, m_nCaretPosition);
      }
      if(dwFlags & CKeyboard::ShiftDown){
        ShiftSelection(nPrevPos, m_nCaretPosition);
        InlayEvent(E_SELCHANGE, m_nCaretPosition);
      }
      ScrollToPosition(m_nCaretPosition);
      UpdateRect();
    }
    return TRUE;

  case VK_HOME:
    nPrevPos = m_nCaretPosition;
    SetCaretPosition(0);
    if(!(dwFlags & CKeyboard::CtrlDown)){
      if(!(dwFlags & CKeyboard::ShiftDown)) ClearSelection();
      SelectItem(m_nCaretPosition);
      InlayEvent(E_SELCHANGE, m_nCaretPosition);
    }
    if(dwFlags & CKeyboard::ShiftDown){
      ShiftSelection(nPrevPos, m_nCaretPosition);
      InlayEvent(E_SELCHANGE, m_nCaretPosition);
    }
    ScrollToPosition(m_nCaretPosition);
    UpdateRect();
    return TRUE;

  case VK_END:
    if(m_aItems.GetSize() <= 0) return TRUE;

    nPrevPos = m_nCaretPosition;
    SetCaretPosition(m_aItems.GetSize() - 1);
    if(!(dwFlags & CKeyboard::CtrlDown)){
      if(!(dwFlags & CKeyboard::ShiftDown)) ClearSelection();
      SelectItem(m_nCaretPosition);
      InlayEvent(E_SELCHANGE, m_nCaretPosition);
    }
    if(dwFlags & CKeyboard::ShiftDown){
      ShiftSelection(nPrevPos, m_nCaretPosition);
      InlayEvent(E_SELCHANGE, m_nCaretPosition);
    }
    ScrollToPosition(m_nCaretPosition);
    UpdateRect();
    return TRUE;

  case VK_SPACE:
    if(dwFlags & CKeyboard::CtrlDown){
      if(m_aItems[m_nCaretPosition]->IsSelected()){
        DeselectItem(m_nCaretPosition);
        InlayEvent(E_SELCHANGE, (DWORD)-1);
      }
      else{
        SelectItem(m_nCaretPosition);
        InlayEvent(E_SELCHANGE, m_nCaretPosition);
      }
    }
    else{
      SelectItem(m_nCaretPosition);
      InlayEvent(E_SELCHANGE, m_nCaretPosition);
    }
    ScrollToPosition(m_nCaretPosition);
    UpdateRect();
    return TRUE;
  }
  return CFrameWindow::OnKeyDown(nChar, dwFlags);
}

BOOL CListControl::OnKeyUp(UINT nChar, DWORD dwFlags)
{
  return CFrameWindow::OnKeyUp(nChar, dwFlags);
}

void CListControl::ScrollToPosition(int nPosition)
{
  if(m_aItems.GetSize() <= 0) return;
  if(nPosition < 0) nPosition = 0;
  if(nPosition >= m_aItems.GetSize()) nPosition = m_aItems.GetSize() - 1;

  if(nPosition < m_nFirstVisible){
    m_nFirstVisible = nPosition;
    UpdateVisibility();
    UpdateScroll();
    return;
  }

  if(nPosition >= m_nLastVisible){
    DWORD dwSizeVisible, dwPos;
    int i;
    dwSizeVisible = GetClientRect()->Height() - m_pLayout->m_dwTopMargin - 
      m_pLayout->m_dwBottomMargin;

    for(dwPos = 0, i = nPosition; i >= m_nFirstVisible; i--){
      dwPos += m_aItems[i]->GetHeight();
      if(dwPos > dwSizeVisible){
        i++; break;
      }
    }
    if(i >= m_aItems.GetSize()) i = m_aItems.GetSize() - 1;
    if(i < m_nFirstVisible) i = m_nFirstVisible;
    m_nFirstVisible = i;
    UpdateVisibility();
    UpdateScroll();
    return;
  }
}

void CListControl::ClearSelection()
{
  int i;

  for(i = 0; i < m_aItems.GetSize(); i++){
    m_aItems[i]->SetSelected(FALSE);
    InlayEvent(E_ItemDeselected, i);
  }
  m_nSelectedItems = 0;
}

void CListControl::SelectItem(int nPosition)
{
  if(!m_aItems[nPosition]->IsSelectable()) return;

  if(!m_aItems[nPosition]->IsSelected()) // wasn't selected
    m_nSelectedItems++;                  // increase number of selcted

  if(!m_bMultiSelect) ClearSelection();
  m_aItems[nPosition]->SetSelected(TRUE);
  InlayEvent(E_ItemSelected, nPosition);
}

void CListControl::DeselectItem(int nPosition)
{
  if(!m_aItems[nPosition]->IsSelectable()) return;
  if(m_aItems[nPosition]->IsSelected()) // was selected
    m_nSelectedItems--;                 // decrease number of selected

  m_aItems[nPosition]->SetSelected(FALSE);
  InlayEvent(E_ItemDeselected, nPosition);
}

void CListControl::ShiftSelection(int nPrevPos, int nNewPos)
{
  if(m_aItems[nPrevPos]->IsSelected()){
    int i;
    if(nPrevPos < nNewPos){
      for(i = nPrevPos; i <= nNewPos; i++){
        SelectItem(i);
      }
    }
    else{
      for(i = nNewPos; i <= nPrevPos; i++){
        SelectItem(i);
      }
    }
  }
  else{
    SelectItem(nNewPos);
  }
}

void CListControl::OnLButtonDown(CPoint point)
{
  CRect rcInner(m_pLayout->m_dwLeftMargin, m_pLayout->m_dwTopMargin,
    GetClientRect()->right - m_pLayout->m_dwRightMargin,
    GetClientRect()->bottom - m_pLayout->m_dwBottomMargin);

  if(!rcInner.PtInRect(point)) return;
  m_bMouseDrag = TRUE;
  SetCapture();

  int nPos = ItemFromPoint(point);
  int nOldPos = m_nCaretPosition;
  if(nPos < 0) return; // no item
  SetCaretPosition(nPos);
  if(!g_pKeyboard->CtrlPressed()){
    if(!g_pKeyboard->ShiftPressed()) ClearSelection();
    SelectItem(m_nCaretPosition);
    InlayEvent(E_SELCHANGE, m_nCaretPosition);
  }
  else{
    if(m_aItems[m_nCaretPosition]->IsSelected()){
      DeselectItem(m_nCaretPosition);
      InlayEvent(E_SELCHANGE, (DWORD)-1);
    }
    else{
      SelectItem(m_nCaretPosition);
      InlayEvent(E_SELCHANGE, m_nCaretPosition);
    }
  }
  if(g_pKeyboard->ShiftPressed()){
    ShiftSelection(nOldPos, m_nCaretPosition);
    InlayEvent(E_SELCHANGE, m_nCaretPosition);
  }
  ScrollToPosition(m_nCaretPosition);
  UpdateRect();
}

void CListControl::OnLButtonUp(CPoint point)
{
  if(m_bMouseDrag){
    m_bMouseDrag = FALSE;
    ReleaseCapture();
  }
}

void CListControl::OnMouseMove(CPoint point)
{
  if(m_bMouseDrag){
  }
}

int CListControl::ItemFromPoint(CPoint point)
{
  DWORD dwPos;

  if(point.x < (int)m_pLayout->m_dwLeftMargin) point.x = m_pLayout->m_dwLeftMargin;
  if(point.x > (int)(GetClientRect()->right - m_pLayout->m_dwRightMargin))
    point.x = GetClientRect()->right - m_pLayout->m_dwRightMargin;
  if(point.y < (int)m_pLayout->m_dwTopMargin) point.y = m_pLayout->m_dwTopMargin;
  if(point.y > (int)(GetClientRect()->bottom - m_pLayout->m_dwBottomMargin))
    point.y = GetClientRect()->bottom - m_pLayout->m_dwBottomMargin;

  if(m_aItems.GetSize() == 0) return -1;

  int i;
  dwPos = m_pLayout->m_dwTopMargin;
  for(i = m_nFirstVisible; i <= m_nLastVisible; i++){
    dwPos += m_aItems[i]->GetHeight();
    if(point.y < (int)dwPos){
      if(m_aItems[i]->IsSelectable()) break;
    }
  }
  if(i > m_nLastVisible) return -1;
  return i;
}

void CListControl::OnLButtonDblClk(CPoint point)
{
  CRect rcInner(m_pLayout->m_dwLeftMargin, m_pLayout->m_dwTopMargin,
    GetClientRect()->right - m_pLayout->m_dwRightMargin,
    GetClientRect()->bottom - m_pLayout->m_dwBottomMargin);

  if(!rcInner.PtInRect(point)) return;

  int nPos = ItemFromPoint(point);
  if(nPos < 0) return;
  SetCaretPosition(nPos);
  SelectItem(m_nCaretPosition);
  InlayEvent(E_SELECTED, m_nCaretPosition);
}

void CListControl::SetItemText(int nIndex, CString strText)
{
  ASSERT_VALID(this);
  ASSERT(nIndex >= 0);
  ASSERT(nIndex < m_aItems.GetSize());

  m_aItems[nIndex]->SetText(strText);
  UpdateRect();
}

void CListControl::SetItemText(int nIndex, DWORD dwSubItem, CString strText)
{
  ASSERT_VALID(this); ASSERT((nIndex >= 0) && (nIndex < m_aItems.GetSize()));
  ASSERT(dwSubItem < m_dwColumns - 1);

  m_aItems[nIndex]->SetText(dwSubItem, strText);
  UpdateRect();
}

int CListControl::GetItemsCount()
{
  ASSERT_VALID(this);
  return m_aItems.GetSize();
}

int CListControl::FindItemData(DWORD dwData)
{
  ASSERT_VALID(this);
  int i;
  for(i = 0; i < m_aItems.GetSize(); i++){
    if(m_aItems[i]->GetData() == dwData)
      return i;
  }
  return -1;
}

BOOL CListControl::IsItemSelected(int nIndex)
{
  ASSERT_VALID(this);
  ASSERT(nIndex >= 0);
  ASSERT(nIndex < m_aItems.GetSize());

  return m_aItems[nIndex]->IsSelected();
}

void CListControl::SetItemSelected(int nIndex, BOOL bSelected)
{
  ASSERT_VALID(this);
  ASSERT(nIndex >= 0);
  ASSERT(nIndex < m_aItems.GetSize());

  m_aItems[nIndex]->SetSelected(bSelected);
  if(bSelected){
    InlayEvent(E_ItemSelected, nIndex);
  }
  else{
    InlayEvent(E_ItemDeselected, nIndex);
  }
}

int CListControl::GetSelectedCount()
{
  return m_nSelectedItems;
}

int CListControl::GetSelectedItem()
{
  int i;
  for(i = 0; i < m_aItems.GetSize(); i++){
    if(m_aItems[i]->IsSelected()) return i;
  }
  return -1;
}

int CListControl::GetNextSelectedItem(int nItem)
{
  ASSERT(nItem >= -1);
  ASSERT(nItem < m_aItems.GetSize());
  for(nItem++; nItem < m_aItems.GetSize(); nItem++){
    if(m_aItems[nItem]->IsSelected()) return nItem;
  }
  return -1;
}

void CListControl::SetWindowPos(CRect *pRect)
{
  CRect rcBound(pRect), rcScroll;

  // small down the window -> some scroll bar
  rcBound.right -= m_pLayout->m_ScrollControlLayout.GetWidth();

  CFrameWindow::SetWindowPosition(&rcBound);

  rcScroll.top = rcBound.top; rcScroll.bottom = rcBound.bottom;
  rcScroll.left = rcBound.right; rcScroll.right = rcBound.right + m_pLayout->m_ScrollControlLayout.GetWidth();

  m_Scroll.SetWindowPosition(&rcScroll);
}

CListControl::CItem *CListControl::NewItem()
{
  CItem *pItem = new CItem();
  pItem->Create(this);
  return pItem;
}

int CListControl::NextSelectable(int nPosition)
{
  for(;nPosition < m_aItems.GetSize(); nPosition++){
    if(m_aItems[nPosition]->IsSelectable()) return nPosition;
  }
  return -1;
}

int CListControl::PrevSelectable(int nPosition)
{
  for(;nPosition >= 0; nPosition--){
    if(m_aItems[nPosition]->IsSelectable()) return nPosition;
  }
  return -1;
}