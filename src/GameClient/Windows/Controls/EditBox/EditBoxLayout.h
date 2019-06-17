// EditBoxLayout.h: interface for the CEditBoxLayout class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_EDITBOXLAYOUT_H__F729F996_4B42_11D3_A024_FDE940773438__INCLUDED_)
#define AFX_EDITBOXLAYOUT_H__F729F996_4B42_11D3_A024_FDE940773438__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "..\..\Window.h"
#include "..\..\Fonts.h"

class CEditBoxLayout : public CFrameWindowLayout  
{
  DECLARE_DYNAMIC(CEditBoxLayout);
public:
#ifdef _DEBUG
  virtual void AssertValid() const;
  virtual void Dump(CDumpContext &dc) const;
#endif

public:
	// returns the font for the text in the edit box
  CFontObject * GetFont();
  // deletes the object
	virtual void Delete();
  // creates the object from given config file
  virtual BOOL Create(CConfigFile *pCfgFile);
  // creates the object from given config file
	virtual BOOL Create(CArchiveFile CfgFile);

	CEditBoxLayout();
	virtual ~CEditBoxLayout();

protected:
	CAnimation * m_pCaret;
	CAnimation * m_pSelection;
	// color of the selected text
  DWORD m_dwSelectedTextColor;
  // color of the text inactive
	DWORD m_dwInactiveTextColor;
  // color of the text (nonselected) active
  DWORD m_dwActiveTextColor;
  // font for the text in the edit box
	CFontObject * m_pFont;
	// Space under the text to the edge of the control
  int m_nBottomMargin;
  // Space above the text to the edge of the control
	int m_nTopMargin;
  // Space right of the text to the edge of the control
	int m_nRightMargin;
  // Space left of the text to the edge of the control
	int m_nLeftMargin;

  friend class CEditBox;
};

#endif // !defined(AFX_EDITBOXLAYOUT_H__F729F996_4B42_11D3_A024_FDE940773438__INCLUDED_)

