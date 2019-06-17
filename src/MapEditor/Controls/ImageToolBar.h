// ImageToolBar.h: interface for the CImageToolBar class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_IMAGETOOLBAR_H__AA867AC6_68EB_11D3_A05D_B2109F41DA31__INCLUDED_)
#define AFX_IMAGETOOLBAR_H__AA867AC6_68EB_11D3_A05D_B2109F41DA31__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CImageToolBar : public CToolBar  
{
  DECLARE_MESSAGE_MAP();

public:
	void InsertImageButton(int nIndex, UINT nID);
	CImageToolBar();
	virtual ~CImageToolBar();


protected:
  afx_msg int OnCreate( LPCREATESTRUCT lpCreateStruct );
};

#endif // !defined(AFX_IMAGETOOLBAR_H__AA867AC6_68EB_11D3_A05D_B2109F41DA31__INCLUDED_)
