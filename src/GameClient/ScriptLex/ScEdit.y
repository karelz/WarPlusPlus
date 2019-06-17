%option prefix="scedit"
%option noyywrap
%option never-interactive

%{
/************************************************************
 *
 * Projekt: Strategicka hra
 *    Cast: Editace skritpu - syntax highlight
 *   Autor: Helena Kupková
 *  
 *   Popis: Lexikalni analyza pro editaci skriptu (syntax highlight) -
 *          - z teto gramatiky bude lexem vygenerovan automat
 * 
 ***********************************************************/

#define YY_SKIP_YYWRAP

#include "stdafx.h"
#include "ScEditLex.h"
#include "ScEditLexGlobals.h"

// globalni promenne pri cteni

DWORD gnOffset=-1;
DWORD gnRow=-1;
WORD gnColumn=-1;

// input
#define YY_INPUT(buf, result, max_size) \
{ \
    if (gpScEditLex->m_pInput->IsEOF()) result=YY_NULL;   \
    else \
    { \
        buf[0] = gpScEditLex->m_pInput->GetCharacter();	\
        result=1; \
    }\
}

#define YY_INIT    { BEGIN(INITIAL);  nCommentLevel=0; }

#define SetToken(token) { gpScEditLex->m_pToken->SetStart(CScEditLexToken::token, g_nRow, g_nColumn); \
                IncreaseColumn(); \
                gpScEditLex->m_pToken->SetEnd(g_nRow, g_nColumn); \
                return 0; }

#define  IncreaseColumn()  { g_nColumn+=yyleng; }

// uvolneni bufferu
void FreeScEditLexBuffer()
{
	if (yy_init!=1)
	{
		yy_delete_buffer(yy_current_buffer);
		yy_init=1;
	}
}

// lokalne globalni promenne

int nCommentLevel=0;

%}


/* exclusive start conditions */

%x COMMENT

/* definitions */

DIGIT	[0-9]
                                                            
%%

    /* RULES */

[ \t]+		IncreaseColumn(); /* empty - whitespace */

"//"[^\n]*	SetToken(SLTComment);

"/*"	{  /* multiline comment */
            gpScEditLex->m_pToken->SetStart(CScEditLexToken::SLTComment, g_nRow, g_nColumn);
            IncreaseColumn();
			nCommentLevel=1;
            BEGIN(COMMENT);
		}

<COMMENT>{
"/*"		    {
                    IncreaseColumn();
                    nCommentLevel++;
                }

[^*\n/]*		IncreaseColumn(); /* empty - anything but a '*' or '/n' or '/' */

"*"+[^*/\n]*  	IncreaseColumn(); /* empty - '*' and anything but '*' and '/' */

\n				{
                    g_nColumn=0;
                    g_nRow++;
                }

"*"+"/"			{
                    IncreaseColumn();
					nCommentLevel--;
					if (nCommentLevel==0) 
                    {
                        BEGIN(INITIAL);
                        gpScEditLex->m_pToken->SetEnd(g_nRow, g_nColumn);
                        return 0;
                    }
				}

<<EOF>>         {
                    BEGIN(INITIAL);
                    nCommentLevel=0;
                    gpScEditLex->m_pToken->SetEnd(g_nRow, g_nColumn);
                    return 0;
                }
}


\"([^\n\"]|"\\\"")*\"	{
				            SetToken(SLTString);
                        }

\"([^\n\"]|"\\\"")*	{
				            SetToken(SLTString);
                        }

\'([^\n\'])*\'	{
				            SetToken(SLTChar);
                        }

\'([^\n\'])*	{
				            SetToken(SLTChar);
                        }

\r              IncreaseColumn();

\n			    {
                    g_nColumn=0;
                    g_nRow++;
                }

"0"[xX][0-9A-Ba-b]+     SetToken(SLTHex);

{DIGIT}+	    SetToken(SLTInteger);
			
{DIGIT}+"."{DIGIT}*	    SetToken(SLTFloat);

{DIGIT}+[Ee]{DIGIT}+	SetToken(SLTFloat);

{DIGIT}+"."{DIGIT}*[Ee]{DIGIT}+		SetToken(SLTFloat);

void | 
unit | 
extends | 
if | 
else | 
command | 
notification |
while | 
do | 
switch |
case | 
constructor | 
destructor | 
return | 
break | 
continue | 
in |
for | 
sendto | 
wait | 
nowait | 
waitfirst | 
instanceof | 
default |
try | 
catch | 
this    {
           SetToken(SLTKeyword);
       }

true |
false |
null   {
           SetToken(SLTConstant);
       }

int | 
float | 
bool | 
char |
String |
Client |
Map |
Path |
Position |
Resources   {
           SetToken(SLTDataType);
       }

"++" | 
"--" | 
"<-" | 
"==" | 
"!=" | 
"<=" | 
">=" | 
"+=" | 
"-=" | 
"*=" |    
"/=" | 
"||" | 
"&&" | 
"+"  | 
"-"  | 
"*"  | 
"/"	 | 
"%"  | 
"<"  | 
"="	 |
";"  | 
":"	 | 
"("	 | 
")"  | 
"["  | 
"]"	 | 
"{"	 |
"}"	 | 
"."	 | 
","  |
">"	 | 
"!"	       SetToken(SLTOperator);

"\\"	SetToken(SLTText);


[A-Za-z_][A-Za-z_0-9]*		SetToken(SLTText);

<<EOF>>		SetToken(SLTEndOfFile);

.	    SetToken(SLTError);


