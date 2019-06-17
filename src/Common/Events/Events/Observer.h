// Observer.h: interface for the CObserver class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_OBSERVER_H__D0A2D057_1644_11D2_8AFD_000000000000__INCLUDED_)
#define AFX_OBSERVER_H__D0A2D057_1644_11D2_8AFD_000000000000__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

class CObserver : public CPersistentObject  
{
public:
  DECLARE_DYNAMIC(CObserver);

#ifdef _DEBUG
public:
  virtual void AssertValid() const;
  virtual void Dump(CDumpContext &dc) const;
#endif

public:
	virtual BOOL Perform(DWORD NotID, DWORD Event, DWORD Param);
	CObserver();
	virtual ~CObserver();

};

#define DECLARE_OBSERVER_MAP(classname) \
public: \
  virtual BOOL Perform(DWORD dwNotID, DWORD dwEvent, DWORD dwParam);

#define BEGIN_OBSERVER_MAP(classname, baseclassname) \
  BOOL classname::Perform(DWORD dwNotID, DWORD dwEvent, DWORD dwParam) \
  { \
    switch(dwNotID){

#define END_OBSERVER_MAP(classname, baseclassname) \
    }  \
    return baseclassname::Perform(dwNotID, dwEvent, dwParam); \
  }

#define BEGIN_NOTIFIER(notid) \
    case notid: \
      switch(dwEvent){

#define END_NOTIFIER() \
      }  \
      break;

#define EVENT(event) \
      case event: \


#endif // !defined(AFX_OBSERVER_H__D0A2D057_1644_11D2_8AFD_000000000000__INCLUDED_)
