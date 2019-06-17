// Clipboard.h: interface for the CClipboard class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_CLIPBOARD_H__1BAC0314_87C9_11D3_A09E_DD1BECDAE931__INCLUDED_)
#define AFX_CLIPBOARD_H__1BAC0314_87C9_11D3_A09E_DD1BECDAE931__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "MapexInstanceSelection.h"

class CClipboard : public CObject  
{
  DECLARE_DYNAMIC(CClipboard);
public:
  struct tagSNode{
    DWORD m_dwMapexID;
    DWORD m_dwX;
    DWORD m_dwY;
    DWORD m_dwLevel;
  };
  typedef struct tagSNode SNode;

	SNode * GetNextPosition(POSITION &pos);
	POSITION GetFirstPosition();
	BOOL IsEmpty();
	void SetMapexInstanceSelection(CMapexInstanceSelection *pSelection, CEMap *pMap);
	void Clear();
	CClipboard();
	virtual ~CClipboard();

#ifdef _DEBUG
  virtual void AssertValid() const;
  virtual void Dump(CDumpContext &dc) const;
#endif

private:
  CTypedPtrList<CPtrList, tagSNode *> m_listNodes;
};

#endif // !defined(AFX_CLIPBOARD_H__1BAC0314_87C9_11D3_A09E_DD1BECDAE931__INCLUDED_)
