#ifndef MAPEXCACHE_H_
#define MAPEXCACHE_H_

// class for caching mapexes
class CMapexCache : public CAbstractCache
{
  DECLARE_DYNAMIC(CMapexCache);

public:
  // Konstruktor 
  CMapexCache();

  // Destruktor
  ~CMapexCache();

// debug functions
#ifdef _DEBUG
  virtual void AssertValid() const;
  virtual void Dump(CDumpContext &dc) const;
#endif
};

#endif