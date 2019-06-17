// EMapexInstance.cpp: implementation of the CEMapexInstance class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "..\MapEditor.h"
#include "EMapexInstance.h"

#include "EMap.h"
#include "..\MapFormats.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CEMapexInstance::CEMapexInstance()
{
  m_pMapex = NULL;
  m_pNext = NULL;
}

CEMapexInstance::~CEMapexInstance()
{
}

BOOL CEMapexInstance::Create(CEMapex *pMapex, DWORD dwX, DWORD dwY)
{
  ASSERT(pMapex != NULL);

  m_pMapex = pMapex;
  m_dwX = dwX; m_dwY = dwY;

  return TRUE;
}

BOOL CEMapexInstance::Create(CArchiveFile file)
{
  if(g_dwMapFileVersion < MAP_FILE_VERSION(1, 0)){
    SMapexInstanceHeader_Old h;

    file.Read(&h, sizeof(h));

    m_dwX = h.m_dwX;
    m_dwY = h.m_dwY;
    m_pMapex = AppGetActiveMap()->GetMapex(h.m_dwMapexID);
    if(m_pMapex == NULL) return FALSE;
  }
  else{
    SMapexInstanceHeader h;

    file.Read(&h, sizeof(h));

    m_dwX = h.m_dwX;
    m_dwY = h.m_dwY;
    m_pMapex = AppGetActiveMap()->GetMapex(h.m_dwMapexID);
    if(m_pMapex == NULL) return FALSE;
  }

  return TRUE;
}

void CEMapexInstance::SaveToFile(CArchiveFile file)
{
  SMapexInstanceHeader h;

  h.m_dwX = m_dwX;
  h.m_dwY = m_dwY;
  h.m_dwMapexID = m_pMapex->GetID();

  file.Write(&h, sizeof(h));
}

void CEMapexInstance::Draw(CPoint ptTopLeft, CRect *pRectDest, CDDrawSurface *pSurface)
{
  CDDrawSurface *pGraphics;
  if(m_pMapex == NULL) return;
  pGraphics = m_pMapex->GetGraphics()->GetFrame(0);
  if(pGraphics == NULL) return;

  pSurface->Paste(ptTopLeft, pGraphics, NULL);
}

void CEMapexInstance::DrawSelected(CPoint ptTopLeft, CRect *pRectDest, CDDrawSurface *pSurface, CDDrawSurface *pBuffer, CDDrawSurface *pSelectionLayer)
{
  CDDrawSurface *pGraphics;
  if(m_pMapex == NULL) return;
  pGraphics = m_pMapex->GetGraphics()->GetFrame(0);
  if(pGraphics == NULL) return;

  CSize size;
  size.cx = pGraphics->GetWidth();
  size.cy = pGraphics->GetHeight();

  CSize sizeBuffer;
  sizeBuffer.cx = pBuffer->GetWidth();
  sizeBuffer.cy = pBuffer->GetHeight();

  CPoint ptSourcePos, ptDestPos;
  CRect rcPart, rcSrc;

  // do we have alpha
  BOOL bAlpha = pGraphics->HasAlphaChannel();

  LPBYTE pSourceAlpha, pBufferAlpha;
  DWORD nXLeft, nYLeft;
  DWORD dwOffS, dwOffD;
  
  nYLeft = size.cy;
  ptDestPos.y = ptTopLeft.y;
  ptSourcePos.y = 0;
  rcPart.top = 0; rcPart.left = 0;
  while(nYLeft > 0){
    if(nYLeft > (DWORD)sizeBuffer.cy)
      rcPart.bottom = rcPart.top + sizeBuffer.cy;
    else
      rcPart.bottom = rcPart.top + nYLeft;

    nXLeft = size.cx;
    ptDestPos.x = ptTopLeft.x;
    ptSourcePos.x = 0;
    while(nXLeft > 0){
      if(nXLeft > (DWORD)sizeBuffer.cx)
        rcPart.right = rcPart.left + sizeBuffer.cx;
      else
        rcPart.right = rcPart.left + nXLeft;

      // copy the graphics
      pBuffer->ClearAlphaChannel();
      rcSrc.left = ptSourcePos.x; rcSrc.top = ptSourcePos.y;
      rcSrc.right = rcSrc.left + rcPart.Width(); rcSrc.bottom = rcSrc.top + rcPart.Height();
      pBuffer->BltFast(0, 0, pGraphics, &rcSrc, 0x80000000); // special flag -> no alpha blit
      // paste the selection layer
      pBuffer->Paste(0, 0, pSelectionLayer);

      // if has alpha channel -> copy it
      if(bAlpha){
        pSourceAlpha = pGraphics->GetAlphaChannel(TRUE);
        pBufferAlpha = pBuffer->GetAlphaChannel();
        int i;
        for(i = 0; i < rcPart.Height(); i++){
          dwOffS = ptSourcePos.x + (ptSourcePos.y + i) * size.cx;
          dwOffD = i * sizeBuffer.cx;
          memcpy(&pBufferAlpha[dwOffD], &pSourceAlpha[dwOffS], rcPart.Width());
        }
        pBuffer->ReleaseAlphaChannel(FALSE);
        pGraphics->ReleaseAlphaChannel(FALSE);
      }


      // paste our buffer to the dest
      pSurface->Paste(ptDestPos, pBuffer, &rcPart);

      nXLeft -= rcPart.Width(); ptDestPos.x += rcPart.Width();
      ptSourcePos.x += rcPart.Width();
    }

    nYLeft -= rcPart.Height(); ptDestPos.y += rcPart.Height();
    ptSourcePos.y += rcPart.Height();
  }
}

BOOL CEMapexInstance::PtInMapex(DWORD dwX, DWORD dwY)
{
  if((dwX < m_dwX) || (dwY < m_dwY)) return FALSE;
  if((dwX >= (m_dwX + m_pMapex->GetSize().cx)) || (dwY >= (m_dwY + m_pMapex->GetSize().cy))) return FALSE;
  return TRUE;
}

void CEMapexInstance::SetPosition(DWORD dwX, DWORD dwY)
{
  m_dwX = dwX; m_dwY = dwY;
}

void CEMapexInstance::DrawLandTypes(CPoint ptTopLeft, CRect *pRectDest, CDDrawSurface *pSurface)
{
  if(m_pMapex == NULL) return;
  CSize size = m_pMapex->GetSize();  
  CRect rcMapCell, rcDraw;
  CEMap *pMap = AppGetActiveMap();
  CLandType *pLandType;
  BYTE nLandType;

  int x, y;
  rcMapCell.top = ptTopLeft.y;
  for(y = 0; y < size.cy; y ++, rcMapCell.top += 16){
    rcMapCell.bottom = rcMapCell.top + 16;

    rcMapCell.left = ptTopLeft.x;
    for(x = 0; x < size.cx; x++, rcMapCell.left += 16){
      // get land type
      nLandType = m_pMapex->GetLandType(x, y);
      if(nLandType == 0) continue; // if transparent -> no draw

      rcMapCell.right = rcMapCell.left + 16;
      if(!rcDraw.IntersectRect(&rcMapCell, pRectDest)) continue;
      pLandType = pMap->GetLandType(nLandType);
      if(pLandType->GetAnimation() != NULL){
        pSurface->Paste(rcDraw.TopLeft(), pLandType->GetAnimation()->GetFrame(0));
      }
      else{
        pSurface->Fill(pLandType->GetColor(), &rcDraw);
      }
    }
  }
}

void CEMapexInstance::DrawLandTypesSelected(CPoint ptTopLeft, CRect *pRectDest, CDDrawSurface *pSurface, CDDrawSurface *pBuffer, CDDrawSurface *pSelectionLayer)
{
  if(m_pMapex == NULL) return;
  CSize size = m_pMapex->GetSize();  
  CRect rcMapCell, rcDraw, rcFill(0, 0, 16, 16);
  CEMap *pMap = AppGetActiveMap();
  CLandType *pLandType;
  BYTE nLandType;

  int x, y;
  rcMapCell.top = ptTopLeft.y;
  for(y = 0; y < size.cy; y ++, rcMapCell.top += 16){
    rcMapCell.bottom = rcMapCell.top + 16;

    rcMapCell.left = ptTopLeft.x;
    for(x = 0; x < size.cx; x++, rcMapCell.left += 16){
      // get land type
      nLandType = m_pMapex->GetLandType(x, y);
      if(nLandType == 0) continue; // if transparent -> no draw

      rcMapCell.right = rcMapCell.left + 16;
      if(!rcDraw.IntersectRect(&rcMapCell, pRectDest)) continue;
      pLandType = pMap->GetLandType(nLandType);
      if(pLandType->GetAnimation() != NULL){
        pBuffer->Paste(0, 0, pLandType->GetAnimation()->GetFrame(0));
      }
      else{
        pBuffer->Fill(pLandType->GetColor(), &rcFill);
      }
      pBuffer->Paste(0, 0, pSelectionLayer, &rcFill);
      pSurface->Paste(rcDraw.TopLeft(), pBuffer);
    }
  }
}
