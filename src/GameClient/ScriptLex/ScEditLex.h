/************************************************************
 *
 * Projekt: Strategicka hra
 *    Cast: Editace skriptu - syntax highlight
 *   Autor: Helena Kupková
 *  
 *   Popis: Objekt CScEditLex implementujici rozhrani k lexem 
 *          vygenerovanemu automatu
 * 
 ***********************************************************/

#if !defined(AFX_ScEditLEX_H__7E976304_8AFE_11D3_AF5D_004F49068BD6__INCLUDED_)
#define AFX_ScEditLEX_H__7E976304_8AFE_11D3_AF5D_004F49068BD6__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "ScEditLexInput.h"

class CScEditLexToken
{
public:
    CScEditLexToken();
    ~CScEditLexToken() {}
public:
    enum tagScEditToken {
      SLTComment = 0,
      SLTString = 1,
      SLTInteger = 2,
      SLTFloat = 3,
      SLTKeyword = 4,
	  SLTConstant = 5,
	  SLTDataType = 6,
	  SLTOperator = 7,
      SLTText = 8,
      SLTChar = 9,
      SLTHex = 10,
      SLTEndOfFile = 11,
      SLTError = 12,
	  };

    typedef tagScEditToken EScEditToken;

    EScEditToken m_nToken;
    DWORD m_nStartRow, m_nEndRow;
    WORD m_nStartColumn, m_nEndColumn;

    void SetStart(EScEditToken nToken, DWORD nStartRow, WORD nStartColumn);
    void SetEnd(DWORD nEndRow, WORD nEndColumn);
};

class CScEditLex  
{
public:
	CScEditLex();
	virtual ~CScEditLex();

    bool Create(CScEditLexInput *pInput);
    void Delete();

    void Reset(CScEditLexInput *pInput, DWORD nRow, WORD nColumn);

    void GetToken(CScEditLexToken &Token);

// tohle musi byt public, ponevadz se na to pristupuje z lexe
    CScEditLexToken *m_pToken;

    CScEditLexInput *m_pInput;
};

#endif // !defined(AFX_ScEditLEX_H__7E976304_8AFE_11D3_AF5D_004F49068BD6__INCLUDED_)
