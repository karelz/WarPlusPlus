// TreeControlLayout.h: interface for the CTreeControlLayout class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_TREECONTROLLAYOUT_H__4D86CBF4_7F3C_11D4_B0F5_004F49068BD6__INCLUDED_)
#define AFX_TREECONTROLLAYOUT_H__4D86CBF4_7F3C_11D4_B0F5_004F49068BD6__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "ListControlLayout.h"

class CTreeControlLayout : public CListControlLayout  
{
public:
	CTreeControlLayout();
	virtual ~CTreeControlLayout();

#ifdef _DEBUG
  virtual void AssertValid() const;
  virtual void Dump(CDumpContext &dc) const;
#endif

	virtual void Delete();
	virtual BOOL Create(CConfigFile *pCfgFile);
	BOOL Create(CArchiveFile CfgFile);

  CAnimation *m_pPlus;
  CAnimation *m_pMinus;

  DWORD m_dwIndentation;

  friend class CTreeControl;
};

#endif // !defined(AFX_TREECONTROLLAYOUT_H__4D86CBF4_7F3C_11D4_B0F5_004F49068BD6__INCLUDED_)
