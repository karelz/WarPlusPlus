// EFindPathGraph.h: interface for the CEFindPathGraph class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_EFINDPATHGRAPH_H__34B13454_80F0_11D4_B0FA_004F49068BD6__INCLUDED_)
#define AFX_EFINDPATHGRAPH_H__34B13454_80F0_11D4_B0FA_004F49068BD6__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "..\AbstractDataClasses\256BitArray.h"

class CEFindPathGraph : public CObject  
{
  DECLARE_DYNAMIC(CEFindPathGraph);
public:
	CEFindPathGraph();
	virtual ~CEFindPathGraph();

#ifdef _DEBUG
  virtual void AssertValid() const;
  virtual void Dump(CDumpContext &dc) const;
#endif

  // creates new graph
  void Create(DWORD dwName, CString strName);
  // loads the graph from file
  void Create(CArchiveFile file);
  // deletes the graph
  virtual void Delete();

  // save the graph to the file
  void Save(CArchiveFile file);

  // Checks if the graph is valid
  virtual BOOL CheckValid();

  // returns the name of the graph
  CString GetName(){ return m_strName; }
  // sets new name of the graph
  void SetName(CString strName){ m_strName = strName; }

  // Returns name
  DWORD GetID(){ return m_dwID; }

  // returns if the land type with given index is allowed
  BOOL GetLandType(DWORD dwIndex){ ASSERT(dwIndex < 256); return m_LandTypes.GetAt((int)dwIndex); }
  // sets allowed state for given landtype (by index)
  void SetLandType(DWORD dwIndex, BOOL bValue){ ASSERT(dwIndex < 256); m_LandTypes.SetAt((int)dwIndex, bValue); }

  void Copy(C256BitArray *pDest){ memcpy(pDest->GetData(), m_LandTypes.GetData(), sizeof(DWORD) * 8); }

private:
  // Name of the graph
  CString m_strName;
  // Allowed land types for this graph
  C256BitArray m_LandTypes;

  // ID
  DWORD m_dwID;
};

#endif // !defined(AFX_EFINDPATHGRAPH_H__34B13454_80F0_11D4_B0FA_004F49068BD6__INCLUDED_)
