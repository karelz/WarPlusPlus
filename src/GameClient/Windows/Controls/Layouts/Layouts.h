// Layouts.h: interface for the CLayouts class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_LAYOUTS_H__F7ED3245_6043_11D3_A04A_F19C5FEDE431__INCLUDED_)
#define AFX_LAYOUTS_H__F7ED3245_6043_11D3_A04A_F19C5FEDE431__INCLUDED_

#include "..\BUTTONS\TextButtonLayout.h"	// Added by ClassView
#include "..\EDITBOX\EditBoxLayout.h"	// Added by ClassView
#include "..\LISTCONTROL\ListControlLayout.h"	// Added by ClassView
#include "..\SCROLLCONTROL\ScrollControlLayout.h"	// Added by ClassView
#include "..\STATIC\StaticTextLayout.h"	// Added by ClassView
#include "..\..\WINDOW\CaptionWindowLayout.h"	// Added by ClassView
#include "..\..\WINDOW\DialogWindowLayout.h"
#include "..\ToolTip\ToolTipLayout.h"

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CLayouts : public CObject  
{
  DECLARE_DYNAMIC(CLayouts)

public:
	CDialogWindowLayout * GetDialogWindowLayout();
	static void Close();
	static BOOL Init(CArchiveFile CfgFile);
	virtual CTextButtonLayout * GetTextButtonLayout();
	virtual CStaticTextLayout * GetStaticTextLayout();
  virtual CScrollControlLayout * GetVerticalScrollControlLayout();
  virtual CScrollControlLayout * GetHorizontalScrollControlLayout();
	virtual CListControlLayout * GetListControlLayout();
	virtual CEditBoxLayout * GetEditBoxLayout();
	virtual CCaptionWindowLayout * GetCaptionWindowLayout();
  virtual CFrameWindowLayout * GetPopupMenuLayout();
  virtual CToolTipLayout * GetToolTipLayout();
	virtual void Delete();
	BOOL Create(CConfigFile *pCfgFile);
	BOOL Create(CArchiveFile CfgFile);
	CLayouts();
	virtual ~CLayouts();

  static CLayouts *m_pDefaults;

#ifdef _DEBUG
  virtual void AssertValid() const;
  virtual void Dump(CDumpContext &dc) const;
#endif
protected:
	BOOL m_bLoaded;
	CCaptionWindowLayout m_CaptionWindowLayout;
  CDialogWindowLayout m_DialogWindowLayout;
	CStaticTextLayout m_StaticTextLayout;
  CScrollControlLayout m_HorizontalScrollControlLayout;
  CScrollControlLayout m_VerticalScrollControlLayout;
	CListControlLayout m_ListControlLayout;
	CEditBoxLayout m_EditBoxLayout;
	CTextButtonLayout m_TextButtonLayout;
  CFrameWindowLayout m_PopupMenuLayout;
  CToolTipLayout m_ToolTipLayout;
};

#endif // !defined(AFX_LAYOUTS_H__F7ED3245_6043_11D3_A04A_F19C5FEDE431__INCLUDED_)
