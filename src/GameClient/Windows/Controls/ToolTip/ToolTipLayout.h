// ToolTipLayout.h: interface for the CToolTipLayout class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_TOOLTIPLAYOUT_H__71BFAD56_E2CE_11D3_A8C4_00105ACA8325__INCLUDED_)
#define AFX_TOOLTIPLAYOUT_H__71BFAD56_E2CE_11D3_A8C4_00105ACA8325__INCLUDED_

#include "..\..\Fonts\FontObject.h"

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CToolTipLayout : public CObject  
{
  DECLARE_DYNAMIC(CToolTipLayout);

public:
	CFontObject * GetFont();
	virtual void Delete();
	BOOL Create(CConfigFile *pCfgFile);
	BOOL Create(CArchiveFile file);
	CToolTipLayout();
	virtual ~CToolTipLayout();

#ifdef _DEBUG
  virtual void AssertValid() const;
  virtual void Dump(CDumpContext &dc) const;
#endif

protected:
  // font for the tooltip
  CFontObject *m_pFont;

  // color of the underlaying window
  DWORD m_dwBackgroundColor;
  // opaque (0 - 255) of the background
  LONG m_nOpaque;

  // color of the text
  DWORD m_dwTextColor;

  // offset of the text in pixels from the top left corner of the window
  CPoint m_ptTextOffset;
  // margin to the right and bottom of the tex
  CSize m_sizeRBMargin;

  friend class CToolTip;
};

#endif // !defined(AFX_TOOLTIPLAYOUT_H__71BFAD56_E2CE_11D3_A8C4_00105ACA8325__INCLUDED_)
