// UnitInterpolations.h: interface for the CUnitInterpolations class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_UNITINTERPOLATIONS_H__3971C523_24E8_11D4_84CB_004F4E0004AA__INCLUDED_)
#define AFX_UNITINTERPOLATIONS_H__3971C523_24E8_11D4_84CB_004F4E0004AA__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CCMap;
class CUnitCache;

class CUnitInterpolations : public CObserver  
{
  DECLARE_DYNAMIC(CUnitInterpolations)
  DECLARE_OBSERVER_MAP(CUnitInterpolations)

public:
  // constructor & destructor
	CUnitInterpolations();
	virtual ~CUnitInterpolations();

  // creation
  void Create(CCMap *pMap, CUnitCache *pUnitCache);
  virtual void Delete();

//debug functions
#ifdef _DEBUG
  virtual void AssertValid() const;
  virtual void Dump(CDumpContext &dc) const;
#endif

protected:
  // the time tick routine
  // here we do ll the job
  void OnTimeTick(DWORD dwTime);

private:
  // Pointer to the map object
  CCMap *m_pMap;

  // The unit cache (list of all units)
  CUnitCache *m_pUnitCache;

  // Our thread to do the interpolations in
  static UINT _InterpolationsThread(LPVOID pParam);
  void InterpolationsThread();
  CWinThread *m_pInterpolationsThread;

  DWORD m_dwLastTickTime;

  // event manager for the interpolations thread
  CEventManager m_EventManager;
};

#endif // !defined(AFX_UNITINTERPOLATIONS_H__3971C523_24E8_11D4_84CB_004F4E0004AA__INCLUDED_)
