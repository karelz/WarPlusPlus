// ProgressWindow.h: interface for the CProgressWindow class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_PROGRESSWINDOW_H__2DD213C4_E6BA_11D3_8445_004F4E0004AA__INCLUDED_)
#define AFX_PROGRESSWINDOW_H__2DD213C4_E6BA_11D3_8445_004F4E0004AA__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CProgressWindow : public CFrameWindow  
{
  DECLARE_DYNAMIC(CProgressWindow);

public:
  void Create(UINT nText, CWindow *pParent){
    CString str; str.LoadString(nText);
    Create(str, pParent);
  }
	void Create(CString strText, CWindow *pParent);
	CProgressWindow();
	virtual ~CProgressWindow();

  void SetText(UINT nText){
    CString str; str.LoadString(nText); SetText(str);
  }
  void SetText(CString strText){ m_Text.SetText(strText); }

#ifdef _DEBUG
  virtual void AssertValid() const;
  virtual void Dump(CDumpContext &dc) const;
#endif

private:
  CStaticText m_Text;
};

#endif // !defined(AFX_PROGRESSWINDOW_H__2DD213C4_E6BA_11D3_8445_004F4E0004AA__INCLUDED_)
