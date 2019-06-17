// KeyAccelerators.h: interface for the CKeyAccelerators class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_KEYACCELERATORS_H__C99F12F5_4CCF_11D3_A026_978C3AFB5730__INCLUDED_)
#define AFX_KEYACCELERATORS_H__C99F12F5_4CCF_11D3_A026_978C3AFB5730__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "Keyboard.h"

class CKeyAccelerators : public CNotifier
{
  DECLARE_DYNAMIC(CKeyAccelerators);

public:
	virtual void DeleteNode(SObserverNode *pNode);
  void Connect(CWindow *pWindow, UINT nChar, DWORD dwFlags, DWORD dwNotID = CKeyboard::DefaultNotID);
	virtual void Delete();
	BOOL Create();
	CKeyAccelerators();
	virtual ~CKeyAccelerators();

#ifdef _DEBUG
  virtual void AssertValid() const;
  virtual void Dump(CDumpContext &dc) const;
#endif

private:
  struct tagSKeyAccNode{
    UINT nChar;
    DWORD dwFlags;
    struct tagSKeyAccNode *pNext;
  };
  typedef struct tagSKeyAccNode SKeyAccNode;

  struct tagSKeyAccConnectionData{
    SKeyAccNode *pNodes;
  };
  typedef struct tagSKeyAccConnectionData SKeyAccConnectionData;

	CKeyAccelerators * m_pPrevKeyAcc;
protected:
  BOOL DoAccelerator(CKeyboard::SKeyInfo *pInfo);

  friend CKeyboard;
};

extern CKeyAccelerators *g_pKeyAccelerators;

#endif // !defined(AFX_KEYACCELERATORS_H__C99F12F5_4CCF_11D3_A026_978C3AFB5730__INCLUDED_)
