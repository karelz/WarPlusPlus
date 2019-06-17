// PLexNsp.h: interface for the CPLexNsp class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_PLEXNSP_H__154C5E45_9FF9_11D3_AF82_004F49068BD6__INCLUDED_)
#define AFX_PLEXNSP_H__154C5E45_9FF9_11D3_AF82_004F49068BD6__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef yyFlexLexer
#define yyFlexLexer nspFlexLexer
#include <FlexLexer.h>
#undef yyFlexLexer
#endif

typedef struct tagNspTokenData
{
    CString m_strString;
    long m_nNumber;
} SNspTokenData;

class CFile;

class CPLexNsp : public nspFlexLexer 
{
public:
	CPLexNsp(CArchiveFile *pfileInput);
	virtual ~CPLexNsp();      

    // tohle vola lex, kdyz chce vstup
    virtual int LexerInput( char* buf, int max_size );

    // tohle vola bison, kdyz chce token
    virtual int YyLex(SNspTokenData *pToken);

    virtual int yylex();

    inline int GetLineNo() { return yylineno; }

public:
    int m_cLastChar;

protected:
    CArchiveFile *m_pfileInput;
    SNspTokenData *m_pToken;
    int m_nCommentLevel;
};

#endif // !defined(AFX_PLEXNSP_H__154C5E45_9FF9_11D3_AF82_004F49068BD6__INCLUDED_)
