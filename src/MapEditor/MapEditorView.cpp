// MapEditorView.cpp : implementation of the CMapEditorView class
//

#include "stdafx.h"
#include "MapEditor.h"

#include "MapEditorDoc.h"
#include "MapEditorView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define MAPVIEW_BUFFERWIDTH 640 // 16 * 40
#define MAPVIEW_BUFFERHEIGHT 480 // 16 * 30

/////////////////////////////////////////////////////////////////////////////
// CMapEditorView

IMPLEMENT_DYNCREATE(CMapEditorView, CScrollView)

BEGIN_MESSAGE_MAP(CMapEditorView, CScrollView)
	//{{AFX_MSG_MAP(CMapEditorView)
	ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_WM_LBUTTONDOWN()
	ON_WM_ERASEBKGND()
	ON_WM_PAINT()
	ON_WM_RBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_RBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_WM_SETCURSOR()
	ON_WM_SIZE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMapEditorView construction/destruction

CMapEditorView::CMapEditorView()
{
  m_bInitialized = FALSE;
  m_bMoved = FALSE;
}

CMapEditorView::~CMapEditorView()
{
}

BOOL CMapEditorView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	return CScrollView::PreCreateWindow(cs);
}

/////////////////////////////////////////////////////////////////////////////
// CMapEditorView drawing

void CMapEditorView::OnDraw(CDC* pDC)
{
  CRect rcClient;
  GetClientRect(&rcClient);

  DrawRect(&rcClient);
}

/////////////////////////////////////////////////////////////////////////////
// CMapEditorView diagnostics

#ifdef _DEBUG
void CMapEditorView::AssertValid() const
{
	CScrollView::AssertValid();
}

void CMapEditorView::Dump(CDumpContext& dc) const
{
	CScrollView::Dump(dc);
}

CMapEditorDoc* CMapEditorView::GetDocument() // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CMapEditorDoc)));
	return (CMapEditorDoc*)m_pDocument;
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CMapEditorView message handlers

void CMapEditorView::OnInitialUpdate() 
{
	CScrollView::OnInitialUpdate();

  m_nDragging = 0;
	
  CEMap *pMap = GetDocument()->GetMap();
  CSize sizeMap;
  sizeMap.cx = pMap->GetWidth() * 16; sizeMap.cy = pMap->GetHeight() * 16;

  SetScrollSizes(MM_TEXT, sizeMap, CSize(320, 320), CSize(16, 16));

  m_bInitialized = TRUE;
}

int CMapEditorView::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CScrollView::OnCreate(lpCreateStruct) == -1)
		return -1;
	
  // create our buffer surface
  m_Buffer.SetWidth(MAPVIEW_BUFFERWIDTH);
  m_Buffer.SetHeight(MAPVIEW_BUFFERHEIGHT);
  m_Buffer.Create();

  m_Clipper.Create(this);

  m_hMoveCursor = AfxGetApp()->LoadCursor(IDC_MOVECURSOR);
  m_hSelectionCursor = AfxGetApp()->LoadCursor(IDC_SELECTIONCURSOR);

  m_nDragging = 0;

	return 0;
}

void CMapEditorView::OnDestroy() 
{
  m_bInitialized = FALSE;

	CScrollView::OnDestroy();
	
  m_Clipper.Delete();
  m_Buffer.Delete();
}

CPoint CMapEditorView::GetMapPosition(CPoint point)
{
  CPoint ptPos = GetScrollPosition();
  CRect rcClient;
  GetClientRect(&rcClient);
  
  return CPoint((ptPos.x + point.x - rcClient.left) / 16,
    (ptPos.y + point.y - rcClient.top) / 16);
}

CPoint CMapEditorView::GetMapCellCoords(DWORD dwX, DWORD dwY)
{
  CPoint ptPos = GetScrollPosition();
  CRect rcClient;
  GetClientRect(&rcClient);

  return CPoint((int)dwX * 16 - ptPos.x + rcClient.left,
    (int)dwY * 16 - ptPos.y + rcClient.top);
}

BOOL CMapEditorView::OnEraseBkgnd(CDC* pDC) 
{
  CBrush br( GetSysColor( COLOR_WINDOW ) );
  FillOutsideRect( pDC, &br );
  return TRUE;
}

void CMapEditorView::DrawRect(CRect *pRect)
{
	CMapEditorDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);

  // get the map
  CEMap *pMap = pDoc->GetMap();

  // set our clipper
  g_pDDPrimarySurface->SetClipper(&m_Clipper);

  // get client rect
  CRect rcClient;
  GetClientRect(&rcClient);

  // clip the givven rect inside the client rect
  CRect rcDraw(pRect);
  rcDraw.IntersectRect(&rcDraw, &rcClient);

  // set outer rect to the edges of map cells
  // shift it in the map real coords
  CRect rcOuter(rcDraw);
  CPoint ptScrollPos = GetScrollPosition();
  rcOuter.OffsetRect(ptScrollPos);
  rcOuter.left -= rcOuter.left % 16;
  rcOuter.top -= rcOuter.top % 16;

  // position on the map in cells
  int nMapPosX, nMapPosY;

  // position on the screen
  CPoint ptScreenPositionStart, ptScreenPos;
  ptScreenPositionStart.x = rcOuter.left - ptScrollPos.x;
  ptScreenPositionStart.y = rcOuter.top - ptScrollPos.y;
  ClientToScreen(&ptScreenPositionStart);

  if(pMap->IsDeleted()){
    CRect rcFill(ptScreenPositionStart.x, ptScreenPositionStart.y, rcDraw.Width(), rcDraw.Height());
    g_pDDPrimarySurface->Fill(0, &rcFill);
    return;
  }

  // pixels to draw
  int nLeftX, nLeftY;

  // helper rect
  CRect rcPart;

  nLeftY = rcOuter.Height();
  rcPart.top = 0; rcPart.left = 0;
  ptScreenPos.y = ptScreenPositionStart.y;
  nMapPosY = rcOuter.top / 16;

  while(nLeftY > 0){
    // get the Y size of the buffer to draw
    if(nLeftY > (int)m_Buffer.GetHeight()){
      rcPart.bottom = rcPart.top + m_Buffer.GetHeight();
    }
    else{
      rcPart.bottom = rcPart.top + nLeftY;
    }

    // set starting pos for Xs
    nLeftX = rcOuter.Width();
    ptScreenPos.x = ptScreenPositionStart.x;
    nMapPosX = rcOuter.left / 16;
    while(nLeftX > 0){
      // get the X size of the buffer to draw
      if(nLeftX > (int)m_Buffer.GetWidth()){
        rcPart.right = rcPart.left + m_Buffer.GetWidth();
      }
      else{
        rcPart.right = rcPart.left + nLeftX;
      }

      // here draw it into buffer
      pMap->DrawView(nMapPosX, nMapPosY, rcPart, &m_Buffer);
      // paste it to screen
      g_pDDPrimarySurface->Paste(ptScreenPos, &m_Buffer, &rcPart);

      // go to the next part on X coord
      nLeftX -= rcPart.Width(); ptScreenPos.x += rcPart.Width();
      nMapPosX += rcPart.Width() / 16;
    }

    // go to the next part on Y coord
    nLeftY -= rcPart.Height(); ptScreenPos.y += rcPart.Height();
    nMapPosY += rcPart.Height() / 16;
  }
}

void CMapEditorView::OnPaint() 
{
  CRect rcUpdate;
  if(GetUpdateRect(NULL, FALSE)){ // if no update region -> normal paint -> paint whole window
    GetUpdateRect(&rcUpdate, FALSE); // if there is some update region -> paint only it
  }
  else{
    GetClientRect(&rcUpdate);
  }

	CPaintDC dc(this); // device context for painting

	CMapEditorDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);

  // get the map
  CEMap *pMap = pDoc->GetMap();
  if(pMap->IsDeleted()){
    CBrush brush;
    brush.CreateSolidBrush(RGB(0, 0, 0));
    dc.FillRect(&rcUpdate, &brush);
    return;
  }

	DrawRect(&rcUpdate);
}

void CMapEditorView::GetMapexViewRect(CEMapexInstance *pMapexInstance, CRect *pRect)
{
  if(pMapexInstance == NULL){ // if NULL -> return empty rect
    pRect->left = pRect->top = pRect->right = pRect->bottom = 0;
    return;
  }

  CPoint ptScroll = GetScrollPosition();
  CRect rcClient;
  GetClientRect(&rcClient);

  // position counted from mapex instance position
  pRect->left = pMapexInstance->GetXPosition() * 16 - ptScroll.x + rcClient.left;
  pRect->top = pMapexInstance->GetYPosition() * 16 - ptScroll.y + rcClient.top;

  // size is same as size of mapex graphics
  CDDrawSurface *pSurface = pMapexInstance->GetMapex()->GetGraphics()->GetFrame(0);
  pRect->right = pRect->left + pSurface->GetWidth();
  pRect->bottom = pRect->top + pSurface->GetHeight();

  // clip it inside the client rect
  pRect->IntersectRect(pRect, &rcClient);
}

void CMapEditorView::GetULMapexViewRect(CEULMapexInstance *pMapexInstance, CRect *pRect)
{
  if(pMapexInstance == NULL){ // if NULL -> return empty rect
    pRect->left = pRect->top = pRect->right = pRect->bottom = 0;
    return;
  }

  CPoint ptScroll = GetScrollPosition();
  CRect rcClient;
  GetClientRect(&rcClient);

  // position counted from mapex instance position
  pRect->left = pMapexInstance->GetXPosition() * 16 - ptScroll.x + rcClient.left;
  pRect->top = pMapexInstance->GetYPosition() * 16 - ptScroll.y + rcClient.top;

  // size is same as size of mapex graphics
  CDDrawSurface *pSurface = pMapexInstance->GetMapex()->GetGraphics()->GetFrame(0);
  pRect->right = pRect->left + pSurface->GetWidth();
  pRect->bottom = pRect->top + pSurface->GetHeight();

  // clip it inside the client rect
  pRect->IntersectRect(pRect, &rcClient);
}


// returns the bounding rectangle of the given unit object
void CMapEditorView::GetUnitViewRect(CEUnit *pUnit, CRect *pRect)
{
  // if no unit specified -> no rectangle (empty one)
  if(pUnit == NULL){
    pRect->SetRectEmpty();
    return;
  }

  // get current view positions and sizes
  CPoint ptScroll = GetScrollPosition();
  CRect rcClient;
  GetClientRect(&rcClient);

  // position from unit instance position
  pRect->left = pUnit->GetXPos() * 16 + pUnit->GetXTransition() - ptScroll.x + rcClient.left;
  pRect->top = pUnit->GetYPos() * 16 + pUnit->GetYTransition() - ptScroll.y + rcClient.top;

  // size is the same as the size of graphics
  CSize size = pUnit->GetDefaultGraphicsSize();
  pRect->right = pRect->left + size.cx;
  pRect->bottom = pRect->top + size.cy;

  // clip it inside the client rect
  pRect->IntersectRect(pRect, &rcClient);
}

void CMapEditorView::GetSelectionViewRect(CMapexInstanceSelection *pSelection, CRect *pRect)
{
  if(pSelection == NULL){
    pRect->left = pRect->top = pRect->right = pRect->bottom = 0;
    return;
  }

  CPoint ptScroll = GetScrollPosition();
  CRect rcClient;
  GetClientRect(&rcClient);

  CRect rcSel = pSelection->GetBoundingRect();
  pRect->left = rcSel.left * 16 - ptScroll.x + rcClient.left;
  pRect->top = rcSel.top * 16 - ptScroll.y + rcClient.top;
  pRect->right = pRect->left + rcSel.Width() * 16;
  pRect->bottom = pRect->top + rcSel.Height() * 16;

  pRect->IntersectRect(pRect, &rcClient);
}

void CMapEditorView::OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint) 
{
  if((lHint == 0) && (pHint == NULL)){
    Invalidate(FALSE);
    return;
  }

  switch(lHint){
  case CMapEditorDoc::Update_Window:
    UpdateWindow();
    break;

  case CMapEditorDoc::Update_MapexInstance:
    {
      CEMapexInstance *pMapexInstance = (CEMapexInstance *)((void *)pHint);
      CRect rcMapex;
      GetMapexViewRect(pMapexInstance, &rcMapex);
      InvalidateRect(&rcMapex, FALSE);
    }
    break;

  case CMapEditorDoc::Update_Selection:
    {
      CMapexInstanceSelection *pSelection = (CMapexInstanceSelection *)((void *)pHint);
      CRect rcSelection;
      GetSelectionViewRect(pSelection, &rcSelection);
      InvalidateRect(&rcSelection, FALSE);
    }
    break;

  case CMapEditorDoc::Update_SelectionRect:
    {
      CRect *pRect = (CRect *)(void *)pHint;
      CPoint ptTL = GetMapCellCoords(pRect->left, pRect->top);
      CPoint ptBR = GetMapCellCoords(pRect->right, pRect->bottom);
      CRect rcUpdate(ptTL.x, ptTL.y, ptBR.x + 1, ptBR.y + 1);
      InvalidateRect(&rcUpdate, FALSE);
    }
    break;

  case CMapEditorDoc::Update_ULMapexInstance:
    {
      CEULMapexInstance *pMapexInstance = (CEULMapexInstance *)((void *)pHint);
      CRect rcMapex;
      GetULMapexViewRect(pMapexInstance, &rcMapex);
      InvalidateRect(&rcMapex, FALSE);
    }
    break;

  case CMapEditorDoc::Update_Unit:
    {
      CEUnit *pUnit = (CEUnit *)((void *)pHint);
      CRect rcUnit;
      GetUnitViewRect(pUnit, &rcUnit);
      InvalidateRect(&rcUnit, FALSE);
    }
    break;

  case CMapEditorDoc::Update_CancelUpdates:
    {
      ValidateRect(NULL);
    }
    break;

  default:
    Invalidate(FALSE);
    break;
  }
}

void CMapEditorView::OnLButtonDown(UINT nFlags, CPoint point) 
{
  if(m_nDragging != 0) return;

  CMapEditorDoc *pDoc = GetDocument();

  CPoint ptMapPos = GetMapPosition(point);
  if(pDoc->PtInSelection(ptMapPos.x, ptMapPos.y)){
    switch(pDoc->GetSelectionType()){
    case CMapEditorDoc::Sel_None:
      break;
    case CMapEditorDoc::Sel_BLMapexInstance:
      if((nFlags & MK_SHIFT) || (nFlags & MK_CONTROL)){
        // select+ mapex (it will disselect one really)
        CEMapexInstance *pInstance;
        CPoint ptMapPos = GetMapPosition(point);
        pInstance = pDoc->GetMapexFromCell(ptMapPos.x, ptMapPos.y);

        pDoc->AddSelectedMapexInstance(pInstance, TRUE);
      }
      else{
        // start moving the selection
        if(!pDoc->GetSelection()->IsEmpty()){
          CPoint ptSelPos;
          CRect rcSel = pDoc->GetSelection()->GetBoundingRect();
          ptSelPos = GetMapPosition(point);
          m_ptDragTrans.x = ptSelPos.x - rcSel.left;
          m_ptDragTrans.y = ptSelPos.y - rcSel.top;
          m_nDragging = 1;
          SetCapture();
          ::SetCursor(m_hMoveCursor);
          m_bMoved = FALSE;
        }
      }
      break;
    case CMapEditorDoc::Sel_ULMapexInstance:
      {
        CPoint ptSelPos;
        CRect rcSel = pDoc->GetSelectionBoundingRect();
        ptSelPos = GetMapPosition(point);
        m_ptDragTrans.x = ptSelPos.x - rcSel.left;
        m_ptDragTrans.y = ptSelPos.y - rcSel.top;
        m_nDragging = 1;
        SetCapture();
        ::SetCursor(m_hMoveCursor);
      }
      break;
    case CMapEditorDoc::Sel_Unit:
      {
        CPoint ptSelPos;
        CRect rcSel = pDoc->GetSelectionBoundingRect();
        ptSelPos = GetMapPosition(point);
        m_ptDragTrans.x = ptSelPos.x - rcSel.left;
        m_ptDragTrans.y = ptSelPos.y - rcSel.top;
        m_nDragging = 1;
        SetCapture();
        ::SetCursor(m_hMoveCursor);
      }
      break;
    }
  }
  else{
    // select mapex
    CEMapexInstance *pInstance;
    CEULMapexInstance *pULInstance;
    CPoint ptMapPos = GetMapPosition(point);
    pULInstance = pDoc->GetULMapexFromCell(ptMapPos.x, ptMapPos.y);
    CEUnit *pUnit = pDoc->GetUnitFromCell(ptMapPos.x, ptMapPos.y);

    if(pUnit != NULL){
      pDoc->SetSelectedUnit(pUnit);
    }
    else{
      if(pULInstance != NULL){
        pDoc->SetSelectedULMapexInstance(pULInstance);
      }
      else{
        pInstance = pDoc->GetMapexFromCell(ptMapPos.x, ptMapPos.y);
        
        if(!(nFlags & MK_SHIFT) || !(nFlags & MK_CONTROL))
          pDoc->ClearSelection();
        if(pInstance != NULL)
          pDoc->AddSelectedMapexInstance(pInstance, TRUE);
        
        // start the selection drag
        m_ptDragTrans = ptMapPos;
        SetCapture();
        m_nDragging = 3;
        ::SetCursor(m_hSelectionCursor);
      }
    }
  }

	CScrollView::OnLButtonDown(nFlags, point);
}

void CMapEditorView::OnRButtonDown(UINT nFlags, CPoint point) 
{
  if(m_nDragging != 0) return;

  CMapEditorDoc *pDoc = GetDocument();

  // get current selected mapex
  CEMapex *pMapex = pDoc->GetSelectedMapex();
  if(pMapex == NULL){
    // add new unit instance

    CEUnitType *pUnitType = pDoc->GetSelectedUnitType();
    CECivilization *pCivilization = pDoc->GetSelectedCivilization();

    CEUnit *pUnit;
    pUnit = pDoc->AddUnitInstance(GetMapPosition(point), pUnitType, pCivilization);
    ValidateRect(NULL);
    pDoc->ClearSelection();
    pDoc->SetSelectedUnit(pUnit);
  }
  else{
    // add new mapex instance

    // UL mapex
    if(pDoc->GetSelectedLevel() == 4){
      CEULMapexInstance *pMapexInstance;
      pMapexInstance = pDoc->AddULMapexInstance(GetMapPosition(point), pMapex);
      ValidateRect(NULL);
      pDoc->ClearSelection();
      pDoc->SetSelectedULMapexInstance(pMapexInstance);
    }
    else{
      // normal BL mapex
      CEMapexInstance *pMapexInstance;
      pMapexInstance = 
        pDoc->AddMapexInstance(GetMapPosition(point), pMapex, pDoc->GetSelectedLevel());
      ValidateRect(NULL); // all will be updated by setting the new one as selected
      pDoc->ClearSelection();
      pDoc->AddSelectedMapexInstance(pMapexInstance, FALSE);
    }
  }

  CPoint ptSelPos;
  CRect rcSel = pDoc->GetSelectionBoundingRect();
  ptSelPos = GetMapPosition(point);
  m_ptDragTrans.x = ptSelPos.x - rcSel.left;
  m_ptDragTrans.y = ptSelPos.y - rcSel.top;
  m_nDragging = 2;
  SetCapture();
  ::SetCursor(m_hMoveCursor);

  CScrollView::OnRButtonDown(nFlags, point);
}

void CMapEditorView::OnLButtonUp(UINT nFlags, CPoint point) 
{
  CMapEditorDoc *pDoc = GetDocument();
  CPoint ptMapPos = GetMapPosition(point);

  if(m_nDragging == 1){
    ReleaseCapture();
    m_nDragging = 0;

    // did anything moved (the mouse especially)
    if(!m_bMoved){
      // No -> just a simple click -> select another mapex
      CEMapexInstance *pInstance = NULL, *pSelected = NULL;
      pSelected = pDoc->GetSelectedMapexFromCell(ptMapPos.x, ptMapPos.y);
      pInstance = pDoc->GetMapexFromCell(ptMapPos.x, ptMapPos.y, pSelected);

      // select the mapex
      pDoc->ClearSelection();
      pDoc->AddSelectedMapexInstance(pInstance, FALSE);
    }
  }

  if((m_nDragging == 3) || (m_nDragging == 4)){
    CMapEditorDoc *pDoc = GetDocument();
    pDoc->DoneRectSelection();
    ReleaseCapture();
    m_nDragging = 0;
  }
  
	CScrollView::OnLButtonUp(nFlags, point);
}

void CMapEditorView::OnRButtonUp(UINT nFlags, CPoint point) 
{
  if(m_nDragging == 2){
    ReleaseCapture();
    m_nDragging = 0;
  }
  
	CScrollView::OnRButtonUp(nFlags, point);
}

void CMapEditorView::OnMouseMove(UINT nFlags, CPoint point) 
{
  CMapEditorDoc *pDoc = GetDocument();
  if(m_nDragging == 0){
    CRect rcClient; GetClientRect(&rcClient);
    if(rcClient.PtInRect(point)){
      CPoint ptMapPos = GetMapPosition(point);
      if(pDoc->PtInSelection(ptMapPos.x, ptMapPos.y)){
        ::SetCursor(m_hMoveCursor);
      }
      else{
        ::SetCursor(AfxGetApp()->LoadStandardCursor(IDC_ARROW));
      }
    }
    else{
      ::SetCursor(AfxGetApp()->LoadStandardCursor(IDC_ARROW));
    }
  }

  if(m_nDragging == 1){
    ::SetCursor(m_hMoveCursor);
    CPoint ptSelPos = GetMapPosition(point);
    CPoint ptNewPos;
    ptNewPos.x = ptSelPos.x - m_ptDragTrans.x;
    if(ptNewPos.x < 0) ptNewPos.x = 0;
    ptNewPos.y = ptSelPos.y - m_ptDragTrans.y;
    if(ptNewPos.y < 0) ptNewPos.y = 0;
    pDoc->SetSelectionPosition(ptNewPos.x, ptNewPos.y);
    m_bMoved = TRUE;
  }
  
  if(m_nDragging == 2){
    ::SetCursor(m_hMoveCursor);
    CPoint ptSelPos = GetMapPosition(point);
    CPoint ptNewPos;
    ptNewPos.x = ptSelPos.x - m_ptDragTrans.x;
    if(ptNewPos.x < 0) ptNewPos.x = 0;
    ptNewPos.y = ptSelPos.y - m_ptDragTrans.y;
    if(ptNewPos.y < 0) ptNewPos.y = 0;
    pDoc->SetSelectionPosition(ptNewPos.x, ptNewPos.y);
  }

  if(m_nDragging == 3){
    ::SetCursor(m_hSelectionCursor);

    CPoint ptSelPos = GetMapPosition(point);
    if((m_ptDragTrans.x != ptSelPos.x) || (m_ptDragTrans.y != ptSelPos.y)){
      m_nDragging = 4;
    }
  }
  if(m_nDragging == 4){
    ::SetCursor(m_hSelectionCursor);

    CRect rcSel;
    CPoint ptSelPos = GetMapPosition(point);
    ptSelPos.x += 1; // enlarge it by one (for intersections)
    ptSelPos.y += 1;
    rcSel.left = m_ptDragTrans.x;
    rcSel.top = m_ptDragTrans.y;
    rcSel.right = ptSelPos.x;
    rcSel.bottom = ptSelPos.y;
    rcSel.NormalizeRect();
    pDoc->SetRectSelection(rcSel);
  }

	CScrollView::OnMouseMove(nFlags, point);
}

BOOL CMapEditorView::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message) 
{
  return TRUE;
}

BOOL CMapEditorView::OnScroll(UINT nScrollCode, UINT nPos, BOOL bDoScroll) 
{
  BOOL bReturn = CScrollView::OnScroll(nScrollCode, nPos, bDoScroll);

  if(bDoScroll){
    UpdateVisibleRect();
  }
  return bReturn;
}

void CMapEditorView::UpdateVisibleRect()
{
  if(!m_bInitialized) return;

  CMapEditorDoc* pDoc = GetDocument();
  ASSERT_VALID(pDoc);
  
  // get the map
  CEMap *pMap = pDoc->GetMap();
  if(pMap == NULL) return;
  
  // get client rect
  CRect rcClient;
  GetClientRect(&rcClient);
  
  // set outer rect to the edges of map cells
  // shift it in the map real coords
  CRect rcOuter(rcClient);
  CPoint ptScrollPos = GetScrollPosition();
  rcOuter.OffsetRect(ptScrollPos);
  
  rcOuter.left /= 16; rcOuter.right /= 16; rcOuter.top /= 16; rcOuter.bottom /= 16;
  pMap->SetVisibleRect(rcOuter);
}

void CMapEditorView::OnSize(UINT nType, int cx, int cy) 
{
	CScrollView::OnSize(nType, cx, cy);
	
  UpdateVisibleRect();	
}

void CMapEditorView::SetVisibleRect(CRect rcRect)
{
  ScrollToPosition(CPoint(rcRect.left * 16, rcRect.top * 16));
}
