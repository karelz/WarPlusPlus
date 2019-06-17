// ScriptMDIFrame.cpp: implementation of the CScriptMDIFrame class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ScriptMDIFrame.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

#include "ScriptEditor.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

IMPLEMENT_DYNAMIC(CScriptMDIFrame, CWindow);

BEGIN_OBSERVER_MAP(CScriptMDIFrame, CWindow)
  BEGIN_KEYBOARD()
    ON_KEYACCELERATOR()
  END_KEYBOARD()
END_OBSERVER_MAP(CScriptMDIFrame, CWindow)

CScriptMDIFrame::CScriptMDIFrame()
{
  m_pActiveView = NULL;
}

CScriptMDIFrame::~CScriptMDIFrame()
{
}

#ifdef _DEBUG

void CScriptMDIFrame::AssertValid() const
{
  CWindow::AssertValid();

  ASSERT(m_pScriptEditor != NULL);
}

void CScriptMDIFrame::Dump(CDumpContext &dc) const
{
  CWindow::Dump(dc);
}

#endif

BOOL CScriptMDIFrame::Create(CRect &rcBound, CWindow *pParent, CScriptEditor *pScriptEditor)
{
  ASSERT_VALID(pParent);
  ASSERT(pScriptEditor != NULL);

  m_pActiveView = NULL;
  m_pScriptEditor = pScriptEditor;

  // yes we are transparent, absolutely
  m_bTransparent = TRUE;

  // register accelerators for the window switching
  ASSERT_VALID(g_pKeyAccelerators);
  g_pKeyAccelerators->Connect(this, VK_TAB, CKeyboard::CtrlDown);
  g_pKeyAccelerators->Connect(this, VK_F4, CKeyboard::CtrlDown);

  return CWindow::Create(&rcBound, pParent);
}

void CScriptMDIFrame::Delete()
{
  if(g_pKeyAccelerators){
    g_pKeyAccelerators->Disconnect(this);
  }

  DeleteAllViews();

  m_pScriptEditor = NULL;
}

void CScriptMDIFrame::DeleteAllViews()
{
  if(m_pActiveView == NULL) return;

  CScriptView *pView = m_pActiveView, *pDeleteView;
  while(pView->m_pNextView != m_pActiveView){
    // go to next one
    pDeleteView = pView;
    pView = pView->m_pNextView;

    // delete it
    pDeleteView->Delete();
    delete pDeleteView;
  }
  // here we have only one view remaining -> the pView
  // so delete it
  pView->Delete();
  delete pView;

  // clear our pointer
  m_pActiveView = NULL;
}

void CScriptMDIFrame::Draw(CDDrawSurface *pSurface, CRect *pRect)
{
  // do nothing, this window is totaly transparent
}

CScriptView * CScriptMDIFrame::CreateView()
{
  ASSERT_VALID(this);
  static int nNewViewPos = 0;

  CScriptView *pView;

  // the new view - set its new position
  CRect rcView, rcWndRect;
  rcWndRect = GetWindowPosition();
  rcView.left = rcView.top = 0;
  rcView.right = rcWndRect.Width() - 100;
  rcView.bottom = rcWndRect.Height() - 100;
  rcView.OffsetRect(nNewViewPos * 20, nNewViewPos * 20);
  nNewViewPos = (nNewViewPos + 1) % 5;

  // create it
  pView = new CScriptView();
  pView->Create(rcView, this, this);

  BOOL bMaximize = TRUE;

  // remember the state of the active view
  if(m_pActiveView != NULL){
    bMaximize = m_pActiveView->GetMaximized();
  }

  // add it to the list
  // so add it to our active position
  if(m_pActiveView == NULL){
    // create the cycle
    pView->m_pNextView = pView;
    pView->m_pPrevView = pView;
  }
  else{
    // add it to the cycle
    pView->m_pNextView = m_pActiveView;
    pView->m_pPrevView = m_pActiveView->m_pPrevView;
    m_pActiveView->m_pPrevView->m_pNextView = pView;
    m_pActiveView->m_pPrevView = pView;
  }
  m_pActiveView = pView;

  // maximize the view if we're to do so
  if(bMaximize)
    MaximizeView(pView);

  return pView;
}

void CScriptMDIFrame::DeleteView(CScriptView *pDeleteView)
{
  ASSERT_VALID(this);
  ASSERT_VALID(pDeleteView);

  // first check if the view was saved
  if(pDeleteView->GetModified()){
    // no -> say it to the script editor - if it returns it means that we can close the view
    if(!m_pScriptEditor->ClosingUnsavedView(pDeleteView)) return;
  }

  // remove it from list
  pDeleteView->m_pPrevView->m_pNextView = pDeleteView->m_pNextView;
  pDeleteView->m_pNextView->m_pPrevView = pDeleteView->m_pPrevView;

  // update the activeview pointer
  if(m_pActiveView == pDeleteView){
    if(pDeleteView->m_pNextView == pDeleteView){
      m_pActiveView = NULL;
    }
    else{
      m_pActiveView = pDeleteView->m_pNextView;
      m_pActiveView->Activate();
    }
  }

  // delete the view
  pDeleteView->Delete();
  delete pDeleteView;
}

BOOL CScriptMDIFrame::OnKeyAccelerator(UINT nChar, DWORD dwFlags)
{
  switch(nChar){
  case VK_TAB:
    if(!(dwFlags & CKeyboard::CtrlDown)) break;
    if(m_pActiveView != NULL){
      // the view itself will notify us of the active view change
      m_pActiveView->m_pNextView->Activate();
    }
    return TRUE;
  case VK_F4:
    if(!(dwFlags & CKeyboard::CtrlDown)) break;
    if(m_pActiveView != NULL){
      DeleteView(m_pActiveView);
    }
    return TRUE;
  }

  return FALSE;
}

void CScriptMDIFrame::ActiveViewChanged(CScriptView *pView)
{
  // try to find the view
  CScriptView *pV = m_pActiveView;
  if(pV == NULL) return;
  do{
    if(pV == pView){
      m_pActiveView = pView;
      return;
    }
    pV = pV->m_pNextView;
  }while(pV != m_pActiveView);
}


BOOL CScriptMDIFrame::CloseAllViews()
{
  // go throuh all views
  CScriptView *pView;

  pView = m_pActiveView;
  if(pView != NULL){
    do{
      
      // if the view is saved -> everything is OK
      if(pView->GetModified()){
        // if it's modified -> try to save it
        
        if(!m_pScriptEditor->ClosingUnsavedView(pView)){
          // if the user cancel the action -> cancel all closing
          return FALSE;
        }
        
        // mark it as saved
        pView->SetModified(FALSE);
      }
      
      // OK -> go to the next view
      pView = pView->m_pNextView;
    }while(pView != m_pActiveView);
  }

  // now close all views
  DeleteAllViews();

  return TRUE;
}

BOOL CScriptMDIFrame::SaveAllViews()
{
  // go through all views
  CScriptView *pView;

  pView = m_pActiveView;
  if(pView != NULL){
    do{
      if(pView->GetModified()){
        if(!m_pScriptEditor->SaveRemoteScript(pView))
          return FALSE;
      }

      pView = pView->m_pNextView;
    }while(pView != m_pActiveView);
  }

  return TRUE;
}

void CScriptMDIFrame::BackupViews(CString &strBackupDirectory)
{
  // go through all views
  CScriptView *pView;

  pView = m_pActiveView;
  if(pView != NULL){
    do{
      
      // if the view is saved -> everything is OK
      if(pView->GetModified()){
        // if it's modified -> back it up
        
        CString strFile;
        CString strFilePath = strBackupDirectory + "\\" + pView->GetFileName();
        if(!pView->GetSaved())
          strFilePath += ".3s";

        strFile = pView->GetScriptEditCtrl()->GetAll();

        // create the file
        HANDLE hFile = CreateFile(strFilePath, GENERIC_WRITE | GENERIC_READ, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
        if(hFile != INVALID_HANDLE_VALUE){ // if some error occured -> it can't be repaired - better to be quiet
          DWORD dwWrite;
          // write it to file
          WriteFile(hFile, (LPCSTR)strFile, strFile.GetLength(), &dwWrite, NULL);
          CloseHandle(hFile);
        }
        // mark it as saved
        pView->SetModified(FALSE);
      }
      
      // OK -> go to the next view
      pView = pView->m_pNextView;
    }while(pView != m_pActiveView);
  }
}

void CScriptMDIFrame::MaximizeView(CScriptView *pView)
{
  ASSERT_VALID(pView);

  pView->m_bIgnoreReposition = TRUE;
  // set its position to fill our whole window
  CRect rcWindow(0, 0, GetWindowPosition().Width(), GetWindowPosition().Height());
  pView->SetWindowPosition(&rcWindow);
  pView->m_bIgnoreReposition = FALSE;

  pView->SetMaximized(TRUE);
}

void CScriptMDIFrame::RestoreView(CScriptView *pView)
{
  ASSERT_VALID(pView);

  pView->m_bIgnoreReposition = TRUE;
  pView->SetWindowPosition(&(pView->GetNormalPosition()));
  pView->m_bIgnoreReposition = FALSE;
  pView->SetMaximized(FALSE);
}

void CScriptMDIFrame::OnSize(CSize size)
{
  // go through all views and if they're maximized -> resize them
  CScriptView *pView = m_pActiveView;

  if(pView == NULL) return;

  do{
    if(pView->GetMaximized()){
      MaximizeView(pView);
    }

    pView = pView->m_pNextView;
  }while(pView != m_pActiveView);
}

CScriptView * CScriptMDIFrame::FindView(CString strFileName, BOOL bLocal)
{
  // go through all views and if they're maximized -> resize them
  CScriptView *pView = m_pActiveView;

  if(pView == NULL) return NULL;

  do{
    if(!pView->GetFileName().CompareNoCase(strFileName)){
      if(bLocal && pView->GetLocal()) return pView;
      if(!bLocal && !pView->GetLocal()) return pView;
    }

    pView = pView->m_pNextView;
  }while(pView != m_pActiveView);

  return NULL;
}

BOOL CScriptMDIFrame::OnActivate ()
{
  return TRUE;
}
