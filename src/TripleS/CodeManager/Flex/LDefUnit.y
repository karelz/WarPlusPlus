/* Lexikalni "gramatika" pro prvni pruchod - zjisteni vsech metod jednotek */

%option noyywrap
%option always-interactive
%option c++
%option yyclass="CLexDefaultUnit"
%option prefix="DUnit"
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

#include "srdafx.h"
#include "..\CodeManager.h"
#include "LexDefaultUnit.h"
#include "SDefUnit.h"

int nCommentLevelDefUnit;

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

"/*"	/* multiline comment */ nCommentLevelDefUnit=1; BEGIN(COMMENT);

\r       /* empty */
\n		 /* empty */

<COMMENT>{
    "/*"	    nCommentLevelDefUnit++; 
[^*\n/]*		/* empty - anything but a "*" or "\n" or "/" */
"*"+[^*/\n]*  	/* empty - "*" and anything but "*" and "/" */
\n              /* new line in comment */ 
"*"+"/"			nCommentLevelDefUnit--; if (nCommentLevelDefUnit==0) BEGIN(INITIAL);
}


unit  return LDF_UNIT;
struct  return LDF_STRUCT;
object  return LDF_OBJECT;

"{"	|
"}"	|
"["	|
"]" |
";" |
"("	|
")"	|
":"		return yytext[0];


[A-Za-z_][A-Za-z_0-9]*	    {
                               this->m_pToken->m_strString=yytext;
                               return LDF_IDENTIFIER;
							}

.           return yytext[0];


