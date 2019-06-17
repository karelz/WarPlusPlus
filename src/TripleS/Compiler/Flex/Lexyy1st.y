/* Lexikalni "gramatika" pro prvni pruchod - zjisteni vsech metod jednotek */

%option prefix="yy1st"
%option noyywrap
%option always-interactive
%option c++
%option yyclass="CPLex1stPass"
%option yylineno

%{
/************************************************************
 *
 * Projekt: Strategicka hra
 *    Cast: Prekladac
 *   Autor: Helena Kupková
 *  
 *   Popis: Lexikalni analyza prvniho pruchodu (funkce yy1stlex())
 * 
 ***********************************************************/

class CObject;

#include "stdafx.h"
#include "CompilerKernel.h"
#include "PLex1stPass.h"

int nBracesLevel=0;
int nCommentLevel1st;
bool bIgnore=false;

#define YY_SKIP_YYWRAP

// inicializace
#define YY_USER_INIT   BEGIN(INITIAL)

%}

/* exclusive start conditions */

%x COMMENT
%x STRING


/* definitions */

DIGIT	[0-9]
                                                            
%%

    /* RULES */

[ \t]+		/* empty - whitespace */

"//"[^\n]*	/* one line comment*/

"/*"	/* multiline comment */ nCommentLevel1st=1; BEGIN(COMMENT);

<COMMENT>{
    "/*"	    nCommentLevel1st++; 
[^*\n/]*		/* empty - anything but a "*" or "\n" or "/" */
"*"+[^*/\n]*  	/* empty - "*" and anything but "*" and "/" */
\n              /* new line in comment */ 
"*"+"/"			nCommentLevel1st--; if (nCommentLevel1st==0) BEGIN(INITIAL);
}


\"		BEGIN(STRING); 

<STRING>{
"\""		BEGIN(INITIAL); 
[^\"]+		/*empty*/
}

\r       /* empty */
\n		 /* empty */

"unit"			{
					if (!bIgnore) 
					{
						nBracesLevel=0; 
						return FIRSTPASS_UNIT_KEYWORD;
					}
				}

"constructor"     /* empty */

"extends"		if (!bIgnore) return FIRSTPASS_EXTENDS;						

"struct"        if (!bIgnore) return FIRSTPASS_STRUCT;			

"{"		{
			if (m_bSearchSets)
            {
                int c;
				while ((c=yyinput())==' ' || c=='\t') ;
				if (c==EOF) return 0;
                if (c=='}') m_nArrayLevel=-1;
                else
                {
                    nBracesLevel++;
        			if (nBracesLevel>=2) bIgnore=true;
		        	else return '{';
                }
            }
            else
            {
                nBracesLevel++;
        		if (nBracesLevel>=2) bIgnore=true;
		        else return '{';
            }
		}


"}"		{
			nBracesLevel--;
 			if (nBracesLevel==1) bIgnore=false;
			if (nBracesLevel<1) return '}';
		}
	

":"     if (!bIgnore) return ':';
"("     if (!bIgnore) return '(';
")"     if (!bIgnore) return ')';
","     if (!bIgnore) return ',';
";"     if (!bIgnore) return ';';

"["     {
			if (!bIgnore) 
			{   // rozsah pole
				int c;
				while ((c=yyinput())!=']' && c!=EOF) ;
				if (c==EOF) return 0;
				m_nArrayLevel++;
			}
        }

[A-Za-z_][A-Za-z_0-9]*	    {
								if (!bIgnore) 
								{
									m_nArrayLevel = 0;
									return FIRSTPASS_IDENTIFIER;
								}
							}

.		if (!bIgnore) return yytext[0];
