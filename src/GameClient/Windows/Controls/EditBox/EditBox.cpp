// EditBox.cpp: implementation of the CEditBox class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "EditBox.h"
#include "..\..\Keyboard.h"
#include "..\Layouts.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

IMPLEMENT_DYNAMIC(CEditBox, CFrameWindow);

BEGIN_OBSERVER_MAP(CEditBox, CFrameWindow)
  BEGIN_ANIMS()
    ON_ANIMSREPAINT()
  END_ANIMS()

  BEGIN_KEYBOARD()
    ON_KEYDOWN()
    ON_CHAR()
  END_KEYBOARD()

  BEGIN_MOUSE()
    ON_LBUTTONDOWN()
    ON_LBUTTONUP()
    ON_LBUTTONDBLCLK()
    ON_MOUSEMOVE()
  END_MOUSE()
END_OBSERVER_MAP(CEditBox, CFrameWindow)

CEditBox::CEditBox()
{
  m_pLayout = NULL;
  m_bPassword = FALSE;
}

CEditBox::~CEditBox()
{
}

#ifdef _DEBUG

void CEditBox::AssertValid() const
{
  CFrameWindow::AssertValid();

  ASSERT(m_pLayout != NULL);
  ASSERT_VALID(m_pLayout);
}

void CEditBox::Dump(CDumpContext &dc) const
{
  dc << "Text : " << m_strText << "\n";

  CFrameWindow::Dump(dc);
}

#endif

CString CEditBox::GetText()
{
  ASSERT_VALID(this);

  return m_strText;
}

void CEditBox::SetText(CString strText)
{
  ASSERT_VALID(this);

  m_strText = strText;
  m_nSelectionStart = -1;
  m_nSelectionEnd = 0;
  m_nFirstVisible = 0;
  m_nCaretPosition = 0;
  UpdateRect();
}

BOOL CEditBox::Create(CRect &rcBound, CEditBoxLayout *pLayout, CWindow *pParent, int nMaxLength)
{
  if(pLayout == NULL){
    m_pLayout = CLayouts::m_pDefaults->GetEditBoxLayout();
  }
  else{
    m_pLayout = pLayout;
  }
  ASSERT_VALID(m_pLayout);
  ASSERT(nMaxLength >= -1);

  CRect rcControl(rcBound);
  BOOL bResult = TRUE;

  m_nMaxLength = nMaxLength;

  // compute the height of the control
  rcControl.bottom = rcControl.top + m_pLayout->GetFont()->GetCharSize('A').cy +
    m_pLayout->m_nTopMargin + m_pLayout->m_nBottomMargin;

  // we have the tabstop -> we want the keyboard input
  m_bTabStop = TRUE;
  // also we want doubleclicks
  m_bDoubleClk = TRUE;

  // set no selection
  m_nSelectionStart = -1;
  m_nSelectionEnd = 0;
  m_bSelectionVisible = FALSE;
  // first visible is at the begining
  m_nFirstVisible = 0;
  // caret at the begining
  m_nCaretPosition = 0;

  // mouse drag -> no
  m_bMouseDrag = FALSE;

  // now create the window itself
  if(!CFrameWindow::Create(rcControl, m_pLayout, pParent, FALSE))
    return FALSE;

  // create animations for cursor and selection
  m_Caret.Create(m_pLayout->m_pCaret, this);
  m_Selection.Create(m_pLayout->m_pSelection, this);

  // create the inner window
  m_rcInner = *GetClientRect();
  m_rcInner.left += m_pLayout->m_nLeftMargin;
  m_rcInner.top += m_pLayout->m_nTopMargin;
  m_rcInner.right -= m_pLayout->m_nRightMargin;
  m_rcInner.bottom -= m_pLayout->m_nBottomMargin;

  return TRUE;
}

void CEditBox::Delete()
{
  m_strText.Empty();
  m_pLayout = NULL;
  CFrameWindow::Delete();
}

void CEditBox::OnSetFocus()
{
  CFrameWindow::OnSetFocus();

  // activate the frame -> set the focus
  if(!m_bActiveFrame){
    m_bActiveFrame = TRUE;
    ChooseLayout();
    UpdateRect();
  }

  // start animations for cursor and selection if needed
  m_Caret.Play();
  m_Selection.Play();

  SetSelectionStart(0);
  SetSelectionEdge(m_strText.GetLength());
  m_nCaretPosition = m_strText.GetLength();
  ScrollToCaret();
  m_bSelectionVisible = TRUE;
}

void CEditBox::OnLoseFocus()
{
  CFrameWindow::OnLoseFocus();

  // deactivate the frame -> lose the focus
  if(m_bActiveFrame){
    m_bActiveFrame = FALSE;
    ChooseLayout();
    UpdateRect();
  }

  if(m_bMouseDrag){
    ReleaseCapture();
    m_bMouseDrag = FALSE;
  }

  // stop animations
  m_Caret.Stop();
  m_Selection.Stop();

  m_nFirstVisible = 0;
  m_bSelectionVisible = FALSE;
}

BOOL CEditBox::OnActivate()
{
  CFrameWindow::OnActivate();

  // we have to deactivate the frame if we are active
  // but don't have the focus
  if(!HasFocus()){
    m_bActiveFrame = FALSE;
    ChooseLayout();
  }
  return FALSE;
}

void CEditBox::Draw(CDDrawSurface *pSurface, CRect *pBoundRect)
{
  CFrameWindow::Draw(pSurface, pBoundRect);

  // first draw the text, then the selection and then the cursor
  CString strLeft, strSel, strRight, strVisible;
  int nSelStartPos, nSelEndPos;
  CFontObject *pFont = m_pLayout->GetFont();

  int nVisibleCount = GetVisibleText(strVisible);

  if(m_bSelectionVisible){
    int nSelStart, nRightStart;
    if(m_nSelectionStart > m_nFirstVisible){
      if(m_nSelectionStart < (m_nFirstVisible + nVisibleCount)){
        strLeft = m_strText.Mid(m_nFirstVisible, (m_nSelectionStart - m_nFirstVisible));
        nSelStart = m_nSelectionStart;
      }
      else{
        strLeft = m_strText;
        nSelStart = -1;
      }
    }
    else{
      strLeft.Empty();
      nSelStart = m_nFirstVisible;
    }
    
    if(nSelStart != -1){
      if(m_nSelectionEnd <= m_nFirstVisible){
        strSel.Empty();
        nRightStart = m_nFirstVisible;
      }
      else{
        if(m_nSelectionEnd <= (m_nFirstVisible + nVisibleCount)){
          strSel = m_strText.Mid(nSelStart, m_nSelectionEnd - nSelStart);
          nRightStart = m_nSelectionEnd;
        }
        else{
          strSel = m_strText.Mid(nSelStart, nVisibleCount - (nSelStart - m_nFirstVisible));
          nRightStart = -1;
        }
      }
    }
    else{
      nRightStart = -1;
    }
    
    if(nRightStart != -1){
      if((m_nFirstVisible + nVisibleCount) <= m_strText.GetLength()){
        strRight = m_strText.Mid(nRightStart, nVisibleCount - (nRightStart - m_nFirstVisible));
      }
      else{
        strRight = m_strText.Mid(nRightStart);
      }
    }
  }

  CRect rcOldClip;
  // set the clipping only to the inner rect for the text
  pSurface->SetClipRect(&m_rcInner, &rcOldClip);

  if(m_bSelectionVisible){
    nSelStartPos = m_rcInner.left + pFont->GetTextSize(strLeft).cx;
    nSelEndPos = nSelStartPos + pFont->GetTextSize(strSel).cx;

    // now draw the selection overlay
    // first count the size of tiling
    CSize sizeSel;
    int nTilesX, nTilesY;
    CRect rcSelection(nSelStartPos, m_rcInner.top, nSelEndPos, m_rcInner.bottom);
    if(!rcSelection.IsRectEmpty()){
      CRect rcPrevSelClip;
      pSurface->SetClipRect(&rcSelection, &rcPrevSelClip);
      
      sizeSel = m_Selection.GetSize();
      nTilesX = (rcSelection.Width() / sizeSel.cx) + 1;
      nTilesY = (rcSelection.Height() / sizeSel.cy) + 1;
      
      // draw the selection
      int i, j;
      for(i = 0; i < nTilesX; i++){
        for(j = 0; j < nTilesY; j++){
          pSurface->Paste(rcSelection.left + i * sizeSel.cx,
            rcSelection.top + j * sizeSel.cy, m_Selection.Frame());
        }
      }
      pSurface->SetClipRect(&rcPrevSelClip, NULL);
    }

    if(!m_bPassword){
      // draw the left part of the text (non selected)
      if(!strLeft.IsEmpty())
        pFont->PaintText(m_rcInner.left, m_rcInner.top, strLeft, pSurface,
          m_pLayout->m_dwActiveTextColor);
      
      // draw the middle part of the text (selection)
      if(!strSel.IsEmpty()){
        pFont->PaintText(nSelStartPos, m_rcInner.top, strSel, pSurface,
          m_pLayout->m_dwSelectedTextColor);
      }
      // draw the right part of the text (non selected)
      if(!strRight.IsEmpty())
        pFont->PaintText(nSelEndPos, m_rcInner.top, strRight, pSurface,
          m_pLayout->m_dwActiveTextColor);
    }
  }
  else{
    if(!m_bPassword){
      if(!strVisible.IsEmpty())
        pFont->PaintText(m_rcInner.left, m_rcInner.top, strVisible, pSurface,
          m_pLayout->m_dwInactiveTextColor);
    }
  }

  // now draw the caret
  if(m_bSelectionVisible){
    CSize sizeCaret = m_Caret.GetSize();
    int nCaretTileY = (m_rcInner.Height() / sizeCaret.cy) + 1;
    CString strBeforeCaret = strVisible.Left(m_nCaretPosition - m_nFirstVisible);
    int nCaretXPos = m_rcInner.left + pFont->GetTextSize(strBeforeCaret).cx - 1;
    if(nCaretXPos < m_rcInner.left) nCaretXPos = m_rcInner.left;
    int i;
    for(i = 0; i < nCaretTileY; i++){
      pSurface->Paste(nCaretXPos, m_rcInner.top + i * sizeCaret.cy,
        m_Caret.Frame());
    }
  }

  // restore the clip rect
  pSurface->SetClipRect(&rcOldClip, NULL);
}

void CEditBox::OnAnimsRepaint(CAnimationInstance *pAnimation)
{
  UpdateRect();
}

BOOL CEditBox::OnKeyDown(UINT nChar, DWORD dwFlags)
{
  switch(nChar){
  case VK_LEFT:
	if ( ! (dwFlags & CKeyboard::ShiftDown))
      SetSelectionStart ( m_nCaretPosition );
    if(dwFlags & CKeyboard::CtrlDown){
    }
    if(m_nCaretPosition > 0){
      m_nCaretPosition--;
      if(dwFlags & CKeyboard::ShiftDown)
        SetSelectionEdge(m_nCaretPosition);
      else
        SetSelectionStart(m_nCaretPosition);
      ScrollToCaret();
      UpdateRect();
    }
    return TRUE;
  case VK_RIGHT:
	if ( !(dwFlags & CKeyboard::ShiftDown) )
      SetSelectionStart ( m_nCaretPosition );
    if(dwFlags & CKeyboard::CtrlDown){
    }
    if(m_nCaretPosition < m_strText.GetLength()){
      m_nCaretPosition++;
      if(dwFlags & CKeyboard::ShiftDown)
        SetSelectionEdge(m_nCaretPosition);
      else
        SetSelectionStart(m_nCaretPosition);
      ScrollToCaret();
      UpdateRect();
    }
    return TRUE;

  case VK_BACK:
    if(dwFlags & CKeyboard::CtrlDown){
    }
    if(dwFlags & CKeyboard::ShiftDown){
    }
BackSpaceDel:;
    if(!NoSelection()){  // if some delete the selection
      DeleteSelection();
      return TRUE;
    }

    if(m_nCaretPosition > 0){
      m_strText.Delete(m_nCaretPosition - 1);
      m_nCaretPosition--;
      InlayEvent(E_CHANGE, 0);
      ScrollToCaret();
      UpdateRect();
    }
    return TRUE;

  case VK_DELETE:
    if(dwFlags & CKeyboard::CtrlDown){
    }
    if((dwFlags & CKeyboard::ShiftDown) && (!(dwFlags & CKeyboard::CtrlDown))){
      if(NoSelection()) goto BackSpaceDel;

      // remove the selection and copy it to the clipboard
      Cut();
      return TRUE;
    }

    if(!NoSelection()){  // if some delete the selection
      DeleteSelection();
      return TRUE;
    }
    if(m_nCaretPosition < m_strText.GetLength()){ // else delete 
      m_strText.Delete(m_nCaretPosition);         // the char beyond the caret
      InlayEvent(E_CHANGE, 0);
      ScrollToCaret();
      UpdateRect();
    }
    return TRUE;

  case VK_HOME:
	if ( !(dwFlags & CKeyboard::ShiftDown))
      SetSelectionStart ( m_nCaretPosition );
    if(m_nCaretPosition > 0){
      m_nCaretPosition = 0;
      if(dwFlags & CKeyboard::ShiftDown)
        SetSelectionEdge(m_nCaretPosition);
      else
        SetSelectionStart(m_nCaretPosition);
      ScrollToCaret();
      UpdateRect();
    }
    return TRUE;
  case VK_END:
	if ( !(dwFlags & CKeyboard::ShiftDown) )
      SetSelectionStart ( m_nCaretPosition );
    if(m_nCaretPosition < m_strText.GetLength()){
      m_nCaretPosition = m_strText.GetLength();
      if(dwFlags & CKeyboard::ShiftDown)
        SetSelectionEdge(m_nCaretPosition);
      else
        SetSelectionStart(m_nCaretPosition);
      ScrollToCaret();
      UpdateRect();
    }
    return TRUE;

  case VK_INSERT:
    if((dwFlags & CKeyboard::CtrlDown) && (!(dwFlags & CKeyboard::ShiftDown))){
      // here we will copy the selection to the clipboard
      Copy();
      return TRUE;
    }
    if((dwFlags & CKeyboard::ShiftDown) && (!(dwFlags & CKeyboard::CtrlDown))){
      // and here we will paste the selection
      Paste();
      return TRUE;
    }
    break;
  }

  return CFrameWindow::OnKeyDown(nChar, dwFlags);
}

BOOL CEditBox::OnChar(UINT nChar, DWORD dwFlags)
{
  // if we can't display the character -> don't read it
  if(m_pLayout->GetFont()->GetCharSize(nChar).cx <= 0){
    return FALSE;
  }

  // delete any selection (overwriteit)
  DeleteSelection();
  if(m_strText < m_nMaxLength){
    // insert the new character
    m_strText.Insert(m_nCaretPosition, nChar);
    // move the caret beoynd it
    m_nCaretPosition++;
    SetSelectionStart(m_nCaretPosition);
  }
  // send notification
  InlayEvent(E_CHANGE, 0);
  // scroll and update the editbox
  ScrollToCaret();
  UpdateRect();
  return TRUE;
}

int CEditBox::GetVisibleText(CString &strVisible)
{
  ASSERT_VALID(this);

  CFontObject *pFont = m_pLayout->GetFont();

  // count max characters to be visible in the edit box
  int i, nWidth = 0, nVisibleCount = 0;
  for(i = m_nFirstVisible; i < m_strText.GetLength(); i++){
    nWidth += pFont->GetCharSize(m_strText[i]).cx;
    nVisibleCount++;
    if(nWidth > m_rcInner.Width()) break;
  }

  // get the visible text
  strVisible = m_strText.Mid(m_nFirstVisible, nVisibleCount);

  return nVisibleCount;
}

BOOL CEditBox::ScrollToCaret()
{
  ASSERT_VALID(this);

  if(m_nCaretPosition < m_nFirstVisible){
    m_nFirstVisible = m_nCaretPosition;
    if(m_nFirstVisible < 0) m_nFirstVisible = 0;
    return TRUE;
  }

  CString strVisible;
  int nLastVisible = GetVisibleText(strVisible) + m_nFirstVisible;
  if(nLastVisible == 0) nLastVisible = 1; // small hack for the empty ones
  if(m_nCaretPosition >= (nLastVisible - 1)){
    // if the cursor goes out of the edit box -> scroll it
    int nWidth = 0;
    CFontObject *pFont = m_pLayout->GetFont();
    
    // start with character beoynd the cursor (better visibility)
    int i;
    i = m_nCaretPosition;
    if(i >= m_strText.GetLength())
      i = m_strText.GetLength() - 1;
    
    // count characters which could be displayed
    for(; i >= 0; i--){
      nWidth += pFont->GetCharSize(m_strText[i]).cx;
      if(nWidth > m_rcInner.Width()){
        i++; break;
      }
    }
    // scrool it
    m_nFirstVisible = i;
    if(m_nFirstVisible < 0) m_nFirstVisible = 0;

    return TRUE;
  }

  return FALSE;
}

void CEditBox::OnLButtonDown(CPoint point)
{
  if(point.x < m_rcInner.left) point.x = m_rcInner.left;
  if(point.x > m_rcInner.right) point.x = m_rcInner.right;
  SetCapture();
  m_bMouseDrag = TRUE;
  m_nCaretPosition = GetCharOn(point);

  SetSelectionStart(m_nCaretPosition);

  ScrollToCaret();
  UpdateRect();
}

void CEditBox::OnLButtonUp(CPoint point)
{
  if(m_bMouseDrag){
    ReleaseCapture();
    m_bMouseDrag = FALSE;
    return;
  }
  CFrameWindow::OnLButtonUp(point);
}

void CEditBox::OnLButtonDblClk(CPoint point)
{
  m_nSelectionStartPosition = 0;
  m_nSelectionStart = 0;
  m_nSelectionEnd = m_strText.GetLength();
  m_nCaretPosition = m_nSelectionEnd;
  ScrollToCaret();
  UpdateRect();
}

void CEditBox::OnMouseMove(CPoint point)
{
  if(m_bMouseDrag){
    if(point.x < m_rcInner.left){
      m_nCaretPosition = m_nFirstVisible - 1;
      if(m_nCaretPosition < 0) m_nCaretPosition = 0;
    }
    else{
      m_nCaretPosition = GetCharOn(point);
    }

    SetSelectionEdge(m_nCaretPosition);
    ScrollToCaret();
    UpdateRect();
  }
  CFrameWindow::OnMouseMove(point);
}

int CEditBox::GetCharOn(CPoint point)
{
  ASSERT_VALID(this);

  int nWidth = m_rcInner.left, i, nPosition = m_strText.GetLength();
  CFontObject *pFont = m_pLayout->GetFont();

  if(point.x > m_rcInner.right) point.x = m_rcInner.right;
  for(i = m_nFirstVisible; i < m_strText.GetLength(); i++){
    nWidth += pFont->GetCharSize(m_strText[i]).cx;
    if(nWidth > point.x){
      nPosition = i;
      break;
    }
  }
  return nPosition;
}

void CEditBox::SetSelectionEdge(int nPosition)
{
  ASSERT_VALID(this);

  if(nPosition < m_nSelectionStartPosition){
    m_nSelectionStart = nPosition;
    m_nSelectionEnd = m_nSelectionStartPosition;
  }
  else{
    m_nSelectionStart = m_nSelectionStartPosition;
    m_nSelectionEnd = nPosition;
  }
}

void CEditBox::SetSelectionStart(int nPosition)
{
  ASSERT_VALID(this);

  m_nSelectionStartPosition = nPosition;
  m_nSelectionStart = 0;
  m_nSelectionEnd = 0;
}

BOOL CEditBox::NoSelection()
{
  if(m_nSelectionStart < 0) return TRUE;
  if(m_nSelectionStart == m_nSelectionEnd) return TRUE;
  return FALSE;
}

void CEditBox::DeleteSelection()
{
  if(NoSelection()) return;
  
  // move the caret to the right position
  if(m_nCaretPosition >= m_nSelectionEnd){
    m_nCaretPosition -= m_nSelectionEnd - m_nSelectionStart;
  }
  else{
    if(m_nCaretPosition >= m_nSelectionStart){
      m_nCaretPosition = m_nSelectionStart - 1;
      if(m_nCaretPosition < 0) m_nCaretPosition = 0;
    }
  }

  // remove the selection
  m_strText.Delete(m_nSelectionStart, m_nSelectionEnd - m_nSelectionStart);
  m_nSelectionStart = -1;
  m_nSelectionEnd = 0;

  InlayEvent(E_CHANGE, 0);

  ScrollToCaret();
}

CString CEditBox::GetSelection()
{
  CString strSel;

  if(NoSelection()){
    strSel.Empty();
    return strSel;
  }

  strSel = m_strText.Mid(m_nSelectionStart, m_nSelectionEnd - m_nSelectionStart);
  return strSel;
}

void CEditBox::InsertText(CString strText, int nPosition)
{
  // move the cursor
  if(m_nCaretPosition >= nPosition){
    m_nCaretPosition += strText.GetLength();
  }

  // insert the text
  m_strText.Insert(nPosition, strText);
  if(m_nMaxLength > -1){
    if(m_strText.GetLength() > m_nMaxLength){
      m_strText = m_strText.Left(m_nMaxLength);
      if(m_nCaretPosition > m_nMaxLength){
        m_nCaretPosition = m_nMaxLength;
      }
    }
  }
  InlayEvent(E_CHANGE, 0);

  ScrollToCaret();
}

void CEditBox::Copy()
{
  ASSERT_VALID(this);

  CString strSelection;

  if(NoSelection()) return;
  strSelection = GetSelection();

  HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE | GMEM_DDESHARE,
    strSelection.GetLength() + 1);
  LPVOID pMem = GlobalLock(hMem);
  memcpy(pMem, strSelection, strSelection.GetLength() + 1);
  GlobalUnlock(hMem);

  if(!OpenClipboard(NULL)){
    GlobalFree(hMem);
    return;
  }
  EmptyClipboard();
  SetClipboardData(CF_TEXT, hMem);
  CloseClipboard();
}

void CEditBox::Paste()
{
  ASSERT_VALID(this);

  HANDLE hMem;
  LPVOID pMem;
  CString strText;

  if(!OpenClipboard(NULL)){
    return;
  }
  hMem = GetClipboardData(CF_TEXT);
  CloseClipboard();

  if(hMem == NULL) return;
  pMem = GlobalLock(hMem);
  strText = (char *)pMem;
  GlobalUnlock(hMem);

  if(!NoSelection()){
    DeleteSelection();
  }
  InsertText(strText, m_nCaretPosition);

  UpdateRect();
}

void CEditBox::Cut()
{
  ASSERT_VALID(this);
  if(NoSelection()) return;
  Copy();
  DeleteSelection();
  InlayEvent(E_CHANGE, 0);
  UpdateRect();
}

void CEditBox::SetMaxLength(int nMaxLength)
{
  ASSERT_VALID(this);
  ASSERT(nMaxLength >= -1);

  m_nMaxLength = nMaxLength;
  if(m_nMaxLength > -1){
    if(m_strText.GetLength() > m_nMaxLength){
      m_strText = m_strText.Left(m_nMaxLength);
      m_nFirstVisible = 0;
      m_nCaretPosition = 0;
      SetSelectionStart(0);
      SetSelectionEdge(0);
      InlayEvent(E_CHANGE, 0);
      UpdateRect();
    }
  }
}

void CEditBox::SetPassword(BOOL bPassword)
{
  m_bPassword = bPassword;
}
