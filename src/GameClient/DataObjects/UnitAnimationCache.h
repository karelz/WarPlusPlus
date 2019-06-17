// UnitAnimationCache.h: interface for the CUnitAnimationCache class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_UNITANIMATIONCACHE_H__00DCBAF3_229A_11D4_84C8_004F4E0004AA__INCLUDED_)
#define AFX_UNITANIMATIONCACHE_H__00DCBAF3_229A_11D4_84C8_004F4E0004AA__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CUnitAnimationCache : public CAbstractCache  
{
  DECLARE_DYNAMIC(CUnitAnimationCache);

public:
	CUnitAnimationCache();
	virtual ~CUnitAnimationCache();

  void Create();
  virtual void Delete();

#ifdef _DEBUG
  virtual void AssertValid() const;
  virtual void Dump(CDumpContext &dc) const;
#endif

protected:
  enum{
    MaxCacheSize = 5000000,
    PrefferedCacheSize = 4000000,
  };

  CCacheLoadQueue m_LoadQueue;
};

#endif // !defined(AFX_UNITANIMATIONCACHE_H__00DCBAF3_229A_11D4_84C8_004F4E0004AA__INCLUDED_)
