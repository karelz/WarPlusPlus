// MapexTBButton.h: interface for the CMapexTBButton class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MAPEXTBBUTTON_H__23152139_6E73_11D3_A06B_C85E707FE131__INCLUDED_)
#define AFX_MAPEXTBBUTTON_H__23152139_6E73_11D3_A06B_C85E707FE131__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "ImageTBButton.h"
#include "..\DataObjects\EMapex.h"

class CMapexTBButton : public CImageTBButton  
{
public:
	CEMapex * GetMapex();
	void SetMapex(CEMapex *pMapex);
	CMapexTBButton();
	virtual ~CMapexTBButton();

private:
	CEMapex * m_pMapex;
};

#endif // !defined(AFX_MAPEXTBBUTTON_H__23152139_6E73_11D3_A06B_C85E707FE131__INCLUDED_)
