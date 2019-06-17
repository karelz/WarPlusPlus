// UnitTBButton.h: interface for the CUnitTBButton class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_UNITTBBUTTON_H__A26A6477_E875_11D3_A8CB_00105ACA8325__INCLUDED_)
#define AFX_UNITTBBUTTON_H__A26A6477_E875_11D3_A8CB_00105ACA8325__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "ImageTBButton.h"
#include "..\DataObjects\EUnitType.h"
#include "..\DataObjects\ECivilization.h"
#include "..\DataObjects\EUnitAppearance.h"

class CUnitTBButton : public CImageTBButton  
{
public:
	CECivilization * GetUnitCivilization();
  CEUnitType *GetUnitType();
  void SetUnitType(CEUnitType *pUnitType, CECivilization *pCivilization);
	CUnitTBButton();
	virtual ~CUnitTBButton();

private:
  CEUnitType * m_pUnitType;
  CECivilization * m_pCivilization;
  CEUnitAppearance m_AppInstance;
protected:
	virtual void BeforePaint();
};

#endif // !defined(AFX_UNITTBBUTTON_H__A26A6477_E875_11D3_A8CB_00105ACA8325__INCLUDED_)
