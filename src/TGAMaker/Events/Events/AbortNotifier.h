// AbortNotifier.h: interface for the CAbortNotifier class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_ABORTNOTIFIER_H__CB0813E4_B83A_11D2_ABE5_E5A84AFB5160__INCLUDED_)
#define AFX_ABORTNOTIFIER_H__CB0813E4_B83A_11D2_ABE5_E5A84AFB5160__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "Notifier.h"

class CAbortNotifier : public CNotifier  
{
  DECLARE_DYNAMIC(CAbortNotifier);

public:
	void Connect(CObserver *pObserver, DWORD dwNotID = DefaultNotID);
	CAbortNotifier();
	virtual ~CAbortNotifier();

  enum { DefaultNotID = 0x0FFFFFFFE };
  enum{
    E_ABORTEVENT = 1
  };

#ifdef _DEBUG
  virtual void AssertValid() const;
  virtual void Dump(CDumpContext &dc) const;
#endif
};

// This function will abort whole application
// It won't exit the application
// it just close all graphics and sound and so on ...
void AbortApplication(DWORD dwErrorCode = 0);

#define BEGIN_ABORT() BEGIN_NOTIFIER(CAbortNotifier::DefaultNotID)
#define END_ABORT() END_NOTIFIER()
#define ON_ABORT() \
    case CAbortNotifier::E_ABORTEVENT: \
      OnAbort(dwParam); \
      return TRUE;

extern CAbortNotifier g_AbortNotifier;

#endif // !defined(AFX_ABORTNOTIFIER_H__CB0813E4_B83A_11D2_ABE5_E5A84AFB5160__INCLUDED_)
