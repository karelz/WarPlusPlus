// ISOString.h: interface for the CISOString class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_ISOSTRING_H__0FB42DD4_C420_11D3_AFB7_004F49068BD6__INCLUDED_)
#define AFX_ISOSTRING_H__0FB42DD4_C420_11D3_AFB7_004F49068BD6__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "IDuplicSystemObject.h"

class CISOString : public CIDuplicSystemObject
{
public:
	CISOString();
	virtual ~CISOString();

public:
    // vytvoreni & zruseni
    virtual bool Create( CInterpret *pInterpret);
    virtual void Delete();
    
    // nastaveni hodnoty - pouziva se z C++ kodu pro naplneni objektu
    inline void Set(CString &strString) { m_strString = strString; }
    inline void Set(char cChar) { m_strString = cChar; }
    inline void Set(LPCTSTR lpszString) { m_strString =lpszString; }

    // load & store polozek objektu
    // objekt String nema zadne polozky
    virtual bool LoadMember( CIDataStackItem&, long) { return false; }
    virtual bool StoreMember( CIDataStackItem*, long) { return false; }

	// duplikace
    virtual bool CreateFrom( CInterpret *pInterpret, CIDuplicSystemObject *pSrc);

  // vraci hodnotu
    CString Get(){ return m_strString; }

	// indexace []
    inline bool GetAt(int i, char &c) 
	{ 
		if (i<0 || i>=m_strString.GetLength()) return false;
		else 
		{
			c = m_strString.GetAt(i);
			return true;
		}
	}

    inline bool SetAt(int i, char c) 
	{ 
		if (i<0 || i>=m_strString.GetLength()) return false;
		else 
		{
			m_strString.SetAt(i,c); 
			return true;
		}
	}

// METODY
protected:
	DECLARE_SYSCALL_INTERFACE_MAP(CISOString, 8);

	DECLARE_SYSCALL_METHOD0(M_GetLength);
	DECLARE_SYSCALL_METHOD2(M_Mid, Int, int *, Int, int *);
	DECLARE_SYSCALL_METHOD1(M_Left, Int, int *);
	DECLARE_SYSCALL_METHOD1(M_Right, Int, int *);
	DECLARE_SYSCALL_METHOD0V(M_MakeLower);
	DECLARE_SYSCALL_METHOD0V(M_MakeUpper);
	DECLARE_SYSCALL_METHOD1(M_Find, String, CISOString *);
	DECLARE_SYSCALL_METHOD2(M_FindFrom, String, CISOString *, Int, int *);


// 0:
    ESyscallResult M_GetLength( CIProcess*, CISyscall **pSyscall, CIDataStackItem *pRet);
// 1:
    ESyscallResult M_Mid( CIProcess*, CISyscall **pSyscall, CIDataStackItem *pRet, int *pPos, int *pCount);
// 2:
    ESyscallResult M_Left( CIProcess*, CISyscall **pSyscall, CIDataStackItem *pRet, int *pCount);
// 3:
    ESyscallResult M_Right( CIProcess*, CISyscall **pSyscall, CIDataStackItem *pRet, int *pCount);
// 4:
    ESyscallResult M_MakeLower( CIProcess*, CISyscall **pSyscall);
// 5:
    ESyscallResult M_MakeUpper( CIProcess*, CISyscall **pSyscall);
// 6:
    ESyscallResult M_Find( CIProcess*, CISyscall **pSyscall, CIDataStackItem *pRet, CISOString *pSubstr);
// 7:
    ESyscallResult M_FindFrom( CIProcess*, CISyscall **pSyscall, CIDataStackItem *pRet, CISOString *pSubstr, int *pStart);

// polozky
public:
    CString m_strString;

/*
object String
{
1: int GetLength();
2: String Mid(int first, int count);
3: String Left(int count);
4: String Right(int count);
5: void MakeLower();
6: void MakeUpper();
7: int Find(String substring);
8: int FindFrom(String substring, int start);
}
*/

public:
// 
// SAVE & LOAD
//
	void PersistentSave( CPersistentStorage &storage);
	void PersistentLoad( CPersistentStorage &storage);
	void PersistentTranslatePointers( CPersistentStorage &storage);
	void PersistentInit();
};

#endif // !defined(AFX_ISOSTRING_H__0FB42DD4_C420_11D3_AFB7_004F49068BD6__INCLUDED_)
