// ScrollControlLayout.h: interface for the CScrollControlLayout class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SCROLLCONTROLLAYOUT_H__22FDD4EB_5081_11D3_A02E_DD87D6C1E031__INCLUDED_)
#define AFX_SCROLLCONTROLLAYOUT_H__22FDD4EB_5081_11D3_A02E_DD87D6C1E031__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "..\..\Animation.h"

class CScrollControlLayout : public CObject  
{
  DECLARE_DYNAMIC(CScrollControlLayout);

public:
  // these return ssize of the scroll
  // we don't know the orientation -> so both returns just
  // the size of the top button
	int GetHeight();
	int GetWidth();

	virtual void Delete();
	virtual BOOL Create(CConfigFile *pCfgFile);
	BOOL Create(CArchiveFile CfgFile);
	CScrollControlLayout();
	virtual ~CScrollControlLayout();

#ifdef _DEBUG
  virtual void AssertValid() const;
  virtual void Dump(CDumpContext &dc) const;
#endif

protected:
  // minimal (middle) button size
	int m_nMinButtonSize;
  // top and bottom button sizes (margine where can't be the middle button)
	int m_nBottomButtonSize;
	int m_nTopButtonSize;

  // middle part of the scroll bar
  CAnimation *m_pPressedMiddle;
  CAnimation *m_pActiveMiddle;
  CAnimation *m_pPassiveMiddle;
  
  // passive top and bottom part (buttons)
  CAnimation *m_pPassiveTop;
  CAnimation *m_pPassiveBottom;
  // active top and bottom part (buttons) (with mouse over them)
  CAnimation *m_pActiveTop;
  CAnimation *m_pActiveBottom;
  // pressed top and bottom parts
  CAnimation *m_pPressedTop;
  CAnimation *m_pPressedBottom;

  // the three parts of the scroll button - passive
  CAnimation *m_pPassiveButtonTop;
  CAnimation *m_pPassiveButtonMiddle;
  CAnimation *m_pPassiveButtonBottom;

  // the three parts of the scroll button - active
  CAnimation *m_pActiveButtonTop;
  CAnimation *m_pActiveButtonMiddle;
  CAnimation *m_pActiveButtonBottom;

  // the three parts of the scroll button - pressed
  CAnimation *m_pPressedButtonTop;
  CAnimation *m_pPressedButtonMiddle;
  CAnimation *m_pPressedButtonBottom;

  friend class CScrollControl;
};

#endif // !defined(AFX_SCROLLCONTROLLAYOUT_H__22FDD4EB_5081_11D3_A02E_DD87D6C1E031__INCLUDED_)
