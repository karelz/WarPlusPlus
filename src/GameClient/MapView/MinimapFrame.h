// MinimapFrame.h: interface for the CMinimapFrame class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MINIMAPFRAME_H__AF15FB81_4F3A_11D4_B521_00105ACA8325__INCLUDED_)
#define AFX_MINIMAPFRAME_H__AF15FB81_4F3A_11D4_B521_00105ACA8325__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "..\\DataObjects\\CMap.h"
#include "MinimapWindow.h"

class CMinimapFrame : public CFrameWindow  
{
  DECLARE_DYNAMIC(CMinimapFrame);
  DECLARE_OBSERVER_MAP(CMinimapFrame);

public:
  // Constructor & destructor
	CMinimapFrame();
	virtual ~CMinimapFrame();

  // Debug functions
#ifdef _DEBUG
  virtual void AssertValid() const;
  virtual void Dump(CDumpContext &dc) const;
#endif

// Creation
  // Creates the frame window and the minimap in it
  // You must give it the map.. :-)
  void Create(CRect &rcRect, CWindow *pParent, CCMap *pMap, CDataArchive *pGraphicsArchive,
    CViewport *pViewport, CCMiniMapClip *pMiniMapClip);
  // Deletes the window
  virtual void Delete();

  enum{
    E_CLOSEWINDOW = 1, // Event sent when the user clicks the close button on this window
  };

protected:
  // The Min/Max sizes of the window
  virtual CSize GetMaximumSize();
  virtual CSize GetMinimumSize();
  // Reaction on the size change
  virtual void OnSize(CSize size);

  enum{
    IDC_CLOSE = 1,
    IDC_PLUS = 2,
    IDC_MINUS = 3,
  };

  void OnClose();
  void OnPlus();
  void OnMinus();


private:
  // The minimap view window
  CMinimapWindow m_wndMinimap;
  // The window layout
  CFrameWindowLayout m_Layout;

  // The close button
  CImageButton m_wndClose;
  CImageButtonLayout m_CloseLayout;
  // The Zoom Plus button
  CImageButton m_wndPlus;
  CImageButtonLayout m_PlusLayout;
  // The Zoom Minus button
  CImageButton m_wndMinus;
  CImageButtonLayout m_MinusLayout;
};

#endif // !defined(AFX_MINIMAPFRAME_H__AF15FB81_4F3A_11D4_B521_00105ACA8325__INCLUDED_)
