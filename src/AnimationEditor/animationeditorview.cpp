// AnimationEditorView.cpp : implementation of the CAnimationEditorView class
//

#include "stdafx.h"
#include "AnimationEditor.h"

#include "AnimationEditorDoc.h"
#include "AnimationEditorView.h"

#include "MainFrm.h"

#include "ViewOptionsDlg.h"
#include "CropFrameDlg.h"

#define BUTTONS_BORDER 20
#define BUTTONS_STARTDELAY 300
#define BUTTONS_NEXTDELAY 100

#define BUFFER_WIDTH 200
#define BUFFER_HEIGHT 200
#define VIEW_WIDTH 640
#define VIEW_HEIGHT 480
#define PAGE_WIDTH 100
#define PAGE_HEIGHT 100
#define LINE_WIDTH 10
#define LINE_HEIGHT 10

#define CROSSHAIR_SIZE 15
#define CROSSHAIR_ALPHA 0x0E0

#define BOUNDINGRECT_COLOR RGB32 ( 100, 128, 255 )

#define REGISTRY_HIVE "Settings"
#define REGISTRY_BACKGROUNDCOLOR "BackgroundColor"
#define REGISTRY_ANIMATIONCOLOR "AnimationColor"
#define REGISTRY_SHOWCROSSHAIR "ShowCrossHair"
#define REGISTRY_CROSSHAIRCOLOR "CrossHairColor"
#define REGISTRY_BACKGROUNDANIMATION "BackgroundAnimation"
#define REGISTRY_DISPLAYBKGASOVERLAY "DisplayBackgroundAsOverlay"
#define REGISTRY_LASTCROPTOLERANCE "LastCropTolerance"
#define REGISTRY_SHOWBOUNDINGRECTANGLE "ShowBoundingRectangle"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

extern CString g_strStartupDirectory;

/////////////////////////////////////////////////////////////////////////////
// CAnimationEditorView

IMPLEMENT_DYNCREATE(CAnimationEditorView, CScrollView)

BEGIN_MESSAGE_MAP(CAnimationEditorView, CScrollView)
	//{{AFX_MSG_MAP(CAnimationEditorView)
	ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_WM_ERASEBKGND()
	ON_COMMAND(ID_VIEW_OPTIONS, OnViewOptions)
	ON_UPDATE_COMMAND_UI(ID_ANIMATION_PLAY, OnUpdateAnimationPlay)
	ON_UPDATE_COMMAND_UI(ID_ANIMATION_STOP, OnUpdateAnimationStop)
	ON_UPDATE_COMMAND_UI(ID_ANIMATION_FIRST, OnUpdateAnimationFirst)
	ON_UPDATE_COMMAND_UI(ID_ANIMATION_LAST, OnUpdateAnimationLast)
	ON_UPDATE_COMMAND_UI(ID_ANIMATION_NEXT, OnUpdateAnimationNext)
	ON_UPDATE_COMMAND_UI(ID_ANIMATION_PREV, OnUpdateAnimationPrev)
	ON_WM_SIZE()
	ON_WM_PAINT()
	ON_COMMAND(ID_ANIMATION_FIRST, OnAnimationFirst)
	ON_COMMAND(ID_ANIMATION_PREV, OnAnimationPrev)
	ON_COMMAND(ID_ANIMATION_NEXT, OnAnimationNext)
	ON_COMMAND(ID_ANIMATION_LAST, OnAnimationLast)
	ON_COMMAND(ID_ANIMATION_PLAY, OnAnimationPlay)
	ON_COMMAND(ID_ANIMATION_STOP, OnAnimationStop)
	ON_WM_TIMER()
	ON_UPDATE_COMMAND_UI(ID_FRAME_MOVELEFT, OnUpdateFrameMoveLeft)
	ON_UPDATE_COMMAND_UI(ID_FRAME_MOVERIGHT, OnUpdateFrameMoveRight)
	ON_UPDATE_COMMAND_UI(ID_FRAME_DELETE, OnUpdateFrameDelete)
	ON_COMMAND(ID_FRAME_MOVELEFT, OnFrameMoveLeft)
	ON_COMMAND(ID_FRAME_MOVERIGHT, OnFrameMoveRight)
	ON_UPDATE_COMMAND_UI(ID_FRAME_INSERT, OnUpdateFrameInsert)
	ON_COMMAND(ID_FRAME_DELETE, OnFrameDelete)
	ON_COMMAND(ID_FRAME_INSERT, OnFrameInsert)
	ON_UPDATE_COMMAND_UI(ID_FRAME_DELAY, OnUpdateFrameDelay)
	ON_UPDATE_COMMAND_UI(ID_ANIMATION_EDITOFFSET, OnUpdateAnimationEditOffset)
	ON_UPDATE_COMMAND_UI(ID_FRAME_EDITOFFSET, OnUpdateFrameEditOffset)
	ON_COMMAND(ID_ANIMATION_EDITOFFSET, OnAnimationEditOffset)
	ON_COMMAND(ID_FRAME_EDITOFFSET, OnFrameEditOffset)
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_UPDATE_COMMAND_UI(ID_FRAME_CROP, OnUpdateFrameCrop)
	ON_COMMAND(ID_FRAME_CROP, OnFrameCrop)
	ON_WM_LBUTTONDBLCLK()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CAnimationEditorView construction/destruction

CAnimationEditorView::CAnimationEditorView()
{
  m_eState = State_Edit;
  m_dwCurrentFrame = 0;
  m_bCreated = false;
  m_dwBackgroundColor = RGB32 ( 0, 0, 0 );
  m_dwAnimationColor = RGB32 ( 220, 0, 0 );
  m_bShowCrossHair = true;
  m_dwCrossHairColor = RGB32 ( 255, 128, 255 );
  m_nFrameTimer = -1;
  m_bUseBackgroundAnimation = false;
  m_bShowMoveButtons = false;
  m_bEditAnimationOffset = true;
  m_nHoverButton = 0; m_nPressedButton = 0;
  m_nMoveTimer = -1;
  m_bDisplayBackgroundAsOverlay = false;
  m_dwLastCropTolerance = 3;
  m_bShowBoundingRectangle = false;
}

CAnimationEditorView::~CAnimationEditorView()
{
}

BOOL CAnimationEditorView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	return CScrollView::PreCreateWindow(cs);
}

/////////////////////////////////////////////////////////////////////////////
// CAnimationEditorView drawing

void CAnimationEditorView::OnDraw(CDC* pDC)
{
  CRect rcUpdate;
  GetClientRect ( &rcUpdate );
  DrawClientRect ( &rcUpdate );
}

void CAnimationEditorView::OnInitialUpdate()
{
	CScrollView::OnInitialUpdate();

	SetScrollSizes(MM_TEXT, CSize ( VIEW_WIDTH * 2, VIEW_HEIGHT * 2 ), CSize ( PAGE_WIDTH, PAGE_HEIGHT ),
    CSize ( LINE_WIDTH, LINE_HEIGHT ) );

  CenterView ();

  // Recompute the position of the move buttons
  RecomputeMoveButtons ();

  m_bCreated = true;
}


// Centres the view
void CAnimationEditorView::CenterView ()
{
  CRect rcClient;
  GetClientRect ( &rcClient );

  ScrollToPosition ( CPoint ( VIEW_WIDTH - ( rcClient.Width () / 2), VIEW_HEIGHT - ( rcClient.Height () / 2 ) ) );
}

// Draws given rect in window coords to the window
void CAnimationEditorView::DrawClientRect ( CRect * pRect )
{
  CPoint pt = GetScrollPosition ();
  // Rect to draw on the screen
  CRect rcScreen ( pRect );
  CRect rcClient;
  GetClientRect ( &rcClient );
  // Rect to draw in view coordinates (relative to view center)
  CRect rcUpdate;
  rcUpdate.left = pRect->left + pt.x - VIEW_WIDTH;
  rcUpdate.right = rcUpdate.left + pRect->Width ();
  rcUpdate.top = pRect->top + pt.y - VIEW_HEIGHT;
  rcUpdate.bottom = rcUpdate.top + pRect->Height ();

  // Move it to the screen coords
  ClientToScreen ( &rcScreen );
  CPoint ptScreenOff ( rcScreen.left - rcUpdate.left, rcScreen.top - rcUpdate.top );

  // Set the clipper
  g_pDDPrimarySurface->SetClipper ( &m_cClipper );

  int nXLeft, nYLeft, nXPos, nYPos;
  CRect rcDraw, rcPaste ( 0, 0, 0, 0 );
  nYLeft = rcUpdate.Height ();
  nYPos = rcUpdate.top;

  while ( nYLeft > 0 )
  {
    rcDraw.top = nYPos;
    if ( nYLeft >= BUFFER_HEIGHT )
    {
      rcDraw.bottom = rcDraw.top + BUFFER_HEIGHT;
    }
    else
    {
      rcDraw.bottom = rcDraw.top + nYLeft;
    }
    nXLeft = rcUpdate.Width ();
    nXPos = rcUpdate.left;

    while ( nXLeft > 0 )
    {
      rcDraw.left = nXPos;
      if ( nXLeft >= BUFFER_WIDTH )
      {
        rcDraw.right = rcDraw.left + BUFFER_WIDTH;
      }
      else
      {
        rcDraw.right = rcDraw.left + nXLeft;
      }

      // Draw the rect to the buffer
      DrawRect ( &rcDraw, &m_cDrawBuffer );

      // Draw it on the screen
      rcPaste.right = rcDraw.Width ();
      rcPaste.bottom = rcDraw.Height ();
      g_pDDPrimarySurface->Paste ( rcDraw.left + ptScreenOff.x, rcDraw.top + ptScreenOff.y, &m_cDrawBuffer, &rcPaste );

      nXPos += rcDraw.Width ();
      nXLeft -= rcDraw.Width ();
    }
    nYPos += rcDraw.Height ();
    nYLeft -= rcDraw.Height ();
  }
}

// Draws given rectangle to the given surface
// The rectangle is given in coords relative to the center of the view (which is always 0, 0 )
void CAnimationEditorView::DrawRect ( CRect * pRect, CDDrawSurface * pSurface )
{
  CAAnimation * pAnimation = GetDocument ()->GetAnimation ();
  ASSERT_VALID ( pAnimation );
  CRect rcDestinationRect ( 0, 0, pRect->Width (), pRect->Height () );

  // Draw background
  {
    pSurface->Fill ( m_dwBackgroundColor, &rcDestinationRect );

    if ( m_bUseBackgroundAnimation && !m_bDisplayBackgroundAsOverlay )
    {
      const CAFrame * pFrame = m_cBackgroundAnimation.GetFrame ( 0 );
      if ( pFrame != NULL )
      {
        CDDrawSurface * pFrameSurface = pFrame->GetSurface ();

        pSurface->Paste ( -m_cBackgroundAnimation.GetXOffset () - pFrame->GetXOffset () - pRect->left,
          -m_cBackgroundAnimation.GetYOffset () - pFrame->GetYOffset () - pRect->top, pFrameSurface );
      }
    }
  }

  // Draw the current frame
  if ( pAnimation->GetFrameCount () > 0 )
  {
    const CAFrame * pFrame = pAnimation->GetFrame ( m_dwCurrentFrame );
    if ( pFrame == NULL ) goto NoDraw;
    CDDrawSurface * pFrameSurface = pFrame->GetSurface ();

    pSurface->Paste ( -pAnimation->GetXOffset () - pFrame->GetXOffset () - pRect->left,
      -pAnimation->GetYOffset () - pFrame->GetYOffset () - pRect->top, pFrameSurface );

    // Draw bounding rectangle (if any)
    if ( m_bShowBoundingRectangle )
    {
      CRect rcFill;
      rcFill.left = -pAnimation->GetXOffset () - pFrame->GetXOffset () - pRect->left;
      rcFill.top = -pAnimation->GetYOffset () - pFrame->GetYOffset () - pRect->top;
      rcFill.right = rcFill.left + pFrameSurface->GetWidth ();
      rcFill.bottom = rcFill.top + 1;
      pSurface->Fill ( BOUNDINGRECT_COLOR, &rcFill );

      rcFill.right = rcFill.left + 1;
      rcFill.bottom = rcFill.top + pFrameSurface->GetHeight ();
      pSurface->Fill ( BOUNDINGRECT_COLOR, &rcFill );

      rcFill.top = rcFill.bottom;
      rcFill.bottom = rcFill.top + 1;
      rcFill.right = rcFill.left + pFrameSurface->GetWidth ();
      pSurface->Fill ( BOUNDINGRECT_COLOR, &rcFill );

      rcFill.top = -pAnimation->GetYOffset () - pFrame->GetYOffset () - pRect->top;
      rcFill.left = rcFill.right;
      rcFill.right = rcFill.left + 1;
      pSurface->Fill ( BOUNDINGRECT_COLOR, &rcFill );
    }
  }

NoDraw:
  ;
  // Draw the bkg as overlay :-)
  {
    if ( m_bUseBackgroundAnimation && m_bDisplayBackgroundAsOverlay )
    {
      const CAFrame * pFrame = m_cBackgroundAnimation.GetFrame ( 0 );
      if ( pFrame != NULL )
      {
        CDDrawSurface * pFrameSurface = pFrame->GetSurface ();

        pSurface->Paste ( -m_cBackgroundAnimation.GetXOffset () - pFrame->GetXOffset () - pRect->left,
          -m_cBackgroundAnimation.GetYOffset () - pFrame->GetYOffset () - pRect->top, pFrameSurface );
      }
    }
  }

  // Draw the cross hair
  if ( m_bShowCrossHair )
  {
    CRect rcCrossHair;
    rcCrossHair.left = -(CROSSHAIR_SIZE/2);
    rcCrossHair.top = -(CROSSHAIR_SIZE/2);
    rcCrossHair.right = rcCrossHair.left + CROSSHAIR_SIZE;
    rcCrossHair.bottom = rcCrossHair.top + CROSSHAIR_SIZE;

    // if there is no intersection -> don't fraw it
    rcCrossHair.IntersectRect ( &rcCrossHair, pRect );
    if ( !rcCrossHair.IsRectEmpty () ) 
    {
      pSurface->Paste ( rcCrossHair.left - pRect->left, -pRect->top, &m_cCrossHairHorizontalLine );
      pSurface->Paste ( -pRect->left, rcCrossHair.top - pRect->top, &m_cCrossHairVerticalLine );
    }
  }

  // Draw move buttons
  if ( (m_eState == State_Edit) && (pAnimation->GetFrameCount () > 0 ) && m_bShowMoveButtons )
  {
    if ( m_nHoverButton == 1 )
      pSurface->Paste ( m_ptMoveUpButton.x - pRect->left - 1, m_ptMoveUpButton.y - pRect->top - 1, &m_cMoveUpButton );
    else
      pSurface->Paste ( m_ptMoveUpButton.x - pRect->left, m_ptMoveUpButton.y - pRect->top, &m_cMoveUpButton );

    if ( m_nHoverButton == 2 )
      pSurface->Paste ( m_ptMoveLeftButton.x - pRect->left - 1, m_ptMoveLeftButton.y - pRect->top - 1, &m_cMoveLeftButton );
    else
      pSurface->Paste ( m_ptMoveLeftButton.x - pRect->left, m_ptMoveLeftButton.y - pRect->top, &m_cMoveLeftButton );

    if ( m_nHoverButton == 3 )
      pSurface->Paste ( m_ptMoveDownButton.x - pRect->left - 1, m_ptMoveDownButton.y - pRect->top - 1, &m_cMoveDownButton );
    else
      pSurface->Paste ( m_ptMoveDownButton.x - pRect->left, m_ptMoveDownButton.y - pRect->top, &m_cMoveDownButton );

    if ( m_nHoverButton == 4 )
      pSurface->Paste ( m_ptMoveRightButton.x - pRect->left - 1, m_ptMoveRightButton.y - pRect->top - 1, &m_cMoveRightButton );
    else
      pSurface->Paste ( m_ptMoveRightButton.x - pRect->left, m_ptMoveRightButton.y - pRect->top, &m_cMoveRightButton );
  }
}

// Sets current frame index
void CAnimationEditorView::SetCurrentFrame ( DWORD dwCurrentFrame )
{
  CAAnimation * pAnimation = GetDocument ()->GetAnimation ();

  if ( dwCurrentFrame >= pAnimation->GetFrameCount () )
    dwCurrentFrame = 0;

  // Copy the current frame number
  m_dwCurrentFrame = dwCurrentFrame;

  // Set it to the toolbar
  g_pMainFrame->SetCurrentFrameForControl ( m_dwCurrentFrame );
  if ( pAnimation->GetFrameCount () > 0 )
    g_pMainFrame->SetFrameDelayForControl ( pAnimation->GetFrame ( m_dwCurrentFrame )->GetFrameDelay () );

  // Recompute the position of the move buttons
  RecomputeMoveButtons ();
  // Redraw us
  Invalidate ();
}

// Sets timer for next frame in the animation (used in preview mode)
void CAnimationEditorView::SetNextFrameTimer ()
{
  ASSERT ( m_eState == State_Preview );

  // Kill previous timer (if any)
  if ( m_nFrameTimer != -1 )
    KillTimer ( m_nFrameTimer );
  // Set the new one
  CAAnimation * pAnimation = GetDocument ()->GetAnimation ();
  if ( pAnimation->GetFrameCount () == 0 ) return;
  DWORD dwDelay;
  dwDelay = pAnimation->GetFrame ( m_dwCurrentFrame )->GetFrameDelay ();
  if ( dwDelay == 0 )
    dwDelay = pAnimation->GetDefaultFrameDelay ();
  m_nFrameTimer = SetTimer ( 1, dwDelay, NULL );
}


// Recomputes move buttons positions
void CAnimationEditorView::RecomputeMoveButtons ()
{
  CAAnimation * pAnimation = GetDocument ()->GetAnimation ();
  // If the animation is empty, nothing to do
  if ( pAnimation->GetFrameCount () == 0 ) return;

  // Elseway, get the current frame position
  CRect rcFrame;
  const CAFrame * pFrame = pAnimation->GetFrame ( m_dwCurrentFrame );
  if ( pFrame == NULL ) return;
  rcFrame.left = -(pFrame->GetXOffset ()) - (pAnimation->GetXOffset () );
  rcFrame.top = -(pFrame->GetYOffset ()) - (pAnimation->GetYOffset () );
  rcFrame.right = rcFrame.left + pFrame->GetSurface ()->GetWidth ();
  rcFrame.bottom = rcFrame.top + pFrame->GetSurface ()->GetHeight ();

  // Resize the frame rect to contain some borders
  rcFrame.left -= BUTTONS_BORDER;
  rcFrame.top -= BUTTONS_BORDER;
  rcFrame.right += BUTTONS_BORDER;
  rcFrame.bottom += BUTTONS_BORDER;

  // Now set the buttons positions
  m_ptMoveUpButton.x = rcFrame.left + (rcFrame.Width () - (int)(m_cMoveUpButton.GetWidth ())) / 2;
  m_ptMoveUpButton.y = rcFrame.top - m_cMoveUpButton.GetHeight ();

  m_ptMoveLeftButton.x = rcFrame.left - m_cMoveLeftButton.GetWidth ();
  m_ptMoveLeftButton.y = rcFrame.top + ( rcFrame.Height () - (int)(m_cMoveLeftButton.GetHeight ()) ) / 2;

  m_ptMoveDownButton.x = rcFrame.left + (rcFrame.Width () - (int)(m_cMoveUpButton.GetWidth ()) ) / 2;
  m_ptMoveDownButton.y = rcFrame.bottom;

  m_ptMoveRightButton.x = rcFrame.right;
  m_ptMoveRightButton.y = rcFrame.top + ( rcFrame.Height () - (int)(m_cMoveLeftButton.GetHeight ()) ) / 2;
}

// Returns button under the mouse position
// Zero - no, 1, 2, 3, 4 - Up, Left, Down, Right
int CAnimationEditorView::GetButtonUnderMouse ()
{
  CPoint pt;
  GetCursorPos ( &pt );
  ScreenToClient ( &pt );
  CPoint ptScroll = GetScrollPosition ();
  pt.x = pt.x + ptScroll.x - VIEW_WIDTH;
  pt.y = pt.y + ptScroll.y - VIEW_HEIGHT;

  CRect rcButton;
  rcButton.SetRect ( m_ptMoveUpButton.x, m_ptMoveUpButton.y,
    m_ptMoveUpButton.x + m_cMoveUpButton.GetWidth (), m_ptMoveUpButton.y + m_cMoveUpButton.GetHeight () );
  if ( rcButton.PtInRect ( pt ) ) return 1;
  rcButton.SetRect ( m_ptMoveLeftButton.x, m_ptMoveLeftButton.y,
    m_ptMoveLeftButton.x + m_cMoveLeftButton.GetWidth (), m_ptMoveLeftButton.y + m_cMoveLeftButton.GetHeight () );
  if ( rcButton.PtInRect ( pt ) ) return 2;
  rcButton.SetRect ( m_ptMoveDownButton.x, m_ptMoveDownButton.y,
    m_ptMoveDownButton.x + m_cMoveDownButton.GetWidth (), m_ptMoveDownButton.y + m_cMoveDownButton.GetHeight () );
  if ( rcButton.PtInRect ( pt ) ) return 3;
  rcButton.SetRect ( m_ptMoveRightButton.x, m_ptMoveRightButton.y,
    m_ptMoveRightButton.x + m_cMoveRightButton.GetWidth (), m_ptMoveRightButton.y + m_cMoveRightButton.GetHeight () );
  if ( rcButton.PtInRect ( pt ) ) return 4;

  return 0;
}

// Updates state of the buttons from mouse position
void CAnimationEditorView::UpdateHoverButton ()
{
  int nOld = m_nHoverButton;

  if ( !m_bShowMoveButtons )
  {
    m_nHoverButton = 0;
    if ( m_nHoverButton != nOld )
      Invalidate ();
  }
  else
  {
    m_nHoverButton = GetButtonUnderMouse ();
    if ( m_nHoverButton == m_nPressedButton )
    {
      m_nHoverButton = 0;
    }
    else
    {
      if ( m_nPressedButton != 0 )
        m_nHoverButton = m_nPressedButton;
    }
    if ( m_nHoverButton != nOld )
      Invalidate ();
  }
}

// Performs the move operation
void CAnimationEditorView::DoMoving ()
{
  int nButton = GetButtonUnderMouse ();
  if ( nButton == 0 ) return;

  CAAnimation * pAnimation = GetDocument ()->GetAnimation ();
  if ( m_bEditAnimationOffset )
  {
    switch ( nButton )
    {
    case 1: pAnimation->SetYOffset ( pAnimation->GetYOffset () + 1 ); break;
    case 2: pAnimation->SetXOffset ( pAnimation->GetXOffset () + 1 ); break;
    case 3: pAnimation->SetYOffset ( pAnimation->GetYOffset () - 1 ); break;
    case 4: pAnimation->SetXOffset ( pAnimation->GetXOffset () - 1 ); break;
    }
  }
  else
  {
    switch ( nButton )
    {
    case 1: pAnimation->SetFrameYOffset ( m_dwCurrentFrame, pAnimation->GetFrame ( m_dwCurrentFrame )->GetYOffset () + 1 ); break;
    case 2: pAnimation->SetFrameXOffset ( m_dwCurrentFrame, pAnimation->GetFrame ( m_dwCurrentFrame )->GetXOffset () + 1 ); break;
    case 3: pAnimation->SetFrameYOffset ( m_dwCurrentFrame, pAnimation->GetFrame ( m_dwCurrentFrame )->GetYOffset () - 1 ); break;
    case 4: pAnimation->SetFrameXOffset ( m_dwCurrentFrame, pAnimation->GetFrame ( m_dwCurrentFrame )->GetXOffset () - 1 ); break;
    }
  }

  RecomputeMoveButtons ();
  Invalidate ();
  UpdateHoverButton ();
  GetDocument ()->SetModifiedFlag ();
}

/////////////////////////////////////////////////////////////////////////////
// CAnimationEditorView diagnostics

#ifdef _DEBUG
void CAnimationEditorView::AssertValid() const
{
	CScrollView::AssertValid();
}

void CAnimationEditorView::Dump(CDumpContext& dc) const
{
	CScrollView::Dump(dc);
}

CAnimationEditorDoc* CAnimationEditorView::GetDocument() // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CAnimationEditorDoc)));
	return (CAnimationEditorDoc*)m_pDocument;
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CAnimationEditorView message handlers

int CAnimationEditorView::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CScrollView::OnCreate(lpCreateStruct) == -1)
		return -1;

  // Create our clipper
  m_cClipper.Create ( this );
  // Create our buffer surface for drawing without flickering
  m_cDrawBuffer.SetWidth ( BUFFER_WIDTH );
  m_cDrawBuffer.SetHeight ( BUFFER_HEIGHT );
  m_cDrawBuffer.Create ();

  // Create cross hair buffers
  {
    m_cCrossHairHorizontalLine.SetWidth ( CROSSHAIR_SIZE );
    m_cCrossHairHorizontalLine.SetHeight ( 1 );
    m_cCrossHairHorizontalLine.SetAlphaChannel ( TRUE );
    m_cCrossHairHorizontalLine.Create ();
    // Fill the alpha channel
    LPBYTE pAlpha = m_cCrossHairHorizontalLine.GetAlphaChannel ( FALSE );
    memset ( pAlpha, CROSSHAIR_ALPHA, CROSSHAIR_SIZE * 1 );
    m_cCrossHairHorizontalLine.ReleaseAlphaChannel ( TRUE );
  }

  {
    m_cCrossHairVerticalLine.SetWidth ( 1 );
    m_cCrossHairVerticalLine.SetHeight ( CROSSHAIR_SIZE );
    m_cCrossHairVerticalLine.SetAlphaChannel ( TRUE );
    m_cCrossHairVerticalLine.Create ();
    // Fill the alpha channel
    LPBYTE pAlpha = m_cCrossHairVerticalLine.GetAlphaChannel ( FALSE );
    memset ( pAlpha, CROSSHAIR_ALPHA, CROSSHAIR_SIZE * 1 );
    m_cCrossHairVerticalLine.ReleaseAlphaChannel ( TRUE );
  }

  // Create buttons
  {
    CDataArchive cArchive;
    cArchive.Create ( g_strStartupDirectory + "\\Graphics.dat", CArchiveFile::modeRead, CDataArchiveInfo::archiveNone );
    m_cMoveUpButton.Create ( cArchive.CreateFile ( "MoveUp.tga" ) );
    m_cMoveLeftButton.Create ( cArchive.CreateFile ( "MoveLeft.tga" ) );
    m_cMoveRightButton.Create ( cArchive.CreateFile ( "MoveRight.tga" ) );
    m_cMoveDownButton.Create ( cArchive.CreateFile ( "MoveDown.tga" ) );
  }

  // Set our current frame to 0
  SetCurrentFrame ( 0 );

  // Load registry settings
  CWinApp * pApp = AfxGetApp ();
  m_dwBackgroundColor = pApp->GetProfileInt ( REGISTRY_HIVE, REGISTRY_BACKGROUNDCOLOR, m_dwBackgroundColor );
  m_dwAnimationColor = pApp->GetProfileInt ( REGISTRY_HIVE, REGISTRY_ANIMATIONCOLOR, m_dwAnimationColor );
  m_bShowCrossHair = pApp->GetProfileInt ( REGISTRY_HIVE, REGISTRY_SHOWCROSSHAIR, 1 ) == 1;
  m_dwCrossHairColor = pApp->GetProfileInt ( REGISTRY_HIVE, REGISTRY_CROSSHAIRCOLOR, m_dwCrossHairColor );
  m_strBackgroundAnimationFilePath = pApp->GetProfileString ( REGISTRY_HIVE, REGISTRY_BACKGROUNDANIMATION, m_strBackgroundAnimationFilePath );
  m_bDisplayBackgroundAsOverlay = pApp->GetProfileInt ( REGISTRY_HIVE, REGISTRY_DISPLAYBKGASOVERLAY, m_bDisplayBackgroundAsOverlay ? 1 : 0 ) == 1;
  m_dwLastCropTolerance = pApp->GetProfileInt ( REGISTRY_HIVE, REGISTRY_LASTCROPTOLERANCE, m_dwLastCropTolerance );
  m_bShowBoundingRectangle = pApp->GetProfileInt ( REGISTRY_HIVE, REGISTRY_SHOWBOUNDINGRECTANGLE, m_bShowBoundingRectangle ? 1 : 0 ) == 1;

  // Try to load the background animation
  try
  {
    CArchiveFile cFile;
    m_cBackgroundAnimation.Delete ();
    cFile = CDataArchive::GetRootArchive ()->CreateFile ( m_strBackgroundAnimationFilePath, CArchiveFile::modeReadWrite | CArchiveFile::shareDenyNone );
    m_cBackgroundAnimation.Create ( cFile );
    m_bUseBackgroundAnimation = true;
    m_cBackgroundAnimation.SetDestinationColor ( m_dwAnimationColor );
  }
  catch ( CException * e )
  {
    e->Delete ();
    m_bUseBackgroundAnimation = false;
  }

  // Fill the cross hair line
  m_cCrossHairHorizontalLine.Fill ( m_dwCrossHairColor );
  m_cCrossHairVerticalLine.Fill ( m_dwCrossHairColor );

	return 0;
}

void CAnimationEditorView::OnDestroy() 
{
	CScrollView::OnDestroy();
	
  // Write registry settings
  CWinApp * pApp = AfxGetApp ();
  pApp->WriteProfileInt ( REGISTRY_HIVE, REGISTRY_BACKGROUNDCOLOR, m_dwBackgroundColor );
  pApp->WriteProfileInt ( REGISTRY_HIVE, REGISTRY_ANIMATIONCOLOR, m_dwAnimationColor );
  pApp->WriteProfileInt ( REGISTRY_HIVE, REGISTRY_SHOWCROSSHAIR, m_bShowCrossHair ? 1 : 0 );
  pApp->WriteProfileInt ( REGISTRY_HIVE, REGISTRY_CROSSHAIRCOLOR, m_dwCrossHairColor );
  pApp->WriteProfileString ( REGISTRY_HIVE, REGISTRY_BACKGROUNDANIMATION, m_strBackgroundAnimationFilePath );
  pApp->WriteProfileInt ( REGISTRY_HIVE, REGISTRY_DISPLAYBKGASOVERLAY, m_bDisplayBackgroundAsOverlay ? 1 : 0 );
  pApp->WriteProfileInt ( REGISTRY_HIVE, REGISTRY_LASTCROPTOLERANCE, m_dwLastCropTolerance );
  pApp->WriteProfileInt ( REGISTRY_HIVE, REGISTRY_SHOWBOUNDINGRECTANGLE, m_bShowBoundingRectangle ? 1 : 0 );

  m_cBackgroundAnimation.Delete ();

  // Delete buttons
  m_cMoveUpButton.Delete (); m_cMoveLeftButton.Delete ();
  m_cMoveRightButton.Delete (); m_cMoveDownButton.Delete ();

  // Delete buffers
  m_cCrossHairHorizontalLine.Delete ();
  m_cCrossHairVerticalLine.Delete ();
  m_cDrawBuffer.Delete ();

	// Destroy our clipper
  m_cClipper.Delete ();
}

BOOL CAnimationEditorView::OnEraseBkgnd(CDC* pDC) 
{
  return TRUE;
}

void CAnimationEditorView::OnViewOptions() 
{
	CViewOptionsDlg dlg;
  CAAnimation * pAnimation = GetDocument ()->GetAnimation ();

  dlg.m_dwBackgroundColor = m_dwBackgroundColor;
  dlg.m_dwAnimationColor = m_dwAnimationColor;
  dlg.m_bShowCrossHair = (BOOL)m_bShowCrossHair;
  dlg.m_dwCrossHairColor = m_dwCrossHairColor;
  dlg.m_strBkgAnimation = m_strBackgroundAnimationFilePath;
  dlg.m_bDisplayBackgroundAsOverlay = m_bDisplayBackgroundAsOverlay;
  dlg.m_bShowBoundingRectangle = m_bShowBoundingRectangle;

  if ( dlg.DoModal () == IDOK )
  {
    // Reload the background animation
    if ( dlg.m_strBkgAnimation != m_strBackgroundAnimationFilePath )
    {
      m_strBackgroundAnimationFilePath = dlg.m_strBkgAnimation;
      m_bUseBackgroundAnimation = false;
      try
      {
        m_cBackgroundAnimation.Delete ();
        CArchiveFile cFile;
        cFile = CDataArchive::GetRootArchive ()->CreateFile ( m_strBackgroundAnimationFilePath,  CArchiveFile::modeReadWrite | CArchiveFile::shareDenyNone );
        m_cBackgroundAnimation.Create ( cFile );
        m_bUseBackgroundAnimation = true;
      }
      catch ( CException * e )
      {
        e->Delete ();
        m_bUseBackgroundAnimation = false;
      }
    }
    m_dwBackgroundColor = dlg.m_dwBackgroundColor;
    m_dwAnimationColor = dlg.m_dwAnimationColor;
    pAnimation->SetDestinationColor ( m_dwAnimationColor );
    if ( m_bUseBackgroundAnimation )
    {
      m_cBackgroundAnimation.SetDestinationColor ( m_dwAnimationColor );
    }
    m_bShowCrossHair = dlg.m_bShowCrossHair ? true : false;
    m_dwCrossHairColor = dlg.m_dwCrossHairColor;
    // Must refill the buffers
    {
      m_cCrossHairHorizontalLine.Fill ( m_dwCrossHairColor );
      m_cCrossHairVerticalLine.Fill ( m_dwCrossHairColor );
    }
    m_bDisplayBackgroundAsOverlay = dlg.m_bDisplayBackgroundAsOverlay ? true : false;
    m_bShowBoundingRectangle = dlg.m_bShowBoundingRectangle ? true : false;
    Invalidate ();
  }
}

void CAnimationEditorView::OnUpdateAnimationPlay(CCmdUI* pCmdUI) 
{ if ( GetDocument ()->GetAnimation ()->GetFrameCount () == 0 ) pCmdUI->Enable ( FALSE );
  else pCmdUI->Enable ( m_eState == State_Edit );	}

void CAnimationEditorView::OnUpdateAnimationStop(CCmdUI* pCmdUI) 
{ pCmdUI->Enable ( m_eState == State_Preview );	}

void CAnimationEditorView::OnUpdateAnimationFirst(CCmdUI* pCmdUI) 
{ if ( m_eState != State_Edit ) pCmdUI->Enable ( FALSE ); else pCmdUI->Enable ( m_dwCurrentFrame > 0 ); }

void CAnimationEditorView::OnUpdateAnimationLast(CCmdUI* pCmdUI) 
{ CAAnimation * pAnimation = GetDocument ()->GetAnimation ();
  if ( m_eState != State_Edit ) pCmdUI->Enable ( FALSE );
  else pCmdUI->Enable ( m_dwCurrentFrame + 1 < pAnimation->GetFrameCount () ); }

void CAnimationEditorView::OnUpdateAnimationPrev(CCmdUI* pCmdUI) 
{ if ( m_eState != State_Edit ) pCmdUI->Enable ( FALSE ); else pCmdUI->Enable ( m_dwCurrentFrame > 0 ); }

void CAnimationEditorView::OnUpdateAnimationNext(CCmdUI* pCmdUI) 
{ CAAnimation * pAnimation = GetDocument ()->GetAnimation ();
  if ( m_eState != State_Edit ) pCmdUI->Enable ( FALSE );
  else pCmdUI->Enable ( m_dwCurrentFrame + 1 < pAnimation->GetFrameCount () ); }



void CAnimationEditorView::OnSize(UINT nType, int cx, int cy) 
{
	CScrollView::OnSize(nType, cx, cy);
	
	if ( m_bCreated )
    CenterView ();
}

void CAnimationEditorView::OnPaint() 
{
  CRect rcUpdate;
  if ( GetUpdateRect ( NULL ) )
  {
    GetUpdateRect ( &rcUpdate );
  }
  else
  {
    GetClientRect ( &rcUpdate );
  }

	CPaintDC dc(this); // device context for painting

  DrawClientRect ( &rcUpdate );
}

void CAnimationEditorView::OnAnimationFirst() 
{
  if ( m_dwCurrentFrame == 0 ) return;
  SetCurrentFrame ( 0 );
}

void CAnimationEditorView::OnAnimationPrev() 
{
  if ( m_dwCurrentFrame == 0 ) return;
  SetCurrentFrame ( m_dwCurrentFrame - 1 );
}

void CAnimationEditorView::OnAnimationNext() 
{
  if ( m_dwCurrentFrame == GetDocument()->GetAnimation ()->GetFrameCount () - 1 ) return;
  SetCurrentFrame ( m_dwCurrentFrame + 1 );
}

void CAnimationEditorView::OnAnimationLast() 
{
  if ( m_dwCurrentFrame == GetDocument()->GetAnimation ()->GetFrameCount () - 1 ) return;
  SetCurrentFrame ( GetDocument ()->GetAnimation ()->GetFrameCount () - 1 );	
}

void CAnimationEditorView::OnAnimationPlay() 
{
	if ( m_eState != State_Edit ) return;

  CAAnimation * pAnim = GetDocument ()->GetAnimation ();
  if ( pAnim->GetFrameCount () == 0 ) return;

  if ( pAnim->GetLoop () == false )
    SetCurrentFrame ( 0 );

  // Change the state
  m_eState = State_Preview;

  // Start the timer
  SetNextFrameTimer ();
}

void CAnimationEditorView::OnAnimationStop() 
{
	if ( m_eState != State_Preview ) return;

  // Kill timer
  KillTimer ( m_nFrameTimer );

  // Change the state
  m_eState = State_Edit;
}

void CAnimationEditorView::OnTimer(UINT nIDEvent) 
{
  CAAnimation * pAnimation = GetDocument ()->GetAnimation ();
  switch ( nIDEvent )
  {
  case 1:
    {
      // Step to the next frame
      DWORD dwCurrentFrame = m_dwCurrentFrame + 1;
      if ( dwCurrentFrame >= pAnimation->GetFrameCount () )
      {
        if ( pAnimation->GetLoop () )
        {
          dwCurrentFrame = 0;
        }
        else
        {
          dwCurrentFrame = m_dwCurrentFrame;
          KillTimer ( m_nFrameTimer );
          m_eState = State_Edit;
          break;
        }
      }
      SetCurrentFrame ( dwCurrentFrame );
      SetNextFrameTimer ();
      Invalidate ();
    }
    break;

  case 2:
    // Move anim/frame
    DoMoving ();
    KillTimer ( m_nMoveTimer );
    m_nMoveTimer = SetTimer ( 2, BUTTONS_NEXTDELAY, NULL );
    break;
  }
  
	CScrollView::OnTimer(nIDEvent);
}

void CAnimationEditorView::OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint) 
{
  switch ( lHint )
  {
  case CAnimationEditorDoc::Update_NoSpecial:
    // Just call the base class
    CScrollView::OnUpdate ( pSender, lHint, pHint );
    break;

  case CAnimationEditorDoc::Update_NewAnimation:
    // New animation is loaded
    m_dwCurrentFrame = 0;
    // We must set the destination color
    GetDocument ()->GetAnimation ()->SetDestinationColor ( m_dwAnimationColor );
    // Recompute the position of the move buttons
    RecomputeMoveButtons ();
    // And call the base class implementation
    CScrollView::OnUpdate ( pSender, lHint, pHint );
    break;
  }
}

void CAnimationEditorView::OnUpdateFrameMoveLeft(CCmdUI* pCmdUI) 
{ if ( m_eState != State_Edit ) pCmdUI->Enable ( FALSE ); else pCmdUI->Enable ( m_dwCurrentFrame > 0 ); }

void CAnimationEditorView::OnUpdateFrameMoveRight(CCmdUI* pCmdUI) 
{ if ( m_eState != State_Edit ) pCmdUI->Enable ( FALSE ); else pCmdUI->Enable ( (m_dwCurrentFrame + 1) < GetDocument ()->GetAnimation ()->GetFrameCount () ); }

void CAnimationEditorView::OnUpdateFrameDelete(CCmdUI* pCmdUI) 
{ if ( m_eState != State_Edit ) pCmdUI->Enable ( FALSE ); else pCmdUI->Enable ( GetDocument()->GetAnimation ()->GetFrameCount () > 0 ); }

void CAnimationEditorView::OnUpdateFrameInsert(CCmdUI* pCmdUI) 
{ pCmdUI->Enable ( m_eState == State_Edit ); }


void CAnimationEditorView::OnFrameMoveLeft() 
{
  if ( m_dwCurrentFrame == 0 ) return;
  GetDocument ()->GetAnimation ()->MoveFrameLeft ( m_dwCurrentFrame );
  SetCurrentFrame ( m_dwCurrentFrame - 1 );
  GetDocument ()->SetModifiedFlag ();
}

void CAnimationEditorView::OnFrameMoveRight() 
{
  CAAnimation *pAnimation  = GetDocument ()->GetAnimation ();
  if ( m_dwCurrentFrame + 1 >= pAnimation->GetFrameCount () ) return;
  pAnimation->MoveFrameRight ( m_dwCurrentFrame );
  SetCurrentFrame ( m_dwCurrentFrame + 1 );
  GetDocument ()->SetModifiedFlag ();
}


void CAnimationEditorView::OnFrameDelete() 
{
  CAAnimation *pAnimation  = GetDocument ()->GetAnimation ();
  if ( AfxMessageBox ( IDS_DELETEFRAMEQUESTION, MB_YESNO ) != IDYES ) return;
  pAnimation->DeleteFrame ( m_dwCurrentFrame );
  if ( m_dwCurrentFrame >= pAnimation->GetFrameCount () )
  {
    if ( m_dwCurrentFrame > 0 ) m_dwCurrentFrame--;
  }
  SetCurrentFrame ( m_dwCurrentFrame );
  GetDocument ()->SetModifiedFlag ();
}

void CAnimationEditorView::OnFrameInsert() 
{
  CString strFilter, strExt, strTitle;
  strFilter.LoadString ( IDS_FRAMEFILTER );
  strExt.LoadString ( IDS_FRAMEEXT );
  strTitle.LoadString ( IDS_INSERTFRAMETITLE );

  CFileDialog dlg ( TRUE, strExt, NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, strFilter, this );
  dlg.m_ofn.lpstrTitle = strTitle;

  if ( dlg.DoModal () != IDOK ) return;

  try
  {
    CAAnimation *pAnimation  = GetDocument ()->GetAnimation ();
    CArchiveFile cFile;
    cFile = CDataArchive::GetRootArchive ()->CreateFile ( dlg.GetPathName (), CArchiveFile::modeReadWrite | CArchiveFile::shareDenyNone );

    // Insert the frame
    DWORD dwBefore = m_dwCurrentFrame + 1;
    if ( pAnimation->GetFrameCount () == 0 ) dwBefore = 0;
    pAnimation->InsertFrame ( cFile, dwBefore );

    SetCurrentFrame ( m_dwCurrentFrame + 1 );
    GetDocument ()->SetModifiedFlag ();
  }
  catch ( CException * e )
  {
    e->ReportError ();
    e->Delete ();
  }
}

void CAnimationEditorView::OnUpdateFrameDelay(CCmdUI* pCmdUI) 
{ pCmdUI->Enable ( ( GetDocument ()->GetAnimation ()->GetFrameCount () > 0 ) && m_eState == State_Edit ); }

void CAnimationEditorView::OnUpdateAnimationEditOffset(CCmdUI* pCmdUI) 
{ pCmdUI->Enable ( ( GetDocument ()->GetAnimation ()->GetFrameCount () > 0 ) && m_eState == State_Edit );
  pCmdUI->SetCheck ( (m_bShowMoveButtons && m_bEditAnimationOffset) ? 1 : 0 ); }

void CAnimationEditorView::OnUpdateFrameEditOffset(CCmdUI* pCmdUI) 
{ pCmdUI->Enable ( ( GetDocument ()->GetAnimation ()->GetFrameCount () > 0 ) && m_eState == State_Edit );
  pCmdUI->SetCheck ( (m_bShowMoveButtons && !m_bEditAnimationOffset) ? 1 : 0 ); }

void CAnimationEditorView::OnAnimationEditOffset() 
{ if ( !m_bEditAnimationOffset ){ m_bEditAnimationOffset = true; m_bShowMoveButtons = true; }
  else m_bShowMoveButtons = !m_bShowMoveButtons;
  Invalidate (); }

void CAnimationEditorView::OnFrameEditOffset() 
{ if ( m_bEditAnimationOffset ){ m_bEditAnimationOffset = false; m_bShowMoveButtons = true; }
  else m_bShowMoveButtons = !m_bShowMoveButtons;
  Invalidate (); }

void CAnimationEditorView::OnLButtonDown(UINT nFlags, CPoint point) 
{
  if ( m_bShowMoveButtons )
  {
    // Find the button under the mouse
    int nButton = GetButtonUnderMouse ();

    if ( nButton != 0 )
    {
      m_nPressedButton = nButton;
      SetCapture ();
      UpdateHoverButton ();
      m_nMoveTimer = SetTimer ( 2, BUTTONS_STARTDELAY, NULL );
      DoMoving ();
    }
  }
	
	CScrollView::OnLButtonDown(nFlags, point);
}

void CAnimationEditorView::OnLButtonDblClk(UINT nFlags, CPoint point) 
{
  OnLButtonDown ( nFlags, point );
	
	CScrollView::OnLButtonDblClk(nFlags, point);
}

void CAnimationEditorView::OnLButtonUp(UINT nFlags, CPoint point) 
{
  if ( m_nPressedButton != 0 )
  {
    KillTimer ( m_nMoveTimer );
    ReleaseCapture ();
    m_nPressedButton = 0;
    UpdateHoverButton ();
  }
	
	CScrollView::OnLButtonUp(nFlags, point);
}

void CAnimationEditorView::OnMouseMove(UINT nFlags, CPoint point) 
{
  if ( m_bShowMoveButtons )
    UpdateHoverButton ();
	
	CScrollView::OnMouseMove(nFlags, point);
}

void CAnimationEditorView::OnUpdateFrameCrop(CCmdUI* pCmdUI) 
{ pCmdUI->Enable ( ( GetDocument ()->GetAnimation ()->GetFrameCount () > 0 ) && m_eState == State_Edit ); }

void CAnimationEditorView::OnFrameCrop() 
{
  CCropFrameDlg dlg;
  CAAnimation * pAnimation = GetDocument ()->GetAnimation ();
  if ( pAnimation->GetFrameCount () == 0 ) return;
  
  dlg.m_dwTolerance = m_dwLastCropTolerance;

  if ( dlg.DoModal () == IDOK )
  {
    m_dwLastCropTolerance = dlg.m_dwTolerance;
    pAnimation->CropFrame ( m_dwCurrentFrame, (BYTE)m_dwLastCropTolerance );
    SetCurrentFrame ( m_dwCurrentFrame );
    GetDocument ()->SetModifiedFlag ();
  }
}
