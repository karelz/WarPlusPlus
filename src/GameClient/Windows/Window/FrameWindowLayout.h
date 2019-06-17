// FrameWindowLayout.h: interface for the CFrameWindowLayout class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_FRAMEWINDOWLAYOUT_H__14523515_E207_11D2_AC29_8B1CBE5F5660__INCLUDED_)
#define AFX_FRAMEWINDOWLAYOUT_H__14523515_E207_11D2_AC29_8B1CBE5F5660__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "..\Animation\Animation.h"
#include "..\Mouse\Mouse.h"

class CFrameWindow;

class CFrameWindowLayout : public CObject  
{
  DECLARE_DYNAMIC(CFrameWindowLayout);

public:
#ifdef _DEBUG
  virtual void AssertValid() const;
  virtual void Dump(CDumpContext &dc) const;
#endif

public:
  // returns TRUE if some of animations has transparent pixel
  // or is not present (NULL)
	virtual BOOL AreAnimsTransparent();
  // deletes the object
	virtual void Delete();
  // creates empty object
  virtual BOOL Create();
  // creates the object from the given config file
  virtual BOOL Create(CConfigFile *pCfgFile);
  // creates the object from the given config file
	virtual BOOL Create(CArchiveFile CfgFile);
  // constructor
	CFrameWindowLayout();
  // destructor
	virtual ~CFrameWindowLayout();

public:
  CAnimation *GetPBottomRight(){return m_pPBottomRight;}
  CAnimation *GetPBottom(){return m_pPBottom;}
  CAnimation *GetPBottomLeft(){return m_pPBottomLeft;}
  CAnimation *GetPMiddle(){return m_pPMiddle;}
  CAnimation *GetPRight(){return m_pPRight;}
  CAnimation *GetPLeft(){return m_pPLeft;}
  CAnimation *GetPTopRight(){return m_pPTopRight;}
  CAnimation *GetPTop(){return m_pPTop;}
  CAnimation *GetPTopLeft(){return m_pPTopLeft;}

  CAnimation *GetABottomRight(){
    return m_pABottomRight ? m_pABottomRight : m_pPBottomRight;}
  CAnimation *GetABottom(){
    return m_pABottom ? m_pABottom : m_pPBottom;}
  CAnimation *GetABottomLeft(){
    return m_pABottomLeft ? m_pABottomLeft : m_pPBottomLeft;}
  CAnimation *GetAMiddle(){
    return m_pAMiddle ? m_pAMiddle : m_pPMiddle;}
  CAnimation *GetARight(){
    return m_pARight ? m_pARight : m_pPRight;}
  CAnimation *GetALeft(){
    return m_pALeft ? m_pALeft : m_pPLeft;}
  CAnimation *GetATopRight(){
    return m_pATopRight ? m_pATopRight : m_pPTopRight;}
  CAnimation *GetATop(){
    return m_pATop ? m_pATop : m_pPTop;}
  CAnimation *GetATopLeft(){
    return m_pATopLeft ? m_pATopLeft : m_pPTopLeft;}

protected:
	DWORD m_dwActiveMiddleColor;
	DWORD m_dwPassiveMiddleColor;
	CSize m_sizeMin;
  CSize m_sizeMax;
  // animations for not activated state
	CAnimation * m_pPBottomRight;
	CAnimation * m_pPBottom;
	CAnimation * m_pPBottomLeft;
	CAnimation * m_pPMiddle;
	CAnimation * m_pPRight;
	CAnimation * m_pPLeft;
	CAnimation * m_pPTopRight;
	CAnimation * m_pPTop;
  CAnimation * m_pPTopLeft;

	// animations for activated state
  CAnimation * m_pABottomRight;
	CAnimation * m_pABottom;
	CAnimation * m_pABottomLeft;
	CAnimation * m_pAMiddle;
	CAnimation * m_pARight;
	CAnimation * m_pALeft;
	CAnimation * m_pATopRight;
	CAnimation * m_pATop;
  CAnimation * m_pATopLeft;

  // cursors for the window
	CCursor *m_pNormalCursor; // normal cursor
  CCursor *m_pMoveCursor; // cursor for moving areas
  CCursor *m_pLRCursor;   // LeftRight cursor sizing
  CCursor *m_pTBCursor;   // TopBottom cursor sizing
  CCursor *m_pTLBRCursor; // TopLeft-BottomRight cursor sizing
  CCursor *m_pTRBLCursor; // TopRight-BottomLeft cursor sizing

  // margins from the bounds of the window
  // they defines the area where the window will react
  // to the mouse for the drag action
  // if the left or top is -1 no draging will be able
  // if the right or bottom is <0 then they will be the
  // size of the rectangle which begins at left and top
  int m_nDragLeft, m_nDragRight, m_nDragTop, m_nDragBottom;

  // edge size for each edge of the window
  // will be used to determine area where sizing will occure
  int m_nEdgeLeft, m_nEdgeRight, m_nEdgeTop, m_nEdgeBottom;

  // distance from corner which will be taken as the corner
  int m_nCornerSize;

  // sound to be played when the window is activated
  CSoundBuffer *m_pActivateSound;
  // sound to be played when the window is deactivated
  CSoundBuffer *m_pDeactivateSound;

  friend CFrameWindow;
  friend class CPopupMenu;
};

#endif // !defined(AFX_FRAMEWINDOWLAYOUT_H__14523515_E207_11D2_AC29_8B1CBE5F5660__INCLUDED_)
