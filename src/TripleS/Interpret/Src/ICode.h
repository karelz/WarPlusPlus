/************************************************************
 *
 * Projekt: Strategicka hra
 *    Cast: Interpret
 *   Autor: Helena Kupková
 *  
 *   Popis: Implementace tridy CICode uchovavajici kod,
 *          ktery je implementovan interpretem.
 * 
 ***********************************************************/

#if !defined(AFX_ICODE_H__8DFAC1D5_9D12_11D3_AF7E_004F49068BD6__INCLUDED_)
#define AFX_ICODE_H__8DFAC1D5_9D12_11D3_AF7E_004F49068BD6__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CFile;

class CICode  
{
public:
	CICode();
	virtual ~CICode();

    // naloadovani
    void Load(CFile &file);

    // porovnani
    bool operator==(CICode &anotherOne);

public:
    // kod
    INSTRUCTION* m_pCode;   // sizeof(INSTRUCTION) == sizeof(REG)

    // delka kodu
    long m_nLength;
};

#endif // !defined(AFX_ICODE_H__8DFAC1D5_9D12_11D3_AF7E_004F49068BD6__INCLUDED_)
