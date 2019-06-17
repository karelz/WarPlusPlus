// MessageBox.cpp: implementation of the CMessageBox class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "MessageBox.h"

#define MSGBOX_WIDTH 400
#define MSGBOX_HEIGHT 80
#define MSGBOX_TEXTPOS_X 25
#define MSGBOX_TEXTPOS_Y 35
#define MSGBOX_TEXTHEIGHT 20
#define MSGBOX_BUTTONWIDTH 80
#define MSGBOX_BUTTONHEIGHT 25

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

IMPLEMENT_DYNAMIC(CMessageBox, CDialogWindow);

BEGIN_OBSERVER_MAP(CMessageBox, CDialogWindow)
  BEGIN_NOTIFIER(CMessageBox::IDC_OK)
    ON_BUTTONCOMMAND(OnOK);
  END_NOTIFIER()
  BEGIN_NOTIFIER(CMessageBox::IDC_Yes)
    ON_BUTTONCOMMAND(OnYes);
  END_NOTIFIER()
  BEGIN_NOTIFIER(CMessageBox::IDC_No)
    ON_BUTTONCOMMAND(OnNo);
  END_NOTIFIER()
END_OBSERVER_MAP(CMessageBox, CDialogWindow)

CMessageBox::CMessageBox()
{
}

CMessageBox::~CMessageBox()
{
}

#ifdef _DEBUG

void CMessageBox::AssertValid() const
{
  CDialogWindow::AssertValid();
}

void CMessageBox::Dump(CDumpContext &dc) const
{
  CDialogWindow::Dump(dc);
}

#endif



BOOL CMessageBox::Create(CRect rcBound, CString strText, CString strCaption, EMessageBoxType eMBType, CWindow *pParent)
{
  // message box can't be created without parent window
  ASSERT_VALID(pParent);

  // copy the type
  m_eMBType = eMBType;
  // copy the text
  m_strText = strText;

  // call the dialog create function
  return CDialogWindow::Create(rcBound, strCaption, NULL, pParent);
}

BOOL CMessageBox::Create(CString strText, CString strCaption, EMessageBoxType eMBType, CWindow *pParent)
{
  CRect rcBound;

  // compute the center of the screen
  CSize MBoxSize, TextSize;
  TextSize = CStaticText::GetControlSize(NULL, strText);
  MBoxSize.cx = TextSize.cx + 2 * MSGBOX_TEXTPOS_X;
  MBoxSize.cy = TextSize.cy + MSGBOX_TEXTPOS_Y + MSGBOX_BUTTONHEIGHT + 15 + 10;

  if(MBoxSize.cx < MSGBOX_WIDTH) MBoxSize.cx = MSGBOX_WIDTH;
  if(MBoxSize.cy < MSGBOX_HEIGHT) MBoxSize.cy = MSGBOX_HEIGHT;
  if(MBoxSize.cx > g_pDDPrimarySurface->GetScreenRect()->Width()){
    MBoxSize.cx = g_pDDPrimarySurface->GetScreenRect()->Width();
  }

  rcBound.left = (g_pDDPrimarySurface->GetScreenRect()->Width() - MBoxSize.cx) / 2;
  rcBound.top = (g_pDDPrimarySurface->GetScreenRect()->Height() - MBoxSize.cy) / 2;
  rcBound.right = rcBound.left + MBoxSize.cx;
  rcBound.bottom = rcBound.top + MBoxSize.cy;

  // create it there
  return Create(rcBound, strText, strCaption, eMBType, pParent);
}

void CMessageBox::Delete()
{
  m_wndText.Delete();
  m_wndCancelButton.Delete();
  m_wndOKButton.Delete();
  m_wndNoButton.Delete();

  CDialogWindow::Delete();
}

void CMessageBox::InitDialog()
{
  CRect rcWndRect = GetWindowPosition();
  CRect rcText(MSGBOX_TEXTPOS_X, MSGBOX_TEXTPOS_Y,
    rcWndRect.Width() - MSGBOX_TEXTPOS_X, 0);

  m_wndText.Create(rcText, NULL, m_strText, this);
  m_wndText.SetAlign(CStaticText::AlignLeft);

  CRect rcButton;

  switch(m_eMBType){
  case MBType_OK:
    rcButton.left = rcWndRect.Width() - MSGBOX_TEXTPOS_X - MSGBOX_BUTTONWIDTH;
    rcButton.right = rcButton.left + MSGBOX_BUTTONWIDTH;
    rcButton.top = rcWndRect.Height() - MSGBOX_BUTTONHEIGHT - 15;
    rcButton.bottom = rcButton.top + MSGBOX_BUTTONHEIGHT;
    m_wndOKButton.Create(&rcButton, NULL, "OK", this);
    m_wndOKButton.Connect(this, IDC_OK);
    m_wndOKButton.SetKeyAcc(VK_RETURN, 0);
    break;
  case MBType_YesNoCancel:
    rcButton.left = rcWndRect.Width() - MSGBOX_TEXTPOS_X - 3 * MSGBOX_BUTTONWIDTH - 20;
    rcButton.right = rcButton.left + MSGBOX_BUTTONWIDTH;
    rcButton.top = rcWndRect.Height() - MSGBOX_BUTTONHEIGHT - 15;
    rcButton.bottom = rcButton.top + MSGBOX_BUTTONHEIGHT;
    m_wndOKButton.Create(&rcButton, NULL, "Ano", this);
    m_wndOKButton.Connect(this, IDC_Yes);
    m_wndOKButton.SetKeyAcc(VK_RETURN, 0);

    rcButton.left = rcWndRect.Width() - MSGBOX_TEXTPOS_X - 2 * MSGBOX_BUTTONWIDTH - 10;
    rcButton.right = rcButton.left + MSGBOX_BUTTONWIDTH;
    rcButton.top = rcWndRect.Height() - MSGBOX_BUTTONHEIGHT - 15;
    rcButton.bottom = rcButton.top + MSGBOX_BUTTONHEIGHT;
    m_wndNoButton.Create(&rcButton, NULL, "Ne", this);
    m_wndNoButton.Connect(this, IDC_No);

    rcButton.left = rcWndRect.Width() - MSGBOX_TEXTPOS_X - MSGBOX_BUTTONWIDTH;
    rcButton.right = rcButton.left + MSGBOX_BUTTONWIDTH;
    rcButton.top = rcWndRect.Height() - MSGBOX_BUTTONHEIGHT - 15;
    rcButton.bottom = rcButton.top + MSGBOX_BUTTONHEIGHT;
    m_wndCancelButton.Create(&rcButton, NULL, "Storno", this);
    m_wndCancelButton.Connect(this, IDC_CANCEL);
    m_wndCancelButton.SetKeyAcc(VK_ESCAPE, 0);
    break;
  case MBType_YesNo:
    rcButton.left = rcWndRect.Width() - MSGBOX_TEXTPOS_X - 2 * MSGBOX_BUTTONWIDTH - 10;
    rcButton.right = rcButton.left + MSGBOX_BUTTONWIDTH;
    rcButton.top = rcWndRect.Height() - MSGBOX_BUTTONHEIGHT - 15;
    rcButton.bottom = rcButton.top + MSGBOX_BUTTONHEIGHT;
    m_wndOKButton.Create(&rcButton, NULL, "Ano", this);
    m_wndOKButton.Connect(this, IDC_Yes);
    m_wndOKButton.SetKeyAcc(VK_RETURN, 0);

    rcButton.left = rcWndRect.Width() - MSGBOX_TEXTPOS_X - MSGBOX_BUTTONWIDTH;
    rcButton.right = rcButton.left + MSGBOX_BUTTONWIDTH;
    rcButton.top = rcWndRect.Height() - MSGBOX_BUTTONHEIGHT - 15;
    rcButton.bottom = rcButton.top + MSGBOX_BUTTONHEIGHT;
    m_wndNoButton.Create(&rcButton, NULL, "Ne", this);
    m_wndNoButton.Connect(this, IDC_No);
    m_wndNoButton.SetKeyAcc(VK_ESCAPE, 0);
    break;
  }
}

void CMessageBox::OnOK()
{
  switch(m_eMBType){
  case MBType_OK:
    EndDialog(IDC_OK);
    break;
  }
}

void CMessageBox::OnYes()
{
  switch(m_eMBType){
  case MBType_YesNo:
  case MBType_YesNoCancel:
    EndDialog(IDC_Yes);
    break;
  }
}

void CMessageBox::OnNo()
{
  switch(m_eMBType){
  case MBType_YesNo:
  case MBType_YesNoCancel:
    EndDialog(IDC_No);
    break;
  }
}

DWORD CMessageBox::Do(CString strText, CString strCaption, EMessageBoxType eMBType, CWindow *pParent)
{
  CMessageBox wndMsgBox;

  DWORD dwRet;
  wndMsgBox.Create(strText, strCaption, eMBType, pParent);
  dwRet = wndMsgBox.DoModal();
  wndMsgBox.Delete();

  return dwRet;
}

DWORD CMessageBox::OK(CString strText, CString strCaption, CWindow *pParent)
{
  return Do(strText, strCaption, MBType_OK, pParent);
}

DWORD CMessageBox::YesNo(CString strText, CString strCaption, CWindow *pParent)
{
  return Do(strText, strCaption, MBType_YesNo, pParent);
}

DWORD CMessageBox::YesNoCancel(CString strText, CString strCaption, CWindow *pParent)
{
  return Do(strText, strCaption, MBType_YesNoCancel, pParent);
}
