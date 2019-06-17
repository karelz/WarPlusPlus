// EUnitGraphics.h: interface for the CEUnitGraphics class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_EUNITAPPEARANCE_H__D6970FB3_8B08_11D3_A0A8_FA054450EB31__INCLUDED_)
#define AFX_EUNITAPPEARANCE_H__D6970FB3_8B08_11D3_A0A8_FA054450EB31__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "EUnitAnimation.h"
#include "EUnitAppearanceType.h"

class CEUnitAppearance : public CObject  
{
  DECLARE_DYNAMIC(CEUnitAppearance)

public:
	// removes the direction (also removes the graphic files from given archive)
  void RemoveDirection(DWORD dwDirection, CDataArchive Archive);
  // adds new direction
  // if the direction already exists returns NULL
	CEUnitAnimation * AddDirection(DWORD dwDirection);

  // gets the color of the appearance
  DWORD GetColor(){ return m_dwColor; }

  // removes the graphics from the disk
	void RemoveGraphics(CDataArchive Archive);
  // stores the anim file names into the appearance type object
	void SaveToType(CEUnitAppearanceType *pType);
  // creates the object from the given UnitAppearanceType and from the given color
  // (has to know the archive to find out the animations)
	BOOL Create(CEUnitAppearanceType *pType, DWORD dwColor, CDataArchive Archive);
  // deletes the object
	virtual void Delete();

  // gets one direction
  CEUnitAnimation *GetDirection(DWORD dwDirection){
    ASSERT(dwDirection < 8); return m_aDirections[dwDirection];
  }

  // constructor & destructor
	CEUnitAppearance();
	virtual ~CEUnitAppearance();

#ifdef _DEBUG
  virtual void AssertValid() const;
  virtual void Dump(CDumpContext &dc) const;
#endif

private:
  // the color of the unit
  DWORD m_dwColor;

  // eight directions of the unit
  // some can be NULL
  CEUnitAnimation *m_aDirections[8];
};

#endif // !defined(AFX_EUNITAPPEARANCE_H__D6970FB3_8B08_11D3_A0A8_FA054450EB31__INCLUDED_)
