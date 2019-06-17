// HelpBrowser.cpp: implementation of the CHelpBrowser class.
//
//////////////////////////////////////////////////////////////////////

#include "..\..\stdafx.h"
#include "..\..\GameClient.h"
#include "..\..\MainFrm.h"
#include "HelpBrowser.h"

#include "HelpBrowserWindow.h"
#include "GameClient\Common\CommonExceptions.h"

#define BROWSER_WIDTH 800
#define BROWSER_HEIGHT 1500
#define BROWSER_SCROLLBARSIZE 20

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

IMPLEMENT_DYNAMIC(CHelpBrowser, CWindow)

BEGIN_OBSERVER_MAP(CHelpBrowser, CWindow)
  BEGIN_NOTIFIER(ID_Self)
    EVENT(E_ViewChange)
      /*OnViewChange();*/ return FALSE;
    EVENT(CHelpBrowserWindow::E_DocumentComplete)
      OnDocumentComplete(); return FALSE;
    EVENT(CHelpBrowserWindow::E_NavigateBefore)
      OnNavigateBefore(); return FALSE;
    EVENT(CHelpBrowserWindow::E_TitleChange)
      OnTitleChange(); return FALSE;
  END_NOTIFIER()

  BEGIN_TIMER()
    ON_TIMETICK()
  END_TIMER()

  BEGIN_MOUSE()
    ON_LBUTTONDOWN()
    ON_LBUTTONUP()
    ON_MOUSEMOVE()
  END_MOUSE()

  BEGIN_NOTIFIER(IDC_BACK)
    ON_BUTTONCOMMAND(OnBack)
  END_NOTIFIER()
  BEGIN_NOTIFIER(IDC_FORWARD)
    ON_BUTTONCOMMAND(OnForward)
  END_NOTIFIER()
  BEGIN_NOTIFIER(IDC_HOME)
    ON_BUTTONCOMMAND(OnHome)
  END_NOTIFIER()
  BEGIN_NOTIFIER(IDC_REFRESH)
    ON_BUTTONCOMMAND(OnRefresh)
  END_NOTIFIER()

  BEGIN_NOTIFIER(VerticalScrollID)
    EVENT(CHelpBrowserScroll::E_ACTIVATED)
      OnScrollActivated();
      break;
    ON_STEPUP(OnVerticalStepUp)
    ON_STEPDOWN(OnVerticalStepDown)
    ON_PAGEUP(OnVerticalPageUp)
    ON_PAGEDOWN(OnVerticalPageDown)
    ON_POSITION(OnVerticalPosition)
  END_NOTIFIER()

  BEGIN_NOTIFIER(HorizontalScrollID)
    EVENT(CHelpBrowserScroll::E_ACTIVATED)
      OnScrollActivated();
      break;
    ON_STEPUP(OnHorizontalStepUp)
    ON_STEPDOWN(OnHorizontalStepDown)
    ON_PAGEUP(OnHorizontalPageUp)
    ON_PAGEDOWN(OnHorizontalPageDown)
    ON_POSITION(OnHorizontalPosition)
  END_NOTIFIER()

END_OBSERVER_MAP(CHelpBrowser, CWindow)

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CHelpBrowser::CHelpBrowser()
{
  m_bUpdateView = FALSE;
  m_pBrowserWindow = NULL;
  m_pViewObjectBrowser = NULL;
  m_bInvalidIEVersion = FALSE;
  m_bRedrawOnNextTimer = false;
}

CHelpBrowser::~CHelpBrowser()
{
}

#ifdef _DEBUG

void CHelpBrowser::AssertValid() const
{
  CWindow::AssertValid();
}

void CHelpBrowser::Dump(CDumpContext &dc) const
{
  CWindow::Dump(dc);
}

#endif

extern CHelpBrowserWindow *g_pHelpBrowserWindow;

void CHelpBrowser::Create(CRect rcWindow, CWindow *pParent, CDataArchive *pArchive)
{
  try{
  // Create the browser window
  CRect rcHelp(0, 0, BROWSER_WIDTH + BROWSER_SCROLLBARSIZE, BROWSER_HEIGHT);
  m_pBrowserWindow = g_pHelpBrowserWindow;
  if(m_pBrowserWindow->m_pWebBrowser == NULL){
    m_bInvalidIEVersion = TRUE;
  }
  else{
    if(m_pBrowserWindow->m_pWebBrowser->GetControlUnknown()->QueryInterface(IID_IViewObject, (void **)&m_pViewObjectBrowser) != S_OK){
      m_bInvalidIEVersion = TRUE;
    }
  }

  rcWindow.right -= CLayouts::m_pDefaults->GetVerticalScrollControlLayout()->GetWidth();
  rcWindow.bottom -= CLayouts::m_pDefaults->GetHorizontalScrollControlLayout()->GetHeight();

  m_ptTopLeft.x = 0; m_ptTopLeft.y = 0;

  m_Buffer.SetWidth(rcWindow.Width());
  m_Buffer.SetHeight(rcWindow.Height());
  m_Buffer.Create();

  if(!m_bInvalidIEVersion){
    m_AdviseSink.m_pHelpBrowser = this;
    m_pViewObjectBrowser->SetAdvise(DVASPECT_CONTENT, ADVF_PRIMEFIRST, &m_AdviseSink);

    m_pBrowserWindow->m_pHelpBrowser = this;
  }

  CWindow::Create(&rcWindow, pParent);

  ((CCaptionWindow *)GetParentWindow())->SetCaption("Nápovìda");

  CRect rcPage(0, 0, 1, 1);
  if(!m_bInvalidIEVersion){
    ((CCaptionWindow *)GetParentWindow())->SetCaption(m_pBrowserWindow->m_pWebBrowser->GetLocationName());

    rcPage = m_pBrowserWindow->GetPageSize();
  }

  CRect rcScroll;
  // create the vertical scroll bar
  rcScroll.left = rcWindow.right; rcScroll.right = rcWindow.right;
  rcScroll.top = rcWindow.top; rcScroll.bottom = rcWindow.bottom;
  if(!m_VerticalScroll.Create(rcScroll, CLayouts::m_pDefaults->GetVerticalScrollControlLayout(),
    pParent, FALSE)) return;
  m_VerticalScroll.Connect(this, VerticalScrollID);
  m_VerticalScroll.SetRange(rcPage.top, rcPage.bottom);
  // create the horizontal scroll bar
  rcScroll.left = rcWindow.left; rcScroll.right = rcWindow.right;
  rcScroll.top = rcWindow.bottom; rcScroll.bottom = rcWindow.bottom;
  if(!m_HorizontalScroll.Create(rcScroll, CLayouts::m_pDefaults->GetHorizontalScrollControlLayout(),
    pParent, TRUE)) return;
  m_HorizontalScroll.Connect(this, HorizontalScrollID);
  m_HorizontalScroll.SetRange(rcPage.left, rcPage.right);

  // Create our cursors
  m_cHyperTextLinkCursor.Create ( pArchive->CreateFile ( "..\\..\\Mouse\\HyperTextLink.cursor" ) );

  // Create overlay buttons
  m_BackLayout.Create(pArchive->CreateFile("Back.button"));
  m_ForwardLayout.Create(pArchive->CreateFile("Forward.button"));
  m_HomeLayout.Create(pArchive->CreateFile("Home.button"));
  m_RefreshLayout.Create(pArchive->CreateFile("Refresh.button"));
  m_Back.Create(CPoint(0, 0), &m_BackLayout, this);
  m_Back.SetToolTip(IDS_HELPBROWSER_BACK_TT);
  m_Back.Connect(this, IDC_BACK);
  m_Forward.Create(CPoint(0, 0), &m_ForwardLayout, this);
  m_Forward.SetToolTip(IDS_HELPBROWSER_FORWARD_TT);
  m_Forward.Connect(this, IDC_FORWARD);
  m_Home.Create(CPoint(0, 0), &m_HomeLayout, this);
  m_Home.SetToolTip(IDS_HELPBROWSER_HOME_TT);
  m_Home.Connect(this, IDC_HOME);
  m_Refresh.Create(CPoint(0, 0), &m_RefreshLayout, this);
  m_Refresh.SetToolTip(IDS_HELPBROWSER_REFRESH_TT);
  m_Refresh.Connect(this, IDC_REFRESH);

  PlaceButtons();

  UpdateScrolls();

  if(!m_bInvalidIEVersion){
    m_pBrowserWindow->GoHome();

    Connect(this, ID_Self);
    g_pTimer->Connect(this, 1000);
  }
  }
  catch ( ... )
  {
    m_bInvalidIEVersion = true;
    m_pBrowserWindow = NULL;
  }
}

void CHelpBrowser::Delete()
{
  try{
  m_VerticalScroll.Delete();
  m_HorizontalScroll.Delete();

  m_Back.Delete(); m_Forward.Delete(); m_Home.Delete(); m_Refresh.Delete();
  m_BackLayout.Delete(); m_ForwardLayout.Delete(); m_HomeLayout.Delete(); m_RefreshLayout.Delete();

  m_cHyperTextLinkCursor.Delete ();

  if(m_pBrowserWindow != NULL){
    m_pBrowserWindow->m_pHelpBrowser = NULL;
    m_pBrowserWindow = NULL;
  }
  if(m_pViewObjectBrowser != NULL){
    m_pViewObjectBrowser->SetAdvise(DVASPECT_CONTENT, 0, NULL);
    m_pViewObjectBrowser = NULL;
  }

  CWindow::Delete();
  m_Buffer.Delete();
  }
  catch ( ... )
  {
  }
}

void CHelpBrowser::OnSize(CSize size)
{
  try{
  m_Buffer.Delete();
  m_Buffer.SetWidth(size.cx);
  m_Buffer.SetHeight(size.cy);
  m_Buffer.Create();
  OnViewChange();

  PlaceButtons();
  }
  catch ( ... )
  {
  }
}

void CHelpBrowser::Draw(CDDrawSurface *pSurface, CRect *pRect)
{
  try {
  if(m_bInvalidIEVersion){
    pSurface->Fill(0);
    g_pSystemFont->PaintText(2, 2, "Minimální podporovaná verze:", pSurface, RGB32(220, 220, 220));
    g_pSystemFont->PaintText(2, 20, "   MicrosoftInternet Explorer 5.5 .", pSurface, RGB32(220, 220, 220));
  }
  else{
    if(m_bUpdateView){
      UpdateView();
    }
    pSurface->Paste(0, 0, &m_Buffer);
  }
  }
  catch (...)
  {
  }
}

CSize CHelpBrowser::GetMaximumSize()
{
  try{
  CSize sz(300, 400);
  if(!m_bInvalidIEVersion && m_pBrowserWindow != NULL){
    sz = m_pBrowserWindow->GetPageSize().Size();
  }
  sz.cx += CLayouts::m_pDefaults->GetVerticalScrollControlLayout()->GetWidth();
  sz.cy += CLayouts::m_pDefaults->GetHorizontalScrollControlLayout()->GetHeight();
  return sz;
  }
  catch (...)
  {
    return CSize ( 300, 400 );
  }
}

void CHelpBrowser::SetPosition(CRect *pRect)
{
  try{
  CRect rcBound(pRect);

  // small down the rect of the edit -> scroll bars
  rcBound.right -= CLayouts::m_pDefaults->GetVerticalScrollControlLayout()->GetWidth();
  rcBound.bottom -= CLayouts::m_pDefaults->GetHorizontalScrollControlLayout()->GetHeight();

  CWindow::SetWindowPosition(&rcBound);

  CRect rcScroll;
  // create the vertical scroll bar
  rcScroll.left = rcBound.right; rcScroll.right = rcBound.right + CLayouts::m_pDefaults->GetVerticalScrollControlLayout()->GetWidth();
  rcScroll.top = rcBound.top; rcScroll.bottom = rcBound.bottom;
  m_VerticalScroll.SetWindowPosition(&rcScroll);

  // create the horizontal scroll bar
  rcScroll.left = rcBound.left; rcScroll.right = rcBound.right;
  rcScroll.top = rcBound.bottom; rcScroll.bottom = rcBound.bottom + CLayouts::m_pDefaults->GetHorizontalScrollControlLayout()->GetHeight();
  m_HorizontalScroll.SetWindowPosition(&rcScroll);

  UpdateScrolls();
  }
  catch (... )
  {
    CWindow::SetWindowPosition ( pRect );
  }
}

void CHelpBrowser::UpdateView()
{
  try{
  if(m_bInvalidIEVersion) return;
  if(m_pBrowserWindow == NULL ) return;

  CRect rcBrowser = m_pBrowserWindow->GetWindowSize();

  CDC *pDC = m_Buffer.GetDC();
  try{
    rcBrowser.OffsetRect(-m_ptTopLeft.x / 2, -m_ptTopLeft.y / 2);
    if(OleDraw(m_pViewObjectBrowser, DVASPECT_CONTENT, *pDC, &rcBrowser) != S_OK){
      TRACE("Error");
    }
  }
  catch(...){
    m_bInvalidIEVersion = TRUE;
  }
  m_Buffer.ReleaseDC(pDC);

  m_bUpdateView = FALSE;
  }
  catch (... )
  {
  }
}

void CHelpBrowser::OnViewChange()
{
  try{
  m_bUpdateView = TRUE;
  UpdateRect();
  }
  catch ( ... )
  {
  }
}

extern CMainFrame * g_pMainFrame;

void CHelpBrowser::OnTimeTick(DWORD dwTime)
{
  try{
  OnTitleChange();

  if ( g_pMainFrame != NULL )
  {
	  g_pMainFrame->PostMessage ( WM_MAINFRAME_SETFOCUSTOOURWINDOW, 0, 0 );
  }

  if ( m_bRedrawOnNextTimer )
  {
    m_bRedrawOnNextTimer = false;
    UpdateView ();
  }
  }
  catch ( ... )
  {
  }
}

void CHelpBrowser::OnLButtonDown(CPoint pt)
{
}

void CHelpBrowser::OnLButtonUp(CPoint pt)
{
  try
  {
  if ( m_pBrowserWindow == NULL ) return;
  if ( m_pBrowserWindow->m_pWebBrowser == NULL ) return;
    CWnd *pWnd = m_pBrowserWindow->m_pWebBrowser->GetWindow(GW_CHILD)->ChildWindowFromPoint(pt, CWP_SKIPDISABLED | CWP_SKIPTRANSPARENT);

    pt.x += m_ptTopLeft.x;
    pt.y += m_ptTopLeft.y;

    pWnd->SendMessage(WM_LBUTTONDOWN, MK_LBUTTON,
      (pt.y << 16) | (pt.x & 0x0FFFF));
    pWnd->SendMessage(WM_LBUTTONUP, 0,
      (pt.y << 16) | (pt.x & 0x0FFFF));

    if ( g_pMainFrame != NULL )
    {
	    g_pMainFrame->PostMessage ( WM_MAINFRAME_SETFOCUSTOOURWINDOW, 0, 0 );
    }
  }
  catch (...)
  {
  }
}

void CHelpBrowser::OnMouseMove(CPoint pt)
{
  try{
    if ( m_pBrowserWindow == NULL ) return;
  pt.x += m_ptTopLeft.x;
  pt.y += m_ptTopLeft.y;

  if(m_pBrowserWindow->IsAnchor(pt)){
    SetWindowCursor ( &m_cHyperTextLinkCursor );
  }
  else{
    SetWindowCursor ( NULL );
  }
  }
  catch ( ... )
  {
  }
}

void CHelpBrowser::OnNavigateBefore()
{
  try{
  SetWindowCursor(g_pMouse->GetWaitingCursor());
  if ( g_pMainFrame != NULL )
  {
	  g_pMainFrame->PostMessage ( WM_MAINFRAME_SETFOCUSTOOURWINDOW, 0, 0 );
  }
  }
  catch ( ... )
  {
  }
}

void CHelpBrowser::OnDocumentComplete()
{
  try{
    if ( m_pBrowserWindow == NULL ) return;
  CRect rcPage = m_pBrowserWindow->GetPageSize();
  m_ptTopLeft.x = rcPage.left; m_ptTopLeft.y = rcPage.top;
  UpdateScrolls();
  OnViewChange();
  if(!m_bInvalidIEVersion){
    ((CCaptionWindow *)GetParentWindow())->SetCaption(m_pBrowserWindow->m_pWebBrowser->GetLocationName());
  }
  SetWindowCursor(NULL);

  if ( g_pMainFrame != NULL )
  {
	  g_pMainFrame->PostMessage ( WM_MAINFRAME_SETFOCUSTOOURWINDOW, 0, 0 );
  }
  m_bRedrawOnNextTimer = true;
  }
  catch ( ... )
  {
  }
}

void CHelpBrowser::UpdateScrolls()
{
  try{
  CRect rcWindow = GetWindowPosition();

  m_VerticalScroll.SetPageSize(rcWindow.Height());
  m_HorizontalScroll.SetPageSize(rcWindow.Width());

  m_VerticalScroll.SetPosition(m_ptTopLeft.y);
  m_HorizontalScroll.SetPosition(m_ptTopLeft.x);
  }
  catch ( ... )
  {
  }
}

void CHelpBrowser::OnHorizontalPosition(int nNewPosition)
{
  try {
  m_HorizontalScroll.SetPosition(nNewPosition);
  m_ptTopLeft.x = m_HorizontalScroll.GetPosition();
  OnViewChange();
  }
  catch (... )
  {
  }
}

void CHelpBrowser::OnHorizontalPageDown()
{
  try{
  m_HorizontalScroll.SetPosition(m_ptTopLeft.x + m_HorizontalScroll.GetPageSize());
  m_ptTopLeft.x = m_HorizontalScroll.GetPosition();
  OnViewChange();
  }
  catch (... )
  {
  }
}

void CHelpBrowser::OnHorizontalPageUp()
{
  try{
  m_HorizontalScroll.SetPosition(m_ptTopLeft.x - m_HorizontalScroll.GetPageSize());
  m_ptTopLeft.x = m_HorizontalScroll.GetPosition();
  OnViewChange();
  }
  catch (... )
  {
  }
}

void CHelpBrowser::OnHorizontalStepDown()
{
  try{
  m_HorizontalScroll.SetPosition(m_ptTopLeft.x + 10);
  m_ptTopLeft.x = m_HorizontalScroll.GetPosition();
  OnViewChange();
  }
  catch (... )
  {
  }
}

void CHelpBrowser::OnHorizontalStepUp()
{
  try{
  m_HorizontalScroll.SetPosition(m_ptTopLeft.x - 10);
  m_ptTopLeft.x = m_HorizontalScroll.GetPosition();
  OnViewChange();
  }
  catch (... )
  {
  }
}


void CHelpBrowser::OnVerticalPosition(int nNewPosition)
{
  try{
  m_VerticalScroll.SetPosition(nNewPosition);
  m_ptTopLeft.y = m_VerticalScroll.GetPosition();
  OnViewChange();
  }
  catch (... )
  {
  }
}

void CHelpBrowser::OnVerticalPageDown()
{
  try{
  m_VerticalScroll.SetPosition(m_ptTopLeft.y + m_VerticalScroll.GetPageSize());
  m_ptTopLeft.y = m_VerticalScroll.GetPosition();
  OnViewChange();
  }
  catch (... )
  {
  }
}

void CHelpBrowser::OnVerticalPageUp()
{
  try{
  m_VerticalScroll.SetPosition(m_ptTopLeft.y - m_VerticalScroll.GetPageSize());
  m_ptTopLeft.y = m_VerticalScroll.GetPosition();
  OnViewChange();
  }
  catch (... )
  {
  }
}

void CHelpBrowser::OnVerticalStepDown()
{
  try {
  m_VerticalScroll.SetPosition(m_ptTopLeft.y + 10);
  m_ptTopLeft.y = m_VerticalScroll.GetPosition();
  OnViewChange();
  }
  catch (... )
  {
  }
}

void CHelpBrowser::OnVerticalStepUp()
{
  try{
  m_VerticalScroll.SetPosition(m_ptTopLeft.y - 10);
  m_ptTopLeft.y = m_VerticalScroll.GetPosition();
  OnViewChange();
  }
  catch (... )
  {
  }
}

void CHelpBrowser::OnScrollActivated()
{
  try{
  this->Activate();
  this->SetFocus();  
  }
  catch (... )
  {
  }
}

void CHelpBrowser::OnTitleChange()
{
  try{
  if(!m_bInvalidIEVersion && m_pBrowserWindow != NULL){
    CString str;
    str = ((CCaptionWindow *)GetParentWindow())->GetCaption();
    CString strNew = m_pBrowserWindow->GetTitle();
    if(str == strNew) return;
    ((CCaptionWindow *)GetParentWindow())->SetCaption(strNew);
  }
  else{
    ((CCaptionWindow *)GetParentWindow())->SetCaption("Nápovìda");
  }
  if ( g_pMainFrame != NULL )
  {
	  g_pMainFrame->PostMessage ( WM_MAINFRAME_SETFOCUSTOOURWINDOW, 0, 0 );
  }
  }
  catch (... )
  {
  }
}

void CHelpBrowser::PlaceButtons()
{
  try{
  CPoint pt;

  CRect rcWindow = GetWindowPosition();
  pt.x = rcWindow.Width() - 119;
  pt.y = 5;
  m_Back.SetWindowPosition(&pt);

  pt.x += 29;
  m_Forward.SetWindowPosition(&pt);

  pt.x += 29;
  m_Home.SetWindowPosition(&pt);

  pt.x += 29;
  m_Refresh.SetWindowPosition(&pt);
  }
  catch (... )
  {
  }
}

void CHelpBrowser::OnBack()
{
  try{
  if(!m_bInvalidIEVersion && m_pBrowserWindow != NULL){
    m_pBrowserWindow->GoBack();
  }
  }
  catch (... )
  {
  }
}

void CHelpBrowser::OnForward()
{
  try{
  if(!m_bInvalidIEVersion && m_pBrowserWindow != NULL){
    m_pBrowserWindow->GoForward();
  }
  }
  catch (... )
  {
  }
}

void CHelpBrowser::OnHome()
{
  try{
  if(!m_bInvalidIEVersion && m_pBrowserWindow != NULL){
    m_pBrowserWindow->GoHome();
  }
  }
  catch (... )
  {
  }
}

void CHelpBrowser::OnRefresh()
{
  try{
  if(!m_bInvalidIEVersion && m_pBrowserWindow != NULL){
    m_pBrowserWindow->Refresh();
  }
  m_bRedrawOnNextTimer = true;
  }
  catch (... )
  {
  }
}