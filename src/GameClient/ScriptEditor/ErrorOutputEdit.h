// ErrorOutputEdit.h: interface for the CErrorOutputEdit class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_ERROROUTPUTEDIT_H__E2BB0203_1FFA_11D4_84C5_004F4E0004AA__INCLUDED_)
#define AFX_ERROROUTPUTEDIT_H__E2BB0203_1FFA_11D4_84C5_004F4E0004AA__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CErrorOutputEdit : public CScriptEdit  
{
  DECLARE_DYNAMIC(CErrorOutputEdit);
  DECLARE_OBSERVER_MAP(CErrorOutputEdit);

public:
  // constructor & destructor
	CErrorOutputEdit();
	virtual ~CErrorOutputEdit();

  // debug functions
#ifdef _DEBUG
  virtual void AssertValid() const;
  virtual void Dump(CDumpContext &dc) const;
#endif

  // creation
  void Create(CRect &rcBound, CScriptEditLayout *pLayout, CWindow *pParent);
  virtual void Delete();

  enum{
    E_LineSelected = 0x0100,
  };

protected:
  // reaction on mouse events
  void OnLButtonDblClk(CPoint point);
  // reaction on keyboard events
  BOOL OnKeyDown(DWORD dwKey, DWORD dwFlags);
};

#endif // !defined(AFX_ERROROUTPUTEDIT_H__E2BB0203_1FFA_11D4_84C5_004F4E0004AA__INCLUDED_)
