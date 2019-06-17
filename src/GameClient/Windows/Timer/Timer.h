// Timer.h: interface for the CTimer class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_TIMER_H__D1F3EFD2_7693_11D2_AB4F_DED7AE979861__INCLUDED_)
#define AFX_TIMER_H__D1F3EFD2_7693_11D2_AB4F_DED7AE979861__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

class CTimer : public CMultithreadNotifier
{
  DECLARE_DYNAMIC(CTimer)
  DECLARE_OBSERVER_MAP(CTimer)

public:
	double GetPreciseTime();
  // sets the new elapse time for the connection
	void SetElapseTime(CObserver *pObserver, DWORD dwElapseTime);
  // connects the observer to the timer
	void Connect(CObserver *pObserver, DWORD dwElapseTime, DWORD dwNotifierID = DefaultNotID);
  // static Close - deletes the global timer
	static void Close();
  // static Init - creates the gloabl timer
	static BOOL Init(DWORD dwTimeSlice = 50);
  // deletes the object
	virtual void Delete();
  // creates the object
	BOOL Create(DWORD dwTimeSlice = 50);
  // constructor
	CTimer();
  // destructor
	virtual ~CTimer();
  // returns the smallest time slice which the timer can recognize
  DWORD GetTimeSlice(){ return m_dwTimeSlice; }
  // return tick count from the start of the system
  // (in milliseconds) use only relative values
  DWORD GetTime(){ return m_dwTime; }

  enum{ DefaultNotID = 0x0FFFF0001 };
  enum{ E_TICK = 1 };

#ifdef _DEBUG
  virtual void AssertValid() const;
  virtual void Dump(CDumpContext &dc) const;
#endif

private:
  // current tick count (in milliseconds)
	DWORD m_dwTime;
  // call back for the system timer
	static void CALLBACK CallbackProc(HWND hWnd, UINT uMsg, UINT idEvent, DWORD dwTime);
  // ID of the system timer
	UINT m_nTimerID;
  // the smallest time slice which the timer can recognize
	DWORD m_dwTimeSlice;

  enum{ TimerID = 1 };

  struct tagSTimerConnectionData{
    DWORD m_dwElapseTime; // ellapse time of this connection
	  DWORD m_dwCurrentElapseTime; // current time from the last event
  };
  typedef struct tagSTimerConnectionData STimerConnectionData;
protected:
  // reaction on abnormal program termination
  // deletes the object
	void OnAbort(DWORD dwExitCode);
  // removes additional information about the connection
	virtual void DeleteNode(SObserverNode *pNode);
};

// global pointer to the timer
extern CTimer *g_pTimer;

#define BEGIN_TIMER() BEGIN_NOTIFIER(CTimer::DefaultNotID)
#define END_TIMER() END_NOTIFIER()
#define ON_TIMETICK() \
    case CTimer::E_TICK: \
      OnTimeTick(dwParam); \
      return FALSE;
// always return FALSE, all observers can recieve this message

#endif // !defined(AFX_TIMER_H__D1F3EFD2_7693_11D2_AB4F_DED7AE979861__INCLUDED_)
