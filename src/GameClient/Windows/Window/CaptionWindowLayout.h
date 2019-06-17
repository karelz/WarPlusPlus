// CaptionWindowLayout.h: interface for the CCaptionWindowLayout class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_CAPTIONWINDOWLAYOUT_H__7A991694_E478_11D2_AC2E_8C7702B45660__INCLUDED_)
#define AFX_CAPTIONWINDOWLAYOUT_H__7A991694_E478_11D2_AC2E_8C7702B45660__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "..\Fonts\FontObject.h"
#include "FrameWindowLayout.h"

class CCaptionWindow;

class CCaptionWindowLayout : public CFrameWindowLayout  
{
  DECLARE_DYNAMIC(CCaptionWindowLayout);

public:
	BOOL Create(CArchiveFile CfgFile);
	CFontObject * GetFont();
  // deletes the object
  virtual void Delete();
  // creates the object
  // should be called after create is complete in derived classes
	virtual BOOL Create(CConfigFile *pCfgFile);
  // constructor
	CCaptionWindowLayout();
  // destructor
	virtual ~CCaptionWindowLayout();

#ifdef _DEBUG
  virtual void AssertValid() const;
  virtual void Dump(CDumpContext &dc) const;
#endif
protected:
	// color of the caption for active window
  DWORD m_dwACaptionColor;
  // color of the caption for deactive window
  DWORD m_dwPCaptionColor;
  // right margin for the caption
  // (from the right edge of the window)
	int m_nRightMargin;
  // position of the text in the caption
  // (text will be centered in the rect between this and right margin)
	CPoint m_ptCaptionPos;
  // font for the caption
	CFontObject * m_pFont;

  friend CCaptionWindow;
};

#endif // !defined(AFX_CAPTIONWINDOWLAYOUT_H__7A991694_E478_11D2_AC2E_8C7702B45660__INCLUDED_)
