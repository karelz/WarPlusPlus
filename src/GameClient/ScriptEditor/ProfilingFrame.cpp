#include "stdafx.h"

#include "ProfilingFrame.h"

#include "..\Resource.h"

IMPLEMENT_DYNAMIC ( CProfilingFrame, CCaptionWindow );

BEGIN_OBSERVER_MAP ( CProfilingFrame, CCaptionWindow )
  BEGIN_NOTIFIER ( IDC_Close )
    ON_BUTTONCOMMAND(OnClose)
  END_NOTIFIER ()

  BEGIN_NOTIFIER ( IDC_ScriptEditor )
    EVENT ( E_UpdateInfo )
      OnUpdateInfo ( (char *) dwParam ); return FALSE;
    EVENT ( E_UpdateCounters )
      OnUpdateCounters ( (__int64 *) dwParam ); return FALSE;
  END_NOTIFIER ()
END_OBSERVER_MAP ( CProfilingFrame, CCaptionWindow )

// A small helper function to nicely print large numbers
CString PrintInt64ByLocale ( __int64 nNumber )
{
  CString strNumber;
  strNumber.Format ( "%I64d", nNumber );

  char pTxt [1024];
  GetNumberFormat ( LOCALE_USER_DEFAULT, LOCALE_NOUSEROVERRIDE, (LPCSTR)strNumber,
    NULL, pTxt, 1023 );
  return CString ( pTxt );
}

// -----------------------------------------------------------
// Constructor & destructor
// -----------------------------------------------------------

// Constructor
CProfilingFrame::CProfilingFrame ()
{
  m_pUpdateSender = NULL;
}

// Destructor
CProfilingFrame::~CProfilingFrame ()
{
  ASSERT ( m_pUpdateSender == NULL );
}


// -----------------------------------------------------------
// Debug functions
// -----------------------------------------------------------

#ifdef _DEBUG

// Asserts object validity
void CProfilingFrame::AssertValid () const
{
  // Call the base class
  CCaptionWindow::AssertValid ();

  ASSERT ( m_pUpdateSender != NULL );
}

// Dumps object's data
void CProfilingFrame::Dump ( CDumpContext &dc ) const
{
  // Call the base class
  CCaptionWindow::Dump ( dc );
}

#endif


// -----------------------------------------------------------
// Creation
// -----------------------------------------------------------

// Creates the window
void CProfilingFrame::Create ( CRect &rcBound, CWindow *pParent, CCaptionWindowLayout *pLayout,
                               CNotifier *pUpdateSender )
{
  m_pUpdateSender = pUpdateSender;

  // Create the caption window
  CString strCaption;
  strCaption.LoadString ( IDS_PROFILING_CAPTION );
  CCaptionWindow::Create ( rcBound, strCaption, pLayout, pParent, TRUE );

  // first create the close button
  {
    CDialogWindowLayout *pLayout = CLayouts::m_pDefaults->GetDialogWindowLayout();

    // create the point and button itself there
    CPoint pt(rcBound.Width() - pLayout->m_dwCloseRightMargin,
      pLayout->m_dwCloseTopMargin);
    m_wndCloseButton.Create(pt, &(pLayout->m_CloseButtonLayout), this, FALSE);

    // connect it
    m_wndCloseButton.Connect(this, IDC_Close);
  }

  // Create the list control
  {
    CRect rcList;
    GetListRect ( &rcList );
    m_wndList.Create ( rcList, NULL, this, 3 );
    m_wndList.SetColumnWidth ( 0, 450 );
    m_wndList.SetColumnWidth ( 1, 60 );
    m_wndList.SetColumnWidth ( 2, 60 );
  }

  // Connect us to the update sender
  m_pUpdateSender->Connect ( this , IDC_ScriptEditor );
}

// Deletes the window
void CProfilingFrame::Delete ()
{
  // Disconnect us
  if ( m_pUpdateSender != NULL )
  {
    m_pUpdateSender->Disconnect ( this );
    m_pUpdateSender = NULL;
  }

  // Delete the list control
  m_wndList.Delete ();

  // Delete the close button
  m_wndCloseButton.Delete ();

  // Delete the caption window
  CCaptionWindow::Delete ();

  m_strProfiling.Empty ();
}

// Sets the profiling information
void CProfilingFrame::SetProfilingInformations ( CString strInfo )
{
  // Copy the info
  m_strProfiling = strInfo;

  // Update our list control
  FillList ();
}

// Sets instruction counters info
void CProfilingFrame::SetInstructionCounters ( __int64 nLocal, __int64 nGlobal )
{
  CString strTitle;

  strTitle.LoadString ( IDS_PROFILING_CAPTION );
  strTitle += " (" + PrintInt64ByLocale ( nLocal ) + "/" + PrintInt64ByLocale ( nGlobal ) + ")";
  SetCaption ( strTitle );
}


// -----------------------------------------------------------
// Event reactions
// -----------------------------------------------------------

// The close button was pressed
void CProfilingFrame::OnClose ()
{
  InlayEvent ( E_StopUpdating, 0, m_pUpdateSender );
  HideWindow ();
}


// The window has changed the size
void CProfilingFrame::OnSize ( CSize size )
{
  CRect rcList;
  GetListRect ( &rcList );
  m_wndList.SetWindowPos ( &rcList );

  {
    CDialogWindowLayout *pLayout = CLayouts::m_pDefaults->GetDialogWindowLayout();
    CPoint pt(size.cx - pLayout->m_dwCloseRightMargin,
      pLayout->m_dwCloseTopMargin);
    m_wndCloseButton.SetWindowPosition(&pt);
  }

  CCaptionWindow::OnSize ( size );
}

// Reaction on the update info event
void CProfilingFrame::OnUpdateInfo ( char *pInfo )
{
  // Set it
  SetProfilingInformations ( pInfo );

  // Delete the info string
  delete pInfo;
}

void CProfilingFrame::OnUpdateCounters ( __int64 * pCounters )
{
  SetInstructionCounters ( pCounters [ 0 ], pCounters [ 1 ] );

  delete pCounters;
}


// -----------------------------------------------------------
// Helper functions
// -----------------------------------------------------------

// Returns the rect for the list control
void CProfilingFrame::GetListRect ( CRect * pRect )
{
  CRect rcBound(GetWindowPosition());

  pRect->left = 7;
  pRect->top = 26;
  pRect->right = rcBound.Width() - 7;
  pRect->bottom = rcBound.Height() - 7;
}

// Fills the list with the profiling data
void CProfilingFrame::FillList ()
{
  m_wndList.RemoveAll ();

  CString strLine;
  int nPos, nLinePos = 0;
  int nItem = 0;
  while ( true )
  {
    nPos = m_strProfiling.Find ( '|', nLinePos );
    if ( nPos == -1 )
    {
      InsertProfileLine ( m_strProfiling.Mid ( nLinePos ), nItem );
      break;
    }
    InsertProfileLine ( m_strProfiling.Mid ( nLinePos, nPos - nLinePos ), nItem );
    nPos++;
    nLinePos = nPos;
  }
}

// Inserts one profiling line to the list
void CProfilingFrame::InsertProfileLine ( CString &strLine, int &nItem )
{
  CString strFunction, strShort, strLong;
  int nPos, nStartPos = 0;

  nPos = strLine.Find ( '/', nStartPos );
  if ( nPos == -1 ) return;
  strFunction = strLine.Mid ( nStartPos, nPos - nStartPos );
  nStartPos = nPos + 1;
  nPos = strLine.Find ( '/', nStartPos );
  if ( nPos == -1 ) return;
  strShort = strLine.Mid ( nStartPos, nPos - nStartPos );
  nStartPos = nPos + 1;
  nPos = strLine.Find ( '/', nStartPos );
  if ( nPos == -1 )
    strLong = strLine.Mid ( nStartPos );
  else
    strLong = strLine.Mid ( nStartPos, nPos - nStartPos );

  int nI = m_wndList.AddItem ( strFunction );
  m_wndList.SetItemText ( nI, 0, strShort );
  m_wndList.SetItemText ( nI, 1, strLong );
}
