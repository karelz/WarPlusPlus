/***********************************************************
 * 
 *     Project: Strategy game
 *        Part: GameClient - MapView
 *      Author: Vit Karas
 * 
 * Description: Frame window for watched unit
 * 
 ***********************************************************/

#ifndef _GAMECLIENT_WATCHUNITFRAME_H_
#define _GAMECLIENT_WATCHUNITFRAME_H_

#include "..\DataObjects\CUnit.h"
#include "..\ServerCommunication\CServerUnitInfoReceiver.h"
#include "InteractiveViewport.h"

class CWatchUnitFrame : public CCaptionWindow  
{
  // RTCI for our classes
  DECLARE_DYNAMIC ( CWatchUnitFrame );

  // Observer map
  DECLARE_OBSERVER_MAP ( CWatchUnitFrame );
public:
// Construction
  // Constructor
	CWatchUnitFrame ();
  // Destructor
	virtual ~CWatchUnitFrame ();

// Debug functions
#ifdef _DEBUG
  // Asserts validity of the object
  virtual void AssertValid () const;
  // Dumps the object to given context
  virtual void Dump ( CDumpContext &dc ) const;
#endif //_DEBUG

// Creation
  // Creates the window on ptPosition, for pUnit with pParent
  void Create ( CPoint &ptPosition, CCUnit *pUnit, CCaptionWindowLayout *pLayout,
                CCServerUnitInfoReceiver * pServerUnitInfoReceiver,
                CUnitSelection * pUnitSelection, CDataArchive * pGraphicsArchive, CWindow * pParent );
  // Deletes the window
  virtual void Delete ();

// Get/Set methods
  // Returns watched unit
  CCUnit * GetWatchedUnit () const { ASSERT_VALID ( this ); return m_pWatchedUnit; }
  // Returns pointer to interactive viewport that is inside this frame
  CInteractiveViewport * GetViewport () const { ASSERT_VALID ( this ); return const_cast < CInteractiveViewport * > ( &m_wndViewport ); }

// Enable/Disable methods
  // Enables all child windows
  void EnableAll ();
  // Disables those child windows that should be disabled, when the mapview is
  // in disabled state (means almost everything - except the close button for example)
  void DisablePartial ();

// Enums
  // Events sent by this notifier
  enum
  {
    E_Close = 1,  // Sent shen the window is about to close (dwParam is (DWORD)(CWatchUnitFrame *)pFrame
                  // which sent this event
  };

protected:
// Window overloaded functions
  // Reaction on size change
  virtual void OnSize ( CSize size );

// Event reactions
  // The close button was pressed
  void OnClose ();

  // Watched unit has disappeared -> close the window
  // This can only happen on the enemy unit
  void OnUnitDisappeared ( CCUnit * pUnit );

  enum
  {
    IDC_Close = 0x0100,  // IDC of the close button
    IDC_FullInfoList = 0x01000, // The full info list box

    IDC_Unit = 0x010000, // The unit we're watching here
  };

private:
// Data members
  // Watched unit (we have Addref called on it)
  CCUnit * m_pWatchedUnit;
  // The server unit info receiver
  CCServerUnitInfoReceiver * m_pServerUnitInfoReceiver;

  // The close button
  CImageButton m_wndCloseButton;

  // The viewport for this watch unit frame
  CInteractiveViewport m_wndViewport;

  // Layout of the full info list
  CListControlLayout m_cFullInfoListLayout;
  // The full info list
  CListControl m_wndFullInfoList;

// Helper functions
  // Recomputes the caption of the window from unit's name and type
  void RecomputeCaption ();
  // Recomputes the viewport position from the unit's position
  void RecomputeViewportPosition ();
  // Refills the full info list
  void UpdateFullInfoList ();

};

#endif //_GAMECLIENT_WATCHUNITFRAME_H_