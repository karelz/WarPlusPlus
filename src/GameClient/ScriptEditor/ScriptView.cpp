// ScriptView.cpp: implementation of the CScriptView class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ScriptView.h"

#include "..\resource.h"

#include "ScriptMDIFrame.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

IMPLEMENT_DYNAMIC(CScriptView, CCaptionWindow);

BEGIN_OBSERVER_MAP(CScriptView, CCaptionWindow)
  BEGIN_NOTIFIER(IDC_SCRIPTEDIT)
    EVENT(CScriptViewEdit::E_CONTEXTMENU)
      OnContextMenu(CPoint(MOUSE_X(dwParam), MOUSE_Y(dwParam))); return FALSE;
    EVENT(CScriptViewEdit::E_MODIFIED)
      OnScriptModified(); return FALSE;
  END_NOTIFIER()

  BEGIN_NOTIFIER(IDC_CUT)
    ON_BUTTONCOMMAND(Cut)
  END_NOTIFIER()
  BEGIN_NOTIFIER(IDC_COPY)
    ON_BUTTONCOMMAND(Copy)
  END_NOTIFIER()
  BEGIN_NOTIFIER(IDC_PASTE)
    ON_BUTTONCOMMAND(Paste)
  END_NOTIFIER()
  BEGIN_NOTIFIER(IDC_DELETE)
    ON_BUTTONCOMMAND(DeleteSelection)
  END_NOTIFIER()

  BEGIN_NOTIFIER(IDC_CLOSE)
    ON_BUTTONCOMMAND(Close)
  END_NOTIFIER()

  BEGIN_MOUSE()
    ON_LBUTTONDBLCLK()
  END_MOUSE()
END_OBSERVER_MAP(CScriptView, CCaptionWindow)

CScriptView::CScriptView()
{
  m_pMDIFrame = NULL;
  m_pNextView = NULL;
  m_pPrevView = NULL;

  m_bSaved = FALSE;
  m_bModified = FALSE;
  m_bLocal = TRUE;
  m_bMaximized = FALSE;
  m_bIgnoreReposition = FALSE;
}

CScriptView::~CScriptView()
{
}

#ifdef _DEBUG

void CScriptView::AssertValid() const
{
  CCaptionWindow::AssertValid();

  ASSERT_VALID(m_pCaptionWindowLayout);
  ASSERT_VALID(m_pScriptEditLayout);

  ASSERT_VALID(m_pMDIFrame);
}

void CScriptView::Dump(CDumpContext &dc) const
{
  CCaptionWindow::Dump(dc);
}

#endif

CCaptionWindowLayout *CScriptView::m_pCaptionWindowLayout = NULL;
CScriptEditLayout *CScriptView::m_pScriptEditLayout = NULL;

void CScriptView::InitLayout(CCaptionWindowLayout *pCaptionWidnowLayout, CScriptEditLayout *pScriptEditLayout)
{
  ASSERT_VALID(pCaptionWidnowLayout);
  ASSERT_VALID(pScriptEditLayout);

  m_pCaptionWindowLayout = pCaptionWidnowLayout;
  m_pScriptEditLayout = pScriptEditLayout;
}

void CScriptView::CloseLayout()
{
  m_pScriptEditLayout = NULL;
  m_pCaptionWindowLayout = NULL;
}



BOOL CScriptView::Create(CRect &rcBound, CWindow *pParent, CScriptMDIFrame *pMDIFrame)
{
  ASSERT_VALID(m_pCaptionWindowLayout);
  ASSERT_VALID(m_pScriptEditLayout);
  ASSERT_VALID(pMDIFrame);

  // copy the MDI Frame pointer
  m_pMDIFrame = pMDIFrame;
  
  // we does want doubleclicks
  m_bDoubleClk = TRUE;
  
  // create the window
  if(!CCaptionWindow::Create(rcBound, "", m_pCaptionWindowLayout, pParent, TRUE)) return FALSE;

  // first create the close button
  {
    CDialogWindowLayout *pLayout = CLayouts::m_pDefaults->GetDialogWindowLayout();

    // create the point and button itself there
    CPoint pt(rcBound.Width() - pLayout->m_dwCloseRightMargin,
      pLayout->m_dwCloseTopMargin);
    m_CloseButton.Create(pt, &(pLayout->m_CloseButtonLayout), this, FALSE);

    // connect it
    m_CloseButton.Connect(this, IDC_CLOSE);
  }

  // create the script edit
  CRect rcEdit;
  GetEditRect(&rcEdit);
  if(!m_ScriptEdit.Create(rcEdit, m_pScriptEditLayout, this)) return FALSE;
  m_ScriptEdit.Connect(this, IDC_SCRIPTEDIT);

  // create the context menu
  m_ContextMenu.AddItem(IDS_CUT, IDC_CUT);
  m_ContextMenu.AddItem(IDS_COPY, IDC_COPY);
  m_ContextMenu.AddItem(IDS_PASTE, IDC_PASTE);
  m_ContextMenu.AddSeparator();
  m_ContextMenu.AddItem(IDS_DELETE, IDC_DELETE);

  // set the position
  m_rcNormalPosition = GetWindowPosition();

  m_ScriptEdit.Activate();

  return TRUE;
}

void CScriptView::Delete()
{
  m_ScriptEdit.Delete();
  m_CloseButton.Delete();

  m_ContextMenu.Delete();
  
  m_pMDIFrame = NULL;

  CCaptionWindow::Delete();
}

void CScriptView::GetEditRect(CRect *pEditRect)
{
  CRect rcBound(GetWindowPosition());

  pEditRect->left = 7;
  pEditRect->top = 26;
  pEditRect->right = rcBound.Width() - 7;
  pEditRect->bottom = rcBound.Height() - 7;
}

void CScriptView::OnSize(CSize size)
{
  CRect rcEdit;
  GetEditRect(&rcEdit);
  m_ScriptEdit.SetWindowPos(&rcEdit);

  CDialogWindowLayout *pLayout = CLayouts::m_pDefaults->GetDialogWindowLayout();
  CPoint pt(size.cx - pLayout->m_dwCloseRightMargin,
    pLayout->m_dwCloseTopMargin);
  m_CloseButton.SetWindowPosition(&pt);

  CCaptionWindow::OnSize(size);

  if(!m_bIgnoreReposition){
    m_bMaximized = FALSE;
    m_rcNormalPosition = GetWindowPosition();
  }
}

void CScriptView::OnMove(CPoint point)
{
  if(!m_bIgnoreReposition){
    m_bMaximized = FALSE;
    m_rcNormalPosition = GetWindowPosition();
  }
}

BOOL CScriptView::OnActivate()
{
  // notify our MDI Frame
  m_pMDIFrame->ActiveViewChanged(this);

  return CCaptionWindow::OnActivate();
}

void CScriptView::OnContextMenu(CPoint point)
{
  m_ContextMenu.Create(point, this);
}

void CScriptView::Copy()
{
  m_ScriptEdit.Copy();
}

void CScriptView::Cut()
{
  m_ScriptEdit.Cut();
}

void CScriptView::Paste()
{
  m_ScriptEdit.Paste();
}

void CScriptView::DeleteSelection()
{
  m_ScriptEdit.DeleteSelectedText();
}

void CScriptView::Close()
{
  m_pMDIFrame->DeleteView(this);
}


// Sets the file name
void CScriptView::SetFileName(CString strFileName)
{
  ASSERT_VALID(this);

  m_strFileName = strFileName;

  UpdateCaption();
}

void CScriptView::UpdateCaption()
{
  CString strCaption;

  strCaption = m_strFileName;
  if(m_bLocal)
    strCaption += " L";
  else
    strCaption += " G";

  if(m_bModified)
    strCaption += " *";

  SetCaption(strCaption);
}

void CScriptView::OnScriptModified()
{
  if(!m_bModified){
    m_bModified = TRUE;
    UpdateCaption();
  }
}

void CScriptView::OnLButtonDblClk(CPoint point)
{
  if(InMoveArea(point)){
    if(m_bMaximized){
      // restore us
      m_pMDIFrame->RestoreView(this);
    }
    else{
      // maximize us
      m_pMDIFrame->MaximizeView(this);
    }
  }
}

void CScriptView::SetCaretLine(DWORD dwLine)
{
  m_ScriptEdit.SetCaretPosition(dwLine, 0);
  m_ScriptEdit.ScrollToPosition(dwLine, 0);
  m_ScriptEdit.Activate();
}