// HelpBrowser.h: interface for the CHelpBrowser class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_HELPBROWSER_H__AC9A7048_7BFD_11D4_B0F0_004F49068BD6__INCLUDED_)
#define AFX_HELPBROWSER_H__AC9A7048_7BFD_11D4_B0F0_004F49068BD6__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "HelpBrowserScroll.h"

class CHelpBrowserWindow;

class CHelpBrowser : public CWindow  
{
  DECLARE_DYNAMIC(CHelpBrowser)
  DECLARE_OBSERVER_MAP(CHelpBrowser)

public:
	CHelpBrowser();
	virtual ~CHelpBrowser();

#ifdef _DEBUG
  virtual void AssertValid() const;
  virtual void Dump(CDumpContext &dc) const;
#endif

  void Create(CRect rcWindow, CWindow *pParent, CDataArchive *pArchive);
  virtual void Delete();

  virtual void Draw(CDDrawSurface *pSurface, CRect *pRect);

  void SetPosition(CRect *pRect);

  CSize GetMaximumSize();

protected:
  virtual void OnSize(CSize size);

  void OnDocumentComplete();
  void OnNavigateBefore();
  void OnTitleChange();

  void OnTimeTick(DWORD dwTime);
  void OnLButtonDown(CPoint pt);
  void OnLButtonUp(CPoint pt);
  void OnMouseMove(CPoint pt);

  class CHelpBrowserAdviseSink : public IAdviseSink
  {
  public:
    CHelpBrowserAdviseSink(){ m_nRefCount = 0; }
    ~CHelpBrowserAdviseSink(){}

    ULONG STDMETHODCALLTYPE AddRef(){ return ++m_nRefCount; }
    ULONG STDMETHODCALLTYPE Release(){ return --m_nRefCount; }

    HRESULT STDMETHODCALLTYPE QueryInterface(REFIID iid, void **ppvObject){
      if(iid == IID_IUnknown){ *ppvObject = (IUnknown *)this; return S_OK; }
      if(iid == IID_IAdviseSink){ *ppvObject = (IAdviseSink *)this; return S_OK; }
      return E_NOINTERFACE;
    }

    void STDMETHODCALLTYPE OnDataChange(FORMATETC *pFormatetc, STGMEDIUM *pStgmed){}
    void STDMETHODCALLTYPE OnViewChange(DWORD dwAspect, LONG lIndex){ m_pHelpBrowser->InlayEvent(E_ViewChange, 0, m_pHelpBrowser); }
    void STDMETHODCALLTYPE OnRename(IMoniker *pmk){}
    void STDMETHODCALLTYPE OnSave(){}
    void STDMETHODCALLTYPE OnClose(){}

    ULONG m_nRefCount;

    CHelpBrowser *m_pHelpBrowser;
  };
  friend class CHelpBrowserAdviseSink;

  // reactions on events from scroll bars
	void OnHorizontalPosition(int nNewPosition);
	void OnHorizontalPageDown();
	void OnHorizontalPageUp();
	void OnHorizontalStepDown();
	void OnHorizontalStepUp();
	void OnVerticalPosition(int nNewPosition);
	void OnVerticalPageDown();
	void OnVerticalPageUp();
	void OnVerticalStepDown();
	void OnVerticalStepUp();

  void OnScrollActivated();

  void UpdateScrolls();

  void OnBack();
  void OnForward();
  void OnHome();
  void OnRefresh();

private:
  // The window which has the browser
  CHelpBrowserWindow *m_pBrowserWindow;
  IViewObject *m_pViewObjectBrowser;

  CScratchSurface m_Buffer;

  void OnViewChange();
  void UpdateView();

  void PlaceButtons();

  BOOL m_bUpdateView;

  enum{ ID_Self = 0x05000, };
  enum{ HorizontalScrollID = 1, VerticalScrollID = 2 };

  enum{ E_ViewChange = 1, };

  CHelpBrowserAdviseSink m_AdviseSink;

  // scroll bars
	CHelpBrowserScroll m_VerticalScroll;
	CHelpBrowserScroll m_HorizontalScroll;
  CPoint m_ptTopLeft;

  enum{
    IDC_BACK = 0x0100,
    IDC_FORWARD = 0x0101,
    IDC_HOME = 0x0102,
    IDC_REFRESH = 0x0103,
  };

  // If true -> redraw the window on the next timer event
  bool m_bRedrawOnNextTimer;

  // If TRUE, the IE version is 5.0 or lower -> some bug instead
  // -> no use of help
  BOOL m_bInvalidIEVersion;

  CImageButtonLayout m_BackLayout, m_ForwardLayout, m_HomeLayout, m_RefreshLayout;
  CImageButton m_Back, m_Forward, m_Home, m_Refresh;

  // Cursor to show when the mouse is above some hypertext link
  CCursor m_cHyperTextLinkCursor;
};

#endif // !defined(AFX_HELPBROWSER_H__AC9A7048_7BFD_11D4_B0F0_004F49068BD6__INCLUDED_)
