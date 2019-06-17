// ILexDefaultUnit.h: interface for the CLexDefaultUnit class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_ILEXDEFAULTUNIT_H__BBFC77D3_9EBD_11D3_AF80_004F49068BD6__INCLUDED_)
#define AFX_ILEXDEFAULTUNIT_H__BBFC77D3_9EBD_11D3_AF80_004F49068BD6__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef yyFlexLexer
#define yyFlexLexer DUnitFlexLexer
#include <FlexLexer.h>
#undef yyFlexLexer
#endif

typedef struct tagSLDefaultUnitTokenData
{
    CString m_strString;
    int m_nNumber;
} SLDefaultUnitTokenData;

class CLexDefaultUnit : public DUnitFlexLexer  
{
public:
	CLexDefaultUnit(CArchiveFile *pfileInput);
	virtual ~CLexDefaultUnit();

    virtual int LexerInput( char* buf, int max_size );

    // tohle vola bison, kdyz chce token
    virtual int YyLex(SLDefaultUnitTokenData *yylval);
    // tohle vola bison, kdyz chce hlasit chybu
    int YyError(char *lpszError);
    
    virtual int yylex();

    int GetLineNo() { return yylineno; }

protected:
    CArchiveFile *m_pfileInput;
    SLDefaultUnitTokenData *m_pToken;
};

#endif // !defined(AFX_ILEXDEFAULTUNIT_H__BBFC77D3_9EBD_11D3_AF80_004F49068BD6__INCLUDED_)
