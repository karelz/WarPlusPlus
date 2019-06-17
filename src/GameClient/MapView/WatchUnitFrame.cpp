/***********************************************************
 * 
 *     Project: Strategy game
 *        Part: GameClient - MapView
 *      Author: Vit Karas
 * 
 * Description: Frame window for watched unit
 * 
 ***********************************************************/

#include "stdafx.h"

#include "WatchUnitFrame.h"

#include "..\GameClientGlobal.h"
#include "..\DataObjects\CCivilization.h"

#define WATCHUNITFRAME_WIDTH 480
#define WATCHUNITFRAME_HEIGHT 240

#define WATCHUNITFRAME_VIEWPORT_LEFT 10
#define WATCHUNITFRAME_VIEWPORT_TOP 30
#define WATCHUNITFRAME_VIEWPORT_WIDTH 200
#define WATCHUNITFRAME_VIEWPORT_HEIGHT 200

#define WATCHUNITFRAME_VSPACE 10

#define WATCHUNITFRAME_FULLINFO_TOP 30
#define WATCHUNITFRAME_FULLINFO_WIDTH 250
#define WATCHUNITFRAME_FULLINFO_HEIGHT 200

// RTCI for our classes
IMPLEMENT_DYNAMIC ( CWatchUnitFrame, CCaptionWindow );

// Observer map
BEGIN_OBSERVER_MAP ( CWatchUnitFrame, CCaptionWindow )
  BEGIN_NOTIFIER ( IDC_Close )
    ON_BUTTONCOMMAND ( OnClose )
  END_NOTIFIER ()

  BEGIN_NOTIFIER ( IDC_Unit )
    EVENT ( CCUnit::E_PositionChanged )
	    RecomputeViewportPosition(); return FALSE;
    EVENT ( CCUnit::E_FullInfoChanged )
      RecomputeCaption(); UpdateFullInfoList(); return FALSE;
    EVENT ( CCUnit::E_UnitDisappeared )
      OnUnitDisappeared ( ( CCUnit * ) dwParam ); return FALSE;
  END_NOTIFIER ()
END_OBSERVER_MAP ( CWatchUnitFrame, CCaptionWindow )


// ----------------------------------------------------
// Construction
// ----------------------------------------------------

// Constructor
CWatchUnitFrame::CWatchUnitFrame ()
{
  // Empty the data
  m_pWatchedUnit = NULL;
}

// Destructor
CWatchUnitFrame::~CWatchUnitFrame ()
{
  // Assert the data is empty
  ASSERT ( m_pWatchedUnit == NULL );
}


// ----------------------------------------------------
// Debug functions
// ----------------------------------------------------

#ifdef _DEBUG

// Asserts validity of the object
void CWatchUnitFrame::AssertValid () const
{
  // Call the base class
  CCaptionWindow::AssertValid ();

  // Assert our data
  ASSERT ( m_pWatchedUnit != NULL );
  ASSERT ( m_pServerUnitInfoReceiver != NULL );
}

// Dumps the object to given context
void CWatchUnitFrame::Dump ( CDumpContext &dc ) const
{
  // Call the base class
  CCaptionWindow::Dump ( dc );

  // Dump our data
  dc << "Watched unit : " << (void *)m_pWatchedUnit << "\n";
  dc << "Server unit info receiver : " << (void *)m_pServerUnitInfoReceiver << "\n";
}

#endif //_DEBUG



// ----------------------------------------------------
// Creation
// ----------------------------------------------------

// Creates the window on ptPosition, for pUnit with pParent
void CWatchUnitFrame::Create ( CPoint & ptPosition, CCUnit * pUnit, CCaptionWindowLayout * pLayout,
                               CCServerUnitInfoReceiver * pServerUnitInfoReceiver,
                               CUnitSelection * pUnitSelection, CDataArchive * pGraphicsArchive, CWindow * pParent )
{
  ASSERT_VALID ( pUnit );
  ASSERT ( pParent != NULL );
  ASSERT ( pServerUnitInfoReceiver != NULL );
  ASSERT ( pUnitSelection != NULL );
  ASSERT ( pGraphicsArchive != NULL );

  // Remember the unit
  // Addref the unit also
  pUnit->AddRef ();
  m_pWatchedUnit = pUnit;
  m_pWatchedUnit->Connect ( this, IDC_Unit );

  // Remember the server unit info receiver
  m_pServerUnitInfoReceiver = pServerUnitInfoReceiver;

  // We want the full info on this unit
  m_pServerUnitInfoReceiver->StartReceivingFullInfo ( m_pWatchedUnit );

  // Create the rect for our window
  CRect rcWindow ( ptPosition.x, ptPosition.y,
    ptPosition.x + WATCHUNITFRAME_WIDTH, ptPosition.y + WATCHUNITFRAME_HEIGHT );

  // Create the caption widnow
  CCaptionWindow::Create ( rcWindow, CString(), pLayout, pParent, FALSE );
  m_bTransparent = FALSE;

  // Create the close button
  {
    CDialogWindowLayout *pLayout = CLayouts::m_pDefaults->GetDialogWindowLayout ();

    // Create the point and button itself there
    CPoint pt ( rcWindow.Width () - pLayout->m_dwCloseRightMargin,
      pLayout->m_dwCloseTopMargin );
    m_wndCloseButton.Create ( pt, & ( pLayout->m_CloseButtonLayout ), this, FALSE );

    // connect it
    m_wndCloseButton.Connect ( this, IDC_Close );
  }

  // Create the viewport
  {
    CRect rcViewport;
    rcViewport.left = WATCHUNITFRAME_VIEWPORT_LEFT;
    rcViewport.top = WATCHUNITFRAME_VIEWPORT_TOP;
    rcViewport.right = rcViewport.left + WATCHUNITFRAME_VIEWPORT_WIDTH;
    rcViewport.bottom = rcViewport.top + WATCHUNITFRAME_VIEWPORT_HEIGHT;

    m_wndViewport.Create ( rcViewport, g_pMap, m_pServerUnitInfoReceiver, this, pUnitSelection, pGraphicsArchive );
  }

  // Create the Full Info list box
  {
    // Create the layout
    m_cFullInfoListLayout.Create ( pGraphicsArchive->CreateFile ( "MapView\\WatchedUnit\\FullInfo.listctrl" ) );
    // Create the list
    CRect rcList;
    rcList.left = WATCHUNITFRAME_VIEWPORT_LEFT + WATCHUNITFRAME_VIEWPORT_WIDTH + WATCHUNITFRAME_VSPACE;
    rcList.top = WATCHUNITFRAME_FULLINFO_TOP;
    rcList.right = rcList.left + WATCHUNITFRAME_FULLINFO_WIDTH;
    rcList.bottom = rcList.top + WATCHUNITFRAME_FULLINFO_HEIGHT;
    m_wndFullInfoList.Create ( rcList, &m_cFullInfoListLayout, this );
    m_wndFullInfoList.Connect ( this, IDC_FullInfoList );
  }

  // Set the caption
  RecomputeCaption ();
  // Set the viewport position
  RecomputeViewportPosition ();
}

// Deletes the window
void CWatchUnitFrame::Delete ()
{
  // Delete the full info list
  m_wndFullInfoList.Delete ();
  m_cFullInfoListLayout.Delete ();

  // Delete the veiwport
  m_wndViewport.Delete ();

  // Delete the close button
  m_wndCloseButton.Delete ();

  // Delete the window first
  CCaptionWindow::Delete ();

  // Stop watching the unit
  m_pServerUnitInfoReceiver->StopReceivingFullInfo ( m_pWatchedUnit );
  // Forget the server unit info receiver
  m_pServerUnitInfoReceiver = NULL;

  // Forget the unit now
  m_pWatchedUnit->Disconnect ( this );
  m_pWatchedUnit->Release ();
  m_pWatchedUnit = NULL;
}



// ----------------------------------------------------
// Enable/Disable methods
// ----------------------------------------------------

// Enables all child windows
void CWatchUnitFrame::EnableAll ()
{
  ASSERT_VALID ( this );

  // Enable the viewport
  m_wndViewport.EnableWindow ();
}

// Disables those child windows that should be disabled, when the mapview is
// in disabled state (means almost everything - except the close button for example)
void CWatchUnitFrame::DisablePartial ()
{
  ASSERT_VALID ( this );

  // Disable the viewport
  m_wndViewport.EnableWindow ( FALSE );
}



// ----------------------------------------------------
// Window overloaded functions
// ----------------------------------------------------

// Reaction on size change
void CWatchUnitFrame::OnSize ( CSize size )
{
  // Move the close button to the right position
  CDialogWindowLayout *pLayout = CLayouts::m_pDefaults->GetDialogWindowLayout ();
  CPoint pt ( size.cx - pLayout->m_dwCloseRightMargin,
    pLayout->m_dwCloseTopMargin );
  m_wndCloseButton.SetWindowPosition ( &pt );
}



// ----------------------------------------------------
// Event reactions
// ----------------------------------------------------

// The close button was pressed
void CWatchUnitFrame::OnClose ()
{
  // Send notification event to the map view - we're closing ourselves
  InlayEvent ( E_Close, (DWORD)this );
}


// ----------------------------------------------------
// Helper functions
// ----------------------------------------------------

// Recomputes the caption of the window from unit's name and type
void CWatchUnitFrame::RecomputeCaption ()
{
  ASSERT_VALID ( this );
  // Get the unit type name
  CString strTypeName = m_pWatchedUnit->GetUnitType ()->GetGeneralUnitType ()->GetName ();
  // Get the civilization name
  CString strCivilizationName;
  if ( m_pWatchedUnit->GetUnitType ()->GetCivilization () != g_pCivilization )
  {
    strCivilizationName = m_pWatchedUnit->GetUnitType ()->GetCivilization ()->GetName ();
  }

  // Compose the caption
  CString strCaption;
  if ( strCivilizationName.IsEmpty () )
  {
    strCaption = strTypeName;
  }
  else
  {
    strCaption.Format ( "%s - %s", strTypeName, strCivilizationName );
  }

  // Set new caption
  CCaptionWindow::SetCaption ( strCaption );
}

// Recomputes the viewport position from the unit's position
void CWatchUnitFrame::RecomputeViewportPosition ()
{
  // Get the viewport size
  CRect rcViewport = m_wndViewport.GetWindowPosition ();

  // Compute the position so that the unit is in the center of the viewport
  int nXPos = m_pWatchedUnit->GetXPixelPosition () - rcViewport.Width () / 2;
  if ( nXPos < 0 ) nXPos = 0;
  int nYPos = m_pWatchedUnit->GetYPixelPosition () - rcViewport.Height () / 2;
  if ( nYPos < 0 ) nYPos = 0;

  // Set the position
  m_wndViewport.SetPosition ( (DWORD)nXPos, (DWORD)nYPos );
}

// Refills the full info list
void CWatchUnitFrame::UpdateFullInfoList ()
{
  ASSERT_VALID ( this );

  // Clear the list
  int nFirstVisible = m_wndFullInfoList.GetFirstVisible ();
  m_wndFullInfoList.RemoveAll ();

  // Retrieve the full info from the unit as a string
  CString strFullInfo;
  {
    char *pHlp = strFullInfo.GetBufferSetLength ( m_pWatchedUnit->GetFullInfoSize () + 1 );
	memcpy ( pHlp, m_pWatchedUnit->GetFullInfo (), m_pWatchedUnit->GetFullInfoSize () );
	pHlp [ m_pWatchedUnit->GetFullInfoSize () ] = 0;
	strFullInfo.ReleaseBuffer ();
  }

  // Go through the string and add each line to the list
  CString strLine;
  int nPos = 0, nFind = 0, nLen = strFullInfo.GetLength ();
  while ( nFind != -1 )
  {
    nFind = strFullInfo.Find ( '\n', nPos );
    if ( nFind != -1 )
    {
      strLine = strFullInfo.Mid ( nPos, nFind - nPos );
      nPos = nFind + 1;
	    m_wndFullInfoList.AddItem ( strLine );
    }
  }
  strLine = strFullInfo.Mid ( nPos );
  if ( !strLine.IsEmpty () )
	m_wndFullInfoList.AddItem ( strLine );

  m_wndFullInfoList.SetFirstVisible ( nFirstVisible );
}

// Watched unit has disappeared -> close the window
// This can only happen on the enemy unit
void CWatchUnitFrame::OnUnitDisappeared ( CCUnit * pUnit )
{
  ASSERT_VALID ( this );

  // Ooops, it's not our unit
  if ( pUnit != m_pWatchedUnit ) return;

  // Elsewaye, just close the window
  // Send notification event to the map view - we're closing ourselves
  InlayEvent ( E_Close, (DWORD)this );
}
