// MessageBox.h: interface for the CMessageBox class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MESSAGEBOX_H__F44D9343_AE54_11D3_99C6_DD3556D59171__INCLUDED_)
#define AFX_MESSAGEBOX_H__F44D9343_AE54_11D3_99C6_DD3556D59171__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "DialogWindow.h"
#include "..\Controls\Buttons.h"
#include "..\Controls\Static.h"

class CMessageBox : public CDialogWindow  
{
  DECLARE_DYNAMIC(CMessageBox);
  DECLARE_OBSERVER_MAP(CMessageBox);

public:

  typedef enum{
    MBType_OK,   // MessageBox with OK button
    MBType_YesNo, // MessageBox with Yes and No buttons
    MBType_YesNoCancel
  } EMessageBoxType;

  static DWORD YesNoCancel(UINT nText, UINT nCaption, CWindow *pParent){
    CString strText, strCaption;
    strText.LoadString(nText); strCaption.LoadString(nCaption);
    return YesNoCancel(strText, strCaption, pParent);
  }
  static DWORD YesNoCancel(CString strText, CString strCaption, CWindow *pParent);

  static DWORD YesNo(UINT nText, UINT nCaption, CWindow *pParent){
    CString strText, strCaption;
    strText.LoadString(nText); strCaption.LoadString(nCaption);
    return YesNo(strText, strCaption, pParent);
  }
  static DWORD YesNo(CString strText, CString strCaption, CWindow *pParent);
	static DWORD Do(CString strText, CString strCaption, EMessageBoxType eMBType, CWindow *pParent);
  static DWORD OK(UINT nText, UINT nCaption, CWindow *pParent){
    CString strText, strCaption;
    strText.LoadString(nText); strCaption.LoadString(nCaption);
    return OK(strText, strCaption, pParent);
  }
  static DWORD OK(CString strText, CString strCaption, CWindow *pParent);

  // creates the message box at the center of the screen
	BOOL Create(CString strText, CString strCaption, EMessageBoxType eMBType, CWindow *pParent);
  // creates the message box at given position
	BOOL Create(CRect rcBound, CString strText, CString strCaption, EMessageBoxType eMBType, CWindow *pParent);
  // deletes the object
	virtual void Delete();

  // constructor & destructor
	CMessageBox();
	virtual ~CMessageBox();

#ifdef _DEBUG
  virtual void AssertValid() const;
  virtual void Dump(CDumpContext &dc) const;
#endif

  enum{
    IDC_Yes = 0x080000003,
    IDC_No = 0x080000002,
  };

protected:
  void OnYes();
  void OnNo();
	void OnOK();
  // inits all controls
	virtual void InitDialog();

  // the text control
  CStaticText m_wndText;

  // the OK or YES button
  CTextButton m_wndOKButton;
  // the NO button if needed
  CTextButton m_wndNoButton;
  // the Cancel button if some
  CTextButton m_wndCancelButton;

  // the message text
  CString m_strText;

  // the of the message box
  EMessageBoxType m_eMBType;
};

#endif // !defined(AFX_MESSAGEBOX_H__F44D9343_AE54_11D3_99C6_DD3556D59171__INCLUDED_)
