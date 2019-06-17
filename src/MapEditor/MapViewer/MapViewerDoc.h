// MapViewerDoc.h : interface of the CMapViewerDoc class
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_MAPVIEWERDOC_H__018E926F_7289_11D4_AB67_004F4906CFC9__INCLUDED_)
#define AFX_MAPVIEWERDOC_H__018E926F_7289_11D4_AB67_004F4906CFC9__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "MapEditor\FindPathGraphCreation\MapDescription.h"

class CMapViewerDoc : public CDocument
{
protected: // create from serialization only
	CMapViewerDoc();
	DECLARE_DYNCREATE(CMapViewerDoc)

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMapViewerDoc)
	public:
	virtual BOOL OnNewDocument();
	virtual void Serialize(CArchive& ar);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CMapViewerDoc();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

public:

	// pøíznak, je-li mapa zmrazena
	BOOL m_bFreezed;
	// popis zmazené mapy
	struct SMapDescription m_sMapDescription;

	// popis mapy
	struct SMapDescription *m_pMapDescription;
	// mapa
	signed char *m_pMap;

protected:
	UINT m_nTimerID;

// Generated message map functions
protected:
	//{{AFX_MSG(CMapViewerDoc)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MAPVIEWERDOC_H__018E926F_7289_11D4_AB67_004F4906CFC9__INCLUDED_)
