// LogWindow.h: interface for the CLogWindow class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_LOGWINDOW_H__102F2AD4_10B0_11D4_849D_004F4E0004AA__INCLUDED_)
#define AFX_LOGWINDOW_H__102F2AD4_10B0_11D4_849D_004F4E0004AA__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

// the log window
class CLogWindow : public CCaptionWindow  
{
  DECLARE_DYNAMIC(CLogWindow);
  DECLARE_OBSERVER_MAP(CLogWindow);

public:
  // constructor & destructor
	CLogWindow();
	virtual ~CLogWindow();

  /// debug functions
#ifdef _DEBUG
  virtual void AssertValid() const;
  virtual void Dump(CDumpContext &dc) const;
#endif

  // creation
  void Create(CScriptEditLayout *pScriptLayout, CCaptionWindowLayout *pCaptionLayout, CWindow *pParent);
  virtual void Delete();

  void Show(){ ASSERT_VALID(this); ShowWindow(); }
  void Hide(){ ASSERT_VALID(this); HideWindow(); }

  void Clear();

  enum{ E_HIDDEN = 1 };

protected:
  enum{
    IDC_SCRIPTEDIT = 0x0200,
    IDC_CLOSE = 0x0201,
    IDC_CLEAR = 0x0202
  };
  // the script edit control
	CScriptViewEdit m_ScriptEdit;

  // the close button
  CImageButton m_CloseButton;

  // the clear button
  CTextButton m_ClearButton;

  // reaction on size changes
  void OnSize(CSize size);

  void OnClose();

  void OnPacketAvailable();
  void OnNetworkError();
  void OnAddBuffer(char *pBuffer);

private:
  void GetEditRect(CRect *pEditRect);

  // the virtual connection to the server's log output
  CVirtualConnection m_VirtualConnection;
  enum{
    ID_VirtualConnection = 0x0100,
    ID_Myself = 0x0101,
  };

  enum{
    E_AddBuffer = 0x010,
  };
};

#endif // !defined(AFX_LOGWINDOW_H__102F2AD4_10B0_11D4_849D_004F4E0004AA__INCLUDED_)

