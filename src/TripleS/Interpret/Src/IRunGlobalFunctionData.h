// IRunGlobalFunctionData.h: interface for the CIRunGlobalFunctionData class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_IRUNGLOBALFUNCTIONDATA_H__BB4E3383_1B4A_11D4_B042_004F49068BD6__INCLUDED_)
#define AFX_IRUNGLOBALFUNCTIONDATA_H__BB4E3383_1B4A_11D4_B042_004F49068BD6__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CIProcess;

class CIRunGlobalFunctionData  
{
public:
	CIRunGlobalFunctionData( LPCTSTR lpszGlobalFunctionName, CIBag &Bag) 
    {
        ASSERT( lpszGlobalFunctionName != NULL);
        m_stiName = g_StringTable.AddItem( lpszGlobalFunctionName);

        m_Bag = Bag;

		m_nProcessID = 0;

		m_ProcessEvent.ResetEvent();
    }

	virtual ~CIRunGlobalFunctionData()
    {
        if (m_stiName != NULL) m_stiName = m_stiName->Release();
    }

public:
    // jmeno
    CStringTableItem *m_stiName;

    // bag
    CIBag m_Bag;

	// id processu, ve kterem bezi interpretace globalni funkce.
	// 0 znamena neplatna hodnota
	unsigned long m_nProcessID;

	// eventa, ktera je nastavena, jakmile je v m_nProcessID platna hodnota
	CEvent m_ProcessEvent;
};

#endif // !defined(AFX_IRUNGLOBALFUNCTIONDATA_H__BB4E3383_1B4A_11D4_B042_004F49068BD6__INCLUDED_)
