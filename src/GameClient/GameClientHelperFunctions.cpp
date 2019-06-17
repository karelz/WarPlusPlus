#include "stdafx.h"

#include "GameClientHelperFunctions.h"

// Reads windows position and show state from registry, from given section
// If there is no specified in the registry, uses given defaults
// Returns true, if the window should be shown
void ReadWindowPositionFromRegistry ( CString strSection, CWindow * pWnd, CRect * pRectDefault, bool bShowDefault )
{
  if ( pWnd->IsDeleted () ) return;
  CWinApp * pApp = AfxGetApp ();

  CRect rc;
  rc.left = pApp->GetProfileInt ( strSection, "Position_Left", pRectDefault->left );
  rc.top = pApp->GetProfileInt ( strSection, "Position_Top", pRectDefault->top );
  rc.right = pApp->GetProfileInt ( strSection, "Position_Right", pRectDefault->right );
  rc.bottom = pApp->GetProfileInt ( strSection, "Position_Bottom", pRectDefault->bottom );

  pWnd->SetWindowPosition ( &rc );

  bool bShow;
  bShow = pApp->GetProfileInt ( strSection, "Visible", bShowDefault ? 1 : 0 ) == 1;

  if ( bShow )
  {
    pWnd->ShowWindow ();
  }
  else
  {
    pWnd->HideWindow ();
  }
}

// Writes windows position and show state to registry to given section
void WriteWindowPositionToRegistry ( CString strSection, CWindow * pWnd )
{
  if ( pWnd->IsDeleted () ) return;
  CWinApp * pApp = AfxGetApp ();
  CRect rc = pWnd->GetWindowPosition ();

  pApp->WriteProfileInt ( strSection, "Position_Left", rc.left );
  pApp->WriteProfileInt ( strSection, "Position_Top", rc.top );
  pApp->WriteProfileInt ( strSection, "Position_Right", rc.right );
  pApp->WriteProfileInt ( strSection, "Position_Bottom", rc.bottom );
  pApp->WriteProfileInt ( strSection, "Visible", pWnd->IsVisible () ? 1 : 0 );
}