// ToolbarInfo.h: interface for the CToolbarInfo class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_TOOLBARINFO_H__EDA298D5_2D9B_11D4_84D4_004F4E0004AA__INCLUDED_)
#define AFX_TOOLBARINFO_H__EDA298D5_2D9B_11D4_84D4_004F4E0004AA__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "ToolbarsErrorOutput.h"
#include "ToolbarButtonInfo.h"

// Object representing one toolbar for one unit type
class CToolbarInfo : public CObject  
{
  DECLARE_DYNAMIC(CToolbarInfo);
public:
  // Constructor & destructor
	CToolbarInfo();
	virtual ~CToolbarInfo();

  // Debug functions
#ifdef _DEBUG
  virtual void AssertValid() const;
  virtual void Dump(CDumpContext &dc) const;
#endif

  // Returns unit type name
  CString GetUnitTypeName(){ return m_strUnitTypeName; }

  // Returns number of buttons
  DWORD GetButtonCount(){ return m_dwButtonsCount; }
  // Returns button from index
  CToolbarButtonInfo *GetButtonInfo(DWORD dwButton){ ASSERT(dwButton < m_dwButtonsCount); return m_pButtons[dwButton]; }

  // Returns button for right click on position
  CToolbarButtonInfo * GetRightClickPositionButton () { ASSERT_VALID ( this ); return m_pRightClickPositionButton; }
  // Returns button for right click on my unit
  CToolbarButtonInfo * GetRightClickMyUnitButton () { ASSERT_VALID ( this ); return m_pRightClickMyUnitButton; }
  // Returns button for right click on enemy unit
  CToolbarButtonInfo * GetRightClickEnemyUnitButton () { ASSERT_VALID ( this ); return m_pRightClickEnemyUnitButton; }
  // Returns button for shift right click on position
  CToolbarButtonInfo * GetShiftRightClickPositionButton () { ASSERT_VALID ( this ); return m_pShiftRightClickPositionButton; }
  // Returns button for shift right click on my unit
  CToolbarButtonInfo * GetShiftRightClickMyUnitButton () { ASSERT_VALID ( this ); return m_pShiftRightClickMyUnitButton; }
  // Returns button for shift right click on enemy unit
  CToolbarButtonInfo * GetShiftRightClickEnemyUnitButton () { ASSERT_VALID ( this ); return m_pShiftRightClickEnemyUnitButton; }

  // creation
  // Creates the toolbar info from config file section
  // Returns TRUE if the creation was successfull
  BOOL Create(CDataArchive Archive, CString strUnitTypeName, CConfigFileSection *pSection, CToolbarsErrorOutput *pErrorOutput);

  // Deletes the toolbar info
  virtual void Delete();

private:
  // name of the unit type (this is identification of the toolbar)
  CString m_strUnitTypeName;

  // Number of buttons on the toolbar
  DWORD m_dwButtonsCount;
  // Array of pointers to buttons
  CToolbarButtonInfo **m_pButtons;

  // Pointer to the next toolbar info
  CToolbarInfo *m_pNext;

  // The button info for the right click on position
  CToolbarButtonInfo *m_pRightClickPositionButton;
  // The button info for the right click on my unit
  CToolbarButtonInfo *m_pRightClickMyUnitButton;
  // The button info for the right click on enemy unit
  CToolbarButtonInfo *m_pRightClickEnemyUnitButton;
  // The button info for the shift right click on position
  CToolbarButtonInfo *m_pShiftRightClickPositionButton;
  // The button info for the shift right click on my unit
  CToolbarButtonInfo *m_pShiftRightClickMyUnitButton;
  // The button info for the shift right click on enemy unit
  CToolbarButtonInfo *m_pShiftRightClickEnemyUnitButton;

  // Helper function for creating buttons from config files
  CToolbarButtonInfo * CToolbarInfo::CreateButton ( CDataArchive Archive, CConfigFileSection SubSection,
    CToolbarsErrorOutput * pErrorOutput, bool bCanBeSeparator = true );

  friend class CToolbarsConfig;
};

#endif // !defined(AFX_TOOLBARINFO_H__EDA298D5_2D9B_11D4_84D4_004F4E0004AA__INCLUDED_)
