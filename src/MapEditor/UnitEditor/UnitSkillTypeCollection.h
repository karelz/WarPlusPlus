// UnitSkillTypeCollection.h: interface for the CUnitSkillTypeCollection class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_UNITSKILLTYPECOLLECTION_H__14C5FA75_99B3_11D3_A0DD_B66FF94DE631__INCLUDED_)
#define AFX_UNITSKILLTYPECOLLECTION_H__14C5FA75_99B3_11D3_A0DD_B66FF94DE631__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "UnitSkillTypeDlg.h"

class CUnitSkillTypeCollection  
{
public:
	CUnitSkillTypeDlg * GetByName(CString strName);
	void FillListCtrl(CListCtrl *pListCtrl);
	void Delete();
	void Create();
	CUnitSkillTypeCollection();
	virtual ~CUnitSkillTypeCollection();

  CTypedPtrList<CPtrList, CUnitSkillTypeDlg *> m_listSkillTypeDlgs;
};

#endif // !defined(AFX_UNITSKILLTYPECOLLECTION_H__14C5FA75_99B3_11D3_A0DD_B66FF94DE631__INCLUDED_)
