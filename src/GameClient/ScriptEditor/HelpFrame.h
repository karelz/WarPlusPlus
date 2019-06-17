// HelpFrame.h: interface for the CHelpFrame class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_HELPFRAME_H__AC9A7043_7BFD_11D4_B0F0_004F49068BD6__INCLUDED_)
#define AFX_HELPFRAME_H__AC9A7043_7BFD_11D4_B0F0_004F49068BD6__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "HelpBrowser\HelpBrowser.h"

class CHelpFrame : public CCaptionWindow  
{
  DECLARE_DYNAMIC(CHelpFrame);
  DECLARE_OBSERVER_MAP(CHelpWindow);

public:
	CHelpFrame();
	virtual ~CHelpFrame();

#ifdef _DEBUG
  virtual void AssertValid() const;
  virtual void Dump(CDumpContext &dc) const;
#endif

  void Create(CRect rcWindow, CWindow *pParent, CCaptionWindowLayout *pLayout, CDataArchive *pArchive);
  virtual void Delete();

protected:
  virtual void OnSize(CSize size);

  CSize GetMaximumSize();
  void OnClose();

private:
  void GetBrowserRect(CRect *pRect);

  CImageButton m_CloseButton;

  enum{
    IDC_CLOSE = 0x0100,
    IDC_HELPBROWSER = 0x0101,
  };

  // The help browser window
  CHelpBrowser m_wndHelpBrowser;
};

#endif // !defined(AFX_HELPFRAME_H__AC9A7043_7BFD_11D4_B0F0_004F49068BD6__INCLUDED_)
