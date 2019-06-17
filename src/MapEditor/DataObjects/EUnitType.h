// EUnit.h: interface for the CEUnit class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_EUNIT_H__CB5410F1_8E0D_11D3_A876_00105ACA8325__INCLUDED_)
#define AFX_EUNIT_H__CB5410F1_8E0D_11D3_A876_00105ACA8325__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "EUnitAppearanceType.h"
#include "..\AbstractDataClasses\256BitArray.h"
#include "EUnitSkillType.h"

class CEUnitType : public CObject  
{
  DECLARE_DYNAMIC(CEUnitType)

public:
  // Sets the Z pos for the unit after its create
  void SetZPos(DWORD dwZPos){ m_dwZPos = dwZPos; }
  // Returns the Z pos for the unit after its create
  DWORD GetZPos(){ return m_dwZPos; }
  
  BOOL CheckValid();

  void MoveSkillTypeUp(CEUnitSkillType *pSkillType);
  void MoveSkillTypeDown(CEUnitSkillType *pSkillType);
  CEUnitSkillType *GetPrevSkillType(CEUnitSkillType *pSkillType);
  CEUnitSkillType *GetNextSkillType(CEUnitSkillType *pSkillType);

	CEUnitAppearanceType * GetAppearanceType(DWORD dwID);
  CEUnitAppearanceType * GetSelectionMarkAppearance(){ return m_pSelectionMarkAppearance; }

	void DeleteSkillType(CEUnitSkillType *pSkillType);
	CEUnitSkillType * AddSkillType(CString strName);

	void FillModesListCtrl(CListCtrl *pListCtrl);
	void FillAppearancesCombo(CComboBox *pComboBox, BOOL bHidden = FALSE);
	void FillModesCombo(CComboBox *pComboBox);
  // removes appearance type from the unit
	void DeleteAppearance(CEUnitAppearanceType *pAppType);
  // returns new appearance ID
	DWORD GetNewAppearanceID();
  // creates new appearance type for this unit
	CEUnitAppearanceType * NewAppearance();

  // fills the given list control with all appearances and as the item data
  // sets th pointer to the CEUnitAppearanceType object
	void FillAppearancesListCtrl(CListCtrl *pListCtrl, BOOL bHidden = FALSE);

  // releases all graphics for hte unit from memory
	void ReleaseGraphics();
  // loads and instantiate all graphics for this unit
	void LoadGraphics();

  // remove all graphics for this unit from the archive
	void RemoveGraphics(CDataArchive Archive);
  // returns the default appearance of the unti
	CEUnitAppearanceType * GetDefaultAppearanceType();

  // save the unit to the archive
  // this creates the unit file (from its id)
	void SaveUnit(CDataArchive Archive);

  // creates the unit from file
	BOOL Create(CArchiveFile file);
  // creates new unit object
  BOOL Create(DWORD dwID);
  // deletes the unit
	virtual void Delete();

  // sets the archive in which the unit is stored
  void SetArchive(CDataArchive Archive);
  
  // returns name
  CString GetName(){ return m_strName; }
  // sets the name
  void SetName(CString strName){ m_strName = strName; }

  // returns flags
  DWORD GetFlags(){ return m_dwFlags; }
  // sets new flags
  void SetFlags(DWORD dwFlags){ m_dwFlags = dwFlags; }

  // returns invisibility flags for given mode
  DWORD GetInvisibilityFlags(DWORD dwMode){ ASSERT(dwMode < 8); return m_aInvisibilityFlags[dwMode]; }
  // sets invisibility flags for given mode
  void SetInvisibilityFlags(DWORD dwMode, DWORD dwFlags){ ASSERT(dwMode < 8); m_aInvisibilityFlags[dwMode] = dwFlags; }
  // returns detection flags for given mode
  DWORD GetDetectionFlags(DWORD dwMode){ ASSERT(dwMode < 8); return m_aDetectionFlags[dwMode]; }
  // sets detection flags for given mode
  void SetDetectionFlags(DWORD dwMode, DWORD dwFlags){ ASSERT(dwMode < 8); m_aDetectionFlags[dwMode] = dwFlags; }

  // get/set max life
  DWORD GetLifeMax(){ return m_dwLifeMax; }
  void SetLifeMax(DWORD dwLifeMax){ m_dwLifeMax = dwLifeMax; }

  // get/set view radius
  DWORD GetVeiwRadius(){ return m_dwViewRadius; }
  void SetViewRadius(DWORD dwViewRadius){ m_dwViewRadius = dwViewRadius; }

  // get/set move width
  DWORD GetMoveWidth(DWORD dwMode){ ASSERT(dwMode < 8); return m_aMoveWidth[dwMode]; }
  void SetMoveWidth(DWORD dwMode, DWORD dwMoveWidth){ ASSERT(dwMode < 8); m_aMoveWidth[dwMode] = dwMoveWidth; }

  // get/set mode names
  CString GetModeName(DWORD dwMode){ ASSERT(dwMode < 8); return m_aModeNames[dwMode]; }
  void SetModeName(DWORD dwMode, CString strName){ ASSERT(dwMode < 8); m_aModeNames[dwMode] = strName; }

  // get/set path graphs
  DWORD GetPathGraphID(DWORD dwMode){ ASSERT(dwMode < 8); return m_aPathGraphs[dwMode]; }
  void SetPathGraphID(DWORD dwMode, DWORD dwID){ ASSERT(dwMode < 8); m_aPathGraphs[dwMode] = dwID; }

  // get/set default appearance id for the modes
  DWORD GetDefaultAppearance(DWORD dwMode){ ASSERT(dwMode < 8); return m_aDefaultAppearances[dwMode]; }
  void SetDefaultAppearance(DWORD dwMode, DWORD dwAppearanceID){ ASSERT(dwMode < 8); m_aDefaultAppearances[dwMode] = dwAppearanceID; }

  // iterates through the skills list
  POSITION GetFirstSkillTypePosition(){ return m_listSkillTypes.GetHeadPosition(); }
  CEUnitSkillType *GetNextSkillType(POSITION &pos){ return m_listSkillTypes.GetNext(pos); }

  // returns the file name
  CString GetUnitFileName(){ return m_strUnitFileName; }
  // returns the ID of the unit
  DWORD GetID(){ return m_dwID; }

  // constructor & destructor
	CEUnitType();
	virtual ~CEUnitType();

#ifdef _DEBUG
  virtual void AssertValid() const;
  virtual void Dump(CDumpContext &dc) const;
#endif

private:
  // id of the unit
  DWORD m_dwID;

  // appearances (the graphics) (only types, don't know the color yet)
  CTypedPtrList<CPtrList, CEUnitAppearanceType *> m_listAppearances;

  // special appearance - the selection mark
  CEUnitAppearanceType *m_pSelectionMarkAppearance;

  // the file name
  CString m_strUnitFileName;

  // the name
  CString m_strName;
  // the max life number
  DWORD m_dwLifeMax;
  // the view radius (in mapcells)
  DWORD m_dwViewRadius;
  // the array of the width for moving (in mapcells) (each item for one mode)
  // if the unit is standing it occupies the square of the size m_dwMoveWidth
  DWORD m_aMoveWidth[8];

  // Path graphs for all modes (its IDs)
  // TODO: replace with pointers to CEFindPathGraph
  DWORD m_aPathGraphs[8];

  // names of modes (only for editor purposes)
  CString m_aModeNames[8];

  // default appearance IDs for all modes
  DWORD m_aDefaultAppearances[8];

  // the list of all skill types
  CTypedPtrList<CPtrList, CEUnitSkillType *> m_listSkillTypes;

  // Some flags for the unit
  DWORD m_dwFlags;

  // Z pos after the unit create
  DWORD m_dwZPos;

  // Invisibility flags for modes
  DWORD m_aInvisibilityFlags[8];
  // Detection flags for modes
  DWORD m_aDetectionFlags[8];

  // the archive from which is the unit loaded -> used for internal proposes
  CDataArchive m_Archive;
};

#endif // !defined(AFX_EUNIT_H__CB5410F1_8E0D_11D3_A876_00105ACA8325__INCLUDED_)
