// EUnit.cpp: implementation of the CEUnit class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "..\MapEditor.h"
#include "EUnit.h"

#include "..\MapFormats.h"

#include "EMap.h"
#include "EUnitAppearance.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CEUnit::CEUnit()
{

}

CEUnit::~CEUnit()
{

}

void CEUnit::Create(CEUnitType *pUnitType, CECivilization *pCivilization)
{
  ASSERT(pUnitType != NULL);
  ASSERT(pCivilization != NULL);

  // copy the unit type and civs
  m_pUnitType = pUnitType;
  m_pCivilization = pCivilization;

  // zero the position
  m_dwX = 0;
  m_dwY = 0;
  m_dwZ = 0;

  // Random direction
  m_nDirection = 8;

  // NULL next
  m_pNext = NULL;
}

void CEUnit::Create(CArchiveFile file)
{
  if(g_dwMapFileVersion < MAP_FILE_VERSION(1, 0)){
    SUnitHeader_Old h;

    // read the header from file
    if(file.Read(&h, sizeof(h)) != sizeof(h)){
      throw new CStringException("Error loading unit instance.");
    }

    // get the unit type and civ. pointers
    m_pUnitType = AppGetActiveMap()->GetUnitType(h.m_dwUnitType);
    m_pCivilization = AppGetActiveMap()->GetCivilization(h.m_dwCivilization);

    // copy the position
    m_dwX = h.m_dwX;
    m_dwY = h.m_dwY;
    m_dwZ = h.m_dwZ;

    // copy direction
    m_nDirection = (BYTE)h.m_nDirection;

    // NULL next
    m_pNext = NULL;
  }
  else{
    SUnitHeader h;

    // read the header from file
    if(file.Read(&h, sizeof(h)) != sizeof(h)){
      throw new CStringException("Error loading unit instance.");
    }

    // get the unit type and civ. pointers
    m_pUnitType = AppGetActiveMap()->GetUnitType(h.m_dwUnitType);
    m_pCivilization = AppGetActiveMap()->GetCivilization(h.m_dwCivilization);

    // copy the position
    m_dwX = h.m_dwX;
    m_dwY = h.m_dwY;
    m_dwZ = h.m_dwZ;

    // copy direction
    m_nDirection = (BYTE)h.m_nDirection;

    // NULL next
    m_pNext = NULL;
  }
}

void CEUnit::Save(CArchiveFile file)
{
  SUnitHeader h;

  // zero the struct
  memset(&h, 0, sizeof(h));

  // copy values to struct
  if(m_pCivilization == NULL) h.m_dwCivilization = 0;
  else h.m_dwCivilization = m_pCivilization->GetID();
  if(m_pUnitType == NULL) h.m_dwUnitType = 0;
  else h.m_dwUnitType = m_pUnitType->GetID();

  h.m_dwX = m_dwX;
  h.m_dwY = m_dwY;
  h.m_dwZ = m_dwZ;
  h.m_nDirection = m_nDirection;

  // write the struct to file
  file.Write(&h, sizeof(h));
}

void CEUnit::Delete()
{
  // clear all pointers
  m_pUnitType = NULL;
  m_pCivilization = NULL;

  m_pNext = NULL;
}

CEUnitAnimation * CEUnit::GetDefaultDirection()
{
  if(m_pUnitType == NULL) return NULL;
  CEUnitAppearanceType *pAppType = m_pUnitType->GetDefaultAppearanceType();
  // compute it for the first mode (supposed to be the most usual one)
  if(pAppType == NULL) return NULL;
  CEUnitAppearance *pApp = pAppType->GetDefaultInstance();

  // find the first usable direction
  DWORD dwDir = m_nDirection;
  if(dwDir == 8) dwDir = 0;
  do{
    if(pApp->GetDirection(dwDir) != NULL)
      break;
    dwDir++;
    if(dwDir > 7) dwDir = 0;
  } while(dwDir != m_nDirection);

  if((dwDir == m_nDirection) && (pApp->GetDirection(dwDir) == NULL)) return NULL;

  return pApp->GetDirection(dwDir);
}

int CEUnit::GetXTransition()
{
  // get the default direction
  CEUnitAnimation *pAnim = GetDefaultDirection();

  // if none - no transition
  CEUnitSurface * pGraphics = pAnim->GetFrame ( 0 );

  if ( pGraphics == NULL ) return 0;

  return (-(pAnim->GetXGraphicalOffset()) - pGraphics->GetXOffset () );
}

int CEUnit::GetYTransition()
{
  // get the default direction
  CEUnitAnimation *pAnim = GetDefaultDirection();

  // if none - no transition
  CEUnitSurface * pGraphics = pAnim->GetFrame ( 0 );

  if ( pGraphics == NULL ) return 0;

  // it's the same as for the x (except the Height)
  // cause we assume that all units have square shape
  return ( -(pAnim->GetYGraphicalOffset()) - pGraphics->GetYOffset () );
}

CSize CEUnit::GetDefaultGraphicsSize()
{
  // get default direction
  CEUnitAnimation *pAnim = GetDefaultDirection();

  // if NULL - zero size
  if(pAnim == NULL) return CSize(0, 0);

  // return the size of the first frame (no animation in editor)
  return pAnim->GetFrame(0)->GetAllRect()->Size();
}

void CEUnit::Draw(CPoint ptTopLeft, CRect *pRectDest, CDDrawSurface *pSurface)
{
  CEUnitSurface *pGraphics;

  if(m_pUnitType == NULL) return;
  if(m_pCivilization == NULL) return;
  CEUnitAppearance *pApp = m_pUnitType->GetDefaultAppearanceType()->CreateInstance(m_pCivilization->GetColor());

  {
    pGraphics = NULL;
    DWORD i = m_nDirection;
    if(i == 8) i = 0;
    do{
      if(pApp->GetDirection(i) != NULL){
        pGraphics = pApp->GetDirection(i)->GetFrame(0);
        break;
      }
      i++; i = i % 8;
    }while(i != m_nDirection);
  }

  if(pGraphics == NULL) return;

  pSurface->Paste(ptTopLeft, pGraphics, NULL);
}

void CEUnit::DrawSelected(CPoint ptTopLeft, CRect *pRectDest, CDDrawSurface *pSurface, CDDrawSurface *pBuffer, CDDrawSurface *pSelectionLayer)
{
  CEUnitSurface *pGraphics;

  if(m_pUnitType == NULL) return;
  if(m_pCivilization == NULL) return;
  CEUnitAppearance *pApp = m_pUnitType->GetDefaultAppearanceType()->CreateInstance(m_pCivilization->GetColor());

  {
    pGraphics = NULL;
    DWORD i = m_nDirection;
    if(i == 8) i = 0;
    do{
      if(pApp->GetDirection(i) != NULL){
        pGraphics = pApp->GetDirection(i)->GetFrame(0);
        break;
      }
      i++; i = i % 8;
    }while(i != m_nDirection);
  }

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
        pSourceAlpha = pGraphics->GetAlphaChannel();
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

BOOL CEUnit::PtInUnit(DWORD dwX, DWORD dwY)
{
  DWORD dwHlp;
  dwHlp = (m_pUnitType->GetMoveWidth(0) + 1) / 2;

  if(dwX < m_dwX - dwHlp) return FALSE;
  if(dwY < m_dwY - dwHlp) return FALSE;

  if(dwX >= (m_dwX + dwHlp)) return FALSE;
  if(dwY >= (m_dwY + dwHlp)) return FALSE;

  return TRUE;
}

BOOL CEUnit::CheckValid()
{
  if(m_pUnitType == NULL) return FALSE;
  if(m_pCivilization == NULL) return FALSE;
  if(m_nDirection > 8) return FALSE;

  return TRUE;
}