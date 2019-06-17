// PreviewWindow.cpp : implementation file
//

#include "stdafx.h"
#include "TGAMaker.h"
#include "PreviewWindow.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CPreviewWindow

IMPLEMENT_DYNCREATE(CPreviewWindow, CFrameWnd)

CPreviewWindow::CPreviewWindow()
{
  m_dwBkgColor = RGB32(0, 0, 0);
}

CPreviewWindow::~CPreviewWindow()
{
}


BEGIN_MESSAGE_MAP(CPreviewWindow, CFrameWnd)
	//{{AFX_MSG_MAP(CPreviewWindow)
	ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_WM_PAINT()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPreviewWindow message handlers

int CPreviewWindow::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CFrameWnd::OnCreate(lpCreateStruct) == -1)
		return -1;
	
  try{
    if(!CDirectDraw::Init(this, CDirectDraw::DM_NoChange, DDSCL_NORMAL)){
      return -1;
    }
    SetWindowText("Preview");
  }
  catch(CException *e){
    e->ReportError();
    e->Delete();
    return -1;
  }

	return 0;
}

void CPreviewWindow::OnDestroy() 
{
	CFrameWnd::OnDestroy();
	
  try{
    if(m_Image.IsValid())
      m_Image.Delete();
    if(m_Result.IsValid())
      m_Result.Delete();
    CDirectDraw::Close();
  }
  catch(CException *e){
    e->ReportError();
    e->Delete();
  }
}

void CPreviewWindow::SetPicture(CString strPicture)
{
  CString str;
  m_strPicture = strPicture;

  if(m_Image.IsValid())
    m_Image.Delete();
  if(m_Result.IsValid())
    m_Result.Delete();

  try{
    CArchiveObject ao;
    if(!ao.Create(strPicture.Left(strPicture.Find("\\", 0) + 1))){
      goto SomeError;
    }
    CArchiveFile file;
    if(!ao.CreateFile(strPicture.Right(strPicture.GetLength() - strPicture.Find("\\", 0) - 1), &file)){
      goto SomeError;
    }

    if(!m_Image.Create(&file)){
      goto SomeError;
    }

    m_Result.SetWidth(m_Image.GetWidth());
    m_Result.SetHeight(m_Image.GetHeight());
    if(!m_Result.Create()){
      goto SomeError;
    }

    m_Result.Fill(m_dwBkgColor);
    m_Result.Paste(0, 0, &m_Image);

    Invalidate();
  }
  catch(CException *e){
//    e->ReportError();
    e->Delete();
    goto SomeError;
  }
  str.Format("%d x %d - ", m_Image.GetWidth(), m_Image.GetHeight());
  str += strPicture;
  SetWindowText(str);
  return;

SomeError:
  SetWindowText("Preview - ERROR");
  if(m_Image.IsValid())
    m_Image.Delete();
  if(m_Result.IsValid())
    m_Result.Delete();
}

void CPreviewWindow::SetBkgColor(DWORD dwColor)
{
  m_dwBkgColor = dwColor;

  if(m_Result.IsValid()){
    m_Result.Fill(m_dwBkgColor);
    m_Result.Paste(0, 0, &m_Image);
  }

  Invalidate();
}

void CPreviewWindow::OnPaint() 
{
	CPaintDC dc(this); // device context for painting

  if(!m_Result.IsValid()) return;

  CRect rcClient;
  GetClientRect(&rcClient);
  CDC *pBufferDC;
  pBufferDC = m_Result.GetDC();
  dc.BitBlt(0, 0, rcClient.Width(), rcClient.Height(), pBufferDC, 0, 0, SRCCOPY);
  m_Result.ReleaseDC(pBufferDC);
  
	// Do not call CFrameWnd::OnPaint() for painting messages
}
