// MinimapWindow.cpp: implementation of the CMinimapWindow class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "MinimapWindow.h"

#include "GameClient\Common\Colors.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

#define MINIMAP_MAXSIZE_X 200
#define MINIMAP_MAXSIZE_Y 200

#define MINIMAP_PIECE_WIDTH 256
#define MINIMAP_PIECE_HEIGHT 256

#define MINIMAP_VIEWRECT_COLOR RGB32(255, 255, 255)

IMPLEMENT_DYNAMIC(CMinimapWindow, CWindow)

BEGIN_OBSERVER_MAP(CMinimapWindow, CWindow)
  BEGIN_MOUSE()
    ON_MOUSEMOVE()
    ON_LBUTTONDOWN()
    ON_LBUTTONUP()
    ON_RBUTTONDOWN()
    ON_RBUTTONUP()
  END_MOUSE()

  BEGIN_NOTIFIER(ID_MiniMapClip)
    EVENT(CCMiniMapClip::E_ClipChanged)
      OnClipChanged(); return FALSE;
  END_NOTIFIER()
END_OBSERVER_MAP(CMinimapWindow, CWindow)

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CMinimapWindow::CMinimapWindow()
{
  m_dwZoom = 2;
  m_dwPositionX = 0;
  m_dwPositionY = 0;

  m_dwBackgroundWidth = 0;
  m_dwBackgroundHeight = 0;
  m_dwBackgroundPositionX = 0;
  m_dwBackgroundPositionY = 0;

  m_pMiniMapClip = NULL;

  m_pMap = NULL;

  m_eDragState = Drag_None;
}

CMinimapWindow::~CMinimapWindow()
{
  ASSERT(m_dwBackgroundWidth == 0);
  ASSERT(m_dwBackgroundHeight == 0);

  ASSERT(m_pMiniMapClip == NULL);

  ASSERT(m_pMap == NULL);
}

#ifdef _DEBUG

void CMinimapWindow::AssertValid() const
{
  CWindow::AssertValid();

  ASSERT(m_dwBackgroundWidth > 0);
  ASSERT(m_dwBackgroundHeight > 0);

  ASSERT((m_dwZoom == 2) || (m_dwZoom == 4) || (m_dwZoom == 8) || (m_dwZoom == 16));

  ASSERT(m_pMiniMapClip != NULL);

  ASSERT(m_pMap != NULL);
}

void CMinimapWindow::Dump(CDumpContext &dc) const
{
  CWindow::Dump(dc);
}

#endif


void CMinimapWindow::Create(CRect &rcRect, CWindow *pParent, CCMap *pMap, CViewport *pViewport, CCMiniMapClip *pMiniMapClip)
{
  ASSERT(pMap != NULL);
  ASSERT(pViewport != NULL);
  ASSERT(pMiniMapClip != NULL);

  VERIFY(m_lockExclusive.Lock());

  // Compute the size of the background bitmap
  m_dwBackgroundWidth = MINIMAP_MAXSIZE_X / MINIMAP_PIECE_WIDTH + 1;
  if(m_dwBackgroundWidth <= 1) m_dwBackgroundWidth = 2;
  m_dwBackgroundHeight = MINIMAP_MAXSIZE_Y / MINIMAP_PIECE_HEIGHT + 1;
  if(m_dwBackgroundHeight <= 1) m_dwBackgroundHeight = 2;

  // Set default position of the minimap
  m_dwPositionX = 0;
  m_dwPositionY = 0;
  m_dwBackgroundPositionX = 0;
  m_dwBackgroundPositionY = 0;
  m_dwZoom = 2;

  // Copy the minimap clip
  m_pMiniMapClip = pMiniMapClip;
  m_pMiniMapClip->RequestClip(m_dwPositionX, m_dwPositionY,
    MINIMAP_MAXSIZE_X * m_dwZoom, MINIMAP_MAXSIZE_Y * m_dwZoom, m_dwZoom);
  m_pMiniMapClip->Connect(this, ID_MiniMapClip);

  // Copy the map pointer
  m_pMap = pMap;

  // Create the color array for civilizations
  {
    DWORD dwCivilization;
    DWORD dwCount = m_pMap->GetCivilizationCount();
    DWORD dwColor;
    int r, g, b;
    double h, s, v;
    for(dwCivilization = 0; dwCivilization < dwCount; dwCivilization++){
      dwColor = m_pMap->GetCivilizationByIndex(dwCivilization)->GetColor();
      Color_RGB2HSV(R32(dwColor), G32(dwColor), B32(dwColor), h, s, v);
      v += 0.25; if(v > 1) v = 1;
      Color_HSV2RGB(h, s, v, r, g, b);
      dwColor = RGB32(r, g, b);
      m_aCivilizationColors[dwCivilization] = dwColor;
    }
  }

  // Create the position observer
  m_PositionObserver.Create(this, pViewport);

  // Create the background bitmap surface
  m_Background.SetWidth(m_dwBackgroundWidth * MINIMAP_PIECE_WIDTH);
  m_Background.SetHeight(m_dwBackgroundHeight * MINIMAP_PIECE_HEIGHT);
  m_Background.Create();

  // Load the background (first time)
  {
    DWORD x, y;
    for(y = 0; y < m_dwBackgroundHeight; y++){
      for(x = 0; x < m_dwBackgroundWidth; x++){
        LoadBackgroundPiece(m_dwBackgroundPositionX + x, m_dwBackgroundPositionY + y, x, y);
      }
    }
  }

  m_bTransparent = FALSE;
  m_bDoubleClk = TRUE;

  // Create the window
  CWindow::Create(&rcRect, pParent);

  VERIFY(m_lockExclusive.Unlock());
}

void CMinimapWindow::Delete()
{
  VERIFY(m_lockExclusive.Lock());

  if(m_eDragState != Drag_None){
    ReleaseCapture();
  }

  CWindow::Delete();

  m_dwZoom = 2;
  m_dwPositionX = 0;
  m_dwPositionY = 0;

  m_dwBackgroundWidth = 0;
  m_dwBackgroundHeight = 0;
  m_dwBackgroundPositionX = 0;
  m_dwBackgroundPositionY = 0;
  m_Background.Delete();

  m_PositionObserver.Delete();

  if(m_pMiniMapClip != NULL){
    m_pMiniMapClip->Disconnect(this);
    m_pMiniMapClip = NULL;
  }

  m_pMap = NULL;

  VERIFY(m_lockExclusive.Unlock());
}

void CMinimapWindow::Draw(CDDrawSurface *pSurface, CRect *pRect)
{
  VERIFY(m_lockExclusive.Lock());

  ASSERT_VALID(this);

  CRect rcRect, rcPos;
  rcPos = GetWindowPosition();
  rcRect.SetRect(0, 0, rcPos.Width(), rcPos.Height());

  // Before everything -> clear the background
  pSurface->Fill(0, &rcRect);

  // First draw the background
  {
    CRect rcSource;
    rcSource.left = (m_dwPositionX - (m_dwBackgroundPositionX * MINIMAP_PIECE_WIDTH * m_dwZoom)) / m_dwZoom;
    rcSource.top = (m_dwPositionY - (m_dwBackgroundPositionY * MINIMAP_PIECE_WIDTH * m_dwZoom)) / m_dwZoom;
    rcSource.right = rcSource.left + rcRect.Width();
    rcSource.bottom = rcSource.top + rcRect.Height();

    rcSource.IntersectRect(&rcSource, m_Background.GetAllRect());

    pSurface->Paste(0, 0, &m_Background, &rcSource);
  }

  // Here draw units
  {
    // Lock the clip to view its contents
    m_pMiniMapClip->LockClip();

    // If the zoom is different -> sorry no units
    if(m_pMiniMapClip->IsCorrectZoom()){
      // Yes the right zoom

      // Get the clip
      SMiniMapClip sClip = m_pMiniMapClip->GetClip();
      DWORD dwScanLineSize = m_pMiniMapClip->GetScanLineSize();

      CRect rcClip;
      rcClip.left = (sClip.nLeft - (int)m_dwPositionX) / (int)m_dwZoom;
      rcClip.top = (sClip.nTop - (int)m_dwPositionY) / (int)m_dwZoom;
      rcClip.right = rcClip.left + sClip.nWidth / (int)m_dwZoom;
      rcClip.bottom = rcClip.top + sClip.nHeight / (int)m_dwZoom;

      CRect rcIntersect;
      rcIntersect.IntersectRect(&rcRect, &rcClip);

      if(rcIntersect.IsRectEmpty()) goto NoUnitToDraw;

      // Alloc the buffer
      BYTE pSourceBuffer[cMiniMapMaxWidth];
      DWORD dwSourceBufferChunk;

      LPBYTE pData;
      DWORD dwPitch;
      // Lock the surface
      {
        CRect rcLock;
        rcLock.left = pSurface->GetTransformation().x + rcIntersect.left;
        rcLock.top = pSurface->GetTransformation().y + rcIntersect.top;
        rcLock.right = rcLock.left + rcIntersect.Width();
        rcLock.bottom = rcLock.top + rcIntersect.Height();

        pData = (LPBYTE)(pSurface->Lock(&rcLock, 0, dwPitch));
      }

      // Start the data flow
      m_pMiniMapClip->StartDataReading();

      // Seek to the start
      if(sClip.nTop < (int)m_dwPositionY){
        int nLine;
        for(nLine = 0; nLine < (rcIntersect.top - rcClip.top); nLine++)
        {
          dwSourceBufferChunk = dwScanLineSize;
          m_pMiniMapClip->ReadData(pSourceBuffer, dwSourceBufferChunk);
          ASSERT(dwSourceBufferChunk == dwScanLineSize);
        }
      }
      if(sClip.nLeft < (int)m_dwPositionX){
        dwSourceBufferChunk = rcIntersect.left - rcClip.left;
        m_pMiniMapClip->ReadData(pSourceBuffer, dwSourceBufferChunk);
        ASSERT((int)dwSourceBufferChunk == rcIntersect.left - rcClip.left);
      }

      DWORD dwXDest, dwYDest, dwDOff;
      DWORD dwWidth = rcIntersect.Width();
      DWORD dwHeight = rcIntersect.Height();

      BYTE nCiv;
      DWORD dwColor;

      for(dwYDest = 0; dwYDest < dwHeight; dwYDest++){
        // Read one line
        dwSourceBufferChunk = dwScanLineSize;
        m_pMiniMapClip->ReadData(pSourceBuffer, dwSourceBufferChunk);

        dwDOff = dwYDest * dwPitch;
        if(g_pDirectDraw->Is32BitMode()){
          for(dwXDest = 0; dwXDest < dwWidth; dwXDest++, dwDOff += 4){
            nCiv = pSourceBuffer[dwXDest];
            if(nCiv == 0xFF) continue;
            dwColor = m_aCivilizationColors[(DWORD)nCiv];

            pData[dwDOff] = ((BYTE*)(&dwColor))[0];
            pData[dwDOff + 1] = ((BYTE*)(&dwColor))[1];
            pData[dwDOff + 2] = ((BYTE*)(&dwColor))[2];
          }
        }
        else{
          for(dwXDest = 0; dwXDest < dwWidth; dwXDest++, dwDOff += 3){
            nCiv = pSourceBuffer[dwXDest];
            if(nCiv == 0xFF) continue;
            dwColor = m_aCivilizationColors[(DWORD)nCiv];

            pData[dwDOff] = ((BYTE*)(&dwColor))[0];
            pData[dwDOff + 1] = ((BYTE*)(&dwColor))[1];
            pData[dwDOff + 2] = ((BYTE*)(&dwColor))[2];
          }
        }
      }

      // Stop the data
      m_pMiniMapClip->CancelDataReading();

      // Unlock the surface
      pSurface->Unlock(pData);
    }

NoUnitToDraw:
    ;

    // Unlock the clip
    m_pMiniMapClip->UnlockClip();
  }

  // Now draw the viewport rectangle
  {
    CRect rc;
    rc.left = (m_rcViewRect.left - (int)m_dwPositionX) / (int)m_dwZoom;
    rc.top = (m_rcViewRect.top - (int)m_dwPositionY) / (int)m_dwZoom;
    rc.right = rc.left + m_rcViewRect.Width() / (int)m_dwZoom;
    rc.bottom = rc.top + m_rcViewRect.Height() / (int)m_dwZoom;

    CRect rcFill;
    rcFill.SetRect(rc.left, rc.top, rc.right, rc.top + 1);
    pSurface->Fill(MINIMAP_VIEWRECT_COLOR, &rcFill);

    rcFill.SetRect(rc.left, rc.top, rc.left + 1, rc.bottom);
    pSurface->Fill(MINIMAP_VIEWRECT_COLOR, &rcFill);

    rcFill.SetRect(rc.right, rc.top, rc.right + 1, rc.bottom + 1);
    pSurface->Fill(MINIMAP_VIEWRECT_COLOR, &rcFill);

    rcFill.SetRect(rc.left, rc.bottom, rc.right, rc.bottom + 1);
    pSurface->Fill(MINIMAP_VIEWRECT_COLOR, &rcFill);
  }

  VERIFY(m_lockExclusive.Unlock());
}

void CMinimapWindow::LoadBackgroundPiece(DWORD dwX, DWORD dwY, DWORD dwBkgPosX, DWORD dwBkgPosY)
{
  // Determine if the piece is on the map (it means if we can find it in the archive)
  if((dwX * MINIMAP_PIECE_WIDTH * m_dwZoom >= m_pMap->GetWidthInCells()) ||
    (dwY * MINIMAP_PIECE_HEIGHT * m_dwZoom >= m_pMap->GetHeightInCells())){
    // Not on the map -> just clear it
    CRect rcFill;
    rcFill.left = dwBkgPosX * MINIMAP_PIECE_WIDTH;
    rcFill.top = dwBkgPosY * MINIMAP_PIECE_HEIGHT;
    rcFill.right = rcFill.left + MINIMAP_PIECE_WIDTH;
    rcFill.bottom = rcFill.top + MINIMAP_PIECE_HEIGHT;
    m_Background.Fill(0, &rcFill);
    return;
  }

  CString strFileName;
  // make the filename
  strFileName.Format("MiniMaps\\%d\\MiniMap%02d%04d%04d.raw", m_dwZoom, m_dwZoom, dwX, dwY);

  CArchiveFile file;
  // open the file
  file = m_pMap->GetMapArchive().CreateFile(strFileName);

  // load it
  BYTE *pBuffer = new BYTE[MINIMAP_PIECE_WIDTH * MINIMAP_PIECE_HEIGHT * 3];
  try{
    file.Read(pBuffer, MINIMAP_PIECE_WIDTH * MINIMAP_PIECE_HEIGHT * 3);

    DWORD x, y;
    DWORD dwSOff, dwDOff;
    DWORD dwPitch;
    BYTE *pDest = (LPBYTE)m_Background.Lock(m_Background.GetAllRect(), 0, dwPitch);

    if(g_pDirectDraw->Is32BitMode()){
      for(y = 0; y < MINIMAP_PIECE_HEIGHT; y++){
        for(x = 0; x < MINIMAP_PIECE_WIDTH; x++){
          dwSOff = (y * MINIMAP_PIECE_WIDTH + x) * 3;
          dwDOff = (y + dwBkgPosY * MINIMAP_PIECE_HEIGHT) * dwPitch + (x + dwBkgPosX * MINIMAP_PIECE_WIDTH) * 4;

          // We must exchange the order to BGR
          pDest[dwDOff] = pBuffer[dwSOff + 2];
          pDest[dwDOff + 1] = pBuffer[dwSOff + 1];
          pDest[dwDOff + 2] = pBuffer[dwSOff];
        }
      }
    }
    else{
      for(y = 0; y < MINIMAP_PIECE_HEIGHT; y++){
        for(x = 0; x < MINIMAP_PIECE_WIDTH; x++){
          dwSOff = (y * MINIMAP_PIECE_WIDTH + x) * 3;
          dwDOff = (y + dwBkgPosY * MINIMAP_PIECE_HEIGHT) * dwPitch + (x + dwBkgPosX * MINIMAP_PIECE_WIDTH) * 3;

          // We must exchange the order to BGR
          pDest[dwDOff] = pBuffer[dwSOff + 2];
          pDest[dwDOff + 1] = pBuffer[dwSOff + 1];
          pDest[dwDOff + 2] = pBuffer[dwSOff];
        }
      }
    }

    m_Background.Unlock(pDest);
  }
  catch(CException *){
    delete pBuffer;
    throw;
  }
  delete pBuffer;
}

void CMinimapWindow::SetPosition(DWORD dwXPosition, DWORD dwYPosition)
{
  VERIFY(m_lockExclusive.Lock());

  ASSERT_VALID(this);
  ASSERT(dwXPosition < m_pMap->GetWidthInCells());
  ASSERT(dwYPosition < m_pMap->GetHeightInCells());

  // If the position stays the same do nothing
  if((m_dwPositionX == dwXPosition) && (m_dwPositionY == dwYPosition)){
    VERIFY(m_lockExclusive.Unlock());
    return;
  }

  DWORD dwMask = m_dwZoom - 1;
  dwXPosition &= ~dwMask;
  dwYPosition &= ~dwMask;

  // Request new clip from the server
  m_pMiniMapClip->RequestClip(dwXPosition, dwYPosition, 
    MINIMAP_MAXSIZE_X * m_dwZoom, MINIMAP_MAXSIZE_Y * m_dwZoom, m_dwZoom);

  // Copy new position
  m_dwPositionX = dwXPosition;
  m_dwPositionY = dwYPosition;

  // Compute current bkg rectangle
  CRect rcBkg;
  rcBkg.SetRect(m_dwBackgroundPositionX, m_dwBackgroundPositionY,
    m_dwBackgroundPositionX + m_dwBackgroundWidth,
    m_dwBackgroundPositionY + m_dwBackgroundHeight);

  // Compute new bkg rectangle
  CRect rcNewBkg;
  rcNewBkg.SetRect((dwXPosition / m_dwZoom) / MINIMAP_PIECE_WIDTH,
    (dwYPosition / m_dwZoom) / MINIMAP_PIECE_HEIGHT,
    (dwXPosition / m_dwZoom) / MINIMAP_PIECE_WIDTH + m_dwBackgroundWidth,
    (dwYPosition / m_dwZoom) / MINIMAP_PIECE_HEIGHT + m_dwBackgroundHeight);

  // If the rects are the same -> do nothing
  if(!rcNewBkg.EqualRect(&rcBkg)){
    // Set new background position
    m_dwBackgroundPositionX = rcNewBkg.left;
    m_dwBackgroundPositionY = rcNewBkg.top;

    // Load all pieces
    DWORD dwX, dwY;
    for(dwY = 0; dwY < m_dwBackgroundHeight; dwY++){
      for(dwX = 0; dwX < m_dwBackgroundWidth; dwX++){
        LoadBackgroundPiece(m_dwBackgroundPositionX + dwX, m_dwBackgroundPositionY + dwY,
          dwX, dwY);
      }
    }
  }

  VERIFY(m_lockExclusive.Unlock());

  // Force updating of the window
  UpdateRect(NULL);
}

void CMinimapWindow::SetZoom(DWORD dwZoom)
{
  VERIFY(m_lockExclusive.Lock());

  ASSERT_VALID(this);
  ASSERT((dwZoom == 2) || (dwZoom == 4) || (dwZoom == 8) || (dwZoom == 16));

  // Request new clip from server
  m_pMiniMapClip->RequestClip(m_dwPositionX, m_dwPositionY,
    MINIMAP_MAXSIZE_X * dwZoom, MINIMAP_MAXSIZE_Y * dwZoom, dwZoom);
  
  // Copy the new zoom
  m_dwZoom = dwZoom;

  DWORD dwMask = m_dwZoom - 1;
  m_dwPositionX &= ~dwMask;
  m_dwPositionY &= ~dwMask;

  // Compute the new background position
  m_dwBackgroundPositionX = (m_dwPositionX / m_dwZoom) / MINIMAP_PIECE_WIDTH;
  m_dwBackgroundPositionY = (m_dwPositionY / m_dwZoom) / MINIMAP_PIECE_HEIGHT;

  // Reload all pieces
  m_Background.Fill(0);
  DWORD dwX, dwY;
  for(dwY = 0; dwY < m_dwBackgroundHeight; dwY++){
    for(dwX = 0; dwX < m_dwBackgroundWidth; dwX++){
      LoadBackgroundPiece(m_dwBackgroundPositionX + dwX, m_dwBackgroundPositionY + dwY,
        dwX, dwY);
    }
  }

  VERIFY(m_lockExclusive.Unlock());

  UpdateRect();
}

void CMinimapWindow::OnMouseMove(CPoint pt)
{
  switch(m_eDragState){
  case Drag_ViewRect:
    SetMouseViewRect(pt);
    break;

  case Drag_Minimap:
    // Compute the move from the start position
    int nNewPosX, nNewPosY;
    nNewPosX = ((m_ptMinimapDragStart.x - pt.x) * m_dwZoom) + m_dwMinimapDragStartX;
    nNewPosY = ((m_ptMinimapDragStart.y - pt.y) * m_dwZoom) + m_dwMinimapDragStartY;

    CRect rcWnd;
    rcWnd = GetWindowPosition();

    if((nNewPosX + rcWnd.Width() * m_dwZoom) > m_pMap->GetWidthInCells())
      nNewPosX = m_pMap->GetWidthInCells() - rcWnd.Width() * m_dwZoom;
    if((nNewPosY + rcWnd.Height() * m_dwZoom) > m_pMap->GetHeightInCells())
      nNewPosY = m_pMap->GetHeightInCells() - rcWnd.Height() * m_dwZoom;
    if(nNewPosX < 0) nNewPosX = 0;
    if(nNewPosY < 0) nNewPosY = 0;

    // Set new position
    SetPosition(nNewPosX, nNewPosY);

    break;
  }
}

void CMinimapWindow::OnLButtonDown(CPoint pt)
{
  if(m_eDragState != Drag_None) return;

  SetMouseViewRect(pt);
  m_eDragState = Drag_ViewRect;
  SetCapture();
}

void CMinimapWindow::OnLButtonUp(CPoint pt)
{
  if(m_eDragState != Drag_ViewRect) return;

  ReleaseCapture();
  m_eDragState = Drag_None;
}

void CMinimapWindow::OnRButtonDown(CPoint pt)
{
  if(m_eDragState != Drag_None) return;

  m_ptMinimapDragStart = pt;
  m_dwMinimapDragStartX = m_dwPositionX;
  m_dwMinimapDragStartY = m_dwPositionY;
  m_eDragState = Drag_Minimap;
  SetCapture();
}

void CMinimapWindow::OnRButtonUp(CPoint pt)
{
  if(m_eDragState != Drag_Minimap) return;

  ReleaseCapture();
  m_eDragState = Drag_None;
}

void CMinimapWindow::CMinimapViewportPositionObserver::OnPositionChanged()
{
  // Determine the viewport rectangle in mapcells
  CRect rcViewRect;
  DWORD dwX, dwY;
  GetViewport()->GetPosition(dwX, dwY);
  CSize size = GetViewport()->GetSize();
  rcViewRect.left = dwX / MAPCELL_WIDTH;
  rcViewRect.top = dwY / MAPCELL_HEIGHT;
  rcViewRect.right = rcViewRect.left + size.cx;
  rcViewRect.bottom = rcViewRect.top + size.cy;

  // Set new position of the view rect
  m_pMinimapWindow->SetViewRect(rcViewRect);
}

void CMinimapWindow::SetViewRect(CRect &rcViewRect)
{
  // Lock us
  VERIFY(m_lockExclusive.Lock());

  // Just clip it into the map
  CRect rcMap(0, 0, m_pMap->GetWidthInCells(), m_pMap->GetHeightInCells());
  m_rcViewRect.IntersectRect(&rcViewRect, &rcMap);

  // Unlock
  VERIFY(m_lockExclusive.Unlock());

  // Update the window
  UpdateRect(NULL);
}

void CMinimapWindow::SetMouseViewRect(CPoint &pt)
{
  // Compute new position
  int nNewX, nNewY;
  CSize sz;

  sz = m_PositionObserver.GetViewport()->GetSize();
  nNewX = (int)m_dwPositionX + (pt.x * (int)m_dwZoom) - (sz.cx / 2);
  nNewY = (int)m_dwPositionY + (pt.y * (int)m_dwZoom) - (sz.cy / 2);

  // And than set new viewport position
  m_PositionObserver.GetViewport()->SetPosition(nNewX * MAPCELL_WIDTH, nNewY * MAPCELL_HEIGHT);
}

// The clip from the server with units has changed
void CMinimapWindow::OnClipChanged()
{
  // Just redraw our window
  UpdateRect();
}