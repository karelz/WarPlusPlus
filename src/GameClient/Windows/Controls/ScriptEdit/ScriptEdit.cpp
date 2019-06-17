// ScriptEdit.cpp: implementation of the CScriptEdit class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ScriptEdit.h"

#include "..\..\Keyboard.h"
#include "..\..\Mouse.h"
#include "..\..\Timer.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

IMPLEMENT_DYNAMIC(CScriptEdit, CFrameWindow)

BEGIN_OBSERVER_MAP(CScriptEdit, CFrameWindow)
  BEGIN_NOTIFIER(VerticalScrollID)
    EVENT(CScriptEditScroll::E_ACTIVATED)
      OnScrollActivated();
      break;
    ON_STEPUP(OnVerticalStepUp)
    ON_STEPDOWN(OnVerticalStepDown)
    ON_PAGEUP(OnVerticalPageUp)
    ON_PAGEDOWN(OnVerticalPageDown)
    ON_POSITION(OnVerticalPosition)
  END_NOTIFIER()

  BEGIN_NOTIFIER(HorizontalScrollID)
    EVENT(CScriptEditScroll::E_ACTIVATED)
      OnScrollActivated();
      break;
    ON_STEPUP(OnHorizontalStepUp)
    ON_STEPDOWN(OnHorizontalStepDown)
    ON_PAGEUP(OnHorizontalPageUp)
    ON_PAGEDOWN(OnHorizontalPageDown)
    ON_POSITION(OnHorizontalPosition)
  END_NOTIFIER()

  BEGIN_ANIMS()
    ON_ANIMSREPAINT()
  END_ANIMS()

  BEGIN_KEYBOARD()
    ON_KEYDOWN()
    ON_CHAR()
  END_KEYBOARD()

  BEGIN_MOUSE()
    ON_MOUSEMOVE()
    ON_LBUTTONDOWN()
    ON_LBUTTONUP()
  END_MOUSE()

  BEGIN_TIMER()
    ON_TIMETICK()
  END_TIMER()
END_OBSERVER_MAP(CScriptEdit, CFrameWindow)

DWORD CScriptEdit::m_dwScrollTickDelay = 250;

CScEditLex *CScriptEdit::m_pScEditLex = NULL;
DWORD CScriptEdit::m_dwScEditLexReferenceCount = 0;

CScriptEdit::CScriptEdit()
{
  m_pLayout = NULL;
  m_bMouseDrag = FALSE;
  m_bTimerTicked = FALSE;

  m_nCaretLine = 0;
  m_nCaretColumn = 0;
  m_nCaretRealColumn = 0;

  m_bReadOnly = FALSE;
}

CScriptEdit::~CScriptEdit()
{

}

#ifdef _DEBUG

void CScriptEdit::AssertValid() const
{
  ASSERT_VALID(m_pLayout);

  CFrameWindow::AssertValid();
}

void CScriptEdit::Dump(CDumpContext &dc) const
{
  CFrameWindow::Dump(dc);
}

#endif


BOOL CScriptEdit::Create(CRect &rcBoundRect, CScriptEditLayout *pLayout, CWindow *pParent)
{
  ASSERT_VALID(pLayout);

  CRect rcBound(rcBoundRect);

  m_pLayout = pLayout;
  m_nLineHeight = m_pLayout->GetFont()->GetCharSize('a').cy;

  // we have the tabstop -> we want the keyboard input
  m_bTabStop = TRUE;
  // also we want doubleclicks
  m_bDoubleClk = TRUE;

  // small down the rect of the edit -> scroll bars
  rcBound.right -= pLayout->m_VerticalScrollControlLayout.GetWidth();
  rcBound.bottom -= pLayout->m_HorizontalScrollControlLayout.GetHeight();

  m_rcInner.left = m_pLayout->m_nLeftMargin;
  m_rcInner.right = rcBound.Width() - m_pLayout->m_nRightMargin;
  m_rcInner.top = m_pLayout->m_nTopMargin;
  m_rcInner.bottom = rcBound.Height() - m_pLayout->m_nBottomMargin;

  m_Caret.Create(m_pLayout->m_pCaret, this);

  m_nSelectionStartLine = 0;
  m_nSelectionStartColumn = 0;
  m_nSelectionEndLine = 0;
  m_nSelectionEndColumn = 0;
  m_nSelectionUserStartLine = 0;
  m_nSelectionUserStartColumn = 0;

  m_bMouseDrag = FALSE;

  g_pTimer->Connect(this, m_dwScrollTickDelay);

  // create the lex if it isn't
  if(m_pScEditLex == NULL){
    m_pScEditLex = new CScEditLex();
  }
  // add the reference counter
  m_dwScEditLexReferenceCount++;

  // create the lex input object
  m_ScEdLexInput.Create(this);


  // create first clear line with error style
  SLine *pLine = new SLine();
  SStyleNode *pNode = new SStyleNode();
  m_aLines.SetSize(1);
  m_aLines[0] = pLine;
  pLine->m_strText.Empty();
  pLine->m_pStyles = pNode;
  pLine->m_nWidth = 0;
  pNode->m_nColumn = 0;
  pNode->m_dwStyle = CScEditLexToken::SLTError;
  pNode->m_pNext = NULL;

  SetFirstVisible(0, 0);
  SetCaretPosition(0, 0);

  if(!CFrameWindow::Create(rcBound, pLayout, pParent, FALSE))
    return FALSE;

  CRect rcScroll;
  // create the vertical scroll bar
  rcScroll.left = rcBound.right; rcScroll.right = rcBound.right;
  rcScroll.top = rcBound.top; rcScroll.bottom = rcBound.bottom;
  if(!m_VerticalScroll.Create(rcScroll, &m_pLayout->m_VerticalScrollControlLayout,
    pParent, FALSE)) return FALSE;
  m_VerticalScroll.Connect(this, VerticalScrollID);

  // create the horizontal scroll bar
  rcScroll.left = rcBound.left; rcScroll.right = rcBound.right;
  rcScroll.top = rcBound.bottom; rcScroll.bottom = rcBound.bottom;
  if(!m_HorizontalScroll.Create(rcScroll, &m_pLayout->m_HorizontalScrollControlLayout,
    pParent, TRUE)) return FALSE;
  m_HorizontalScroll.Connect(this, HorizontalScrollID);

  UpdateScrolls();
  // recompute the highlighting
  ComputeHighlighting(0, 0, 0, 1);

  return TRUE;
}

void CScriptEdit::Delete()
{
  if(m_bMouseDrag) ReleaseCapture();

  // decrease the ref. counter
  m_dwScEditLexReferenceCount--;
  // if it goes down to zero -> delete the lex object
  if(m_dwScEditLexReferenceCount <= 0){
    if(m_pScEditLex)
      delete m_pScEditLex;
    m_pScEditLex = NULL;
  }

  // destroy the lex input object
  m_ScEdLexInput.Delete();

  m_pLayout = NULL;

  if(g_pTimer != NULL) g_pTimer->Disconnect(this);

  m_VerticalScroll.Delete();
  m_HorizontalScroll.Delete();

  DeleteLines();

  CFrameWindow::Delete();
}

BOOL CScriptEdit::OnActivate()
{
  CFrameWindow::OnActivate();

  // we have to deactivate the frame if we are active
  // but don't have the focus
  if(!HasFocus()){
    m_bActiveFrame = FALSE;
    ChooseLayout();
  }
  return TRUE;
}

void CScriptEdit::OnSetFocus()
{
  CFrameWindow::OnSetFocus();

  // activate the frame -> set the focus
  if(!m_bActiveFrame){
    m_bActiveFrame = TRUE;
    ChooseLayout();
    UpdateRect();
  }

  m_Caret.Play();
}

void CScriptEdit::OnLoseFocus()
{
  CFrameWindow::OnLoseFocus();

  // deactivate the frame -> lose the focus
  if(m_bActiveFrame){
    m_bActiveFrame = FALSE;
    ChooseLayout();
    UpdateRect();
  }

  if(m_bMouseDrag) ReleaseCapture();

  m_Caret.Stop();
}

void CScriptEdit::OnAnimsRepaint(CAnimationInstance *pAnim)
{
  if(pAnim == (&m_Caret)){
    // update range around the caret
    UpdateRange(m_nCaretLine, m_nCaretRealColumn - 1, m_nCaretLine, m_nCaretRealColumn + 1);
    return;
  }
  CFrameWindow::OnAnimsRepaint(pAnim);
}

void CScriptEdit::DeleteLine(SLine *pLine)
{
  ASSERT(pLine != NULL);

  // delete style nodes list
  SStyleNode *pNode = pLine->m_pStyles, *pDelNode;
  while(pNode != NULL){
    pDelNode = pNode;
    pNode = pNode->m_pNext;
    delete pDelNode;
  }

  // delete the line
  delete pLine;
}

void CScriptEdit::DeleteLines()
{
  int i;

  // go through the lines and delete them
  for(i = 0; i < m_aLines.GetSize(); i++){
    DeleteLine(m_aLines[i]);
  }

  // clear the array
  m_aLines.RemoveAll();
}

void CScriptEdit::OnScrollActivated()
{
  this->Activate();
  this->SetFocus();  
}

void CScriptEdit::DrawSelectionLine(int nLine, int nStartCol, int nEndCol, CDDrawSurface *pSurface, CRect *pRectBound, BOOL bEOLSelected)
{
/*  CFontObject *pFont = m_pLayout->GetFont();
  SLine *pLine = m_aLines[nLine];
  CRect rcSel;

  rcSel.top = m_rcInner.top +
    (nLine - m_nFirstVisibleLine) * m_nLineHeight;
  rcSel.bottom = rcSel.top + m_nLineHeight;
  rcSel.left = pFont->GetTextSize(pLine->m_strText.Left(nStartCol)).cx -
    m_nFirstVisibleColumn + m_rcInner.left;
  rcSel.right = rcSel.left + pFont->GetTextSize(pLine->m_strText.Mid(nStartCol, nEndCol - nStartCol)).cx;
  if(bEOLSelected) rcSel.right += pFont->GetCharSize(' ').cx;
  rcSel.IntersectRect(&rcSel, pRectBound);
  rcSel.IntersectRect(&rcSel, &m_rcInner);

  if(rcSel.IsRectEmpty()) return;
  
  int nXPos, nYPos, nXLeft, nYLeft;
  CRect rcSrc; rcSrc.left = 0; rcSrc.top = 0;
  nYLeft = rcSel.Height(); nYPos = rcSel.top;
  while(nYLeft > 0){
    if(nYLeft >= (int)pSelection->GetHeight())
      rcSrc.bottom = pSelection->GetHeight();
    else
      rcSrc.bottom = nYLeft;

    nXLeft = rcSel.Width(); nXPos = rcSel.left;
    while(nXLeft > 0){
      if(nXLeft >= (int)pSelection->GetWidth())
        rcSrc.right = pSelection->GetWidth();
      else
        rcSrc.right = nXLeft;

      pSurface->Paste(nXPos, nYPos, pSelection, &rcSrc);

      nXPos += rcSrc.Width();
      nXLeft -= rcSrc.Width();
    }

    nYPos += rcSrc.Height();
    nYLeft -= rcSrc.Height();
  }*/
}

void CScriptEdit::Draw(CDDrawSurface *pSurface, CRect *pRectBound)
{
  CFrameWindow::Draw(pSurface, pRectBound);

  int i;
  CFontObject *pFont = m_pLayout->GetFont();
  CRect rcOldClip, rcClip;

  pSurface->GetClipRect(&rcClip);
  rcClip.IntersectRect(&rcClip, &m_rcInner);
  pSurface->SetClipRect(&rcClip, &rcOldClip);

/*  // draw the selection
  if(!IsSelectionEmpty()){
    CDDrawSurface *pSelection = m_Selection.Frame();
    CRect rcSel;

    if(m_nSelectionStartLine == m_nSelectionEndLine){
      DrawSelectionLine(m_nSelectionStartLine, m_nSelectionStartColumn,
        m_nSelectionEndColumn, pSurface, pRectBound, FALSE);
    }
    else{
      DrawSelectionLine(m_nSelectionStartLine, m_nSelectionStartColumn,
        m_aLines[m_nSelectionStartLine]->m_strText.GetLength(),
        pSurface, pRectBound, TRUE);

      int i;
      for(i = m_nSelectionStartLine + 1; i < m_nSelectionEndLine; i++){
        DrawSelectionLine(i, 0, m_aLines[i]->m_strText.GetLength(),
          pSurface, pRectBound, TRUE);
      }

      DrawSelectionLine(m_nSelectionEndLine, 0, m_nSelectionEndColumn,
        pSurface, pRectBound, FALSE);
    }
  }*/

  // draw the text

  CString strText;
  int nYPos, nLinePos, nXPos;
  int nFirstLine, nLastLine, nLastLinePos, nTextLen, nOldLinePos;
  BOOL bNewLine, bSelection;
  SStyleNode *pNode;
	CScriptEditLayout::STokenStyle *pTokenStyle;
  SLine *pLine;
  // compute the line range to draw
  nFirstLine = m_nFirstVisibleLine;
  if(pRectBound->top > m_rcInner.top){
    nFirstLine += (pRectBound->top - m_rcInner.top) / m_nLineHeight;
  }
  if(nFirstLine >= m_aLines.GetSize()) goto NoTextDraw;
  nLastLine = m_nLastVisibleLine;
  nLastLinePos = (nLastLine - m_nFirstVisibleLine) * m_nLineHeight + m_rcInner.top;
  if(pRectBound->bottom < nLastLinePos){
    nLastLine -= (nLastLinePos - pRectBound->bottom) / m_nLineHeight + 1;
  }
  if(nLastLine < nFirstLine) 
    goto NoTextDraw;

  // if the first line to draw doesn't have style node
  // with pos 0 -> go up in lines to find the last
  // style node before
  pLine = m_aLines[nFirstLine];
  if((pLine->m_pStyles == 0) || (pLine->m_pStyles->m_nColumn > 0)){
    int nLine = nFirstLine;
    while(nLine >= 0){
      nLine--;
      pNode = m_aLines[nLine]->m_pStyles;
      if(pNode == NULL) continue;     // if none -> one line up
      while(pNode->m_pNext != NULL){  // go to the end of the list
        pNode = pNode->m_pNext;
      }
      break;
    }
    if(nLine < 0) ASSERT(FALSE); // there should be the first node
  }
  else{
    pNode = pLine->m_pStyles;
  }

  // now in pNode is the style for the begining of the first line
  // go through all lines to draw
  nYPos = m_rcInner.top + (nFirstLine - m_nFirstVisibleLine) * m_nLineHeight;
  for(i = nFirstLine; i <= nLastLine; i++){
    pLine = m_aLines[i];
    nOldLinePos = 0; nLinePos = 0; nXPos = m_rcInner.left - m_nFirstVisibleColumn;
    nTextLen = pLine->m_strText.GetLength();

    if((i >= m_nSelectionStartLine) && (i <= m_nSelectionEndLine))
      bSelection = TRUE;
    else
      bSelection = FALSE;

    if((pLine->m_pStyles != NULL) && (pLine->m_pStyles->m_nColumn == 0)){
      pNode = pLine->m_pStyles;
      bNewLine = FALSE;
    }
    while(nLinePos < nTextLen){
      pTokenStyle = m_pLayout->m_aTokenStyles[pNode->m_dwStyle];
      if(pNode->m_pNext != NULL){ // if there is something
        pNode = pNode->m_pNext;   // cut some part of the text
        strText = pLine->m_strText.Mid(nLinePos, pNode->m_nColumn - nLinePos);
        nLinePos = pNode->m_nColumn;
        bNewLine = FALSE;
      }
      else{
        if(bNewLine && (pLine->m_pStyles != NULL)){
          pNode = pLine->m_pStyles;
          strText = pLine->m_strText.Mid(nLinePos, pNode->m_nColumn - nLinePos);
          nLinePos = pNode->m_nColumn;
          bNewLine = FALSE;
        }
        else{
          strText = pLine->m_strText.Mid(nLinePos);
          nLinePos = pLine->m_strText.GetLength();
        }
      }

      if(bSelection){
        int nStart, nEnd;

        // compute the start and the end of the selection inside the painted text (strText)
        if(i == m_nSelectionStartLine){
          if(nOldLinePos >= m_nSelectionStartColumn)
            nStart = nOldLinePos;
          else
            nStart = m_nSelectionStartColumn;
        }
        else{
          nStart = nOldLinePos;
        }
        if(i == m_nSelectionEndLine){
          if(nLinePos <= m_nSelectionEndColumn)
            nEnd = nLinePos;
          else
            nEnd = m_nSelectionEndColumn;
        }
        else{
          nEnd = nLinePos;
        }

        CString strSelText;
        if(nEnd <= nStart) goto NormalPaint;
				if(nStart > nLinePos) goto NormalPaint;

        if(nStart > nOldLinePos){
          strSelText = strText.Left(nStart - nOldLinePos);
          pTokenStyle->m_pNormalFont->PaintText(nXPos, nYPos, strSelText, pSurface);
          nXPos += pTokenStyle->m_pNormalFont->GetTextSize(strSelText).cx;
        }
        strSelText = strText.Mid(nStart - nOldLinePos, nEnd - nStart);
        pTokenStyle->m_pSelectionFont->PaintText(nXPos, nYPos, strSelText, pSurface);
        nXPos += pTokenStyle->m_pSelectionFont->GetTextSize(strSelText).cx;
        if((i != m_nSelectionEndLine) && (nEnd >= pLine->m_strText.GetLength())){
          // we have to be at the end of line, and we are to draw the space char here
          strSelText = " ";
          pTokenStyle->m_pSelectionFont->PaintText(nXPos, nYPos, strSelText, pSurface);
          nXPos += pTokenStyle->m_pSelectionFont->GetTextSize(strSelText).cx;
        }
        if(nEnd < nLinePos){
          strSelText = strText.Mid(nEnd - nOldLinePos);
          pTokenStyle->m_pNormalFont->PaintText(nXPos, nYPos, strSelText, pSurface);
          nXPos += pTokenStyle->m_pNormalFont->GetTextSize(strSelText).cx;
        }
        goto AlreadyPainted;
      }

NormalPaint:
      ;
      pTokenStyle->m_pNormalFont->PaintText(nXPos, nYPos, strText, pSurface);
      nXPos += pFont->GetTextSize(strText).cx;

AlreadyPainted:
      ;
      nOldLinePos = nLinePos;
    }

    bNewLine = TRUE;
    nYPos += m_nLineHeight;
  }

NoTextDraw:

  // now draw the caret
  if((m_nCaretLine >= nFirstLine) && (m_nCaretLine <= nLastLine) && HasFocus()){
    CDDrawSurface *pCaret = m_Caret.Frame();
    int nYPos = (m_nCaretLine - m_nFirstVisibleLine) * m_nLineHeight + m_rcInner.top;
    int nXPos = m_rcInner.left - m_nFirstVisibleColumn - pCaret->GetWidth() / 2 - 1+
      m_pLayout->GetFont()->GetTextSize(m_aLines[m_nCaretLine]->m_strText.Left(m_nCaretRealColumn)).cx;
    if((nXPos < m_rcInner.left) && (m_nFirstVisibleColumn == 0)) nXPos = m_rcInner.left;
    int nLeft = m_nLineHeight;
    while(nLeft > 0){
      if((int)(pCaret->GetHeight()) < nLeft){
        pSurface->Paste(nXPos, nYPos, pCaret);
        nYPos += pCaret->GetHeight();
        nLeft -= pCaret->GetHeight();
      }
      else{
        CRect rc(0, 0, pCaret->GetWidth(), nLeft);
        pSurface->Paste(nXPos, nYPos, pCaret, &rc);
        break;
      }
    }
  }

  pSurface->SetClipRect(&rcOldClip, NULL);
}

void CScriptEdit::SetCaretPosition(int nLine, int nColumn)
{
  UpdateRange(m_nCaretLine, m_nCaretRealColumn - 1, m_nCaretLine, m_nCaretRealColumn + 1);

  if(nLine >= m_aLines.GetSize()) nLine = m_aLines.GetSize() - 1;
  if(nLine < 0) nLine = 0;
  m_nCaretLine = nLine;

  if(m_aLines.GetSize() > 0){

	SLine *pLine = m_aLines[nLine];
	if(nColumn < 0) nColumn = 0;
	if(nColumn > pLine->m_strText.GetLength()){
	    m_nCaretRealColumn = pLine->m_strText.GetLength();
	}
	else{
	    m_nCaretRealColumn = nColumn;
	}
	m_nCaretColumn = nColumn;
  }
  else{
    m_nCaretRealColumn = 0;
	m_nCaretColumn = 0;
	m_nCaretLine = 0;
  }

  m_Caret.SetPosition(0); // restart caret animation
  UpdateRange(m_nCaretLine, m_nCaretRealColumn - 1, m_nCaretLine, m_nCaretRealColumn + 1);
}

void CScriptEdit::SetFirstVisible(int nLine, int nColumn)
{
  if(nLine < 0) nLine = 0;
  if(nLine >= m_aLines.GetSize()) nLine = m_aLines.GetSize() - 1;
  if(nColumn < 0) nColumn = 0;
  m_nFirstVisibleLine = nLine;
  m_nFirstVisibleColumn = nColumn;
  
  m_nLastVisibleLine = m_nFirstVisibleLine +
    m_rcInner.Height() / m_nLineHeight;
  if(m_nLastVisibleLine >= m_aLines.GetSize()){
    m_nLastVisibleLine = m_aLines.GetSize() - 1;
  }
  if(m_nLastVisibleLine < m_nFirstVisibleLine)
    m_nLastVisibleLine = m_nFirstVisibleLine;
}

void CScriptEdit::ComputeHighlighting(int nStartLine, int nStartColumn, int nEndLine, int nEndColumn)
{
  int nFirstLine; // the really first line to begin
    // the lex analyzer on (we have to go back to find
    // beggining of some lexical element to start from)
  int nFirstColumn; // the start column of that token

  // go through the list of token on each line and try to find
  // some token, which starts before out first column
  // if no one, go up one line and try it again but
  // for the last column (some nonsence number like 1000000)
  SLine *pLine;
  nFirstColumn = nStartColumn;
  for(nFirstLine = nStartLine; nFirstLine >= 0; nFirstLine--){
    pLine = m_aLines[nFirstLine];

    // go through the tokens and find the one right before
    // the nFirstColumn
    SStyleNode *pNode, *pPrevNode = NULL;
    pNode = pLine->m_pStyles;
    while(pNode != NULL){
      if(pNode->m_nColumn > nFirstColumn)
        break;
      pPrevNode = pNode;
      pNode = pNode->m_pNext;
    }
    // if the pPrevNode == NULL -> no such token
    // so we have to go up one line and try it again
    if(pPrevNode == NULL){
      nFirstColumn = 1000000; // after last column (some very big number)
      continue; // go up one line and try again
    }

    // the start column if the one of the found token
    nFirstColumn = pPrevNode->m_nColumn;
    // if there is some pPrevNode -> we found it
    break;
  }
  // if we passed the beginnig -> start from the first line and first column
  if(nFirstLine < 0){ nFirstLine = 0; nFirstColumn = 0; }

  // we have to find first node after the range
  // that position is safe and doesn't have to be recomputed
  // so we will shift the end of the region on that position
  {
    SLine *pLine;
    while(nEndLine < m_aLines.GetSize()){
      pLine = m_aLines[nEndLine];
      SStyleNode *pNode = pLine->m_pStyles;
      while(pNode != NULL){
        if(pNode->m_nColumn >= nEndColumn){ // we've found it
          nEndColumn = pNode->m_nColumn;
          goto EndPosFound;
        }
        pNode = pNode->m_pNext;
      }
      nEndColumn = 0; // on next line we just have to find the first node
      nEndLine++;
    }
    // none found -> set it as the end of whole text
    nEndLine = m_aLines.GetSize() - 1;
    nEndColumn = m_aLines[nEndLine]->m_strText.GetLength();
EndPosFound:
    ;
  }

  // find the node after which we will insert first node
  // of new got style
  // this will hold the pointer to the place, where's stored the pointer
  // to the node before which we are to insert the first of our nodes
  SStyleNode **pInsertOn;
  {
    SLine *pLine;
    pLine = m_aLines[nFirstLine];
    // start at the begining of the line
    pInsertOn = &(pLine->m_pStyles);
    while((*pInsertOn) != NULL){
      if((*pInsertOn)->m_nColumn >= nFirstColumn){
        break;
      }
      pInsertOn = &((*pInsertOn)->m_pNext);
    }
  }

  // now we have in nFirstLine and nFirstColumn position
  // of the first letter of some lex token, from which one
  // we will start our recomputiation of highlighting
  // So -> create and set up lex input
  m_ScEdLexInput.SetRange(nFirstLine, nFirstColumn, nEndLine, nEndColumn);

  // reset the lex to our input
  m_pScEditLex->Reset(&m_ScEdLexInput, nFirstLine, nFirstColumn);

  int nCurrentLine, nCurrentColumn;
  nCurrentLine = nFirstLine; nCurrentColumn = nFirstColumn;
  CString strToken;

  CScEditLexToken Token;

  // the pointer to the place where's stored
  // the pointer to the next node in the old list
  // it's the candidate for a deletion
  SStyleNode **pOldNodeForDeletion;
  // the line of that node
  DWORD dwOldNodeForDeletionLine;

  // set it to the first node after the start
  pOldNodeForDeletion = pInsertOn;
  dwOldNodeForDeletionLine = nFirstLine;
  // if we are at the end of the list -> go on next line (and so on)
  while(*pOldNodeForDeletion == NULL){
    dwOldNodeForDeletionLine++;
    if(dwOldNodeForDeletionLine >= (DWORD)(m_aLines.GetSize())) break;
    pOldNodeForDeletion = &(m_aLines[dwOldNodeForDeletionLine]->m_pStyles);
  }
  // now we have in pOldNodeForDeletion and dwOldNodeForDeletionLine
  // the position of the next token in the list of them

  // remeber last added token
  // we can join the same styles together
  SStyleNode *pLastAdded = NULL;

  pLine = m_aLines[nCurrentLine];

  while(TRUE){
    // read next token
    m_pScEditLex->GetToken(Token);

    // if we reached the end of it -> end our loop too
    if(Token.m_nToken == CScEditLexToken::SLTEndOfFile)
      break;

    // delete all nodes we are to rewrite
    // also detect if the node we are adding is not the same as the one we are to delete
    // if so -> end the lex analyzer (here nothing changed, so it can't change in future)

    // so we are to delete the nodes in the pOldNodeForDeletion
    // so go throug them
    while(TRUE){
      // if none -> all is done
      if(*pOldNodeForDeletion == NULL) break;

      // if the old node is on or before the last token line
      // it's a good candidate for deletion
      if((dwOldNodeForDeletionLine < Token.m_nEndRow) ||
        ((dwOldNodeForDeletionLine == Token.m_nEndRow) &&
        ((*pOldNodeForDeletion)->m_nColumn < Token.m_nEndColumn))){
        
        // if the one to delete is the same as the one to add -> end it now
        if((dwOldNodeForDeletionLine == Token.m_nStartRow) &&
          ((*pOldNodeForDeletion)->m_nColumn == Token.m_nStartColumn) &&
          ((*pOldNodeForDeletion)->m_dwStyle = (DWORD)Token.m_nToken)){
          if(dwOldNodeForDeletionLine > (DWORD)nEndLine) goto EndTheTokenLoop;
          if((dwOldNodeForDeletionLine == (DWORD)nEndLine) &&
            ((*pOldNodeForDeletion)->m_nColumn >= nEndColumn))
            goto EndTheTokenLoop;
        }
        
        // now delete it
        SStyleNode *pDelNode = *pOldNodeForDeletion;
        *pOldNodeForDeletion = (*pOldNodeForDeletion)->m_pNext;
        delete pDelNode;
        
        // if at the end of the list go to next line (and so on)
        while(*pOldNodeForDeletion == NULL){
          dwOldNodeForDeletionLine++;
          if(dwOldNodeForDeletionLine >= (DWORD)(m_aLines.GetSize())) break;
          pOldNodeForDeletion = &(m_aLines[dwOldNodeForDeletionLine]->m_pStyles);
        }
        
        
        // continue on the next token in pOldNodeForDeletion
        // cause it might be also in the new token range
        continue;
      }

      // yes we have delete all of them
      // so break it
      break;
    }

    // now insert new token style

    // try to join it with the last one
/*    if(pLastAdded != NULL){
      if(pLastAdded->m_dwStyle == (DWORD)(Token.m_nToken))
        // continue with the next token
        goto NextTokenEntry;
    }*/
    SStyleNode *pNewNode;
    pNewNode = new SStyleNode();
    pNewNode->m_dwStyle = Token.m_nToken;
    pNewNode->m_nColumn = nCurrentColumn;
    pNewNode->m_pNext = *pInsertOn;
    *pInsertOn = pNewNode;
//    pLastAdded = pNewNode;

    if(Token.m_nEndRow >= (DWORD)(m_aLines.GetSize())){
      Token.m_nEndRow = m_aLines.GetSize() - 1;
      Token.m_nEndColumn = m_aLines[Token.m_nEndRow]->m_strText.GetLength();
    }

    if(pInsertOn == pOldNodeForDeletion){
      // compute new InsertOn and also move the pOldNodeForDeletion
      if(Token.m_nEndRow == (DWORD)(nCurrentLine)){
        pInsertOn = &(pNewNode->m_pNext);
      }
      else{
        pInsertOn = &(m_aLines[Token.m_nEndRow]->m_pStyles);
      }
      pOldNodeForDeletion = pInsertOn;
      dwOldNodeForDeletionLine = Token.m_nEndRow;
    }
    else{
      // compute new InsertOn only
      if(Token.m_nEndRow == (DWORD)(nCurrentLine)){
        pInsertOn = &(pNewNode->m_pNext);
      }
      else{
        pInsertOn = &(m_aLines[Token.m_nEndRow]->m_pStyles);
      }
    }

/*NextTokenEntry:
    ;*/
    // move current position
    nCurrentLine = Token.m_nEndRow; pLine = m_aLines[nCurrentLine];
    nCurrentColumn = Token.m_nEndColumn;
  }

EndTheTokenLoop:
  ;

  if((m_aLines[0]->m_pStyles == NULL) || (m_aLines[0]->m_pStyles->m_nColumn > 0)){
    SStyleNode *pErrorNode = new SStyleNode();

    pErrorNode->m_dwStyle = CScEditLexToken::SLTError;
    pErrorNode->m_nColumn = 0;
    pErrorNode->m_pNext = m_aLines[0]->m_pStyles;
    m_aLines[0]->m_pStyles = pErrorNode;
  }

  // now update graphics
  CRect rcUpdate;
  rcUpdate.top = m_rcInner.top + (nFirstLine - m_nFirstVisibleLine) * m_nLineHeight;
  rcUpdate.bottom = m_rcInner.top + (nCurrentLine - m_nFirstVisibleLine + 1) * m_nLineHeight;
  rcUpdate.left = m_rcInner.left;
  rcUpdate.right = m_rcInner.right;
  rcUpdate.IntersectRect(&rcUpdate, &m_rcInner);
  UpdateRect(&rcUpdate);
}

void CScriptEdit::UpdateScrolls()
{
  // first do the vertical scroll
  // just count lines -> whole range
  // count lines on the screen -> one page
  // first visible pos -> position
  m_VerticalScroll.SetRange(0, m_aLines.GetSize());
  m_VerticalScroll.SetPageSize(m_rcInner.Height() / m_nLineHeight);
  m_VerticalScroll.SetPosition(m_nFirstVisibleLine);

  // than the horizontal scroll
  // we have to count max width of line
  int nMaxWidth = 0, nLine;
  for(nLine = 0; nLine < m_aLines.GetSize(); nLine++){
    if(m_aLines[nLine]->m_nWidth > nMaxWidth)
      nMaxWidth = m_aLines[nLine]->m_nWidth;
  }
  nMaxWidth += m_pLayout->GetFont()->GetCharSize('a').cx;
  // max width -> range
  // width of screen -> page
  // first vis comlumn -> pos
  m_HorizontalScroll.SetRange(0, nMaxWidth);
  m_HorizontalScroll.SetPageSize(m_rcInner.Width());
  m_HorizontalScroll.SetPosition(m_nFirstVisibleColumn);
}

void CScriptEdit::OnVerticalStepUp()
{
  if(m_nFirstVisibleLine == 0) return;
  SetFirstVisible(m_nFirstVisibleLine - 1, m_nFirstVisibleColumn);
  UpdateRect(&m_rcInner);
  UpdateScrolls();
}

void CScriptEdit::OnVerticalStepDown()
{
  if(m_nFirstVisibleLine >= (m_aLines.GetSize() - 1)) return;
  SetFirstVisible(m_nFirstVisibleLine + 1, m_nFirstVisibleColumn);
  UpdateRect(&m_rcInner);
  UpdateScrolls();
}

void CScriptEdit::OnVerticalPageUp()
{
  if(m_nFirstVisibleLine == 0) return;
  m_nFirstVisibleLine -= m_rcInner.Height() / m_nLineHeight;
  if(m_nFirstVisibleLine < 0)
    m_nFirstVisibleLine = 0;
  SetFirstVisible(m_nFirstVisibleLine, m_nFirstVisibleColumn);
  UpdateRect(&m_rcInner);
  UpdateScrolls();
}

void CScriptEdit::OnVerticalPageDown()
{
  if(m_nFirstVisibleLine >= (m_aLines.GetSize() - 1)) return;
  m_nFirstVisibleLine += m_rcInner.Height() / m_nLineHeight;
  if(m_nFirstVisibleLine > (m_aLines.GetSize() - 1))
    m_nFirstVisibleLine = m_aLines.GetSize() - 1;
  SetFirstVisible(m_nFirstVisibleLine, m_nFirstVisibleColumn);
  UpdateRect(&m_rcInner);
  UpdateScrolls();
}

void CScriptEdit::OnVerticalPosition(int nNewPosition)
{
  if(m_nFirstVisibleLine == nNewPosition) return;
  m_nFirstVisibleLine = nNewPosition;
  if(m_nFirstVisibleLine < 0)
    m_nFirstVisibleLine = 0;
  if(m_nFirstVisibleLine > (m_aLines.GetSize() - 1))
    m_nFirstVisibleLine = m_aLines.GetSize() - 1;
  SetFirstVisible(m_nFirstVisibleLine, m_nFirstVisibleColumn);
  UpdateRect(&m_rcInner);
  UpdateScrolls();
}

void CScriptEdit::OnHorizontalStepUp()
{
  if(m_nFirstVisibleColumn == 0) return;
  m_nFirstVisibleColumn -= m_pLayout->GetFont()->GetCharSize('A').cx;
  if(m_nFirstVisibleColumn < 0) m_nFirstVisibleColumn = 0;
  SetFirstVisible(m_nFirstVisibleLine, m_nFirstVisibleColumn);
  UpdateRect(&m_rcInner);
  UpdateScrolls();
}

void CScriptEdit::OnHorizontalStepDown()
{
  if((m_nFirstVisibleColumn + m_rcInner.Width()) >= m_HorizontalScroll.GetRangeMax()) return;
  m_nFirstVisibleColumn += m_pLayout->GetFont()->GetCharSize('A').cx;
  if((m_nFirstVisibleColumn + m_rcInner.Width()) > m_HorizontalScroll.GetRangeMax())
    m_nFirstVisibleColumn = m_HorizontalScroll.GetRangeMax() - m_rcInner.Width();
  SetFirstVisible(m_nFirstVisibleLine, m_nFirstVisibleColumn);
  UpdateRect(&m_rcInner);
  UpdateScrolls();
}

void CScriptEdit::OnHorizontalPageUp()
{
  if(m_nFirstVisibleColumn == 0) return;
  m_nFirstVisibleColumn -= m_rcInner.Width();
  if(m_nFirstVisibleColumn < 0) m_nFirstVisibleColumn = 0;
  SetFirstVisible(m_nFirstVisibleLine, m_nFirstVisibleColumn);
  UpdateRect(&m_rcInner);
  UpdateScrolls();
}

void CScriptEdit::OnHorizontalPageDown()
{
  if((m_nFirstVisibleColumn + m_rcInner.Width()) >= m_HorizontalScroll.GetRangeMax()) return;
  m_nFirstVisibleColumn += m_rcInner.Width();
  if((m_nFirstVisibleColumn + m_rcInner.Width()) > m_HorizontalScroll.GetRangeMax())
    m_nFirstVisibleColumn = m_HorizontalScroll.GetRangeMax() - m_rcInner.Width();
  SetFirstVisible(m_nFirstVisibleLine, m_nFirstVisibleColumn);
  UpdateRect(&m_rcInner);
  UpdateScrolls();
}

void CScriptEdit::OnHorizontalPosition(int nNewPosition)
{
  if(m_nFirstVisibleColumn == nNewPosition) return;
  m_nFirstVisibleColumn = nNewPosition;
  if(m_nFirstVisibleColumn < 0) m_nFirstVisibleColumn = 0;
  if((m_nFirstVisibleColumn + m_rcInner.Width()) > m_HorizontalScroll.GetRangeMax())
    m_nFirstVisibleColumn = m_HorizontalScroll.GetRangeMax() - m_rcInner.Width();
  SetFirstVisible(m_nFirstVisibleLine, m_nFirstVisibleColumn);
  UpdateRect(&m_rcInner);
  UpdateScrolls();
}

void CScriptEdit::UpdateLine(int nLine)
{
  CRect rc;
  SLine *pLine = m_aLines[nLine];
  rc.left = m_rcInner.left;
  rc.right = pLine->m_nWidth + m_rcInner.left - m_nFirstVisibleColumn;
  rc.top = m_rcInner.top + (nLine - m_nFirstVisibleLine) * m_nLineHeight;
  rc.bottom = rc.top + m_nLineHeight;
  UpdateRect(&rc);
}

void CScriptEdit::UpdateRange(int nStartLine, int nStartCol, int nEndLine, int nEndCol)
{
  if(nStartLine >= m_aLines.GetSize()) return;
  if(nEndLine >= m_aLines.GetSize()){
    nEndLine = m_aLines.GetSize() - 1;
    nEndCol = m_aLines[nEndLine]->m_strText.GetLength();
  }
  if(nStartLine > nEndLine) return;
  CRect rc;
  if(nStartCol < 0) nStartCol = 0;
  if(nStartLine == nEndLine){
    int nEndColOld = nEndCol;
    SLine *pLine = m_aLines[nStartLine];
    if(nEndCol > pLine->m_strText.GetLength()) nEndCol = pLine->m_strText.GetLength();
    if(nStartCol > nEndCol) return;
    rc.left = m_rcInner.left - m_nFirstVisibleColumn +
      m_pLayout->GetFont()->GetTextSize(pLine->m_strText.Left(nStartCol)).cx;
    rc.right = rc.left +
      m_pLayout->GetFont()->GetTextSize(pLine->m_strText.Mid(nStartCol, nEndCol - nStartCol)).cx;
    if(nEndCol < nEndColOld) rc.right += m_pLayout->GetFont()->GetCharSize('a').cx;
    rc.top = m_rcInner.top + (nStartLine - m_nFirstVisibleLine) * m_nLineHeight;
    rc.bottom = rc.top + m_nLineHeight;
  }
  else{
    rc.left = m_rcInner.left;
    rc.right = m_rcInner.right;
    rc.top = m_rcInner.top + (nStartLine - m_nFirstVisibleLine) * m_nLineHeight;
    rc.bottom = rc.top + (nEndLine - nStartLine + 1) * m_nLineHeight;
  }
  rc.IntersectRect(&rc, &m_rcInner);
  if(rc.IsRectEmpty()) return;
  UpdateRect(&rc);
}

void CScriptEdit::ScrollToPosition(int nLine, int nCol)
{
  BOOL bUpdate = FALSE;

  if(nLine < 0) nLine = 0;
  if(nLine >= m_aLines.GetSize()) nLine = m_aLines.GetSize() - 1;
  if(nCol < 0) nCol = 0;

  if(nLine < m_nFirstVisibleLine){
    SetFirstVisible(nLine, m_nFirstVisibleColumn);
    bUpdate = TRUE;
  }
  if(nLine >= m_nLastVisibleLine){
    if((nLine - m_nFirstVisibleLine)*m_nLineHeight >= (m_rcInner.Height() - m_nLineHeight)){
      SetFirstVisible(nLine - ((m_rcInner.Height() - m_nLineHeight - 1)/ m_nLineHeight), m_nFirstVisibleColumn);
      bUpdate = TRUE;
    }
  }
  int nColPos = m_pLayout->GetFont()->GetTextSize(m_aLines[nLine]->m_strText.Left(nCol)).cx;
  if(nColPos < m_nFirstVisibleColumn){
    SetFirstVisible(m_nFirstVisibleLine,
      nColPos - m_pLayout->GetFont()->GetCharSize('a').cx);
    bUpdate = TRUE;
  }
  if(nColPos > m_nFirstVisibleColumn + m_rcInner.Width() - 5){
    SetFirstVisible(m_nFirstVisibleLine,
      nColPos - m_rcInner.Width() + 8 * m_pLayout->GetFont()->GetCharSize('a').cx);
    bUpdate = TRUE;
  }

  if(bUpdate){
    UpdateRect(&m_rcInner);
    UpdateScrolls();
  }
}

BOOL CScriptEdit::OnKeyDown(int nChar, DWORD dwFlags)
{
  switch(nChar){
    //////////////////////////////
    // Up arrow key
  case VK_UP:

    // set the position one line up
    SetCaretPosition(m_nCaretLine - 1, m_nCaretColumn);

    // scroll to that position
    ScrollToPosition(m_nCaretLine, m_nCaretRealColumn);

    // if the shift is pressed -> select it
    if(dwFlags & CKeyboard::ShiftDown)
      SetSelectionEdge(m_nCaretLine, m_nCaretRealColumn);
    else
      SetSelectionStart(m_nCaretLine, m_nCaretRealColumn);

    return TRUE;


    //////////////////////////////
    // Down arrow key
  case VK_DOWN:

    // set the position one line down
    SetCaretPosition(m_nCaretLine + 1, m_nCaretColumn);

    // scroll to that position
    ScrollToPosition(m_nCaretLine, m_nCaretRealColumn);

    // if the shift is pressed -> select it
    if(dwFlags & CKeyboard::ShiftDown)
      SetSelectionEdge(m_nCaretLine, m_nCaretRealColumn);
    else
      SetSelectionStart(m_nCaretLine, m_nCaretRealColumn);

    return TRUE;

    //////////////////////////////
    // Left arrow key
  case VK_LEFT:

    // set the position one character left
    SetCaretPosition(m_nCaretLine, m_nCaretRealColumn - 1);

    // scroll to that position
    m_nCaretColumn = m_nCaretRealColumn;
    ScrollToPosition(m_nCaretLine, m_nCaretRealColumn);

    // if the shift is down -> select it
    if(dwFlags & CKeyboard::ShiftDown)
      SetSelectionEdge(m_nCaretLine, m_nCaretRealColumn);
    else
      SetSelectionStart(m_nCaretLine, m_nCaretRealColumn);

    return TRUE;


    //////////////////////////////
    // Right arrow key
  case VK_RIGHT:

    // set the position one character right
    SetCaretPosition(m_nCaretLine, m_nCaretRealColumn + 1);

    // scroll to that position
    m_nCaretColumn = m_nCaretRealColumn;
    ScrollToPosition(m_nCaretLine, m_nCaretRealColumn);

    // if the shift is pressed -> select it
    if(dwFlags & CKeyboard::ShiftDown)
      SetSelectionEdge(m_nCaretLine, m_nCaretRealColumn);
    else
      SetSelectionStart(m_nCaretLine, m_nCaretRealColumn);

    return TRUE;
    
    //////////////////////////////
    // Page down key
  case VK_NEXT: // page down

    // set the position one page down
    SetCaretPosition(m_nCaretLine + (m_rcInner.Height() / m_nLineHeight), m_nCaretColumn);

    // scroll to that position
    ScrollToPosition(m_nCaretLine, m_nCaretRealColumn);

    // if the shift is presse -> select it
    if(dwFlags & CKeyboard::ShiftDown)
      SetSelectionEdge(m_nCaretLine, m_nCaretRealColumn);
    else
      SetSelectionStart(m_nCaretLine, m_nCaretRealColumn);

    return TRUE;


    //////////////////////////////
    // Page up key
  case VK_PRIOR: // page up
    // set the position one page up
    SetCaretPosition(m_nCaretLine - (m_rcInner.Height() / m_nLineHeight), m_nCaretColumn);

    // scroll to that position
    ScrollToPosition(m_nCaretLine, m_nCaretRealColumn);

    // if the shift is down -> select it
    if(dwFlags & CKeyboard::ShiftDown)
      SetSelectionEdge(m_nCaretLine, m_nCaretRealColumn);
    else
      SetSelectionStart(m_nCaretLine, m_nCaretRealColumn);

    return TRUE;

    //////////////////////////////
    // Home key
  case VK_HOME:
    // if the Ctrl is pressed -> go to the beggining of the file
    if(dwFlags & CKeyboard::CtrlDown)
      SetCaretPosition(0, 0);
    else
      SetCaretPosition(m_nCaretLine, 0);

    // scroll to that position
    ScrollToPosition(m_nCaretLine, m_nCaretRealColumn);

    // if the shift is pressed -> select it
    if(dwFlags & CKeyboard::ShiftDown)
      SetSelectionEdge(m_nCaretLine, m_nCaretRealColumn);
    else
      SetSelectionStart(m_nCaretLine, m_nCaretRealColumn);

    return TRUE;

    //////////////////////////////
    // End key
  case VK_END:
    // if the Ctrl is pressed -> go to the end of the file
    if(dwFlags & CKeyboard::CtrlDown)
      SetCaretPosition(m_aLines.GetSize() - 1, m_aLines[m_aLines.GetSize() - 1]->m_strText.GetLength());
    else
      SetCaretPosition(m_nCaretLine, m_aLines[m_nCaretLine]->m_strText.GetLength());

    // scroll to that position
    ScrollToPosition(m_nCaretLine, m_nCaretRealColumn);

    // if the shift is pressed -> select it
    if(dwFlags & CKeyboard::ShiftDown)
      SetSelectionEdge(m_nCaretLine, m_nCaretRealColumn);
    else
      SetSelectionStart(m_nCaretLine, m_nCaretRealColumn);

    return TRUE;

    //////////////////////////////
    // Insert key
  case VK_INSERT:

    // Shift - Insert -> Paste
    if(dwFlags & CKeyboard::ShiftDown){
      Paste();
      return TRUE;
    }

    // Ctrl - Insert -> Copy
    if(dwFlags & CKeyboard::CtrlDown){
      Copy();
      return TRUE;
    }

    break;

    //////////////////////////////
    // Delete key
  case VK_DELETE:

    // Shift - Delete -> Cut
    if(dwFlags & CKeyboard::ShiftDown){
      Cut();
      return TRUE;
    }

    // if read only -> no action
    if(m_bReadOnly) return TRUE;
    InlayEvent(E_MODIFIED, 0);

    // if there was some selection -> delete it
    if(!IsSelectionEmpty()){
      // remember the start of the selection, there we will place the cursor after the operation
      int nLine = m_nSelectionStartLine, nCol = m_nSelectionStartColumn;
      DeleteSelection();
      SetCaretPosition(nLine, nCol);
      m_nCaretColumn = m_nCaretRealColumn;
      ScrollToPosition(m_nCaretLine, m_nCaretRealColumn);
      return TRUE;
    }

    // no selection -> delete one character right to the cursor
    // if we are at the end of the line -> delete line
    if(m_nCaretRealColumn == m_aLines[m_nCaretLine]->m_strText.GetLength()){
      if(m_nCaretLine == (m_aLines.GetSize() - 1)) return TRUE;
      InsertLineText(m_nCaretLine, m_nCaretRealColumn, m_aLines[m_nCaretLine + 1]->m_strText);
      DeleteLine(m_nCaretLine + 1);
    }
    else{
      DeleteLineText(m_nCaretLine, m_nCaretRealColumn, 1);
    }
    // set new selection start
    SetSelectionStart(m_nCaretLine, m_nCaretRealColumn);

    return TRUE;

    //////////////////////////////
    // Backspace key
  case VK_BACK:{}    
    
    // if read only -> no action
    if(m_bReadOnly) return TRUE;
    InlayEvent(E_MODIFIED, 0);

    // some selection ? -> delete it
    if(!IsSelectionEmpty()){
      // remeber the selection start -> there we will place the cursor after the operation
      int nLine = m_nSelectionStartLine, nCol = m_nSelectionStartColumn;
      DeleteSelection();
      SetCaretPosition(nLine, nCol);
      m_nCaretColumn = m_nCaretRealColumn;
      ScrollToPosition(m_nCaretLine, m_nCaretRealColumn);
      return TRUE;
    }

    // no selection -> delete one character left to the cursor
    if(m_nCaretRealColumn == 0){
      if(m_nCaretLine == 0) return TRUE;
      int nCaretLine = m_nCaretLine - 1;
      int nCaretColumn = m_aLines[nCaretLine]->m_strText.GetLength();
      InsertLineText(m_nCaretLine - 1, m_aLines[m_nCaretLine - 1]->m_strText.GetLength(), m_aLines[m_nCaretLine]->m_strText);
      DeleteLine(m_nCaretLine);
      SetCaretPosition(nCaretLine, nCaretColumn);
      ScrollToPosition(m_nCaretLine, m_nCaretRealColumn);
    }
    else{
      DeleteLineText(m_nCaretLine, m_nCaretRealColumn - 1, 1);
    }

    // set new selection start
    SetSelectionStart(m_nCaretLine, m_nCaretRealColumn);

    return TRUE;

    //////////////////////////////
    // Tab key
  case VK_TAB:

    // if the Ctrl is pressed, it's not our key -> throw it away
    if(dwFlags & CKeyboard::CtrlDown)
      break;

    // if read only -> no action
    if(m_bReadOnly) return TRUE;
    InlayEvent(E_MODIFIED, 0);

    // delete the selection (if any)
    DeleteSelection();

    // insert spaces (of tab size)
    {
      CString str; int i;
      for(i = 0; i < m_pLayout->m_nTabSize; i++){
        str += " ";
      }
      InsertLineText(m_nCaretLine, m_nCaretRealColumn, str);
      SetCaretPosition(m_nCaretLine, m_nCaretRealColumn + m_pLayout->m_nTabSize);
      m_nCaretColumn = m_nCaretRealColumn;
      ScrollToPosition(m_nCaretLine, m_nCaretRealColumn);
    }

    // set new selection start
    SetSelectionStart(m_nCaretLine, m_nCaretRealColumn);

    return TRUE;

    //////////////////////////////
    // Return (Enter) key
  case VK_RETURN:

    // if read only -> no action
    if(m_bReadOnly) return TRUE;
    InlayEvent(E_MODIFIED, 0);

    // delete the selection (if any)
    DeleteSelection();

    // insert end of line here
    // intend the new line aproprietly
    {
      CString strNewLine, strSpace;
      int nIndent, i;
      int nCaretLine = m_nCaretLine;
      strNewLine = m_aLines[nCaretLine]->m_strText.Mid(m_nCaretRealColumn);
      DeleteLineText(nCaretLine, m_nCaretRealColumn, strNewLine.GetLength());
      strNewLine.TrimLeft();
      nIndent = GetIndentation(nCaretLine);
      for(i = 0; i < nIndent; i++) strSpace += ' ';
      strNewLine.Insert(0, strSpace);
      InsertLine(nCaretLine + 1, strNewLine);
      SetCaretPosition(nCaretLine + 1, nIndent);
      ScrollToPosition(m_nCaretLine, m_nCaretRealColumn);
    }

    // set new selection start
    SetSelectionStart(m_nCaretLine, m_nCaretRealColumn);

    return TRUE;
  }

  return CFrameWindow::OnKeyDown(nChar, dwFlags);
}

BOOL CScriptEdit::OnChar(int nChar, DWORD dwFlags)
{
  if(m_pLayout->GetFont()->GetCharSize(nChar).cx <= 0) return FALSE;

  // if read only -> no action
  if(m_bReadOnly) return TRUE;
  InlayEvent(E_MODIFIED, 0);

  DeleteSelection();
  InsertLineText(m_nCaretLine, m_nCaretRealColumn, (char)nChar);
  SetCaretPosition(m_nCaretLine, m_nCaretRealColumn + 1);
  m_nCaretColumn = m_nCaretRealColumn;
  ScrollToPosition(m_nCaretLine, m_nCaretRealColumn);
  UpdateRange(m_nCaretLine, m_nCaretRealColumn - 2, m_nCaretLine, m_nCaretRealColumn + 1);
  UpdateScrolls();
  SetSelectionStart(m_nCaretLine, m_nCaretRealColumn);
  return TRUE;
}

void CScriptEdit::DeleteLineText(int nLine, int nStartColumn, int nCount)
{
  ASSERT_VALID(this);
  ASSERT(nLine >= 0); ASSERT(nLine < m_aLines.GetSize());

  if(nCount <= 0) return;
  SLine *pLine = m_aLines[nLine];
  if(nStartColumn < 0) return;
  if(nStartColumn >= pLine->m_strText.GetLength()) return;
  
  if((nStartColumn + nCount) > pLine->m_strText.GetLength())
    nCount = pLine->m_strText.GetLength() - nStartColumn;

  if(nLine == (m_aLines.GetSize() - 1)){
    // if this is the last line -> update it whole
    UpdateLine(nLine);
  }

  pLine->m_strText.Delete(nStartColumn, nCount);
  pLine->m_nWidth = m_pLayout->GetFont()->GetTextSize(pLine->m_strText).cx;

  // move the caret if its on the line and after deleted text
  if(m_nCaretLine == nLine){
    if(m_nCaretRealColumn >= (nStartColumn + nCount)){
      SetCaretPosition(m_nCaretLine, m_nCaretRealColumn - nCount);
      m_nCaretColumn = m_nCaretRealColumn;
      ScrollToPosition(m_nCaretLine, m_nCaretRealColumn);
    }
    else{
      if(m_nCaretRealColumn >= nStartColumn){
        SetCaretPosition(m_nCaretLine, nStartColumn);
        m_nCaretColumn = m_nCaretRealColumn;
        ScrollToPosition(m_nCaretLine, m_nCaretRealColumn);
      }
    }
  }

  // shift all style nodes on the line
  SStyleNode *pNode = pLine->m_pStyles, *pPrevNode = NULL;
  while(pNode != NULL){
    if(pNode->m_nColumn >= nStartColumn){
      // elseway just shift it
      pNode->m_nColumn -= nCount;

      if(pNode->m_nColumn < nStartColumn) pNode->m_nColumn = nStartColumn;
    }

    pPrevNode = pNode;
    pNode = pNode->m_pNext;
  }

  UpdateScrolls();
  ComputeHighlighting(nLine, nStartColumn - 1, nLine, nStartColumn + 1);
}

void CScriptEdit::InsertLineText(int nLine, int nColumn, CString strText)
{
  ASSERT_VALID(this);
  ASSERT(nLine >= 0); ASSERT(nLine < m_aLines.GetSize());
  ASSERT(nColumn >= 0); ASSERT(nColumn <= m_aLines[nLine]->m_strText.GetLength());

  m_aLines[nLine]->m_strText.Insert(nColumn, strText);

  // here we have to move some infos in the token styles
  // cause some letters could have moved its position on the line
  // So -> go through the style infos and if it was after
  // the insert point -> move the position info
  SStyleNode *pNode;
  pNode = m_aLines[nLine]->m_pStyles;
  while(pNode != NULL){
    if(pNode->m_nColumn >= nColumn){
      pNode->m_nColumn += strText.GetLength();
    }
    pNode = pNode->m_pNext;
  }
  m_aLines[nLine]->m_nWidth =
    m_pLayout->GetFont()->GetTextSize(m_aLines[nLine]->m_strText).cx;

  UpdateScrolls();

  // move the caret if needed
  if(m_nCaretLine == nLine){
    if(m_nCaretRealColumn > nColumn){
      SetCaretPosition(m_nCaretLine, m_nCaretRealColumn + strText.GetLength());
      m_nCaretColumn = m_nCaretRealColumn;
    }
  }

  // now recompute the highlighting
  ComputeHighlighting(nLine, nColumn, nLine, nColumn + strText.GetLength());
}

void CScriptEdit::InsertLine(int nLine, CString strLine, BOOL bUpdate)
{
  ASSERT_VALID(this);
  ASSERT(nLine >= 0);
  ASSERT(nLine <= m_aLines.GetSize());

  SLine *pNewLine = new SLine();
  pNewLine->m_strText = strLine;
  pNewLine->m_pStyles = NULL;
  pNewLine->m_nWidth = m_pLayout->GetFont()->GetTextSize(strLine).cx;
  m_aLines.InsertAt(nLine, pNewLine);

  // recompute the visibility
  SetFirstVisible(m_nFirstVisibleLine, m_nFirstVisibleColumn);
  UpdateScrolls();

  // move the caret
  if(m_nCaretLine >= nLine){
    SetCaretPosition(m_nCaretLine + 1, m_nCaretRealColumn);
  }

  if(bUpdate){
    ComputeHighlighting(nLine, 0, nLine, pNewLine->m_strText.GetLength());
    UpdateRect();
  }
}

void CScriptEdit::DeleteLine(int nLine)
{
  ASSERT_VALID(this);
  ASSERT(nLine >= 0); ASSERT(nLine < m_aLines.GetSize());

  DeleteLine(m_aLines[nLine]);
  m_aLines.RemoveAt(nLine);

  // recoimpute visiblity
  SetFirstVisible(m_nFirstVisibleLine, m_nFirstVisibleColumn);
  UpdateScrolls();

  // move the caret
  if(m_nCaretLine == nLine){
    SetCaretPosition(m_nCaretLine, m_nCaretRealColumn);
    m_nCaretColumn = m_nCaretRealColumn;
  }
  if(m_nCaretLine > nLine){
    SetCaretPosition(m_nCaretLine - 1, m_nCaretRealColumn);
  }

  if(nLine < m_aLines.GetSize()){
    ComputeHighlighting(nLine, 0, nLine, m_aLines[nLine]->m_strText.GetLength());
  }
  // yes I know it's slooow
  // but how can I compute here what to redraw
  // all the text below the line has to be moved
  // it's probably whole screen to draw anyway
  UpdateRect();
}

int CScriptEdit::GetIndentation(int nLine)
{
  if(nLine >= m_aLines.GetSize()) nLine = m_aLines.GetSize() - 1;

  SLine *pLine;
  int i;
  while(nLine >= 0){
    pLine = m_aLines[nLine];
    for(i = 0; i < pLine->m_strText.GetLength(); i++){
      if(pLine->m_strText[i] != ' ')
        return i;
    }
    nLine--;
  }
  return 0;
}

void CScriptEdit::SetSelectionStart(int nLine, int nColumn)
{
  // clear old selection
  if(!IsSelectionEmpty())
    UpdateRange(m_nSelectionStartLine, m_nSelectionStartColumn,
      m_nSelectionEndLine, m_nSelectionEndColumn);

  m_nSelectionUserStartLine = nLine;
  m_nSelectionUserStartColumn = nColumn;

  // set new one -> empty
  m_nSelectionStartLine = nLine;
  m_nSelectionStartColumn= nColumn;
  m_nSelectionEndLine = nLine;
  m_nSelectionEndColumn = nColumn;

  // no redraw -> empty selection
}

void CScriptEdit::SetSelectionEdge(int nLine, int nColumn)
{
  BOOL bOldLeft = FALSE;
  if((m_nSelectionStartLine < m_nSelectionUserStartLine) ||
    ((m_nSelectionStartLine == m_nSelectionUserStartLine) &&
    (m_nSelectionStartColumn < m_nSelectionUserStartColumn)))
    bOldLeft = TRUE;

  if((nLine < m_nSelectionUserStartLine) ||
    ((nLine == m_nSelectionUserStartLine) && (nColumn < m_nSelectionUserStartColumn))){
    if(bOldLeft){
      if((nLine < m_nSelectionStartLine) ||
        ((nLine == m_nSelectionStartLine) && (nColumn < m_nSelectionStartColumn))){
        UpdateRange(nLine, nColumn, m_nSelectionStartLine, m_nSelectionStartColumn);
      }
      else{
        UpdateRange(m_nSelectionStartLine, m_nSelectionStartColumn, nLine, nColumn);
      }
    }
    else{
      UpdateRange(nLine, nColumn, m_nSelectionEndLine, m_nSelectionEndColumn);
    }
    m_nSelectionStartLine = nLine; m_nSelectionStartColumn = nColumn;
    m_nSelectionEndLine = m_nSelectionUserStartLine;
    m_nSelectionEndColumn = m_nSelectionUserStartColumn;
  }
  else{
    if(bOldLeft){
      UpdateRange(m_nSelectionStartLine, m_nSelectionStartColumn, nLine, nColumn);
    }
    else{
      if((nLine < m_nSelectionEndLine) ||
        ((nLine == m_nSelectionEndLine) && (nColumn < m_nSelectionEndColumn))){
        UpdateRange(nLine, nColumn, m_nSelectionEndLine, m_nSelectionEndColumn);
      }
      else{
        UpdateRange(m_nSelectionEndLine, m_nSelectionEndColumn, nLine, nColumn);
      }
    }
    m_nSelectionStartLine = m_nSelectionUserStartLine;
    m_nSelectionStartColumn = m_nSelectionUserStartColumn;
    m_nSelectionEndLine = nLine; m_nSelectionEndColumn = nColumn;
  }
}

BOOL CScriptEdit::IsSelectionEmpty()
{
  if(m_nSelectionEndLine < m_nSelectionStartLine) return TRUE;
  if(m_nSelectionEndLine == m_nSelectionStartLine){
    if(m_nSelectionEndColumn <= m_nSelectionStartColumn) return TRUE; // ############################################
  }
  return FALSE;
}

void CScriptEdit::ClearSelection()
{
  // clear old selection
  UpdateRange(m_nSelectionStartLine, m_nSelectionStartColumn,
    m_nSelectionEndLine, m_nSelectionEndColumn);

  m_nSelectionEndLine = m_nSelectionStartLine = 0;
  m_nSelectionEndColumn = m_nSelectionStartColumn = 0;
}

void CScriptEdit::SelectAll()
{
  m_nSelectionStartLine = 0; m_nSelectionStartColumn = 0;
  m_nSelectionEndLine = m_aLines.GetSize() - 1;
  m_nSelectionEndColumn = m_aLines[m_nSelectionEndLine]->m_strText.GetLength();

  UpdateRange(m_nSelectionStartLine, m_nSelectionStartColumn,
    m_nSelectionEndLine, m_nSelectionEndColumn);
}

void CScriptEdit::SetSelection(int nStartLine, int nStartColumn, int nEndLine, int nEndColumn)
{
  m_nSelectionStartLine = nStartLine;
  m_nSelectionStartColumn = nStartColumn;
  m_nSelectionEndLine = nEndLine;
  m_nSelectionEndColumn = nEndColumn;

  UpdateRange(m_nSelectionStartLine, m_nSelectionStartColumn,
    m_nSelectionEndLine, m_nSelectionEndColumn);
}

void CScriptEdit::OnMouseMove(CPoint point)
{
  int nLine, nColumn;
  if(m_bMouseDrag){
    nLine = m_nFirstVisibleLine; nColumn = m_nFirstVisibleColumn;
    if(point.y < m_rcInner.top)
      m_nFirstVisibleLine -= (m_rcInner.top - point.y) / 20 + 1;
    if(point.y > m_rcInner.bottom)
      m_nFirstVisibleLine += (point.y - m_rcInner.bottom) / 20 + 1;
    if(point.x < m_rcInner.left)
      m_nFirstVisibleColumn -= (m_rcInner.left - point.x);
    if(point.x > m_rcInner.right)
      m_nFirstVisibleColumn += (point.x - m_rcInner.right);
    SetFirstVisible(m_nFirstVisibleLine, m_nFirstVisibleColumn);
    UpdateScrolls();
    if((m_nFirstVisibleLine != nLine) || (m_nFirstVisibleColumn != nColumn))
      UpdateRect();

    PositionFromPoint(point, nLine, nColumn);
    SetCaretPosition(nLine, nColumn);
    m_nCaretColumn = m_nCaretRealColumn;
    ScrollToPosition(m_nCaretLine, m_nCaretRealColumn);
    if((m_nCaretLine != m_nLastMouseLine) ||
      (m_nCaretRealColumn != m_nLastMouseColumn)){
      SetSelectionEdge(nLine, nColumn);
      m_nLastMouseLine = m_nCaretLine;
      m_nLastMouseColumn = m_nCaretRealColumn;
    }
  }
  CFrameWindow::OnMouseMove(point);
}

void CScriptEdit::OnLButtonDown(CPoint point)
{
  int nLine, nColumn;
  PositionFromPoint(point, nLine, nColumn);
  SetSelectionStart(nLine, nColumn);
  SetCaretPosition(nLine, nColumn);
  m_nCaretColumn = m_nCaretRealColumn;
  ScrollToPosition(m_nCaretLine, m_nCaretRealColumn);
  m_bMouseDrag = TRUE;
  m_nLastMouseLine = m_nCaretLine;
  m_nLastMouseColumn = m_nCaretRealColumn;
  SetCapture();

  CFrameWindow::OnLButtonDown(point);
}

void CScriptEdit::OnLButtonUp(CPoint point)
{
  if(m_bMouseDrag){
    ReleaseCapture();
    m_bMouseDrag = FALSE;
    m_nLastMouseLine = -1;
    m_nLastMouseColumn = -1;
  }

  CFrameWindow::OnLButtonUp(point);
}

void CScriptEdit::PositionFromPoint(CPoint point, int &nLine, int &nColumn)
{
  if(point.x < m_rcInner.left) point.x = m_rcInner.left;
  if(point.x > m_rcInner.right) point.x = m_rcInner.right;
  if(point.y < m_rcInner.top) point.y = m_rcInner.top;
  if(point.y > m_rcInner.bottom) point.y = m_rcInner.bottom;

  nLine = (point.y - m_rcInner.top) / m_nLineHeight + m_nFirstVisibleLine;
  if(nLine < 0) nLine = 0;
  if(nLine >= m_aLines.GetSize()) nLine = m_aLines.GetSize() - 1;

  int nXPos = m_rcInner.left - m_nFirstVisibleColumn;
  SLine *pLine = m_aLines[nLine];
  CFontObject *pFont = m_pLayout->GetFont();
  for(nColumn = 0; nColumn < pLine->m_strText.GetLength(); nColumn++){
    nXPos += pFont->GetCharSize(pLine->m_strText[nColumn]).cx;
    if(nXPos > point.x) break;
  }
}

void CScriptEdit::OnTimeTick(DWORD dwTime)
{
  m_bTimerTicked = TRUE;
}

void CScriptEdit::DeleteSelection()
{
  int nLine = m_nSelectionStartLine, nColumn = m_nSelectionStartColumn;
  if(IsSelectionEmpty()) return;

  if(m_nSelectionStartLine == m_nSelectionEndLine){
    DeleteLineText(m_nSelectionStartLine, m_nSelectionStartColumn,
      m_nSelectionEndColumn - m_nSelectionStartColumn);
  }
  else{
    CString strTxt;
    DeleteLineText(m_nSelectionStartLine, m_nSelectionStartColumn,
      m_aLines[m_nSelectionStartLine]->m_strText.GetLength() - m_nSelectionStartColumn);
    strTxt = m_aLines[m_nSelectionEndLine]->m_strText.Mid(m_nSelectionEndColumn);
    InsertLineText(m_nSelectionStartLine, m_nSelectionStartColumn,
      strTxt);
    DeleteLineText(m_nSelectionEndLine, 0, m_nSelectionEndColumn);
    int i;
    for(i = m_nSelectionStartLine + 1; i <= m_nSelectionEndLine; i++){
      DeleteLine(m_nSelectionStartLine + 1);
    }
  }
  ClearSelection();

  // Set new caret position
  SetCaretPosition ( nLine, nColumn );
}

void CScriptEdit::Insert(int nLine, int nColumn, CString strText)
{
  ASSERT_VALID(this);
#ifdef _DEBUG
  if(m_aLines.GetSize() > 0){
	ASSERT(nLine >= 0); ASSERT(nLine < m_aLines.GetSize());
	ASSERT(nColumn >= 0); ASSERT(nColumn <= m_aLines[nLine]->m_strText.GetLength());
  }
#endif

  CString strTxt, strAfterInsertPos;
  CString strClearedText;
  int i, j, nPos, nLinePos;
  BOOL bMultiLine = FALSE;
  for(i = 0; i < strText.GetLength(); i++){
    if(strText[i] == 0x0a){
      bMultiLine = TRUE;
      break;
    }
  }
  if(!bMultiLine){
    InsertLineText(nLine, nColumn, strText);
    return;
  }

  char *pText = new char[strText.GetLength() * m_pLayout->m_nTabSize + 1];
  for(j = 0, i = 0; i < strText.GetLength(); i++){
    if((strText[i] != 0x0d) && (strText[i] != 0x09)){
      pText[j++] = strText[i];
    }
    if(strText[i] == 0x09){
      pText[j++] = ' ';
      pText[j++] = ' ';
    }
  }
  pText[j] = 0x0;
  strClearedText = pText;
  delete pText;

  if(m_aLines.GetSize() > 0){
	strAfterInsertPos = m_aLines[nLine]->m_strText.Mid(nColumn);
  }
  else{
    strAfterInsertPos.Empty();
  }
  DeleteLineText(nLine, nColumn, strAfterInsertPos.GetLength());

  nPos = 0; nLinePos = nLine;
  for(i = 0; i < strClearedText.GetLength(); i++){
    if(strClearedText[i] == 0x0a)
      break;
  }
  strTxt = strClearedText.Left(i);
  nPos = i + 1;
  InsertLineText(nLine, nColumn, strTxt);
  nLinePos++;

  for(i = nPos; i < strClearedText.GetLength(); i++){
    if(strClearedText[i] == 0x0a){
      strTxt = strClearedText.Mid(nPos, i - nPos);
      nPos = i + 1;
      InsertLine(nLinePos, strTxt, FALSE);
      nLinePos++;
    }
  }
  strTxt = strClearedText.Mid(nPos);
  strTxt += strAfterInsertPos;
  InsertLine(nLinePos, strTxt, FALSE);

  ComputeHighlighting(nLine, nColumn, nLinePos, strTxt.GetLength());
  UpdateRect ();
}

CString CScriptEdit::GetSelection()
{
  CString strSelection;

  if(IsSelectionEmpty()){
    strSelection.Empty();
    return strSelection;
  }

  if(m_nSelectionStartLine == m_nSelectionEndLine){
    strSelection = m_aLines[m_nSelectionStartLine]->m_strText.Mid(m_nSelectionStartColumn,
      m_nSelectionEndColumn - m_nSelectionStartColumn);
  }
  else{
    strSelection = m_aLines[m_nSelectionStartLine]->m_strText.Mid(m_nSelectionStartColumn);
    strSelection += "\r\n";
    int i;
    for(i = m_nSelectionStartLine + 1; i < m_nSelectionEndLine; i++){
      strSelection += m_aLines[i]->m_strText;
      strSelection += "\r\n";
    }
    strSelection += m_aLines[m_nSelectionEndLine]->m_strText.Left(m_nSelectionEndColumn);
  }
  return strSelection;
}

void CScriptEdit::Copy()
{
  ASSERT_VALID(this);

  if(IsSelectionEmpty()) return;
  CString strSelection = GetSelection();

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

void CScriptEdit::Cut()
{
  Copy();

  // if read only -> no action
  if(m_bReadOnly) return;
  InlayEvent(E_MODIFIED, 0);

  int nLine = m_nSelectionStartLine, nCol = m_nSelectionStartColumn;
  DeleteSelection();
  SetCaretPosition(nLine, nCol);
  m_nCaretColumn = m_nCaretRealColumn;
  ScrollToPosition(m_nCaretLine, m_nCaretRealColumn);
}

void CScriptEdit::Paste()
{
  ASSERT_VALID(this);

  // if read only -> no action
  if(m_bReadOnly) return;
  InlayEvent(E_MODIFIED, 0);

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

  if(!IsSelectionEmpty()){
    DeleteSelection();
  }
  Insert(m_nCaretLine, m_nCaretRealColumn, strText);
}

void CScriptEdit::SaveToFile(CArchiveFile file)
{
  CString strWhole;
  int i;

  strWhole = m_aLines[0]->m_strText;
  for(i = 1; i < m_aLines.GetSize(); i++){
    strWhole += "\r\n";
    strWhole += m_aLines[i]->m_strText;
  }

  file.SeekToBegin();
  file.Write((LPCSTR)strWhole, strWhole.GetLength());
}

void CScriptEdit::LoadFromFile(CArchiveFile file)
{
  DeleteLines();
  // create first clear line with error style
  SLine *pLine = new SLine();
  SStyleNode *pNode = new SStyleNode();
  m_aLines.SetSize(1);
  m_aLines[0] = pLine;
  pLine->m_strText.Empty();
  pLine->m_pStyles = pNode;
  pLine->m_nWidth = 0;
  pNode->m_nColumn = 0;
  pNode->m_dwStyle = m_pLayout->m_dwErrorStyleIndex;
  pNode->m_pNext = NULL;

  m_nCaretLine = 0; m_nCaretColumn = m_nCaretRealColumn = 0;
  m_nSelectionStartLine = m_nSelectionEndLine = 0;
  m_nSelectionStartColumn = m_nSelectionEndColumn = 0;
  m_nFirstVisibleLine = 0; m_nFirstVisibleColumn = 0;

  char *pText;
  int nLength = file.GetLength();
  pText = new char[nLength + 1];
  file.SeekToBegin();
  file.Read(pText, nLength);
  pText[nLength] = 0;

  Insert(0, 0, pText);

  delete pText;
}

void CScriptEdit::OnSize(CSize size)
{
  CFrameWindow::OnSize(size);
}

void CScriptEdit::SetWindowPos(CRect *pRect)
{
  CRect rcBound(pRect);

  // small down the rect of the edit -> scroll bars
  rcBound.right -= m_pLayout->m_VerticalScrollControlLayout.GetWidth();
  rcBound.bottom -= m_pLayout->m_HorizontalScrollControlLayout.GetHeight();

  m_rcInner.left = m_pLayout->m_nLeftMargin;
  m_rcInner.right = rcBound.Width() - m_pLayout->m_nRightMargin;
  m_rcInner.top = m_pLayout->m_nTopMargin;
  m_rcInner.bottom = rcBound.Height() - m_pLayout->m_nBottomMargin;

  CFrameWindow::SetWindowPosition(&rcBound);

  CRect rcScroll;
  // create the vertical scroll bar
  rcScroll.left = rcBound.right; rcScroll.right = rcBound.right + m_pLayout->m_VerticalScrollControlLayout.GetWidth();
  rcScroll.top = rcBound.top; rcScroll.bottom = rcBound.bottom;
  m_VerticalScroll.SetWindowPosition(&rcScroll);

  // create the horizontal scroll bar
  rcScroll.left = rcBound.left; rcScroll.right = rcBound.right;
  rcScroll.top = rcBound.bottom; rcScroll.bottom = rcBound.bottom + m_pLayout->m_HorizontalScrollControlLayout.GetHeight();
  m_HorizontalScroll.SetWindowPosition(&rcScroll);

  SetFirstVisible(m_nFirstVisibleLine, m_nFirstVisibleColumn);
  UpdateScrolls();
}

void CScriptEdit::DeleteSelectedText()
{
  int nLine = m_nSelectionStartLine, nCol = m_nSelectionStartColumn;
  DeleteSelection();
  SetCaretPosition(nLine, nCol);
  m_nCaretColumn = m_nCaretRealColumn;
  ScrollToPosition(m_nCaretLine, m_nCaretRealColumn);
}


// Appends the text to the end
void CScriptEdit::AppendText(CString &strText)
{
  int nLine = m_aLines.GetSize() - 1;
  int nColumn = m_aLines[nLine]->m_strText.GetLength() - 0;

  Insert(nLine, nColumn, strText);
}

// Returns the shole contents
CString CScriptEdit::GetAll()
{
  CString strWhole;
  int i;

  strWhole = m_aLines[0]->m_strText;
  for(i = 1; i < m_aLines.GetSize(); i++){
    strWhole += "\r\n";
    strWhole += m_aLines[i]->m_strText;
  }

  return strWhole;
}

void CScriptEdit::DeleteAll()
{
  // go through all lines and remove them
  {
    int i;
    for(i = 0; i < m_aLines.GetSize(); i++){
      DeleteLine(m_aLines[i]);
    }
    m_aLines.RemoveAll();
  }
  // create first clear line with error style
  SLine *pLine = new SLine();
  SStyleNode *pNode = new SStyleNode();
  m_aLines.SetSize(1);
  m_aLines[0] = pLine;
  pLine->m_strText.Empty();
  pLine->m_pStyles = pNode;
  pLine->m_nWidth = 0;
  pNode->m_nColumn = 0;
  pNode->m_dwStyle = CScEditLexToken::SLTError;
  pNode->m_pNext = NULL;

  // recoimpute visiblity
  SetFirstVisible(m_nFirstVisibleLine, m_nFirstVisibleColumn);
  UpdateScrolls();

  // move the caret
  SetCaretPosition(0, 0);
  m_nCaretColumn = m_nCaretRealColumn;

  UpdateRect();
}

// Sets the whole contents
void CScriptEdit::SetAll(CString strAll)
{
  DeleteAll();

  // now fill in the new contents
  Insert(0, 0, strAll);

  // and that's all
}

void CScriptEdit::SetSelectionLine(int nStartLine, int nEndLine)
{
  SetSelection(nStartLine, 0, nEndLine, GetLine(nEndLine).GetLength());
}