// MapexInstanceSelection.h: interface for the CMapexInstanceSelection class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MAPEXINSTANCESELECTION_H__737092D3_8498_11D3_A09B_A3110230D631__INCLUDED_)
#define AFX_MAPEXINSTANCESELECTION_H__737092D3_8498_11D3_A09B_A3110230D631__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "DataObjects\EMapexInstance.h"
class CEMap;

class CMapexInstanceSelection : public CObject  
{
  DECLARE_DYNAMIC(CMapexInstanceSelection);

public:
  // Moves selected mapexes one level up
  void MoveSelectionUp(CEMap *pMap);
  // Moves selected mapexes one level down
  void MoveSelectionDown(CEMap *pMap);

  // Returns the highest level in which some of selected mapexes is
  DWORD GetHighestLevel(CEMap *pMap);
  // Returns the lowest level in which some of selected mapexes is
  DWORD GetLowestLevel(CEMap *pMap);

	CEMapexInstance * GetMapexFromCell(DWORD dwX, DWORD dwY);
	void Copy(CMapexInstanceSelection *pSource);
	void SetPosition(DWORD dwX, DWORD dwY, CEMap *pMap);
  // returns TRUE if the given point (in mapcells) is in some
  // of the selected mapex instances (takes care about landscapes)
  // (it means, if there is some with landscape 0 (under the point)-> takes other)
	BOOL PtInSelection(DWORD dwX, DWORD dwY);
  // returns TRUE if there is no selected mapex instance
	BOOL IsEmpty();
  // deletes all mapex instances in the selection from the given map
	void DeleteSelection(CEMap *pMap);

  // returns text to be displayed in status bar as description of the selection
	CString GetStatusBarText(CEMap *pMap);

  // returns rect (in mapcells) which outlines all mapex instances in selection
	CRect GetBoundingRect();

	// adds whole rectangle (means every mapex instance which intersects the rectangle)
  // doesn't include invisible levels (and the unit level)
  // returns TRUE if some new mapex appeared (or some old was kicked off)
  BOOL SetRect(CRect rcSelection, CEMap *pMap);
  // removes mapex instance from selection
	void RemoveInstance(CEMapexInstance *pInstance);
  // returns TRUE if the given mapex instance is in the selection
	BOOL Includes(CEMapexInstance *pInstance);
  // adds mapex instance to the selection
  // returns FALSE if the mapex is already in selection
	BOOL AddInstance(CEMapexInstance *pInstance);
  // clears the selection
	void Clear();

	CMapexInstanceSelection();
	virtual ~CMapexInstanceSelection();

#ifdef _DEBUG
  virtual void AssertValid() const;
  virtual void Dump(CDumpContext &dc) const;
#endif

private:
  // list containing all selected mapex instacnes
  CTypedPtrList<CPtrList, CEMapexInstance *> m_listInstances;

  friend class CClipboard;
};

#endif // !defined(AFX_MAPEXINSTANCESELECTION_H__737092D3_8498_11D3_A09B_A3110230D631__INCLUDED_)
