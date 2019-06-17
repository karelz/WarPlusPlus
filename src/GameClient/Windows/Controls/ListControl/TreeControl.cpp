// TreeControl.cpp: implementation of the CTreeControl class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "TreeControl.h"

IMPLEMENT_DYNAMIC(CTreeControl, CListControl);

BEGIN_OBSERVER_MAP(CTreeControl, CListControl)
  BEGIN_MOUSE()
    ON_LBUTTONDOWN()
  END_MOUSE()
END_OBSERVER_MAP(CTreeControl, CListControl)

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CTreeControl::CTreeControl()
{

}

CTreeControl::~CTreeControl()
{

}


#ifdef _DEBUG

void CTreeControl::AssertValid() const
{
  CListControl::AssertValid();
}

void CTreeControl::Dump(CDumpContext &dc) const
{
  CListControl::Dump(dc);
}

#endif

BOOL CTreeControl::Create(CRect &rcBound, CTreeControlLayout *pLayout, CWindow *pParent, DWORD dwColumns,
                          BOOL bMultiSelect)
{
  ASSERT_VALID(pLayout);
  m_pLayout = pLayout;

  return CListControl::Create(rcBound, pLayout, pParent, dwColumns, bMultiSelect);
}

void CTreeControl::Delete()
{
  m_pLayout = NULL;
  CListControl::Delete();
}

CListControl::CItem *CTreeControl::NewItem()
{
  CItem *pItem = new CTreeControl::CItem();
  pItem->Create(this);
  return pItem;
}

void CTreeControl::CItem::Draw(CDDrawSurface *pSurface, CRect *pRectBound)
{
  if(IsHidden()) return;

  CDDrawSurface *pBkg = NULL;
  DWORD dwColor = GetTextColor(&pBkg);

  DrawBackground(pSurface, pBkg, pRectBound);

  CTreeControlLayout *pLayout = ((CTreeControl *)m_pControl)->GetLayout();
  CDDrawSurface *pMark;
  if(IsCollapsed()) pMark = pLayout->m_pPlus->GetFrame(0);
  else pMark = pLayout->m_pMinus->GetFrame(0);

  DWORD dwXPos = pLayout->m_dwIndentation * m_dwNestedLevel;
  DWORD dwYPos = (GetHeight() - pMark->GetAllRect()->Height()) / 2;

  // Have our item any children
  BOOL bHasChildren = FALSE;
  do{
    int nItem = GetControl()->FindItem(this);

    if(GetControl()->GetFirstChild(nItem) != -1) bHasChildren = TRUE;
  } while(FALSE);

  // Draw the mark
  if(bHasChildren)
    pSurface->Paste(dwXPos, dwYPos, pMark);
  dwXPos += pMark->GetAllRect()->Width();

  // Draw the main item
  pLayout->GetFont()->PaintText(dwXPos, 0, m_strText, pSurface, dwColor);

  // then draw the subitems text
  {
    DWORD dwSubItem;
    DWORD dwXPos = ((CTreeControl *)m_pControl)->m_aColumnWidths[0];

    for(dwSubItem = 0; dwSubItem < ((CTreeControl *)m_pControl)->m_dwColumns - 1; dwSubItem++){
      pLayout->GetFont()->PaintText(dwXPos, 0, m_aSubItems[dwSubItem], pSurface, dwColor);
      dwXPos += ((CTreeControl *)m_pControl)->m_aColumnWidths[dwSubItem + 1];
    }
  }
}

void CTreeControl::OnLButtonDown(CPoint pt)
{
  int nItem;
  nItem = ItemFromPoint(pt);
  if(nItem == -1) return;

  CItem *pItem = (CItem *)m_aItems[nItem];

  DWORD dwXPos = GetLayout()->m_dwIndentation * pItem->m_dwNestedLevel + m_pLayout->m_dwLeftMargin;

  if(pt.x < (int)dwXPos) return;

  if(pt.x < (int)(dwXPos + GetLayout()->m_pPlus->GetFrame(0)->GetAllRect()->Width())){
    // expand or collapse
    if(pItem->IsCollapsed()){
      ExpandItem(nItem);
    }
    else{
      CollapseItem(nItem);
    }

    return;
  }
  CListControl::OnLButtonDown(pt);
}

void CTreeControl::OnLButtonDblClk(CPoint pt)
{
  int nItem;
  nItem = ItemFromPoint(pt);
  if(nItem == -1) return;

  CItem *pItem = (CItem *)m_aItems[nItem];
  DWORD dwXPos = GetLayout()->m_dwIndentation * pItem->m_dwNestedLevel + m_pLayout->m_dwLeftMargin;
  if(pt.x < (int)dwXPos) return;
  if(pt.x < (int)(dwXPos + GetLayout()->m_pPlus->GetFrame(0)->GetAllRect()->Width())) return;

  CListControl::OnLButtonDblClk(pt);
}

int CTreeControl::AddItem(int nParent, CString strText)
{
  return InsertItem(nParent, -1, strText);
}

int CTreeControl::InsertItem(int nParent, int nAfter, CString strText)
{
  ASSERT(nParent >= -1);
  ASSERT(nParent < m_aItems.GetSize());

  int nLastChild;
  DWORD dwChildLevel;
  BOOL bAfterReached = FALSE;

  if(nParent == -1) dwChildLevel = 0;
  else dwChildLevel = GetItem(nParent)->m_dwNestedLevel + 1;

  for(nLastChild = nParent + 1; nLastChild < m_aItems.GetSize(); nLastChild++){
    if(GetItem(nLastChild)->m_dwNestedLevel < dwChildLevel) break;
    if(GetItem(nLastChild)->m_dwNestedLevel == dwChildLevel){
      if(bAfterReached) break;
      if(GetItem(nLastChild)->m_pParent != GetItem(nParent)) break;
    }
    if(nLastChild == nAfter) bAfterReached = TRUE;
  }
  int nItem = CListControl::InsertItem(nLastChild, strText);

  if(nParent == -1){
    GetItem(nItem)->m_dwNestedLevel = 0;
    GetItem(nItem)->m_pParent = NULL;
  }
  else{
    GetItem(nItem)->m_dwNestedLevel = GetItem(nParent)->m_dwNestedLevel + 1;
    GetItem(nItem)->m_pParent = GetItem(nParent);
  }

  return nItem;
}

void CTreeControl::DeleteItem(int nItem)
{
  ASSERT(nItem >= 0);
  ASSERT(nItem < m_aItems.GetSize());

  CItem *pParent = GetItem(nItem);

  int nChild;
  for(nChild = nItem + 1; nChild < m_aItems.GetSize(); nChild++){
    if(GetItem(nChild)->m_pParent == pParent){
      DeleteItem(nChild);
    }
  }

  CListControl::DeleteItem(nChild);
}

void CTreeControl::ExpandItem(int nItem)
{
  ASSERT(nItem >= -1);
  ASSERT(nItem < m_aItems.GetSize());

  if(nItem == -1){
    int nChild;
    for(nChild = 0; nChild < m_aItems.GetSize();nChild++){
      if(GetItem(nChild)->m_dwNestedLevel > 0) continue;

      GetItem(nChild)->m_dwTreeState &= ~(CItem::Hidden);
      if(GetItem(nChild)->IsExpanded()){
        ExpandItem(nChild);
      }
    }
    return;
  }

  // Set expanded flag
  if(GetItem(nItem)->IsCollapsed()) InlayEvent(E_ItemExpanded, nItem);
  GetItem(nItem)->m_dwTreeState &= ~(CItem::Collapsed);

  if(GetItem(nItem)->IsHidden()) return;

  // Go through all children and unhide them
  int nChild;
  DWORD dwChildLevel = GetItem(nItem)->m_dwNestedLevel + 1;

  for(nChild = nItem + 1; nChild < m_aItems.GetSize(); nChild++){
    if(GetItem(nChild)->m_dwNestedLevel < dwChildLevel) break;
    if(GetItem(nChild)->m_pParent != GetItem(nItem)) continue;

    GetItem(nChild)->m_dwTreeState &= ~(CItem::Hidden);
    if(GetItem(nChild)->IsExpanded()){
      ExpandItem(nChild);
    }
  }

  UpdateScroll();
  UpdateRect();
}

void CTreeControl::CollapseItem(int nItem)
{
  ASSERT(nItem >= 0);
  ASSERT(nItem < m_aItems.GetSize());

  // Set expanded flag
  if(GetItem(nItem)->IsExpanded()) InlayEvent(E_ItemCollapsed, nItem);
  GetItem(nItem)->m_dwTreeState |= CItem::Collapsed;

  // Go through all children and unhide them
  int nChild;
  DWORD dwChildLevel = GetItem(nItem)->m_dwNestedLevel + 1;

  for(nChild = nItem + 1; nChild < m_aItems.GetSize(); nChild++){
    if(GetItem(nChild)->m_dwNestedLevel < dwChildLevel) break;

    GetItem(nChild)->m_dwTreeState |= CItem::Hidden;
    if(GetItem(nChild)->IsSelected()) DeselectItem(nChild);
  }

  UpdateScroll();
  UpdateRect();
}

int CTreeControl::GetParentItem(int nItem)
{
  ASSERT(nItem >= 0);
  ASSERT(nItem < m_aItems.GetSize());

  CItem *pItem = GetItem(nItem);
  int nParent;
  if(pItem->m_pParent == NULL) nParent = -1;
  else nParent = FindItem(pItem->m_pParent);

  return nParent;
}

int CTreeControl::GetFirstChild(int nItem)
{
  ASSERT(nItem >= 0);
  ASSERT(nItem < m_aItems.GetSize());

  if(nItem == m_aItems.GetSize() - 1) return -1;

  if(GetItem(nItem + 1)->m_dwNestedLevel != GetItem(nItem)->m_dwNestedLevel + 1) return -1;

  return nItem + 1;
}

int CTreeControl::GetNextSibbling(int nItem)
{
  ASSERT(nItem >= 0);
  ASSERT(nItem < m_aItems.GetSize());

  if(nItem == m_aItems.GetSize() - 1) return -1;

  int nNext;
  DWORD dwLevel = GetItem(nItem)->m_dwNestedLevel;
  for(nNext = nItem + 1; nNext < m_aItems.GetSize(); nNext++){
    if(GetItem(nNext)->m_dwNestedLevel < dwLevel) break;
    if(GetItem(nNext)->m_dwNestedLevel == dwLevel) return nNext;
  }
  return -1;
}

BOOL CTreeControl::IsItemExpanded(int nItem)
{
  ASSERT(nItem >= 0);
  ASSERT(nItem < m_aItems.GetSize());

  return GetItem(nItem)->IsExpanded();
}