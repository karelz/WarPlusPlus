// ScriptEditLayout.h: interface for the CScriptEditLayout class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SCRIPTEDITLAYOUT_H__7C4CD785_5A18_11D3_A037_B2F8384EE131__INCLUDED_)
#define AFX_SCRIPTEDITLAYOUT_H__7C4CD785_5A18_11D3_A037_B2F8384EE131__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "..\..\Window.h"
#include "..\SCROLLCONTROL\ScrollControlLayout.h"	// Added by ClassView

class CScriptEditLayout : public CFrameWindowLayout  
{
  DECLARE_DYNAMIC(CScriptEditLayout)

public:
	CFontObject * GetFont();
	virtual void Delete();
	BOOL Create(CArchiveFile CfgFile);
	virtual BOOL Create(CConfigFile *pCfgFile);
	CScriptEditLayout();
	virtual ~CScriptEditLayout();

  struct tagSTokenStyle{ // description of one token style
    DWORD m_dwColor; // the color of the text
    CGraphicCacheFont *m_pNormalFont;  // font chaches for normal text
    CGraphicCacheFont *m_pSelectionFont; // for selected text
  };
  typedef struct tagSTokenStyle STokenStyle;

#ifdef _DEBUG
  virtual void AssertValid() const;
  virtual void Dump(CDumpContext &dc) const;
#endif

protected:
  int m_nTabSize;
  int m_nLeftMargin;
  int m_nRightMargin;
  int m_nTopMargin;
  int m_nBottomMargin;
	CAnimation * m_pCaret;
	CScrollControlLayout m_VerticalScrollControlLayout;
  CScrollControlLayout m_HorizontalScrollControlLayout;
	CFontObject * m_pFont;
  DWORD m_dwErrorStyleIndex;
  DWORD m_dwBackgroundColor;
  DWORD m_dwSelectionColor;
  CTypedPtrArray<CPtrArray, STokenStyle *> m_aTokenStyles;

  friend class CScriptEdit;
};

#endif // !defined(AFX_SCRIPTEDITLAYOUT_H__7C4CD785_5A18_11D3_A037_B2F8384EE131__INCLUDED_)
