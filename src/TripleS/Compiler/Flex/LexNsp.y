/* Lexikalni "gramatika" pro prvni pruchod - zjisteni vsech metod jednotek */

%option prefix="nsp"
%option noyywrap
%option always-interactive
%option c++
%option yyclass="CPLexNsp"
%option yylineno

%{
/************************************************************
 *
 * Projekt: Strategicka hra
 *    Cast: Prekladac
 *   Autor: Helena Kupková
 *  
 *   Popis: Lexikalni analyza pro nacteni namespace	z konfig.souboru
 * 
 ***********************************************************/

class CObject;

#include "stdafx.h"
#include "CompilerKernel.h"
#include "PLexNsp.h"
#include "Nsp.h"

#define YY_SKIP_YYWRAP

// inicializace
#define YY_USER_INIT   BEGIN(INITIAL)

%}

/* exclusive start conditions */

%x COMMENT

/* definitions */

%%

    /* RULES */

[ \t]+		/* empty - whitespace */

"//"[^\n]*	/* one line comment*/

"/*"	/* multiline comment */ m_nCommentLevel=1; BEGIN(COMMENT);

\r       /* empty */
\n		 /* empty */

<COMMENT>{
    "/*"	    m_nCommentLevel++; 
[^*\n/]*		/* empty - anything but a "*" or "\n" or "/" */
"*"+[^*/\n]*  	/* empty - "*" and anything but "*" and "/" */
\n              /* new line in comment */ 
"*"+"/"			m_nCommentLevel--; if (m_nCommentLevel==0) BEGIN(INITIAL);
}


object	return NSP_OBJECT;
event	return NSP_EVENT;
functions	return NSP_FUNCTIONS;
noinstances return NSP_NOINSTANCES;

[0-9]+	{
			m_pToken->m_nNumber=atoi(yytext);
			return NSP_NUMBER;
		}


"{"	|
"}"	|
"["	|
"]" |
";" |
"("	|
")"	|
":"		return yytext[0];


[A-Za-z_][A-Za-z_0-9]*	    {
                               m_pToken->m_strString=yytext;
                               return NSP_IDENTIFIER;
							}

.           return yytext[0];


