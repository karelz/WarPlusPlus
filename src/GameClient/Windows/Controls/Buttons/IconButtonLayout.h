// IconButtonLayout.h: interface for the CIconButtonLayout class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_ICONBUTTONLAYOUT_H__176028F5_AB37_11D3_99C2_99ADDD254479__INCLUDED_)
#define AFX_ICONBUTTONLAYOUT_H__176028F5_AB37_11D3_99C2_99ADDD254479__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "..\..\Animation\Animation.h"
#include "..\..\Mouse\Cursor.h"

class CIconButtonLayout : public CObject  
{
  DECLARE_DYNAMIC(CIconButtonLayout);

public:
	virtual BOOL AreAnimsTransparent();
	BOOL Create(CConfigFile *pCfgFile);
	virtual void Delete();
	BOOL Create(CArchiveFile CfgFile);
	CIconButtonLayout();
	virtual ~CIconButtonLayout();

#ifdef _DEBUG
  virtual void AssertValid() const;
  virtual void Dump(CDumpContext &dc) const;
#endif

protected:
  // animations
  CAnimation *m_pPassive;
  CAnimation *m_pActive;
  CAnimation *m_pPressed;

  // cursor
  CCursor *m_pCursor;

  // sounds
  // sound played if the button is actvated by the mouse
  CSoundBuffer *m_pHoverSound;
  // sound played if the button is deactivated by the mouse
  CSoundBuffer *m_pLeaveSound;
  // sound played when the button is pressed
  CSoundBuffer *m_pPressSound;
  // sound played when the button is released
  CSoundBuffer *m_pReleaseSound;

  // icon offsets
  CPoint m_ptPassiveOffset;
  CPoint m_ptActiveOffset;
  CPoint m_ptPressedOffset;

  BOOL m_bCenter;
private:

  friend class CIconButton;
};

#endif // !defined(AFX_ICONBUTTONLAYOUT_H__176028F5_AB37_11D3_99C2_99ADDD254479__INCLUDED_)
