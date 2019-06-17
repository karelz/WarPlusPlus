// MapViewMessageBar.cpp: implementation of the CMapViewMessageBar class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "MapViewMessageBar.h"

#define MSGBAR_LINECOUNT 4
#define MSGBAR_MSGTIMEOUT 10000

#define MSGBAR_TOP 0
#define MSGBAR_LEFT 5

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

IMPLEMENT_DYNAMIC(CMapViewMessageBar, CWindow);

BEGIN_OBSERVER_MAP(CMapViewMessageBar, CWindow)
  BEGIN_TIMER()
    ON_TIMETICK()
  END_TIMER()

  BEGIN_MOUSE()
    ON_LBUTTONDOWN()
  END_MOUSE()
END_OBSERVER_MAP(CMapViewMessageBar, CWindow)

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

DWORD CMapViewMessageBar::m_aMsgTypeColor[MsgType_Count] = {
    RGB32(255, 255, 100),
    RGB32(220, 180, 220),
    RGB32(255, 150, 150),
    RGB32(255, 50, 50),
    RGB32(150, 150, 255),
  };

CMapViewMessageBar::CMapViewMessageBar()
{
  m_dwLinesCount = 0;
  m_pLines = NULL;
}

CMapViewMessageBar::~CMapViewMessageBar()
{
  ASSERT(m_pLines == NULL);
  ASSERT(m_dwLinesCount == 0);
}

#ifdef _DEBUG

void CMapViewMessageBar::AssertValid() const
{
  CWindow::AssertValid();
}

void CMapViewMessageBar::Dump(CDumpContext &dc) const
{
  CWindow::Dump(dc);
}

#endif

void CMapViewMessageBar::Create(CRect &rcWindow, CWindow *pParent, CDataArchive Archive)
{
  m_LocationImage.Create(Archive.CreateFile("MessageBar\\Location.tga"));

  CRect rc(rcWindow);
  rc.bottom = rc.top + g_pSystemFont->GetCharSize('A').cy * MSGBAR_LINECOUNT + MSGBAR_TOP;

  m_bTransparent = TRUE;
  VERIFY(CWindow::Create(&rc, pParent));

  g_pTimer->Connect(this, 0);

  ASSERT(MSGBAR_LINECOUNT > 0);
}

void CMapViewMessageBar::Delete()
{
  // Remove all lines
  {
    SMessageLine *pLine = m_pLines, *pDel;
    while(pLine != NULL){
      pDel = pLine;
      pLine = pLine->m_pNext;
      delete pDel;
    }
    m_pLines = NULL;
    m_dwLinesCount = 0;
  }

  g_pTimer->Disconnect(this);

  CWindow::Delete();

  m_LocationImage.Delete();
}

void CMapViewMessageBar::Draw(CDDrawSurface *pSurface, CRect *pRect)
{
  // Go through the list and draw them
  DWORD dwLineHeight = g_pSystemFont->GetCharSize('A').cy;

  SMessageLine *pLine;
  int nLine;
  for(nLine = MSGBAR_LINECOUNT - 1, pLine = m_pLines; nLine >= 0; nLine--)
  {
    if(pLine != NULL){
      if(HasLineLocation(pLine)){
        pSurface->Paste(MSGBAR_LEFT, MSGBAR_TOP + nLine * dwLineHeight, &m_LocationImage);
      }
      g_pSystemFont->PaintText(MSGBAR_LEFT + m_LocationImage.GetAllRect()->Width(), MSGBAR_TOP + nLine * dwLineHeight,
        pLine->m_strText, pSurface, pLine->m_dwColor);

      pLine = pLine->m_pNext;
    }
  }
}

void CMapViewMessageBar::Message(CString strText, EMessageType eType)
{
  InternalMessage(strText, eType);
}

void CMapViewMessageBar::Message(CString strText, EMessageType eType, DWORD dwUnitID)
{
  SMessageLine *pLine;
  pLine = InternalMessage(strText, eType);
  pLine->m_dwUnitID = dwUnitID;
}

void CMapViewMessageBar::Message(CString strText, EMessageType eType, DWORD dwXPos, DWORD dwYPos)
{
  SMessageLine *pLine;
  pLine = InternalMessage(strText, eType);
  pLine->m_dwXPos = dwXPos;
  pLine->m_dwYPos = dwYPos;
}

CMapViewMessageBar::SMessageLine *CMapViewMessageBar::InternalMessage(CString strText, EMessageType eType)
{
  if(m_dwLinesCount == MSGBAR_LINECOUNT){
    DeleteLast();
  }

  // Add new line to the begining
  SMessageLine *pLine = new SMessageLine();
  pLine->m_strText = strText;
  pLine->m_dwTime = g_pTimer->GetTime() + MSGBAR_MSGTIMEOUT;
  pLine->m_dwColor = m_aMsgTypeColor[eType];
  pLine->m_pNext = m_pLines;
  pLine->m_dwXPos = 0x0F0000000;
  pLine->m_dwYPos = 0x0F0000000;
  pLine->m_dwUnitID = 0x0FFFFFFFF;
  m_pLines = pLine;
  m_dwLinesCount++;

  ResetTimer();
  UpdateRect();

  return pLine;
}

void CMapViewMessageBar::OnTimeTick(DWORD dwTime)
{
  DeleteLast();
  ResetTimer();
  UpdateRect();
}

void CMapViewMessageBar::DeleteLast()
{
  if(m_dwLinesCount == 0) return;

  // Remove last item
  SMessageLine **pLine = &m_pLines;
  while((*pLine)->m_pNext != NULL) pLine = &((*pLine)->m_pNext);
  
  delete (*pLine);
  *pLine = NULL;

  m_dwLinesCount--;
}

void CMapViewMessageBar::ResetTimer()
{
  if(m_dwLinesCount == 0){
    g_pTimer->SetElapseTime(this, 0);
    return;
  }
  // Find the last line
  SMessageLine *pLine = m_pLines;
  while(pLine->m_pNext != NULL) pLine = pLine->m_pNext;

  if(pLine->m_dwTime <= g_pTimer->GetTime()){
    g_pTimer->SetElapseTime(this, 1);
  }
  else{
    g_pTimer->SetElapseTime(this, pLine->m_dwTime - g_pTimer->GetTime());
  }
}

CWindow *CMapViewMessageBar::WindowFromPoint(CPoint &pt)
{
  CWindow *pWnd = CWindow::WindowFromPoint(pt);
  if(pWnd != this) return pWnd;

  CPoint pt2(pt);
  CRect rcWindow = GetWindowPosition();
  pt2.x -= rcWindow.left;
  pt2.y -= rcWindow.top;

  SMessageLine *pLine = m_pLines;
  while(pLine != NULL){
    if(IsInLineLocation(pLine, pt2)) return this;
    pLine = pLine->m_pNext;
  }
  return NULL;
}

BOOL CMapViewMessageBar::HasLineLocation(SMessageLine *pLine)
{
  ASSERT(pLine != NULL);

  if((pLine->m_dwXPos == 0x0F0000000) && (pLine->m_dwUnitID == 0x0FFFFFFFF)){
    return FALSE;
  }
  return TRUE;
}

BOOL CMapViewMessageBar::IsInLineLocation(SMessageLine *pLine, CPoint &pt)
{
  if(!HasLineLocation(pLine)) return FALSE;

  DWORD dwLineHeight = g_pSystemFont->GetCharSize('A').cy;

  SMessageLine *pl = m_pLines;
  DWORD dwIndex = 0;
  while(pl != NULL){
    if(pl == pLine) break;
    dwIndex ++;
    pl = pl->m_pNext;
  }
  if(dwIndex >= m_dwLinesCount) return FALSE;

  CRect rc;
  rc.left = MSGBAR_LEFT; rc.right = rc.left + m_LocationImage.GetAllRect()->Width();
  rc.top = MSGBAR_TOP + dwIndex * dwLineHeight; rc.bottom = rc.top + dwLineHeight;

  return rc.PtInRect(pt);
}

void CMapViewMessageBar::OnLButtonDown(CPoint pt)
{
  SMessageLine *pLine = m_pLines;
  while(pLine != NULL){
    if(IsInLineLocation(pLine, pt)) break;
    pLine = pLine->m_pNext;
  }

  if(pLine == NULL) return;

  if(pLine->m_dwUnitID != 0x0FFFFFFFF){
    // Unit location
    InlayEvent(E_UnitLocation, pLine->m_dwUnitID);
  }
  else{
    InlayEvent(E_MapLocation, (pLine->m_dwXPos << 16) & (pLine->m_dwYPos & 0x0FFFF));
  }
}