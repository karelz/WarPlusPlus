// MapViewerDoc.cpp : implementation of the CMapViewerDoc class
//

#include "stdafx.h"
#include "MapViewer.h"

#include "MapViewerDoc.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CMapViewerDoc

IMPLEMENT_DYNCREATE(CMapViewerDoc, CDocument)

BEGIN_MESSAGE_MAP(CMapViewerDoc, CDocument)
	//{{AFX_MSG_MAP(CMapViewerDoc)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMapViewerDoc construction/destruction

CMapViewerDoc::CMapViewerDoc () 
{
	// TODO: add one-time construction code here
	m_nTimerID = 0;
}

CMapViewerDoc::~CMapViewerDoc()
{
	AfxGetMainWnd ()->KillTimer ( m_nTimerID );
}

BOOL CMapViewerDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;

	// TODO: add reinitialization code here
	// (SDI documents will reuse this document)
	m_bFreezed = FALSE;

	// popis mapy
	m_pMapDescription = theApp.m_pMapDescription;
	// mapa
	m_pMap = theApp.m_pMap;

	VERIFY ( theApp.m_mutexSharedMemoryLock.Lock () );
	m_sMapDescription = *m_pMapDescription;
	VERIFY ( theApp.m_mutexSharedMemoryLock.Unlock () );

	m_sMapDescription.m_nMapBorder = 10;
	m_sMapDescription.m_nSizeX = 1024;
	m_sMapDescription.m_nSizeY = 1024;
	m_sMapDescription.m_nMapSizeX = m_sMapDescription.m_nSizeX + 2 * 
		m_sMapDescription.m_nMapBorder;
	m_sMapDescription.m_nMapSizeY = m_sMapDescription.m_nSizeY + 2 * 
		m_sMapDescription.m_nMapBorder;

	m_nTimerID = AfxGetMainWnd ()->SetTimer ( (UINT)this, 500, NULL );

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// CMapViewerDoc serialization

void CMapViewerDoc::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{
		// TODO: add storing code here
	}
	else
	{
		// TODO: add loading code here
	}
}

/////////////////////////////////////////////////////////////////////////////
// CMapViewerDoc diagnostics

#ifdef _DEBUG
void CMapViewerDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void CMapViewerDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CMapViewerDoc commands
