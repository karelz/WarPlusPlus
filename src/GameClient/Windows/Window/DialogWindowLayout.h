// DialogWindowLayout.h: interface for the CDialogWindowLayout class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_DIALOGWINDOWLAYOUT_H__0ED47411_A8F2_11D3_99BA_D7A76AF4F871__INCLUDED_)
#define AFX_DIALOGWINDOWLAYOUT_H__0ED47411_A8F2_11D3_99BA_D7A76AF4F871__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "CaptionWindowLayout.h"
#include "..\Controls\Buttons.h"

class CDialogWindowLayout : public CCaptionWindowLayout  
{
  DECLARE_DYNAMIC(CDialogWindowLayout)

public:
	BOOL Create(CArchiveFile CfgFile);
	virtual void Delete();
	virtual BOOL Create(CConfigFile *pCfgFile);
	CDialogWindowLayout();
	virtual ~CDialogWindowLayout();

#ifdef _DEBUG
  virtual void AssertValid() const;
  virtual void Dump(CDumpContext &dc) const;
#endif

  // layout of the close button
  CImageButtonLayout m_CloseButtonLayout;
  // the margins of the close button from the top right sides of the window
  DWORD m_dwCloseRightMargin, m_dwCloseTopMargin;
  // the margins for close button defining the sensitive area
  DWORD m_dwCloseDummyLeft, m_dwCloseDummyRight, m_dwCloseDummyTop, m_dwCloseDummyBottom;

private:

  friend class CDialogWindow;
};

#endif // !defined(AFX_DIALOGWINDOWLAYOUT_H__0ED47411_A8F2_11D3_99BA_D7A76AF4F871__INCLUDED_)
