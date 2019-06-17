// UpdateRectList.h: interface for the CUpdateRectList class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_UPDATERECTLIST_H__FCAFC5D5_61D5_11D3_A04E_AB891B48E831__INCLUDED_)
#define AFX_UPDATERECTLIST_H__FCAFC5D5_61D5_11D3_A04E_AB891B48E831__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CUpdateRectList : public CObject  
{
  DECLARE_DYNAMIC(CUpdateRectList)

public:
	void RemoveFirstNode();
	BOOL IsEmpty();
  struct tagSRectNode{
    CRect m_rcRect; // the rectangle
    int m_nSize;    // size of the rect
    int m_nCover; // size covered by this rect (sum of all rects in this)
    double m_dbCoverCoeficient; // nCover / nSize
    struct tagSRectNode *m_pNext;
  };
  typedef struct tagSRectNode SRectNode;

	SRectNode * GetFirstRectNode();
	void RemoveAll();
	void AddRect(CRect *pRect);
	virtual void Delete();
	BOOL Create(double dbMinCoverCoeficient, double dbMinCoverDegradation);
	CUpdateRectList();
	virtual ~CUpdateRectList();

#ifdef _DEBUG
  virtual void AssertValid() const;
  virtual void Dump(CDumpContext &dc) const;
#endif

private:
	void AddNode(SRectNode *pNode);
	SRectNode * m_pFirstNode;

  // minimal value of Cover/Size of the union which can be joined
  double m_dbMinCoverCoeficient; 
  // minimal value NewCover / OldCover which can be joined
  double m_dbMinCoverDegradation;

  // memory pool for this list
  CTypedMemoryPool<SRectNode> m_RectNodePool;
};

#endif // !defined(AFX_UPDATERECTLIST_H__FCAFC5D5_61D5_11D3_A04E_AB891B48E831__INCLUDED_)
