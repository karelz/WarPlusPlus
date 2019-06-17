// MousePrimarySurface.cpp : implementation file
//

#include "stdafx.h"
#include "MousePrimarySurface.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define MPS_BUFFERWIDTH 50
#define MPS_BUFFERHEIGHT 50

CMousePrimarySurface *g_pMousePrimarySurface = NULL;
int dwCounter = 1;

/////////////////////////////////////////////////////////////////////////////
// CMousePrimarySurface

IMPLEMENT_DYNAMIC(CMousePrimarySurface, CDDPrimarySurface)

CMousePrimarySurface::CMousePrimarySurface()
{
  m_dwMouseX = 0; m_dwMouseY = 0;
  m_dwHotX = 0; m_dwHotY = 0;
  m_pMouseBitmap = NULL;
  m_bVisible = FALSE;
  m_pClipWindow = NULL;
}

CMousePrimarySurface::~CMousePrimarySurface()
{
  m_pMouseBitmap = NULL;
}


/////////////////////////////////////////////////////////////////////////////
// CMousePrimarySurface message handlers


void CMousePrimarySurface::SetMouseCursor(CDDrawSurface * pMouseBitmap, DWORD dwHotX, DWORD dwHotY)
{
  ASSERT_VALID(this);
#ifdef _DEBUG
  if(m_pMouseBitmap != NULL){
    ASSERT_VALID(m_pMouseBitmap);
  }
#endif

  if(m_pClipWindow){
    return;
  }

  CRect rectOldCursor(0, 0, 0, 0);
  CRect rectNewCursor(0, 0, 0, 0);
  CRect rectBoth, rectBothOnDisplay, rectBothBuffer;

  VERIFY(m_lockExclusive.Lock());

  try{

  if(m_pMouseBitmap == NULL){ // If there was no bitmap it's easy
    if(m_bVisible){ // if the mouse was visible just draw the new one
      m_pMouseBitmap = pMouseBitmap;
      m_dwHotX = dwHotX; m_dwHotY = dwHotY;
      ScanMouse();
      DrawMouse();
    }
    else{ // if wasn't visible just fill our variables
      m_pMouseBitmap = pMouseBitmap;
      m_dwHotX = dwHotX; m_dwHotY = dwHotY;
    }
    VERIFY(m_lockExclusive.Unlock());
    return;
  }

  if(m_bVisible == FALSE){ // If the mouse is not visible just fill the variables
    m_pMouseBitmap = pMouseBitmap;
    m_dwHotX = dwHotX; m_dwHotY = dwHotY;
    VERIFY(m_lockExclusive.Unlock());
    return;
  }

  if(pMouseBitmap == NULL){ // If the new one is NULL just hide the old one
    ClearMouse();
    m_pMouseBitmap = pMouseBitmap;
    m_dwHotX = dwHotX; m_dwHotY = dwHotY;
    VERIFY(m_lockExclusive.Unlock());
    return;
  }

  // elseway we have to do some wierd stuff
  // count the rect of the old cursor
  rectOldCursor = *(m_pMouseBitmap->GetAllRect());
  rectOldCursor.OffsetRect(-(LONG)m_dwHotX, -(LONG)m_dwHotY);

  // count the rect of the new cursor
  rectNewCursor = *(pMouseBitmap->GetAllRect());
  rectNewCursor.OffsetRect(-(LONG)dwHotX, -(LONG)dwHotY);

  // get the union of these
  rectBoth.UnionRect(&rectOldCursor, &rectNewCursor);
  
  // transform the rects to the buffer coordinates
  rectNewCursor.OffsetRect(-rectBoth.left, - rectBoth.top);
  rectOldCursor.OffsetRect(-rectBoth.left, - rectBoth.top);
  
  // get the rect of buffer on the screen
  rectBothOnDisplay = rectBoth;
  rectBothOnDisplay.OffsetRect(m_dwMouseX, m_dwMouseY);

  rectBothBuffer.SetRect(0, 0, rectBoth.Width(), rectBoth.Height());

  // copy the screen to the buffer
  m_MoveSurface.BltFast(0, 0, this, &rectBothOnDisplay);
  // erase the old cursor from the buffer
  m_MoveSurface.BltFast(rectOldCursor.left, rectOldCursor.top,
    &m_MouseBackground, m_pMouseBitmap->GetAllRect());
  // get the new cursor background
  m_MouseBackground.BltFast(0, 0, &m_MoveSurface, &rectNewCursor);
  // draw the new cursor to the buffer
  m_MoveSurface.BltFast(rectNewCursor.left, rectNewCursor.top,
    pMouseBitmap, pMouseBitmap->GetAllRect());
  // draw the buffer to the screen
  BltFast(rectBothOnDisplay.left, rectBothOnDisplay.top,
    &m_MoveSurface, &rectBothBuffer);

  m_pMouseBitmap = pMouseBitmap;
  m_dwHotX = dwHotX; m_dwHotY = dwHotY;

  }
  catch(...){
    VERIFY(m_lockExclusive.Unlock());
    throw;
  }

  VERIFY(m_lockExclusive.Unlock());
}

void CMousePrimarySurface::SetMousePos(DWORD dwX, DWORD dwY)
{
  ASSERT_VALID(this);
  
  if(m_pClipWindow){
    return;
  }
  
  CPoint pt(dwX, dwY);

  VERIFY(m_lockExclusive.Lock());

  try{

  if(!GetAllRect()->PtInRect(pt)){
    ASSERT(FALSE);
    VERIFY(m_lockExclusive.Unlock());
    return;
  }

  MoveMouse(dwX, dwY);

  }
  catch(...){
    VERIFY(m_lockExclusive.Unlock());
    throw;
  }

  VERIFY(m_lockExclusive.Unlock());
}

/////////////////////////////////////////////////////////////////////////////
// Debug methods

#ifdef _DEBUG

void CMousePrimarySurface::AssertValid() const
{
  CDDPrimarySurface::AssertValid();
  if(m_pMouseBitmap != NULL) ASSERT_VALID(m_pMouseBitmap);
}

void CMousePrimarySurface::Dump(CDumpContext & dc) const
{
  CDDPrimarySurface::Dump(dc);
  dc << "Clipper window : " << m_pClipWindow << "  (if not null -> standard Windows mouse is used)\n";
  dc << "Mouse bitmap : " << m_pMouseBitmap << "\n";
  dc << "Mouse X : " << m_dwMouseX << "\n";
  dc << "Mouse Y : " << m_dwMouseY << "\n";
  dc << "Mouse visible : " << m_bVisible << "\n";
}

#endif

// Creates an instance of MousePrimarySurface an
// sets g_pMousePrimarySurface to it
// There can be only one instance of this object so it is legal
// the pClipWindow is pointer to the CWnd object of the window
// in wich is our primary surface (in fullscreen -> NULL)
BOOL CMousePrimarySurface::Init(CWnd *pClipWindow, DWORD dwBackBufferCount)
{
  ASSERT_VALID(g_pDirectDraw);
  ASSERT(g_pMousePrimarySurface == NULL);
  
  g_pMousePrimarySurface = new CMousePrimarySurface();
  g_pMousePrimarySurface->SetClipWindow(pClipWindow);
  if((dwBackBufferCount > 0) && (pClipWindow == NULL)){
    g_pMousePrimarySurface->SetCaps(DDSCAPS_COMPLEX | DDSCAPS_FLIP);
    g_pMousePrimarySurface->SetBackBufferCount(dwBackBufferCount);
  }
  g_pMousePrimarySurface->m_pClipWindow = pClipWindow;
  
  if(!g_pMousePrimarySurface->Create()){
    TRACE("Failed to create mouse primary surface.\n");
    ASSERT(FALSE);
    return FALSE;
  }

  return TRUE;
}

void CMousePrimarySurface::Close()
{
  if(g_pMousePrimarySurface != NULL){
    ASSERT_VALID(g_pMousePrimarySurface);

    g_pMousePrimarySurface->Delete();
    delete g_pMousePrimarySurface;
    g_pMousePrimarySurface = NULL;
  }
}

BOOL CMousePrimarySurface::PostCreate(LPDIRECTDRAWSURFACE lpDDSurface, BOOL bCallRelease)
{
  ASSERT_VALID(g_pDirectDraw);
  
  if(!CDDPrimarySurface::PostCreate(lpDDSurface, bCallRelease)) return FALSE;

  // Create a scratch surface for moveing
  m_MoveSurface.SetWidth(MPS_BUFFERWIDTH * 2);
  m_MoveSurface.SetHeight(MPS_BUFFERHEIGHT * 2);
  m_MoveSurface.SetVideoMemory(TRUE);
  if(!m_MoveSurface.Create()) return FALSE;

  // Create a surface to store the background of the mouse
  // what's under the mouse pointer
  m_MouseBackground.SetWidth(MPS_BUFFERWIDTH);
  m_MouseBackground.SetHeight(MPS_BUFFERHEIGHT);
  m_MouseBackground.SetVideoMemory(TRUE);
  if(!m_MouseBackground.Create()) return FALSE;

  m_bVisible = FALSE;
  
  return TRUE;
}

void CMousePrimarySurface::Delete()
{
  m_MoveSurface.Delete();
  m_MouseBackground.Delete();
  
  CDDPrimarySurface::Delete();
}

void CMousePrimarySurface::ShowMouse()
{
  ASSERT_VALID(this);

  // if in the window -> no action
  if(m_pClipWindow){
    return;
  }

  VERIFY(m_lockExclusive.Lock());

  try{

  if(m_bVisible == TRUE){
    TRACE("Not shown for still visible\n");
    VERIFY(m_lockExclusive.Unlock());
    return;
  }

  if(m_pMouseBitmap == NULL){
    VERIFY(m_lockExclusive.Unlock());
    return;
  }

  ASSERT_VALID(m_pMouseBitmap);

  ScanMouse();
  DrawMouse();

  m_bVisible = TRUE;
  }
  catch(...){
    VERIFY(m_lockExclusive.Unlock());
    throw;
  }

  VERIFY(m_lockExclusive.Unlock());
}

void CMousePrimarySurface::HideMouse()
{
  ASSERT_VALID(this);

  // if in the window -> no action
  if(m_pClipWindow){
    return;
  }

  VERIFY(m_lockExclusive.Lock());

  try{
  
  if(m_bVisible == FALSE){
    VERIFY(m_lockExclusive.Unlock());
    return;
  }

  ClearMouse();

  m_bVisible = FALSE;

  }
  catch(...){
    VERIFY(m_lockExclusive.Unlock());
    throw;
  }

  VERIFY(m_lockExclusive.Unlock());
}

// Draws the mouse pointer
void CMousePrimarySurface::DrawMouse()
{
  // if we are in the window -> no cursor
  if(m_pClipWindow){
    return;
  }

  CRect rect(m_dwMouseX - m_dwHotX, m_dwMouseY - m_dwHotY,
    m_dwMouseX - m_dwHotX + m_pMouseBitmap->GetWidth(),
    m_dwMouseY - m_dwHotY + m_pMouseBitmap->GetHeight());

  m_MoveSurface.BltFast(0, 0, g_pDDPrimarySurface, &rect, 0);
  m_MoveSurface.BltFast(0, 0, m_pMouseBitmap, m_pMouseBitmap->GetAllRect(), 0);
  g_pDDPrimarySurface->CDDPrimarySurface::BltFast(m_dwMouseX - m_dwHotX, m_dwMouseY - m_dwHotY,
    &m_MoveSurface, m_pMouseBitmap->GetAllRect(), 0);
}

// Reads the mouse background to the buffer
void CMousePrimarySurface::ScanMouse()
{
  // if we are in the window -> no sense of this action
  if(m_pClipWindow){
    return;
  }

  CRect rect(m_dwMouseX - m_dwHotX, m_dwMouseY - m_dwHotY,
    m_dwMouseX - m_dwHotX + m_pMouseBitmap->GetWidth(),
    m_dwMouseY - m_dwHotY + m_pMouseBitmap->GetHeight());
  m_MouseBackground.BltFast(0, 0, g_pDDPrimarySurface, &rect, 0);
}

// Clears the mouse pointer from the screen
void CMousePrimarySurface::ClearMouse()
{
  // if we are in the window -> no sense of this action
  if(m_pClipWindow){
    return;
  }

  g_pDDPrimarySurface->BltFast(m_dwMouseX - m_dwHotX, m_dwMouseY - m_dwHotY,
    &m_MouseBackground, m_pMouseBitmap->GetAllRect(), 0);
}

// Moves the mouse cursor ( no flashing )
void CMousePrimarySurface::MoveMouse(DWORD dwNewX, DWORD dwNewY)
{
  // if in window -> no action (Windows will do it for us)
  if(m_pClipWindow){
    return;
  }

  // the dwNewX and dwNewY has to be in the screen
  // no assertion for the speed reasons (even in debug version)
  
  // Test if we have to move it
  // if the position didn't changed we won't do anything
  if((dwNewX == m_dwMouseX) && (dwNewY == m_dwMouseY)) return;
  if(m_pMouseBitmap == NULL) return;

  CRect *pMouseRect;
  // get the mouse rect
  pMouseRect = m_pMouseBitmap->GetAllRect();

  CRect rectWholeOnDisplay; // Rect for the whole area which is affected by the move
  CRect rectOldCursor(pMouseRect); // Rect for the old cursor ( to be cleared )
  CRect rectNewCursor(pMouseRect); // Rect for the new cursor ( to be drawn )
  CRect rectWhole;
  CPoint ptTopLeft;

  // move the cursors to the display positions
  rectOldCursor.OffsetRect(m_dwMouseX - m_dwHotX, m_dwMouseY - m_dwHotY);
  rectNewCursor.OffsetRect(dwNewX - m_dwHotX, dwNewY - m_dwHotY);

  // union the rects to determine the whole area to be affected
  rectWholeOnDisplay.UnionRect(&rectOldCursor, &rectNewCursor);
  // if the cursor is too far from the old position we will use
  // another algorithm
  if((rectWholeOnDisplay.Width() > (pMouseRect->Width() * 2)) ||
    (rectWholeOnDisplay.Height() > (pMouseRect->Height() * 2))){
    // can use the simple hide-move-show
    // no flashing can appear ( the cursor moves too far )
    
    // hide the mouse
    ClearMouse();

    m_dwMouseX = dwNewX; m_dwMouseY = dwNewY;
    // show the mouse
    ScanMouse();
    DrawMouse();

    return;
  }

  rectWhole.SetRect(0, 0, rectWholeOnDisplay.Width(), rectWholeOnDisplay.Height());
  ptTopLeft = rectWholeOnDisplay.TopLeft();
  rectOldCursor.OffsetRect(-ptTopLeft.x, -ptTopLeft.y);
  rectNewCursor.OffsetRect(-ptTopLeft.x, -ptTopLeft.y);

  // copy the rect to the memory
  m_MoveSurface.BltFast(0, 0, this, &rectWholeOnDisplay);
  // erase the old cursor
  m_MoveSurface.BltFast(rectOldCursor.left, rectOldCursor.top, &m_MouseBackground, m_pMouseBitmap->GetAllRect());
  // hide the background of new cursor
  m_MouseBackground.BltFast(0, 0, &m_MoveSurface, &rectNewCursor);
  // draw the new cursor
  m_MoveSurface.BltFast(rectNewCursor.left, rectNewCursor.top, m_pMouseBitmap, m_pMouseBitmap->GetAllRect());
  // draw it to the display
  BltFast(ptTopLeft.x, ptTopLeft.y, &m_MoveSurface, &rectWhole);

  // update the cursor pos
  m_dwMouseX = dwNewX; m_dwMouseY = dwNewY;
}

// if the mouse cursor is above the blitted region
// we have to do some wierd stuff to disable the flashing
void CMousePrimarySurface::DoPaste(LONG lX, LONG lY, CDDrawSurface * pSrcSurface, CRect * pRectSource)
{
  // two ways
  // a) fullscreen -> graphical mouse
  // b) windowed -> standard Windows mouse
  
  if(m_pClipWindow == NULL){
    // our mouse -> care about the flickering of the cursor

    if((!m_bVisible) || (m_pMouseBitmap == NULL)){
      BltFast(lX, lY, pSrcSurface, pRectSource);
      return;
    }
    
    if(pRectSource == NULL) pRectSource = pSrcSurface->GetAllRect();
    
    VERIFY(m_lockExclusive.Lock());
    
    try{
      
      // rect on screen where is the mouse
      CRect rectScrMouse(m_pMouseBitmap->GetAllRect());
      rectScrMouse.OffsetRect(m_dwMouseX - m_dwHotX, m_dwMouseY - m_dwHotY);
      
      // rect on screen which is affected by mouse
      CRect rectDest(lX, lY, lX + pRectSource->Width(), lY + pRectSource->Height());
      
      rectDest.IntersectRect(&rectDest, &rectScrMouse);
      if(rectDest.IsRectEmpty()){  // if the mouse is not above it just blit it
        BltFast(lX, lY, pSrcSurface, pRectSource);
        VERIFY(m_lockExclusive.Unlock());
        return;
      }
      
      CRect rectBackground(&rectDest);  // rect in MouseBackground to update
      rectBackground.OffsetRect(-rectScrMouse.left, -rectScrMouse.top);
      
      CRect rectSource(&rectDest);  // rect in SrcSurface which is affected by mouse
      rectSource.OffsetRect(pRectSource->left - (LONG)lX,
        pRectSource->top - (LONG)lY);
      
      // update the mouse background from the SrcSurface
      m_MouseBackground.BltFast(rectBackground.left, rectBackground.top,
        pSrcSurface, &rectSource);
      
      // draw the mouse to the SrcSurface
      pSrcSurface->BltFast(rectSource.left, rectSource.top,
        m_pMouseBitmap, &rectBackground);
      
      // draw the SrcSurface to the screen
      BltFast(lX, lY, pSrcSurface, pRectSource);
      
      // remove the mouse from SrcSurface
      pSrcSurface->BltFast(rectSource.left, rectSource.top,
        &m_MouseBackground, &rectBackground);
      
    }
    catch(...){
      VERIFY(m_lockExclusive.Unlock());
      throw;
    }
    
    VERIFY(m_lockExclusive.Unlock());
    
  }
  else{
    // normal Windows mouse -> don't care about the cursor
    CDDPrimarySurface::DoPaste(lX, lY, pSrcSurface, pRectSource);
  }
}

BOOL CMousePrimarySurface::Flip()
{
  ASSERT_VALID(this);
  ASSERT(m_pClipWindow == NULL); // available only in fullscreen

  BOOL bRet;

  VERIFY(m_lockExclusive.Lock());

  try{  

  if((!m_bVisible) || (m_pMouseBitmap == NULL)){
    bRet = CDDPrimarySurface::Flip();
    VERIFY(m_lockExclusive.Unlock());
    return bRet;
  }

  ASSERT_VALID(m_pMouseBitmap);

  // Compute the rect of the mouse on screen
  // in back buffer the rect is the same
  CRect rectMouse(m_pMouseBitmap->GetAllRect());
  rectMouse.OffsetRect(m_dwMouseX - m_dwHotX, m_dwMouseY - m_dwHotY);

  // copy the background of the mouse in the back buffer
  m_MoveSurface.BltFast(0, 0, &m_BackBuffer, &rectMouse);
  // draw the mouse to the back buffer
  m_BackBuffer.BltFast(rectMouse.left, rectMouse.top,
    m_pMouseBitmap, m_pMouseBitmap->GetAllRect());

  // flip the surfaces
  bRet = CDDPrimarySurface::Flip();

  // Hide the mouse in the backbuffer (the new one)
  m_BackBuffer.BltFast(rectMouse.left, rectMouse.top,
    &m_MouseBackground, m_pMouseBitmap->GetAllRect());
  // Copy the new background to the m_MouseBackground
  m_MouseBackground.BltFast(0, 0, &m_MoveSurface,
    m_pMouseBitmap->GetAllRect());

  }
  catch(...){
    VERIFY(m_lockExclusive.Unlock());
    throw;
  }

  VERIFY(m_lockExclusive.Unlock());

  return bRet;
}

void CMousePrimarySurface::Restore()
{
  CDDPrimarySurface::Restore();
}
