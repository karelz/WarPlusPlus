// AnimationEditorDoc.cpp : implementation of the CAnimationEditorDoc class
//

#include "stdafx.h"
#include "AnimationEditor.h"

#include "AnimationEditorDoc.h"
#include "AnimationEditorView.h"

#include "CropFrameDlg.h"

#include "MainFrm.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CAnimationEditorDoc

IMPLEMENT_DYNCREATE(CAnimationEditorDoc, CDocument)

BEGIN_MESSAGE_MAP(CAnimationEditorDoc, CDocument)
	//{{AFX_MSG_MAP(CAnimationEditorDoc)
	ON_COMMAND(IDOK, OnOk)
	ON_COMMAND(ID_COLORING_HUELESS, OnColoringHueLess)
	ON_COMMAND(ID_COLORING_HUEMORE, OnColoringHueMore)
	ON_UPDATE_COMMAND_UI(ID_COLORING_HUEMORE, OnUpdateColoringHueMore)
	ON_UPDATE_COMMAND_UI(ID_COLORING_HUELESS, OnUpdateColoringHueLess)
	ON_COMMAND(ID_COLORING_SATMORE, OnColoringSatMore)
	ON_COMMAND(ID_COLORING_SATLESS, OnColoringSatLess)
	ON_UPDATE_COMMAND_UI(ID_COLORING_SATMORE, OnUpdateColoringSatMore)
	ON_UPDATE_COMMAND_UI(ID_COLORING_SATLESS, OnUpdateColoringSatLess)
	ON_COMMAND(ID_COLORING_VALMORE, OnColoringValMore)
	ON_COMMAND(ID_COLORING_VALLESS, OnColoringValLess)
	ON_UPDATE_COMMAND_UI(ID_COLORING_VALMORE, OnUpdateColoringValMore)
	ON_UPDATE_COMMAND_UI(ID_COLORING_VALLESS, OnUpdateColoringValLess)
	ON_COMMAND(ID_COLORING_SOURCECOLOR, OnColoringSourceColor)
	ON_UPDATE_COMMAND_UI(ID_ANIMATION_LOOP, OnUpdateAnimationLoop)
	ON_COMMAND(ID_ANIMATION_LOOP, OnAnimationLoop)
	ON_UPDATE_COMMAND_UI(ID_ANIMATION_CROP, OnUpdateAnimationCrop)
	ON_COMMAND(ID_ANIMATION_CROP, OnAnimationCrop)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CAnimationEditorDoc construction/destruction

CAnimationEditorDoc::CAnimationEditorDoc()
{
}

CAnimationEditorDoc::~CAnimationEditorDoc()
{
}

BOOL CAnimationEditorDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;

  m_cAnimation.Delete ();
  // Just create the animation
  m_cAnimation.Create ();
  // Update our views
  UpdateAllViews ( NULL, Update_NewAnimation, NULL );

  UpdateColoring ( false );
  UpdateDelays ( false );

	return TRUE;
}

void CAnimationEditorDoc::DeleteContents() 
{
  // Delete the animation
  m_cAnimation.Delete ();
	
	CDocument::DeleteContents();
}

/////////////////////////////////////////////////////////////////////////////
// CAnimationEditorDoc diagnostics

#ifdef _DEBUG
void CAnimationEditorDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void CAnimationEditorDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CAnimationEditorDoc commands


BOOL CAnimationEditorDoc::OnOpenDocument(LPCTSTR lpszPathName) 
{
	if (!CDocument::OnOpenDocument(lpszPathName))
		return FALSE;

  try{
    // Create the file from the given path
    CArchiveFile cFile = CDataArchive::GetRootArchive ()->CreateFile ( lpszPathName, CArchiveFile::modeReadWrite | CArchiveFile::shareDenyNone );

    // Create the animation
    m_cAnimation.Create ( cFile );

    // Close the file
    cFile.Close ();

    // Update all views
    UpdateAllViews ( NULL, Update_NewAnimation, NULL );
    UpdateColoring ( false );
    UpdateDelays ( false );
  }
  catch ( CException * e )
  {
    e->ReportError ();
    e->Delete ();

    OnNewDocument ();
    return FALSE;
  }

	return TRUE;
}

BOOL CAnimationEditorDoc::OnSaveDocument(LPCTSTR lpszPathName) 
{
  try
  {
    // Create the file from given path
    CArchiveFile cFile = CDataArchive::GetRootArchive ()->CreateFile ( lpszPathName,
      CArchiveFile::modeReadWrite | CArchiveFile::modeCreate | CArchiveFile::modeUncompressed );

    // Save the animation
    m_cAnimation.Save ( cFile );

    // Close the file
    cFile.Close ();

    // Reset the modified flag
    SetModifiedFlag ( FALSE );
  }
  catch ( CException * e )
  {
    e->ReportError ();
    e->Delete ();
    return FALSE;
  }

  return TRUE;
}

void CAnimationEditorDoc::OnOk() 
{
	UpdateColoring ( true );
  UpdateDelays ( true );
  UpdateCurrentFrame ( true );

  if ( m_cAnimation.IsModified () ) SetModifiedFlag ();

  POSITION pos = GetFirstViewPosition ();
  CView * pView = GetNextView ( pos );
  pView->SetFocus ();
}

/////////////////////////////////////////////////////////////////////////////
// Helper functions

// Updates coloring informations
// If the param is false , the coloring info is written to controls
// elseway it's readfrom controls to memory structures
void CAnimationEditorDoc::UpdateColoring ( bool bSave )
{
  if ( bSave )
  {
    SColoring * pColoring = m_cAnimation.GetColoring ();
    pColoring->m_dbHTolerance = g_pMainFrame->GetHueToleranceFromControl ();
    pColoring->m_dbSTolerance = g_pMainFrame->GetSatToleranceFromControl ();
    pColoring->m_dbVTolerance = g_pMainFrame->GetValToleranceFromControl ();
    pColoring->m_dwSourceColor = g_pMainFrame->GetSourceColorFromControl ();
    m_cAnimation.ReleaseColoring ();
    UpdateAllViews ( NULL, Update_NoSpecial, NULL );
  }
  else
  {
    SColoring * pColoring = m_cAnimation.GetColoring ();
    g_pMainFrame->SetHueToleranceForControl ( pColoring->m_dbHTolerance );
    g_pMainFrame->SetSatToleranceForControl ( pColoring->m_dbSTolerance );
    g_pMainFrame->SetValToleranceForControl ( pColoring->m_dbVTolerance );
    g_pMainFrame->SetSourceColorForControl ( pColoring->m_dwSourceColor );
  }
}

// Updates current frame informations
// If the param is false, the current frame is written to controls
// elseway it's read from the controls to memory structures
void CAnimationEditorDoc::UpdateCurrentFrame ( bool bSave )
{
  // Get the view
  POSITION pos = GetFirstViewPosition ();
  CAnimationEditorView * pView = (CAnimationEditorView *) GetNextView ( pos );

  if ( bSave )
  {
    // Read it from the controls
    pView->SetCurrentFrame ( g_pMainFrame->GetCurrentFrameFromControl () );
  }
  else
  {
    // Set it to controls
    g_pMainFrame->SetCurrentFrameForControl ( pView->GetCurrentFrame () );
  }
}

// Updates delays informations
void CAnimationEditorDoc::UpdateDelays ( bool bSave )
{
  // Get the view
  POSITION pos = GetFirstViewPosition ();
  CAnimationEditorView * pView = (CAnimationEditorView *) GetNextView ( pos );

  if ( bSave )
  {
    // Read it from the controls
    int nDelay = g_pMainFrame->GetDefaultAnimationDelayFromControl ();
    if ( nDelay < 0 ) nDelay = 0;
    m_cAnimation.SetDefaultFrameDelay ( (DWORD)nDelay );

    nDelay = g_pMainFrame->GetFrameDelayFromControl ();
    if ( nDelay < 0 ) nDelay = 0;
    if ( m_cAnimation.GetFrameCount () > 0 )
      m_cAnimation.SetFrameDelay ( pView->GetCurrentFrame (), (DWORD)nDelay );
  }
  else
  {
    // Set it to controls
    g_pMainFrame->SetDefaultAnimationDelayForControl ( m_cAnimation.GetDefaultFrameDelay () );
    if ( m_cAnimation.GetFrameCount () > 0 )
      g_pMainFrame->SetFrameDelayForControl ( m_cAnimation.GetFrame ( pView->GetCurrentFrame () )->GetFrameDelay () );
    else
      g_pMainFrame->SetFrameDelayForControl ( 0 );
  }
}

void CAnimationEditorDoc::OnColoringHueMore() 
{
  SColoring * pColoring = m_cAnimation.GetColoring ();
  if ( pColoring->m_dbHTolerance < 360 ) pColoring->m_dbHTolerance += 1; else return;
  m_cAnimation.ReleaseColoring ();
  UpdateAllViews ( NULL, Update_NoSpecial, NULL );
  UpdateColoring ( false );
  SetModifiedFlag ();
}

void CAnimationEditorDoc::OnColoringHueLess() 
{
  SColoring * pColoring = m_cAnimation.GetColoring ();
  if ( pColoring->m_dbHTolerance > 0 ) pColoring->m_dbHTolerance -= 1; else return;
  m_cAnimation.ReleaseColoring ();
  UpdateAllViews ( NULL, Update_NoSpecial, NULL );
  UpdateColoring ( false );
  SetModifiedFlag ();
}

void CAnimationEditorDoc::OnUpdateColoringHueMore(CCmdUI* pCmdUI) 
{ pCmdUI->Enable ( m_cAnimation.GetColoring ()->m_dbHTolerance < 360 ); }

void CAnimationEditorDoc::OnUpdateColoringHueLess(CCmdUI* pCmdUI) 
{ pCmdUI->Enable ( m_cAnimation.GetColoring ()->m_dbHTolerance > 0 ); }


void CAnimationEditorDoc::OnColoringSatMore() 
{
  SColoring * pColoring = m_cAnimation.GetColoring ();
  if ( pColoring->m_dbSTolerance < 1 ) pColoring->m_dbSTolerance += 0.01; else return;
  m_cAnimation.ReleaseColoring ();
  UpdateAllViews ( NULL, Update_NoSpecial, NULL );
  UpdateColoring ( false );
  SetModifiedFlag ();
}

void CAnimationEditorDoc::OnColoringSatLess() 
{
  SColoring * pColoring = m_cAnimation.GetColoring ();
  if ( pColoring->m_dbSTolerance > 0 ) pColoring->m_dbSTolerance -= 0.01; else return;
  m_cAnimation.ReleaseColoring ();
  UpdateAllViews ( NULL, Update_NoSpecial, NULL );
  UpdateColoring ( false );
  SetModifiedFlag ();
}

void CAnimationEditorDoc::OnUpdateColoringSatMore(CCmdUI* pCmdUI) 
{ pCmdUI->Enable ( m_cAnimation.GetColoring ()->m_dbSTolerance < 1 ); }

void CAnimationEditorDoc::OnUpdateColoringSatLess(CCmdUI* pCmdUI) 
{ pCmdUI->Enable ( m_cAnimation.GetColoring ()->m_dbSTolerance > 0 ); }

void CAnimationEditorDoc::OnColoringValMore() 
{
  SColoring * pColoring = m_cAnimation.GetColoring ();
  if ( pColoring->m_dbVTolerance < 1 ) pColoring->m_dbVTolerance += 0.01; else return;
  m_cAnimation.ReleaseColoring ();
  UpdateAllViews ( NULL, Update_NoSpecial, NULL );
  UpdateColoring ( false );
  SetModifiedFlag ();
}

void CAnimationEditorDoc::OnColoringValLess() 
{
  SColoring * pColoring = m_cAnimation.GetColoring ();
  if ( pColoring->m_dbVTolerance > 0 ) pColoring->m_dbVTolerance -= 0.01; else return;
  m_cAnimation.ReleaseColoring ();
  UpdateAllViews ( NULL, Update_NoSpecial, NULL );
  UpdateColoring ( false );
  SetModifiedFlag ();
}

void CAnimationEditorDoc::OnUpdateColoringValMore(CCmdUI* pCmdUI) 
{ pCmdUI->Enable ( m_cAnimation.GetColoring ()->m_dbVTolerance < 1 ); }

void CAnimationEditorDoc::OnUpdateColoringValLess(CCmdUI* pCmdUI) 
{ pCmdUI->Enable ( m_cAnimation.GetColoring ()->m_dbVTolerance > 0 ); }

void CAnimationEditorDoc::OnColoringSourceColor() 
{
  UpdateColoring ( true );	
  if ( m_cAnimation.IsModified () ) SetModifiedFlag ();
}

void CAnimationEditorDoc::OnUpdateAnimationLoop(CCmdUI* pCmdUI) 
{ pCmdUI->Enable ( TRUE ); pCmdUI->SetCheck ( m_cAnimation.GetLoop () ? 1 : 0 ); }

void CAnimationEditorDoc::OnAnimationLoop() 
{ m_cAnimation.SetLoop ( !m_cAnimation.GetLoop () ); SetModifiedFlag (); }

void CAnimationEditorDoc::OnUpdateAnimationCrop(CCmdUI* pCmdUI) 
{ pCmdUI->Enable ( m_cAnimation.GetFrameCount () > 0 ); }

void CAnimationEditorDoc::OnAnimationCrop() 
{
  if ( m_cAnimation.GetFrameCount () == 0 ) return;
  // Go through all frames and crop them	
  CCropFrameDlg dlg;
  POSITION pos = GetFirstViewPosition ();
  CAnimationEditorView * pView = (CAnimationEditorView *) GetNextView ( pos );
  
  dlg.m_dwTolerance = pView->GetLastCropTolerance ();

  if ( dlg.DoModal () == IDOK )
  {
    pView->SetLastCropTolerance ( dlg.m_dwTolerance );
    DWORD dwFrame;
    for ( dwFrame = 0; dwFrame < m_cAnimation.GetFrameCount (); dwFrame ++ )
    {
      m_cAnimation.CropFrame ( dwFrame, (BYTE)dlg.m_dwTolerance );
    }
    SetModifiedFlag ();
  }

  UpdateAllViews ( NULL, 0 );
}
