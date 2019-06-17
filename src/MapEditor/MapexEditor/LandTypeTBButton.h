// LandTypeTBButton.h: interface for the CLandTypeTBButton class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_LANDTYPETBBUTTON_H__5AAAB756_6C3C_11D3_8C69_00105ACA8325__INCLUDED_)
#define AFX_LANDTYPETBBUTTON_H__5AAAB756_6C3C_11D3_8C69_00105ACA8325__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "..\DataObjects\LandType.h"
#include "..\Controls\ImageTBButton.h"

class CLandTypeTBButton : public CImageTBButton  
{
public:
	CLandType * GetLandType();
	void SetLandType(CLandType *pLandType);
	CLandTypeTBButton();
	virtual ~CLandTypeTBButton();

protected:
	CLandType * m_pLandType;
};

#endif // !defined(AFX_LANDTYPETBBUTTON_H__5AAAB756_6C3C_11D3_8C69_00105ACA8325__INCLUDED_)
