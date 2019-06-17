// ScriptViewEdit.h: interface for the CScriptViewEdit class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SCRIPTVIEWEDIT_H__13246FC3_E078_11D3_A8C3_00105ACA8325__INCLUDED_)
#define AFX_SCRIPTVIEWEDIT_H__13246FC3_E078_11D3_A8C3_00105ACA8325__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CScriptViewEdit : public CScriptEdit  
{
  DECLARE_DYNAMIC(CScriptViewEdit);
  DECLARE_OBSERVER_MAP(CScriptViewEdit);

public:
  // constructor & destructor
	CScriptViewEdit();
	virtual ~CScriptViewEdit();

  // debug functions
#ifdef _DEBUG
  virtual void AssertValid() const;
  virtual void Dump(CDumpContext &dc) const;
#endif

  enum Events{
    E_CONTEXTMENU = 0x0100
  };
protected:
	void OnRButtonUp(CPoint point);
};

#endif // !defined(AFX_SCRIPTVIEWEDIT_H__13246FC3_E078_11D3_A8C3_00105ACA8325__INCLUDED_)
