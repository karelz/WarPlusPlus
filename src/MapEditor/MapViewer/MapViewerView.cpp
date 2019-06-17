// MapViewerView.cpp : implementation of the CMapViewerView class
//

#include "stdafx.h"
#include "MapViewer.h"

#include "MapViewerDoc.h"
#include "MapViewerView.h"

#include "MainFrame.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define MAP_SQUARE_SIZE			64

// velikost informace o MapCellu (v pixelech)
#define MAPCELL_DRAWINFO_SIZE		5
#define MAPCELL_TEXTINFO_SIZE		17
#define MAPCELL_TEXTINFO_OFFSETX	1
#define MAPCELL_TEXTINFO_OFFSETY	3
// velikost stránky na skrolování
#define PAGE_SIZE					20

#define MAPCELL_EXPLORED_BKCOLOR			RGB ( 255, 0, 0 )
#define MAPCELL_NONE_BKCOLOR				RGB ( 200, 200, 200 )
#define MAPCELL_UNEXPLORED_BKCOLOR		RGB ( 255, 255, 255 )
#define MAPCELL_AREA_BKCOLOR				RGB ( 0, 0, 255 )
#define MAPCELL_AREA_EXPLORED_BKCOLOR	RGB ( 255, 0, 255)
#define LINE_COLOR							RGB ( 128, 96, 0 )

#define COL(r,g,b) RGB ( r, g, b ), RGB ( r, g, b )

#define MAX_COLOR		50
COLORREF g_aColors[MAX_COLOR] = { COL ( 0, 0, 0 ), 
	COL ( 255, 0, 0 ),
//	COL ( 255, 64, 0 ),
	COL ( 255, 128, 0 ),
//	COL ( 255, 192, 0 ),
	COL ( 255, 255, 0 ),
//	COL ( 192, 255, 0 ),
	COL ( 128, 255, 0 ),
//	COL ( 64, 255, 0 ),
	COL ( 0, 255, 0 ),
//	COL ( 0, 255, 64 ),
	COL ( 0, 255, 128 ),
//	COL ( 0, 255, 192 ),
	COL ( 0, 255, 255 ),
//	COL ( 0, 192, 255 ),
	COL ( 0, 128, 255 ),
//	COL ( 0, 64, 255 ),
	COL ( 0, 0, 255 ),
//	COL ( 64, 0, 255 ),
	COL ( 128, 0, 255 ),
//	COL ( 192, 0, 255 ),
	COL ( 255, 0, 255 ),
//	COL ( 255, 0, 192 ),
	COL ( 255, 0, 128 ),
//	COL ( 255, 0, 64 )

//	COL ( 255, 0, 0 ),
	COL ( 255, 64, 0 ),
//	COL ( 255, 128, 0 ),
	COL ( 255, 192, 0 ),
//	COL ( 255, 255, 0 ),
	COL ( 192, 255, 0 ),
//	COL ( 128, 255, 0 ),
	COL ( 64, 255, 0 ),
//	COL ( 0, 255, 0 ),
	COL ( 0, 255, 64 ),
//	COL ( 0, 255, 128 ),
	COL ( 0, 255, 192 ),
//	COL ( 0, 255, 255 ),
	COL ( 0, 192, 255 ),
//	COL ( 0, 128, 255 ),
	COL ( 0, 64, 255 ),
//	COL ( 0, 0, 255 ),
	COL ( 64, 0, 255 ),
//	COL ( 128, 0, 255 ),
	COL ( 192, 0, 255 ),
//	COL ( 255, 0, 255 ),
	COL ( 255, 0, 192 ),
//	COL ( 255, 0, 128 ),
	COL ( 255, 0, 64 )
};
/*COLORREF g_aColors[MAX_COLOR] = { RGB ( 0, 0, 0 ), RGB ( 0, 0, 0 ), 
	RGB ( 0, 0, 255 ), RGB ( 0, 0, 255 ),
	RGB ( 0, 255, 0 ), RGB ( 0, 255, 0 ),
	RGB ( 255, 0, 0 ), RGB ( 255, 0, 0 ),
	RGB ( 0, 0, 224 ), RGB ( 0, 0, 224 ),
	RGB ( 0, 224, 0 ), RGB ( 0, 224, 0 ),
	RGB ( 224, 0, 0 ), RGB ( 224, 0, 0 ),
	RGB ( 0, 0, 192 ), RGB ( 0, 0, 192 ),
	RGB ( 0, 192, 0 ), RGB ( 0, 192, 0 ),
	RGB ( 192, 0, 0 ), RGB ( 192, 0, 0 ),
	RGB ( 0, 0, 160 ), RGB ( 0, 0, 160 ),
	RGB ( 0, 160, 0 ), RGB ( 0, 160, 0 ),
	RGB ( 160, 0, 0 ), RGB ( 160, 0, 0 ),
	RGB ( 0, 0, 128 ), RGB ( 0, 0, 128 ),
	RGB ( 0, 128, 0 ), RGB ( 0, 128, 0 ),
	RGB ( 128, 0, 0 ), RGB ( 128, 0, 0 ),
	RGB ( 0, 0, 96 ), RGB ( 0, 0, 96 ),
	RGB ( 0, 96, 0 ), RGB ( 0, 96, 0 ),
	RGB ( 96, 0, 0 ), RGB ( 96, 0, 0 ),
	RGB ( 0, 0, 64 ), RGB ( 0, 0, 64 ),
	RGB ( 0, 64, 0 ), RGB ( 0, 64, 0 ),
	RGB ( 64, 0, 0 ), RGB ( 64, 0, 0 ),
	RGB ( 0, 0, 32 ), RGB ( 0, 0, 32 ),
	RGB ( 0, 32, 0 ), RGB ( 0, 32, 0 ),
	RGB ( 32, 0, 0 ), RGB ( 32, 0, 0 )
};*/

/////////////////////////////////////////////////////////////////////////////
// CMapViewerView

IMPLEMENT_DYNCREATE(CMapViewerView, CScrollView)

BEGIN_MESSAGE_MAP(CMapViewerView, CScrollView)
	//{{AFX_MSG_MAP(CMapViewerView)
	ON_WM_MOUSEMOVE()
	ON_WM_PAINT()
	ON_WM_TIMER()
	ON_WM_DESTROY()
	ON_WM_ERASEBKGND()
	ON_COMMAND(ID_TOGGLEVIEW, OnToggleView)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMapViewerView construction/destruction

CMapViewerView::CMapViewerView () 
{
	// TODO: add construction code here
	m_nTimerID = 0;
	m_bDrawView = TRUE;
}

CMapViewerView::~CMapViewerView () 
{
}

BOOL CMapViewerView::PreCreateWindow ( CREATESTRUCT &cs ) 
{
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	return CScrollView::PreCreateWindow ( cs );
}

/////////////////////////////////////////////////////////////////////////////
// CMapViewerView drawing

void CMapViewerView::OnInitialUpdate () 
{
	CScrollView::OnInitialUpdate ();

	UpdateScroll ( CPoint ( 0, 0 ) );

	m_nTimerID = SetTimer ( 2, 1000, NULL );

	m_cFont.CreatePointFont ( 90, "Courier" );
}

/////////////////////////////////////////////////////////////////////////////
// CMapViewerView diagnostics

#ifdef _DEBUG
void CMapViewerView::AssertValid() const
{
	CScrollView::AssertValid();
}

void CMapViewerView::Dump(CDumpContext& dc) const
{
	CScrollView::Dump(dc);
}

CMapViewerDoc* CMapViewerView::GetDocument() // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CMapViewerDoc)));
	return (CMapViewerDoc*)m_pDocument;
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CMapViewerView message handlers

void CMapViewerView::OnMouseMove(UINT nFlags, CPoint point) 
{
	CPoint ptPos = GetScrollPosition();
	CRect rcClient;
	GetClientRect(&rcClient);

	CPoint ptResult;
	ptResult.x = point.x - rcClient.left + ptPos.x;
	ptResult.y = point.y - rcClient.top + ptPos.y;

	BOOL bDrawView = m_bDrawView;
	int nMapCellSize = bDrawView ? MAPCELL_DRAWINFO_SIZE : MAPCELL_TEXTINFO_SIZE;

	CMainFrame *pFrame = (CMainFrame *)AfxGetMainWnd ();
	pFrame->SetMapPositionPane ( ptResult.x / nMapCellSize, ptResult.y / nMapCellSize );

	CScrollView::OnMouseMove(nFlags, point);
}

void CMapViewerView::OnPaint () 
{
	CRect rectUpdate;
	if ( GetUpdateRect ( NULL, FALSE ) )
	{
		GetUpdateRect ( &rectUpdate, FALSE );
	}
	else
	{
		GetClientRect ( &rectUpdate );
	}

	CPaintDC dc ( this );
	DrawRect ( rectUpdate );
}

void CMapViewerView::DrawRect ( CRect &rectUpdate ) 
{
	BOOL bDrawView = m_bDrawView;
	int nMapCellSize = bDrawView ? MAPCELL_DRAWINFO_SIZE : MAPCELL_TEXTINFO_SIZE;

	CRect rectClient;
	GetClientRect ( &rectClient );
	CDC *pDC = GetDC ();

	CPoint pointPosition = GetScrollPosition ();

	CPoint pointResult;
	pointResult.x = rectClient.right - rectClient.left + pointPosition.x;
	pointResult.y = rectClient.bottom - rectClient.top + pointPosition.y;

	CMainFrame *pFrame = (CMainFrame *)AfxGetMainWnd ();
	pFrame->SetMapPositionsPane ( pointPosition.x / nMapCellSize, 
		pointPosition.y / nMapCellSize, pointResult.x / nMapCellSize, 
		pointResult.y / nMapCellSize );

	SMapDescription *pMapDescription = &((CMapViewerDoc *)GetDocument ())->m_sMapDescription;
	int nMapBorder = pMapDescription->m_nMapBorder;
	int nMapSizeX = pMapDescription->m_nMapSizeX;
	signed char *pMap = ((CMapViewerDoc *)GetDocument ())->m_pMap;

	int nCornerX = ( rectUpdate.left - rectClient.left + pointPosition.x ) / nMapCellSize;
	int nCornerY = ( rectUpdate.top - rectClient.top + pointPosition.y ) / nMapCellSize;
//	int nCornerX = ( pointPosition.x + rectUpdate.x - rectClient.x ) / nMapCellSize;
//	int nCornerY = ( pointPosition.y + rectUpdate.y - rectClient.y ) / nMapCellSize;
	int nSizeX = ( rectUpdate.right - rectUpdate.left + nMapCellSize - 1 ) / nMapCellSize;
	int nSizeY = ( rectUpdate.bottom - rectUpdate.top + nMapCellSize - 1 ) / nMapCellSize;

	int nPositionX = nCornerX * nMapCellSize;
	int nPositionY = nCornerY * nMapCellSize;

	CPoint pointStart;
	CPoint pointEnd;

	int nBorderX = pMapDescription->m_nSizeX;
	int nBorderY = pMapDescription->m_nSizeY;

	CPen cPen ( PS_SOLID, 1, LINE_COLOR );

	pointStart.y = nPositionY - pointPosition.y;
	pointEnd.y = nPositionY + ( nSizeY + 1 ) * nMapCellSize - pointPosition.y;
	int nPosX = nPositionX - pointPosition.x;
	int nX = nCornerX - nMapBorder;
	for ( int nIndexX = nSizeX + 1; nIndexX-- > 0; nX++ )
	{
		pointEnd.x = pointStart.x = nPosX;
		nPosX += nMapCellSize;
		if ( nX % MAP_SQUARE_SIZE == 0 )
		{
			CPen *pOldPen = pDC->SelectObject ( &cPen );
			pDC->MoveTo ( pointStart );
			pDC->LineTo ( pointEnd );
			VERIFY ( pDC->SelectObject ( pOldPen ) == &cPen );
		}
		else
		{
			pDC->MoveTo ( pointStart );
			pDC->LineTo ( pointEnd );
		}
	}

	pointStart.x = nPositionX - pointPosition.x;
	pointEnd.x = nPositionX + ( nSizeX + 1 ) * nMapCellSize - pointPosition.x;
	int nPosY = nPositionY - pointPosition.y;
	int nY = nCornerY - nMapBorder;
	for ( int nIndexY = nSizeY + 1; nIndexY-- > 0; nY++ )
	{
		pointEnd.y = pointStart.y = nPosY;
		nPosY += nMapCellSize;
		if ( nY % MAP_SQUARE_SIZE == 0 )
		{
			CPen *pOldPen = pDC->SelectObject ( &cPen );
			pDC->MoveTo ( pointStart );
			pDC->LineTo ( pointEnd );
			VERIFY ( pDC->SelectObject ( pOldPen ) == &cPen );
		}
		else
		{
			pDC->MoveTo ( pointStart );
			pDC->LineTo ( pointEnd );
		}
	}

	if ( bDrawView )
	{	// kreslené informace
		nBorderX += nMapBorder;
		nBorderY += nMapBorder;
		for ( nIndexX = nSizeX + 1, nX = nCornerX; nIndexX-- > 0; nX++ )
		{
			for ( nIndexY = nSizeY + 1, nY = nCornerY; nIndexY-- > 0; nY++ )
			{
				COLORREF crFill;
				if ( ( nX < nMapBorder ) || ( nX >= nBorderX ) || ( nY < nMapBorder ) || 
					( nY >= nBorderY ) )
				{
					crFill = MAPCELL_NONE_BKCOLOR;
				}
				else
				{
					signed char cMapCellID = pMap[nY * nMapSizeX + nX] & 
						((signed char)~(signed char)0x80);
					ASSERT ( cMapCellID >= 0 );
					if ( cMapCellID < MAX_COLOR )
					{
						crFill = g_aColors[cMapCellID];
					}
					else
					{
						crFill = g_aColors[MAX_COLOR-1];
					}
				}

				pDC->FillSolidRect ( nX * nMapCellSize + 1 - pointPosition.x, 
					nY * nMapCellSize + 1 - pointPosition.y, nMapCellSize - 1, 
					nMapCellSize - 1, crFill );
			}
		}
	}
	else
	{	// textové informace
		nBorderX += nMapBorder;
		nBorderY += nMapBorder;
		pDC->SetBkMode ( TRANSPARENT );
		pDC->SelectObject ( &m_cFont );
		for ( nIndexX = nSizeX + 1, nX = nCornerX; nIndexX-- > 0; nX++ )
		{
			for ( nIndexY = nSizeY + 1, nY = nCornerY; nIndexY-- > 0; nY++ )
			{
				signed char cMapCellID = pMap[nY * nMapSizeX + nX];
				BOOL bMapCellExplored = ( cMapCellID & 0x80 ) ? TRUE : FALSE;
				BOOL bMapCellArea = ( cMapCellID & 0x01 ) ? TRUE : FALSE;
				cMapCellID = cMapCellID & ((signed char)~(signed char)0x80);
				CString str;
				str.Format ( "%d", cMapCellID );

				pDC->FillSolidRect ( nX * nMapCellSize + 1 - pointPosition.x, 
					nY * nMapCellSize + 1 - pointPosition.y, nMapCellSize - 1, 
					nMapCellSize - 1, ( ( nX < nMapBorder ) || ( nX >= nBorderX ) || 
					( nY < nMapBorder ) || ( nY >= nBorderY ) ) ? MAPCELL_NONE_BKCOLOR : 
					( bMapCellExplored ? ( bMapCellArea ? MAPCELL_AREA_EXPLORED_BKCOLOR : 
					MAPCELL_EXPLORED_BKCOLOR ) : ( bMapCellArea ? MAPCELL_AREA_BKCOLOR : 
					MAPCELL_UNEXPLORED_BKCOLOR ) ) );

				pDC->TextOut ( nX * nMapCellSize + MAPCELL_TEXTINFO_OFFSETX - pointPosition.x, 
					nY * nMapCellSize + MAPCELL_TEXTINFO_OFFSETY - pointPosition.y, str );
			}
		}
	}
}

void CMapViewerView::OnDraw ( CDC *pDC ) 
{
}

void CMapViewerView::OnUpdate ( CView *pSender, LPARAM lHint, CObject *pHint ) 
{
	if ( lHint == 1 )
	{
		UpdateScroll ( ( pHint != NULL ) ? *(CPoint *)pHint : CPoint ( 0, 0 ) );

		InvalidateRect ( NULL );
	}
}

void CMapViewerView::UpdateScroll ( CPoint pt ) 
{
	BOOL bDrawView = m_bDrawView;
	int nMapCellSize = bDrawView ? MAPCELL_DRAWINFO_SIZE : MAPCELL_TEXTINFO_SIZE;

	SMapDescription *pMapDescription = &((CMapViewerDoc *)GetDocument ())->m_sMapDescription;

	SetScrollSizes ( MM_TEXT, CSize ( pMapDescription->m_nMapSizeX * nMapCellSize, 
		pMapDescription->m_nMapSizeY * nMapCellSize ), CSize ( nMapCellSize * PAGE_SIZE, 
		nMapCellSize * PAGE_SIZE ), CSize ( nMapCellSize, nMapCellSize ) );
	ScrollToPosition ( CPoint ( pt.x * nMapCellSize, pt.y * nMapCellSize ) );
}

void CMapViewerView::OnTimer(UINT nIDEvent) 
{
	if ( nIDEvent == 2 )
	{
		InvalidateRect ( NULL );
		return;
	}

	CScrollView::OnTimer ( nIDEvent );
}

void CMapViewerView::OnDestroy() 
{
	CScrollView::OnDestroy ();

	KillTimer ( m_nTimerID );
}

BOOL CMapViewerView::OnEraseBkgnd ( CDC *pDC ) 
{
	return TRUE;
}

void CMapViewerView::OnToggleView () 
{
	BOOL bDrawView = m_bDrawView;
	int nMapCellSize = bDrawView ? MAPCELL_DRAWINFO_SIZE : MAPCELL_TEXTINFO_SIZE;
	m_bDrawView = !bDrawView;

	CPoint pt = GetScrollPosition ();
	pt.x /= nMapCellSize;
	pt.y /= nMapCellSize;

	OnUpdate ( NULL, 1, (CObject *)&pt );
}
