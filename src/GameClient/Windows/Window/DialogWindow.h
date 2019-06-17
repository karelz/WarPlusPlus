// DialogWindow.h: interface for the CDialogWindow class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_DIALOGWINDOW_H__D12FB6B3_A42A_11D3_A890_00105ACA8325__INCLUDED_)
#define AFX_DIALOGWINDOW_H__D12FB6B3_A42A_11D3_A890_00105ACA8325__INCLUDED_

#include "..\CONTROLS\BUTTONS\ImageButton.h"	// Added by ClassView
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "CaptionWindow.h"
#include "DialogWindowLayout.h"

class CDialogWindow : public CCaptionWindow  
{
  DECLARE_DYNAMIC(CDialogWindow);
  DECLARE_OBSERVER_MAP(CDialogWindow);

public:
  // creates the dialog with default size and layout and caption
  // sets it into the modal state
  // waits till it ends
  // deletes the dialog and returns the result code
	DWORD DoModal(CWindow *pParent);
  // the dialog must be created previously
  // sets the dialog into the modal state anmd waits till its end
	virtual DWORD DoModal();

  // creates the dialog
  // sets the dialog into the modals state
  // waits until it ends
  // deletes the dialog
  // returns the end reason code
	virtual DWORD DoModal(CRect &rcBound, CString strCaption, CDialogWindowLayout *pLayout, CWindow *pParent);

  // ends the dialog with given return code
	void EndDialog(DWORD dwCode);

  // deletes the object
	virtual void Delete();

  // creates the dialog
	virtual BOOL Create(CRect &rcBound, CString strCaption, CDialogWindowLayout *pLayout, CWindow *pParent);

  // constructor & destructor
	CDialogWindow();
	virtual ~CDialogWindow();

#ifdef _DEBUG
  virtual void AssertValid() const;
  virtual void Dump(CDumpContext &dc) const;
#endif

  enum {
    IDC_OK = 0x080000000,    // the OK button control ID
    IDC_CANCEL = 0x080000001 // the cancel button control ID (also for the close button)
  };

protected:
	CRect GetCenterPosition(CSize size);
  // return the default caption
	virtual CString GetDefaultCaption();
  // return the default position of the dialog
	virtual CRect GetDefaultPosition();

  // reaction on the cancel button
	void OnCancel();

  // does the message loop
	virtual DWORD DoLoop();

  // called right after the window is created
  // in this function you should create all dialog controls
	virtual void InitDialog();

  // the layout object for the dialog
  CDialogWindowLayout *m_pLayout;

private:
  // the event manager for which we're looping the messages
  // if this is not NULL than we're in modal state
	CEventManager *m_pEventManager;
  // the owner of this dialog - this window will be disabled if in modal state
	CWindow * m_pOwner;
  // the close button
	CImageButton m_CloseButton;

};

#endif // !defined(AFX_DIALOGWINDOW_H__D12FB6B3_A42A_11D3_A890_00105ACA8325__INCLUDED_)
