// EMapSquare.h: interface for the CEMapSquare class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_EMAPSQUARE_H__23152135_6E73_11D3_A06B_C85E707FE131__INCLUDED_)
#define AFX_EMAPSQUARE_H__23152135_6E73_11D3_A06B_C85E707FE131__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "EMapexInstance.h"
#include "EULMapexInstance.h"
#include "EUnit.h"

class CEMapSquare  
{
public:
	DWORD GetSaveSize();
	void AddUnit(CEUnit *pUnit);
  void DeleteUnit(CEUnit *pUnit);
	void DeleteULMapexInstance(CEULMapexInstance *pMapexInstance);
	void AddULMapexInstance(CEULMapexInstance *pMapexInstance);
	DWORD GetBLMapexLevel(CEMapexInstance *pMapexInstance);
  // removes the mapex instance from lists 
  // IT DOESN'T DELETE THE OBJECT
  void DeleteBLMapexInstance(CEMapexInstance *pMapexInstance);
  // adds new mapex instance to the level (dwLevel) in this square
	void AddBLMapexInstance(DWORD dwLevel, CEMapexInstance *pMapexInstance);
  // appends BL mapexes from this square on the level (dwLevel) to the list of BL mapexes
	void AppendBLMapexes(DWORD dwLevel, CTypedPtrList<CPtrList, CEMapexInstance *> *pList);
  // appends UL mapexes from this square on the unit level to the list of UL mapexes
  // it ensorts the new ones to the list (descending z pos)
  void AppendULMapexes(CTypedPtrList<CPtrList, CEULMapexInstance *> *pList);
  // appends units from this squareto the list of units
  // it ensorts the new ones to the list (descending z pos)
  void AppendUnits(CTypedPtrList<CPtrList, CEUnit *> *pList);

  // saves the square to the file
	void SaveToFile(CArchiveFile file);

  // deletes the square
	void Delete();
  // creates the square from file
	BOOL Create(CArchiveFile file);
  // creates the new square
	BOOL Create();

  // constructor & destructor
	CEMapSquare();
	~CEMapSquare();

private:
  // if the given mapex instance is in the (dwLevel) list, than returns previous mapex instance in the list
  // if the pMapexInstance is the first -> NULL
  // if it isn't in the level's list returns 0xFFFFFFFF
  CEMapexInstance * IsBLMapexInstanceInLevel(DWORD dwLevel, CEMapexInstance *pMapexInstance);
  // return first BL mapex in the list for givven level (1 - 3)
	CEMapexInstance * GetBLFirst(DWORD dwLevel);
  // loads BL mapexes from file starting with given list head
	CEMapexInstance * LoadBLMapexes(CArchiveFile file, DWORD dwCount);
  // saves BL mapexes to file starting with given list head
	void SaveBLMapexes(CEMapexInstance *pFirst, CArchiveFile file);
  // sorts BL mapexes in order thay will be drawen on client
  void SortBLMapexes(CEMapexInstance **ppFirst);
  // counts BL mapexes starting with given list head
	DWORD CountBLMapexes(CEMapexInstance *pFirst);
  // deletes BL mapexes starting with givven list head
	void DeleteBLMapexes(CEMapexInstance *pHead);

  // BL - Background Level
  CEMapexInstance *m_pBL1Mapexes; // level 1 mapexes list
  CEMapexInstance *m_pBL2Mapexes; // level 2 mapexes list
  CEMapexInstance *m_pBL3Mapexes; // level 3 mapexes list
  // UL - Unit Level (level 4)
  CEULMapexInstance *m_pULMapexes; // unit level mapexes
  // Units (level 4)
  CEUnit *m_pUnits;

  // lock for accessing this square
  BOOL m_bLock;

  friend class CEMap;
};

typedef CEMapSquare *LPMapSquare;

#endif // !defined(AFX_EMAPSQUARE_H__23152135_6E73_11D3_A06B_C85E707FE131__INCLUDED_)
