// EUnitAnimation.h: interface for the CEUnitAnimation class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_EUNITANIMATION_H__43B8C723_8B70_11D3_A0A9_F5E9C97CE931__INCLUDED_)
#define AFX_EUNITANIMATION_H__43B8C723_8B70_11D3_A0A9_F5E9C97CE931__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "EUnitSurface.h"

class CEUnitAnimation : public CObject  
{
  DECLARE_DYNAMIC(CEUnitAnimation)

public:
  // removes associated graphic files from the given archive
	void RemoveGraphics(CDataArchive Archive);
  // sets new graphics
  // also copies the graphic files into the archive (with new names)
  // the names are generated from ids (unit, appearance, direction)
	void SetGraphics(DWORD dwUnitID, DWORD dwAppearanceID, DWORD dwDirection, CString strGraphicsFile, CDataArchive archive);

  // get looping
  BOOL Loop(){ return m_bLoop; }

  // creates the animation from given config file and coloring params
  BOOL Create(CConfigFile *pConfig, CEUnitSurface::SColoringParams *pColoringParams);
	// creates the animation from image file (should be the anim file)
  // the dwColor is the color of the civilization
  BOOL Create(CArchiveFile file, DWORD dwColor);
  // deletes the animation
	virtual void Delete();
  // creates empty animation
	BOOL Create(DWORD dwColor = 0);

  // returns the file name of the animation
  CString GetFileName(){ return m_strFileName; }

  // returns number of frames in animation
  DWORD GetFrameNum(){ return m_aFrames.GetSize(); }
  // returns speed (delay between frames in milliseconds
  DWORD GetSpeed(){ return m_dwSpeed; }

  // returns the graphical offset - X
  int GetXGraphicalOffset(){ return m_nXGraphicalOffset; }
  // returns the graphical offset - Y
  int GetYGraphicalOffset(){ return m_nYGraphicalOffset; }

  // returns frame surface for given frame index (dwFrameNum)
  CEUnitSurface *GetFrame(DWORD dwFrameNum){ if((int)dwFrameNum >= m_aFrames.GetSize()) return NULL;
    return m_aFrames[dwFrameNum]; }

	CEUnitAnimation();
	virtual ~CEUnitAnimation();

#ifdef _DEBUG
  virtual void AssertValid() const;
  virtual void Dump(CDumpContext &dc) const;
#endif

private:
	void DeleteAnimation();
  // speed of the animation
  DWORD m_dwSpeed;
  // frames array
  CTypedPtrArray<CPtrArray, CEUnitSurface *> m_aFrames;
  // loop it ?
  BOOL m_bLoop;

  // Graphical offset - all images will be draw moved by this offset to top left
  int m_nXGraphicalOffset;
  int m_nYGraphicalOffset;

  // filename of the animations graphics
  CString m_strFileName;

  // the dest color
  DWORD m_dwDestColor;
};

#endif // !defined(AFX_EUNITANIMATION_H__43B8C723_8B70_11D3_A0A9_F5E9C97CE931__INCLUDED_)
