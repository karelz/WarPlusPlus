// HelpBrowserWindow.cpp : implementation file
//

#include "..\..\stdafx.h"
#include "..\..\gameclient.h"
#include "HelpBrowserWindow.h"

#include "HelpBrowser.h"

#include "Mshtml.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define IDC_EXPLORER 100

#define WM_HELPBROWSER_ACTION WM_USER + 0x0100

enum{
  HelpBrowserAction_Back,
  HelpBrowserAction_Forward,
  HelpBrowserAction_Home,
  HelpBrowserAction_Refresh,
};

/////////////////////////////////////////////////////////////////////////////
// CHelpBrowserWindow

CHelpBrowserWindow::CHelpBrowserWindow()
{
  m_pHelpBrowser = NULL;
}

CHelpBrowserWindow::~CHelpBrowserWindow()
{
  if(m_pWebBrowser != NULL){
    delete m_pWebBrowser;
    m_pWebBrowser = NULL;
  }
}


BEGIN_MESSAGE_MAP(CHelpBrowserWindow, CWnd)
	//{{AFX_MSG_MAP(CHelpBrowserWindow)
	ON_WM_CREATE()
	ON_WM_DESTROY()
  ON_MESSAGE(WM_HELPBROWSER_ACTION, OnHelpBrowserAction)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

BEGIN_EVENTSINK_MAP(CHelpBrowserWindow, CWnd)
    //{{AFX_EVENTSINK_MAP(CHelpBrowserWindow)
	ON_EVENT(CHelpBrowserWindow, IDC_EXPLORER, 259 /* DocumentComplete */, OnDocumentComplete, VTS_DISPATCH VTS_PVARIANT)
	ON_EVENT(CHelpBrowserWindow, IDC_EXPLORER, 113 /* TitleChange */, OnTitleChange, VTS_BSTR)
	ON_EVENT(CHelpBrowserWindow, IDC_EXPLORER, 250 /* BeforeNavigate2 */, OnBeforeNavigate, VTS_DISPATCH VTS_PVARIANT VTS_PVARIANT VTS_PVARIANT VTS_PVARIANT VTS_PVARIANT VTS_PBOOL)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()

/////////////////////////////////////////////////////////////////////////////
// CHelpBrowserWindow message handlers

int CHelpBrowserWindow::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CWnd::OnCreate(lpCreateStruct) == -1)
		return -1;
	
  // Create the browser control
  CRect rcClient;
  GetClientRect(&rcClient);
  CRect rc;

  try{
    m_pWebBrowser = new CWebBrowser2();
    m_pWebBrowser->Create("WebBrowser", WS_CHILD | WS_VISIBLE, rcClient, this, IDC_EXPLORER);
    m_pWebBrowser->GetClientRect(&rc);
    m_pWebBrowser->SetSilent(TRUE);
  }
  catch(...){
    m_pWebBrowser = NULL;
  }

  m_rcWindow = rc;
  m_rcPage.left = 2;
  m_rcPage.top = 2;
  m_rcPage.right = m_rcPage.left + rc.Width() - 20;
  m_rcPage.bottom = m_rcPage.top + rc.Height() - 20;

	return 0;
}

void CHelpBrowserWindow::OnDestroy() 
{
  if(m_pWebBrowser != NULL){
    m_pWebBrowser->DestroyWindow();
  }

  CWnd::OnDestroy();
}

void CHelpBrowserWindow::SetURL(CString strURL)
{
  COleVariant varEmpty;
  if(m_pWebBrowser == NULL) return;

  try{
    m_pWebBrowser->Navigate(strURL, &varEmpty, &varEmpty, &varEmpty, &varEmpty);
  }
  catch(...)
  {
    m_pWebBrowser = NULL;
  }
}

void CHelpBrowserWindow::OnDocumentComplete(LPDISPATCH pDisp, VARIANT *URL)
{
  if(m_pWebBrowser == NULL) return;
  if(m_pHelpBrowser == NULL) return;
  m_pHelpBrowser->InlayEvent(E_DocumentComplete, 0, m_pHelpBrowser);
}

void CHelpBrowserWindow::OnTitleChange(LPCSTR Text)
{
  if(m_pWebBrowser == NULL) return;
  if(m_pHelpBrowser == NULL) return;
  m_pHelpBrowser->InlayEvent(E_TitleChange, 0, m_pHelpBrowser);
}

void CHelpBrowserWindow::OnBeforeNavigate(LPDISPATCH pDisp, VARIANT *URL, VARIANT *Flags, VARIANT *TargetFrameName,
                                          VARIANT *PostData, VARIANT *Headers, BOOL *Cancel)
{
  if(m_pWebBrowser == NULL) return;
  if(m_pHelpBrowser == NULL) return;
  m_pHelpBrowser->InlayEvent(E_NavigateBefore, 0, m_pHelpBrowser);
}

LRESULT CHelpBrowserWindow::OnHelpBrowserAction(WPARAM wParam, LPARAM lParam)
{
  if(m_pWebBrowser == NULL) return 0;
  try{
    switch(wParam){
    case HelpBrowserAction_Home:
      SetURL(m_strHomeURL);
      break;
    case HelpBrowserAction_Back:
      m_pWebBrowser->GoBack();
      break;
    case HelpBrowserAction_Forward:
      m_pWebBrowser->GoForward();
      break;
    case HelpBrowserAction_Refresh:
      m_pWebBrowser->Refresh();
      break;
    default:
      break;
    }
  }
  catch(...){
  }

  return 0;
}

void CHelpBrowserWindow::GoHome()
{
  if(m_pWebBrowser == NULL) return;
  PostMessage(WM_HELPBROWSER_ACTION, HelpBrowserAction_Home, 0);
}

void CHelpBrowserWindow::GoBack()
{
  if(m_pWebBrowser == NULL) return;
  PostMessage(WM_HELPBROWSER_ACTION, HelpBrowserAction_Back, 0);
}

void CHelpBrowserWindow::GoForward()
{
  if(m_pWebBrowser == NULL) return;
  PostMessage(WM_HELPBROWSER_ACTION, HelpBrowserAction_Forward, 0);
}

void CHelpBrowserWindow::Refresh()
{
  if(m_pWebBrowser == NULL) return;
  PostMessage(WM_HELPBROWSER_ACTION, HelpBrowserAction_Refresh, 0);
}

CString CHelpBrowserWindow::GetTitle()
{
  if(m_pWebBrowser == NULL) return CString();

  IHTMLDocument2 *pDoc = NULL;
  LPDISPATCH pDispDoc = NULL;
  pDispDoc = m_pWebBrowser->GetDocument();
  
  CString strTitle;

  do{
    if(pDispDoc == NULL) break;

    if(pDispDoc->QueryInterface(IID_IHTMLDocument2, (void **)&pDoc) != S_OK) break;

    BSTR bstrTitle;
    if(pDoc->get_title(&bstrTitle) != S_OK) break;

    strTitle = bstrTitle;
    if(pDoc != NULL) pDoc->Release();
    if(pDispDoc != NULL) pDispDoc->Release();
    return strTitle;
  } while(FALSE);

  if(pDoc != NULL) pDoc->Release();
  if(pDispDoc != NULL) pDispDoc->Release();
  return m_pWebBrowser->GetLocationName();
}

BOOL CHelpBrowserWindow::IsAnchor(CPoint pt)
{
  if(m_pWebBrowser == NULL) return FALSE;

  try
  {
  IHTMLDocument2 *pDoc = NULL;
  LPDISPATCH pDispDoc = NULL;
  pDispDoc = m_pWebBrowser->GetDocument();
  IHTMLElement *pElement = NULL;

  do{
    if(pDispDoc == NULL) break;

    if(pDispDoc->QueryInterface(IID_IHTMLDocument2, (void **)&pDoc) != S_OK) break;

    if(pDoc->elementFromPoint(pt.x, pt.y, &pElement) != S_OK) break;
    if(pElement == NULL) break;

    IHTMLAnchorElement *pAnchor;
    if(pElement->QueryInterface(IID_IHTMLAnchorElement, (void **)&pAnchor) != S_OK) break;

    if(pDoc != NULL) pDoc->Release();
    if(pDispDoc != NULL) pDispDoc->Release();
    if(pElement != NULL) pElement->Release();
  
    if(pAnchor != NULL) pAnchor->Release();

    return TRUE;
  } while(FALSE);

  if(pDoc != NULL) pDoc->Release();
  if(pDispDoc != NULL) pDispDoc->Release();
  if(pElement != NULL) pElement->Release();
  return FALSE;

  }
  catch ( ... )
  {
    m_pWebBrowser = NULL;
    return FALSE;
  }
}