// DesktopWindow.h: interface for the CDesktopWindow class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_DESKTOPWINDOW_H__40EEB9D4_E61A_11D3_8444_004F4E0004AA__INCLUDED_)
#define AFX_DESKTOPWINDOW_H__40EEB9D4_E61A_11D3_8444_004F4E0004AA__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CDesktopWindow : public CWindow  
{
  DECLARE_DYNAMIC(CDesktopWindow);

public:
	virtual void Delete();
	void Create();
	CDesktopWindow();
	virtual ~CDesktopWindow();

  void SetImage(CArchiveFile file);
  void ClearImage();

#ifdef _DEBUG
  virtual void AssertValid() const;
  virtual void Dump(CDumpContext &dc) const;
#endif

  virtual void Draw(CDDrawSurface *pSurface, CRect *pRect);

  CImageSurface m_Image;
  BOOL m_bHasImage;
};

#endif // !defined(AFX_DESKTOPWINDOW_H__40EEB9D4_E61A_11D3_8444_004F4E0004AA__INCLUDED_)
