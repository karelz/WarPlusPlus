// FrameWindowLayout.cpp: implementation of the CFrameWindowLayout class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "FrameWindowLayout.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

IMPLEMENT_DYNAMIC(CFrameWindowLayout, CObject);

CFrameWindowLayout::CFrameWindowLayout()
{
  m_pABottom = NULL;
  m_pABottomLeft = NULL;
  m_pABottomRight = NULL;
  m_pALeft = NULL;
  m_pAMiddle = NULL;
  m_pARight = NULL;
  m_pATop = NULL;
  m_pATopLeft = NULL;
  m_pATopRight = NULL;

  m_pPBottom = NULL;
  m_pPBottomLeft = NULL;
  m_pPBottomRight = NULL;
  m_pPLeft = NULL;
  m_pPMiddle = NULL;
  m_pPRight = NULL;
  m_pPTop = NULL;
  m_pPTopLeft = NULL;
  m_pPTopRight = NULL;

  m_pActivateSound = NULL;
  m_pDeactivateSound = NULL;

  m_pNormalCursor = NULL;
  m_pMoveCursor = NULL;
  m_pLRCursor = NULL;
  m_pTBCursor = NULL;
  m_pTLBRCursor = NULL;
  m_pTRBLCursor = NULL;

  m_nDragLeft = 0;
  m_nDragRight = 0;
  m_nDragTop = 0;
  m_nDragBottom = 0;

  m_nEdgeLeft = 0;
  m_nEdgeRight = 0;
  m_nEdgeTop = 0;
  m_nEdgeBottom = 0;
  m_nCornerSize = 0;

  m_dwPassiveMiddleColor = 0;
  m_dwActiveMiddleColor = 0;

  m_sizeMax.cx = 0x070000000;
  m_sizeMax.cy = 0x070000000;
}

CFrameWindowLayout::~CFrameWindowLayout()
{
  ASSERT(m_pABottom == NULL);
  ASSERT(m_pABottomLeft == NULL);
  ASSERT(m_pABottomRight == NULL);
  ASSERT(m_pALeft == NULL);
  ASSERT(m_pAMiddle == NULL);
  ASSERT(m_pARight == NULL);
  ASSERT(m_pATop == NULL);
  ASSERT(m_pATopLeft == NULL);
  ASSERT(m_pATopRight == NULL);

  ASSERT(m_pPBottom == NULL);
  ASSERT(m_pPBottomLeft == NULL);
  ASSERT(m_pPBottomRight == NULL);
  ASSERT(m_pPLeft == NULL);
  ASSERT(m_pPMiddle == NULL);
  ASSERT(m_pPRight == NULL);
  ASSERT(m_pPTop == NULL);
  ASSERT(m_pPTopLeft == NULL);
  ASSERT(m_pPTopRight == NULL);

  ASSERT(m_pActivateSound == NULL);
  ASSERT(m_pDeactivateSound == NULL);

  ASSERT(m_pNormalCursor == NULL);
  ASSERT(m_pMoveCursor == NULL);
  ASSERT(m_pLRCursor == NULL);
  ASSERT(m_pTBCursor == NULL);
  ASSERT(m_pTLBRCursor == NULL);
  ASSERT(m_pTRBLCursor == NULL);
}

#ifdef _DEBUG

void CFrameWindowLayout::AssertValid() const
{
  CObject::AssertValid();
}

void CFrameWindowLayout::Dump(CDumpContext &dc) const
{
  CObject::Dump(dc);
}

#endif


BOOL CFrameWindowLayout::Create(CConfigFile *pCfgFile)
{
  CFG_BEGIN(pCfgFile);


  CFG_ANIMATION_O("Passive_Bottom", m_pPBottom);
  CFG_ANIMATION_O("Passive_BottomLeft", m_pPBottomLeft);
  CFG_ANIMATION_O("Passive_BottomRight", m_pPBottomRight);

  CFG_ANIMATION_O("Passive_Middle", m_pPMiddle);
  if(m_pPMiddle == NULL){
    CFG_COLOR("Passive_Middle_Color", m_dwPassiveMiddleColor);
  }

  CFG_ANIMATION_O("Passive_Left", m_pPLeft);
  CFG_ANIMATION_O("Passive_Right", m_pPRight);

  CFG_ANIMATION_O("Passive_Top", m_pPTop);
  CFG_ANIMATION_O("Passive_TopLeft", m_pPTopLeft);
  CFG_ANIMATION_O("Passive_TopRight", m_pPTopRight);

  CFG_ANIMATION_O("Active_Bottom", m_pABottom);
  CFG_ANIMATION_O("Active_BottomLeft", m_pABottomLeft);
  CFG_ANIMATION_O("Active_BottomRight", m_pABottomRight);

  CFG_ANIMATION_O("Active_Middle", m_pAMiddle);
  if(m_pPMiddle == NULL){
    m_dwActiveMiddleColor = m_dwPassiveMiddleColor;
    CFG_COLOR_O("Active_Middle_Color", m_dwActiveMiddleColor);
  }
  CFG_ANIMATION_O("Active_Left", m_pALeft);
  CFG_ANIMATION_O("Active_Right", m_pARight);

  CFG_ANIMATION_O("Active_Top", m_pATop);
  CFG_ANIMATION_O("Active_TopLeft", m_pATopLeft);
  CFG_ANIMATION_O("Active_TopRight", m_pATopRight);
  
  CFG_CURSOR_O("NormalCursor", m_pNormalCursor);
  CFG_CURSOR_O("MoveCursor", m_pMoveCursor);
  CFG_CURSOR_O("LRCursor", m_pLRCursor);
  CFG_CURSOR_O("TBCursor", m_pTBCursor);
  CFG_CURSOR_O("TLBRCursor", m_pTLBRCursor);
  CFG_CURSOR_O("TRBLCursor", m_pTRBLCursor);

  CFG_SOUND_O("ActivateSound", m_pActivateSound);
  CFG_SOUND_O("DeactivateSound", m_pDeactivateSound);

  
  CFG_LONG_O("DragLeft", m_nDragLeft);
  CFG_LONG_O("DragRight", m_nDragRight);
  CFG_LONG_O("DragTop", m_nDragTop);
  CFG_LONG_O("DragBottom", m_nDragBottom);

  CFG_LONG_O("EdgeSize_Left", m_nEdgeLeft);
  CFG_LONG_O("EdgeSize_Right", m_nEdgeRight);
  CFG_LONG_O("EdgeSize_Top", m_nEdgeTop);
  CFG_LONG_O("EdgeSize_Bottom", m_nEdgeBottom);
  CFG_LONG_O("EdgeSize_Corner", m_nCornerSize);

  CFG_SIZE("MinimalSize", m_sizeMin);
  CFG_SIZE_O("MaximalSize", m_sizeMax);

  CFG_END();

  return TRUE;
}

void CFrameWindowLayout::Delete()
{
  CFG_DELETE(m_pPBottom); CFG_DELETE(m_pPBottomLeft); CFG_DELETE(m_pPBottomRight);
  CFG_DELETE(m_pPMiddle); CFG_DELETE(m_pPLeft); CFG_DELETE(m_pPRight);
  CFG_DELETE(m_pPTop); CFG_DELETE(m_pPTopLeft); CFG_DELETE(m_pPTopRight);

  CFG_DELETE(m_pABottom); CFG_DELETE(m_pABottomLeft); CFG_DELETE(m_pABottomRight);
  CFG_DELETE(m_pAMiddle); CFG_DELETE(m_pALeft); CFG_DELETE(m_pARight);
  CFG_DELETE(m_pATop); CFG_DELETE(m_pATopLeft); CFG_DELETE(m_pATopRight);

  CFG_DELETE(m_pNormalCursor); CFG_DELETE(m_pMoveCursor);
  CFG_DELETE(m_pLRCursor); CFG_DELETE(m_pTBCursor);
  CFG_DELETE(m_pTLBRCursor); CFG_DELETE(m_pTRBLCursor);

  CFG_DELETE(m_pActivateSound);
  CFG_DELETE(m_pDeactivateSound);
}

BOOL CFrameWindowLayout::AreAnimsTransparent()
{
  if((m_pPBottom == NULL) || m_pPBottom->IsTransparent()) return TRUE;
  if((m_pPBottomLeft == NULL) || m_pPBottomLeft->IsTransparent()) return TRUE;
  if((m_pPBottomRight == NULL) || m_pPBottomRight->IsTransparent()) return TRUE;
  if((m_pPLeft == NULL) || m_pPLeft->IsTransparent()) return TRUE;
//  if((m_pPMiddle == NULL) || m_pPMiddle->IsTransparent()) return TRUE;
  if((m_pPRight == NULL) || m_pPRight->IsTransparent()) return TRUE;
  if((m_pPTop == NULL) || m_pPTop->IsTransparent()) return TRUE;
  if((m_pPTopLeft == NULL) || m_pPTopLeft->IsTransparent()) return TRUE;
  if((m_pPTopRight == NULL) || m_pPTopRight->IsTransparent()) return TRUE;

  if((m_pABottom == NULL) || m_pABottom->IsTransparent()) return TRUE;
  if((m_pABottomLeft == NULL) || m_pABottomLeft->IsTransparent()) return TRUE;
  if((m_pABottomRight == NULL) || m_pABottomRight->IsTransparent()) return TRUE;
  if((m_pALeft == NULL) || m_pALeft->IsTransparent()) return TRUE;
//  if((m_pAMiddle == NULL) || m_pAMiddle->IsTransparent()) return TRUE;
  if((m_pARight == NULL) || m_pARight->IsTransparent()) return TRUE;
  if((m_pATop == NULL) || m_pATop->IsTransparent()) return TRUE;
  if((m_pATopLeft == NULL) || m_pATopLeft->IsTransparent()) return TRUE;
  if((m_pATopRight == NULL) || m_pATopRight->IsTransparent()) return TRUE;

  return FALSE;
}

BOOL CFrameWindowLayout::Create(CArchiveFile CfgFile)
{
  CConfigFile Config;
  BOOL bResult = TRUE;

  // create the config file
  Config.Create(CfgFile);

  if(!Create(&Config)){
    bResult = FALSE;
  }

  Config.Delete();

  return bResult;
}

BOOL CFrameWindowLayout::Create()
{
  return TRUE;
}