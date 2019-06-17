// ImageButtonLayout.h: interface for the CImageButtonLayout class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_IMAGEBUTTONLAYOUT_H__CA876BB3_A8EB_11D3_99BA_D7A76AF4F871__INCLUDED_)
#define AFX_IMAGEBUTTONLAYOUT_H__CA876BB3_A8EB_11D3_99BA_D7A76AF4F871__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "..\..\Animation\Animation.h"
#include "..\..\Mouse\Cursor.h"

class CImageButtonLayout : public CObject  
{
  DECLARE_DYNAMIC(CImageButtonLayout)

public:
	virtual BOOL AreAnimsTransparent();
	BOOL Create(CArchiveFile CfgFile);
	virtual void Delete();
	BOOL Create(CConfigFile *pCfgFile);
	CImageButtonLayout();
	virtual ~CImageButtonLayout();

#ifdef _DEBUG
  virtual void AssertValid() const;
  virtual void Dump(CDumpContext &dc) const;
#endif

protected:
  // 3 animations for 2 states
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

private:

  friend class CImageButton;
};

#endif // !defined(AFX_IMAGEBUTTONLAYOUT_H__CA876BB3_A8EB_11D3_99BA_D7A76AF4F871__INCLUDED_)
