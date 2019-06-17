// MapViewStatusBar.h: interface for the CMapViewStatusBar class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MAPVIEWSTATUSBAR_H__2E5376A3_791C_11D4_B0ED_004F49068BD6__INCLUDED_)
#define AFX_MAPVIEWSTATUSBAR_H__2E5376A3_791C_11D4_B0ED_004F49068BD6__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CMapViewStatusBar : public CWindow  
{
public:
	CMapViewStatusBar();
	virtual ~CMapViewStatusBar();

#ifdef _DEBUG
  virtual void AssertValid() const;
  virtual void Dump(CDumpContext &dc) const;
#endif

  void Create(CDataArchive Archive, CWindow *pParent);
  virtual void Delete();

  void Show(CString strText, DWORD dwColor = RGB32(255, 255, 255));
  void Hide();

  virtual void Draw(CDDrawSurface *pDDSurface, CRect *pRect);

private:
  CImageSurface m_Left;
  CImageSurface m_Middle;
  CImageSurface m_Right;

  CString m_strText;
  DWORD m_dwColor;
};

#endif // !defined(AFX_MAPVIEWSTATUSBAR_H__2E5376A3_791C_11D4_B0ED_004F49068BD6__INCLUDED_)
