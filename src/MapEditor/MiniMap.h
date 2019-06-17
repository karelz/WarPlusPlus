// MiniMap.h: interface for the CMiniMap class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MINIMAP_H__AF43BF36_7809_11D3_A859_00105ACA8325__INCLUDED_)
#define AFX_MINIMAP_H__AF43BF36_7809_11D3_A859_00105ACA8325__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CEMap;

class CMiniMap : public CObject  
{
public:
	void SetVisibleRect(CRect rcRect);
  CRect GetVisibleRect(){ return m_rcVisibleRect; }
  void SetDrawWnd(CWnd *pWnd);
	void UpdateRect(CRect rcPiece);
	void SetMap(CEMap *pMap);
	void SetZoom(DWORD dwZoom);
	DWORD GetZoom();
	void Delete();
	BOOL Create();
	static UINT MiniMapThreadProc(LPVOID pParam);
	CMiniMap();
	virtual ~CMiniMap();

private:
	CWnd * m_pDrawWnd;
  CUpdateRectList m_UpdateList;
  CSemaphore m_semaphoreUpdateList;
  CSemaphore m_semaphoreRectInlayed;

  // rect which is visible in the view (full size view)
  CRect m_rcVisibleRect;

  BYTE *m_pLandTypeBuffer;
	CScratchSurface m_MiniMapBuffer;
  // computes one piece of minimap
  // the rcPiece has to have the width and height some multiplier of
  // the m_dwZoom
  void ComputePiece(CRect rcPiece);

  // actual used zoom factor -> this one is used in computations
  // can be changed only when the update list lock is down
  DWORD m_dwZoom;
  // wanted zoom factor can be changed any time
  // it's copied to the m_dwZoom every time the new rect is read from the update list
  DWORD m_dwWantedZoom;
	CWinThread * m_pMiniMapThread;
	CEMap * m_pMap;

  BOOL m_bExit;
};

#endif // !defined(AFX_MINIMAP_H__AF43BF36_7809_11D3_A859_00105ACA8325__INCLUDED_)
