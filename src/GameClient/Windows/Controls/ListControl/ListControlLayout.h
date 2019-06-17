// ListControlLayout.h: interface for the CListControlLayout class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_LISTCONTROLLAYOUT_H__22FDD4E9_5081_11D3_A02E_DD87D6C1E031__INCLUDED_)
#define AFX_LISTCONTROLLAYOUT_H__22FDD4E9_5081_11D3_A02E_DD87D6C1E031__INCLUDED_

#include "..\SCROLLCONTROL\ScrollControlLayout.h"	// Added by ClassView
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "..\..\Window.h"

class CListControlLayout : public CFrameWindowLayout  
{
  DECLARE_DYNAMIC(CListControlLayout)

public:
  DWORD GetNormalTextColor(){ return m_dwNormalTextColor; }
  DWORD GetSelectedTextColor(){ return m_dwSelectedTextColor; }
  DWORD GetFocusTextColor(){ return m_dwFocusTextColor; }
  DWORD GetSelectedFocusTextColor(){ return m_dwSelectedFocusTextColor; }
	CFontObject * GetFont();
	virtual void Delete();
	virtual BOOL Create(CConfigFile *pCfgFile);
	BOOL Create(CArchiveFile CfgFile);
	CListControlLayout();
	virtual ~CListControlLayout();

#ifdef _DEBUG
  virtual void AssertValid() const;
  virtual void Dump(CDumpContext &dc) const;
#endif

  friend class CListControl;
  friend class CTreeControl;
protected:
  // anims for the selections
  CAnimation *m_pSelection;
  CAnimation *m_pFocus;
  CAnimation *m_pSelectionFocus;
  // color for the text items (normal, selected, selected with focus)
  DWORD m_dwNormalTextColor;
  DWORD m_dwSelectedTextColor;
  DWORD m_dwFocusTextColor;
  DWORD m_dwSelectedFocusTextColor;
  // the margins which is not to be used for items
  DWORD m_dwBottomMargin;
  DWORD m_dwTopMargin;
  DWORD m_dwRightMargin;
  DWORD m_dwLeftMargin;
  // the font for the text of the items
	CFontObject * m_pFont;
  // the layout object for the scroll bar
	CScrollControlLayout m_ScrollControlLayout;
  CScrollControlLayout m_HorizontalScrollControlLayout;
  BOOL m_bHasHorizontalScroll;
};

#endif // !defined(AFX_LISTCONTROLLAYOUT_H__22FDD4E9_5081_11D3_A02E_DD87D6C1E031__INCLUDED_)
