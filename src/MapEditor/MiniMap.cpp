// MiniMap.cpp: implementation of the CMiniMap class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "MiniMap.h"

#include "DataObjects\EMap.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

#define GBUFFER_WIDTH 50
#define GBUFFER_HEIGHT 50
#define MBUFFER_WIDTH 128
#define MBUFFER_HEIGHT 128

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CMiniMap::CMiniMap()
: m_semaphoreRectInlayed(0, 10000)
{
  m_pMap = NULL;
  m_dwZoom = 1; m_dwWantedZoom = 1;
  m_pDrawWnd = NULL;
  m_bExit = FALSE;
}

CMiniMap::~CMiniMap()
{
}

UINT CMiniMap::MiniMapThreadProc(LPVOID pParam)
{
  CMiniMap *pThis = (CMiniMap *)pParam;
  CDDrawSurface *pMiniMapSurface;

  CRect rcCompute;

  while(1){
    pThis->m_semaphoreRectInlayed.Lock();
    if(pThis->m_bExit){
      return 0;
    }

    if(pThis->m_pMap == NULL) continue;
    pThis->m_pMap->LockMiniMapMove();

    CPoint ptTL;
    pThis->m_semaphoreUpdateList.Lock();
    if(pThis->m_UpdateList.GetFirstRectNode() == NULL){
      pThis->m_semaphoreUpdateList.Unlock();
      pThis->m_pMap->UnlockMiniMapMove();
      continue;
    }
    rcCompute = pThis->m_UpdateList.GetFirstRectNode()->m_rcRect;
    pThis->m_UpdateList.RemoveFirstNode();
    rcCompute.IntersectRect(&rcCompute, &(pThis->m_pMap->GetMiniMapRect()));
    pThis->m_dwZoom = pThis->m_dwWantedZoom;
    ptTL = pThis->m_pMap->GetMiniMapRect().TopLeft();
    pThis->m_semaphoreUpdateList.Unlock();

    if(rcCompute.IsRectEmpty()){
      pThis->m_pMap->UnlockMiniMapMove();
      continue;
    }

    int nXLeft, nYLeft;
    CRect rcPart;
    CRect rcBufPart(0, 0, 0, 0);
    CSize szPart;
    CPoint ptPos;
    szPart.cx = MBUFFER_WIDTH;
    szPart.cx -= szPart.cx % pThis->m_dwZoom;
    if((szPart.cx / pThis->m_dwZoom) > GBUFFER_WIDTH) szPart.cx = GBUFFER_WIDTH * pThis->m_dwZoom;
    szPart.cy = MBUFFER_HEIGHT;
    szPart.cy -= szPart.cy % pThis->m_dwZoom;
    if((szPart.cy / pThis->m_dwZoom) > GBUFFER_HEIGHT) szPart.cy = GBUFFER_HEIGHT * pThis->m_dwZoom;

    nYLeft = rcCompute.Height();
    rcPart.top = rcCompute.top;
    ptPos.y = (rcCompute.top - ptTL.y) / pThis->m_dwZoom;
    rcBufPart.right = szPart.cx / pThis->m_dwZoom;
    rcBufPart.bottom = szPart.cy / pThis->m_dwZoom;
    while(nYLeft > 0){
      if(nYLeft >= szPart.cy){
        rcPart.bottom = rcPart.top + szPart.cy;
      }
      else{
        rcPart.bottom = rcPart.top + nYLeft;
      }
      rcBufPart.bottom = rcPart.Height() / pThis->m_dwZoom;

      nXLeft = rcCompute.Width();
      rcPart.left = rcCompute.left;
      ptPos.x = (rcCompute.left - ptTL.x) / pThis->m_dwZoom;
      while(nXLeft > 0){
        if(nXLeft >= szPart.cx){
          rcPart.right = rcPart.left + szPart.cx;
        }
        else{
          rcPart.right = rcPart.left + nXLeft;
        }
        rcBufPart.right = rcPart.Width() / pThis->m_dwZoom;

        pThis->ComputePiece(rcPart);

        pThis->m_pMap->GetExclusiveAccess();
        pMiniMapSurface = pThis->m_pMap->GetMiniMap();
        pMiniMapSurface->Paste(ptPos, &(pThis->m_MiniMapBuffer), &rcBufPart);
        pThis->m_pMap->ReleaseMiniMap();
        pThis->m_pMap->ReleaseExclusiveAccess();

        nXLeft -= rcPart.Width();
        ptPos.x += rcPart.Width() / pThis->m_dwZoom;
        rcPart.left += rcPart.Width();
      }

      nYLeft -= rcPart.Height();
      ptPos.y += rcPart.Height() / pThis->m_dwZoom;
      rcPart.top += rcPart.Height();
    }

    if(pThis->m_pDrawWnd != NULL){
      pThis->m_pDrawWnd->InvalidateRect(NULL);
    }
    pThis->m_pMap->UnlockMiniMapMove();
  }

  return 0;
}

BOOL CMiniMap::Create()
{
  m_pMiniMapThread = AfxBeginThread(CMiniMap::MiniMapThreadProc, (LPVOID)this, THREAD_PRIORITY_BELOW_NORMAL,
    0, NULL);
  m_pMiniMapThread->m_bAutoDelete = FALSE;

  m_MiniMapBuffer.SetWidth(GBUFFER_WIDTH);
  m_MiniMapBuffer.SetHeight(GBUFFER_HEIGHT);
  m_MiniMapBuffer.Create();

  m_pLandTypeBuffer = new BYTE[MBUFFER_WIDTH * MBUFFER_HEIGHT];

  m_UpdateList.Create(0.90, 0.90);

  m_pMap = NULL;

  return TRUE;
}

void CMiniMap::Delete()
{
  m_bExit = TRUE;
  m_semaphoreRectInlayed.Unlock();

  if(WaitForSingleObject((HANDLE)(*m_pMiniMapThread), 2000) != WAIT_OBJECT_0){
    TRACE("Terminating thread of minimap.");
    TerminateThread((HANDLE)(*m_pMiniMapThread), 1);
  }

  delete m_pMiniMapThread;

  m_UpdateList.Delete();

  delete m_pLandTypeBuffer;

  m_MiniMapBuffer.Delete();
  m_pMap = NULL;
}

DWORD CMiniMap::GetZoom()
{
  return m_dwWantedZoom;
}

void CMiniMap::SetZoom(DWORD dwZoom)
{
  m_semaphoreUpdateList.Lock();
  m_dwWantedZoom = dwZoom;
  m_UpdateList.RemoveAll();
  // down the semaphore to 0
  while(WaitForSingleObject((HANDLE)m_semaphoreRectInlayed, 0) == WAIT_OBJECT_0);
  CRect rcAll(0, 0, 0, 0);
  if(m_pMap != NULL){
    rcAll.right = m_pMap->GetWidth(); rcAll.bottom = m_pMap->GetHeight();
    // Clear the minimap graphics buffer
    CDDrawSurface *pMiniMap = m_pMap->GetMiniMap();
    pMiniMap->Fill(0);
    m_pMap->ReleaseMiniMap();
  }
  m_MiniMapBuffer.Fill(0);
  m_UpdateList.AddRect(&rcAll);
  m_semaphoreRectInlayed.Unlock();
  m_semaphoreUpdateList.Unlock();



  if(m_pDrawWnd != NULL){
    CString str;
    // invalidate all the window
    m_pDrawWnd->InvalidateRect(NULL);
    str.Format("MiniMap %dx", m_dwWantedZoom);
    m_pDrawWnd->SetWindowText(str);
  }
}

void CMiniMap::SetMap(CEMap *pMap)
{
  m_semaphoreUpdateList.Lock();
  m_pMap = pMap;

  m_UpdateList.RemoveAll();
  // down the semaphore to 0
  while(WaitForSingleObject((HANDLE)m_semaphoreRectInlayed, 0) == WAIT_OBJECT_0);
  CRect rcAll(0, 0, 0, 0);
  if(m_pMap != NULL){
    rcAll.right = m_pMap->GetWidth(); rcAll.bottom = m_pMap->GetHeight();
  }
  m_UpdateList.AddRect(&rcAll);
  m_semaphoreRectInlayed.Unlock();
  m_semaphoreUpdateList.Unlock();
}

void CMiniMap::ComputePiece(CRect rcPiece)
{
  DWORD dwWidth, dwHeight;

  dwWidth = rcPiece.Width() / m_dwZoom;
  dwHeight = rcPiece.Height() / m_dwZoom;

  // get exclusive access to the map
  // it means no one can delete it or so
  m_pMap->GetExclusiveAccess();

  // first get all squares than are interesting
  CTypedPtrList<CPtrList, CEMapSquare *> listSquares;
  CEMapSquare *pSquare;
  int x, y;
  for(y = rcPiece.top - MAPSQUARE_HEIGHT; y < rcPiece.bottom + MAPSQUARE_HEIGHT; y += MAPSQUARE_HEIGHT){
    if(y < 0) continue;
    if(y >= (int)m_pMap->GetHeight()) continue;
    for(x = rcPiece.left - MAPSQUARE_WIDTH; x < rcPiece.right + MAPSQUARE_WIDTH; x += MAPSQUARE_WIDTH){
      if((x < 0) || (x >= (int)m_pMap->GetWidth())) continue;

      pSquare = m_pMap->GetMapSquareFromCell(x, y);
      m_pMap->LockSquare(pSquare);
      listSquares.AddHead(pSquare);
    }
  }

  // now for every level

  // cleare the buffer
  memset(m_pLandTypeBuffer, 0, MBUFFER_WIDTH * MBUFFER_HEIGHT);
  DWORD dwLevel;
  CSize sizeMapex;
  BYTE bLandType;
  POSITION pos;
  int xh, yh;
  CEMapexInstance *pMapexInst;
  CEMapex *pMapex;
  CTypedPtrList<CPtrList, CEMapexInstance *> listMapexes;
  for(dwLevel = 1; dwLevel <= 3; dwLevel++){

    // get all mapexes
    pos = listSquares.GetHeadPosition();
    while(pos != NULL){
      pSquare = listSquares.GetNext(pos);

      pSquare->AppendBLMapexes(dwLevel, &listMapexes);
    }

    // now draw them
    pos = listMapexes.GetHeadPosition();
    while(pos != NULL){
      pMapexInst = listMapexes.GetNext(pos);
      pMapex = pMapexInst->GetMapex();
      if(pMapex != NULL){
        sizeMapex = pMapex->GetSize();
      }
      else{
        sizeMapex.cx = 0; sizeMapex.cy = 0;
      }

      yh = pMapexInst->GetYPosition() - rcPiece.top;
      for(y = 0; y < sizeMapex.cy; y++, yh++){
        if((yh < 0) || (yh >= rcPiece.Height())) continue;
        
        xh = pMapexInst->GetXPosition() - rcPiece.left;
        for(x = 0; x < sizeMapex.cx; x++, xh++){
          if((xh < 0) || (xh >= rcPiece.Width())) continue;

          bLandType = pMapex->GetLandType(x, y);
          if(bLandType != 0){
            m_pLandTypeBuffer[xh + yh * MBUFFER_WIDTH] = bLandType;
          }
        }
      }
    }

    listMapexes.RemoveAll();
  }

  // now draw the unit level
  {
    CTypedPtrList<CPtrList, CEULMapexInstance *> listULMapexes;
    CEULMapexInstance *pULMapexInst;
    // get all mapexes
    pos = listSquares.GetHeadPosition();
    while(pos != NULL){
      pSquare = listSquares.GetNext(pos);

      pSquare->AppendULMapexes(&listULMapexes);
    }

    // now draw them
    pos = listULMapexes.GetHeadPosition();
    while(pos != NULL){
      pULMapexInst = listULMapexes.GetNext(pos);
      pMapex = pULMapexInst->GetMapex();
      sizeMapex = pMapex->GetSize();

      yh = pULMapexInst->GetYPosition() - rcPiece.top;
      for(y = 0; y < sizeMapex.cy; y++, yh++){
        if((yh < 0) || (yh >= rcPiece.Height())) continue;
        
        xh = pULMapexInst->GetXPosition() - rcPiece.left;
        for(x = 0; x < sizeMapex.cx; x++, xh++){
          if((xh < 0) || (xh >= rcPiece.Width())) continue;

          bLandType = pMapex->GetLandType(x, y);
          if(bLandType != 0){
            m_pLandTypeBuffer[xh + yh * MBUFFER_WIDTH] = bLandType;
          }
        }
      }
    }

    listULMapexes.RemoveAll();
  }

  pos = listSquares.GetHeadPosition();
  while(pos != NULL){
    pSquare = listSquares.GetNext(pos);
    m_pMap->ReleaseSquare(pSquare);
  }
  listSquares.RemoveAll();

  // release exclusive access to the map
  // it means by now someone can delete it or reload
  m_pMap->ReleaseExclusiveAccess();

  DWORD dwX, dwY, dwOff;
  CRect rcMapPiece;
  DWORD dwR, dwG, dwB, dwColor;
  CLandType *pLandType;
  DWORD dwCount = m_dwZoom * m_dwZoom;
  LPBYTE pDest; DWORD dwPitch;
  CRect rcLock(0, 0, dwWidth, dwHeight);
  pDest = (LPBYTE)m_MiniMapBuffer.Lock(&rcLock, 0, dwPitch);

  for(dwY = 0; dwY < dwHeight; dwY++){
    for(dwX = 0; dwX < dwWidth; dwX++){
      yh = dwY * m_dwZoom;
      dwR = 0; dwG = 0; dwB = 0;
      for(y = 0; y < (int)m_dwZoom; y++, yh++){
        xh = dwX * m_dwZoom;
        for(x = 0; x < (int)m_dwZoom; x++, xh++){
          bLandType = m_pLandTypeBuffer[xh + yh * MBUFFER_WIDTH];
          if(bLandType != 0){
            pLandType = m_pMap->GetLandType(bLandType);
            if(pLandType != NULL){
              dwColor = pLandType->GetColor();
              dwR += R32(dwColor);
              dwG += G32(dwColor);
              dwB += B32(dwColor);
            }
          }
        }
      }
      dwR /= dwCount;
      dwG /= dwCount;
      dwB /= dwCount;
      if(g_pDirectDraw->Is32BitMode())
        dwOff = dwX * 4 + dwY * dwPitch;
      else
        dwOff = dwX * 3 + dwY * dwPitch;
      pDest[dwOff] = (BYTE)dwB;
      pDest[dwOff + 1] = (BYTE)dwG;
      pDest[dwOff + 2] = (BYTE)dwR;
    }
  }
  m_MiniMapBuffer.Unlock(pDest);
}

void CMiniMap::UpdateRect(CRect rcPiece)
{
  rcPiece.left -= rcPiece.left % m_dwWantedZoom;
  rcPiece.top -= rcPiece.top % m_dwWantedZoom;
  rcPiece.right += m_dwWantedZoom - (rcPiece.right % m_dwWantedZoom);
  rcPiece.bottom += m_dwWantedZoom - (rcPiece.bottom % m_dwWantedZoom);
  m_semaphoreUpdateList.Lock();

  m_UpdateList.AddRect(&rcPiece);

  m_semaphoreUpdateList.Unlock();
  m_semaphoreRectInlayed.Unlock();
}

void CMiniMap::SetDrawWnd(CWnd *pWnd)
{
  m_pDrawWnd = pWnd;
 
  if(m_pDrawWnd != NULL){
    CString str;
    str.Format("MiniMap %dx", m_dwZoom);
    m_pDrawWnd->SetWindowText(str);
  }
}

void CMiniMap::SetVisibleRect(CRect rcRect)
{
  m_rcVisibleRect = rcRect;
  if(m_pDrawWnd != NULL){
    m_pDrawWnd->Invalidate(FALSE);
  }
}
