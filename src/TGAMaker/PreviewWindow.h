#if !defined(AFX_PREVIEWWINDOW_H__3F8D6D94_226C_11D3_9FC8_BE71DF0D3438__INCLUDED_)
#define AFX_PREVIEWWINDOW_H__3F8D6D94_226C_11D3_9FC8_BE71DF0D3438__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// PreviewWindow.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CPreviewWindow frame

#include "DirectX\DirectDraw.h"

class CPreviewWindow : public CFrameWnd
{
	DECLARE_DYNCREATE(CPreviewWindow)
public:
  CPreviewWindow();           // protected constructor used by dynamic creation
	virtual ~CPreviewWindow();

// Attributes
public:

// Operations
public:
	void SetBkgColor(DWORD dwColor);
	void SetPicture(CString strPicture);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CPreviewWindow)
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CPreviewWindow)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();
	afx_msg void OnPaint();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
private:
	DWORD m_dwBkgColor;
	CImageSurface m_Image;
	CString m_strPicture;
	CScratchSurface m_Result;
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PREVIEWWINDOW_H__3F8D6D94_226C_11D3_9FC8_BE71DF0D3438__INCLUDED_)
