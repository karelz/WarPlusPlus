// MapEditorDoc.h : interface of the CMapEditorDoc class
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_MAPEDITORDOC_H__43E086CD_6434_11D3_A054_ADE3A89A0638__INCLUDED_)
#define AFX_MAPEDITORDOC_H__43E086CD_6434_11D3_A054_ADE3A89A0638__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


#include "DataObjects\EMap.h"
#include "MiniMap.h"	// Added by ClassView
#include "Clipboard.h"

class CMapEditorDoc : public CDocument
{
protected: // create from serialization only
	CMapEditorDoc();
	DECLARE_DYNCREATE(CMapEditorDoc)

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMapEditorDoc)
	public:
	virtual BOOL OnNewDocument();
	virtual BOOL OnOpenDocument(LPCTSTR lpszPathName);
	virtual BOOL OnSaveDocument(LPCTSTR lpszPathName);
	//}}AFX_VIRTUAL

  enum{
    Update_Window = 1,
    Update_MapexInstance = 2,
    Update_Selection = 3,
    Update_SelectionRect = 4, 
    Update_ULMapexInstance = 5,
    Update_Unit = 6,
    Update_CancelUpdates = 20
  };

// Implementation
public:
	CEMapexInstance * GetSelectedMapexFromCell(DWORD dwX, DWORD dwY);
	void OnChangedMapexZPos(DWORD dwZPos);
	CEULMapexInstance * GetULMapexFromCell(DWORD dwX, DWORD dwY);
  CEUnit * GetUnitFromCell(DWORD dwX, DWORD dwY);
	int GetSelectionType();
	BOOL PtInSelection(DWORD dwX, DWORD dwY);
	CRect GetSelectionBoundingRect();
	void SetSelectedULMapexInstance(CEULMapexInstance *pInstance);
  void SetSelectedUnit(CEUnit *pUnit);
	DWORD GetSelectedZPos();
	CEULMapexInstance * AddULMapexInstance(CPoint ptPos, CEMapex *pMapex);
	CRect GetULInstanceRect(CEULMapexInstance *pInstance);
  CRect GetUnitInstanceRect(CEUnit *pInstance);
	void UpdateMiniMapSelection();
	void UpdateSelection();
	void DoneRectSelection();
	void SetRectSelection(CRect rcSelection);
	void SetSelectionPosition(DWORD dwX, DWORD dwY);
	void ClearSelection();
  // adds mapex into the selection
  // if the revert param. is TRUE : if the mapex insatnce is already
  // in the selection -> removes it elseway add it
  // if the revert param. is FALSE : add the mapex always (if exists do nothing)
  void AddSelectedMapexInstance(CEMapexInstance *pMapexInstance, BOOL bRevert);
  CMapexInstanceSelection *GetSelection(){ return &m_Selection; }
	void SetVisibleRect(CRect rcRect);
	void GetMapexInstanceRect(CEMapexInstance *pMapexInstance, CRect *pRect);
	CMiniMap * GetMiniMap();
	CEMapexInstance * GetMapexFromCell(DWORD dwX, DWORD dwY, CEMapexInstance *pPrevSel = NULL);
  BOOL IsBL1Visible(){ return m_Map.m_bBL1Visible; }
  BOOL IsBL2Visible(){ return m_Map.m_bBL2Visible; }
  BOOL IsBL3Visible(){ return m_Map.m_bBL3Visible; }
  BOOL IsULVisible(){ return m_Map.m_bULVisible; }
  int GetSelectedLevel(){ return m_Map.GetSelectedLevel(); }
	CEMapexInstance *AddMapexInstance(CPoint ptPos, CEMapex *pMapex, int nLevel);
  CEUnit *AddUnitInstance(CPoint ptPos, CEUnitType *pUnitType, CECivilization *pCivilization);

	CEMapex * GetSelectedMapex();
	void SetSelectedMapex(CEMapex *pMapex);

	void SetSelectedUnitType(CEUnitType *pUnitType, CECivilization *pCiv);
  CEUnitType * GetSelectedUnitType(){ return m_pSelectedUnitType; }
  CECivilization * GetSelectedCivilization(){ return m_pSelectedCivilization; }

	CEMap * GetMap();
	virtual ~CMapEditorDoc();

  enum ELevels{
    BackgroundLevel1 = 1,
    BackgroundLevel2 = 2,
    BackgroundLevel3 = 3,
    UnitLevel = 4
  };

  enum{
    Sel_None = 0,
    Sel_BLMapexInstance = 1,
    Sel_ULMapexInstance = 2,
    Sel_Unit = 3
  };

#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
  //{{AFX_MSG(CMapEditorDoc)
	afx_msg void OnMapexEditor();
	afx_msg void OnFileProperties();
	afx_msg void OnMapexNewLibrary();
	afx_msg void OnMapexOpenLibrary();
	afx_msg void OnMapexEditLibrary();
	afx_msg void OnMapexDeleteLibrary();
	afx_msg void OnFileSetLandTypes();
	afx_msg void OnViewBL1();
	afx_msg void OnViewBL2();
	afx_msg void OnViewBL3();
	afx_msg void OnViewUL();
	afx_msg void OnUpdateViewBL1(CCmdUI* pCmdUI);
	afx_msg void OnUpdateViewBL2(CCmdUI* pCmdUI);
	afx_msg void OnUpdateViewBL3(CCmdUI* pCmdUI);
	afx_msg void OnUpdateViewUL(CCmdUI* pCmdUI);
	afx_msg void OnSelectBL1();
	afx_msg void OnSelectBL2();
	afx_msg void OnSelectBL3();
	afx_msg void OnSelectUL();
	afx_msg void OnUpdateSelectBL1(CCmdUI* pCmdUI);
	afx_msg void OnUpdateSelectBL2(CCmdUI* pCmdUI);
	afx_msg void OnUpdateSelectBL3(CCmdUI* pCmdUI);
	afx_msg void OnUpdateSelectUL(CCmdUI* pCmdUI);
	afx_msg void OnMapexDelete();
	afx_msg void OnUpdateMapexDelete(CCmdUI* pCmdUI);
	afx_msg void OnMapGrid();
	afx_msg void OnUpdateMapGrid(CCmdUI* pCmdUI);
	afx_msg void OnUpdateGridSize1(CCmdUI* pCmdUI);
	afx_msg void OnUpdateGridSize2(CCmdUI* pCmdUI);
	afx_msg void OnUpdateGridSize4(CCmdUI* pCmdUI);
	afx_msg void OnUpdateGridSize8(CCmdUI* pCmdUI);
	afx_msg void OnUpdateGridSize16(CCmdUI* pCmdUI);
	afx_msg void OnGridSize1();
	afx_msg void OnGridSize2();
	afx_msg void OnGridSize4();
	afx_msg void OnGridSize8();
	afx_msg void OnGridSize16();
	afx_msg void OnMapLandTypes();
	afx_msg void OnUpdateMapLandTypes(CCmdUI* pCmdUI);
	afx_msg void OnMinimapZoomin();
	afx_msg void OnUpdateMinimapZoomin(CCmdUI* pCmdUI);
	afx_msg void OnMinimapZoomout();
	afx_msg void OnUpdateMinimapZoomout(CCmdUI* pCmdUI);
	afx_msg void OnUpdateEditCopy(CCmdUI* pCmdUI);
	afx_msg void OnEditCopy();
	afx_msg void OnUpdateEditCut(CCmdUI* pCmdUI);
	afx_msg void OnEditCut();
	afx_msg void OnUpdateEditPaste(CCmdUI* pCmdUI);
	afx_msg void OnEditPaste();
  afx_msg void OnUpdateMapexZPos(CCmdUI* pCmdUI);
	afx_msg void OnMapexZPos();
	afx_msg void OnUnitCivilizations();
	afx_msg void OnUnitDeleteLibrary();
	afx_msg void OnUnitEditLibrary();
	afx_msg void OnUnitNewLibrary();
	afx_msg void OnUnitOpenLibrary();
	afx_msg void OnUnitEditor();
	afx_msg void OnUnitDirectionLeft();
	afx_msg void OnUpdateUnitDirectionLeft(CCmdUI* pCmdUI);
	afx_msg void OnUnitDirectionRandom();
	afx_msg void OnUpdateUnitDirectionRandom(CCmdUI* pCmdUI);
	afx_msg void OnUnitDirectionRight();
	afx_msg void OnUpdateUnitDirectionRight(CCmdUI* pCmdUI);
	afx_msg void OnFileResources();
	afx_msg void OnUpdateMapexUp(CCmdUI* pCmdUI);
	afx_msg void OnMapexUp();
	afx_msg void OnUpdateMapexDown(CCmdUI* pCmdUI);
	afx_msg void OnMapexDown();
	afx_msg void OnFileInvisibility();
	afx_msg void OnFileScriptSet();
	afx_msg void OnFileFindPathGraphs();
	afx_msg void OnFileSetMapSize();
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()

private:
	int m_nMiniMapZoom;
	CMiniMap m_MiniMap;

  // selected mapex
  CEMapex *m_pSelectedMapex;

  // selected unit type
  CEUnitType *m_pSelectedUnitType;
  CECivilization *m_pSelectedCivilization;

	CEMap m_Map;
  CMapexInstanceSelection m_Selection;
  int m_nSelectionType;
  CEULMapexInstance *m_pSelectedULInstance;
  CEUnit * m_pSelectedUnit;

  CClipboard m_Clipboard;
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MAPEDITORDOC_H__43E086CD_6434_11D3_A054_ADE3A89A0638__INCLUDED_)
