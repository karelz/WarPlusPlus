// MapScrollHelper.h: interface for the CMapScrollHelper class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MAPSCROLLHELPER_H__BE234FC3_2F0B_11D4_84D7_004F4E0004AA__INCLUDED_)
#define AFX_MAPSCROLLHELPER_H__BE234FC3_2F0B_11D4_84D7_004F4E0004AA__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

// Helper window for map view (fullscreen) scrolling
class CMapScrollHelper : public CWindow  
{
  DECLARE_DYNAMIC(CMapScrollHelper);
  DECLARE_OBSERVER_MAP(CMapScrollHelper);

public:
	CMapScrollHelper();
	virtual ~CMapScrollHelper();

#ifdef _DEBUG
  virtual void AssertValid() const;
  virtual void Dump(CDumpContext &dc) const;
#endif

  void Create(CRect &rcWindow, CWindow *pParent, DWORD dwTimeDelay);
  virtual void Delete();
  
  enum{ E_SCROLLEVENT = 1, };

protected:
  void OnMouseMove(CPoint pt);
  void OnMouseLeave();
  void OnTimeTick(DWORD dwTime);

private:
  DWORD m_dwTimeDelay;
  BOOL m_bTimerConnected;
};

#endif // !defined(AFX_MAPSCROLLHELPER_H__BE234FC3_2F0B_11D4_84D7_004F4E0004AA__INCLUDED_)
