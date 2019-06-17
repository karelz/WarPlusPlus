// InteractiveViewport.cpp: implementation of the CInteractiveViewport class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "InteractiveViewport.h"
#include "..\GameClientGlobal.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

#define SELECTION_RECT_COLOR RGB32 ( 0, 255, 0 )
#define SELECTION_RECT_THICKNESS 1

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

IMPLEMENT_DYNAMIC ( CInteractiveViewport, CViewport )

BEGIN_OBSERVER_MAP ( CInteractiveViewport, CViewport )
  BEGIN_MOUSE ()
    ON_LBUTTONDOWN ()
    ON_LBUTTONUP ()
    ON_LBUTTONDBLCLK ()
    ON_RBUTTONDOWN ()
    ON_RBUTTONUP ()
    ON_MOUSEMOVE ()
  END_MOUSE ()
END_OBSERVER_MAP ( CInteractiveViewport, CViewport )

CInteractiveViewport::CInteractiveViewport()
{
  m_eViewportState = ViewportState_Normal;
  m_eViewportQuestion = ViewportQuestion_None;
  m_pUnitSelection = NULL;
  m_bLifeBarMarking = true;
  m_bMouseDragging = false;
  m_bRectangleSelectOurUnits = true;
  m_bRectangleSelectionEnabled = false;
}

CInteractiveViewport::~CInteractiveViewport()
{
  ASSERT(m_pUnitSelection == NULL);
}

#ifdef _DEBUG

void CInteractiveViewport::AssertValid() const
{
  CViewport::AssertValid();

  ASSERT(m_pUnitSelection != NULL);
}

void CInteractiveViewport::Dump(CDumpContext &dc) const
{
  CViewport::Dump(dc);
}

#endif

// Creates the viewport
void CInteractiveViewport::Create(CRect &rcWindowRect, CCMap *pMap, CCServerUnitInfoReceiver *pServerUnitInfoReceiver,
                             CWindow *pParent, CUnitSelection *pUnitSelection, CDataArchive *pGraphicsArchive,
                             CDDrawSurface *pExternalFullScreenBuffer)
{
  ASSERT(m_pUnitSelection == NULL);
  ASSERT_VALID(pUnitSelection);

  // Copy the selection object
  m_pUnitSelection = pUnitSelection;
  // Create our selection
  m_cKeepSelectionWhileDragging.Create ( m_pUnitSelection->GetUnitCache () );

  // Create cursors
  m_LocationCursor.Create(pGraphicsArchive->CreateFile("Mouse\\Location.cursor"));
  m_EnemyUnitCursor.Create(pGraphicsArchive->CreateFile("Mouse\\EnemyUnit.cursor"));
  m_MyUnitCursor.Create(pGraphicsArchive->CreateFile("Mouse\\MyUnit.cursor"));
  m_MyDragCursor.Create ( pGraphicsArchive->CreateFile ( "Mouse\\MyDrag.cursor"));
  m_EnemyDragCursor.Create ( pGraphicsArchive->CreateFile ( "Mouse\\EnemyDrag.cursor" ));

  // Create the viewport
  m_bDoubleClk = TRUE;
  CViewport::Create(rcWindowRect, pMap, pServerUnitInfoReceiver, pParent, pExternalFullScreenBuffer);
}

// Deletes the viewport
void CInteractiveViewport::Delete()
{
  // Call it on our base class
  CViewport::Delete();

  // Delete cursors
  m_MyDragCursor.Delete (); m_EnemyDragCursor.Delete ();
  m_MyUnitCursor.Delete(); m_EnemyUnitCursor.Delete(); m_LocationCursor.Delete();

  // forget the selection object
  m_cKeepSelectionWhileDragging.Delete ();
  m_pUnitSelection = NULL;
}

// Called before the all units are locked
// int the computing frames
// If you need some lock in GetUnitMarks or AdjustNodeSize lock it here
void CInteractiveViewport::BeforeUnitsLocking ()
{
  ASSERT_VALID ( this );

  m_pUnitSelection->Lock ();
}

// Called after all units are unlocked
// unlock everything you've locked in BeforeUnitsLocking
void CInteractiveViewport::AfterUnitsLocking ()
{
  ASSERT_VALID ( this );

  m_pUnitSelection->Unlock ();
}


// Returns marks for units in the viewport
DWORD CInteractiveViewport::GetUnitMarks(CCUnit *pUnit)
{
  ASSERT_VALID(this);

  DWORD dwMarks = CViewport::UnitMark_None;
  bool bSelected = false;

  // Try to find the unit in the selection
  // If it's there -> mark it as selected
  // elseway no marks
  if ( m_pUnitSelection->Find ( pUnit ) != NULL )
  {
    // Unit is in the selection
    // So mark it as selected
    dwMarks |= CViewport::UnitMark_Selected;
    bSelected = true;
  }

  // If the lifebar marking is switched on -> mark it
  if ( m_bLifeBarMarking )
  {
    switch ( pUnit->GetUnitType ()->GetGeneralUnitType ()->GetLifeBarFlags () )
    {
    case UnitTypeFlags_LifeBar_ShowWhenSelected:
      if ( !bSelected ) break;
    case UnitTypeFlags_LifeBar_ShowAlways:
      dwMarks |= CViewport::UnitMark_LifeBar;
    }
  }

  return dwMarks;
}


//////////////////////////////////////////////////////////////////
// Mouse event reactions

// LButton was clicked
void CInteractiveViewport::OnLButtonDown(CPoint pt)
{
  ASSERT_VALID(this);

  CPoint ptMap = pt;

  // Convert the point to pixels on the map
  ptMap.x += CViewport::m_dwXPosition;
  ptMap.y += CViewport::m_dwYPosition;

  // Find the unit under the mouse cursor
  CCUnit *pFoundUnit = FindSelectableUnitByPoint(ptMap);

  switch(m_eViewportState){
  case ViewportState_Normal:
    // First set us to dragging mode (enable the rectangle selection)
    if ( !m_bMouseDragging && m_bRectangleSelectionEnabled )
    {
      m_bMouseDragging = true;
      // Capture the mouse
      SetCapture ();
      // Remember the start position
      m_ptDragStart = ptMap;
    }

    // Lock the selection
    // We have to do it, cause we will call functions, that need it
    // (Like IsOurCivilization)
    m_pUnitSelection->Lock ();

    // If no unit found -> Clear the selection (if none modification key is down)
    if ( pFoundUnit == NULL )
    {
      if ( !g_pKeyboard->ShiftPressed () )
      {
        m_pUnitSelection->Clear ();
        m_bRectangleSelectOurUnits = true;
      }
      else
      {
        if ( m_pUnitSelection->IsOurCivilization () )
          m_bRectangleSelectOurUnits = true;
        else
          m_bRectangleSelectOurUnits = false;
      }
      // Also remember the selection for drag selections
      m_cKeepSelectionWhileDragging.Clear ();
      m_cKeepSelectionWhileDragging.Append ( m_pUnitSelection );
      m_pUnitSelection->Unlock ();
      break;
    }

    // Elseway look at keyboard status
    if(g_pKeyboard->ShiftPressed()){
      // If the shift is down, left the selection and just add/remove the found unit

      if(m_pUnitSelection->Find(pFoundUnit)){
        // If the unit is in the selection -> remove it
        m_pUnitSelection->Remove(pFoundUnit);
      }
      else{
        // If the unit is not in the selection -> add it
        // this can cause cleaning of the selection because of
        // different civlizations of selected units
        // For details see CUnitSelection::Add
        m_pUnitSelection->Add(pFoundUnit);
      }
    }
    else{
      // No shift key -> clear the selection and add this unit only
      m_pUnitSelection->Clear();
      m_pUnitSelection->Add(pFoundUnit);
    }
    if ( m_pUnitSelection->IsOurCivilization () )
      m_bRectangleSelectOurUnits = true;
    else
      m_bRectangleSelectOurUnits = false;
    // Also remember the selection for drag selections
    m_cKeepSelectionWhileDragging.Clear ();
    m_cKeepSelectionWhileDragging.Append ( m_pUnitSelection );
    m_pUnitSelection->Unlock ();

    break;

  case ViewportState_Disabled:
    break;

  case ViewportState_Asking:
    {
      switch(m_eViewportQuestion){
      case ViewportQuestion_None:
        break;
      case ViewportQuestion_Position:
        {
          DWORD dwPosition;
          dwPosition = ((ptMap.x / MAPCELL_WIDTH) << 16) | ((ptMap.y / MAPCELL_HEIGHT) & 0x0FFFF);
          InlayEvent(E_Position, dwPosition);
        }
        break;

      case ViewportQuestion_MyUnit:
        {
          if((pFoundUnit != NULL) && (pFoundUnit->GetUnitType()->GetCivilization() == g_pCivilization)){
            pFoundUnit->AddRef();
            InlayEvent(E_Unit, (DWORD)pFoundUnit);
          }
        }
        break;

      case ViewportQuestion_EnemyUnit:
        {
          if((pFoundUnit != NULL) && (pFoundUnit->GetUnitType()->GetCivilization() != g_pCivilization)){
            pFoundUnit->AddRef();
            InlayEvent(E_Unit, (DWORD)pFoundUnit);
          }
        }
        break;

      case ViewportQuestion_AnyUnit:
        {
          if(pFoundUnit != NULL){
            pFoundUnit->AddRef();
            InlayEvent(E_Unit, (DWORD)pFoundUnit);
          }
        }
        break;
      }
    }
    break;
  }
}

// LButton was released
void CInteractiveViewport::OnLButtonUp(CPoint pt)
{
  // If we were dragging release the mouse
  if ( m_bMouseDragging )
  {
    // Clear the selection rectangle
    CRect rcSelection ( 0, 0, 0, 0 );
    SetRectangleSelection ( rcSelection );

    // Release the mouse
    m_bMouseDragging = false;
    ReleaseCapture ();

    // Leave the unit selection as it is, cause we've selected all units during
    // the mouse drag
  }

  ASSERT_VALID(this);
}

// LButton was doubleclicked
void CInteractiveViewport::OnLButtonDblClk ( CPoint pt )
{
  ASSERT_VALID ( this );

  CPoint ptMap = pt;

  // Convert the point to pixels on the map
  ptMap.x += CViewport::m_dwXPosition;
  ptMap.y += CViewport::m_dwYPosition;

  // Find the unit under the mouse cursor
  CCUnit *pFoundUnit = FindSelectableUnitByPoint(ptMap);

  switch ( m_eViewportState )
  {
  case ViewportState_Normal:
    {
      // No unit found -> nothing to do
      if ( pFoundUnit == NULL )
      {
        return;
      }

      // Send event to mapview to open new window for watching the unit
      pFoundUnit->AddRef ();
      InlayEvent ( E_WatchUnit, (DWORD) pFoundUnit );
    }
    break;

  case ViewportState_Disabled:
    break;

  case ViewportState_Asking:
    break;
  }
}

// RButton was clicked
void CInteractiveViewport::OnRButtonDown(CPoint pt)
{
  ASSERT_VALID(this);
}

void CInteractiveViewport::OnRButtonUp(CPoint pt)
{
  ASSERT_VALID(this);

  switch(m_eViewportState){
  case ViewportState_Normal:
    {
      CPoint ptMap = pt;

      // Convert the point to pixels on the map
      ptMap.x += CViewport::m_dwXPosition;
      ptMap.y += CViewport::m_dwYPosition;

      // Find the unit under the mouse cursor
      CCUnit *pFoundUnit = FindSelectableUnitByPoint(ptMap);

      if ( pFoundUnit != NULL )
      {
        pFoundUnit->AddRef ();
        if ( g_pKeyboard->ShiftPressed () )
          InlayEvent ( E_ShiftRightClickUnit, (DWORD)pFoundUnit );
        else
          InlayEvent ( E_RightClickUnit, (DWORD)pFoundUnit );
      }
      else
      {
        // Position clicked
        DWORD dwPosition;
        dwPosition = ((ptMap.x / MAPCELL_WIDTH) << 16) | ((ptMap.y / MAPCELL_HEIGHT) & 0x0FFFF);
        if ( g_pKeyboard->ShiftPressed () )
          InlayEvent ( E_ShiftRightClickPosition, dwPosition );
        else
          InlayEvent ( E_RightClickPosition, dwPosition );
      }
    }
    break;
  case ViewportState_Disabled:
    InlayEvent(E_Cancel, 0);
    break;
  case ViewportState_Asking:
    InlayEvent(E_Cancel, 1);
    break;
  }
}

void CInteractiveViewport::OnMouseMove(CPoint pt)
{
  ASSERT_VALID(this);

  CPoint ptMap = pt;

  // Convert the point to pixels on the map
  ptMap.x += CViewport::m_dwXPosition;
  ptMap.y += CViewport::m_dwYPosition;

  switch(m_eViewportState){
  case ViewportState_Normal:
    {
      // If we're dragging -> update the dragging rectangle
      if ( m_bMouseDragging && m_bRectangleSelectionEnabled )
      {
        // Lock the selection
        m_pUnitSelection->Lock ();

        // Set the window cursor to the one, that is equivalent to
        // the selection
        if ( m_bRectangleSelectOurUnits )
        {
          // Set our civilization cursor
          SetWindowCursor ( &m_MyDragCursor );
        }
        else
        {
          // Set the enemy cursor
          SetWindowCursor ( &m_EnemyDragCursor );
        }

        // Unlock the selection
        m_pUnitSelection->Unlock ();

        // Update the rectangle selection
        CRect rcSelection;
        if ( m_ptDragStart == ptMap )
        {
          rcSelection.left = m_ptDragStart.x;
          rcSelection.top = m_ptDragStart.y;
          rcSelection.right = rcSelection.left + 1;
          rcSelection.bottom = rcSelection.top + 1;
        }
        else
        {
          if ( ptMap.x < m_ptDragStart.x ){
            rcSelection.left = ptMap.x;
            rcSelection.right = m_ptDragStart.x;
          }
          else{
            rcSelection.left = m_ptDragStart.x;
            rcSelection.right = ptMap.x;
          }
          if ( ptMap.y < m_ptDragStart.y ){
            rcSelection.top = ptMap.y;
            rcSelection.bottom = m_ptDragStart.y;
          }
          else{
            rcSelection.top = m_ptDragStart.y;
            rcSelection.bottom = ptMap.y;
          }
        }
        SetRectangleSelection ( rcSelection );
      }
      else
      {
        // Else way just update the cursor
        CCUnit *pUnit = FindSelectableUnitByPoint(ptMap);
        if(pUnit == NULL){
          // No unit under cursor
          SetWindowCursor(NULL);
        }
        else{
          if(pUnit->GetUnitType()->GetCivilization() == g_pCivilization){
            // Our unit under cursor
            SetWindowCursor(&m_MyUnitCursor);
          }
          else{
            // Enemy unit under cursor
            SetWindowCursor(&m_EnemyUnitCursor);
          }
        }
      }
    }
    break;

  case ViewportState_Disabled:
    // Do nothing
    break;

  case ViewportState_Asking:
    {
      CCUnit *pUnit = FindSelectableUnitByPoint(ptMap);
      switch(m_eViewportQuestion){

      case ViewportQuestion_None:
        break;

      case ViewportQuestion_Position:
        break;

      case ViewportQuestion_MyUnit:
        if((pUnit != NULL) && (pUnit->GetUnitType()->GetCivilization() == g_pCivilization)){
          SetWindowCursor(&m_MyUnitCursor);
        }
        else{
          SetWindowCursor(NULL);
        }
        break;

      case ViewportQuestion_EnemyUnit:
        if((pUnit != NULL) && (pUnit->GetUnitType()->GetCivilization() != g_pCivilization)){
          SetWindowCursor(&m_EnemyUnitCursor);
        }
        else{
          SetWindowCursor(NULL);
        }
        break;

      case ViewportQuestion_AnyUnit:
        if(pUnit == NULL){
          SetWindowCursor(NULL);
        }
        else{
          if(pUnit->GetUnitType()->GetCivilization() == g_pCivilization){
            SetWindowCursor(&m_MyUnitCursor);
          }
          else{
            SetWindowCursor(&m_EnemyUnitCursor);
          }
        }
        break;

      }
    }
    break;
  }
}

//////////////////////////////////////////////////////////////////
// Helper functions

// Returns the unit which is selectable and given point is on it
// Given point is in pixel on the map !!!!
CCUnit *CInteractiveViewport::FindSelectableUnitByPoint(CPoint pt)
{
  // Go through the unit nodes list and find it

  CCUnit *pFoundUnit = NULL;
  CViewport::SUnitNode *pNode;

  // Lock the viewport for reading
  VERIFY(m_lockViewport.ReaderLock());

  // Because the unit node list is sorted in a drawing way
  // we'll get the last unit node that intersects our point
  // and that is selectable
  pNode = m_pUnitNodeList;
  while(pNode != NULL){
    // Is the point in the unit node ?
    if(pNode->m_rcPosition.PtInRect(pt)){
      // Yes, so this is a good one
      // Is it selectable ?
      if(pNode->m_pUnit->GetUnitType()->GetGeneralUnitType()->IsSelectable()){
        // Yes, even better
        // This is the new candidate to be returned
        pFoundUnit = pNode->m_pUnit;
      }
    }

    // go to the next unit node
    pNode = pNode->m_pNext;
  }

  // Unlock the viewport
  m_lockViewport.ReaderUnlock();

  return pFoundUnit;
}

void CInteractiveViewport::SetViewportState(EViewportState eState)
{
  m_eViewportState = eState;
  switch(eState){
  case ViewportState_Normal:
    // set normal cursor for us
    SetWindowCursor(NULL);
    break;
  case ViewportState_Disabled:
    // set waiting cursor for us
    SetWindowCursor(g_pMouse->GetWaitingCursor());
    break;
  case ViewportState_Asking:
    // set normal cursor for us
    SetWindowCursor(NULL);
    break;
  }
}

void CInteractiveViewport::SetViewportQuestion(EViewportQuestion eQuestion)
{
  m_eViewportQuestion = eQuestion;
  switch(eQuestion){
  case ViewportQuestion_None:
    SetWindowCursor(NULL);
    break;
  case ViewportQuestion_Position:
    SetWindowCursor(&m_LocationCursor);
    break;
  case ViewportQuestion_MyUnit:
  case ViewportQuestion_EnemyUnit:
  case ViewportQuestion_AnyUnit:
    SetWindowCursor(NULL);
    break;
  }
}

// Sets new rectangle selection
void CInteractiveViewport::SetRectangleSelection ( CRect & rcSelection )
{
  VERIFY ( m_lockRectangleSelection.Lock () );
  // Just copoy it to our variable
  m_rcCurrentRectangleSelection = rcSelection;
  VERIFY ( m_lockRectangleSelection.Unlock () );
}

// This functions is called before computing the frame updates
// You can add any rectangles to given list and this is the way you
// register some rects for updating
// The viewport is locked
void CInteractiveViewport::AddOverlayUpdates ( CUpdateRectList * pUpdateRectList )
{
  // Lock the rectangle selection
  VERIFY ( m_lockRectangleSelection.Lock () );

  // If the selection rectangle changed -> update changes
  if ( !m_rcCurrentRectangleSelection.EqualRect ( &m_rcLastDrawnRectangleSelection ) )
  {
    // We'll do it in the very simple way, just update the old one
    // and the new one as well

    // So first the old one
    CRect rcUpdate;
    if ( !m_rcLastDrawnRectangleSelection.IsRectEmpty () )
    {
      // Top line
      rcUpdate.left = m_rcLastDrawnRectangleSelection.left - m_dwXPosition;
      rcUpdate.top = m_rcLastDrawnRectangleSelection.top - m_dwYPosition;
      rcUpdate.right = m_rcLastDrawnRectangleSelection.right - m_dwXPosition;
      rcUpdate.bottom = rcUpdate.top + SELECTION_RECT_THICKNESS;
      pUpdateRectList->AddRect ( &rcUpdate );

      // Left line
      rcUpdate.right = rcUpdate.left + SELECTION_RECT_THICKNESS;
      rcUpdate.bottom = m_rcLastDrawnRectangleSelection.bottom - m_dwYPosition;
      pUpdateRectList->AddRect ( &rcUpdate );

      // bottom line
      rcUpdate.right = m_rcLastDrawnRectangleSelection.right - m_dwXPosition;
      rcUpdate.top = rcUpdate.bottom - SELECTION_RECT_THICKNESS;
      pUpdateRectList->AddRect ( &rcUpdate );

      // Right line
      rcUpdate.left = rcUpdate.right - SELECTION_RECT_THICKNESS;
      rcUpdate.top = m_rcLastDrawnRectangleSelection.top - m_dwYPosition;
      pUpdateRectList->AddRect ( &rcUpdate );
    }

    // Now the new one
    if ( !m_rcCurrentRectangleSelection.IsRectEmpty () )
    {
      pUpdateRectList->AddRect ( &m_rcCurrentRectangleSelection );
      // Top line
      rcUpdate.left = m_rcCurrentRectangleSelection.left - m_dwXPosition;
      rcUpdate.top = m_rcCurrentRectangleSelection.top - m_dwYPosition;
      rcUpdate.right = m_rcCurrentRectangleSelection.right - m_dwXPosition;
      rcUpdate.bottom = rcUpdate.top + SELECTION_RECT_THICKNESS;
      pUpdateRectList->AddRect ( &rcUpdate );

      // Left line
      rcUpdate.right = rcUpdate.left + SELECTION_RECT_THICKNESS;
      rcUpdate.bottom = m_rcCurrentRectangleSelection.bottom - m_dwYPosition;
      pUpdateRectList->AddRect ( &rcUpdate );

      // bottom line
      rcUpdate.right = m_rcCurrentRectangleSelection.right - m_dwXPosition;
      rcUpdate.top = rcUpdate.bottom - SELECTION_RECT_THICKNESS;
      pUpdateRectList->AddRect ( &rcUpdate );

      // Right line
      rcUpdate.left = rcUpdate.right - SELECTION_RECT_THICKNESS;
      rcUpdate.top = m_rcCurrentRectangleSelection.top - m_dwYPosition;
      pUpdateRectList->AddRect ( &rcUpdate );
    }

    // Now copy the new one to the old one
    m_rcLastDrawnRectangleSelection = m_rcCurrentRectangleSelection;
  }

  // And now find all units, that should be in the selection
  if ( !m_rcLastDrawnRectangleSelection.IsRectEmpty () )
  {
    // Lock the unit selection
    m_pUnitSelection->Lock ();

    // We don't have to lock this viewport, cause it's already locked

    CUnitSelection cTempSelection;
    cTempSelection.Create ( m_pUnitSelection->GetUnitCache () );

    // Go through all units in the selection and copy out those, which is in
    // the old (current) one and aren't about to be kept
    {
      CUnitSelection::SUnitNode * pNode = m_pUnitSelection->GetFirstNode ();
      while ( pNode != NULL )
      {
        if ( !m_cKeepSelectionWhileDragging.Find ( pNode->m_dwUnitID ) )
          cTempSelection.Add ( pNode->m_dwUnitID, pNode->m_dwUnitTypeID, pNode->m_dwCivilizationID );
        pNode = m_pUnitSelection->GetNext ( pNode );
      }
    }

    // Now add all units in the selected rectangle
    // So go through all unit nodes on the view and check them
    {
      SUnitNode * pNode;
      CRect rcRect;
      for ( pNode = m_pUnitNodeList; pNode != NULL; pNode = pNode->m_pNext )
      {
        // If the unit is not in the selection rectangle
        rcRect.IntersectRect ( &m_rcLastDrawnRectangleSelection, &(pNode->m_rcPosition) );
        if ( rcRect.IsRectEmpty () )
          continue;
        // If the unit is selectable use it
        if ( !pNode->m_pUnit->GetUnitType()->GetGeneralUnitType()->IsSelectable() )
          continue;

        if ( m_bRectangleSelectOurUnits )
        {
          if ( pNode->m_pUnit->GetUnitType ()->GetCivilization () != g_pCivilization )
            continue;
        }
        else
        {
          if ( pNode->m_pUnit->GetUnitType ()->GetCivilization () == g_pCivilization )
            continue;
        }

        // First remove it from the temp selection
        cTempSelection.Remove ( pNode->m_pUnit );
        // And add it to the release selection
        m_pUnitSelection->Add ( pNode->m_pUnit );
      }
    }

    // Now go through all units, which are left in the temp list
    // and remove them from the selection
    {
      CUnitSelection::SUnitNode * pNode = cTempSelection.GetFirstNode ();
      while ( pNode != NULL )
      {
        m_pUnitSelection->Remove ( pNode->m_pUnit );
        pNode = cTempSelection.GetNext ( pNode );
      }
    }

    cTempSelection.Delete ();

    // Unlock the unit selection
    m_pUnitSelection->Unlock ();
  }

  // Unlock the selection rectangle
  VERIFY ( m_lockRectangleSelection.Unlock () );
}

// Draw any overlay - it means here you can draw anything over
// the viewport graphics
// But if you want to change it, you must register a rect for
// updating in AddOverlayUpdates
// You should draw it to given surface, as if you'd draw it to the
// frame buffer (usually it will be the frame buffer)
// The rcUpdateRect is the rect that you should draw into
// (in fact the surface will have set the clipping just for this rect)
// The viewport is locked
void CInteractiveViewport::DrawOverlay ( CDDrawSurface * pSurface, CRect & rcUpdateRect )
{
  // Lock the selection rectangle
  VERIFY ( m_lockRectangleSelection.Lock () );

  // If the selection rectangle is not empty draw it
  if ( !m_rcLastDrawnRectangleSelection.IsRectEmpty () )
  {
    CRect rcUpdate;

    // Top line
    rcUpdate.left = m_rcLastDrawnRectangleSelection.left - m_dwXPosition;
    rcUpdate.top = m_rcLastDrawnRectangleSelection.top - m_dwYPosition;
    rcUpdate.right = m_rcLastDrawnRectangleSelection.right - m_dwXPosition;
    rcUpdate.bottom = rcUpdate.top + SELECTION_RECT_THICKNESS;
    pSurface->Fill ( SELECTION_RECT_COLOR, &rcUpdate );

    // Left line
    rcUpdate.right = rcUpdate.left + SELECTION_RECT_THICKNESS;
    rcUpdate.bottom = m_rcLastDrawnRectangleSelection.bottom - m_dwYPosition;
    pSurface->Fill ( SELECTION_RECT_COLOR, &rcUpdate );

    // bottom line
    rcUpdate.right = m_rcLastDrawnRectangleSelection.right - m_dwXPosition;
    rcUpdate.top = rcUpdate.bottom - SELECTION_RECT_THICKNESS;
    pSurface->Fill ( SELECTION_RECT_COLOR, &rcUpdate );

    // Right line
    rcUpdate.left = rcUpdate.right - SELECTION_RECT_THICKNESS;
    rcUpdate.top = m_rcLastDrawnRectangleSelection.top - m_dwYPosition;
    pSurface->Fill ( SELECTION_RECT_COLOR, &rcUpdate );
  }

  // Unlock the selection rectangle
  VERIFY ( m_lockRectangleSelection.Unlock () );
}
