#ifndef CUNITANIMATION_H_
#define CUNITANIMATION_H_

#include "CUnitSurface.h"
#include "UnitAnimationCache.h"

#include "CUnitSurfaceManager.h"

// class representing one unit animation (one direction in one appearance)
// it's in fact animation, which has the color in addition
class CCUnitAnimation : public CCachedObject
{
  DECLARE_DYNAMIC(CCUnitAnimation);

public:
  // constructor & destructor
  CCUnitAnimation();
  virtual ~CCUnitAnimation();

#ifdef _DEBUG
  virtual void AssertValid() const;
  virtual void Dump(CDumpContext &dc) const;
#endif

// Creation
  // creates the object, just envelope - no data will be loaded
  void Create(CArchiveFile File, DWORD dwColor);
  // deletes the object
  void Delete();

  // loads object into the memory
  void Load();
  // unloads it
  void Unload();

  // Cache functions
  static void InitCache();
  static void CloseCache();

  static void InitManager();
  static void CloseManager();

// Get/Set functions
  // returns number of frames
  DWORD GetFramesCount(){ return m_dwFramesCount; }
  // returns the speed
  DWORD GetSpeed(){ return m_dwSpeed; }
  // returns the loop
  BOOL GetLoop(){ return m_bLoop; }

  // returns X graphical offset
  int GetXGraphicalOffset(){ return m_nXGraphicalOffset; }
  // returns Y graphical offset
  int GetYGraphicalOffset(){ return m_nYGraphicalOffset; }


  // returns the frame by index
  CCUnitSurface *GetFrame(DWORD dwIndex){
    ASSERT(dwIndex < m_dwFramesCount);
    return m_aFrames[dwIndex];
  }

private:
// Private methods
  // load anim file
  void LoadAnim(CConfigFile *pCfgFile);

// Data
  // array of frames
  // here we will allocate it dynamicaly at load time
  CCUnitSurface **m_aFrames;

  // number of frames
  DWORD m_dwFramesCount;

  // the speed
  DWORD m_dwSpeed;

  // looping
  BOOL m_bLoop;

  // the file where is graphics
  CArchiveFile m_File;

  // color which to colorize loaded images
  DWORD m_dwColor;

  // coloring params
  CCUnitSurface::SColoringParams m_sColoringParams;

  // graphical offsets
  int m_nXGraphicalOffset;
  int m_nYGraphicalOffset;

  static CCUnitSurfaceManager *m_pUnitSurfaceManager;

  static CUnitAnimationCache *m_pAnimationCache;
};

#endif