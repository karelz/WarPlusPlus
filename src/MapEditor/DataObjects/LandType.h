// LandType.h: interface for the CLandType class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_LANDTYPE_H__39607C56_6A80_11D3_8C66_00105ACA8325__INCLUDED_)
#define AFX_LANDTYPE_H__39607C56_6A80_11D3_8C66_00105ACA8325__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CLandType  
{
public:
  // returns name
	CString GetName();
  // returns land type ID
	BYTE GetID();
  // return the color
	DWORD GetColor();
  // returns animation (can be NULL)
	CAnimation * GetAnimation();

  // deletes the land type
	void Delete();
  // creates the land type (width no animation)
	BOOL Create(BYTE nID, CString strName, DWORD dwColor);
  // creates the land type (with animation)
	BOOL Create(BYTE nID, CString strName, CAnimation *pAnimation, DWORD dwColor);

  // constructor & destructor
	CLandType();
	virtual ~CLandType();

private:
  // color for use if no animation
  DWORD m_dwColor;
  // animation graphics - if NULL use the m_dwColor to fill the rect with
  CAnimation *m_pAnimation;
  // ID (just BYTE)
  BYTE m_nID;
  // name of the land type
  CString m_strName;
};

#endif // !defined(AFX_LANDTYPE_H__39607C56_6A80_11D3_8C66_00105ACA8325__INCLUDED_)
