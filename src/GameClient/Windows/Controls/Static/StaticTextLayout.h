// StaticTextLayout.h: interface for the CStaticTextLayout class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_STATICTEXTLAYOUT_H__22FDD4E5_5081_11D3_A02E_DD87D6C1E031__INCLUDED_)
#define AFX_STATICTEXTLAYOUT_H__22FDD4E5_5081_11D3_A02E_DD87D6C1E031__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "..\..\Fonts.h"

class CStaticTextLayout : public CObject  
{
  DECLARE_DYNAMIC(CStaticTextLayout)

public:
	CFontObject * GetFont();
	virtual void Delete();
	virtual BOOL Create(CConfigFile *pCfgFile);
	BOOL Create(CArchiveFile CfgFile);
	CStaticTextLayout();
	virtual ~CStaticTextLayout();

#ifdef _DEBUG
  virtual void AssertValid() const;
  virtual void Dump(CDumpContext &dc) const;
#endif

protected:
	DWORD m_dwTextColor;
	int m_eAlign;
	CFontObject * m_pFont;

  friend class CStaticText;
};

#endif // !defined(AFX_STATICTEXTLAYOUT_H__22FDD4E5_5081_11D3_A02E_DD87D6C1E031__INCLUDED_)
