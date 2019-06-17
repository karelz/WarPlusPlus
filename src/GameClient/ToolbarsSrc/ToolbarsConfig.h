// ToolbarsConfig.h: interface for the CToolbarsConfig class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_TOOLBARSCONFIG_H__EDA298D3_2D9B_11D4_84D4_004F4E0004AA__INCLUDED_)
#define AFX_TOOLBARSCONFIG_H__EDA298D3_2D9B_11D4_84D4_004F4E0004AA__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "ToolbarInfo.h"

// Class for loading toolbar configs
class CToolbarsConfig : public CObject  
{
  DECLARE_DYNAMIC(CToolbarsConfig);

public:
  // Constructor & destructor
	CToolbarsConfig();
	virtual ~CToolbarsConfig();

#ifdef _DEBUG
  virtual void AssertValid() const;
  virtual void Dump(CDumpContext &dc) const;
#endif

  // Returns no selection toolbar
  CToolbarInfo * GetNoSelectionToolbar () { return m_pNoSelectionToolbar; }
  // Returns the default toolbar (for different unit types selected)
  CToolbarInfo *GetDefaultToolbar(){ return m_pDefaultToolbar; }
  // Returns the enemy toolbar (for enemy units selected)
  CToolbarInfo *GetEnemyToolbar(){ return m_pEnemyToolbar; }
  // Returns toolbar for given unit type (if not found -> default one)
  CToolbarInfo *GetUnitToolbar(CString strUnitTypeName);

  // creation
  // Creates the toolbars objects from the archive
  // The archive object must contain file named "Toolbars.txt"
  void Create(CDataArchive Archive);

  // Deletes the object
  virtual void Delete();

private:
  // Pointer to the first toolbar info in the list
  CToolbarInfo *m_pToolbars;

  // No selection toolbar
  CToolbarInfo * m_pNoSelectionToolbar;

  // The default toolbar (some different unit types in selection)
  CToolbarInfo *m_pDefaultToolbar;

  // The enemy toolbar (when enemy unit is selected)
  CToolbarInfo *m_pEnemyToolbar;
};

#endif // !defined(AFX_TOOLBARSCONFIG_H__EDA298D3_2D9B_11D4_84D4_004F4E0004AA__INCLUDED_)
