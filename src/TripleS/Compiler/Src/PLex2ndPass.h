// PLex2ndPass.h: interface for the CPLex2ndPass class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_PLEX2NDPASS_H__154C5E44_9FF9_11D3_AF82_004F49068BD6__INCLUDED_)
#define AFX_PLEX2NDPASS_H__154C5E44_9FF9_11D3_AF82_004F49068BD6__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef yyFlexLexer
#define yyFlexLexer yy2ndFlexLexer
#include <FlexLexer.h>
#undef yyFlexLexer
#endif

class CPLex2ndPass : public yy2ndFlexLexer  
{
public:
	CPLex2ndPass(CCompilerInput *pStdIn, CCompilerKernel *pCompilerKrnl);
	virtual ~CPLex2ndPass();

    // tohle vola lex, kdyz chce vstup
    virtual int LexerInput( char* buf, int max_size );

    // tohle vola bison, kdyz chce token
    virtual int YyLex(CPExpr *pExpr);

    virtual int yylex();

    inline int GetLineNo() { return yylineno; }
    inline void SetLineNo( int nLineNo) { yylineno = nLineNo; }

protected:
    CCompilerInput *m_pStdIn;
    CPExpr *m_pExpr;
    int m_nCommentLevel, m_nInt;
    CString m_strString;

    CCompilerKernel *m_pCompilerKrnl;
};

#endif // !defined(AFX_PLEX2NDPASS_H__154C5E44_9FF9_11D3_AF82_004F49068BD6__INCLUDED_)
