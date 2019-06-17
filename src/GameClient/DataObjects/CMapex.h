#ifndef CMAPEX_H_
#define CMAPEX_H_

#include "MapexCache.h"

// mapex on the client
class CCMapex : public CCachedObject
{
public:
  // constructor and destructor
  CCMapex();
  ~CCMapex();
  
// cache
  // inits the global cache object for all mapexes
  static void InitCache();
  // closes the global cache
  static void CloseCache();
  
// creation
  // loads mapex from the file (saved by map editor)
  // doesn't load any graphics, just remember the graphics location
  void Create(CArchiveFile MapexFile);
 
  // Deletes the mapex
  void Delete();
  
	 // ...

// Get/Set methods
  // Returns the ID of the mapex
  DWORD GetID(){ return m_dwID; };

  // returns the Width
  DWORD GetWidth(){ return m_dwXSize; };
  // returns the heiight
  DWORD GetHeight(){ return m_dwYSize; };

  // returns pointer to the animation object
  // you should lock this object to get the anim in the right state
  CAnimation *GetGraphics(){ return &m_Animation; }

  
protected:
// Cache functions
  // Returns size of the object in bytes (if not loaded just guess the size by assuming it's one frame anim)
  virtual DWORD GetSize();

  // Loads object into memory -> loads the graphics
  virtual void Load();
  // Unloads the object from memory -> frees graphics
  virtual void Unload();

private:
// Data
  // ID
  DWORD m_dwID;

  // size in mapcells of this mapex
  DWORD m_dwXSize;
  DWORD m_dwYSize;
  
  // the land type information
  // 2D array of BYTEs of m_dwXSize, m_dwYSize sizes
  BYTE *m_pLandTypes;
  
// Graphics
  // file where the graphics is stored
  CArchiveFile m_fileGraphics;
   
  // the animation object
  // this is the graphic of the mapex
  // this will be loaded or freed (for caching)
  CAnimation m_Animation;
   
// Cache
  // the only instance of the mapex cache
  static CMapexCache *m_pMapexCache;

// List pointers
  // next mapex in the list of all mapexes in the map
  CCMapex *m_pMapNext;

  // map is our friend cause of the list (see above)
  friend class CCMap;
};

#endif