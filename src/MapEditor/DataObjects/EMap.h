// EMap.h: interface for the CEMap class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_EMAP_H__6868A9A7_6755_11D3_A059_B7FF8C0EE331__INCLUDED_)
#define AFX_EMAP_H__6868A9A7_6755_11D3_A059_B7FF8C0EE331__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "EMapexLibrary.h"
#include "EMapex.h"
#include "LandType.h"
#include "ECivilization.h"

#include "EUnitType.h"
#include "EUnitLibrary.h"

#include "EResource.h"
#include "EInvisibility.h"
#include "EScriptSet.h"

#include "EFindPathGraph.h"

#include "EMapSquare.h"
#include "..\MiniMap.h"

#include "..\MapexInstanceSelection.h"

#include "..\MapFormats.h"

#include "..\UnitEditor\UnitSkillTypeCollection.h"

extern CUnitSkillTypeCollection *g_pUnitSkillTypeCollection;

class CEMap : public CObject
{
  DECLARE_DYNAMIC(CEMap);

public:
  // draws view
  // dwXPos, dwYPos -> top left in mapcells
  // rcDest destination rectangle in the surface (in pixels)
  // pSurface destination surface
	void DrawView(DWORD dwXPos, DWORD dwYPos, CRect &rcDest, CDDrawSurface *pSurface);

  // returns TRUE if the map is valid
  BOOL CheckValid();

  // returns mapex from its ID
	CEMapex * GetMapex(DWORD dwID);

  // saves all libraries in the map
	void SaveLibraries();

  // Changes the size of the map
  // the given sizes is in mapcells and must be a multiple of map square
  // size
  void SetSize ( DWORD dwWidth, DWORD dwHeight );

  // fills list control with all land types
  // each item has as its data address of the land type object (CLandType)
	void FillLandTypesListCtrl(CListCtrl *pListCtrl);
  // returns land type object from its ID
	CLandType * GetLandType(BYTE nID);
  // loads land types from givven file
	void LoadLandTypes(CArchiveFile file);

  // returns new (unique) ID for use with mapexes
	DWORD GetNewMapexID();

  // Returns the resource for given index
  CEResource *GetResource(DWORD dwIndex){ ASSERT(dwIndex < RESOURCE_COUNT); return m_aResources[dwIndex]; }

  // Returns the invisibility for given index
  CEInvisibility *GetInvisibility(DWORD dwIndex){ ASSERT(dwIndex < INVISIBILITY_COUNT); return m_aInvisibilities[dwIndex]; }

  POSITION GetFirstUnitPosition(){ return m_listUnits.GetHeadPosition(); }
  CEUnit *GetNextUnit(POSITION &rPos){ return m_listUnits.GetNext(rPos); }
  
  // Returns script set by its ID
  CEScriptSet *GetScriptSet(DWORD dwID);

  // Removes the script set (also deletes the object)
  void DeleteScriptSet(CEScriptSet *pScriptSet);
  // Adds the script set
  void AddScriptSet(CEScriptSet *pScriptSet);

  void FillScriptSetComboBox(CComboBox *pComboBox);
  void FillScriptSetListCtrl(CListCtrl *pListCtrl);

  // Removes the find path graph (also deletes the pointer)
  void DeleteFindPathGraph(CEFindPathGraph *pGraph);
  // Adds new find path graph
  void AddFindPathGraph(CEFindPathGraph *pGraph);

  void FillFindPathGraphListCtrl(CListCtrl *pListCtrl);

  // returns TRUE if the civilization is unique to the others
  BOOL IsCivUnique(CECivilization *pCiv);

  // fills givven combo box with mapex libraries in the map
  // each item has as its data address of the mapex library object (CEMapexLibrary)
	void FillMapexLibrariesComboBox(CComboBox *pComboBox);

  // opens mapex library from givven file
	void OpenMapexLibrary(CString strFileName);
  // deletes mapex library
	void DeleteMapexLibrary(CEMapexLibrary *pMapexLibrary);

  // these just removes mapexes from map, not from library
  // removes mapex from the map (used by libraries when deleted)
	void DeleteMapex(CEMapex *pMapex);
  // adds mapex to the map (used by libraries when loading)
	void AddMapex(CEMapex *pMapex);

  // iterates through mapex libraries (start with GetFirstMapexLibraryPosition)
	CEMapexLibrary * GetNextMapexLibrary(POSITION &pos);
  // returns position of the first mapex library
  POSITION GetFirstMapexLibraryPosition();

  // adds new mapex library to the map (creates it)
  // and returns pointer to it
	CEMapexLibrary *NewMapexLibrary(CString strFileName);

  // sets description of the map
	void SetDescription(CString strDescription);
  // returns description of the map
	CString GetDescription();

  // sets name of the map
	void SetName(CString strName);
  // returns name of the map
	CString GetName();

  // get map square (square coords)
  CEMapSquare *GetMapSquare(DWORD dwX, DWORD dwY){
    ASSERT(dwX < m_dwSquareWidth); ASSERT(dwY < m_dwSquareHeight);
    ASSERT_VALID(this); return m_pMapSquares[dwX + dwY * m_dwSquareWidth];
  }
  // get map square (cell coords)
  CEMapSquare *GetMapSquareFromCell(DWORD dwX, DWORD dwY){
    return GetMapSquare(dwX / MAPSQUARE_WIDTH, dwY / MAPSQUARE_HEIGHT);
  }

  // get sizes
  DWORD GetWidth(){ return m_dwWidth; }
  DWORD GetHeight(){ return m_dwHeight; }

  DWORD GetSquareWidth(){ return m_dwSquareWidth; }
  DWORD GetSquareHeight(){ return m_dwSquareHeight; }

  // saves map to the file
	void SaveMap(CArchiveFile MapFile);

  // returns TRUE if the object is deleted
	BOOL IsDeleted();

  // creates new (empty) map
	BOOL Create(DWORD dwWidth, DWORD dwHeight);
  // deletes the map
	void Delete(CProgressCtrl *pProgress = NULL);
  // creates the map from file (loads it)
  BOOL Create(CArchiveFile MapFile, CProgressCtrl *pProgress = NULL);

  // destructor
	CEMap();
  // constructor
	virtual ~CEMap();

#ifdef _DEBUG
  virtual void AssertValid() const;
  virtual void Dump(CDumpContext &dc) const;
#endif

public:
	void RebuildLibraries();
  void DeleteUnit(CEUnit *pUnit);
  void InsertUnit(CEUnit *pUnit);
	CECivilization * GetCivilization(DWORD dwID);
	DWORD GetNewUnitTypeID();
	void SaveUnitLibraries();

	void FillCivilizationsComboBox(CComboBox *pComboBox);

  void FillUnitLibrariesComboBox(CComboBox *pComboBox);

  void FillUnitTypesComboBox(CComboBox *pComboBox);
	void OpenUnitLibrary(CString strFileName);
  // delets the unit type library
	void DeleteUnitLibrary(CEUnitLibrary *pUnitLibrary);
  // gets next unit type libary
	CEUnitLibrary * GetNextUnitLibrary(POSITION &pos);
  // returns position of the first unit type library
	POSITION GetFirstUnitLibraryPosition();
  // creates new unit library
	CEUnitLibrary * NewUnitLibrary(CString strFileName);

  // returns unit type from the ID
	CEUnitType * GetUnitType(DWORD dwID);
  // removes unit type from the map of unit types (map for IDs)
  // doesn't delete the unit type -> delte it in the library
	void DeleteUnitType(CEUnitType *pUnitType);
  // adds new unit type to the map of all unit types (map for IDs)
  // this doesn't allocate any unit type or so... !!
	void AddUnitType(CEUnitType *pUnitType);
  // removes civilization from the map
	void DeleteCivilization(CECivilization *pCivilization);
  // adds new civilization to the map
	void AddCivilization(CECivilization *pCivilization);
  // returns new Civilization ID
	DWORD GetNewCivilizationID();
  // fills the list control with all civilizations in the map
	void FillCivilizationsListCtrl(CListCtrl *pListCtrl);

  // returns mapex from the UL for the given cell
	CEULMapexInstance * GetULMapexFromCell(DWORD dwX, DWORD dwY);
  // returns unit for given cell
  CEUnit * GetUnitFromCell(DWORD dwX, DWORD dwY);

  // sets new unit level selection
	void SetULSelection(CEULMapexInstance *pSel);
  // sets new unit instance selection
  void SetUnitSelection(CEUnit *pUnit);

  // sets selection rectangle (in mapcells)
  // return TRUE if it differs from the old one
	BOOL SetSelectionRectangle(CRect *pSelection);
  // gets the selection rectangle (in mapcells)
  CRect GetSelectionRectangle(){ return m_rcSelection; }
  // returns the zoom factor for the minimap
  DWORD GetMiniMapZoom(){ return m_pMiniMap->GetZoom(); }
  // sets the visible rectangle in the view
  // (in mapcells)
  void SetVisibleRect(CRect rcRect);
  // gets the visible rectangle in the view
  // (in mapcells)
  CRect GetVisibleRect(){ return m_pMiniMap->GetVisibleRect(); }

  // sets the minimap object for the map
	void SetMiniMap(CMiniMap *pMiniMap);

  // sets the rectangle which is visible in the minimap
  // (in mapcells coords)
	void SetMiniMapRect(CRect rcRect);
  // returns the rectangle which is visible in the minimap
  // (in mapcells coords)
	CRect GetMiniMapRect();

  // releases the minimap
	void ReleaseMiniMap();
  // returns pointer to the minimap surface
  // it locks the minimap -> relase it with ReleaseMiniMap
	CDDrawSurface * GetMiniMap();
  CDDrawSurface * GetMiniMapBuffer(){ return &m_MiniMapBuffer; }

  void LockMiniMapMove(){ m_semaphoreMiniMapMove.Lock(); }
  void UnlockMiniMapMove(){ m_semaphoreMiniMapMove.Unlock(); }

  // releases whole map
	void ReleaseExclusiveAccess();
  // locks whole map
	void GetExclusiveAccess();

  // releases one square on the map
	void ReleaseSquare(CEMapSquare *pSquare);
  // lock one square on the map
	void LockSquare(CEMapSquare *pSquare);

  // draw land types or not
  void SetDrawLandTypes(BOOL bDraw){ m_bDrawLandTypes = bDraw; }
  BOOL GetDrawLandTypes(){ return m_bDrawLandTypes; }

  // selected level
  void SetSelectedLevel(DWORD dwLevel){ m_dwSelectedLevel = dwLevel; }
  DWORD GetSelectedLevel(){ return m_dwSelectedLevel; }

  // grid size (in mapcells)
	void SetGridSize(DWORD dwGridSize);
	DWORD GetGridSize();

  // draw grid flag
	void SetDrawGrid(BOOL bDraw);
	BOOL GetDrawGrid();

  // modified flag
	void SetModified(BOOL bModified);
	BOOL IsModified();

  // returns mapex which touches given cell and which is at the top (greates level)
  // takes care about land types -> transparent -> next level
  // takes care about visibility (skips invisible levels)
	CEMapexInstance * GetMapexFromCell(DWORD dwX, DWORD dwY, CEMapexInstance *pPrevSel = NULL);
  // sets current selected mapex instance (the blue one)
	void SetSelection(CMapexInstanceSelection *pSelection);
  // returns current selected mapex instance (the blue one)
	CMapexInstanceSelection * GetSelection();

  // visibility
  BOOL m_bBL1Visible;
  BOOL m_bBL2Visible;
  BOOL m_bBL3Visible;
  BOOL m_bULVisible;

private:
	CMiniMap * m_pMiniMap;
  // deletes the minimap surface
	void DeleteMiniMap();
  // craetes the minimap surface
	void CreateMiniMap();

	// if TRUE -> the map is modified
  BOOL m_bModified;
  // if TRUE -> the map is deleted
	BOOL m_bDeleted;

  // creates some special buffers (surfaces)
	void CreateSpecialBuffers();
  // deletes special buffers
	void DeleteSpecialBuffers();

  // deletes all land types objects
	void DeleteLandTypes();

  // mapex libraries used in map
  CTypedPtrList<CPtrList, CEMapexLibrary *> m_listLibraries;

  // unit libraries used in map
  CTypedPtrList<CPtrList, CEUnitLibrary *> m_listUnitLibraries;

  // size of the map (in mapcells)
  DWORD m_dwWidth, m_dwHeight;
  // size of the map (in mapsquares)
  DWORD m_dwSquareWidth, m_dwSquareHeight;

  // two dimensional array of pointers to map squares
  // size is : size of the map diveded by size of one MapSquare
  CEMapSquare **m_pMapSquares;

  // name of the map
  CString m_strName;
  // description
  CString m_strDescription;

  // map of mapexes
  CMap<DWORD, DWORD, CEMapex *, CEMapex *> m_mapMapexes;
  // map of unit types
  CMap<DWORD, DWORD, CEUnitType *, CEUnitType *> m_mapUnitTypes;

  // list of units
  CTypedPtrList<CPtrList, CEUnit *> m_listUnits;

  // land types
  CTypedPtrList<CPtrList, CLandType *> m_listLandTypes;

  // civilizations
  CTypedPtrList<CPtrList, CECivilization *> m_listCivilizations;


  // Resources
  CEResource *m_aResources[RESOURCE_COUNT];

  // Invisibilities
  CEInvisibility *m_aInvisibilities[INVISIBILITY_COUNT];

  // path to the land types file
  CString m_strLandTypesFile;

  // scratch surfaces for drawing selected mapexes
  CScratchSurface m_MapexBuffer;
  CScratchSurface m_SelectionLayer;
  CScratchSurface m_HorizontalGridLine;
  CScratchSurface m_VerticalGridLine;
  CScratchSurface m_LandTypeBuffer;
  CScratchSurface m_HorizontalSelectionLine;
  CScratchSurface m_VerticalSelectionLine;

  // selection rectangle
  CRect m_rcSelection;

  // minimap surface
  CScratchSurface m_MiniMap;
  CScratchSurface m_MiniMapBuffer;
  // top left position of the map
  CRect m_rcMiniMap;
  // semaphore for access to minimap surface
  CSemaphore m_semaphoreMiniMap;
  CSemaphore m_semaphoreMiniMapMove;

  // the grid parameters
	DWORD m_dwGridSize; // draw the grid
	BOOL m_bDrawGrid;   // size of the grid in mapcells

  // draw land types or graphics
  BOOL m_bDrawLandTypes;

  // selected level number
  DWORD m_dwSelectedLevel;

  // the selection of mapex instances
  CMapexInstanceSelection *m_pSelection;
  CEULMapexInstance *m_pULSelection;
  CEUnit *m_pUnitSelection;

  // semaphore for acces to square locks
  CSemaphore m_semaphoreSquareLock;
  // event which it set when some square lock is released
  CEvent m_eventSquareLockReleased;
  // semaphore for exclusive acces to the map
  CSemaphore m_semaphoreExlusive;

  // Unit skill type collection
  CUnitSkillTypeCollection m_UnitSkillTypeCollection;
 
  // Script sets
  CTypedPtrList<CPtrList, CEScriptSet *> m_listScriptSet;

  // FindPathGraphs
  CTypedPtrList<CPtrList, CEFindPathGraph *> m_listFindPathGraphs;
  // The Fly FindPathGraph
  CEFindPathGraph m_cFlyFindPathGraph;

  friend class CMainFrame;
};

#endif // !defined(AFX_EMAP_H__6868A9A7_6755_11D3_A059_B7FF8C0EE331__INCLUDED_)
