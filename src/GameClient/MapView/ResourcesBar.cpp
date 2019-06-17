// ResourcesBar.cpp: implementation of the CResourcesBar class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ResourcesBar.h"

#include "..\GameClientGlobal.h"
#include "..\DataObjects\CMap.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

#define RESOURCEBAR_ITEM_WIDTH  55
#define RESOURCEBAR_LINE_HEIGHT 18

#define RESOURCEBAR_TEXT_COLOR RGB32(255, 255, 255)

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

IMPLEMENT_DYNAMIC(CResourcesBar, CWindow)

BEGIN_OBSERVER_MAP(CResourcesBar, CWindow)
  case -1:
    break;
END_OBSERVER_MAP(CResourcesBar, CWindow)

CResourcesBar::CResourcesBar()
{
  m_pFont = NULL;
  memset(m_aResources, 0, sizeof(int) * RESOURCE_COUNT);
}

CResourcesBar::~CResourcesBar()
{
  ASSERT(m_pFont == NULL);
}

#ifdef _DEBUG

void CResourcesBar::AssertValid() const
{
  CWindow::AssertValid();
  ASSERT(m_pFont != NULL);
}

void CResourcesBar::Dump(CDumpContext &dc) const
{
  CWindow::Dump(dc);
}

#endif

void CResourcesBar::Create(CWindow *pParent, CFontObject *pFont)
{
  ASSERT(pParent != NULL);
  ASSERT(pFont != NULL);

  m_pFont = pFont;

  // First determine count of used resources
  DWORD dwCount = 0, i;
  for(i = 0; i < RESOURCE_COUNT; i++){
    if(!g_pMap->GetResource(i)->GetName().IsEmpty()){
      dwCount ++;
    }
  }

  // Compute the positioning
  CRect rcParent;
  rcParent = pParent->GetWindowPosition();
  int nStartPosition, nRowCount = 1;

  while((int)((dwCount / nRowCount) * RESOURCEBAR_ITEM_WIDTH) > rcParent.Width())
    nRowCount++;

  nStartPosition = rcParent.Width() - ((dwCount + (nRowCount / 2)) / nRowCount) * RESOURCEBAR_ITEM_WIDTH;
  int nPos = nStartPosition, nRow = 0;
  for(i = 0; i < RESOURCE_COUNT; i++){
    if(g_pMap->GetResource(i)->GetName().IsEmpty()){
      m_aPositions[i].x = -1;
    }
    else{
      m_aPositions[i].x = nPos - nStartPosition;
      m_aPositions[i].y = nRow * RESOURCEBAR_LINE_HEIGHT;
      nPos += RESOURCEBAR_ITEM_WIDTH;
      if(nPos >= rcParent.Width()){
        nPos = nStartPosition;
        nRow++;
      }
    }
  }

  // Comptute the position of the window
  CRect rcWnd;
  rcWnd.left = nStartPosition;
  rcWnd.right = rcParent.Width();
  rcWnd.top = 0;
  rcWnd.bottom = nRowCount * RESOURCEBAR_LINE_HEIGHT;

  m_bTransparent = TRUE;
  CWindow::Create(&rcWnd, pParent);
}

void CResourcesBar::Delete()
{
  CWindow::Delete();
  m_pFont = NULL;
}

void CResourcesBar::SetResource(DWORD dwIndex, int nNewValue)
{
  ASSERT(dwIndex < RESOURCE_COUNT);

  m_aResources[dwIndex] = nNewValue;

  CRect rc;
  rc.left = m_aPositions[dwIndex].x;
  if(rc.left == -1) return;
  rc.top = m_aPositions[dwIndex].y;
  rc.right = rc.left + RESOURCEBAR_ITEM_WIDTH;
  rc.bottom = rc.top + RESOURCEBAR_LINE_HEIGHT;
  UpdateRect(&rc);
}

void CResourcesBar::Draw(CDDrawSurface *pSurface, CRect *pRect)
{
  DWORD i;

  CString str;
  int nVal;
  for(i = 0; i < RESOURCE_COUNT; i++){
    if(m_aPositions[i].x == -1) continue;

    pSurface->Paste(m_aPositions[i], g_pMap->GetResource(i)->GetIcon());
    
    nVal = m_aResources[i];
    if(abs(nVal) > 99999){ // nVal / 1000 k
      if(abs(nVal) > 9999999){ // nVal / 1000000 M
        if(abs(nVal) > 9999999999){ // nVal / 1000000000 G
          str.Format("%dG", nVal / 1000000000);
        }
        else{
          str.Format("%dM", nVal / 1000000);
        }
      }
      else{
        str.Format("%dk", nVal / 1000);
      }
    }
    else{
      str.Format("%d", nVal);
    }

    m_pFont->PaintText(m_aPositions[i].x + 17, m_aPositions[i].y + (RESOURCEBAR_LINE_HEIGHT - m_pFont->GetCharSize('A').cy) / 2,
      str, pSurface, RESOURCEBAR_TEXT_COLOR);
  }
}

// Returns the window from givven screen point
// works only on subtree of windows
CWindow * CResourcesBar::WindowFromPoint(CPoint &pt)
{
  return NULL;
}
